/** 
 * @file llvostars.cpp
 * @brief LLVOStars class implementation
 *
 * Copyright (c) 2001-2007, Linden Research, Inc.
 * 
 * Second Life Viewer Source Code
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

#include "llvostars.h"

#include "lldrawpoolstars.h"
#include "llface.h"
#include "llsky.h"
#include "llvosky.h"
#include "llworld.h"
#include "pipeline.h"

const U32 NUMBER_OF_STARS		= 1000;
const F32 DISTANCE_TO_STARS		= (HORIZON_DIST - 10.f)*0.25f;


LLVOStars::LLVOStars(const LLUUID &id, const LLPCode pcode, LLViewerRegion *regionp)
	: LLStaticViewerObject(id, pcode, regionp)
{
	initStars();
}

LLVOStars::~LLVOStars()
{
	delete[] mStarVertices;
	mStarVertices = NULL;
	delete[] mStarColors;
	mStarColors = NULL;
	delete[] mStarIntensities;
	mStarIntensities = NULL;
}

LLDrawable *LLVOStars::createDrawable(LLPipeline *pipeline)
{
	pipeline->allocDrawable(this);
	mDrawable->setLit(FALSE);

	LLDrawPoolStars *poolp = (LLDrawPoolStars*) gPipeline.getPool(LLDrawPool::POOL_STARS);

	mFace = mDrawable->addFace(poolp, NULL);
	mDrawable->setRenderType(LLPipeline::RENDER_TYPE_STARS);
	mFace->setSize(NUMBER_OF_STARS, NUMBER_OF_STARS);

	return mDrawable;
}

BOOL LLVOStars::idleUpdate(LLAgent &agent, LLWorld &world, const F64 &time)
{
	return TRUE;
}

BOOL LLVOStars::updateGeometry(LLDrawable *drawable)
{
	updateStarColors();
	updateStarGeometry(drawable);

	LLPipeline::sCompiles++;
	return TRUE;
}

BOOL LLVOStars::updateStarGeometry(LLDrawable *drawable)
{
	LLStrider<LLVector3> verticesp;
	LLStrider<LLVector3> normalsp;
	LLStrider<LLVector2> texCoordsp;
	LLStrider<LLColor4U> colorsp;
	LLStrider<U32> indicesp;
	S32 index_offset;

	if (mFace->mVertexBuffer.isNull())
	{
		mFace->mVertexBuffer = new LLVertexBuffer(LLDrawPoolStars::VERTEX_DATA_MASK, GL_STREAM_DRAW_ARB);
		mFace->mVertexBuffer->allocateBuffer(mFace->getGeomCount(), mFace->getIndicesCount(), TRUE);
		mFace->setGeomIndex(0);
		mFace->setIndicesIndex(0);
	}
	
	index_offset = mFace->getGeometryColors(verticesp,normalsp,texCoordsp,colorsp, indicesp);
	
	if (-1 == index_offset)
	{
		return TRUE;
	}
	
	for (U32 vtx = 0; vtx < NUMBER_OF_STARS; ++vtx)
	{
		*(verticesp++)  = mStarVertices[vtx];
		*(colorsp++)    = LLColor4U(mStarColors[vtx]);
		*(indicesp++)   = index_offset + vtx;
	}

	return TRUE;
}


void LLVOStars::initStars()
{
	// Initialize star map
	mStarVertices = new LLVector3 [NUMBER_OF_STARS];
	mStarColors = new LLColor4 [NUMBER_OF_STARS];
	mStarIntensities = new F32 [NUMBER_OF_STARS];

	LLVector3 * v_p = mStarVertices;
	LLColor4 * v_c = mStarColors;
	F32  * v_i = mStarIntensities;

	U32 i;
	for (i = 0; i < NUMBER_OF_STARS; i++ )
	{
		v_p->mV[VX] = ll_frand() - 0.5f;
		v_p->mV[VY] = ll_frand() - 0.5f;
		
		// we only want stars on the top half of the dome!

		v_p->mV[VZ] = ll_frand()/2.f;

		v_p->normVec();
		*v_p *= DISTANCE_TO_STARS;
		*v_i = llmin((F32)pow(ll_frand(),2.f) + 0.1f, 1.f);
		v_c->mV[VRED]   = 0.75f + ll_frand() * 0.25f ;
		v_c->mV[VGREEN] = 1.f ;
		v_c->mV[VBLUE]  = 0.75f + ll_frand() * 0.25f ;
		v_c->mV[VALPHA] = 1.f;
		v_c->clamp();
		v_p++;
		v_c++;
		v_i++;
	}
}

void LLVOStars::updateStarColors()
{
	LLColor4 * v_c;
	v_c = mStarColors;
	F32 * v_i = mStarIntensities;
	LLVector3* v_p = mStarVertices;

	const F32 var = 0.15f;
	const F32 min = 0.5f; //0.75f;
	const F32 sunclose_max = 0.6f;
	const F32 sunclose_range = 1 - sunclose_max;

	F32 below_horizon = - llmin(0.0f, gSky.mVOSkyp->getToSunLast().mV[2]);
	F32 brightness_factor = llmin(1.0f, below_horizon * 20);

	static S32 swap = 0;
	swap++;

	if ((swap % 2) == 1)
	{
		F32 intensity;						//  max intensity of each star
		U32 x;
		for (x = 0; x < NUMBER_OF_STARS; x++)
		{
			F32 sundir_factor = 1;
			LLVector3 tostar = *v_p;
			tostar.normVec();
			const F32 how_close_to_sun = tostar * gSky.mVOSkyp->getToSunLast();
			if (how_close_to_sun > sunclose_max)
			{
				sundir_factor = (1 - how_close_to_sun) / sunclose_range;
			}
			intensity = *(v_i);
			F32 alpha = v_c->mV[VALPHA] + (ll_frand() - 0.5f) * var * intensity;
			if (alpha < min * intensity)
			{
				alpha = min * intensity;
			}
			if (alpha > intensity)
			{
				alpha = intensity;
			}
			alpha *= brightness_factor * sundir_factor;
			alpha = llclamp(alpha, 0.f, 1.f);
			v_c->mV[VALPHA] = alpha;
			v_c++;
			v_i++;
			v_p++;
		}
	}
}
