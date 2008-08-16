/** 
 * @file llvopart.cpp
 * @brief Viewer-object derived particle system.
 *
 * Copyright (c) 2001-2007, Linden Research, Inc.
 * 
 * The source code in this file ("Source Code") is provided by Linden Lab
 * to you under the terms of the GNU General Public License, version 2.0
 * ("GPL"), unless you have obtained a separate licensing agreement
 * ("Other License"), formally executed by you and Linden Lab.  Terms of
 * the GPL can be found in doc/GPL-license.txt in this distribution, or
 * online at http://secondlife.com/developers/opensource/gplv2
 * 
 * There are special exceptions to the terms and conditions of the GPL as
 * it is applied to this Source Code. View the full text of the exception
 * in the file doc/FLOSS-exception.txt in this software distribution, or
 * online at http://secondlife.com/developers/opensource/flossexception
 * 
 * By copying, modifying or distributing this software, you acknowledge
 * that you have read and understood your obligations described above,
 * and agree to abide by those obligations.
 * 
 * ALL LINDEN LAB SOURCE CODE IS PROVIDED "AS IS." LINDEN LAB MAKES NO
 * WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
 * COMPLETENESS OR PERFORMANCE.
 */

#include "llviewerprecompiledheaders.h"

#include "llvopart.h"

#include "llfasttimer.h"
#include "message.h"

#include "llagent.h"
#include "lldrawable.h"
#include "llface.h"
#include "llsky.h"
#include "llviewercamera.h"
#include "llviewerimagelist.h"
#include "llviewerregion.h"
#include "pipeline.h"

const F32 MAX_PART_LIFETIME = 120.f;

extern U64 gFrameTime;

LLVOPart::LLVOPart(const LLUUID &id, const LLPCode pcode, LLViewerRegion *regionp)
:	LLViewerObject(id, pcode, regionp)
{
	mParticlesDead = FALSE;
	setNumTEs(1);
	setDefaultValues();

	mbCanSelect = FALSE;			// users can't select particle systems
	mNumLiveParticles = 0;
}


LLVOPart::~LLVOPart()
{
	delete [] mParticleState;
	mParticleState = NULL;

	delete [] mDeadArr;
	mDeadArr = NULL;
}

void LLVOPart::initClass()
{
}

U32 LLVOPart::processUpdateMessage(LLMessageSystem *mesgsys,
										  void **user_data,
										  U32 block_num,
										  const EObjectUpdateType update_type,
										  LLDataPacker *dp)
{
	S32 dataSize;
	U8 packed_psys_data[180];
	// Do base class updates...
	mTimeLastFrame = gFrameTime;
	U32 retval = LLViewerObject::processUpdateMessage(mesgsys, user_data, block_num, update_type, dp);

	if (update_type == OUT_TERSE_IMPROVED)
	{
		// Nothing else needs to be done for the terse message.
		return retval;
	}
	
	dataSize = mesgsys->getSizeFast(_PREHASH_ObjectData, block_num, _PREHASH_Data);

	if(dataSize == sizeof(LLPartInitData)) 
	{
		// Uncompressed particle.  Is this used?  JC
		mesgsys->getBinaryDataFast(_PREHASH_ObjectData, _PREHASH_Data, &mInitSysData, dataSize, block_num);

		if(mInitSysData.createMe)
		{
			if (mInitSysData.initialParticles >= mInitSysData.maxParticles)
			{
				mInitSysData.initialParticles = mInitSysData.maxParticles - 1;
			}
			setParticleParams(mInitSysData.bounce_b, 
							  getPositionRegion().mV, 
							  getRotation().mQ, 
							  mInitSysData.maxParticles, 
							  mInitSysData.mImageUuid,
							  mInitSysData.mFlags);
			initializeParticlesAndConstraints(mInitSysData.initialParticles,
														mInitSysData.diffEqAlpha,
														mInitSysData.diffEqScale,
														mInitSysData.scale_range,
														mInitSysData.alpha_range,
														mInitSysData.vel_offset,
														mInitSysData.killPlaneZ,
														mInitSysData.killPlaneNormal,
														mInitSysData.bouncePlaneZ,
														mInitSysData.bouncePlaneNormal,
														mInitSysData.spawnRange,
														mInitSysData.spawnFrequency,
														mInitSysData.spawnFreqencyRange,
														mInitSysData.spawnDirection,
														mInitSysData.spawnDirectionRange,
														mInitSysData.spawnVelocity,
														mInitSysData.spawnVelocityRange,
														mInitSysData.speedLimit,
														mInitSysData.windWeight,
														mInitSysData.currentGravity,
														mInitSysData.gravityWeight,
														mInitSysData.globalLifetime,
														mInitSysData.individualLifetime,
														mInitSysData.individualLifetimeRange,
														mInitSysData.alphaDecay,
														mInitSysData.scaleDecay,
														mInitSysData.distanceDeath,
														mInitSysData.dampMotionFactor,
														mInitSysData.windDiffusionFactor);
			setParticlesDistFadeout(mInitSysData.mDistBeginFadeout, 
					mInitSysData.mDistEndFadeout);
		}
	}
	else if(dataSize > 4) 
	{
		mesgsys->getBinaryDataFast(_PREHASH_ObjectData, _PREHASH_Data, packed_psys_data, dataSize, block_num);	
	
		LLPartSysCompressedPacket  CompObjectData;
		U32 sizeUsed;
	
		CompObjectData.fromUnsignedBytes(packed_psys_data, dataSize);
		CompObjectData.toLLPartInitData(&mInitSysData, &sizeUsed);
		
		if(mInitSysData.createMe)
		{
			if (mInitSysData.initialParticles >= mInitSysData.maxParticles)
			{
				mInitSysData.initialParticles = mInitSysData.maxParticles - 1;
			}
			setParticleParams(mInitSysData.bounce_b, 
							  getPositionRegion().mV, 
							  getRotation().mQ, 
							  mInitSysData.maxParticles, 
							  mInitSysData.mImageUuid, 
							  mInitSysData.mFlags);
			initializeParticlesAndConstraints(mInitSysData.initialParticles,
													mInitSysData.diffEqAlpha,
													mInitSysData.diffEqScale,
													mInitSysData.scale_range,
													mInitSysData.alpha_range,
													mInitSysData.vel_offset,
													mInitSysData.killPlaneZ,
													mInitSysData.killPlaneNormal,
													mInitSysData.bouncePlaneZ,
													mInitSysData.bouncePlaneNormal,
													mInitSysData.spawnRange,
													mInitSysData.spawnFrequency,
													mInitSysData.spawnFreqencyRange,
													mInitSysData.spawnDirection,
													mInitSysData.spawnDirectionRange,
													mInitSysData.spawnVelocity,
													mInitSysData.spawnVelocityRange,
													mInitSysData.speedLimit,
													mInitSysData.windWeight,
													mInitSysData.currentGravity,
													mInitSysData.gravityWeight,
													mInitSysData.globalLifetime,
													mInitSysData.individualLifetime,
													mInitSysData.individualLifetimeRange,
													mInitSysData.alphaDecay,
													mInitSysData.scaleDecay,
													mInitSysData.distanceDeath,
													mInitSysData.dampMotionFactor,
													mInitSysData.windDiffusionFactor);
			setParticlesDistFadeout(mInitSysData.mDistBeginFadeout, 
				mInitSysData.mDistEndFadeout);
		}
	}

	translateParticlesTo(getPositionRegion());
	rotateParticlesTo(getRotation());
	return retval;
}


