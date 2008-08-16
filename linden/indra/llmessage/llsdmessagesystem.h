/** 
 * @file llsdmessagesystem.h
 * @brief LLSDMessageSystem class header file
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

#ifndef LLSD_MESSAGE_SYSTEM_H
#define LLSD_MESSAGE_SYSTEM_H

#include "message.h"
#include "llsd.h"

class LLSDMessageSystem : public LLMessageSystem
{
public:
	LLSDMessageSystem(); // Testing only.
	LLSDMessageSystem(LLMessageSystem& delegateSystem);
	virtual ~LLSDMessageSystem();

	bool handleMessage(std::string name, const LLSD& message);

	void setInput(const LLSD& input); // Testing only.

	// Input methods, read from input LLSD.
	virtual void getU32Fast(const char *block, const char *var, U32 &data, S32 blocknum = 0);
	virtual void getUUIDFast(const char *block, const char *var, LLUUID &uuid, S32 blocknum = 0);
	virtual void getIPAddrFast(const char *block, const char *var, U32 &ip, S32 blocknum = 0);
	virtual void getIPPortFast(const char *block, const char *var, U16 &port, S32 blocknum = 0);
	virtual void getU64Fast(const char *block, const char *var, U64 &data, S32 blocknum = 0);
	virtual void getStringFast(const char *block, const char *var, S32 buffer_size, char *buffer, S32 blocknum = 0);

	// Output methods, routed to delegate message system.
	virtual void newMessageFast(const char *name);
	virtual void nextBlockFast(const char *blockname);

	virtual void addU32Fast(const char *varname, U32 u);
	virtual void addUUIDFast( const char *varname, const LLUUID& uuid);

	virtual S32 sendReliable(const LLHost &host);

	// Global Information messages, routed to delegate message system
	virtual U32 getOurCircuitCode();

	static void useServices();
	// call this from any application, when you set up the http tree

private:
	LLMessageSystem& mMsg;
	LLSD mInput;
};

#endif // LLSD_MESSAGE_SYSTEM_H
