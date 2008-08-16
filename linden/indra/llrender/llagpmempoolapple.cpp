/** 
 * @file llagpmempoolapple.cpp
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

#ifdef LL_DARWIN

#include "linden_common.h"

#include "llagpmempoolapple.h"
#include "llgl.h"

#include "llglheaders.h"

LLAGPMemPoolAPPLE::LLAGPMemPoolAPPLE(S32 request) : LLAGPMemPool()
{
	llinfos << "Creating LLAGPMemPoolAPPLE" << llendl;
	stop_glerror();
	if (!gGLManager.mHasAPPLEFence || !gGLManager.mHasAPPLEVertexArrayRange)
	{
		llerrs << "necessary extensions not present!" << llendl;
	}
	
	// No special allocation is necessary for the Apple extensions
	mBase    = (U8*)::malloc(request);
	mSize     = request;

	if (mBase)
	{
		mFreeList.append(*(new LLFreeBlock(0,mSize)));
	}
	else
	{
		mSize = 0;
	}

	flush_glerror();
}

LLAGPMemBlockAPPLE::~LLAGPMemBlockAPPLE()
{
	mMemPoolp->freeBlock(this); 
}
 
LLAGPMemBlock *LLAGPMemPoolAPPLE::createBlock(const U32 offset, const U32 size)
{
	return new LLAGPMemBlockAPPLE(this, mBase, offset, size);
}

LLAGPMemPoolAPPLE::~LLAGPMemPoolAPPLE()
{	
	if (mBase)
	{
		// MBW -- This really belongs in a call which is the opposite of bind()...
		glVertexArrayRangeAPPLE(0, 0);
		::free(mBase);
	}
}

void LLAGPMemPoolAPPLE::bind()
{
	if(mBase)
	{
		glVertexArrayRangeAPPLE(mSize, mBase);
	}
}

void LLAGPMemPoolAPPLE::enable()
{
	glEnableClientState(GL_VERTEX_ARRAY_RANGE_APPLE);
}

void LLAGPMemPoolAPPLE::disable()
{
	glDisableClientState(GL_VERTEX_ARRAY_RANGE_APPLE);
}

void LLAGPMemPoolAPPLE::flush()
{
	if(mBase)
	{
		glFlushVertexArrayRangeAPPLE(mSize, mBase);
	}
}

void LLAGPMemPoolAPPLE::dump()
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


U32  LLAGPMemPoolAPPLE::createFence()
{
	U32 fence;
    glGenFencesAPPLE(1, (GLuint*)&fence);
	glSetFenceAPPLE(fence);
    glFinishFenceAPPLE(fence);
	return fence;
}


void  LLAGPMemPoolAPPLE::deleteFence(const U32 fence)
{
    glDeleteFencesAPPLE(1, (GLuint*)&fence);
}


void LLAGPMemPoolAPPLE::sendFence(U32 fence)
{
    glSetFenceAPPLE(fence);
}


void LLAGPMemPoolAPPLE::waitFence(U32 fence)
{
    if(!glTestFenceAPPLE(fence))
    {
        glFinishFenceAPPLE(fence);
	}
}


/////////////////////////////
//
// LLAGPMemBlockAPPLE
//
// APPLE Implementation of an AGP memory block
//

LLAGPMemBlockAPPLE::LLAGPMemBlockAPPLE(LLAGPMemPool *mem_poolp, U8 *baseptr, S32 offset, const U32 size) : LLAGPMemBlock(mem_poolp)
{
	mMemp = baseptr + offset;
	mOffset = offset;
	mSize = size;
}


void LLAGPMemBlockAPPLE::bindGLVertexPointer(const U32 stride, const U32 offset)
{
	if (!mMemp)
	{
		llerrs << "Binding empty vertex array" << llendl;
	}
	glVertexPointer(3, GL_FLOAT, stride, mMemp + offset);
}

void LLAGPMemBlockAPPLE::bindGLNormalPointer(const U32 stride, const U32 offset)
{
	if (!mMemp)
	{
		llerrs << "Binding empty normal array" << llendl;
	}
	glNormalPointer(GL_FLOAT, stride, mMemp + offset);
}


void LLAGPMemBlockAPPLE::bindGLColorPointer(const U32 stride, const U32 offset)
{
	if (!mMemp)
	{
		llerrs << "Binding empty color array" << llendl;
	}
	glColorPointer(4, GL_UNSIGNED_BYTE, stride, mMemp + offset);
}


void LLAGPMemBlockAPPLE::bindGLTexCoordPointer(const U32 stride, const U32 offset)
{
	if (!mMemp)
	{
		llerrs << "Binding empty texcoord array" << llendl;
	}
	glTexCoordPointer(2, GL_FLOAT, stride, mMemp + offset);
}


void LLAGPMemBlockAPPLE::bindGLBinormalPointer(const S32 index, const U32 stride, const U32 offset)
{
	if (!mMemp)
	{
		llerrs << "Binding empty vertex weight array" << llendl;
	}

	if (index > 0) glVertexAttribPointerARB(index, 3, GL_FLOAT, FALSE, stride, (F32 *)(mMemp + offset));
}

void LLAGPMemBlockAPPLE::bindGLVertexWeightPointer(const S32 index, const U32 stride, const U32 offset)
{
	if (!mMemp)
	{
		llerrs << "Binding empty vertex weight array" << llendl;
	}

	if (index > 0) glVertexAttribPointerARB(index, 1, GL_FLOAT, FALSE, 0, (F32 *)(mMemp + offset));
}

void LLAGPMemBlockAPPLE::bindGLVertexClothingWeightPointer(const S32 index, const U32 stride, const U32 offset)
{
	if (!mMemp)
	{
		llerrs << "Binding empty vertex weight array" << llendl;
	}
	set_vertex_clothing_weights(index, stride, (LLVector4 *)(mMemp + offset));
}

U8* LLAGPMemBlockAPPLE::getMappedMem()
{
	return mMemp;
}


void LLAGPMemBlockAPPLE::copy(void *mem, const U32 size)
{
	if (!mMemp || !mem)
	{
		return;
	}
	llassert(size <= mSize);

	memcpy( mMemp, mem, size );
	
	glFlushVertexArrayRangeAPPLE(size, mMemp);
}

void LLAGPMemBlockAPPLE::copyColor(void *mem, const U32 size)
{
	if (!mMemp || !mem)
	{
		return;
	}
	llassert(size <= mSize);

	memcpy( mMemp, mem, size );
	
	glFlushVertexArrayRangeAPPLE(size, mMemp);
}



U32  LLAGPMemBlockAPPLE::createFence()
{
	U32 fence;
    glGenFencesAPPLE(1, (GLuint*)&fence);
	glSetFenceAPPLE(fence);
    glFinishFenceAPPLE(fence);
	return fence;
}

void  LLAGPMemBlockAPPLE::deleteFence(const U32 fence)
{
    glDeleteFencesAPPLE(1, (GLuint*)&fence);
}

void LLAGPMemBlockAPPLE::sendFence(U32 fence)
{
    glSetFenceAPPLE(fence);
}

void LLAGPMemBlockAPPLE::waitFence(U32 fence)
{
    if(!glTestFenceAPPLE(fence))
    {
        glFinishFenceAPPLE(fence);
	}
}

// MBW -- May want this at some point...
#if 0
void LLAGPMemBlockAPPLE::flush()
{
	glFlushVertexArrayRangeAPPLE(mSize, mMemp);
}
#endif

#endif // LL_DARWIN
