/** 
 * @file llwaterpatch.h
 * @brief LLWaterTri class header file
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

#ifndef LL_WATER_PATCH_H
#define LL_WATER_PATCH_H

#include "llmath.h"
#include "v3math.h"
#include "llroam.h"

const U8 MAX_LEVEL = 10;

class LL2Coord
{
protected:
	S32	mX;
	S32 mY;
public:
	LL2Coord() {}
	//LL2Coord() : mX(0), mY(0) {}
	LL2Coord (S32 i, S32 j) : mX(i), mY(j) {}
	LL2Coord operator+ (const LL2Coord& c) const
	{
		return LL2Coord(mX + c.mX, mY + c.mY);
	}
	LL2Coord operator* (F32 c) const
	{
		return LL2Coord(llround(mX * c), llround(mY * c));
	}
	S32 x() const { return mX; }
	S32 y() const { return mY; }

	S32& x() { return mX; }
	S32& y() { return mY; }

	LL2Coord middle(const LL2Coord& c2) const
	{
		return LL2Coord((x() + c2.x()) >> 1, (y() + c2.y()) >> 1);
	}

	S32 distance2(const LL2Coord& c2) const
	{
		S32 dx = x() - c2.x();
		S32 dy = y() - c2.y();

		return dx * dx + dy * dy;
	}

	F32 distance(const LL2Coord& c2) const
	{
		return (F32) sqrt((F32)distance2(c2));
	}
};






class LLWaterGrid;

class LLWaterTri : public LLRoamTriNode
{
protected:
	LL2Coord	mLvtx; // Left vertex
	LL2Coord	mRvtx; // Right vertex
	LL2Coord	mTvtx; // Top vertex
	LL2Coord	mMiddle; // Top vertex

	F32			mSize;

	BOOL		mCurr;
	BOOL		mRefine;

public:
	static LL2Coord sCam;
	static F32 sClipFar;
	static U32 sMaxDivLevel;
	static U32 sMinStep;

	static BOOL sCurrRound;


public:
	LLWaterTri (const LL2Coord& l, const LL2Coord& r, const LL2Coord& t):
		LLRoamTriNode(0, 0, 0), mLvtx(l), mRvtx(r), mTvtx(t), mRefine(FALSE)
	{
		mSize = mLvtx.distance(mRvtx) * sMinStep;
		mCurr = sCurrRound;
		mMiddle = mLvtx.middle(mRvtx);
	}

	LLWaterTri (U8 level = 0, S8 type = 0, LLWaterTri* par = 0);

	virtual LLRoamTriNode* newLChild()
	{
		return new LLWaterTri(mLevel+1, -1, this);
	}
	virtual LLRoamTriNode* newRChild()
	{
		return new LLWaterTri(mLevel+1, 1, this);
	}

	virtual ~LLWaterTri() {}

	const LL2Coord& Lvtx() const { return mLvtx; }
	const LL2Coord& Rvtx() const { return mRvtx; }
	const LL2Coord& Tvtx() const { return mTvtx; }

	F32 size() const { return mSize; }

	LL2Coord middleSide() const { return mMiddle; }//middle(mLvtx, mRvtx); }

	void setLvtx(const LL2Coord& c) { mLvtx = c; }
	void setRvtx(const LL2Coord& c) { mRvtx = c; }
	void setTvtx(const LL2Coord& c) { mTvtx = c; }

	void updatePassive();
	BOOL refine();
	void initForcefulRefine()
	{
		setUpToDate();
		mRefine = TRUE;
	}
	void flushFromQueue() { setUpToDate(); }

	BOOL upToDate() const { return mCurr == sCurrRound; }
	void setUpToDate() { mCurr = sCurrRound; }
	void setNotUpToDate() { mCurr = !sCurrRound; }
	static void nextRound() { sCurrRound = !sCurrRound; }

	BOOL checkUpToDate() const
	{
		BOOL ok = leaf() ? upToDate() :
			upToDate() && ((LLWaterTri*)Lchild())->upToDate() && ((LLWaterTri*)Rchild())->upToDate();
		if (!ok)
			return ok;
		else
			return ok;
	}

};




class LLWaterPatch : public LLRoamPatch
{
protected:
	LL2Coord	mOrig; // Bottom left vertex
	U32			mSize;
	U32			mRegionWidth;
	LLVector3	mCenter;			
	BOOL		mVis;

public:
	LLWaterPatch() : 
		LLRoamPatch(MAX_LEVEL, TRUE), mOrig(0, 0), mSize(32), mRegionWidth(256) {}

	LLWaterPatch(const LL2Coord o, U32 size, U32 width, const LLVector3& center,
									U8 max_level = MAX_LEVEL, BOOL back_slash = TRUE) : 
		LLRoamPatch(back_slash, max_level), mOrig(o), mSize(size), mRegionWidth(width), mCenter(center)
		{ createTris(); }

	LLWaterPatch(S32 o1, S32 o2, U32 size, U32 width, const LLVector3& center,
									U8 max_level = MAX_LEVEL, BOOL back_slash = TRUE) : 
		LLRoamPatch(back_slash, max_level), mOrig(o1, o2), mSize(size), mRegionWidth(width), mCenter(center)
		{ createTris(); }


	const LL2Coord& orig() const { return mOrig; }
	void set (S32 o1, S32 o2, U32 size, U32 width, const LLVector3& center,
								U8 max_level = MAX_LEVEL, BOOL back_slash = TRUE)
	{
		deleteTris();
		mBackSlash = back_slash;
		mMaxLevel = max_level;
		mOrig.x() = o1;
		mOrig.y() = o2;
		mSize = size;
		mCenter = center;
		mRegionWidth = width;
		mNumTris = 0;
		createTris();
	}

	void setMaxLevel (U8 max_level) { mMaxLevel = max_level; }

	void createTris()
	{
		if (mBackSlash)
		{
			mTri[0]  = new LLWaterTri(LL2Coord(mOrig.x() + mSize, mOrig.y()),
									LL2Coord(mOrig.x(), mOrig.y() + mSize), mOrig);
			mTri[1] = new LLWaterTri(LL2Coord(mOrig.x(), mOrig.y() + mSize),
									LL2Coord(mOrig.x() + mSize, mOrig.y()),
									LL2Coord(mOrig.x() + mSize, mOrig.y() + mSize));
		} else {
			mTri[0]  = new LLWaterTri(mOrig,
									LL2Coord(mOrig.x() + mSize, mOrig.y() + mSize),
									LL2Coord(mOrig.x(), mOrig.y() + mSize));
			mTri[1] = new LLWaterTri(LL2Coord(mOrig.x() + mSize, mOrig.y() + mSize),
									mOrig,
									LL2Coord(mOrig.x() + mSize, mOrig.y()));
		}
		setTris();
		((LLWaterTri*)mTri[0])->setUpToDate();
		((LLWaterTri*)mTri[1])->setUpToDate();
	}
	//virtual ~LLWaterPatch() {}
	void setInvisible() { mVis = FALSE; }
	void setVisible() { mVis = TRUE; }

	BOOL visible() const { return mVis; }
	
	BOOL updateTree(const LLVector3 &camera_pos, const LLVector3 &look_at, const LLVector3 &reg_orig)
	{
		const static F32 patch_rad = mRegionWidth * F_SQRT2 * 0.5f;

		LLVector3 to_patch = reg_orig + mCenter - camera_pos;
		F32 to_patch_dist = to_patch.normVec();

		if ( to_patch_dist < patch_rad)
		{
			setVisible();
			update();
		} else {
			const F32 sin_min_angle = patch_rad / to_patch_dist;
			const F32 cos_min_angle = (F32)sqrt(1.f - sin_min_angle * sin_min_angle);
			const F32 cos_max = OO_SQRT2 * (cos_min_angle - sin_min_angle);

			if (to_patch * look_at > cos_max)
			{
				setVisible();
				update();
			} else {
				setInvisible();
				updatePassive();
			}
		}

		return mVis;
	}

	BOOL updateVisibility(const LLVector3 &camera_pos, const LLVector3 &look_at, const LLVector3 &reg_orig)
	{
		const static F32 patch_rad = mRegionWidth * F_SQRT2 * 0.5f;
		const static U32 reg_width_half = mRegionWidth / 2;
		//const static F32 patch_rad2 = patch_rad * patch_rad;

		LLVector3 to_patch = reg_orig + mCenter - camera_pos;
		//const F32 to_patch_dist2D2 = to_patch.mV[VX] * to_patch.mV[VX] + to_patch.mV[VY] * to_patch.mV[VY];

		if (fabs(to_patch.mV[VX]) <= reg_width_half && fabs(to_patch.mV[VY]) <= reg_width_half)
		//if ( to_patch_dist2D2 < patch_rad2)
		{
			setVisible();
		} else {
			F32 to_patch_dist = to_patch.normVec();
			//const F32 to_patch_dist = sqrt(to_patch_dist2D2 + to_patch.mV[VZ] * to_patch.mV[VZ]);
			const F32 sin_min_angle = patch_rad / to_patch_dist;
			if (sin_min_angle >= 1)
			{
				setVisible();
			} else {
				const F32 cos_min_angle = (F32)sqrt(1.f - sin_min_angle * sin_min_angle);
				const F32 cos_max = OO_SQRT2 * (cos_min_angle - sin_min_angle);

				if (to_patch * look_at > cos_max)
				{
					setVisible();
				} else {
					setInvisible();
				}
			}
		}

		return mVis;
	}
	void checkUpToDate() const
	{
		for (U8 h = 0; h < 2; h++)
		{
			((LLWaterTri*)left())->checkUpToDate();
			((LLWaterTri*)right())->checkUpToDate();
		}
	}

};


#endif
