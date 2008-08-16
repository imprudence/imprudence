/** 
 * @file llagpmempoolnv.cpp
 * @brief LLAGPMemPoolNV base class
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

#ifndef LL_LINUX

#include "llagpmempoolnv.h"
#include "llgl.h"

#include "llglheaders.h"

#if LL_WINDOWS
//#define LL_USE_NEW_MEM_OPS 1
#endif
#if LL_USE_NEW_MEM_OPS
#include "new_mem_ops.h"
#endif

BOOL LLAGPMemPoolNV::sWriteOK = TRUE;

LLAGPMemPoolNV::LLAGPMemPoolNV(int request) : LLAGPMemPool()
{
	llinfos << "Creating LLAGPMemPoolNV" << llendl;
	stop_glerror();
	if (!gGLManager.mHasNVFence)
	{
		llerrs << "wglAllocateMemoryNV not defined!" << llendl;
	}

	mBase = 0;

	if (!mBase)
	{
		// More than 4MB of AGP available
		while (!mBase && (request > 0))
		{
			// Stupid arbitrary nVidia magic numbers.
			// read freq, write freq, priority
			// AGP: 0.0, 0.1, 0.5
			// Video: 0.0, 0.1, 1.0
			mBase    = (U8*)wglAllocateMemoryNV(request, 0.0f, 0.1f, 0.5f);
			mSize     = request;
			request >>= 1;
		}
	}

	if (mBase)
	{
		mFreeList.append(*(new LLFreeBlock(0,mSize)));
	}
	else
	{
		mSize = 0;
	}

	sWriteOK = TRUE;
	flush_glerror();
}


LLAGPMemBlock *LLAGPMemPoolNV::createBlock(const U32 offset, const U32 size)
{
	return new LLAGPMemBlockNV(this, mBase, offset, size);
}

LLAGPMemPoolNV::~LLAGPMemPoolNV()
{
	if (mBase)
	{
		wglFreeMemoryNV(mBase);
	}
}

void LLAGPMemPoolNV::bind()
{
	glVertexArrayRangeNV(mSize, mBase);
}

void LLAGPMemPoolNV::enable()
{
	glEnableClientState(GL_VERTEX_ARRAY_RANGE_NV);
	sWriteOK = FALSE;
}

void LLAGPMemPoolNV::disable()
{
	glDisableClientState(GL_VERTEX_ARRAY_RANGE_NV);
	sWriteOK = TRUE;
}

void LLAGPMemPoolNV::flush()
{
	glFlushVertexArrayRangeNV();
}

U32  LLAGPMemPoolNV::createFence()
{
	U32 fence;
    glGenFencesNV(1, &fence);
	glSetFenceNV(fence, GL_ALL_COMPLETED_NV);
    glFinishFenceNV(fence);
	return fence;
}

void LLAGPMemPoolNV::deleteFence(const U32 fence)
{
	glDeleteFencesNV(1, &fence);
}

void LLAGPMemPoolNV::sendFence(U32 fence)
{
    glSetFenceNV(fence, GL_ALL_COMPLETED_NV);
}

void LLAGPMemPoolNV::waitFence(U32 fence)
{
    if(!glTestFenceNV(fence))
    {
        glFinishFenceNV(fence);
	}
}


//static
BOOL LLAGPMemPoolNV::isWriteOK()
{
	return sWriteOK;
}

void LLAGPMemPoolNV::dump()
{
	LLFreeBlock *prev = 0;
	LLFreeBlock *block = mFreeList.getFirst();

	int d=0;

	int i=0;
	while (block)
	{
		i++;
		if (prev)
		{
			d = (S32)block->mOffset - ((S32)prev->mOffset + prev->mSize);
		}
		else d = 0;

		prev  = block;
		block = block->getNext();
	}
}


LLAGPMemBlockNV::LLAGPMemBlockNV(LLAGPMemPool *mem_poolp, U8 *baseptr, S32 offset, const U32 size) : LLAGPMemBlock(mem_poolp)
{
	mMemp = baseptr + offset;
	mOffset = offset;
	mSize = size;
}

extern U8* gAGPVertices;

void LLAGPMemBlockNV::bindGLVertexPointer(const U32 stride, const U32 offset)
{
	if (!mMemp)
	{
		llerrs << "Binding empty vertex array" << llendl;
	}
	glVertexPointer(3, GL_FLOAT, stride, mMemp + offset);
}

void LLAGPMemBlockNV::bindGLNormalPointer(const U32 stride, const U32 offset)
{
	if (!mMemp)
	{
		llerrs << "Binding empty normal array" << llendl;
	}
	glNormalPointer(GL_FLOAT, stride, mMemp + offset);
}


void LLAGPMemBlockNV::bindGLColorPointer(const U32 stride, const U32 offset)
{
	if (!mMemp)
	{
		llerrs << "Binding empty color array" << llendl;
	}
	glColorPointer(4, GL_UNSIGNED_BYTE, stride, mMemp + offset);
}


void LLAGPMemBlockNV::bindGLTexCoordPointer(const U32 stride, const U32 offset)
{
	if (!mMemp)
	{
		llerrs << "Binding empty texcoord array" << llendl;
	}
	glTexCoordPointer(2, GL_FLOAT, stride, mMemp + offset);
}


void LLAGPMemBlockNV::bindGLBinormalPointer(const S32 index, const U32 stride, const U32 offset)
{
	if (!mMemp)
	{
		llerrs << "Binding empty vertex weight array" << llendl;
	}

	glVertexAttribPointerARB(index, 3, GL_FLOAT, FALSE, stride, (F32 *)(mMemp + offset));
}

void LLAGPMemBlockNV::bindGLVertexWeightPointer(const S32 index, const U32 stride, const U32 offset)
{
	if (!mMemp)
	{
		llerrs << "Binding empty vertex weight array" << llendl;
	}

	glVertexAttribPointerARB(index, 1, GL_FLOAT, FALSE, 0, (F32 *)(mMemp + offset));
}

void LLAGPMemBlockNV::bindGLVertexClothingWeightPointer(const S32 index, const U32 stride, const U32 offset)
{
	if (!mMemp)
	{
		llerrs << "Binding empty vertex weight array" << llendl;
	}
	set_vertex_clothing_weights(index, stride, (LLVector4 *)(mMemp + offset));
}

U8* LLAGPMemBlockNV::getMappedMem()
{
	return mMemp;
}

void LLAGPMemBlockNV::copy(void *mem, const U32 size)
{
	if (!mMemp || !mem)
	{
		return;
	}
	llassert(LLAGPMemPoolNV::isWriteOK());
	llassert(size <= mSize);

#if LL_USE_NEW_MEM_OPS
	inline_new_memcpy( mMemp, mem, size );
#else
	memcpy( mMemp, mem, size );
#endif
}

void LLAGPMemBlockNV::copyColor(void *mem, const U32 size)
{
	if (!mMemp || !mem)
	{
		return;
	}
	llassert(LLAGPMemPoolNV::isWriteOK());
	llassert(size <= mSize);

#if LL_USE_NEW_MEM_OPS
	inline_new_memcpy( mMemp, mem, size );
#else
	memcpy( mMemp, mem, size );
#endif
}


U32  LLAGPMemBlockNV::createFence()
{
	U32 fence;
    glGenFencesNV(1, &fence);
	glSetFenceNV(fence, GL_ALL_COMPLETED_NV);
    glFinishFenceNV(fence);
	return fence;
}

void LLAGPMemBlockNV::deleteFence(const U32 fence)
{
	glDeleteFencesNV(1, &fence);
}

void LLAGPMemBlockNV::sendFence(U32 fence)
{
    glSetFenceNV(fence, GL_ALL_COMPLETED_NV);
}

void LLAGPMemBlockNV::waitFence(U32 fence)
{
    if(!glTestFenceNV(fence))
    {
        glFinishFenceNV(fence);
	}
}

#endif //LL_LINUX
