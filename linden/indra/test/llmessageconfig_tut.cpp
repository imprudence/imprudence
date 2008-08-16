/** 
 * @file llmessageconfig_tut.cpp
 * @date   March 2007
 * @brief LLMessageConfig unit tests
 *
 * Copyright (c) 2006-2007, Linden Research, Inc.
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

#include <tut/tut.h>
#include "lltut.h"
#include "llmessageconfig.h"
#include "llsdserialize.h"
#include "llfile.h"
#include "lltimer.h"
#include "llframetimer.h"

namespace tut
{
	///var/tmp/babbage/dev/message-liberation/etc
	static const char file_name[] = "/tmp/message.xml";
	static const F32 refreshRate = 6.0*1000.0; // milliseconds
	
	struct LLMessageConfigTestData {

		LLSD getCurrentConfig()
		{
			LLSD data;
			// store aside the current config to overwrite the test ones
			// when the test finishes
			llifstream in_file(file_name);
			if (in_file.is_open())
			{
				LLSDSerialize::fromXML(data, in_file);
			}
			return data;
		}

		void writeConfigFile(const LLSD& config)
		{
			LLMessageConfig::initClass("simulator", "/tmp");
			llofstream file(file_name);
			if (file.is_open())
			{
				LLSDSerialize::toPrettyXML(config, file);
			}
			file.close();
			ms_sleep(refreshRate);
			LLFrameTimer::updateFrameTime();
		}
	};
	
	typedef test_group<LLMessageConfigTestData> LLMessageConfigTestGroup;
	typedef LLMessageConfigTestGroup::object LLMessageConfigTestObject;
	LLMessageConfigTestGroup llMessageConfigTestGroup("LLMessageConfig");
	
	template<> template<>
	void LLMessageConfigTestObject::test<1>()
		// tests server defaults
	{
		LLSD config_backup = getCurrentConfig();
		LLSD config;
		config["serverDefaults"]["simulator"] = "template";
		writeConfigFile(config);
		ensure_equals("Ensure server default is not llsd",
					  LLMessageConfig::isServerDefaultBuilderLLSD(),
					  false);
		ensure_equals("Ensure server default is template",
					  LLMessageConfig::isServerDefaultBuilderTemplate(),
					  true);
		writeConfigFile(config_backup);
	}

	template<> template<>
	void LLMessageConfigTestObject::test<2>()
		// tests message builders
	{
		LLSD config_backup = getCurrentConfig();
		LLSD config;
		config["serverDefaults"]["simulator"] = "template";
		config["messages"]["msg1"]["builder"] = "template";
		config["messages"]["msg2"]["builder"] = "llsd";
		writeConfigFile(config);
		ensure_equals("Ensure msg template builder not llsd",
					  LLMessageConfig::isMessageBuiltLLSD("msg1"),
					  false);
		ensure_equals("Ensure msg template builder",
					  LLMessageConfig::isMessageBuiltTemplate("msg1"),
					  true);
		ensure_equals("Ensure msg llsd builder",
					  LLMessageConfig::isMessageBuiltLLSD("msg2"),
					  true);
		ensure_equals("Ensure msg llsd builder not template",
					  LLMessageConfig::isMessageBuiltTemplate("msg2"),
					  false);
		writeConfigFile(config_backup);
	}

	template<> template<>
	void LLMessageConfigTestObject::test<4>()
		// tests message builder defaults
	{
		LLSD config_backup = getCurrentConfig();
		LLSD config;
		config["serverDefaults"]["simulator"] = "llsd";
		config["messages"]["msg1"]["trusted-sender"] = true;
		writeConfigFile(config);
		ensure_equals("Ensure missing message defaults to server builder, not template",
					  LLMessageConfig::isMessageBuiltTemplate("Test"),
					  false);
		ensure_equals("Ensure missing message default to server builder llsd",
					  LLMessageConfig::isMessageBuiltLLSD("Test"),
					  true);
		ensure_equals("Ensure missing builder defaults to server builder, not template",
					  LLMessageConfig::isMessageBuiltTemplate("msg1"),
					  false);
		ensure_equals("Ensure missing builder default to server builder llsd",
					  LLMessageConfig::isMessageBuiltLLSD("msg1"),
					  true);

		ensure_equals("Ensure server default is not llsd",
					  LLMessageConfig::isServerDefaultBuilderLLSD(),
					  true);
		ensure_equals("Ensure server default is template",
					  LLMessageConfig::isServerDefaultBuilderTemplate(),
					  false);

		writeConfigFile(config_backup);
	}

	template<> template<>
	void LLMessageConfigTestObject::test<3>()
		// tests trusted/untrusted senders
	{
		LLSD config_backup = getCurrentConfig();
		LLSD config;
		config["serverDefaults"]["simulator"] = "template";
		config["messages"]["msg1"]["builder"] = "llsd";
		config["messages"]["msg1"]["trusted-sender"] = false;
		config["messages"]["msg2"]["builder"] = "llsd";
		config["messages"]["msg2"]["trusted-sender"] = true;
		writeConfigFile(config);
		ensure_equals("Ensure untrusted is not trusted",
					  LLMessageConfig::isMessageTrusted("msg1"),
					  false);
		ensure_equals("Ensure untrusted is untrusted",
					  LLMessageConfig::isValidUntrustedMessage("msg1"),
					  true);
		ensure_equals("Ensure trusted is trusted",
					  LLMessageConfig::isMessageTrusted("msg2"),
					  true);
		ensure_equals("Ensure trusted is not untrusted",
					  LLMessageConfig::isValidUntrustedMessage("msg2"),
					  false);
		writeConfigFile(config_backup);
	}
	
	template<> template<>
	void LLMessageConfigTestObject::test<5>()
		// tests trusted/untrusted without flag, only builder
	{
		LLSD config_backup = getCurrentConfig();
		LLSD config;
		config["serverDefaults"]["simulator"] = "template";
		config["messages"]["msg1"]["builder"] = "llsd";
		writeConfigFile(config);
		ensure_equals("Ensure missing trusted is not trusted",
					  LLMessageConfig::isMessageTrusted("msg1"),
					  false);
		ensure_equals("Ensure missing trusted is not untrusted",
					  LLMessageConfig::isValidUntrustedMessage("msg1"),
					  false);
		writeConfigFile(config_backup);
	}

	template<> template<>
	void LLMessageConfigTestObject::test<6>()
		// tests message builder defaults
	{
		LLSD config_backup = getCurrentConfig();
		LLSD config;
		config["serverDefaults"]["simulator"] = "template";
		config["messages"]["msg1"]["trusted-sender"] = true;
		writeConfigFile(config);
		ensure_equals("Ensure missing message defaults to server builder, not template",
					  LLMessageConfig::isMessageBuiltTemplate("Test"),
					  true);
		ensure_equals("Ensure missing message default to server builder llsd",
					  LLMessageConfig::isMessageBuiltLLSD("Test"),
					  false);
		ensure_equals("Ensure missing builder defaults to server builder, not template",
					  LLMessageConfig::isMessageBuiltTemplate("msg1"),
					  true);
		ensure_equals("Ensure missing builder default to server builder llsd",
					  LLMessageConfig::isMessageBuiltLLSD("msg1"),
					  false);

		ensure_equals("Ensure server default is not llsd",
					  LLMessageConfig::isServerDefaultBuilderLLSD(),
					  false);
		ensure_equals("Ensure server default is template",
					  LLMessageConfig::isServerDefaultBuilderTemplate(),
					  true);

		writeConfigFile(config_backup);
	}
}