BOOL LLVOPart::isActive() const
{
	return TRUE;
}

BOOL LLVOPart::idleUpdate(LLAgent &agent, LLWorld &world, const F64 &time)
{
	if (mDeathTimer.getElapsedTimeF32() > MAX_PART_LIFETIME)
	{
		//llinfos << "LLVOPart dead due to extended lifetime" << llendl;
		return FALSE;
	}

	if (gPipeline.hasRenderType(LLPipeline::RENDER_TYPE_PARTICLES))
	{
		if (!mDrawable)
		{
			llwarns << "LLVOPart idle with no drawable!" << llendl;
			return FALSE;
		}
		// I don't know why you'd want to do ANYTHING with invisible particles. ??? - Doug
		if(mFlags[PART_SYS_INVISIBLE_BYTE] & PART_SYS_INVISIBLE_BIT)
		{
			llwarns << "Invisible particle, killing" << llendl;
			return FALSE;
		}

		F64 delta_time = ((S64)(gFrameTime - mTimeLastFrame))*(1.0/((F64)USEC_PER_SEC));
		mParticlesDead = !iterateParticles((F32)delta_time);

		if(mParticlesDead)
		{
			return FALSE;
		}

		translateParticlesTo(getPositionRegion());

		mTimeLastFrame = gFrameTime;
		setChanged(GEOMETRY);
		gPipeline.markRebuild(mDrawable, LLDrawable::REBUILD_VOLUME, TRUE);

	}
	LLViewerObject::idleUpdate(agent, world, time);
	return TRUE;
}


void LLVOPart::updateTextures(LLAgent &agent)
{
	if (getTEImage(0))
	{
		LLVector3 relative_position = getPositionAgent() - agent.getCameraPositionAgent();
		F32 dot_product = relative_position * agent.getFrameAgent().getAtAxis();
		F32 cos_angle = dot_product / relative_position.magVec();

		if (cos_angle > 1.f)
		{
			cos_angle = 1.f;
		}

		getTEImage(0)->addTextureStats(mPixelArea, 1.f, cos_angle);
	}
}


LLDrawable* LLVOPart::createDrawable(LLPipeline *pipeline)
{
	pipeline->allocDrawable(this);
	mDrawable->setLit(FALSE);
	mDrawable->setRenderType(LLPipeline::RENDER_TYPE_PARTICLES);

	LLDrawPool *pool = gPipeline.getPool(LLDrawPool::POOL_ALPHA);
	mDrawable->setNumFaces(mNumPart, pool, getTEImage(0));
	return mDrawable;
}

BOOL LLVOPart::updateGeometry(LLDrawable *drawable)
{
	if (isChanged(LLPrimitive::GEOMETRY))
	{
		LLFace *face;

		///////////////////////
		//
		// Allocate/deallocate faces based on number of particles we need to render
		//
		//
		if (drawable->getNumFaces())
		{
			face = drawable->getFace(0);
			drawable->setNumFaces(mNumPart, face->getPool(), getTEImage(0));
		}
		else
		{
			LLDrawPool *pool = gPipeline.getPool(LLDrawPool::POOL_ALPHA);
			drawable->setNumFaces(mNumPart, pool, getTEImage(0));
		}

		LLVector3 light_norm;

		if (gSky.sunUp())
		{
			light_norm = -gSky.getSunDirection();
		}
		else
		{
			light_norm = -gSky.getMoonDirection();
		}
		light_norm.normVec();
		
		// Figure out the lighting for the particle system.
		LLColor4 color(1.f,1.f,1.f,1.f);
		LLVector3 at, left, up;

		at = gCamera->getAtAxis();
		left = gCamera->getLeftAxis();
		up = gCamera->getUpAxis();

		LLVector3 v_agent[4];

		LLMatrix3 cached_oo;
		cached_oo.setRot(mOriginOrientation);
		U32 i;
		U32 cur_face = 0;
		for (i = 0; i < mNumPart; i++)
		{
			face = drawable->getFace(cur_face++);

			if (0 != mDeadArr[i])
			{
				face->setSize(0);
				continue; // if this particle is dead, don't render it
			}


			LLStrider<LLVector3> verticesp;
			LLStrider<LLVector3> normalsp;
			LLStrider<LLVector2> texCoordsp;
			U32 *indicesp;
			S32 index_offset;

			face->setPrimType(LLTriangles);
			face->setSize(4, 6);
			index_offset = face->getGeometry(verticesp,normalsp,texCoordsp, indicesp);
			if (-1 == index_offset)
			{
				llerrs << "Error allocating geometry!" << llendl;
			}

			LLVector3 position_agent;
			LLVector3 part_pos_local;
			F32 alpha = 1.0f;
			F32 scale = 1.0f; // elements of the particle system have random scales too! -- MDS

			position_agent = mSpawnPoint + getRegion()->getOriginAgent();

			if(mFlags[PART_SYS_FOLLOW_VEL_BYTE] & PART_SYS_FOLLOW_VEL_BIT)
			{
				part_pos_local.mV[0] = mParticleState[i].position[0]*cached_oo.mMatrix[0][0]+
					mParticleState[i].position[1]*cached_oo.mMatrix[0][1]+
					mParticleState[i].position[2]*cached_oo.mMatrix[0][2];
			
				part_pos_local.mV[1] = mParticleState[i].position[0]*cached_oo.mMatrix[1][0]+
					mParticleState[i].position[1]*cached_oo.mMatrix[1][1]+
					mParticleState[i].position[2]*cached_oo.mMatrix[1][2];

				part_pos_local.mV[2] = mParticleState[i].position[0]*cached_oo.mMatrix[2][0]+
					mParticleState[i].position[1]*cached_oo.mMatrix[2][1]+
					mParticleState[i].position[2]*cached_oo.mMatrix[2][2];

				scale = mParticleState[i].scale[0];
				alpha = mParticleState[i].alpha[0]; 

				//26 September 2001 - alter alpha and scale as approach death
				//j = death_offset_i(i);
				scale *= ((1.f - mScaleDecay) + (mScaleDecay * mParticleState[i].deathOffset));
				alpha *= ((1.f - mAlphaDecay) + (mAlphaDecay * mParticleState[i].deathOffset));

				up.mV[0] = -mParticleState[i].position[0]; 
				up.mV[1] = -mParticleState[i].position[1];
				up.mV[2] = -mParticleState[i].position[2]; // set "up" to trail velocity
				
				if(up.magVec() == 0.0f) // alleviate potential divide by zero bug
				{
					up.mV[2] += 1.0f;
				}

				up.normVec();
				up = up - (up*at) * at;  
				
				left = up % at;

				up *= scale * 0.5f;
				left *= scale * 0.5f;
				position_agent += part_pos_local;
				face->mCenterAgent = position_agent;
				v_agent[0] = position_agent + left + up;
				v_agent[1] = position_agent - left + up;
				v_agent[2] = position_agent - left - up;
				v_agent[3] = position_agent + left - up;
				*(texCoordsp) = LLVector2(0.f, 1.f);
				texCoordsp++;
				*(texCoordsp) = LLVector2(0.f, 0.f);
				texCoordsp++;
				*(texCoordsp) = LLVector2(1.f, 1.f);
				texCoordsp++;
				*(texCoordsp) = LLVector2(1.f, 0.f);
				texCoordsp++;
			}
			else
			{
				part_pos_local.mV[0] = mParticleState[i].position[0]*cached_oo.mMatrix[0][0]+
					mParticleState[i].position[1]*cached_oo.mMatrix[0][1]+
					mParticleState[i].position[2]*cached_oo.mMatrix[0][2];
			
				part_pos_local.mV[1] = mParticleState[i].position[0]*cached_oo.mMatrix[1][0]+
					mParticleState[i].position[1]*cached_oo.mMatrix[1][1]+
					mParticleState[i].position[2]*cached_oo.mMatrix[1][2];

				part_pos_local.mV[2] = mParticleState[i].position[0]*cached_oo.mMatrix[2][0]+
					mParticleState[i].position[1]*cached_oo.mMatrix[2][1]+
					mParticleState[i].position[2]*cached_oo.mMatrix[2][2];


				scale = mParticleState[i].scale[0];
				alpha = mParticleState[i].alpha[0]; 

				//26 September 2001 - alter alpha and scale as approach death
				scale *= ((1.f - mScaleDecay) + (mScaleDecay * mParticleState[i].deathOffset));
				alpha *= ((1.f - mAlphaDecay) + (mAlphaDecay * mParticleState[i].deathOffset));
				
				LLVector3 part_up = scale * 0.5f * up;
				LLVector3 part_left = scale * 0.5f * left;

				position_agent += part_pos_local;
				face->mCenterAgent = position_agent;
				v_agent[0] = position_agent + part_left + part_up;
				v_agent[1] = position_agent - part_left + part_up;
				v_agent[2] = position_agent - part_left - part_up;
				v_agent[3] = position_agent + part_left - part_up;
				*(texCoordsp) = LLVector2(0.f, 1.f);
				texCoordsp++;
				*(texCoordsp) = LLVector2(0.f, 0.f);
				texCoordsp++;
				*(texCoordsp) = LLVector2(1.f, 1.f);
				texCoordsp++;
				*(texCoordsp) = LLVector2(1.f, 0.f);
				texCoordsp++;
			}

			color.mV[3] = alpha;
			face->setFaceColor(color);
			
			*(verticesp++)  = v_agent[1];
			*(verticesp++)  = v_agent[2];
			*(verticesp++)  = v_agent[0];
			*(verticesp++)  = v_agent[3];

			*(indicesp++) = index_offset + 0;
			*(indicesp++) = index_offset + 2;
			*(indicesp++) = index_offset + 1;

			*(indicesp++) = index_offset + 1;
			*(indicesp++) = index_offset + 2;
			*(indicesp++) = index_offset + 3;
		}
		LLPipeline::sCompiles++;
	}

	return TRUE;
}


