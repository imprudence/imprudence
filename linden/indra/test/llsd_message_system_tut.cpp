/** 
 * @file llsd_message_system_tut.cpp
 * @brief Testing the LLSDMessageSystem.
 *
 * Copyright (c) 2006-2007, Linden Research, Inc.
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

/**
 *
 * These classes test the LLSDMessageSystem.
 *
 */

#include <tut/tut.h>
#include "llsdmessagesystem.h"
#include "llsdutil.h"

namespace tut
{
	class LLSDMessageSystemTestData
	{
	public:
		LLSDMessageSystemTestData() {;}
		~LLSDMessageSystemTestData() {;}

		LLSDMessageSystem mMsgSystem;
	};

	typedef test_group<LLSDMessageSystemTestData>	LLSDMessageSystemTestGroup;
	typedef LLSDMessageSystemTestGroup::object		LLSDMessageSystemTestObject;
	LLSDMessageSystemTestGroup llsdMessageSystemTestGroup("llsd_message_system");

	template<> template<>
	void LLSDMessageSystemTestObject::test<1>()
	{
		LLSD input;
		U32 valueIn, valueOut;
		valueIn = 42;
		input["Block"]["Var"] = ll_sd_from_U32(valueIn);
		mMsgSystem.setInput(input);
		mMsgSystem.getU32Fast("Block", "Var", valueOut);
		ensure_equals("U32 from message system matches input U32", valueIn, valueOut);
	}

	template<> template<>
	void LLSDMessageSystemTestObject::test<2>()
	{
		LLSD input;
		LLUUID valueIn, valueOut;
		valueIn.generate();
		input["Block"]["Var"] = valueIn;
		mMsgSystem.setInput(input);
		mMsgSystem.getUUIDFast("Block", "Var", valueOut);
		ensure_equals("UUID from message system matches input UUID", valueIn, valueOut);
	}

	template<> template<>
	void LLSDMessageSystemTestObject::test<3>()
	{
		LLSD input;
		U32 valueIn, valueOut;
		LLHost host("127.0.0.1:80");
		valueIn = host.getAddress();
		input["Block"]["Var"] = ll_sd_from_U32(valueIn);
		mMsgSystem.setInput(input);
		mMsgSystem.getIPAddrFast("Block", "Var", valueOut);
		ensure_equals("IP from message system matches input IP", valueIn, valueOut);
	}

	template<> template<>
	void LLSDMessageSystemTestObject::test<4>()
	{
		LLSD input;
		U16 valueIn, valueOut;
		LLHost host("127.0.0.1:80");
		valueIn = host.getPort();
		input["Block"]["Var"] = (S32)valueIn;
		mMsgSystem.setInput(input);
		mMsgSystem.getIPPortFast("Block", "Var", valueOut);
		ensure_equals("Port from message system matches input port", valueIn, valueOut);
	}

	template<> template<>
	void LLSDMessageSystemTestObject::test<5>()
	{
		LLSD input;
		U64 valueIn, valueOut;
		valueIn = 42;
		input["Block"]["Var"] = ll_sd_from_U64(valueIn);
		mMsgSystem.setInput(input);
		mMsgSystem.getU64Fast("Block", "Var", valueOut);
		ensure_equals("Port from message system matches input port", valueIn, valueOut);
	}

	template<> template<>
	void LLSDMessageSystemTestObject::test<6>()
	{
		LLSD input;
		std::string valueIn = "Value";
		input["Block"]["Var"] = valueIn;
		mMsgSystem.setInput(input);
		const U32 buffLen = 16;
		char buff[buffLen];
		mMsgSystem.getStringFast("Block", "Var", buffLen, buff);
		ensure_equals("string read from message system matches llsd input", std::string(buff), valueIn);
	}

	template<> template<>
	void LLSDMessageSystemTestObject::test<7>()
	{
		LLSD input;
		U32 valueIn, valueOut;
		valueIn = 42;
		input["Block"][0]["Var"] = ll_sd_from_U32(valueIn);
		input["Block"][1]["Var"] = ll_sd_from_U32(valueIn + 1);
		mMsgSystem.setInput(input);
		mMsgSystem.getU32Fast("Block", "Var", valueOut, 0);
		ensure_equals("U32 from message system matches input U32", valueIn, valueOut);
		mMsgSystem.getU32Fast("Block", "Var", valueOut, 1);
		ensure_equals("U32 from message system matches input U32", (valueIn + 1), valueOut);
	}
}
