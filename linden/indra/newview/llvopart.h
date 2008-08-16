/** 
 * @file llvopart.h
 * @brief Description of LLVOPart class
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

#ifndef LL_LLVOPART_H
#define LL_LLVOPART_H

#include "llviewerobject.h"
#include "v3math.h"
#include "llframetimer.h"
#include "partsyspacket.h" // for compressing / decompressing particle system data

//28 September 2001 - try and make the internal structs more meaningful than a huge array of floats!
typedef	struct	OneParticleData
{
	F32 position[3];
	F32 velocity[3];
	F32 acceleration[3];
	F32 scale[3];
	F32 alpha[3];
	F32 remainingLifetime;
	F32 deathOffset;
	F32 localWind[3];
}OneParticleData;





/* viewer object for a particle system */
const U32 PART_SYS_BYTES_OF_FLAGS		= 8;

class LLVOPart : public LLViewerObject
{
public:
	LLVOPart(const LLUUID &id, const LLPCode pcode, LLViewerRegion *regionp);

	~LLVOPart();

	// Initialize data that's only inited once per class.
	static void initClass();

	/*virtual*/ BOOL    isActive() const; // Whether this object needs to do an idleUpdate.
	BOOL idleUpdate(LLAgent &agent, LLWorld &world, const F64 &time);


	/*virtual*/ U32 processUpdateMessage(LLMessageSystem *mesgsys,
											void **user_data,
											U32 block_num,
											const EObjectUpdateType update_type,
											LLDataPacker *dp);

	/*virtual*/ void updateTextures(LLAgent &agent);

	/*virtual*/ LLDrawable* createDrawable(LLPipeline *pipeline);
	/*virtual*/ BOOL        updateGeometry(LLDrawable *drawable);

protected:
	void setParticleCountdownStateWaitingDead(U32 particleNumber);

		//Override these methods for new particle classes
	void spawnParticle(const U32 particleNumber);
	void onParticleBounce(const U32 particleNumber);

	void setDefaultValues();

	U8 setParticleParams(F32 bounce_b, 
		const F32 o_pos[3], const F32 o_or[3], U32 n, LLUUID image_uuid, U8 flags[PART_SYS_BYTES_OF_FLAGS]);

	U8 initializeParticlesAndConstraints(U32 initialNumberOfParticles,
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
										F32	spawnFreqencyRange,
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
										F32 windDiffusionFactor[3]);
	// initializes particles randomly within these ranges
	// scale ranges and alpha ranges contain initial conditions plus rates of change
	// of initial conditions  My naming is inconsistent (passing alpha derivatives with alpha 
	// values), but this class is wrapped by another one, and this function is called only once

	U8 iterateParticles(F32 deltaT); // step by one deltaT, check for bounces and particle deaths 

	void reverseTranslateParticlesAndPotentiallyKill(const LLVector3 &moveBy);

	void translateParticlesBy(const LLVector3 &moveBy);
	void translateParticlesTo(const LLVector3 &moveTo);
	void rotateParticlesBy(const LLQuaternion &q);
	void rotateParticlesTo(const LLQuaternion &q);

	///////////////////////
	//
	// Legacy, "dead" methods
	U8 setParticlesDistFadeout(F32 beginFadeout, F32 endFadeout);

protected:
	LLFrameTimer mDeathTimer;
	U32 mNumPart;   // number of particles 
	U32 mNumLiveParticles;
	BOOL mParticlesDead; // am I dead yet? (i.e. should I keep rendering?)
	LLPartInitData mInitSysData; // data for initializing the particle system.

	U64 mTimeLastFrame;
	
	//27 September 2001 - LLParticleSystem no longer a seperate class, as does not allow access to all object facilities
	//the following private data and methods used to be in LLParticleSystem

	F32 mCurrTime; // time value in simulator for particle system -- note: may be independent of simulator time 
	F32 mLastTime; // time last frame

	//26 September - these are replaced by alpha and scale parameters below
	//F32 mDiffEq[18]; // constants for second-order differential equation describing particle motion 
	//                  MDS -- july, doubled the number of constants to handle scale, alpha and rotation
	//26 September - just use mKillPlaneZ, as the rest really confusing to set sometimes
	//F32 mKillParameters[6]; // take a dot-product of a particle's <x,y,z,t, 1, mag^2> values
							// with this and compare to zero to determine if its killed 
							// computational trick, only call it a "kill" when the dot product of
							// the velocity with the first three values of this is below zero 

	//26 September - use mBouncePlaneZ as the parameters are interesting but hard to manipulate predictably
	//F32 mBounceParams[5]; // same as "mKillParameters" only they determine when an object bounces 
	// this is actually for generalized bouncing, might indicate decayed bouncing, or sliding behavior 

	F32 mBounceBehavior; // first three are multiplied pointwise with x,y and z velocities,
							  // next three are added pointwise to x, y and z positions 

	F32 mAlpha;					// so I can fade out the whole system