void LLVOPart::setDefaultValues() 
{
	U32	i;

	// initialize to safe but meaningless values : no other constructors 
	mParticleState = NULL;
	mNumPart = 0;
	mAlpha = 1.0f;
	mLastTime = mCurrTime = 0.0f;
	//mOriginPosition[0] = mOriginPosition[1] = mOriginPosition[2] = 0.0f;
	mOriginOrientation.setQuatInit(0.0f, 0.0f, 0.0f, 1.0f);
	mDeadArr = NULL;
	
	mKillPlaneNormal.mV[VX] = 0.0f;//Straight up - needs to be unit
	mKillPlaneNormal.mV[VY] = 0.0f;
	mKillPlaneNormal.mV[VZ] = 1.0f;

	mBouncePlaneNormal.mV[VX] = 0.0f;//Straight up - needs to be unit
	mBouncePlaneNormal.mV[VY] = 0.0f;
	mBouncePlaneNormal.mV[VZ] = 1.0f;

	mSpawnPoint.mV[VX] = 0.0f;
	mSpawnPoint.mV[VY] = 0.0f;
	mSpawnPoint.mV[VZ] = 0.0f;

	mSpawnDirection.mV[VX] = 0.0f;//Straight up - needs to be unit
	mSpawnDirection.mV[VY] = 0.0f;
	mSpawnDirection.mV[VZ] = 1.0f;

	mCurrentWind.mV[VX] = 0.0f;
	mCurrentWind.mV[VY] = 0.0f;
	mCurrentWind.mV[VZ] = 0.0f;

	mCurrentWindMagnitude = 0.0f;
	mCurrentWindMagnitudeSquareRoot = 0.0f;

	mCurrentGravity.mV[VX] = 0.0f;//Straight down
	mCurrentGravity.mV[VY] = 0.0f;
	mCurrentGravity.mV[VZ] = -9.81f;

	mVelocityOffset.mV[VX] = 0.0f;
	mVelocityOffset.mV[VY] = 0.0f;
	mVelocityOffset.mV[VZ] = 0.0f;

	for(i = 0; i < PART_SYS_BYTES_OF_FLAGS; i++) 
	{
		mFlags[i] = 0x00;
	}

	//set default action and kill flags
	//These defaults are for an explosion - a short lived set of debris affected by gravity.
		//Action flags default to PART_SYS_AFFECTED_BY_WIND + PART_SYS_AFFECTED_BY_GRAVITY + PART_SYS_DISTANCE_DEATH 
	mFlags[PART_SYS_ACTION_BYTE] = PART_SYS_AFFECTED_BY_WIND | PART_SYS_AFFECTED_BY_GRAVITY | PART_SYS_DISTANCE_DEATH;
	mFlags[PART_SYS_KILL_BYTE] = PART_SYS_DISTANCE_DEATH + PART_SYS_TIME_DEATH;

	for (i = 0; i < 3; i++)
	{
		mDiffEqAlpha[i] = 0.0f;
		mDiffEqScale[i] = 0.0f;
	}

	mScale_range[0] = 1.00f;
	mScale_range[1] = 5.00f;
	mScale_range[2] = mScale_range[3] = 0.0f;

	mAlpha_range[0] = mAlpha_range[1] = 1.0f;
	mAlpha_range[2] = mAlpha_range[3] = 0.0f;


	mKillPlaneZ = 0.0f;
	mBouncePlaneZ = 0.0f;

	mSpawnRange = 1.0f;
	mSpawnFrequency = 0.0f;
	mSpawnFrequencyRange = 0.0f;
	mSpawnDirectionRange = 1.0f;	//everywhere
	mSpawnVelocity = 0.75f;
	mSpawnVelocityRange = 0.25f;	//velocity +/- 0.25
	mSpeedLimitSquared = 1.0f;
	mWindWeight = 0.5f;	//0.0f means looks like a heavy object (if gravity is on), 1.0f means light and fluffy
	mGravityWeight = 0.5f;	//0.0f means boyed by air, 1.0f means it's a lead weight
	mGlobalLifetime = 0.0f;	//Arbitrary, but default is no global die, so doesn't matter
	mOriginalGlobalLifetime = 0.0f;
	mIndividualLifetime = 5.0f;
	if (mIndividualLifetime > 0.0f)
	{
		mOneOverIndividualLifetime = 1.0f / mIndividualLifetime;
	}
	else
	{
		mOneOverIndividualLifetime = 0.0f;
	}
	mIndividualLifetimeRange = 1.0f;	//Particles last 5 secs +/- 1
	mAlphaDecay = 1.0f;	//normal alpha fadeout
	mScaleDecay = 0.0f;	//no scale decay
	mDistanceDeathSquared = 10.0f;	//die if hit unit radius
	if (mDistanceDeathSquared > 0.0f)
	{
		mOneOverDistanceDeathSquared = 1.0f / mDistanceDeathSquared;
	}
	else
	{
		mOneOverDistanceDeathSquared = 0.0f;
	}
	mDampMotionFactor = 0.0f;

	mWindDiffusionFactor.mV[VX] = 0.0f;
	mWindDiffusionFactor.mV[VY] = 0.0f;
	mWindDiffusionFactor.mV[VZ] = 0.0f;

	mUpdatePhysicsInputsTime = mCurrTime;
}


