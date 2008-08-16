/** 
 * @file llagpmempool.h
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

#ifndef LL_LLAGPMEMPOOL_H
#define LL_LLAGPMEMPOOL_H

#include "stdtypes.h"
#include "lldlinked.h"

// Raw memory handling abstraction, which handles interaction with
// the nVidia and ATI AGP extensions.

// Minimum size we allow allocation for, in order to allow AGP usage...
const S32 MIN_AGP_SIZE = 8000000;

class LLAGPMemBlock;

class LLAGPMemPool
{
public:
	LLAGPMemPool();
	virtual ~LLAGPMemPool();

	virtual LLAGPMemBlock *allocBlock(const S32 size);
	virtual void freeBlock(LLAGPMemBlock *blockp);

	virtual void flush() = 0;
	virtual void dump() = 0;
	virtual void enable() = 0;
	virtual void disable() = 0;
	virtual void bind() = 0;

	virtual S32    getSize()		{ return mSize; }

	S32 getTotalAllocated() const	{ return mTotalAllocated; }
	static LLAGPMemPool *createPool(const U32 size, const BOOL use_vbo);

	struct LLFreeBlock: public LLDLinked<LLFreeBlock>
	{
		S32  mOffset;
		S32  mSize;
		LLFreeBlock(const S32 offset, const S32 size) { mOffset = offset; mSize = size; sNumBlocks++; }
		~LLFreeBlock() { sNumBlocks--; }

		static S32 sNumBlocks;
	};

	// Fencing (for nVidia and Apple) - default is to do nothing (ATI, ARB do not need fencing)
	virtual U32  createFence() { return 0; }
	virtual void deleteFence(const U32 fence) {}
	virtual void sendFence(U32 fence) {}
	virtual void waitFence(U32 fence) {}

	void printFreeList();
protected:

	void coalesce(LLFreeBlock *free_block);
	virtual LLAGPMemBlock *createBlock(const U32 offset, const U32 size) = 0;
	LLDLinked<LLFreeBlock> mFreeList;

	S32 mSize;
	S32 mTotalAllocated;
};

// An AGP memory block, which contains all the info needed to
// copy data in/out.
class LLAGPMemBlock
{
public:
	LLAGPMemBlock(LLAGPMemPool *mem_poolp) : mMemPoolp(mem_poolp) {}
	virtual ~LLAGPMemBlock()					{}
	virtual void copy     (void *source, const U32 size_bytes) = 0;
	virtual void copyColor(void *source, const U32 size_bytes) = 0;

	virtual void bindGLVertexPointer(const U32 stride, const U32 offset) = 0;
	virtual void bindGLNormalPointer(const U32 stride, const U32 offset) = 0;
	virtual void bindGLBinormalPointer(const S32 index, const U32 stride, const U32 offset) = 0;
	virtual void bindGLColorPointer(const U32 stride, const U32 offset) = 0;
	virtual void bindGLTexCoordPointer(const U32 stride, const U32 offset) = 0;
	virtual void bindGLVertexWeightPointer(const S32 index, const U32 stride, const U32 offset) = 0;
	virtual void bindGLVertexClothingWeightPointer(const S32 index, const U32 stride, const U32 offset) = 0;

	virtual BOOL hasMappedMem() const = 0;
	virtual U8*	getMappedMem() = 0;

	virtual U32  createFence() = 0;
	virtual void deleteFence(const U32 fence) = 0;
	virtual void sendFence(U32 fence) = 0;
	virtual void waitFence(U32 fence) = 0;

	virtual U32 getOffset() const = 0;
	virtual U32 getSize() const = 0;
protected:
	LLAGPMemPool *mMemPoolp;
};

#endif // LL_LLAGPMEMPOOL_H
