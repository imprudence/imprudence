/** 
 * @file llpacketack.h
 * @brief Reliable UDP helpers for the message system.
 *
 * Copyright (c) 2001-2007, Linden Research, Inc.
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

#ifndef LL_LLPACKETACK_H
#define LL_LLPACKETACK_H

#include <cstring>
#include <stdio.h>

#include "llerror.h"
#include "lltimer.h"
#include "llhost.h"

//class LLPacketAck
//{
//public:
//	LLHost mHost;
//	TPACKETID          mPacketID;
//public:
//	LLPacketAck(const LLHost &host, TPACKETID packet_id)
//		{
//			mHost = host;
//			mPacketID = packet_id;
//		};
//	~LLPacketAck(){};
//};

class LLReliablePacketParams
{
public:
	LLHost	mHost;
	S32				mRetries;
	BOOL			mPingBasedRetry;
	F32				mTimeout;
	void			(*mCallback)(void **,S32);
	void			**mCallbackData;
	char			*mMessageName;

public:
	LLReliablePacketParams()
	{
		clear();
	};

	~LLReliablePacketParams() { };

	void clear()
	{
		mHost.invalidate();
		mRetries = 0;
		mPingBasedRetry = TRUE;
		mTimeout = 0.f;
		mCallback = NULL;
		mCallbackData = NULL;
		mMessageName = NULL;
	};

	void set (	const LLHost &host, S32 retries, BOOL ping_based_retry,
				F32 timeout, 
				void (*callback)(void **,S32), void **callback_data, char *name )
	{
		mHost = host;
		mRetries = retries;
		mPingBasedRetry = ping_based_retry;
		mTimeout = timeout;
		mCallback = callback;
		mCallbackData = callback_data;
		mMessageName = name;
	};
};

class LLReliablePacket
{
public:
	LLReliablePacket(S32 socket, U8 *buf_ptr, S32 buf_len, LLReliablePacketParams *params) :
		mBuffer(NULL),
		mBufferLength(0)
	{
		if (params)
		{
			mHost = params->mHost;
			mRetries = params->mRetries;
			mPingBasedRetry = params->mPingBasedRetry;
			mTimeout = params->mTimeout;
			mCallback = params->mCallback;
			mCallbackData = params->mCallbackData;
			mMessageName = params->mMessageName;
		}
		else
		{
			mRetries = 0;
			mPingBasedRetry = TRUE;
			mTimeout = 0.f;
			mCallback = NULL;
			mCallbackData = NULL;
			mMessageName = NULL;
		}

		mExpirationTime = (F64)((S64)totalTime())/1000000.0 + mTimeout;
		mPacketID = buf_ptr[1] + ((buf_ptr[0] & 0x0f ) * 256);
		if (sizeof(TPACKETID) == 4)
		{
			mPacketID *= 256;
			mPacketID += buf_ptr[2];
			mPacketID *= 256;
			mPacketID += buf_ptr[3];
		}

		mSocket = socket;
		if (mRetries)
		{
			mBuffer = new U8[buf_len];
			if (mBuffer != NULL)
			{
				memcpy(mBuffer,buf_ptr,buf_len);	/*Flawfinder: ignore*/
				mBufferLength = buf_len;
			}
			
		}
	};

	~LLReliablePacket()
	{ 
		mCallback = NULL;
		delete [] mBuffer;
		mBuffer = NULL;
	};

	friend class LLCircuitData;
protected:
	S32				mSocket;
	LLHost	        mHost;
	S32				mRetries;
	BOOL			mPingBasedRetry;
	F32				mTimeout;
	void			(*mCallback)(void **,S32);
	void			**mCallbackData;
	char			*mMessageName;
	
	U8				*mBuffer;
	S32				mBufferLength;

	TPACKETID				mPacketID;

	F64				mExpirationTime;
	
};

#endif

