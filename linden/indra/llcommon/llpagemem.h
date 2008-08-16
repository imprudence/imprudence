/** 
 * @file llpagemem.h
 *
 * Copyright (c) 2002-2007, Linden Research, Inc.
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
#ifndef LL_LLPAGEMEM_H
#define LL_LLPAGEMEM_H

#if !LL_DARWIN
#include "malloc.h"
#endif

#include "llrand.h"


#ifndef __GNUC__
template <class Object, U32 mPageSize=1024> 
class LLPageMemory 
{
private:

	union XObject
	{
		XObject *mNextFreeObject;
		U8		 mObject[sizeof(Object)];
	};

	template <U32 mPageSize> struct Page
	{
		XObject *mFirstFreeObject;
		U32      mObjectCount;
		U8       mObjects[mPageSize-8];

		void init(U32 mObjPerPage)
		{
			XObject *o = (XObject*)&mObjects;
			mObjectCount     = 0;
			mFirstFreeObject = o;
        
			for (U32 i = 0; i < mObjPerPage-1; i++) 
			{
				o->mNextFreeObject = o+1;
				o++;
			}
			o->mNextFreeObject = NULL;
		};

		Object* alloc()
		{
			if (mFirstFreeObject)
			{
				XObject     *ret = mFirstFreeObject;
				mFirstFreeObject = mFirstFreeObject->mNextFreeObject;
				ret->mNextFreeObject = NULL;
				mObjectCount++;
				return (Object*)&ret->mObject;
			};
			return NULL;
		}
	};

	U32     mObjPerPage;
	U32     mMaxPages;
	U32     mObjectCount;

	Page<mPageSize>*   mPageMemory;
	Page<mPageSize>*   mFirstPage;

public:
	U32 precise, anywhere, fail;

	void    init()
	{
		Page<mPageSize> *p = mFirstPage;
		for (U32 i = 0; i < mMaxPages; i++) 
		{
			p[i].init(mObjPerPage);
		}

		precise  = 0;
		anywhere = 0;
		fail     = 0;
	};

public:

	Page<mPageSize>*  pageof(Object *object)
	{
		return (Page<mPageSize>*) ((((U32)object - 8) / mPageSize) * mPageSize );
	}

	LLPageMemory(U32 maxObjects)
	{
		mObjPerPage = (mPageSize-8) / sizeof(Object);
		mMaxPages   = (maxObjects / mObjPerPage) + 1;
		mObjectCount= 0;

		//printf("%d objects per page, %d pages total, %d/%d objects\n",
		//	mObjPerPage, mMaxPages, mMaxPages * mObjPerPage,maxObjects);

		mPageMemory = (Page<mPageSize>*)calloc(mPageSize,mMaxPages+1);
		mFirstPage = mPageMemory;
		U32 fix = ((U32)mPageMemory % mPageSize);
		if (fix) mFirstPage = (Page<mPageSize>*)((U32)mPageMemory+mPageSize-fix);
        
		//printf("fix = %d\n",fix);

		init();
	};

	LLPageMemory(void *pageMem, U32 bytes)
	{
	}

	~LLPageMemory()
	{
		if (mPageMemory)
		{
			free(mPageMemory);
		}
	}
	
	Object*     alloc(Object *after=NULL)
	{
		Page<mPageSize> * p = mFirstPage;
		Object          * o = NULL;
		U32               i = mMaxPages;

		if (after)
		{
			o = pageof(after)->alloc();
			if (o) 
			{
				precise++;
				return o;
			}
			return NULL;
		}

		F32 frac   = 1.0f / (F32)mMaxPages;		
		F32 thresh = 0.0f;
		for (i = 0; i < mMaxPages; i++)
		{
			if (thresh > ((F32)p[i].mObjectCount / (F32)mObjPerPage))
			{
				o = p[i].alloc();
				if (o) 
				{
					//printf("allocating page %x obj %x\n",p, o);
					anywhere++;
					return o;
				}
			}
			thresh += frac;
		} 

		fail++;
		return NULL;
	};

	void        free(Object *o)
	{
		if (!o) return;

		Page<mPageSize> *page = pageof(o);
		XObject *obj = (XObject*)o;

		//printf("freeing %x\n",obj);

		obj->mNextFreeObject    = page->mFirstFreeObject;
		page->mFirstFreeObject  = obj;
		page->mObjectCount--;

		//printf("freeing page %x %d\n",page, page->mObjectCount);
	};

	U32  indexof(Object *object)
	{
		if (!object) return -1;

		return ((((U32)object-8) % mPageSize) / sizeof(Object)) + 
			   ((pageof(object) - mFirstPage) * mObjPerPage);
	}


	void dump()
	{

		for (U32 i=0;i < mMaxPages;i++)
		{
			//printf("page %d   %d/%d objects\n",i,mFirstPage[i].mObjectCount,mObjPerPage);
		}
		//printf("precise = %d anywhere %d ratio = %f\n",precise,anywhere,(F32)precise/(F32)(anywhere+precise));
		//printf("fail = %d\n",fail);

	}

	static void test()
	{
		struct Foo
		{
			U32 ord;
			U32 foo[8];
		};

		const U32 count = 100;
		U32 i,c;
		U32 mix = 50;

		LLPageMemory<Foo> mem(count);
		LLRand rand(LLUUID::getRandomSeed());

		Foo *obj[count];

		for (i=0;i<count;i++) obj[i] = 0;

		U32 x= 0;
		for (U32 run=0; run < 10000 ;run++)
		{
			U32 m =rand.llrand(count);
			for (c=0;c < m;c++)
			{
				U32 j = rand.llrand(count);
				if (obj[j]) 
				{
					mem.free(obj[j]);
					obj[j] = 0;
				}
			}

			m =rand.llrand(count);
			for (c=0;c<m;c++)
			{
				U32 i = rand.llrand(count);
				while (obj[i] && i < count) i++;

				if (!obj[i]) 
				{
					if (i > 0) obj[i] = mem.alloc(obj[i-1]);
					else       obj[i] = mem.alloc();
					if (obj[i]) obj[i]->ord = x++;
				}
			}
		}

		for (i = 0; i< count; i++)
		{
			//printf("obj[%2d] = %08x %02d %4d\n",i,obj[i],mem.indexof(obj[i]),(obj[i] ? obj[i]->ord : -1));
		}

		mem.dump();
	}
};


template <class Object> 
class LLObjectPool
{
private:

	class XObject
	{
	public:
		Object   mObject;
		U32      mNextFreeObject;

		XObject() { mObject = NULL; mNextFreeObject = 0; }
	};

	U32      mNumObjects;
	U32		 mMaxObjects;
	XObject* mObjects;
	U32      mFirstFreeObject;

	U32 indexof(XObject *xobj) { return (xobj - mObjects); }

public:
	LLObjectPool(U32 maxObjects)
	{
		mMaxObjects = maxObjects;
		mFirstFreeObject = 0;

		mObjects = new XObject[mMaxObjects];

		//printf("objectpool allocated %d bytes\n",sizeof(XObject) * mMaxObjects);

		for (U32 i=0;i<mMaxObjects;i++) mObjects[i].mNextFreeObject = i+1;
	};

	~LLObjectPool()
	{
		delete [] mObjects;
		mObjects = NULL;
	}

	Object*     alloc(Object *after=NULL)
	{
		XObject *obj     = &mObjects[mFirstFreeObject];
		mFirstFreeObject = obj->mNextFreeObject;
		if (mFirstFreeObject >= mMaxObjects)
		{
			llerrs << "Attempted to allocate too many objects out of pool\n" << llendl;
			llassert(0);
		}
		return &obj->mObject;
	};

	void        free(Object *obj)
	{
		if (!obj) return;
		XObject *xobj         = (XObject*)obj;
		xobj->mNextFreeObject = mFirstFreeObject;
		mFirstFreeObject      = indexof(xobj);
	};

	static void test()
	{
		struct Foo
		{
			U32 ord;
			U32 foo[8];
		};

		const U32 count = 100;
		U32 i,c;
		U32 mix = 50;

		LLPageMemory<Foo> mem(count);
		LLRand rand(LLUUID::getRandomSeed());

		Foo *obj[count];

		for (i=0;i<count;i++) obj[i] = 0;

		U32 x= 0;
		for (U32 run=0; run < 10000 ;run++)
		{
			U32 m =rand.llrand(count);
			for (c=0;c < m;c++)
			{
				U32 j = rand.llrand(count);
				if (obj[j]) 
				{
					mem.free(obj[j]);
					obj[j] = 0;
				}
			}

			m =rand.llrand(count);
			for (c=0;c<m;c++)
			{
				U32 i = rand.llrand(count);
				while (obj[i] && i < count) i++;

				if (!obj[i]) 
				{
					if (i > 0) obj[i] = mem.alloc(obj[i-1]);
					else       obj[i] = mem.alloc();
					if (obj[i]) obj[i]->ord = x++;
				}
			}
		}

		for (i = 0; i< count; i++)
		{
			//printf("obj[%2d] = %08x %02d %4d\n",i,obj[i],mem.indexof(obj[i]),(obj[i] ? obj[i]->ord : -1));
		}
	}
};
#endif // !defined __GNUC__

#endif
