/** 
 * @file llroam.h
 * @brief LLRoam and related class definitions
 *
 * Copyright (c) 2003-2007, Linden Research, Inc.
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

/****************************************/
/*	Implementation of ROAM algorithm	*/
/****************************************/

#ifndef LL_ROAM_H
#define LL_ROAM_H

#include "stdtypes.h"
#include "doublelinkedlist.h"

class LLRoamPatch;
class LLRoam;

class LLRoamTriNode
{
protected:
	U8				mLevel;
	S8				mType;	// -1 - left child, 1 - righ child, 0 - top vertex
	LLRoamTriNode*	mParent;
	LLRoamTriNode*	mLchild;
	LLRoamTriNode*	mRchild;
	LLRoamTriNode*	mLeft;
	LLRoamTriNode*	mRight;
	LLRoamTriNode*	mBase;
	LLRoamPatch*	mPatch;
	BOOL			mLeaf;
	BOOL			mInQueue;

public:
	static LLRoam*	sQueues;

	//LLRoamTriNode(S8 level = 0, const LLRoamTriNode* left = 0
	//	const LLRoamTriNode* right = 0, const LLRoamTriNode* base = 0): 
	//	mLevel(level), mLeft(left), mRight(right), mBase(base), mLchild(0), mRchild(0) {}
	LLRoamTriNode(U8 level = 0, S8 type = 0, LLRoamTriNode* par = 0):
		mLevel(level), mType(type), mParent(par), mLchild(0), mRchild(0),
		mLeft(0), mRight(0), mBase(0), mPatch(0),
		mLeaf(TRUE), mInQueue(FALSE) {}

	virtual LLRoamTriNode* newLChild()
	{
		return new LLRoamTriNode(mLevel+1, -1, this);
	}
	virtual LLRoamTriNode* newRChild()
	{
		return new LLRoamTriNode(mLevel+1, 1, this);
	}

	virtual ~LLRoamTriNode();
	LLRoamTriNode*	Lchild() const { return mLchild; }
	LLRoamTriNode*	Rchild() const { return mRchild; }
	LLRoamTriNode*	left() const { return mLeft; }
	LLRoamTriNode*	right() const { return mRight; }
	LLRoamTriNode*	base() const { return mBase; }
	LLRoamTriNode*	parent() const { return mParent; }
	void setLeft(LLRoamTriNode* left) { mLeft = left; }
	void setRight(LLRoamTriNode* right) { mRight = right; }
	void setBase(LLRoamTriNode* base) { mBase = base; }
	void setParent(LLRoamTriNode* parent) { mParent = parent; }
	U8 level() const { return mLevel; }
	BOOL leaf() const { return mLeaf; }

	void updateLink(LLRoamTriNode* old_link, LLRoamTriNode* new_link);
	BOOL split(BOOL forceful = FALSE);
	BOOL merge();
	void mergeSimple();
	//void refine();
	void update();
	virtual void updatePassive() {}

	virtual BOOL refine();// { return patch()->refine(this); }
	virtual void initForcefulRefine() {}
	virtual void flushFromQueue() {}
	LLRoamPatch* patch() const { return mPatch; }
	void setPatch(LLRoamPatch*  p) { mPatch = p; }

	const LLRoamTriNode* getFirstLeaf() const;
	const LLRoamTriNode* getNextLeaf() const;

	BOOL inQueue() const { return mInQueue; }
	void enqueue() { mInQueue = TRUE; }
	void dequeue() { mInQueue = FALSE; }

	BOOL checkConsistensy() const
	{
		BOOL ok = TRUE;
		if (leaf())
		{
			if (base() && !base()->leaf())
				ok = FALSE;
			if (Lchild() && !Lchild()->leaf())
				ok = FALSE;
			if (Rchild() && !Rchild()->leaf())
				ok = FALSE;
		} else {
			if (base() && base()->leaf())
				ok = FALSE;
		}
		return ok;
	}

};


class LLRoamPatch
{
protected:
	BOOL mBackSlash;
	LLRoamTriNode* mTri[2];
	U8 mMaxLevel;
	U32 mNumTris;
public:
	LLRoamPatch(U8 max_level, BOOL back_slash) : mBackSlash(back_slash), mMaxLevel(max_level), mNumTris(0)
	{
		mTri[0] = 0;
		mTri[1] = 0;
	}