U8 LLVOPart::setParticlesDistFadeout(F32 beginFadeout, F32 endFadeout)
{
	// This doesn't do anything...
	return 1;
}

unsigned char LLVOPart::setParticleParams(F32 bounce_b, 
										 const F32 o_pos[3], 
										 const F32 o_or[3], 
										 U32 n, 
										 LLUUID image_uuid,
										 U8 flags[PART_SYS_BYTES_OF_FLAGS]) 
{
	mBounceBehavior = bounce_b;
	mSpawnPoint.setVec(o_pos[0], o_pos[1], o_pos[2]);

	mOriginOrientation.setQuatInit(o_or[0], o_or[1], o_or[2], 
		(F32)sqrt(1.0f - o_or[0]*o_or[0] - o_or[1]*o_or[1] - o_or[2]*o_or[2]));

	if(mNumPart < n ) 
	{ 
		// this crazy logic is just in case "setParams" gets called multiple times 
		if(mNumPart != 0) 
		{
			if(mParticleState != NULL)
			{
				delete [] mParticleState;
			}
			if(mDeadArr != NULL)
			{
				delete [] mDeadArr;
			}

			mParticleState = NULL;
			mDeadArr = NULL;
			mNumPart = 0;
		}
		mNumPart = n;
		//state_arr = new F32[n*FLOATS_PER_PARTICLE];
		mParticleState = new OneParticleData[n];
		mDeadArr = new U8[n];
		memset(mDeadArr,1,n);		// initialize these to 1;
	} 
	else
	{
		mNumPart = n;
	}

	setTETexture(0, image_uuid);

	for(U32 i = 0; i< PART_SYS_BYTES_OF_FLAGS; i++)
	{
		mFlags[i] = flags[i];
	}

	return '\0'; // success 
}


void LLVOPart::setParticleCountdownStateWaitingDead(const U32 particleNumber)
{
	F32	frequency;

	frequency = mSpawnFrequency;
	// **** Hack!  remainingLifetime counts up from negative, to avoid subtracts! - djs
	mParticleState[particleNumber].remainingLifetime = -(((mSpawnFrequencyRange * 2.0f)*frand(1.f)) + frequency - mSpawnFrequencyRange);
}

