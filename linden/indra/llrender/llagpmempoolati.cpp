/** 
 * @file llagpmempoolati.cpp
 * @brief LLAGPMemPoolATI base class
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

#include "llagpmempoolati.h"
#include "llgl.h"

#include "llglheaders.h"

LLAGPMemPoolATI::LLAGPMemPoolATI(S32 request) : LLAGPMemPool()
{
	llinfos << "Creating LLAGPMemPoolATI" << llendl;
	stop_glerror();
	if (!gGLManager.mHasATIVAO)
	{
		llerrs << "No ATI AGI memory extension!" << llendl;
	}

	mName = 0;

	// More than 4MB of agp available
	while (!mName && (request > 0))
	{
		mName = glNewObjectBufferATI(request, NULL, GL_DYNAMIC_ATI);
		mSize     = request;
		request >>= 1;
	}

	if (mName)
	{
		mFreeList.append(*(new LLFreeBlock(0, mSize)));
	}
	else
	{
		llinfos << "Unable to allocate AGP memory!" << llendl;
		mSize = 0;
	}
	flush_glerror();
}


LLAGPMemPoolATI::~LLAGPMemPoolATI()
{
	if (mName)
	{
		glFreeObjectBufferATI(mName);
	} 
}


LLAGPMemBlock *LLAGPMemPoolATI::createBlock(const U32 offset, const U32 size)
{
	return new LLAGPMemBlockATI(this, mName, offset, size);
}


void LLAGPMemPoolATI::dump()
{
}


/////////////////////////////
//
// LLAGPMemBlockATI
//
// ATI Implementation of an AGP memory block
//

LLAGPMemBlockATI::LLAGPMemBlockATI(LLAGPMemPool *mem_poolp, const U32 name, const U32 offset, const U32 size) :
	LLAGPMemBlock(mem_poolp)
{
	mName = name;
	mOffset = offset;
	mSize = size;
}


void LLAGPMemBlockATI::bindGLVertexPointer(const U32 stride, const U32 offset)
{
	glArrayObjectATI(GL_VERTEX_ARRAY, 3, GL_FLOAT, stride, mName, mOffset + offset);
}


void LLAGPMemBlockATI::bindGLNormalPointer(const U32 stride, const U32 offset)
{
	glArrayObjectATI(GL_NORMAL_ARRAY, 3, GL_FLOAT, stride, mName, mOffset + offset);
}


void LLAGPMemBlockATI::bindGLColorPointer(const U32 stride, const U32 offset)
{
	glArrayObjectATI(GL_COLOR_ARRAY, 4, GL_UNSIGNED_BYTE, stride, mName, mOffset + offset);
}


void LLAGPMemBlockATI::bindGLTexCoordPointer(const U32 stride, const U32 offset)
{
	glArrayObjectATI(GL_TEXTURE_COORD_ARRAY, 2, GL_FLOAT, stride, mName, mOffset + offset);
}


void LLAGPMemBlockATI::bindGLBinormalPointer(const S32 index, const U32 stride, const U32 offset)
{
	glVertexAttribArrayObjectATI(index, 3, GL_FLOAT, FALSE, stride, mName, mOffset + offset);
}


void LLAGPMemBlockATI::bindGLVertexWeightPointer(const S32 index, const U32 stride, const U32 offset)
{
	//glArrayObjectATI(GL_WEIGHT_ARRAY_ARB, 1, GL_FLOAT, stride, mName, mOffset + offset);
	glVertexAttribArrayObjectATI(index, 1, GL_FLOAT, FALSE, stride, mName, mOffset + offset);
}

void LLAGPMemBlockATI::bindGLVertexClothingWeightPointer(const S32 index, const U32 stride, const U32 offset)
{
	glVertexAttribArrayObjectATI(index, 4, GL_FLOAT, FALSE, stride, mName, mOffset + offset);
}

U8* LLAGPMemBlockATI::getMappedMem()
{
	return NULL;
}

void LLAGPMemBlockATI::copy(void *mem, const U32 size)
{
	llassert(size <= mSize);
	glUpdateObjectBufferATI(mName, mOffset, size, mem, GL_PRESERVE_ATI);
}

void LLAGPMemBlockATI::copyColor(void *mem, const U32 size)
{
	llassert(size <= mSize);
	glUpdateObjectBufferATI(mName, mOffset, size, mem, GL_PRESERVE_ATI);
}

