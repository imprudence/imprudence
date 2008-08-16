/** 
 * @file lllogtextmessage.cpp
 * @author Phoenix
 * @date 2005-01-12
 * @brief Impelmentation of the text logger.
 *
 * Copyright (c) 2005-2007, Linden Research, Inc.
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

#include "linden_common.h"
#include "lllogtextmessage.h"

#include "message.h"


LLLogTextMessage::~LLLogTextMessage()
{
	S32 queue_size = (S32)mQueue.size();
	if(queue_size > 0)
	{
		llwarns << "Deleting queus with messages still on it." << llendl;
	}
}

void LLLogTextMessage::log(
	const LLUUID& from,
	const LLUUID& to,
	const char* mesg)
{
	const F64 F64_ZERO = 0.0;
	log(from, to, F64_ZERO, F64_ZERO, mesg);
}

void LLLogTextMessage::log(
	const LLUUID& from,
	F64 global_x,
	F64 global_y,
	const char* mesg)
{
	log(from, LLUUID::null, global_x, global_y, mesg);
}

void LLLogTextMessage::log(
	const LLUUID& from,
	const LLUUID& to,
	F64 global_x,
	F64 global_y,
	const char* mesg)
{
	const S32 OVERHEAD = (2*sizeof(LLUUID))+(2*sizeof(F64))+(sizeof(S32));
	const U32 MAX_LOGS_PER_MSG = 100;
	const U32 LOG_MTU = 4000;
	LLLogTextMessageData data(from, to, global_x, global_y, mesg);
	mQueue.push_back(data);
	mPendingSize += (S32)(OVERHEAD + data.mMessage.size());
	if((mQueue.size() >= MAX_LOGS_PER_MSG) || (mPendingSize > (S32)LOG_MTU))
	{
		flush();
	}
}

void LLLogTextMessage::flush()
{
	mPendingSize = 0;
	if(mQueue.empty()) return;
	LLMessageSystem* msg = gMessageSystem;
	msg->newMessageFast(_PREHASH_LogTextMessage);
	while(!mQueue.empty())
	{
		LLLogTextMessageData data(mQueue.front());
		mQueue.pop_front();
		msg->nextBlockFast(_PREHASH_DataBlock);
		msg->addUUIDFast(_PREHASH_FromAgentId, data.mFromID);
		msg->addUUIDFast(_PREHASH_ToAgentId, data.mToID);
		msg->addF64Fast(_PREHASH_GlobalX, data.mGlobalX);
		msg->addF64Fast(_PREHASH_GlobalY, data.mGlobalY);
		msg->addU32Fast(_PREHASH_Time, data.mTime);
		msg->addStringFast(_PREHASH_Message, data.mMessage.c_str());
	}

	// Try to make this reliable, but don't try too hard
	msg->sendReliable(mDataserver, 3, TRUE, 0.f, NULL, NULL);
}

LLLogTextMessage::LLLogTextMessageData::LLLogTextMessageData(
	const LLUUID& from,
	const LLUUID& to,
	const F64& global_x,
	const F64& gloabl_y,
	const char* message) :
	mFromID(from),
	mToID(to),
	mGlobalX(global_x),
	mGlobalY(gloabl_y),
	mMessage(message)
{
	mTime = (S32)time(NULL);
}