//Can override later
void LLVOPart::spawnParticle(const U32 particleNumber)
{
	F32	randomUnitValue;
	LLVector3	direction;

	if (particleNumber >= mNumPart)
	{
		llinfos << "Trying to spawn particle beyond initialized particles! " << particleNumber << " : " << mNumPart << llendl;
		return;
	}

	mDeadArr[particleNumber] = 0; // not dead yet!

	//j = pos_offset_i(particleNumber);
	mParticleState[particleNumber].position[0] = 2.f*mSpawnRange*(frand(1.f)) - mSpawnRange;
	mParticleState[particleNumber].position[1] = 2.f*mSpawnRange*(frand(1.f)) - mSpawnRange;
	mParticleState[particleNumber].position[2] = 2.f*mSpawnRange*(frand(1.f)) - mSpawnRange;
	
	//mParticleStateArray[j] = (pos_ranges[1] - pos_ranges[0])*(F32)rand()/((F32)RAND_MAX) + pos_ranges[0];
	//j++;
	
	//mParticleStateArray[j] = (pos_ranges[3] - pos_ranges[2])*(F32)rand()/((F32)RAND_MAX) + pos_ranges[2];
	//j++;
	
	//mParticleStateArray[j] = (pos_ranges[5] - pos_ranges[4])*(F32)rand()/((F32)RAND_MAX) + pos_ranges[4];

	//Create the ranged direction vector first - then rotate by the actual direction and then scale
	//Creating a random value about 1,0,0
	//1. pick a random angle YZ orientation through full circle.
	randomUnitValue = (frand(1.f));
	direction.mV[VY] = sinf(randomUnitValue * 2.0 * F_PI);
	direction.mV[VZ] = cosf(randomUnitValue * 2.0 * F_PI);

	//2. pick a rotation to this angle to project into z which is scaled by mSpawnDirectionRange
	randomUnitValue = (frand(1.f));
	randomUnitValue *= mSpawnDirectionRange;
	direction.mV[VY] = direction.mV[VY] * sinf(randomUnitValue * F_PI);
	direction.mV[VZ] = direction.mV[VZ] * sinf(randomUnitValue * F_PI);
	direction.mV[VX] = cosf(randomUnitValue * F_PI);	//works as still dealing with a unit vector

	//3.To rotate into the spawn direction coord system, derive a yaw and pitch (roll doesnt matter)
	//from the offset between the unit vector 1,0,0 and random direction.
	{F32	length;

		//TODO - math behind this may be incorrect
		//Assume the initial axis is +ve x
		//derive pitch using the XZ or XY components.
		//derive yaw using YZ components.
		length = (mSpawnDirection.mV[VY]*mSpawnDirection.mV[VY]) +
							(mSpawnDirection.mV[VZ]*mSpawnDirection.mV[VZ]);
		if (length > 0.0f)
		{//Only happens when spawn a particle, so can afford some heavy math.
			F32 xYaw, yYaw, xPitch, yPitch;
			LLVector3	tempResult, tempResult2;

			//Pitch is the XZ component (but if Z is 0 and Y is not, switch)
			tempResult.setVec(mSpawnDirection.mV[VX], mSpawnDirection.mV[VY], mSpawnDirection.mV[VZ]);
			if (0.0f != mSpawnDirection.mV[VZ])
			{
				length = sqrtf((mSpawnDirection.mV[VX]*mSpawnDirection.mV[VX]) +
								(mSpawnDirection.mV[VZ]*mSpawnDirection.mV[VZ]));
				if (length > 0.0f)
				{
					xPitch = tempResult.mV[VX] / length;
					yPitch = -tempResult.mV[VZ] / length;
				}
				else
				{//length is 0, so no x component, so pitch must be PI/2
					xPitch = 0.0f;
					yPitch = 1.0f;
				}

				//To obtain yaw, remove pitch from the direction vector by inverse rotation (negate the yPitch)
				tempResult2.mV[VX] = (tempResult.mV[VX] * xPitch) + (tempResult.mV[VZ] * (-yPitch));
				tempResult2.mV[VY] = tempResult.mV[VY];
				tempResult2.mV[VZ] = (tempResult.mV[VZ] * xPitch) - (tempResult.mV[VX] * (-yPitch));
			}
			else
			{//Need XY, because if XZ is zero there is no pitch, and yaw may not pick up the discrepancy.
			//This also avoids roll, so less math.
				length = sqrtf((mSpawnDirection.mV[VX]*mSpawnDirection.mV[VX]) +
								(mSpawnDirection.mV[VY]*mSpawnDirection.mV[VY]));
				if (length > 0.0f)
				{
					xPitch = tempResult.mV[VX] / length;
					yPitch = -tempResult.mV[VY] / length;
				}
				else
				{//length is 0, so no x component, so pitch must be PI/2
					xPitch = 0.0f;
					yPitch = 1.0f;
				}

				//To obtain yaw, remove pitch from the direction vector by inverse rotation (negate the yPitch)
				tempResult2.mV[VX] = (tempResult.mV[VX] * xPitch) + (tempResult.mV[VY] * (-yPitch));
				tempResult2.mV[VY] = (tempResult.mV[VY] * xPitch) - (tempResult.mV[VX] * (-yPitch));
				tempResult2.mV[VZ] = tempResult.mV[VZ];
			}

			//Yaw is the YZ component
			length = sqrtf((tempResult2.mV[VZ]*tempResult2.mV[VZ]) +
							(tempResult2.mV[VY]*tempResult2.mV[VY]));
			if (length > 0.0f)
			{
				xYaw = tempResult2.mV[VZ] / length;
				yYaw = tempResult2.mV[VY] / length;
			}
			else
			{
				xYaw = 1.0f;
				yYaw = 0.0f;
			}

			//Now apply the rotations to the actual data in the same order as derived above (pitch first)
			tempResult.setVec(direction.mV[VX], direction.mV[VY], direction.mV[VZ]);
			//Remember which axis pitch was on, as need to apply in the same manner here for consistency.
			if (0.0f != mSpawnDirection.mV[VZ])
			{
				tempResult2.mV[VX] = (tempResult.mV[VX] * xPitch) + (tempResult.mV[VZ] * yPitch);
				tempResult2.mV[VY] = tempResult.mV[VY];
				tempResult2.mV[VZ] = (tempResult.mV[VZ] * xPitch) - (tempResult.mV[VX] * yPitch);
			}
			else
			{
				tempResult2.mV[VX] = (tempResult.mV[VX] * xPitch) + (tempResult.mV[VY] * yPitch);
				tempResult2.mV[VY] = (tempResult.mV[VY] * xPitch) - (tempResult.mV[VX] * yPitch);
				tempResult2.mV[VZ] = tempResult.mV[VZ];
			}
			direction.mV[VX] = tempResult2.mV[VX];
			direction.mV[VY] = (tempResult2.mV[VY] * xYaw) + (tempResult2.mV[VZ] * yYaw);
			direction.mV[VZ] = (tempResult2.mV[VZ] * xYaw) - (tempResult2.mV[VY] * yYaw);
		}
		else
		{//The is no YZ component, so we a pointing straight along X axis (default) & therefore no rotation
			//However, direction may be reversed
			if (mSpawnDirection.mV[VX] < 0.0f)
			{
				direction.mV[VX] = -direction.mV[VX];
				direction.mV[VZ] = -direction.mV[VZ];
			}

		}
	}


	//4. scale the vector by a random scale by mSpawnVelocityRange and offset by mSpawnVelocity
	randomUnitValue = (frand(1.f));
	randomUnitValue = (randomUnitValue * mSpawnVelocityRange) + mSpawnVelocity;

	mParticleState[particleNumber].velocity[0] = direction.mV[VX] * randomUnitValue;
	mParticleState[particleNumber].velocity[1] = direction.mV[VY] * randomUnitValue;
	mParticleState[particleNumber].velocity[2] = direction.mV[VZ] * randomUnitValue;
	
	//add in velocity offset to match what spawned these particles
	mParticleState[particleNumber].velocity[0] += mVelocityOffset.mV[VX];
	mParticleState[particleNumber].velocity[1] += mVelocityOffset.mV[VY];
	mParticleState[particleNumber].velocity[2] += mVelocityOffset.mV[VZ];
	
	mParticleState[particleNumber].acceleration[0] = 0.0f; 
	
	mParticleState[particleNumber].acceleration[1] = 0.0f; 
	
	mParticleState[particleNumber].acceleration[2] = 0.0f;

	mParticleState[particleNumber].scale[0] = (mScale_range[1] - mScale_range[0])*frand(1.f) + mScale_range[0];
	mParticleState[particleNumber].scale[1] = (mScale_range[3] - mScale_range[2])*frand(1.f) + mScale_range[2];
	mParticleState[particleNumber].scale[2] = 0.0f;

	mParticleState[particleNumber].alpha[0] = (mAlpha_range[1] - mAlpha_range[0])*frand(1.f) + mAlpha_range[0];
	mParticleState[particleNumber].alpha[1] = (mAlpha_range[3] - mAlpha_range[2])*frand(1.f) + mAlpha_range[2];
	mParticleState[particleNumber].alpha[2] = 0.0f;

	// **** Hack!  remainingLifetime counts up from negative, to avoid subtracts! - djs
	mParticleState[particleNumber].remainingLifetime = -((mIndividualLifetimeRange*2.0f)*frand(1.f) + mIndividualLifetime - mIndividualLifetimeRange);

	//rest of the state - 0 for now
	mParticleState[particleNumber].deathOffset = 0.0f; 
	mParticleState[particleNumber].localWind[0] = 0.0f; 
	mParticleState[particleNumber].localWind[1] = 0.0f; 
	mParticleState[particleNumber].localWind[2] = 0.0f; 
}

//Can override later
void LLVOPart::onParticleBounce(const U32 particleNumber)
{
	mParticleState[particleNumber].velocity[0] +=  - (1.0f + mBounceBehavior) * mBouncePlaneNormal.mV[0] * mParticleState[particleNumber].velocity[0];
	mParticleState[particleNumber].velocity[1] +=  - (1.0f + mBounceBehavior) * mBouncePlaneNormal.mV[1] * mParticleState[particleNumber].velocity[1];
	mParticleState[particleNumber].velocity[2] +=  - (1.0f + mBounceBehavior) * mBouncePlaneNormal.mV[2] * mParticleState[particleNumber].velocity[2];


	//Need to offset particle so above the plane, so it doesn't oscillate!
	if (mParticleState[particleNumber].position[2] < mBouncePlaneZ)
	{
		mParticleState[particleNumber].position[2] = mBouncePlaneZ;
	}
}


