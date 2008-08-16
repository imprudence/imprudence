/** 
 * @file llagparray.inl
 * @brief LLAGPArray - arrays used for rendering w/ AGP memory (if on)
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

#include "llagpmempool.h"
#include "llglheaders.h"
#include "pipeline.h" // for gPipeline
#include "v4math.h"

template <class Type>
void LLAGPArray<Type>::flushAGP()
{
	delete mAGPp;
	mAGPp = NULL;

	setDirty();
}

template <class Type>
void LLAGPArray<Type>::reset(S32 reserve_count)	   
{
	mNumObj = 0;
	realloc(reserve_count);
	setDirty();
}

template <class Type>
S32 LLAGPArray<Type>::find(const Type &obj) const
{
	for ( S32 i = 0; i < (S32)mNumObj; i++ )
	{
		if (mMemp[i] == obj)
		{
			return i;
		}
	}
	return FAIL;
}

template <class Type>
Type* LLAGPArray<Type>::reserve_block(const U32 num)
{
	if (!num) 
	{
		return NULL;
	}

	setDirty();

	U32 new_count = mNumObj + num;
	if (new_count >= mMaxObj)
	{
		if (!realloc(new_count))
		{
			llwarns << "Unable to realloc " << num << " objects in LLAGPArray!" << llendl;
			return NULL;
		}
	}

	if (mMemp)
	{
		Type *ret   = &mMemp[mNumObj];
		mNumObj    += num;
		return ret;
	}

	return NULL;
}

template <class Type>
S32	LLAGPArray<Type>::put(const Type &obj) 
{ 
	if (mNumObj >= mMaxObj)
	{
		if (!mMaxObj)
		{
			mMaxObj = 1;
		}
		if (!realloc(mMaxObj*2))
		{
			llwarns << "Unable to put in AGPArray due to realloc!" << llendl;
			return FAIL;
		}
	}

	mMemp[mNumObj] = obj;
	mNumObj++;
	setDirty();
	return OKAY;
}

template <class Type>
S32	LLAGPArray<Type>::shrinkTo(S32 newcount)
{
	mNumObj = newcount;
	return mNumObj;
}

template <class Type>
LLAGPArray<Type>::LLAGPArray(const S32 size, const U32 target) 
{
	if (!target)
	{
		mTarget = GL_ARRAY_BUFFER_ARB;
	}
	else
	{
		mTarget = target;
	}

	mSynced = FALSE;
	mMaxObj = 0; 
	mNumObj = 0; 
	mMemp = NULL;
	mUnalignedMemp = NULL;
	mUseAGP = FALSE;
	mAGPp = NULL;
	mDirty = TRUE;

	if (size)
	{
		if (!realloc(size))
		{
			llerrs << "Unable to allocate AGP array of size: " << size << llendl;
		}
	}
}

template <class Type>
BOOL LLAGPArray<Type>::realloc(U32 new_count)
{
	const U32 min_count = 1024;
	U32 max_count = llmax(min_count, mMaxObj);
	if (new_count == 0)
	{
		delete[] mUnalignedMemp;
		mUnalignedMemp = NULL;
		mMemp = NULL;

		delete mAGPp;
		mAGPp  = NULL;

		mNumObj  = 0;
		mMaxObj  = 0;

		return TRUE;
	}
	if (new_count < min_count)
	{
		new_count = min_count;
	}
	else if (new_count < max_count>>1)
	{
		new_count = new_count + (new_count>>1);
	}
	else
	{
		while (max_count < new_count)
		{
			max_count += max_count>>1;
		}
		new_count = max_count;
	}
	if (new_count != mMaxObj)
	{
		//Type *new_obj_array = new Type[new_count];
		const size_t ALIGN_BYTES = 64;
		U8* new_unaligned_obj_array = new U8[ sizeof(Type) * new_count + ALIGN_BYTES - 1];
		if (!new_unaligned_obj_array)
		{
			llerrs << "Out of system memory in LLAGPArray::realloc - tried to alloc " << new_count << llendl;
			return FALSE;
		}

		Type* new_obj_array;
		// Use size_t to avoid warning -- size_t should be the same size as a pointer
		// for 32 and 64 bit architectures. JC
		size_t offset = (size_t)new_unaligned_obj_array;
		S32 skip_bytes = (S32)(offset % ALIGN_BYTES);
		if( skip_bytes )
		{
			new_obj_array = (Type*) (new_unaligned_obj_array + ALIGN_BYTES - skip_bytes);
		}
		else
		{
			new_obj_array = (Type*) new_unaligned_obj_array;
		}

		delete mAGPp;
		mAGPp = NULL;

		if (mUseAGP)
		{
			mAGPp = gPipeline.allocAGPFromPool(new_count * sizeof(Type), mTarget);
			setDirty();
		}

		if (!mAGPp && mUseAGP)
		{
#ifdef DEBUG_AGP
			llwarns << "Failed AGP allocation in realloc!" << llendl;
#endif
		}

		// Copying data from previous array to new array.
		if (new_count < mNumObj)
		{
			mNumObj = new_count;
		}

		if (mNumObj > 0  &&  mMemp)
		{
			memcpy(new_obj_array, mMemp, mNumObj*sizeof(Type));
		}
		
		delete[] mUnalignedMemp;
		mUnalignedMemp = new_unaligned_obj_array;
		mMemp = new_obj_array;

		mMaxObj = new_count;
	}

	return TRUE;
}

template <class Type>
void LLAGPArray<Type>::copyToMem(const S32 offset, const U8 *source, const S32 size)
{
	llassert(mMemp);
	memcpy((mMemp + offset), source, size*sizeof(Type)); 
}

template <class Type>
BOOL LLAGPArray<Type>::sync()
{
	if (mUseAGP && !mAGPp && !mDirty)
	{
		llwarns << "No AGP, yet not dirty!" << llendl;
	}

	if (mUseAGP && mDirty && mNumObj)
	{
		if (!mAGPp)
		{
			mAGPp = gPipeline.allocAGPFromPool(mMaxObj * sizeof(Type), mTarget);
			if (!mAGPp)
			{
#ifdef DEBUG_AGP
				llwarns << "Failure to allocate AGP in sync!" << llendl;
#endif
				mUseAGP = FALSE;
			}
			else
			{
				mAGPp->copy(mMemp, mNumObj * sizeof(Type));
			}
		}
		else
		{
			mAGPp->copy(mMemp, mNumObj * sizeof(Type));
		}
		mSynced = TRUE;
	}
	mDirty = FALSE;
	return mSynced;
}

template <class Type>
BOOL LLAGPArray<Type>::syncColor()
{
	if (mUseAGP && mDirty && mNumObj)
	{
		if (!mAGPp)
		{
			mAGPp = gPipeline.allocAGPFromPool(mMaxObj * sizeof(Type), mTarget);
			if (!mAGPp)
			{
#ifdef DEBUG_AGP
				llwarns << "Failure to allocate AGP in synccolor!" << llendl;
#endif
				mUseAGP = FALSE;
			}
			else
			{
				mAGPp->copyColor(mMemp, mNumObj * sizeof(Type));
			}
		}
		else
		{
			mAGPp->copyColor(mMemp, mNumObj * sizeof(Type));
		}
		mSynced = TRUE;
	}
	mDirty = FALSE;
	return mSynced;
}

template <class Type>
BOOL LLAGPArray<Type>::setUseAGP(const BOOL on)
{
	if (on == mUseAGP)
	{
		return TRUE;
	}

	mUseAGP = on;
	if (!on)
	{
		delete mAGPp;
		mAGPp = NULL;
	}
	else if (on)
	{
		mAGPp = gPipeline.allocAGPFromPool(mMaxObj * sizeof(Type), mTarget);
		setDirty();
		if (!mAGPp)
		{ 
#ifdef DEBUG_AGP
			llwarns << "Failure to allocate AGP in setUseAGP! Block size = " << (mMaxObj * sizeof(Type)) << llendl;
#endif
			mUseAGP = FALSE;
			return FALSE;
		}
	}
	return TRUE;
}

extern U8* gAGPVertices;
extern U8* gAGPNormals;

template <class Type>
U8* LLAGPArray<Type>::getScratchMemory()
{
	U8* memp = NULL;

	if (mAGPp && gPipeline.isAGPBound())
	{
		 memp = mAGPp->getMappedMem();
	}
	if (!memp)
	{
		memp = ((U8*)mMemp);
	}

	return memp;
}

template <class Type>
void LLAGPArray<Type>::bindGLVertexPointer(const U32 stride, const U32 offset)
{
	if (mAGPp && gPipeline.isAGPBound())
	{
		mAGPp->bindGLVertexPointer(stride, offset);
	}
	else
	{
		if (gPipeline.isAGPBound())
		{
			llerrs << "Binding non-AGP vertex pointer when AGP enabled" << llendl;
		}
		if (!mMemp)
		{
			llerrs << "Binding empty vertex array" << llendl;
		}
		llassert(mMemp);
		glVertexPointer(3, GL_FLOAT, stride, ((U8 *)mMemp) + offset);
	}
}

template <class Type>
void LLAGPArray<Type>::bindGLTexCoordPointer(const U32 stride, const U32 offset)
{
	if (mAGPp && gPipeline.isAGPBound())
	{
		mAGPp->bindGLTexCoordPointer(stride, offset);
	}
	else
	{
		if (gPipeline.isAGPBound())
		{
			llwarns << "Binding non-AGP texture coords pointer when AGP enabled" << llendl;
		}
		if (!mMemp)
		{
			llerrs << "Binding empty tex coord array" << llendl;
		}
		glTexCoordPointer(2, GL_FLOAT, stride, ((U8 *)mMemp) + offset);
	}
}

template <class Type>
void LLAGPArray<Type>::bindGLNormalPointer(const U32 stride, const U32 offset)
{
	if (mAGPp && gPipeline.isAGPBound())
	{
		mAGPp->bindGLNormalPointer(stride, offset);
	}
	else
	{
		if (gPipeline.isAGPBound())
		{
			llwarns << "Binding non-AGP normals pointer when AGP enabled" << llendl;
		}
		if (!mMemp)
		{
			llerrs << "Binding empty normal array" << llendl;
		}
		glNormalPointer(GL_FLOAT, stride, ((U8 *)mMemp) + offset);
	}
}

template <class Type>
void LLAGPArray<Type>::bindGLBinormalPointer(const S32 index, const U32 stride, const U32 offset)
{
	if (mAGPp && gPipeline.isAGPBound())
	{
		mAGPp->bindGLBinormalPointer(index, stride, offset);
	}
	else
	{
		if (gPipeline.isAGPBound())
		{
			llwarns << "Binding non-AGP binormal pointer when AGP enabled" << llendl;
		}
		if (!mMemp)
		{
			llerrs << "Binding empty binormal array" << llendl;
		}
		set_binormals(index, stride, (LLVector3 *)(((U8 *)mMemp) + offset));
	}
}


template <class Type>
void LLAGPArray<Type>::bindGLColorPointer(const U32 stride, const U32 offset)
{
	if (mAGPp && gPipeline.isAGPBound())
	{
		mAGPp->bindGLColorPointer(stride, offset);
	}
	else
	{
		if (gPipeline.isAGPBound())
		{
			llwarns << "Binding non-AGP color pointer when AGP enabled" << llendl;
		}
		if (!mMemp)
		{
			llerrs << "Binding empty color array" << llendl;
		}
		glColorPointer(4, GL_UNSIGNED_BYTE, stride, ((U8 *)mMemp) + offset);
	}
}

template <class Type>
void LLAGPArray<Type>::bindGLVertexWeightPointer(const S32 index, const U32 stride, const U32 offset)
{
	if (mAGPp && gPipeline.isAGPBound())
	{
		mAGPp->bindGLVertexWeightPointer(index, stride, offset);
	}
	else
	{
		if (gPipeline.isAGPBound())
		{
			llwarns << "Binding non-AGP vertex weight pointer when AGP enabled" << llendl;
		}
		if (!mMemp)
		{
			llerrs << "Binding empty vertex weight array" << llendl;
		}
		set_vertex_weights(index, (F32 *)(((U8 *)mMemp) + offset));
	}
}

template <class Type>
void LLAGPArray<Type>::bindGLVertexClothingWeightPointer(const S32 index, const U32 stride, const U32 offset)
{
	if (mAGPp && gPipeline.isAGPBound())
	{
		mAGPp->bindGLVertexClothingWeightPointer(index, stride, offset);
	}
	else
	{
		if (gPipeline.isAGPBound())
		{
			llwarns << "Binding non-AGP vertex weight pointer when AGP enabled" << llendl;
		}
		if (!mMemp)
		{
			llerrs << "Binding empty vertex weight array" << llendl;
		}
		set_vertex_clothing_weights(index, stride, (LLVector4 *)(((U8 *)mMemp) + offset));
	}
}

