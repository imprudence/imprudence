/** 
 * @file llagpmempoolapple.h
 * @brief LLAGPMemPoolAPPLE base class
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

#ifndef LL_LLAGPMEMPOOLAPPLE_H
#define LL_LLAGPMEMPOOLAPPLE_H

#include "llagpmempool.h"

#if LL_DARWIN
class LLAGPMemPoolAPPLE : public LLAGPMemPool
{
public:
	LLAGPMemPoolAPPLE(S32 request);
	virtual ~LLAGPMemPoolAPPLE();

	/*virtual*/ void flush();
	/*virtual*/ void dump();
	/*virtual*/ void enable();
	/*virtual*/ void disable();
	/*virtual*/ void bind();

	/*virtual*/ U32  createFence();
	/*virtual*/ void deleteFence(const U32 fence);
	/*virtual*/ void sendFence(U32 fence);
	/*virtual*/ void waitFence(U32 fence);

protected:
	/*virtual*/ LLAGPMemBlock *createBlock(const U32 offset, const U32 size);

protected:
	U8 *mBase;
};

class LLAGPMemBlockAPPLE : public LLAGPMemBlock
{
public:
	LLAGPMemBlockAPPLE(LLAGPMemPool *mem_poolp, U8 *baseptr, const S32 offset, const U32 size);
	virtual ~LLAGPMemBlockAPPLE();

	/*virtual*/ void copy     (void *source, const U32 size_bytes);
	/*virtual*/ void copyColor(void *source, const U32 size_bytes);
	/*virtual*/ void free();

	/*virtual*/ void bindGLVertexPointer(const U32 stride, const U32 offset);
	/*virtual*/ void bindGLNormalPointer(const U32 stride, const U32 offset);
	/*virtual*/ void bindGLBinormalPointer(const S32 index, const U32 stride, const U32 offset);
	/*virtual*/ void bindGLColorPointer(const U32 stride, const U32 offset);
	/*virtual*/ void bindGLTexCoordPointer(const U32 stride, const U32 offset);
	/*virtual*/ void bindGLVertexWeightPointer(const S32 index, const U32 stride, const U32 offset);
	/*virtual*/ void bindGLVertexClothingWeightPointer(const S32 index, const U32 stride, const U32 offset);

	/*virtual*/ U32 getOffset()        const { return mOffset; }
	/*virtual*/ U32 getSize()          const { return mSize; }

	/*virtual*/ BOOL hasMappedMem() const		{ return TRUE; }
	/*virtual*/ U8*  getMappedMem();
	/*virtual*/ U32  createFence();
	/*virtual*/ void deleteFence(const U32 fence);
	/*virtual*/ void sendFence(U32 fence);
	/*virtual*/ void waitFence(U32 fence);

private:
	U8 *mMemp;
	U32 mOffset; // Offset from base
	U32 mSize;
	friend class LLAGPMemPoolAPPLE;
};

#endif // LL_DARWIN

#endif // LL_LLAGPMEMPOOLAPPLE_H
