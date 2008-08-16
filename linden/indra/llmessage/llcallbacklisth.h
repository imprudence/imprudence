/** 
 * @file llcallbacklisth.h
 * @brief Like LLCallbackList but uses handles instead of pointers for
 * user data
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

#ifndef LL_LLCALLBACKLISTH_H
#define LL_LLCALLBACKLISTH_H

#include "doublelinkedlist.h"

typedef void(*LLHandleCallback)(void** handle );

class LLCallbackNodeH;

class LLCallbackListH
{
protected:
	class LLCallbackNodeH
	{
	public:
		LLCallbackNodeH( LLHandleCallback cb, void** h ) : mCallback(cb), mHandle(h) {}
		void	call()					{ (*mCallback)(mHandle); }

	public:
		LLHandleCallback	mCallback;
		void**	mHandle;
	};

public:
	~LLCallbackListH()
		{ mCallbackList.deleteAllData(); }

	void addFunction( LLHandleCallback func, void** handle = NULL ) 		// register a callback, which will be called as func(data)
		{ mCallbackList.addDataAtEnd( new LLCallbackNodeH( func, handle ) ); }

	BOOL containsFunction( LLHandleCallback func, void** handle = NULL )	// true if list already contains the function/data pair
		{
			for( LLCallbackNodeH *p = mCallbackList.getFirstData(); p; p = mCallbackList.getNextData() )
			{
				if( p->mCallback == func && p->mHandle == handle)
				{
					return TRUE;
				}
			}
			return FALSE;
		}

	BOOL deleteFunction( LLHandleCallback func, void** handle = NULL )		// removes the first instance of this function/data pair from the list, false if not found
		{
			for( LLCallbackNodeH *p = mCallbackList.getFirstData(); p; p = mCallbackList.getNextData() )
			{
				if( p->mCallback == func && p->mHandle == handle)
				{
					mCallbackList.deleteCurrentData();
					return TRUE;
				}
			}
			return FALSE;
		}

	void callFunctions()													// calls all functions
		{
			for( LLCallbackNodeH *p = mCallbackList.getFirstData(); p; p = mCallbackList.getNextData() )
			{
				p->call();
			}
		}

	void deleteAllFunctions()
		{ mCallbackList.deleteAllData(); }

protected:
	LLDoubleLinkedList<LLCallbackNodeH>	mCallbackList;
};

#endif  // LL_LLCALLBACKLISTH_H
