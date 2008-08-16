/** 
 * @file llroam.cpp
 * @brief LLRoam and related class implementations
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

#include "llviewerprecompiledheaders.h"

#include "llroam.h"

LLRoam*	LLRoamTriNode::sQueues;// = new LLRoam();

void LLRoamTriNode::updateLink(LLRoamTriNode* old_link, LLRoamTriNode* new_link)
{
	if (old_link == mLeft)
		mLeft = new_link;
	else if (old_link == mRight)
		mRight = new_link;
	else if (old_link == mBase)
		mBase = new_link;
}

BOOL LLRoamTriNode::split(BOOL forceful)
{
	//U32 c1 = patch()->checkCount();
	if (forceful)
	{
		initForcefulRefine();
	} /*else {
		if (!refine())
			return FALSE;
	}*/

	if (!leaf())
		return TRUE;

	if (mLchild == NULL)
	{
		mLchild = newLChild();
		mRchild = newRChild();

		//mLchild->setPatch(patch());
		//mRchild->setPatch(patch());

		mLchild->setLeft(mRchild);
		mRchild->setRight(mLchild);
	}

	mLchild->setBase(mLeft);
	mRchild->setBase(mRight);

	if (mLeft)
		mLeft->updateLink(this, mLchild);
	if (mRight)
		mRight->updateLink(this, mRchild);

	mLeaf = FALSE;

	patch()->numTrisInc();

	//U32 c2 = patch()->checkCount();
	sQueues->numOfProcessedTrisInc();
	if (mBase == NULL)
		return TRUE;

	if (mBase->base() != this)	// not Diamond shape
		mBase->split(TRUE);

	if (mBase->leaf())
		mBase->split(TRUE);

	mLchild->setRight(mBase->Rchild());
	mRchild->setLeft(mBase->Lchild());
	//U32 c3 = patch()->checkCount();
	return TRUE;
}

BOOL LLRoamTriNode::merge()
{
	//U32 c1 = patch()->checkCount();
	if (leaf())
		return TRUE;

	if (mBase && mBase->refine())
		return FALSE;

	if (!mLchild->merge() || !mRchild->merge())
		return FALSE;

	//U32 c2 = patch()->checkCount();
	mLeft = mLchild->base();
	mRight = mRchild->base();

	if (mLeft)
		mLeft->updateLink(mLchild, this);
	if (mRight)
		mRight->updateLink(mRchild, this);

	//U32 c4 = patch()->checkCount();
	mLeaf = TRUE;

	patch()->numTrisDec();

	//U32 c5 = patch()->checkCount();
	if (mBase != NULL)
	{
		if (!mBase->merge())
		{
			mLeaf = FALSE;
			patch()->numTrisInc();
			return FALSE;
		}
	}

	//U32 c6 = patch()->checkCount();
	return TRUE;
}

void LLRoamTriNode::mergeSimple()
{
	mLeft = mLchild->base();
	mRight = mRchild->base();
	
	if (mLeft)
		mLeft->updateLink(mLchild, this);
	if (mRight)
		mRight->updateLink(mRchild, this);
	
	mLeaf = TRUE;
	patch()->numTrisDec();
	sQueues->numOfProcessedTrisInc();
}

void LLRoamTriNode::update()
{
	if (refine())
		sQueues->queueForSplit(this);
	else if (!leaf())
		sQueues->queueForMerge(this);
}

/*
void LLRoamTriNode::update()
{
	//U32 c1 = patch()->checkCount();
	if (split())
	{
		mLchild->update();
		mRchild->update();
	//U32 c2 = patch()->checkCount();
	}
	else if (!leaf())
	{
		merge();
	}
}
*/

LLRoamTriNode::~LLRoamTriNode()
{
	delete mLchild;
	mLchild = 0;
	delete mRchild;
	mRchild = 0;
	//delete sQueues;
	//sQueues = 0;
}



const LLRoamTriNode* LLRoamTriNode::getFirstLeaf() const
{
	const LLRoamTriNode* node = this;
	while (!node->leaf())
		node = node->Lchild();
	return node;
}
const LLRoamTriNode* LLRoamTriNode::getNextLeaf() const
{
	const LLRoamTriNode* child = this;
	const LLRoamTriNode* prev = parent();
	while (prev) {
		if (prev->Lchild() == child)
			return prev->Rchild()->getFirstLeaf();
		child = prev;
		prev = prev->parent();
	};
	return NULL;
}

void LLRoam::queueForSplit(LLRoamTriNode* t, BOOL process_base)
{
	//if (splitQueueTooLong())
	//	processSplit();
	//if (t->base() && process_base)
	//	queueForSplit(t->base(), FALSE)
	pushSplit(t);
}

void LLRoam::queueForMerge(LLRoamTriNode* t, BOOL process_base)
{
	//if (mergeQueueTooLong())
	//	processMerge();
	if (t->leaf())
		return;
	queueForMerge(t->Lchild());
	queueForMerge(t->Rchild());
	if (t->base() && process_base)
		queueForMerge(t->base(), FALSE);
	pushMerge(t);
}


void LLRoam::processSplit()
{
	while(!mSplitQ.isEmpty())
	{
		LLRoamTriNode* tri = popSplit();
		if (tri->split())
		{
			tri->Lchild()->update();
			tri->Rchild()->update();
		}
		//checkTiming();
	}
}

void LLRoam::processMerge()
{
	while(!mMergeQ.isEmpty())
	{
		LLRoamTriNode* tri = popMerge();
		if (tri->refine())
			continue;
		if (tri->leaf())
			continue;
		if (!tri->Lchild()->leaf() || !tri->Rchild()->leaf())
			continue;
		if (LLRoamTriNode* b = tri->base())
		{
			if (b->leaf() || (b->Lchild()->leaf() && b->Rchild()->leaf()))
			{
				tri->mergeSimple();
			}
		}
		//checkTiming();
	}
}

void LLRoam::process()
{
	while(!mSplitQ.isEmpty() || !mMergeQ.isEmpty())
	{
		processMerge();
		processSplit();
	}
}


void LLRoam::flushSplit()
{
	while(!mSplitQ.isEmpty())
	{
		LLRoamTriNode* tri = popSplit();
		tri->flushFromQueue();
	}
}

void LLRoam::flushMerge()
{
	while(!mMergeQ.isEmpty())
	{
		LLRoamTriNode* tri = popMerge();
		if (tri->leaf())
			continue;
		if (tri->base()->leaf())
		{
			tri->mergeSimple();
		}
		tri->flushFromQueue();
	}
}
