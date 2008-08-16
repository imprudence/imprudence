/** 
 * @file llsdserialize_tut.cpp
 * @date   April 2006
 * @brief LLSDSerialize unit tests
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

#include <math.h>
#include <sstream>
#include <tut/tut.h>

#include "llsd.h"
#include "llsdserialize.h"
#include "lltut.h"
#include "llformat.h"

// These tests take too long to run on Windows. JC
#if !LL_WINDOWS

namespace tut
{
	struct sd_xml_data
	{
		sd_xml_data()
		{
			mFormatter = new LLSDXMLFormatter;
		}
		LLSD mSD;
		LLPointer<LLSDXMLFormatter> mFormatter;
		void xml_test(const char* name, const std::string& expected)
		{
			std::ostringstream ostr;
			mFormatter->format(mSD, ostr);
			ensure_equals(name, ostr.str(), expected);
		}
	};

	typedef test_group<sd_xml_data> sd_xml_test;
	typedef sd_xml_test::object sd_xml_object;
	tut::sd_xml_test sd_xml_stream("sd_xml_serialization");

	template<> template<>
	void sd_xml_object::test<1>()
	{
		// random atomic tests
		std::string expected;

		expected = "<llsd><undef /></llsd>\n";
		xml_test("undef", expected);

		mSD = 3463;
		expected = "<llsd><integer>3463</integer></llsd>\n";
		xml_test("integer", expected);

		mSD = "";
		expected = "<llsd><string /></llsd>\n";
		xml_test("empty string", expected);

		mSD = "foobar";
		expected = "<llsd><string>foobar</string></llsd>\n";
		xml_test("string", expected);

		mSD = LLUUID::null;
		expected = "<llsd><uuid /></llsd>\n";
		xml_test("null uuid", expected);
		
		mSD = LLUUID("c96f9b1e-f589-4100-9774-d98643ce0bed");
		expected = "<llsd><uuid>c96f9b1e-f589-4100-9774-d98643ce0bed</uuid></llsd>\n";
		xml_test("uuid", expected);

		mSD = LLURI("https://secondlife.com/login");
		expected = "<llsd><uri>https://secondlife.com/login</uri></llsd>\n";
		xml_test("uri", expected);

		mSD = LLDate("2006-04-24T16:11:33Z");
		expected = "<llsd><date>2006-04-24T16:11:33Z</date></llsd>\n";
		xml_test("date", expected);

		// *FIX: test binary
	}
	
	template<> template<>
	void sd_xml_object::test<2>()
	{
		// tests with boolean values.
		std::string expected;

		mFormatter->boolalpha(true);
		mSD = true;
		expected = "<llsd><boolean>true</boolean></llsd>\n";
		xml_test("bool alpha true", expected);
		mSD = false;
		expected = "<llsd><boolean>false</boolean></llsd>\n";
		xml_test("bool alpha false", expected);

		mFormatter->boolalpha(false);
		mSD = true;
		expected = "<llsd><boolean>1</boolean></llsd>\n";
		xml_test("bool true", expected);
		mSD = false;
		expected = "<llsd><boolean>0</boolean></llsd>\n";
		xml_test("bool false", expected);
	}


	template<> template<>
	void sd_xml_object::test<3>()
	{
		// tests with real values.
		std::string expected;

		mFormatter->realFormat("%.2f");
		mSD = 1.0;
		expected = "<llsd><real>1.00</real></llsd>\n";
		xml_test("real 1", expected);

		mSD = -34379.0438;
		expected = "<llsd><real>-34379.04</real></llsd>\n";
		xml_test("real reduced precision", expected);
		mFormatter->realFormat("%.4f");
		expected = "<llsd><real>-34379.0438</real></llsd>\n";
		xml_test("higher precision", expected);

		mFormatter->realFormat("%.0f");
		mSD = 0.0;
		expected = "<llsd><real>0</real></llsd>\n";
		xml_test("no decimal 0", expected);
		mSD = 3287.4387;
		expected = "<llsd><real>3287</real></llsd>\n";
		xml_test("no decimal real number", expected);
	}

	template<> template<>
	void sd_xml_object::test<4>()
	{
		// tests with arrays
		std::string expected;

		mSD = LLSD::emptyArray();
		expected = "<llsd><array /></llsd>\n";
		xml_test("empty array", expected);

		mSD.append(LLSD());
		expected = "<llsd><array><undef /></array></llsd>\n";
		xml_test("1 element array", expected);

		mSD.append(1);
		expected = "<llsd><array><undef /><integer>1</integer></array></llsd>\n";
		xml_test("2 element array", expected);
	}

	template<> template<>
	void sd_xml_object::test<5>()
	{
		// tests with arrays
		std::string expected;

		mSD = LLSD::emptyMap();
		expected = "<llsd><map /></llsd>\n";
		xml_test("empty map", expected);

		mSD["foo"] = "bar";
		expected = "<llsd><map><key>foo</key><string>bar</string></map></llsd>\n";
		xml_test("1 element map", expected);

		mSD["baz"] = LLSD();
		expected = "<llsd><map><key>baz</key><undef /><key>foo</key><string>bar</string></map></llsd>\n";
		xml_test("2 element map", expected);
	}
	
	
	class TestLLSDSerializeData
	{
	public:
		TestLLSDSerializeData();
		~TestLLSDSerializeData();

		void doRoundTripTests(const std::string&);
		void checkRoundTrip(const std::string&, const LLSD& v);
		
		LLPointer<LLSDFormatter> mFormatter;
		LLPointer<LLSDParser> mParser;
	};

	TestLLSDSerializeData::TestLLSDSerializeData()
	{
	}

	TestLLSDSerializeData::~TestLLSDSerializeData()
	{
	}

	void TestLLSDSerializeData::checkRoundTrip(const std::string& msg, const LLSD& v)
	{
		std::stringstream stream;	
		mFormatter->format(v, stream);
		//llinfos << "checkRoundTrip: length " << stream.str().length() << llendl;
		LLSD w;
		mParser->parse(stream, w);
		
		try
		{
			ensure_equals(msg, w, v);
		}
		catch (...)
		{
			std::cerr << "the serialized string was:" << std::endl;
			std::cerr << stream.str() << std::endl;
			throw;
		}
	}

	static void fillmap(LLSD& root, U32 width, U32 depth)
	{
		if(depth == 0)
		{
			root["foo"] = "bar";
			return;
		}

		for(U32 i = 0; i < width; ++i)
		{
			std::string key = llformat("child %d", i);
			root[key] = LLSD::emptyMap();
			fillmap(root[key], width, depth - 1);
		}
	}
	
	void TestLLSDSerializeData::doRoundTripTests(const std::string& msg)
	{
		LLSD v;
		checkRoundTrip(msg + " undefined", v);
		
		v = true;
		checkRoundTrip(msg + " true bool", v);
		
		v = false;
		checkRoundTrip(msg + " false bool", v);
		
		v = 1;
		checkRoundTrip(msg + " positive int", v);
		
		v = 0;
		checkRoundTrip(msg + " zero int", v);
		
		v = -1;
		checkRoundTrip(msg + " negative int", v);
		
		v = 1234.5f;
		checkRoundTrip(msg + " positive float", v);
		
		v = 0.0f;
		checkRoundTrip(msg + " zero float", v);
		
		v = -1234.5f;
		checkRoundTrip(msg + " negative float", v);
		
		// FIXME: need a NaN test
		
		v = LLUUID::null;
		checkRoundTrip(msg + " null uuid", v);
		
		LLUUID newUUID;
		newUUID.generate();
		v = newUUID;
		checkRoundTrip(msg + " new uuid", v);
		
		v = "";
		checkRoundTrip(msg + " empty string", v);
		
		v = "some string";
		checkRoundTrip(msg + " non-empty string", v);
		
		v =
"Second Life is a 3-D virtual world entirely built and owned by its residents. "
"Since opening to the public in 2003, it has grown explosively and today is "
"inhabited by nearly 100,000 people from around the globe.\n"
"\n"
"From the moment you enter the World you’ll discover a vast digital continent, "
"teeming with people, entertainment, experiences and opportunity. Once you’ve "
"explored a bit, perhaps you’ll find a perfect parcel of land to build your "
"house or business.\n"
"\n"
"You’ll also be surrounded by the Creations of your fellow residents. Because "
"residents retain the rights to their digital creations, they can buy, sell "
"and trade with other residents.\n"
"\n"
"The Marketplace currently supports millions of US dollars in monthly "
"transactions. This commerce is handled with the in-world currency, the Linden "
"dollar, which can be converted to US dollars at several thriving online "
"currency exchanges.\n"
"\n"
"Welcome to Second Life. We look forward to seeing you in-world!\n"
		;
		checkRoundTrip(msg + " long string", v);

		static const U32 block_size = 0x000020;
		for (U32 block = 0x000000; block <= 0x10ffff; block += block_size)
		{
			std::ostringstream out;
			
			for (U32 c = block; c < block + block_size; ++c)
			{
				if (c <= 0x000001f
					&& c != 0x000009
					&& c != 0x00000a)
				{
					// see XML standard, sections 2.2 and 4.1
					continue;
				}
				if (0x00d800 <= c  &&  c <= 0x00dfff) { continue; }
				if (0x00fdd0 <= c  &&  c <= 0x00fdef) { continue; }
				if ((c & 0x00fffe) == 0x00fffe) { continue; }		
					// see Unicode standard, section 15.8 
				
				if (c <= 0x00007f)
				{
					out << (char)(c & 0x7f);
				}
				else if (c <= 0x0007ff)
				{
					out << (char)(0xc0 | ((c >> 6) & 0x1f));
					out << (char)(0x80 | ((c >> 0) & 0x3f));
				}
				else if (c <= 0x00ffff)
				{
					out << (char)(0xe0 | ((c >> 12) & 0x0f));
					out << (char)(0x80 | ((c >>  6) & 0x3f));
					out << (char)(0x80 | ((c >>  0) & 0x3f));
				}
				else
				{
					out << (char)(0xf0 | ((c >> 18) & 0x07));
					out << (char)(0x80 | ((c >> 12) & 0x3f));
					out << (char)(0x80 | ((c >>  6) & 0x3f));
					out << (char)(0x80 | ((c >>  0) & 0x3f));
				}
			}
			
			v = out.str();

			std::ostringstream blockmsg;
			blockmsg << msg << " unicode string block 0x" << std::hex << block; 
			checkRoundTrip(blockmsg.str(), v);
		}
		
		LLDate epoch;
		v = epoch;
		checkRoundTrip(msg + " epoch date", v);
		
		LLDate aDay("2002-12-07T05:07:15.00Z");
		v = aDay;
		checkRoundTrip(msg + " date", v);
		
		LLURI path("http://slurl.com/secondlife/Ambleside/57/104/26/");
		v = path;
		checkRoundTrip(msg + " url", v);
		
		const char source[] = "it must be a blue moon again";
		std::vector<U8> data;
		copy(&source[0], &source[sizeof(source)], back_inserter(data));
		
		v = data;
		checkRoundTrip(msg + " binary", v);
		
		v = LLSD::emptyMap();
		checkRoundTrip(msg + " empty map", v);
		
		v = LLSD::emptyMap();
		v["name"] = "luke";		//v.insert("name", "luke");
		v["age"] = 3;			//v.insert("age", 3);
		checkRoundTrip(msg + " map", v);
		
		v.clear();
		v["a"]["1"] = true;
		v["b"]["0"] = false;
		checkRoundTrip(msg + " nested maps", v);
		
		v = LLSD::emptyArray();
		checkRoundTrip(msg + " empty array", v);
		
		v = LLSD::emptyArray();
		v.append("ali");
		v.append(28);
		checkRoundTrip(msg + " array", v);
		
		v.clear();
		v[0][0] = true;
		v[1][0] = false;
		checkRoundTrip(msg + " nested arrays", v);

		v = LLSD::emptyMap();
		fillmap(v, 10, 6); // 10^6 maps
		checkRoundTrip(msg + " many nested maps", v);
	}
	
	typedef tut::test_group<TestLLSDSerializeData> TestLLSDSerialzeGroup;
	typedef TestLLSDSerialzeGroup::object TestLLSDSerializeObject;
	TestLLSDSerialzeGroup gTestLLSDSerializeGroup("llsd serialization");

	template<> template<> 
	void TestLLSDSerializeObject::test<1>()
	{
		mFormatter = new LLSDNotationFormatter();
		mParser = new LLSDNotationParser();
		doRoundTripTests("notation serialization");
	}
	
	template<> template<> 
	void TestLLSDSerializeObject::test<2>()
	{
		mFormatter = new LLSDXMLFormatter();
		mParser = new LLSDXMLParser();
		doRoundTripTests("xml serialization");
	}
	
	template<> template<> 
	void TestLLSDSerializeObject::test<3>()
	{
		mFormatter = new LLSDBinaryFormatter();
		mParser = new LLSDBinaryParser();
		doRoundTripTests("binary serialization");
	}

	
	
	
	class TestLLSDXMLParsing
	{
	public:
		TestLLSDXMLParsing()
		{
			mParser = new LLSDXMLParser;
		}
		void ensureParse(const std::string& msg, const char* xml, const LLSD& expected);
		
		LLPointer<LLSDXMLParser> mParser;
	};
	
	void TestLLSDXMLParsing::ensureParse(
		const std::string& msg, const char* xmlstring, const LLSD& expected)
	{
		std::stringstream input;
		input.str(xmlstring);
		
		LLSD parsedResult;
		mParser->parse(input, parsedResult);
		
		ensure_equals(msg, parsedResult, expected);
	}
	

	typedef tut::test_group<TestLLSDXMLParsing> TestLLSDXMLParsingGroup;
	typedef TestLLSDXMLParsingGroup::object TestLLSDXMLParsingObject;
	TestLLSDXMLParsingGroup gTestLLSDXMLParsingGroup("llsd XML parsing");

	template<> template<> 
	void TestLLSDXMLParsingObject::test<1>()
	{
		// test handling of xml not recognized as llsd results in an LLSD Undefined"
		
		ensureParse("malformed xml", "<llsd><string>ha ha</string>", LLSD());
		ensureParse("not llsd", "<html><body><p>ha ha</p></body></html>", LLSD());
		ensureParse("value without llsd", "<string>ha ha</string>", LLSD());
		ensureParse("key without llsd", "<key>ha ha</key>", LLSD());
	}
	
	
	template<> template<> 
	void TestLLSDXMLParsingObject::test<2>()
	{
		// test handling of unrecognized or unparseable llsd values
		
		LLSD v;
		v["amy"] = 23;
		v["bob"] = LLSD();
		v["cam"] = 1.23;
		
		ensureParse("unknown data type",
			"<llsd><map>"
				"<key>amy</key><integer>23</integer>"
				"<key>bob</key><bigint>99999999999999999</bigint>"
				"<key>cam</key><real>1.23</real>"
			"</map></llsd>", v);
	}
	
	template<> template<> 
	void TestLLSDXMLParsingObject::test<3>()
	{
		// test handling of nested bad data
		
		LLSD v;
		v["amy"] = 23;
		v["cam"] = 1.23;
		
		ensureParse("map with html",
			"<llsd><map>"
				"<key>amy</key><integer>23</integer>"
				"<html><body>ha ha</body></html>"
				"<key>cam</key><real>1.23</real>"
			"</map></llsd>", v);
			
		v.clear();
		v["amy"] = 23;
		v["cam"] = 1.23;
		ensureParse("map with value for key",
			"<llsd><map>"
				"<key>amy</key><integer>23</integer>"
				"<string>ha ha</string>"
				"<key>cam</key><real>1.23</real>"
			"</map></llsd>", v);
			
		v.clear();
		v["amy"] = 23;
		v["bob"] = LLSD::emptyMap();
		v["cam"] = 1.23;
		ensureParse("map with map of html",
			"<llsd><map>"
				"<key>amy</key><integer>23</integer>"
				"<key>bob</key>"
				"<map>"
					"<html><body>ha ha</body></html>"
				"</map>"
				"<key>cam</key><real>1.23</real>"
			"</map></llsd>", v);

		v.clear();
		v[0] = 23;
		v[1] = LLSD();
		v[2] = 1.23;
		
		ensureParse("array value of html",
			"<llsd><array>"
				"<integer>23</integer>"
				"<html><body>ha ha</body></html>"
				"<real>1.23</real>"
			"</array></llsd>", v);
			
		v.clear();
		v[0] = 23;
		v[1] = LLSD::emptyMap();
		v[2] = 1.23;
		ensureParse("array with map of html",
			"<llsd><array>"
				"<integer>23</integer>"
				"<map>"
					"<html><body>ha ha</body></html>"
				"</map>"
				"<real>1.23</real>"
			"</array></llsd>", v);
	}

	/*
	TODO:
		test XML parsing
			binary with unrecognized encoding
			nested LLSD tags
			multiple values inside an LLSD
	*/
}

#endif
