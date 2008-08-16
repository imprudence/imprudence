/** 
 * @file llagpmempoolarb.cpp
 * @brief LLAGPMemPoolARB base class
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
#include "llagpmempoolarb.h"
#include "llgl.h"

#include "llglheaders.h"

LLAGPMemPoolARB::LLAGPMemPoolARB(S32 request) : LLAGPMemPool()
{
	llinfos << "Creating LLAGPMemPoolARB" << llendl;
	stop_glerror();
	if (!gGLManager.mHasVertexBufferObject)
	{
		llerrs << "No ARB vertex buffer object extension!" << llendl;
	}

	mName = 0;

	mSize = request;
	flush_glerror();
}


LLAGPMemPoolARB::~LLAGPMemPoolARB()
{
}


LLAGPMemBlock* LLAGPMemPoolARB::allocBlock(const S32 size) 
{
	return allocBlock(size, GL_ARRAY_BUFFER_ARB); 
}

LLAGPMemBlock *LLAGPMemPoolARB::allocBlock(const S32 size, U32 target)
{
	S32 aligned_size = size;
	if (size & 0x0f)
	{
		aligned_size += 16 - (size & 0x0f);
	}
	
	if (aligned_size > (mSize - mTotalAllocated))
	{
		// We're totally out of AGP memory, bail.
		return (LLAGPMemBlock *)0;
	}

	mTotalAllocated += aligned_size;
	return createBlock(0, aligned_size, target);
}


void LLAGPMemPoolARB::freeBlock(LLAGPMemBlock *blockp)
{
	if (!blockp->getSize())
	{
		return;
	}
	LLAGPMemBlockARB *arb_blockp = (LLAGPMemBlockARB*)blockp;
	U32 name[1];
	name[0] = arb_blockp->getName();
	stop_glerror();
	glDeleteBuffersARB(1, (GLuint*)name);
	stop_glerror();
	mTotalAllocated -= blockp->getSize();
}

LLAGPMemBlock *LLAGPMemPoolARB::createBlock(const U32 offset, const U32 size)
{
	return createBlock(offset, size, GL_ARRAY_BUFFER_ARB);
}

LLAGPMemBlock *LLAGPMemPoolARB::createBlock(const U32 offset, const U32 size, const U32 target)
{
	U32 name[1];
	stop_glerror();
	glGenBuffersARB(1, (GLuint*)name);
	stop_glerror();
	return new LLAGPMemBlockARB(this, name[0], offset, size, target);
}

void LLAGPMemPoolARB::disable()
{
}

void LLAGPMemPoolARB::dump()
{
}


/////////////////////////////
//
// LLAGPMemBlockARB
//
// ARB ImplementARBon of an AGP memory block
//

LLAGPMemBlockARB::LLAGPMemBlockARB(LLAGPMemPool *mem_poolp, const U32 name, const U32 offset, const U32 size, U32 target) :
	LLAGPMemBlock(mem_poolp), mTarget(target)
{
	llassert(name > 0);
	mName = name;
	stop_glerror();
	glBindBufferARB(mTarget, mName);
	stop_glerror();
	
	glBufferDataARB(mTarget, size, NULL, GL_DYNAMIC_DRAW_ARB);
	
	stop_glerror();
	glBindBufferARB(mTarget, 0);
	stop_glerror();
	mSize = size;
}


void LLAGPMemBlockARB::bindGLVertexPointer(const U32 stride, const U32 offset)
{
	stop_glerror();
	glBindBufferARB(mTarget, mName);
	glVertexPointer(3, GL_FLOAT, stride, (GLvoid*)((intptr_t)offset));
	glBindBufferARB(mTarget, 0);
}


void LLAGPMemBlockARB::bindGLNormalPointer(const U32 stride, const U32 offset)
{
	stop_glerror();
	glBindBufferARB(mTarget, mName);
	glNormalPointer(GL_FLOAT, stride, (GLvoid*)((intptr_t)offset));
	glBindBufferARB(mTarget, 0);
	stop_glerror();
}


void LLAGPMemBlockARB::bindGLColorPointer(const U32 stride, const U32 offset)
{
	stop_glerror();
	glBindBufferARB(mTarget, mName);
	glColorPointer(4, GL_UNSIGNED_BYTE, stride, (GLvoid*)((intptr_t)offset));
	glBindBufferARB(mTarget, 0);
	stop_glerror();
}


void LLAGPMemBlockARB::bindGLTexCoordPointer(const U32 stride, const U32 offset)
{
	stop_glerror();
	glBindBufferARB(mTarget, mName);
	glTexCoordPointer(2, GL_FLOAT, stride, (GLvoid*)((intptr_t)offset));
	glBindBufferARB(mTarget, 0);
	stop_glerror();
}


void LLAGPMemBlockARB::bindGLVertexWeightPointer(const S32 index, const U32 stride, const U32 offset)
{
	stop_glerror();
	glBindBufferARB(mTarget, mName);
	set_vertex_weights(index, (F32*)(intptr_t)offset);
	glBindBufferARB(mTarget, 0);
	stop_glerror();
}

void LLAGPMemBlockARB::bindGLBinormalPointer(const S32 index, const U32 stride, const U32 offset)
{
	stop_glerror();
	glBindBufferARB(mTarget, mName);
	set_binormals(index, stride, (LLVector3*)(intptr_t)offset);
	glBindBufferARB(mTarget, 0);
	stop_glerror();
}


void LLAGPMemBlockARB::bindGLVertexClothingWeightPointer(const S32 index, const U32 stride, const U32 offset)
{
	return;
}


void LLAGPMemBlockARB::copy(void *mem, const U32 size)
{
	stop_glerror();
	llassert(size <= mSize);
	glBindBufferARB(mTarget, mName);
	glBufferSubDataARB(mTarget, 0, size, mem);
	glBindBufferARB(mTarget, 0);
	stop_glerror();
}

void LLAGPMemBlockARB::copyColor(void *mem, const U32 size)
{
	stop_glerror();
	llassert(size <= mSize);
	glBindBufferARB(mTarget, mName);
	glBufferSubDataARB(mTarget, 0, size, mem);
	glBindBufferARB(mTarget, 0);
	stop_glerror();
}


U8*	LLAGPMemBlockARB::getMappedMem()
{
	return NULL;
}