	LLQuaternion mOriginOrientation; // quaternion for origin orientation 
	
	// geometry / image data for particles visual representation 
	
	//F32 *state_arr; // big array of velocity and position and acceleration 
	OneParticleData *mParticleState; // big array of velocity and position and acceleration 
	U8 *mDeadArr; // mDeadArr[i] == true if particle i is dead

	U8 mFlags[PART_SYS_BYTES_OF_FLAGS];    // just copy the flags from the initialization data


	//New structures added as of 26 September 2001

	//Have to remember the states if are to spawn new particles.
	//These are from the original implementation, but now remembered!
	F32 mDiffEqAlpha[3];
	F32 mDiffEqScale[3];
	F32 mScale_range[4];
	F32 mAlpha_range[4];
	LLVector3	mVelocityOffset;


	//Incorporated in mFlags[PART_SYS_ACTION_BYTE]
		//What actions will this system perform - bit mask of flags so can perform multiple operations.
		//PART_SYS_SPAWN,
		//PART_SYS_BOUNCE,
		//PART_SYS_AFFECTED_BY_WIND,
		//PART_SYS_AFFECTED_BY_GRAVITY,
		//PART_SYS_EVALUATE_WIND_PER_PARTICLE,
		//PART_SYS_APPLY_FORCE
		//PART_SYS_DAMP_MOTION
		//are valid flags.

	//Incorporated in mFlags[PART_SYS_KILL_BYTE]
		//What actions will this system perform - bit mask of flags so can perform multiple operations.
		//PART_SYS_KILL_PLANE,
		//PART_SYS_GLOBAL_DIE,
		//PART_SYS_DISTANCE_DEATH
		//are valid flags.

	F32	mKillPlaneZ;
		//For simplicity assume the XY plane, so this sets an altitude at which to die
	LLVector3	mKillPlaneNormal;
		//Normal direction to the kill plane - UNIT vector
	F32	mBouncePlaneZ;
		//For simplicity assume the XY plane, so this sets an altitude at which to bounce
	LLVector3	mBouncePlaneNormal;
		//Normal direction to the bounce plane - UNIT vector
	LLVector3	mSpawnPoint;
		//If new particles are created, this is where they come from
	F32	mSpawnRange;
		//Range of emission points about the mSpawnPoint
	F32	mSpawnFrequency;
		//Required if the system is to spawn new particles.
		//This variable determines the time after a particle dies when it is respawned.
	F32	mSpawnFrequencyRange;
		//Determines the random range of time until a new particle is spawned.
	LLVector3	mSpawnDirection;
		//Direction vector giving the mean direction in which particles are spawned
	F32	mSpawnDirectionRange;
		//Direction limiting the angular range of emissions about the mean direction. 1.0f means everywhere, 0.0f means uni-directional
	F32	mSpawnVelocity;
		//The mean speed at which particles are emitted
	F32	mSpawnVelocityRange;
		//The range of speeds about the mean at which particles are emitted.
	F32	mSpeedLimitSquared;
		//Used to constrain particle maximum velocity (squared to minimize calcs)
	LLVector3	mCurrentWind;
		//Wind direction used in update calculations
	F32	mCurrentWindMagnitude, mCurrentWindMagnitudeSquareRoot;
		//Velocity of current wind and square root of it to save calculations
	F32	mWindWeight;
		//How much of an effect does wind have
	LLVector3	mCurrentGravity;
		//Gravity direction used in update calculations
	F32	mGravityWeight;
		//How much of an effect does gravity have
	F32	mGlobalLifetime, mOriginalGlobalLifetime;
		//If particles re-spawn, a system can exist forever.
		//If (ActionFlags & PART_SYS_GLOBAL_DIE) is TRUE this variable is used to determine how long the system lasts.
	F32	mIndividualLifetime;
		//How long does each particle last if nothing else happens to it
	F32	mOneOverIndividualLifetime;
		//For fast reciprocals
	F32	mIndividualLifetimeRange;
		//Range of variation in individual lifetimes
	F32	mAlphaDecay;
		//By what factor does alpha decrease as the lifetime of a particle is approached.
	F32	mScaleDecay;
		//By what factor does scale decrease as the lifetime of a particle is approached.
	F32	mDistanceDeathSquared;
		//With the increased functionality, particle systems can expand to indefinite size
		//(e.g. wind can chaotically move particles into a wide spread).
		//To avoid particles exceeding normal object size constraints,
		//set the PART_SYS_DISTANCE_DEATH flag, and set a distance value here, representing a radius around the spawn point.
		//Square to reduce computation.
	F32	mOneOverDistanceDeathSquared;
		//For fast reciprocals
	F32	mDampMotionFactor;
		//How much to damp motion
	LLVector3	mWindDiffusionFactor;
		//How does wind affect the apearance of sprites

	F32	mUpdatePhysicsInputsTime;
		//Internal value to determine whether the time is right to update physical environmental effects
};

#endif // LL_VO_PART_H
