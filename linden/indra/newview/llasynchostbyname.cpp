/** 
 * @file llasynchostbyname.cpp
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

// standard LL includes
#include "llviewerprecompiledheaders.h"

// self include
#include "llasynchostbyname.h"

//
// Globals
//
LLAsyncHostByName gAsyncHostByName;


//
// Parallel implementations for Windows/UNIX!
//
#if LL_WINDOWS

#include "llviewerwindow.h"
// Place to register the Windows callback
extern void (*gAsyncMsgCallback)(const MSG &msg);


// member functions
LLAsyncHostByName::LLAsyncHostByName() 
	: 
	mRequestHandle(0),
	mCallback(NULL),
	mUserdata(NULL)
{
	gAsyncMsgCallback = LLAsyncHostByName::handleMessageCallback;
	memset(mOutputBuffer, 0, sizeof( mOutputBuffer ) );
}


LLAsyncHostByName::~LLAsyncHostByName() 
{
}


BOOL LLAsyncHostByName::startRequest( const LLString& domain_name, LLAsyncHostByNameCallback callback, void* userdata )
{
	if( isPendingRequest() )
	{
		llwarns << "LLAsyncHostByName::startRequest() cancelled existing request." << llendl;
		cancelPendingRequest();
	}

	mCallback = callback;
	mUserdata = userdata;
	memset(mOutputBuffer, 0, sizeof( mOutputBuffer ) );
	mDomainName = domain_name;

	mRequestHandle = WSAAsyncGetHostByName( 
		(HWND)gViewerWindow->getPlatformWindow(),
		LL_WM_HOST_RESOLVED,
		domain_name.c_str(),
		mOutputBuffer,
		sizeof( mOutputBuffer ) );

	if( !mRequestHandle )
	{
		llwarns << "LLAsyncHostByName::startRequest() failed: " << WSAGetLastError() << llendl;
		return FALSE;
	}

	return TRUE;
}


void LLAsyncHostByName::handleMessage( const MSG& msg )
{
	if( (HANDLE)msg.wParam != mRequestHandle ) 
	{
		llwarns << "LL_WM_HOST_RESOLVED received for request we weren't waiting for.  Ignored." << llendl;
		return;
	}
	llinfos << "LL_WM_HOST_RESOLVED" << llendl;

	BOOL success = FALSE;
	U32 ip = 0;
	S32 error = WSAGETASYNCERROR( msg.lParam );
	if( error )
	{
		if( error == WSANO_DATA) 
		{
			llwarns << "Unknown host" << llendl;
		} 
		else 
		{
			llwarns << "Resolve host error" << WSAGetLastError () << llendl;
		}
	}
	else
	{
		HOSTENT* he = (HOSTENT*) mOutputBuffer;
		char** addr_list = he->h_addr_list;
		if (!addr_list)
		{
			llwarns << "Bad HOSTENT in LLAsyncHostByName" << llendl;
			return;
		}
		char* first_addr = addr_list[0];
		if (!first_addr)
		{
			llwarns << "Bad address in HOSTENT in LLAsyncHostByName" << llendl;
			return;
		}
		ip = *(U32*)first_addr;
		success = TRUE;
	}

	if( mCallback )
	{
		mCallback( success, mDomainName, ip, mUserdata );
	}
	mCallback = NULL;
	mUserdata = NULL;
	mRequestHandle = 0;
	mDomainName.clear();
}


BOOL LLAsyncHostByName::cancelPendingRequest()
{	
	if( mCallback )
	{
		mCallback( FALSE, mDomainName, 0, mUserdata );
	}
	mUserdata = NULL;
	mCallback = NULL;

	if( mRequestHandle )
	{
		S32 ret = WSACancelAsyncRequest( mRequestHandle );
		if( SOCKET_ERROR == ret )
		{
			llwarns << "LLAsyncHostByName::cancelPendingRequest() failed: " << WSAGetLastError() << llendl;
			return FALSE;
		}
		memset(mOutputBuffer, 0, sizeof( mOutputBuffer ) );
		mRequestHandle = 0;
		return TRUE;
	}

	return FALSE;
}


// static
void LLAsyncHostByName::handleMessageCallback(const MSG& msg)
{
	gAsyncHostByName.handleMessage(msg);
}


#else // !LL_WINDOWS


#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>


// member functions
LLAsyncHostByName::LLAsyncHostByName() 
{
}


LLAsyncHostByName::~LLAsyncHostByName() 
{
}


BOOL LLAsyncHostByName::startRequest( const LLString& domain_name, LLAsyncHostByNameCallback callback, void* userdata )
{
	struct hostent *response;
	U32 ip;
	BOOL result = FALSE;
	
	response = gethostbyname(domain_name.c_str());
	
	if(response != NULL)
	{
		if(response->h_addrtype == AF_INET)
		{
			ip = ((struct in_addr*)response->h_addr_list[0])->s_addr;
			result = TRUE;
			(*callback)(result, domain_name, ip, userdata);
		}
	}
	return result;
}


BOOL LLAsyncHostByName::cancelPendingRequest()
{	
	// Since this implementation is synchronous, there's nothing to do here.
	return TRUE;
}
#endif // !LL_WINDOWS
