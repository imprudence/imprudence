/** 
 * @file llagparray.h
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

#ifndef LL_LLAGPARRAY_H
#define LL_LLAGPARRAY_H

#include <stdlib.h>
#include "llagpmempool.h"

class LLAGPMemBlock;

template <class Type> class LLAGPArray
{
public:
	enum
	{
		OKAY = 0,
		FAIL = -1
	};

	inline LLAGPArray(const S32 size=0, const U32 target=0);
	inline ~LLAGPArray();

	inline void init();
	inline void destroy();

	inline void flushAGP();
	inline void setDirty()							{ mDirty = TRUE; }
	inline BOOL isDirty() const						{ return mDirty; }

	inline BOOL setUseAGP(const BOOL on = TRUE);	// Returns false if AGP memory is not available
	inline BOOL isAGP() const						{ return mAGPp ? TRUE : FALSE; }
	LLAGPMemBlock *getAGPMemBlock() const			{ return mAGPp; }

	inline BOOL realloc(U32 newsize);
	inline S32  getMax() const						{ return mMaxObj; }
	inline S32  count() const;
	inline S32  shrinkTo(S32 newcount);
	inline void reset(S32 reserve_count);
	
	inline S32 find      (const Type &obj) const;
	inline S32 fastRemove(const S32 i);
	inline S32 safeRemove(const S32 r);
	inline S32 remove    (const S32 i);
	inline S32 removeObj (const Type &obj);
	inline S32 removeLast()							{ return (mNumObj > 0 ? mNumObj-- : 0); }

	inline Type* reserve_block(const U32 num);

	inline S32         put(const Type &obj);
	inline const Type& get(const S32 i) const;
	inline Type&       get(const S32 i);

	inline BOOL        sync     ();
	inline BOOL        syncColor();

	inline const Type* getMem  () const;
	inline const S32   getIndex(const Type *objp) const { return objp - mMemp; }
	inline const Type& operator[](const S32 i) const;
	inline Type&       operator[](const S32 i);

	void bindGLVertexPointer(const U32 stride, const U32 offset);
	void bindGLTexCoordPointer(const U32 stride, const U32 offset);
	void bindGLNormalPointer(const U32 stride, const U32 offset);
	void bindGLBinormalPointer(const S32 index, const U32 stride, const U32 offset);
	void bindGLColorPointer(const U32 stride, const U32 offset);
	void bindGLVertexWeightPointer(const S32 index, const U32 stride, const U32 offset);
	void bindGLVertexClothingWeightPointer(const S32 index, const U32 stride, const U32 offset);

	void copyToMem(const S32 offset, const U8 *source, const S32 size);

	U8*  getScratchMemory();

	U32  createFence()        { return (mAGPp ? mAGPp->createFence() : 0); }
	void deleteFence(const U32 fence) { if (mAGPp) { mAGPp->deleteFence(fence); } }
	void sendFence(U32 fence) { if (mAGPp) mAGPp->sendFence(fence); }
	void waitFence(U32 fence) { if (mAGPp) mAGPp->waitFence(fence); }

	S32			getSysMemUsage() { return sizeof(Type)*mMaxObj; }

protected:
	BOOL		mDirty;
	BOOL		mUseAGP;
	U32			mNumObj;
	U32			mMaxObj;
	U8			*mUnalignedMemp;
	Type		*mMemp;
	U32			mTarget;	//for VBO implementations of this class, store the type of buffer this is

	LLAGPMemBlock *mAGPp;

public:
	BOOL       mSynced; // for graph only -- not critical
	
public:
	static S32 sNumRealloced;
	static S32 sBytesRealloced;
	static char* sTypeName;
};

template <class Type> S32 LLAGPArray<Type>::sNumRealloced = 0;
template <class Type> S32 LLAGPArray<Type>::sBytesRealloced = 0;

// constructor is in .inl file because it calls realloc()
// which access gPipeline (yuck!)

template <class Type>
LLAGPArray<Type>::~LLAGPArray()
{
	destroy();
}

template <class Type> void
LLAGPArray<Type>::destroy()
{
	realloc(0);
	delete mAGPp;
	mAGPp = NULL;
}

template <class Type>
const Type *LLAGPArray<Type>::getMem() const
{
	return mMemp;
}

template <class Type>
const Type& LLAGPArray<Type>::get(const S32 i) const
{
	llassert(i < (S32)mNumObj);
	return mMemp[i];
}

template <class Type>
Type& LLAGPArray<Type>::get(const S32 i)
{
	setDirty();
	llassert(i < (S32)mNumObj);
	return mMemp[i];
}

template <class Type>
const Type& LLAGPArray<Type>::operator[](const S32 i) const
{ 
	return get(i); 
}

template <class Type>
Type& LLAGPArray<Type>::operator[](const S32 i)
{
	return get(i); 
}

template <class Type>
S32 LLAGPArray<Type>::fastRemove(const S32 i)
{
	if (i < 0 || mNumObj <= 0) return FAIL;
	setDirty();
	mMemp[i] = mMemp[--mNumObj];

	return OKAY;
}

template <class Type>
S32 LLAGPArray<Type>::remove(const S32 i)
{
	setDirty();
	return fastRemove(i);
}

template <class Type>
S32 LLAGPArray<Type>::removeObj(const Type& obj)
{
	setDirty();

	S32 ind = find(obj);
	if (ind >= 0)
	{
		return fastRemove(ind);
	}
	return FAIL;
}

template <class Type>
S32	LLAGPArray<Type>::count() const
{
	return mNumObj;
}


#endif // LL_LLAGPARRAY_H
