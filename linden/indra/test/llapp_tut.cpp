/** 
 * @file llapp_tut.cpp
 * @author Phoenix
 * @date 2006-09-12
 *
 * $LicenseInfo:firstyear=2006&license=viewergpl$
 * 
 * Copyright (c) 2006-2009, Linden Research, Inc.
 * 
 * Second Life Viewer Source Code
 * The source code in this file ("Source Code") is provided by Linden Lab
 * to you under the terms of the GNU General Public License, version 2.0
 * ("GPL"), unless you have obtained a separate licensing agreement
 * ("Other License"), formally executed by you and Linden Lab.  Terms of
 * the GPL can be found in doc/GPL-license.txt in this distribution, or
 * online at http://secondlifegrid.net/programs/open_source/licensing/gplv2
 * 
 * There are special exceptions to the terms and conditions of the GPL as
 * it is applied to this Source Code. View the full text of the exception
 * in the file doc/FLOSS-exception.txt in this software distribution, or
 * online at
 * http://secondlifegrid.net/programs/open_source/licensing/flossexception
 * 
 * By copying, modifying or distributing this software, you acknowledge
 * that you have read and understood your obligations described above,
 * and agree to abide by those obligations.
 * 
 * ALL LINDEN LAB SOURCE CODE IS PROVIDED "AS IS." LINDEN LAB MAKES NO
 * WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
 * COMPLETENESS OR PERFORMANCE.
 * $/LicenseInfo$
 */

#include <tut/tut.hpp>

#include "linden_common.h"
#include "llapp.h"
#include "lltut.h"


namespace tut
{
	struct application
	{
		class LLTestApp : public LLApp
		{
		public:
			virtual bool init() { return true; }
			virtual bool cleanup() { return true; }
			virtual bool mainLoop() { return true; }
		};
		LLTestApp* mApp;
		application()
		{
			mApp = new LLTestApp;
		}
		~application()
		{
			delete mApp;
		}
	};

	typedef test_group<application> application_t;
	typedef application_t::object application_object_t;
	tut::application_t tut_application("application");

	template<> template<>
	void application_object_t::test<1>()
	{
		LLSD defaults;
		defaults["template"] = "../../../scripts/messages/message_template.msg";
		defaults["configdir"] = ".";
		defaults["db_host"] = "mysql.shakti.lindenlab.com";
		defaults["db_user"] = "linden";
		defaults["db_password"] = "gomez";
		defaults["datadir"] = "data";
		mApp->setOptionData(LLApp::PRIORITY_DEFAULT, defaults);

		LLSD db_user_sd = mApp->getOption("db_user");
		ensure_equals("data type", db_user_sd.type(), LLSD::TypeString);
		ensure_equals(
			"data value", db_user_sd.asString(), std::string("linden"));
	}

	template<> template<>
	void application_object_t::test<2>()
	{
		const int ARGC = 13;
		char* ARGV[ARGC] =
		{
			"", // argv[0] is usually the application name
			"-crashcount",
			"2",
			"-space",
			"spaceserver.grid.lindenlab.com",
			"-db_host",
			"localhost",
			"--allowlslhttprequests",
			"-asset-uri",
			"http://asset.grid.lindenlab.com/assets",
			"-data",
			"127.0.0.1",
			"--smtp"
		};
		bool ok = mApp->parseCommandOptions(ARGC, ARGV);
		ensure("command line parsed", ok);
		ensure_equals(
			"crashcount", mApp->getOption("crashcount").asInteger(), 2);
		ensure_equals(
			"space",
			mApp->getOption("space").asString(),
			std::string("spaceserver.grid.lindenlab.com"));
		ensure_equals(
			"db_host",
			mApp->getOption("db_host").asString(),
			std::string("localhost"));
		ensure("allowlshlttprequests", mApp->getOption("smtp"));
		ensure_equals(
			"asset-uri",
			mApp->getOption("asset-uri").asString(),
			std::string("http://asset.grid.lindenlab.com/assets"));
		ensure_equals(
			"data",
			mApp->getOption("data").asString(),
			std::string("127.0.0.1"));
		ensure("smtp", mApp->getOption("smtp"));
	}

	template<> template<>
	void application_object_t::test<3>()
	{
		const int ARGC = 4;
		char* ARGV[ARGC] =
		{
			"", // argv[0] is usually the application name
			"crashcount",
			"2",
			"--space"
		};
		bool ok = mApp->parseCommandOptions(ARGC, ARGV);
		ensure("command line parse failure", !ok);
	}

	template<> template<>
	void application_object_t::test<4>()
	{
		const int ARGC = 4;
		char* ARGV[ARGC] =
		{
			"", // argv[0] is usually the application name
			"--crashcount",
			"2",
			"space"
		};
		bool ok = mApp->parseCommandOptions(ARGC, ARGV);
		ensure("command line parse failure", !ok);
	}


	template<> template<>
	void application_object_t::test<5>()
	{
		LLSD options;
		options["boolean-test"] = true;
		mApp->setOptionData(LLApp::PRIORITY_GENERAL_CONFIGURATION, options);
		ensure("bool set", mApp->getOption("boolean-test").asBoolean());
		options["boolean-test"] = false;
		mApp->setOptionData(LLApp::PRIORITY_RUNTIME_OVERRIDE, options);
		ensure("bool unset", !mApp->getOption("boolean-test").asBoolean());
	}
}
