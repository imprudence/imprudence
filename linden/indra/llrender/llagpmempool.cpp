/** 
 * @file llagpmempool.cpp
 * @brief LLAGPMemPool base class
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

#include "linden_common.h"

#include "llagpmempool.h"
#include "llgl.h"

#include "llagpmempoolarb.h"
#include "llagpmempoolnv.h"
#include "llagpmempoolati.h"

#if LL_DARWIN
#include "llagpmempoolapple.h"
#endif // LL_DARWIN

//static
S32 LLAGPMemPool::LLFreeBlock::sNumBlocks = 0;

LLAGPMemPool::LLAGPMemPool()
{
	mSize = 0;
	mTotalAllocated = 0;
}

LLAGPMemPool::~LLAGPMemPool()
{
	mFreeList.deleteAll();
}

LLAGPMemPool *LLAGPMemPool::createPool(const U32 size, const BOOL use_vbo)
{
	if (gGLManager.mHasVertexBufferObject && use_vbo)
	{ 
		return new LLAGPMemPoolARB(size);
	}
#if LL_WINDOWS  // *FIX: linux can use these, too, with some work.
	if (gGLManager.mHasNVFence)
	{
		return new LLAGPMemPoolNV(size);
	}
	else if (gGLManager.mHasATIVAO)
	{
		return new LLAGPMemPoolATI(size);
	}
	else
#elif LL_DARWIN
	if (gGLManager.mHasAPPLEFence && gGLManager.mHasAPPLEVertexArrayRange)
	{ 
		return new LLAGPMemPoolAPPLE(size);
	}
	else
#endif
	{
		// No AGP memory allocation at all!
		return NULL;
	}
}


LLAGPMemBlock *LLAGPMemPool::allocBlock(const S32 size)
{
	S32 aligned_size = size;
	if (size & 0x0f)
	{
		aligned_size += 16 - (size & 0x0f);
	}
	
	if (aligned_size > (mSize - mTotalAllocated))
	{
		// We're totally out of AGP memory, bail.
		return NULL;
	}

	LLFreeBlock *free_block = mFreeList.getFirst();

	while (free_block && free_block->mSize < aligned_size)
	{
		free_block = free_block->getNext();
	}

	U32 offset = 0;

	if (free_block)
	{
		if (free_block->mSize == aligned_size)
		{
			free_block->unlink();
			offset = free_block->mOffset;
			delete free_block;
		}
		else
		{
			offset = free_block->mOffset + free_block->mSize - aligned_size;
			free_block->mSize -= aligned_size;
			if (0 == free_block->mSize)
			{
				free_block->unlink();
			}
		}
	}
	else
	{
		//llwarns << "LLAGPMemPool unable to allocate " << size << " bytes" << llendl;
		return NULL;
	}

	mTotalAllocated += aligned_size;
	return createBlock(offset, aligned_size);
}


void LLAGPMemPool::freeBlock(LLAGPMemBlock *blockp)
{
	if (!blockp->getSize())
	{
		return;
	}

	LLFreeBlock *prev_free = NULL;
	LLFreeBlock *cur_free = mFreeList.getFirst();

	while (cur_free && blockp->getOffset() > (U32)cur_free->mOffset)
	{
		prev_free = cur_free;
		cur_free = cur_free->getNext();
	}

	LLFreeBlock *new_free = new LLFreeBlock(blockp->getOffset(), blockp->getSize());

	if (prev_free)
	{
		prev_free->append(*new_free);
		coalesce(new_free);
		coalesce(prev_free);
	}
	else
	{
		mFreeList.append(*new_free);
		coalesce(new_free);
	}
	mTotalAllocated -= blockp->getSize();
}

void LLAGPMemPool::coalesce(LLFreeBlock *free_block)
{
	LLFreeBlock *next = free_block->getNext();

	if (next && (free_block->mOffset + free_block->mSize == next->mOffset))
	{
		free_block->mSize += next->mSize;

		next->unlink();
		delete next;

		coalesce(free_block);
	}
}

void LLAGPMemPool::printFreeList()
{
	LLFreeBlock *cur_block = mFreeList.getFirst();
	while (cur_block)
	{
		llinfos << "Cur block begin: " << cur_block->mOffset << llendl;
		llinfos << "Cur block end:   " << cur_block->mOffset + cur_block->mSize << llendl;
		cur_block = cur_block->getNext();
	}
}