	virtual ~LLRoamPatch() { deleteTris(); }
	void deleteTris()
	{
		delete mTri[0];
		mTri[0] = 0;
		delete mTri[1];
		mTri[1] = 0;
	}

	void updatePassive()
	{
		mTri[0]->updatePassive();
		mTri[1]->updatePassive();
	}

	void update()
	{
		mTri[0]->update();
		mTri[1]->update();
		
		//checkCount();
	}

	U32 checkCount()
	{
		BOOL ok = TRUE;
		U32 ntri = 0;
		for (U8 h = 0; h < 2; h++)
		{
			for (const LLRoamTriNode*   tri = mTri[h]->getFirstLeaf();
										tri != NULL;
										tri = tri->getNextLeaf())
				ntri++;
		}
		if (ntri != mNumTris)
			ok = FALSE;
		return mNumTris;
	}

	LLRoamTriNode*	left() const { return mTri[0]; }
	LLRoamTriNode*	right() const { return mTri[1]; }
	LLRoamTriNode*	half(U8 h) const { return mTri[h]; }

	U8 maxLevel() const { return mMaxLevel; }
	BOOL refine(const LLRoamTriNode* tri) const { return tri->level() < mMaxLevel; }
	U32 numTris() const { return mNumTris; }
	U32 numTrisInc() { mNumTris++; return mNumTris; }
	U32 numTrisDec() { mNumTris--; return mNumTris; }
	void setTris(LLRoamTriNode* left, LLRoamTriNode* right)
	{
		mTri[0] = left;
		mTri[1] = right;
		setTris();
	}

	void setTris()
	{
		mTri[0]->setBase(mTri[1]);
		mTri[1]->setBase(mTri[0]);
		mTri[0]->setPatch(this);
		mTri[1]->setPatch(this);
		mNumTris = 2;
	}
	void checkConsistensy() const
	{
		for (U8 h = 0; h < 2; h++)
		{
			left()->checkConsistensy();
			right()->checkConsistensy();
			/*
			for (const LLWaterTri*	tri = (LLWaterTri*)mTri[h]->getFirstLeaf();
									tri != NULL;
									tri = (LLWaterTri*)tri->getNextLeaf())
			{
				if (!tri->upToDate())
					return FALSE;
			} */
		}
	}
};

inline BOOL LLRoamTriNode::refine() { return patch()->refine(this); }

class LLRoam
{
protected:
	LLDoubleLinkedList<LLRoamTriNode> mSplitQ;
	LLDoubleLinkedList<LLRoamTriNode> mMergeQ;
	U32		mNum;
public:
	LLRoam() {}
	~LLRoam() { mSplitQ.removeAllNodes(); mMergeQ.removeAllNodes(); }

	void pushSplit(LLRoamTriNode* t)
	{
		if (!t->inQueue())
		{
			mSplitQ.addDataAtEnd(t);
			t->enqueue();
		}
	}
	LLRoamTriNode* popSplit()
	{
		LLRoamTriNode* t = mSplitQ.getFirstData();
		t->dequeue();
		mSplitQ.removeCurrentData();
		return t;
	}
	void pushMerge(LLRoamTriNode* t)
	{
		if (!t->inQueue())
		{
			mMergeQ.addDataAtEnd(t);
			t->enqueue();
		}
	}
	LLRoamTriNode* popMerge()
	{
		LLRoamTriNode* t = mMergeQ.getFirstData();
		t->dequeue();
		mMergeQ.removeCurrentData();
		return t;
	}

	void queueForSplit(LLRoamTriNode* t, BOOL process_base = TRUE);
	void queueForMerge(LLRoamTriNode* t, BOOL process_base = TRUE);

	void processSplit();
	void processMerge();
	void process();

	U32  numOfProcessedTris() const { return mNum; }
	void numOfProcessedTrisInc() { mNum++; }
	void resetCount() { mNum = 0; }
//	BOOL processTooLong() const { return mNum > 10000; }
	void flushSplit();
	void flushMerge();
	/*
	void cutProcessing()
	{
		flushMerge();
		flushSplit();
		resetCount();
	} */
	void checkTiming()
	{
		if (mNum > 1000)
		{
			flushMerge();
			flushSplit();
			resetCount();
		}
	}

	BOOL mergeQueueTooLong() const { return mMergeQ.getLength() > 1000; }
	BOOL splitQueueTooLong() const { return mSplitQ.getLength() > 1000; }
};

#endif
