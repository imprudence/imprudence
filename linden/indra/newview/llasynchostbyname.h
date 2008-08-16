/** 
 * @file llasynchostbyname.h
 * @brief Wrapper for Windows asychronous DNS lookup functionality
 *
 * Copyright (c) 2003-2007, Linden Research, Inc.
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

#ifndef LL_LLASYNCHOSTBYNAME_H
#define LL_LLASYNCHOSTBYNAME_H

#if LL_WINDOWS
//
// Only implemented on Windows
//

#define LL_WM_HOST_RESOLVED      (WM_APP + 1)

typedef void (*LLAsyncHostByNameCallback)(BOOL success, const LLString& domain_name, U32 ip, void*);

class LLAsyncHostByName
{
public:
	LLAsyncHostByName();
	~LLAsyncHostByName();

	BOOL	startRequest( const LLString& domain_name, LLAsyncHostByNameCallback callback, void* userdata );
	void	handleMessage( const MSG& msg );

	BOOL	isPendingRequest() { return (mRequestHandle != 0); }
	BOOL	cancelPendingRequest();

	static void	handleMessageCallback(const MSG& msg);
private:
	HANDLE	mRequestHandle;
	char	mOutputBuffer[ MAXGETHOSTSTRUCT ];			/*Flawfinder: ignore*/

	LLAsyncHostByNameCallback mCallback;
	void*	mUserdata;
	LLString mDomainName;
	
};

extern LLAsyncHostByName gAsyncHostByName;

#else // LL_WINDOWS

// MBW -- Replicate this object on top of the posix gethostbyname() API.
// For now, it will be synchronous.

#include "llstring.h"

typedef void (*LLAsyncHostByNameCallback)(BOOL success, const LLString& domain_name, U32 ip, void*);

class LLAsyncHostByName
{
public:
	LLAsyncHostByName();
	~LLAsyncHostByName();

	BOOL	startRequest( const LLString& domain_name, LLAsyncHostByNameCallback callback, void* userdata );

	BOOL	isPendingRequest() { return FALSE; }
	BOOL	cancelPendingRequest();

private:
};

extern LLAsyncHostByName gAsyncHostByName;

#endif // !LL_WINDOWS

#endif // LL_LLASYNCHOSTBYNAME_H
