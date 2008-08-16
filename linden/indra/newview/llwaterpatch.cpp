/** 
 * @file llwaterpatch.cpp
 * @brief LLWaterTri class implementation
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

#include "llwaterpatch.h"

#include "llvowater.h"

U32 LLWaterTri::sMinStep;
LL2Coord LLWaterTri::sCam;
F32 LLWaterTri::sClipFar;
U32 LLWaterTri::sMaxDivLevel;
BOOL LLWaterTri::sCurrRound = FALSE;


LLWaterTri::LLWaterTri (U8 level, S8 type, LLWaterTri* par): LLRoamTriNode(level, type, par), mRefine(FALSE)
{
	if (!parent() || type == 0)
	{
		mSize = mLvtx.distance(mRvtx) * sMinStep;
		mCurr = sCurrRound;
		setNotUpToDate();
		return;
	}
	
	mSize = par->size() * OO_SQRT2;
	setPatch(par->patch());
	//		LL2Coord c1 = par->Lvtx();
	//		LL2Coord c2 = par->Rvtx();
	
	//		if (c1.x() - c2.x() == 1 || c1.y() - c2.y() == 1)
	//			bool stophere = true;
	
	if (type < 0)	// left child
	{
		mLvtx = par->Tvtx();
		mRvtx = par->Lvtx();
		//mTvtx = middle(c1, c2);
	} else {
		mRvtx = par->Tvtx();
		mLvtx = par->Rvtx();
		//mTvtx = middle(c1, c2);
	}
	mTvtx = par->middleSide();
	mMiddle = mLvtx.middle(mRvtx);
	if (((LLWaterPatch*)patch())->visible())
		setNotUpToDate();
	else
		setUpToDate();
}


void LLWaterTri::updatePassive()
{
	setUpToDate();
	if (!leaf())
	{
		mLchild->updatePassive();
		mRchild->updatePassive();
	}
}


BOOL LLWaterTri::refine()
{
	if (upToDate())
		return mRefine;

	if (!patch()->refine(this))
	{
		setUpToDate();
		mRefine = FALSE;
		return mRefine;
	}

	const static F32 a = 0.6f;
	const static F32 K = sMinStep / 50.f;//0.08f;
	const static F32 eps = K;//0.01f;
	
	const F32 tri_dist = llmin(sCam.distance(middleSide()) * sMinStep, sClipFar);
	const F32 func = K * (1 - (1 - eps) * exp(a * (tri_dist - sClipFar)));
	
	
	//const F32 tri_size = distance(mLvtx, mRvtx) * sMinStep;// * min_step2;
	const F32 ratio = mSize / (tri_dist + 1);
	
	if (tri_dist > 0.8 * sClipFar)
		mRefine =  ratio > func;
	else
		mRefine = (ratio > func) && (mLevel < sMaxDivLevel);

	if (!mRefine && !mLeaf)
	{
		if (mLchild->refine())
			mRefine = TRUE;
		else if (mRchild->refine())
			mRefine = TRUE;
	}

	setUpToDate();
	return mRefine;

	//return mGrid->refine(this);
	//return FALSE;
}