unsigned char LLVOPart::initializeParticlesAndConstraints(U32 initialParticles,
																F32 diffEqAlpha[3],
																F32 diffEqScale[3],
																F32 scale_ranges[4],
																F32 alpha_ranges[4],
																F32 velocityOffset[3],
																F32	killPlaneZ,
																F32	killPlaneNormal[3],
																F32	bouncePlaneZ,
																F32	bouncePlaneNormal[3],
																F32	spawnRange,
																F32	spawnFrequency,
																F32	spawnFrequencyRange,
																F32	spawnDirection[3],
																F32	spawnDirectionRange,
																F32	spawnVelocity,
																F32	spawnVelocityRange,
																F32	speedLimit,
																F32	windWeight,
																F32	currentGravity[3],
																F32	gravityWeight,
																F32	globalLifetime,
																F32	individualLifetime,
																F32	individualLifetimeRange,
																F32	alphaDecay,
																F32	scaleDecay,
																F32	distanceDeath,
																F32	dampMotionFactor,
																F32 windDiffusionFactor[3])
{
	// initializes particles randomly within these ranges
	// scale ranges and alpha ranges contain initial conditions plus rates of change
	// of initial conditions  My naming is incosistent (passing alpha derivatives with alpha 
	// values), but this class is wrapped by another one, and this function is called only once
	U32 i;


	for (i = 0; i < 3; i++)
	{
		mDiffEqAlpha[i] = diffEqAlpha[i];
		mDiffEqScale[i] = diffEqScale[i];
	}

	//First - store the initial conditions
	for (i = 0; i < 4; i++)
	{
		mScale_range[i] = scale_ranges[i];
		mAlpha_range[i] = alpha_ranges[i];
	}

	mVelocityOffset.setVec(velocityOffset[0], velocityOffset[1], velocityOffset[2]);

	mKillPlaneZ = killPlaneZ;
	mKillPlaneNormal.setVec(killPlaneNormal[0], killPlaneNormal[1], killPlaneNormal[2]);
	mKillPlaneNormal.normVec();
	mBouncePlaneZ = bouncePlaneZ;
	mBouncePlaneNormal.setVec(bouncePlaneNormal[0], bouncePlaneNormal[1], bouncePlaneNormal[2]);
	mBouncePlaneNormal.normVec();

	mSpawnRange = spawnRange;

	mSpawnFrequency = spawnFrequency;
	mSpawnFrequencyRange = spawnFrequencyRange;

	mSpawnDirection.setVec(spawnDirection[0], spawnDirection[1], spawnDirection[2]);
	mSpawnDirection.normVec();
	mSpawnDirectionRange = spawnDirectionRange;

	mSpawnVelocity = spawnVelocity;
	mSpawnVelocityRange = spawnVelocityRange;
	mSpeedLimitSquared = speedLimit * speedLimit;
	if (mSpeedLimitSquared < 0.0000001f)
	{
		mSpeedLimitSquared = 0.0000001f;	//speed must be finite +ve to avoid divide by zero
	}

	mWindWeight = windWeight;
	mCurrentGravity.setVec(currentGravity[0], currentGravity[1], currentGravity[2]);
	mGravityWeight = gravityWeight;

	mGlobalLifetime = globalLifetime;
	mOriginalGlobalLifetime = mGlobalLifetime;
	mIndividualLifetime = individualLifetime;
	if (mIndividualLifetime > 0.0f)
	{
		mOneOverIndividualLifetime = 1.0f / mIndividualLifetime;
	}
	else
	{
		mOneOverIndividualLifetime = 0.0f;
	}
	mIndividualLifetimeRange = individualLifetimeRange;

	mAlphaDecay = alphaDecay;
	mScaleDecay = scaleDecay;
	mDistanceDeathSquared = distanceDeath * distanceDeath;
	if (mDistanceDeathSquared > 0.0f)
	{
		mOneOverDistanceDeathSquared = 1.0f / mDistanceDeathSquared;
	}
	else
	{
		mOneOverDistanceDeathSquared = 0.0f;
	}
	mDampMotionFactor = dampMotionFactor;

	mWindDiffusionFactor.setVec(windDiffusionFactor[0], windDiffusionFactor[1], windDiffusionFactor[2]);
	//Scale the values down a lot, as can expand sprites incredibly fast
	mWindDiffusionFactor *= 0.02f;
	if (mWindDiffusionFactor.mV[VX] > 0.02f)
	{
		mWindDiffusionFactor.mV[VX] = 0.02f;
	}
	if (mWindDiffusionFactor.mV[VY] > 0.02f)
	{
		mWindDiffusionFactor.mV[VY] = 0.02f;
	}
	if (mWindDiffusionFactor.mV[VZ] > 0.02f)
	{
		mWindDiffusionFactor.mV[VZ] = 0.02f;
	}
	//llinfos << "Made a particle system" << llendl;

	for(i = 0; i < initialParticles; i++)
	{
		spawnParticle(i);
	}

	for(i = initialParticles; i < mNumPart; i++)
	{
		//create initial conditions - dead waiting to live timer
		setParticleCountdownStateWaitingDead(i);
	}
	return '\0'; // success 
}

