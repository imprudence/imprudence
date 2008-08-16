/**
 * @file   lluri_tut.cpp
 * @brief  LLURI unit tests
 * @date   September 2006
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

#include "llsd.h"
#include "lluri.h"
#include "llhost.h"

namespace tut
{
	struct URITestData {
		void checkParts(const LLURI& u,
				const char* expectedScheme,
				const char* expectedOpaque,
				const char* expectedAuthority,
				const char* expectedPath,
				const char* expectedQuery = "")
		{
			ensure_equals("scheme",		u.scheme(),		expectedScheme);
			ensure_equals("opaque",		u.opaque(),		expectedOpaque);
			ensure_equals("authority",	u.authority(),	expectedAuthority);
			ensure_equals("path",		u.path(),		expectedPath);
			ensure_equals("query",		u.query(),		expectedQuery);
		}
	};
	
	typedef test_group<URITestData>	URITestGroup;
	typedef URITestGroup::object	URITestObject;

	URITestGroup uriTestGroup("LLURI");
	
	template<> template<>
	void URITestObject::test<1>()
	{
		LLURI u("http://abc.com/def/ghi?x=37&y=hello");

		ensure_equals("scheme",		u.scheme(),		"http");
		ensure_equals("authority",	u.authority(),	"abc.com");
		ensure_equals("path",		u.path(),		"/def/ghi");
		ensure_equals("query",		u.query(),		"x=37&y=hello");

		ensure_equals("host name", u.hostName(), "abc.com");
		ensure_equals("host port", u.hostPort(), 80);

		LLSD query = u.queryMap();
		ensure_equals("query x", query["x"].asInteger(), 37);
		ensure_equals("query y", query["y"].asString(), "hello");

		query = LLURI::queryMap("x=22.23&y=https://lindenlab.com/");
		ensure_equals("query x", query["x"].asReal(), 22.23);
		ensure_equals("query y", query["y"].asURI().asString(), "https://lindenlab.com/");
	}

	template<> template<>
	void URITestObject::test<2>()
	{
		// empty string
		checkParts(LLURI(""), "", "", "", "");
	}
	
	template<> template<>
	void URITestObject::test<3>()
	{
		// no scheme
		checkParts(LLURI("foo"), "", "foo", "", "");
		checkParts(LLURI("foo%3A"), "", "foo:", "", "");
	}

	template<> template<>
	void URITestObject::test<4>()
	{
		// scheme w/o paths
		checkParts(LLURI("mailto:zero@ll.com"),
			"mailto", "zero@ll.com", "", "");
		checkParts(LLURI("silly://abc/def?foo"),
			"silly", "//abc/def?foo", "", "");
	}

	template<> template<>
	void URITestObject::test<5>()
	{
		// authority section
		checkParts(LLURI("http:///"),
			"http", "///", "", "/");
			
		checkParts(LLURI("http://abc"),
			"http", "//abc", "abc", "");
			
		checkParts(LLURI("http://a%2Fb/cd"),
			"http", "//a/b/cd", "a/b", "/cd");
			
		checkParts(LLURI("http://host?"),
			"http", "//host?", "host", "");
	}

	template<> template<>
	void URITestObject::test<6>()
	{		
		// path section
		checkParts(LLURI("http://host/a/b/"),
				"http", "//host/a/b/", "host", "/a/b/");
				
		checkParts(LLURI("http://host/a%3Fb/"),
				"http", "//host/a?b/", "host", "/a?b/");
				
		checkParts(LLURI("http://host/a:b/"),
				"http", "//host/a:b/", "host", "/a:b/");
	}

	template<> template<>
	void URITestObject::test<7>()
	{		
		// query string
		checkParts(LLURI("http://host/?"),
				"http", "//host/?", "host", "/", "");
				
		checkParts(LLURI("http://host/?x"),
				"http", "//host/?x", "host", "/", "x");
				
		checkParts(LLURI("http://host/??"),
				"http", "//host/??", "host", "/", "?");
				
		checkParts(LLURI("http://host/?%3F"),
				"http", "//host/??", "host", "/", "?");
	}

	template<> template<>
	void URITestObject::test<8>()
	{
		LLSD path;
		path.append("x");
		path.append("123");
		checkParts(LLURI::buildHTTP("host", path),
			"http", "//host/x/123", "host", "/x/123");
		
		LLSD query;
		query["123"] = "12";
		query["abcd"] = "abc";
		checkParts(LLURI::buildHTTP("host", path, query),
			"http", "//host/x/123?123=12&abcd=abc&",
			"host", "/x/123", "123=12&abcd=abc&");
	}

	template<> template<>
	void URITestObject::test<9>()
	{
		// test unescaped path components
		LLSD path;
		path.append("x@*//*$&^");
		path.append("123");
		checkParts(LLURI::buildHTTP("host", path),
			"http", "//host/x@*//*$&^/123", "host", "/x@*//*$&^/123");
	}

	template<> template<>
	void URITestObject::test<10>()
	{
		// test unescaped query components
		LLSD path;
		path.append("x");
		path.append("123");
		LLSD query;
		query["123"] = "?&*#//";
		query["**@&?//"] = "abc";
		checkParts(LLURI::buildHTTP("host", path, query),
			"http", "//host/x/123?**@&?//=abc&123=?&*#//&",
			"host", "/x/123", "**@&?//=abc&123=?&*#//&");
	}

	template<> template<>
	void URITestObject::test<11>()
	{
		// test unescaped host components
		LLSD path;
		path.append("x");
		path.append("123");
		LLSD query;
		query["123"] = "12";
		query["abcd"] = "abc";
		checkParts(LLURI::buildHTTP("hi123*33--}{:portstuffs", path, query),
			"http", "//hi123*33--}{:portstuffs/x/123?123=12&abcd=abc&",
			"hi123*33--}{:portstuffs", "/x/123", "123=12&abcd=abc&");
	}
	
	template<> template<>
	void URITestObject::test<12>()
	{
		// test funky host_port values that are actually prefixes
		
		checkParts(LLURI::buildHTTP("http://example.com:8080", LLSD()),
			"http", "//example.com:8080",
			"example.com:8080", "");
			
		checkParts(LLURI::buildHTTP("http://example.com:8080/", LLSD()),
			"http", "//example.com:8080/",
			"example.com:8080", "/");

		checkParts(LLURI::buildHTTP("http://example.com:8080/a/b", LLSD()),
			"http", "//example.com:8080/a/b",
			"example.com:8080", "/a/b");
	}

	template<> template<>
	void URITestObject::test<13>()
	{
		const std::string unreserved =   
			"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
			"0123456789"
			"-._~";
		// test escape
		ensure_equals("escaping", LLURI::escape("abcdefg", "abcdef"), "abcdef%67");
		ensure_equals("escaping", LLURI::escape("|/&\\+-_!@", ""), "%7C%2F%26%5C%2B%2D%5F%21%40");
		ensure_equals("escaping as query variable", 
					  LLURI::escape("http://10.0.1.4:12032/agent/god/agent-id/map/layer/?resume=http://station3.ll.com:12032/agent/203ad6df-b522-491d-ba48-4e24eb57aeff/send-postcard", unreserved + ":@!$'()*+,="), 
					  "http:%2F%2F10.0.1.4:12032%2Fagent%2Fgod%2Fagent-id%2Fmap%2Flayer%2F%3Fresume=http:%2F%2Fstation3.ll.com:12032%2Fagent%2F203ad6df-b522-491d-ba48-4e24eb57aeff%2Fsend-postcard");
	}
	

#if LL_ENABLE_JANKY_DEPRECATED_WEB_SERVICE_CALLS
	template<> template<>
	void URITestObject::test<14>()
	{
		// test various build utilities
		
		LLUUID id("11111111-2222-3333-4444-5566778899aa");
		
		
		checkParts(LLURI::buildAgentSessionURI(id, NULL),
			"http", "//localhost:12040/agent/11111111-2222-3333-4444-5566778899aa/session",
			"localhost:12040", "/agent/11111111-2222-3333-4444-5566778899aa/session");

		checkParts(LLURI::buildAgentLoginInfoURI(id, "datasever:12345"),
			"http", "//datasever:12345/agent/11111111-2222-3333-4444-5566778899aa/logininfo",
			"datasever:12345", "/agent/11111111-2222-3333-4444-5566778899aa/logininfo");
	}
#endif // LL_ENABLE_JANKY_DEPRECATED_WEB_SERVICE_CALLS
}

