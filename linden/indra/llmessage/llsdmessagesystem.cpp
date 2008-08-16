/** 
 * @file llsdmessagesystem.cpp
 * @brief LLSDMessageSystem class implementation
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
#include "llsdmessagesystem.h"

#include "llhttpnode.h"
#include "llsdutil.h"

LLSDMessageSystem::LLSDMessageSystem() : mMsg(* new LLMessageSystem())
{
}

LLSDMessageSystem::LLSDMessageSystem(LLMessageSystem& delegateSystem) : mMsg(delegateSystem)
{
}

LLSDMessageSystem::~LLSDMessageSystem()
{
}

bool LLSDMessageSystem::handleMessage(std::string name, const LLSD& message)
{
	setInput(message);
	return mMsg.callHandler(name.c_str(), false, this);
}

void LLSDMessageSystem::setInput(const LLSD& input)
{
	mInput = input;
}

LLSD getLLSD(const LLSD& input, const char* block, const char* var, S32 blocknum)
{
	if(input[block].isArray())
	{
		return input[block][blocknum][var];
	}
	return input[block][var];
}

//virtual 
void LLSDMessageSystem::getU32Fast(const char *block, const char *var, U32 &data, S32 blocknum)
{
	data = ll_U32_from_sd(getLLSD(mInput, block, var, blocknum));
}

//virtual 
void LLSDMessageSystem::getUUIDFast(const char *block, const char *var, LLUUID &uuid, S32 blocknum)
{
	uuid = getLLSD(mInput, block, var, blocknum).asUUID();
}

//virtual 
void LLSDMessageSystem::getIPAddrFast(const char *block, const char *var, U32 &ip, S32 blocknum)
{
	ip = ll_ipaddr_from_sd(getLLSD(mInput, block, var, blocknum));
}

//virtual 
void LLSDMessageSystem::getIPPortFast(const char *block, const char *var, U16 &port, S32 blocknum)
{
	port = getLLSD(mInput, block, var, blocknum).asInteger();
}

//virtual 
void LLSDMessageSystem::getU64Fast(const char *block, const char *var, U64 &data, S32 blocknum)
{
	data = ll_U64_from_sd(getLLSD(mInput, block, var, blocknum));
}

//virtual 
void LLSDMessageSystem::getStringFast(const char *block, const char *var, S32 buffer_size, char *buffer, S32 blocknum)
{
	std::string data = getLLSD(mInput, block, var, blocknum).asString();
	S32 length = data.length();
	memset(buffer, 0, buffer_size);
	strncpy(buffer, data.c_str(), llmin(length, buffer_size));	/* Flawfinder: ignore */
}

//virtual 
void LLSDMessageSystem::newMessageFast(const char *name)
{
	mMsg.newMessageFast(name);
}

//virtual 
void LLSDMessageSystem::nextBlockFast(const char *blockname)
{
	mMsg.nextBlockFast(blockname);
}

//virtual 
void LLSDMessageSystem::addU32Fast(const char *varname, U32 u)
{
	mMsg.addU32Fast(varname, u);
}

//virtual 
void LLSDMessageSystem::addUUIDFast( const char *varname, const LLUUID& uuid)
{
	mMsg.addUUIDFast(varname, uuid);
}

//virtual 
S32 LLSDMessageSystem::sendReliable(const LLHost &host)
{
	return mMsg.sendReliable(host);
}

//virtual
U32 LLSDMessageSystem::getOurCircuitCode()
{
	return mMsg.getOurCircuitCode();
}

void LLSDMessageSystem::useServices()
{
	/*
		Having this function body here, causes the classes and globals in this
		file to be linked into any program that uses the llmessage library.
	*/
}

class LLMessageService : public LLHTTPNode
{
	virtual bool validate(const std::string& name, LLSD& context) const
		{ return true; }

	virtual void post(LLHTTPNode::ResponsePtr response, const LLSD& context, const LLSD& input) const;
};

//virtual 
void LLMessageService::post(LLHTTPNode::ResponsePtr response, const LLSD& context, const LLSD& input) const
{
	static LLSDMessageSystem msg(*gMessageSystem);
	
	std::string name = context["request"]["wildcard"]["message-name"];
	
	if (msg.handleMessage(name, input))
	{
		response->result(LLSD());
	}
	else
	{
		response->notFound();
	}
}


LLHTTPRegistration<LLMessageService>
	gHTTPRegistrationMessageService("/message/<message-name>");
