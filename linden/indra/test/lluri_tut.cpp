/**
 * @file   lluri_tut.cpp
 * @brief  LLURI unit tests
 * @date   September 2006
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

#include <tut/tut.h>
#include "lltut.h"

#include "llsd.h"
#include "lluri.h"
#include "llhost.h"

namespace tut
{
	struct URITestData {
		void checkParts(const std::string& uriString,
				const char* expectedScheme,
				const char* expectedOpaque,
				const char* expectedAuthority,
				const char* expectedPath,
				const char* expectedQuery)
		{
			LLURI u(uriString);

			ensure_equals("scheme",		u.scheme(),		expectedScheme);
			ensure_equals("opaque",		u.opaque(),		expectedOpaque);
			ensure_equals("authority",	u.authority(),	expectedAuthority);
			ensure_equals("path",		u.path(),		expectedPath);
			ensure_equals("query",		u.query(),		expectedQuery);
		}

		void checkPartsHTTP(const char* host_and_port,
							const LLSD& path,
							const char* expectedOpaque,
							const char* expectedAuthority,
							const char* expectedPath)
		{
			LLURI u = LLURI::buildHTTP(host_and_port, path);
			ensure_equals("scheme",		u.scheme(),		"HTTP");
			ensure_equals("opaque",		u.opaque(),		expectedOpaque);
			ensure_equals("authority",	u.authority(),	expectedAuthority);
			ensure_equals("path",		u.path(),		expectedPath);
			ensure_equals("query",		u.query(),		"");
		}

		void checkPartsHTTP(const char* host_and_port,
							const LLSD& path,
							const LLSD& args,
							const char* expectedOpaque,
							const char* expectedAuthority,
							const char* expectedPath,
							const char* expectedQuery)
		{
			LLURI u = LLURI::buildHTTP(host_and_port, path, args);
			ensure_equals("scheme",		u.scheme(),		"HTTP");
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
		checkParts("", "", "", "", "", "");
	}
	
	template<> template<>
	void URITestObject::test<3>()
	{
		// no scheme
		checkParts("foo", "", "foo", "", "", "");
		checkParts("foo%3A", "", "foo:", "", "", "");
	}

	template<> template<>
	void URITestObject::test<4>()
	{
		// scheme w/o paths
		checkParts("mailto:zero@ll.com", "mailto", "zero@ll.com", "", "", "");
		checkParts("silly://abc/def?foo", "silly", "//abc/def?foo", "", "", "");
	}

	template<> template<>
	void URITestObject::test<5>()
	{
		// authority section
		checkParts("http:///", "http", "///", "", "/", "");
		checkParts("http://abc", "http", "//abc", "abc", "", "");
		checkParts("http://a%2Fb/cd", "http", "//a/b/cd", "a/b", "/cd", "");
		checkParts("http://host?", "http", "//host?", "host", "", "");
	}

	template<> template<>
	void URITestObject::test<6>()
	{		
		// path section
		checkParts("http://host/a/b/", "http", "//host/a/b/",
				"host", "/a/b/", "");
		checkParts("http://host/a%3Fb/", "http", "//host/a?b/",
				"host", "/a?b/", "");
		checkParts("http://host/a:b/", "http", "//host/a:b/",
				"host", "/a:b/", "");
	}

	template<> template<>
	void URITestObject::test<7>()
	{		
		// query string
		checkParts("http://host/?", "http", "//host/?",
				"host", "/", "");
		checkParts("http://host/?x", "http", "//host/?x",
				"host", "/", "x");
		checkParts("http://host/??", "http", "//host/??",
				"host", "/", "?");
		checkParts("http://host/?%3F", "http", "//host/??",
				"host", "/", "?");
	}

	template<> template<>
	void URITestObject::test<8>()
	{
		LLSD path;
		path.append("x");
		path.append("123");
		checkPartsHTTP("host", path, "//host/x/123", "//host", "/x/123");
		LLSD query;
		query["123"] = "12";
		query["abcd"] = "abc";
		checkPartsHTTP("host", path, query, "//host/x/123?123=12&abcd=abc&", "//host", "/x/123", "123=12&abcd=abc&");
	}

	template<> template<>
	void URITestObject::test<9>()
	{
		// test unescaped path components
		LLSD path;
		path.append("x@*//*$&^");
		path.append("123");
		checkPartsHTTP("host", path, "//host/x@*//*$&^/123", "//host", "/x@*//*$&^/123");
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
		checkPartsHTTP("host", path, query, "//host/x/123?**@&?//=abc&123=?&*#//&", "//host", "/x/123", "**@&?//=abc&123=?&*#//&");
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
		checkPartsHTTP("hi123*33--}{:portstuffs", path, query, "//hi123*33--}{:portstuffs/x/123?123=12&abcd=abc&", "//hi123*33--}{:portstuffs", "/x/123", "123=12&abcd=abc&");
	}
}