U8  LLVOPart::iterateParticles(F32 deltaT) 
{
	const F32 PART_SYS_UPDATE_PHYSICS_INPUTS_TIME = 0.2f;	//How many seconds between querying wind force on a particle

	U32 i;	//, pos_off, vel_off, scale_off, alpha_off, lifetime_off, death_off, local_wind_off;
	F32	weightedDeathSum;
	U8 any_leftQ = 0;

	F32 windWeightDT = mWindWeight*deltaT;
	F32 gravityWeightDT = mGravityWeight*deltaT;

	mLastTime = mCurrTime;
	mCurrTime += deltaT; 

	if ((mCurrTime - mUpdatePhysicsInputsTime) > PART_SYS_UPDATE_PHYSICS_INPUTS_TIME)
	{
		//  If needed, obtain latest wind for the whole system
		mCurrentWind = mRegionp->mWind.getVelocity(getPositionRegion());
		mCurrentWindMagnitude = sqrt((mCurrentWind.mV[VX] * mCurrentWind.mV[VX]) +
			(mCurrentWind.mV[VY] * mCurrentWind.mV[VY]) +
			(mCurrentWind.mV[VZ] * mCurrentWind.mV[VZ]));
		//don't know a max, so just make up a reasonably large value for now and cap.
		mCurrentWindMagnitude = mCurrentWindMagnitude * 0.05f;
		if (mCurrentWindMagnitude > 1.0f)
		{
			mCurrentWindMagnitude = 1.0f;
		}
		mCurrentWindMagnitudeSquareRoot = sqrtf(mCurrentWindMagnitude);
	}

	//TO DO - 	LOD algorithm (as with display??)
	for(i = 0; i < mNumPart; i++) 
	{
		//lifetime_off = remaining_lifetime_offset_i(i);
		if(0 != mDeadArr[i])
		{
			//test for spawns amongst the dead as follows
			if (mFlags[PART_SYS_ACTION_BYTE] & PART_SYS_SPAWN)
			{
				// **** Hack!  remainingLifetime counts up from negative, to avoid subtracts! - djs
				//if die below and spawn flag set, set mParticleState[i].remainingLifetime to a respawn random time
				//based on mSpawnFrequency and mSpawnFrequencyRange
				//in this section, if spawn flag set, count down - deltaT. If time is negative, call Spawn method, else continue
				mParticleState[i].remainingLifetime += deltaT;
				if (mParticleState[i].remainingLifetime > 0.0f)
				{
					spawnParticle(i);
				}
			}

			if(0 != mDeadArr[i])
			{
				continue; // stop animating dead particles -- else they might come back alive!
					// since we're not alive, do nothing to "any_leftQ"
			}
		}


		//New way - apply external forces to each particle and then update
		// position at .... 
		mParticleState[i].position[0] += mParticleState[i].velocity[0]*deltaT;		// x position
		mParticleState[i].position[1] += mParticleState[i].velocity[1]*deltaT;	// y position
		mParticleState[i].position[2] += mParticleState[i].velocity[2]*deltaT;	// z position

		//then apply force if required
		if (mFlags[PART_SYS_ACTION_BYTE] & PART_SYS_AFFECTED_BY_WIND)
		{
			mParticleState[i].velocity[0] += mCurrentWind.mV[0]*windWeightDT;
			mParticleState[i].velocity[1] += mCurrentWind.mV[1]*windWeightDT;
			mParticleState[i].velocity[2] += mCurrentWind.mV[2]*windWeightDT;
		}
		if (mFlags[PART_SYS_ACTION_BYTE] & PART_SYS_AFFECTED_BY_GRAVITY)
		{
			mParticleState[i].velocity[0] += mCurrentGravity.mV[0]*gravityWeightDT;
			mParticleState[i].velocity[1] += mCurrentGravity.mV[1]*gravityWeightDT;
			mParticleState[i].velocity[2] += mCurrentGravity.mV[2]*gravityWeightDT;
		}
		if (mFlags[PART_SYS_ACTION_BYTE] & PART_SYS_EVALUATE_WIND_PER_PARTICLE)
		{
			if ((mCurrTime - mUpdatePhysicsInputsTime) > PART_SYS_UPDATE_PHYSICS_INPUTS_TIME)
			{
				LLVector3 wind, current_position_region;
				//  If needed, obtain latest wind per particle
				//Particle positions are relative to the object center.
				current_position_region.setVec(getPositionRegion());
				current_position_region.mV[VX] += (mParticleState[i].position[0]);
				current_position_region.mV[VY] += (mParticleState[i].position[1]);
				current_position_region.mV[VZ] += (mParticleState[i].position[2]);

				wind = mRegionp->mWind.getVelocity(current_position_region);
				mParticleState[i].localWind[0] = wind.mV[0];
				mParticleState[i].localWind[1] = wind.mV[1];
				mParticleState[i].localWind[2] = wind.mV[2];
			}
			mParticleState[i].velocity[0] += mParticleState[i].localWind[0]*windWeightDT;
			mParticleState[i].velocity[1] += mParticleState[i].localWind[1]*windWeightDT;
			mParticleState[i].velocity[2] += mParticleState[i].localWind[2]*windWeightDT;
		}

		//then apply drag if required
		if (mFlags[PART_SYS_ACTION_BYTE] & PART_SYS_DAMP_MOTION)
		{
			F32	dampAmount;

			dampAmount = (mParticleState[i].velocity[0] * mParticleState[i].velocity[0]) +
					(mParticleState[i].velocity[1] * mParticleState[i].velocity[1]) +
					(mParticleState[i].velocity[2] * mParticleState[i].velocity[2]);
			dampAmount = 1.0f - ((mSpeedLimitSquared - dampAmount) / mSpeedLimitSquared);
			if (dampAmount < 0.0f)
			{
				dampAmount = 0.0f;
			}
			else if (dampAmount > 1.0f)
			{
				dampAmount = 1.0f;
			}
			
			//Damp prop to deltaT
			dampAmount = -1.f * dampAmount * mDampMotionFactor * deltaT;

			mParticleState[i].velocity[0] += dampAmount * mParticleState[i].velocity[0];
			mParticleState[i].velocity[1] += dampAmount * mParticleState[i].velocity[1];
			mParticleState[i].velocity[2] += dampAmount * mParticleState[i].velocity[2];
		}

		//check for bounces if required
		if (mFlags[PART_SYS_ACTION_BYTE] & PART_SYS_BOUNCE)
		{//Is the particle below the bounce plane??
			LLVector3	tempBounceTest;
			tempBounceTest.setVec(mParticleState[i].position[0], mParticleState[i].position[1], mParticleState[i].position[2]);
			tempBounceTest.mV[VZ] -= mBouncePlaneZ;

			//This is simplistic for now
			if ((tempBounceTest.mV[VZ] < 0.0f) && (mParticleState[i].velocity[2] < 0.0f))
			{
				onParticleBounce(i);
			}
		}

		//maintain the old way of updating scale and alpha
		// clamp scale and alpha to reasonable values
		// should this section of code only be called if we're *actually* animating scale and alpha?
		mParticleState[i].alpha[2] = mDiffEqAlpha[0] +   // constant term
			mDiffEqAlpha[1]*mParticleState[i].alpha[0] +  // zeroth derivative term
			mDiffEqAlpha[2]*mParticleState[i].alpha[1]; // first derivative term

		mParticleState[i].scale[2] = mDiffEqScale[0] +  // constant term
			mDiffEqScale[1]*mParticleState[i].scale[0] +  // zeroth derivative term
			mDiffEqScale[2]*mParticleState[i].scale[1]; // first derivative term

		mParticleState[i].scale[0] += mParticleState[i].scale[1]*deltaT;	// scale
		
		mParticleState[i].alpha[0] += mParticleState[i].alpha[1]*deltaT;	// alpha
		if (mParticleState[i].scale[0] < 0.0f)
		{
			mParticleState[i].scale[0] = 0.01f;
		}

		if (mParticleState[i].alpha[0] < 0.0f)
		{
			mParticleState[i].alpha[0] = 0.0f;
		}
		else if (mParticleState[i].alpha[0] > 1.0f)
		{
			mParticleState[i].alpha[0] = 1.0f;
		}
		mParticleState[i].scale[1] += mParticleState[i].scale[2]*deltaT;
		mParticleState[i].alpha[1] += mParticleState[i].alpha[2]*deltaT;

		//Is particle appearance affected by wind?
		if (mFlags[PART_SYS_ACTION_BYTE] & PART_SYS_WIND_DIFFUSION)
		{
			F32 windMagnitude, windMagnitudeSqrt;

			if (mFlags[PART_SYS_ACTION_BYTE] & PART_SYS_EVALUATE_WIND_PER_PARTICLE)
			{
				windMagnitude = sqrt((mParticleState[i].localWind[0] * mParticleState[i].localWind[0]) +
					(mParticleState[i].localWind[1] * mParticleState[i].localWind[1]) +
					(mParticleState[i].localWind[2] * mParticleState[i].localWind[2]));
				//don't know a max, so just make up a reasonably large value for now and cap.
				windMagnitude = windMagnitude * 0.05f;
				if (windMagnitude > 1.0f)
				{
					windMagnitude = 1.0f;
				}
				windMagnitudeSqrt = sqrt(mCurrentWindMagnitude);
			}
			else
			{
				windMagnitude = mCurrentWindMagnitude;
				windMagnitudeSqrt = mCurrentWindMagnitudeSquareRoot;
			}

			//ignore alpha for now, as it is reduced by distance death and life expectancy in any case.
			//mParticleState[i].alpha[0] *= (1.0 - (windMagnitude * mWindDiffusionFactor.mV[VX]));
			//Scaling prop to sqr root. Should really be cube root, as we are dealing with a volume, but
			//heavy math, and sqr looks OK.
			mParticleState[i].scale[0] *= (1.f + (windMagnitudeSqrt * mWindDiffusionFactor.mV[VX]));			
		}

		// we have to store the death of the particle somewhere

		//now test for death if life expires or hits edge of death radius (given flags enabled)
		//aapply alpha and scale fade proportional to proximity to death - test for death flags and produce a weighted sum
		//taking into account time to live and distance to death.
		weightedDeathSum = 1.0f;
		
		if (mFlags[PART_SYS_KILL_BYTE] & PART_SYS_TIME_DEATH)
		{
			// **** Hack!  remainingLifetime counts up from negative, to avoid subtracts! - djs
			mParticleState[i].remainingLifetime += deltaT;
			weightedDeathSum *= -1.f * (mParticleState[i].remainingLifetime * mOneOverIndividualLifetime);
			if (mParticleState[i].remainingLifetime > 0.0f)
			{
				mDeadArr[i] = 1;
				if (mFlags[PART_SYS_ACTION_BYTE] & PART_SYS_SPAWN)
				{
					setParticleCountdownStateWaitingDead(i);
				}
				weightedDeathSum = 0.0f;
			}
			else if (weightedDeathSum > 1.0f)
			{
				weightedDeathSum = 1.0f;	
			}
		}
		if (mFlags[PART_SYS_KILL_BYTE] & PART_SYS_DISTANCE_DEATH)
		{
			F32	radius;
			radius = (mParticleState[i].position[0]*mParticleState[i].position[0] + 
				mParticleState[i].position[1]*mParticleState[i].position[1] + 
				mParticleState[i].position[2]*mParticleState[i].position[2]);
			radius = (mDistanceDeathSquared - radius) * mOneOverDistanceDeathSquared;
			weightedDeathSum = weightedDeathSum * radius;
			if (radius <= 0.0f)
			{
				mDeadArr[i] = 1;
				if (mFlags[PART_SYS_ACTION_BYTE] & PART_SYS_SPAWN)
				{
					setParticleCountdownStateWaitingDead(i);
				}
				weightedDeathSum = 0.0f;
			}
			else if (weightedDeathSum > 1.0f)
			{
				weightedDeathSum = 1.0f;	
			}
		}
		if (mFlags[PART_SYS_KILL_BYTE] & PART_SYS_KILL_PLANE)
		{
			if (mParticleState[i].position[2] <= mKillPlaneZ)
			{
				llinfos << "kill plane" << llendl;
				mDeadArr[i] = 1;
				if (mFlags[PART_SYS_ACTION_BYTE] & PART_SYS_SPAWN)
				{
					setParticleCountdownStateWaitingDead(i);
				}
				weightedDeathSum = 0.0f;
			}
		}
		if (mFlags[PART_SYS_KILL_BYTE] & PART_SYS_GLOBAL_DIE)
		{
			weightedDeathSum = weightedDeathSum * (mGlobalLifetime / mOriginalGlobalLifetime);
		}


		mParticleState[i].deathOffset = weightedDeathSum;

		any_leftQ |= (1 - mDeadArr[i]);
	}

	if (mFlags[PART_SYS_ACTION_BYTE] & PART_SYS_SPAWN)
	{//May be just waiting for new particles to be born, with none around currently
		any_leftQ = TRUE;
	}


	if (mFlags[PART_SYS_KILL_BYTE] & PART_SYS_GLOBAL_DIE)
	{
		mGlobalLifetime -= deltaT;
		if (mGlobalLifetime < 0.0f)
		{
			any_leftQ = FALSE;
		}
	}


	if ((mCurrTime - mUpdatePhysicsInputsTime) > PART_SYS_UPDATE_PHYSICS_INPUTS_TIME)
	{//Have to execute this at end of itterate, as several independent processes rely on the test being true.
		mUpdatePhysicsInputsTime = mCurrTime;
	}

	mNumLiveParticles = 0;
	for ( i = 0; i < mNumPart; i++)
	{
		if (!mDeadArr[i])
		{
			mNumLiveParticles++;
		}
	}
	return any_leftQ;
}

void LLVOPart::reverseTranslateParticlesAndPotentiallyKill(const LLVector3 &moveBy) 
{
	U32	i;//pos_off;

	for(i = 0; i < mNumPart; i++) 
	{
		if(0 == mDeadArr[i])
		{
			//pos_off = pos_offset_i(i);
			mParticleState[i].position[0] += moveBy.mV[VX];
			mParticleState[i].position[1] += moveBy.mV[VY];
			mParticleState[i].position[2] += moveBy.mV[VZ];
			
		}
	}
}

void LLVOPart::translateParticlesBy(const LLVector3 &moveBy) 
{
	if (!(mFlags[PART_SYS_ACTION_BYTE] & PART_SYS_BOUNCE))
	{//Do not translate if bouncing is on, as the whole system shifts adding or subtractiong potential energy

		//If the object moves, the particles should not, otherwise will look like a rigid structure
		reverseTranslateParticlesAndPotentiallyKill(moveBy);

		mSpawnPoint.mV[VX] += moveBy.mV[VX];
		mSpawnPoint.mV[VY] += moveBy.mV[VY];
		mSpawnPoint.mV[VZ] += moveBy.mV[VZ];
	}
}


void LLVOPart::translateParticlesTo(const LLVector3 &moveTo) 
{
	LLVector3 moveBy;

	if (!(mFlags[PART_SYS_ACTION_BYTE] & PART_SYS_BOUNCE))
	{//Do not translate if bouncing is on, as the whole system shifts adding or subtractiong potential energy

		//If the object moves, the particles should not, otherwise will look like a rigid structure
		moveBy.setVec(moveTo.mV[VX] - mSpawnPoint.mV[VX], moveTo.mV[VY] - mSpawnPoint.mV[VY], moveTo.mV[VZ] - mSpawnPoint.mV[VZ]);
		reverseTranslateParticlesAndPotentiallyKill(moveBy);

		mSpawnPoint.mV[VX] = moveTo.mV[VX];
		mSpawnPoint.mV[VY] = moveTo.mV[VY];
		mSpawnPoint.mV[VZ] = moveTo.mV[VZ];
	}
}


void LLVOPart::rotateParticlesBy(const LLQuaternion &q) 
{
	mOriginOrientation *= q;
}


void LLVOPart::rotateParticlesTo(const LLQuaternion &q) 
{
	mOriginOrientation.setQuatInit(q.mQ[VX], q.mQ[VY], q.mQ[VZ], q.mQ[VW]);
}
