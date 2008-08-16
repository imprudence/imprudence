/** 
 * @file llstring_tut.cpp
 * @author Adroit
 * @date 2006-12-24
 * @brief Test cases of llstring.cpp
 *
 * Copyright (c) 2007-2007, Linden Research, Inc.
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
#include "linden_common.h"
#include "llstring.h"

namespace tut
{
	struct string_index
	{
	};
	typedef test_group<string_index> string_index_t;
	typedef string_index_t::object string_index_object_t;
	tut::string_index_t tut_string_index("string_test");

	template<> template<>
	void string_index_object_t::test<1>()
	{
		LLString llstr1;
		ensure("Empty LLString", (llstr1.size() == 0) && llstr1.empty());

		LLString llstr2("Hello");
		ensure("LLString = Hello", (!strcmp(llstr2.c_str(), "Hello")) && (llstr2.size() == 5) && !llstr2.empty());

		LLString llstr3(llstr2);
		ensure("LLString = LLString(LLString)", (!strcmp(llstr3.c_str(), "Hello")) && (llstr3.size() == 5) && !llstr3.empty());

		std::string str("Hello World");
		LLString llstr4(str, 6);
		ensure("LLString = LLString(s, size_type pos, size_type n = npos)", (!strcmp(llstr4.c_str(), "World")) && (llstr4.size() == 5) && !llstr4.empty());

		LLString llstr5(str, str.size());
		ensure("LLString = LLString(s, size_type pos, size_type n = npos)", (llstr5.size() == 0) && llstr5.empty());

		LLString llstr6(5, 'A');
		ensure("LLString = LLString(count, c)", (!strcmp(llstr6.c_str(), "AAAAA")) && (llstr6.size() == 5) && !llstr6.empty());

		LLString llstr7("Hello World", 5);
		ensure("LLString(s, n)", (!strcmp(llstr7.c_str(), "Hello")) && (llstr7.size() == 5) && !llstr7.empty());

		LLString llstr8("Hello World", 6, 5);
		ensure("LLString(s, n, count)", (!strcmp(llstr8.c_str(), "World")) && (llstr8.size() == 5) && !llstr8.empty());

		LLString llstr9("Hello World", sizeof("Hello World")-1, 5); // go past end
		ensure("LLString(s, n, count) goes past end", (llstr9.size() == 0) && llstr9.empty());
	}

	template<> template<>
	void string_index_object_t::test<2>()
	{
		LLString str("Len=5");
		ensure("isValidIndex failed", LLString::isValidIndex(str, 0) == TRUE &&
									  LLString::isValidIndex(str, 5) == TRUE &&
									  LLString::isValidIndex(str, 6) == FALSE);

		LLString str1;
		ensure("isValidIndex failed fo rempty string", LLString::isValidIndex(str1, 0) == FALSE);
	}

	template<> template<>
	void string_index_object_t::test<3>()
	{
		LLString str_val("               Testing the extra whitespaces   ");
		LLString::trimHead(str_val);
		ensure_equals("1: trimHead failed", str_val, "Testing the extra whitespaces   ");

		LLString str_val1("\n\t\r\n  Testing the extra whitespaces   ");
		LLString::trimHead(str_val1);
		ensure_equals("2: trimHead failed", str_val1, "Testing the extra whitespaces   ");
	}

	template<> template<>
	void string_index_object_t::test<4>()
	{
		LLString str_val("  Testing the   extra     whitespaces         ");
		LLString::trimTail(str_val);
		ensure_equals("1: trimTail failed", str_val, "  Testing the   extra     whitespaces");

		LLString str_val1("\n  Testing the extra whitespaces  \n\t\r\n   ");
		LLString::trimTail(str_val1);
		ensure_equals("2: trimTail failed", str_val1, "\n  Testing the extra whitespaces");
	}


	template<> template<>
	void string_index_object_t::test<5>()
	{
		LLString str_val("  \t \r Testing the   extra     \r\n whitespaces     \n \t    ");
		LLString::trim(str_val);
		ensure_equals("1: trim failed", str_val, "Testing the   extra     \r\n whitespaces");
	}

	template<> template<>
	void string_index_object_t::test<6>()
	{
		LLString str("Second LindenLabs");
		LLString::truncate(str, 6);
		ensure_equals("1: truncate", str, "Second");

		// further truncate more than the length
		LLString::truncate(str, 0);
		ensure_equals("2: truncate", str, "");
	}

	template<> template<>
	void string_index_object_t::test<7>()
	{
		LLString str_val("SecondLife Source");
		LLString::toUpper(str_val);
		ensure_equals("toUpper failed", str_val, "SECONDLIFE SOURCE");
	}

	template<> template<>
	void string_index_object_t::test<8>()
	{
		LLString str_val("SecondLife Source");
		LLString::toLower(str_val);
		ensure_equals("toLower failed", str_val, "secondlife source");
	}

	template<> template<>
	void string_index_object_t::test<9>()
	{
		LLString str_val("Second");
		ensure("1. isHead failed", LLString::isHead(str_val, "SecondLife Source") == TRUE);
		ensure("2. isHead failed", LLString::isHead(str_val, " SecondLife Source") == FALSE);
		LLString str_val2("");
		ensure("3. isHead failed", LLString::isHead(str_val2, "") == FALSE);
	}

	template<> template<>
	void string_index_object_t::test<10>()
	{
		LLString str_val("Hello.\n\n Lindenlabs. \n This is \na simple test.\n");
		LLString orig_str_val(str_val);
		LLString::addCRLF(str_val);
		ensure_equals("addCRLF failed", str_val, "Hello.\r\n\r\n Lindenlabs. \r\n This is \r\na simple test.\r\n");
		LLString::removeCRLF(str_val);
		ensure_equals("removeCRLF failed", str_val, orig_str_val);
	}

	template<> template<>
	void string_index_object_t::test<11>()
	{
		LLString str_val("Hello.\n\n\t \t Lindenlabs. \t\t");
		LLString orig_str_val(str_val);
		LLString::replaceTabsWithSpaces(str_val, 1);
		ensure_equals("replaceTabsWithSpaces failed", str_val, "Hello.\n\n    Lindenlabs.   ");
		LLString::replaceTabsWithSpaces(orig_str_val, 0);
		ensure_equals("replaceTabsWithSpaces failed for 0", orig_str_val, "Hello.\n\n  Lindenlabs. ");

		str_val = "\t\t\t\t";
		LLString::replaceTabsWithSpaces(str_val, 0);
		ensure_equals("replaceTabsWithSpaces failed for all tabs", str_val, "");
	}

	template<> template<>
	void string_index_object_t::test<12>()
	{
		LLString str_val("Hello.\n\n\t\t\r\nLindenlabsX.");
		LLString::replaceNonstandardASCII(str_val, 'X');
		ensure_equals("replaceNonstandardASCII failed", str_val, "Hello.\n\nXXX\nLindenlabsX.");
	}

	template<> template<>
	void string_index_object_t::test<13>()
	{
		LLString str_val("Hello.\n\t\r\nABCDEFGHIABABAB");
		LLString::replaceChar(str_val, 'A', 'X');
		ensure_equals("1: replaceChar failed", str_val, "Hello.\n\t\r\nXBCDEFGHIXBXBXB");
		LLString str_val1("Hello.\n\t\r\nABCDEFGHIABABAB");
	}

	template<> template<>
	void string_index_object_t::test<14>()
	{
		LLString str_val("Hello.\n\r\t");
		ensure("containsNonprintable failed", LLString::containsNonprintable(str_val) == TRUE);

		str_val = "ABC ";
		ensure("containsNonprintable failed", LLString::containsNonprintable(str_val) == FALSE);
	}

	template<> template<>
	void string_index_object_t::test<15>()
	{
		LLString str_val("Hello.\n\r\t Again!");
		LLString::stripNonprintable(str_val);
		ensure_equals("stripNonprintable failed", str_val, "Hello. Again!");

		str_val = "\r\n\t\t";
		LLString::stripNonprintable(str_val);
		ensure_equals("stripNonprintable resulting in empty string failed", str_val, "");
	}

	template<> template<>
	void string_index_object_t::test<16>()
	{
		BOOL value;
		LLString str_val("1");
		ensure("convertToBOOL 1 failed", LLString::convertToBOOL(str_val, value) && value);
		str_val = "T";
		ensure("convertToBOOL T failed", LLString::convertToBOOL(str_val, value) && value);
		str_val = "t";
		ensure("convertToBOOL t failed", LLString::convertToBOOL(str_val, value) && value);
		str_val = "TRUE";
		ensure("convertToBOOL TRUE failed", LLString::convertToBOOL(str_val, value) && value);
		str_val = "True";
		ensure("convertToBOOL True failed", LLString::convertToBOOL(str_val, value) && value);
		str_val = "true";
		ensure("convertToBOOL true failed", LLString::convertToBOOL(str_val, value) && value);

		str_val = "0";
		ensure("convertToBOOL 0 failed", LLString::convertToBOOL(str_val, value) && !value);
		str_val = "F";
		ensure("convertToBOOL F failed", LLString::convertToBOOL(str_val, value) && !value);
		str_val = "f";
		ensure("convertToBOOL f failed", LLString::convertToBOOL(str_val, value) && !value);
		str_val = "FALSE";
		ensure("convertToBOOL FASLE failed", LLString::convertToBOOL(str_val, value) && !value);
		str_val = "False";
		ensure("convertToBOOL False failed", LLString::convertToBOOL(str_val, value) && !value);
		str_val = "false";
		ensure("convertToBOOL false failed", LLString::convertToBOOL(str_val, value) && !value);

		str_val = "Tblah";
		ensure("convertToBOOL false failed", !LLString::convertToBOOL(str_val, value));
	}

	template<> template<>
	void string_index_object_t::test<17>()
	{
		U8 value;
		LLString str_val("255");
		ensure("1: convertToU8 failed", LLString::convertToU8(str_val, value) && value == 255);

		str_val = "0";
		ensure("2: convertToU8 failed", LLString::convertToU8(str_val, value) && value == 0);

		str_val = "-1";
		ensure("3: convertToU8 failed", !LLString::convertToU8(str_val, value));

		str_val = "256"; // bigger than MAX_U8
		ensure("4: convertToU8 failed", !LLString::convertToU8(str_val, value));
	}

	template<> template<>
	void string_index_object_t::test<18>()
	{
		S8 value;
		LLString str_val("127");
		ensure("1: convertToS8 failed", LLString::convertToS8(str_val, value) && value == 127);

		str_val = "0";
		ensure("2: convertToS8 failed", LLString::convertToS8(str_val, value) && value == 0);

		str_val = "-128";
		ensure("3: convertToS8 failed", LLString::convertToS8(str_val, value) && value == -128);

		str_val = "128"; // bigger than MAX_S8
		ensure("4: convertToS8 failed", !LLString::convertToS8(str_val, value));

		str_val = "-129"; 
		ensure("5: convertToS8 failed", !LLString::convertToS8(str_val, value));
	}

	template<> template<>
	void string_index_object_t::test<19>()
	{
		S16 value;
		LLString str_val("32767"); 
		ensure("1: convertToS16 failed", LLString::convertToS16(str_val, value) && value == 32767);

		str_val = "0";
		ensure("2: convertToS16 failed", LLString::convertToS16(str_val, value) && value == 0);

		str_val = "-32768";
		ensure("3: convertToS16 failed", LLString::convertToS16(str_val, value) && value == -32768);

		str_val = "32768"; 
		ensure("4: convertToS16 failed", !LLString::convertToS16(str_val, value));

		str_val = "-32769";
		ensure("5: convertToS16 failed", !LLString::convertToS16(str_val, value));
	}

	template<> template<>
	void string_index_object_t::test<20>()
	{
		U16 value;
		LLString str_val("65535"); //0xFFFF
		ensure("1: convertToU16 failed", LLString::convertToU16(str_val, value) && value == 65535);

		str_val = "0";
		ensure("2: convertToU16 failed", LLString::convertToU16(str_val, value) && value == 0);

		str_val = "-1"; 
		ensure("3: convertToU16 failed", !LLString::convertToU16(str_val, value));

		str_val = "65536"; 
		ensure("4: convertToU16 failed", !LLString::convertToU16(str_val, value));
	}

	template<> template<>
	void string_index_object_t::test<21>()
	{
		U32 value;
		LLString str_val("4294967295"); //0xFFFFFFFF
		ensure("1: convertToU32 failed", LLString::convertToU32(str_val, value) && value == 4294967295);

		str_val = "0";
		ensure("2: convertToU32 failed", LLString::convertToU32(str_val, value) && value == 0);

		str_val = "-1"; 
		ensure("3: convertToU32 failed", LLString::convertToU32(str_val, value) && value == 4294967295);

		str_val = "4294967296"; 
		ensure("4: convertToU32 failed", !LLString::convertToU32(str_val, value));
	}

	template<> template<>
	void string_index_object_t::test<22>()
	{
		S32 value;
		LLString str_val("2147483647"); //0x7FFFFFFF
		ensure("1: convertToS32 failed", LLString::convertToS32(str_val, value) && value == 2147483647);

		str_val = "0";
		ensure("2: convertToS32 failed", LLString::convertToS32(str_val, value) && value == 0);

		str_val = "-2147483648"; 
		ensure("3: convertToS32 failed", LLString::convertToS32(str_val, value)  && value == -2147483648);

		str_val = "2147483648"; 
		ensure("4: convertToS32 failed", !LLString::convertToS32(str_val, value));

		str_val = "-2147483649"; 
		ensure("5: convertToS32 failed", !LLString::convertToS32(str_val, value));
	}

	template<> template<>
	void string_index_object_t::test<23>()
	{
		F32 value;
		LLString str_val("2147483647"); //0x7FFFFFFF
		ensure("1: convertToF32 failed", LLString::convertToF32(str_val, value) && value == 2147483647);

		str_val = "0";
		ensure("2: convertToF32 failed", LLString::convertToF32(str_val, value) && value == 0);

		/* Need to find max/min F32 values
		str_val = "-2147483648"; 
		ensure("3: convertToF32 failed", LLString::convertToF32(str_val, value)  && value == -2147483648);

		str_val = "2147483648"; 
		ensure("4: convertToF32 failed", !LLString::convertToF32(str_val, value));

		str_val = "-2147483649"; 
		ensure("5: convertToF32 failed", !LLString::convertToF32(str_val, value));
		*/
	}

	template<> template<>
	void string_index_object_t::test<24>()
	{
		F64 value;
		LLString str_val("9223372036854775807"); //0x7FFFFFFFFFFFFFFF
		ensure("1: convertToF64 failed", LLString::convertToF64(str_val, value) && value == 9223372036854775807);

		str_val = "0";
		ensure("2: convertToF64 failed", LLString::convertToF64(str_val, value) && value == 0);

		/* Need to find max/min F64 values
		str_val = "-2147483648"; 
		ensure("3: convertToF32 failed", LLString::convertToF32(str_val, value)  && value == -2147483648);

		str_val = "2147483648"; 
		ensure("4: convertToF32 failed", !LLString::convertToF32(str_val, value));

		str_val = "-2147483649"; 
		ensure("5: convertToF32 failed", !LLString::convertToF32(str_val, value));
		*/
	}

	template<> template<>
	void string_index_object_t::test<25>()
	{
		char* str1 = NULL;
		char* str2 = NULL;

		ensure("1: compareStrings failed", LLString::compareStrings(str1, str2) == 0);
		str2 = "A";
		ensure("2: compareStrings failed", LLString::compareStrings(str1, str2) > 0);
		ensure("3: compareStrings failed", LLString::compareStrings(str2, str1) < 0);
		
		str1 = "A is smaller than B";
		str2 = "B is greater than A";
		ensure("4: compareStrings failed", LLString::compareStrings(str1, str2) < 0);

		str2 = "A is smaller than B";
		ensure("5: compareStrings failed", LLString::compareStrings(str1, str2) == 0);
	}

	template<> template<>
	void string_index_object_t::test<26>()
	{
		char* str1 = NULL;
		char* str2 = NULL;

		ensure("1: compareInsensitive failed", LLString::compareInsensitive(str1, str2) == 0);
		str2 = "A";
		ensure("2: compareInsensitive failed", LLString::compareInsensitive(str1, str2) > 0);
		ensure("3: compareInsensitive failed", LLString::compareInsensitive(str2, str1) < 0);
		
		str1 = "A is equal to a";
		str2 = "a is EQUAL to A";
		ensure("4: compareInsensitive failed", LLString::compareInsensitive(str1, str2) == 0);
	}

	template<> template<>
	void string_index_object_t::test<27>()
	{
		LLString lhs_str("PROgraM12files");
		LLString rhs_str("PROgram12Files");
		ensure("compareDict 1 failed", LLString::compareDict(lhs_str, rhs_str) < 0);
		ensure("precedesDict 1 failed", LLString::precedesDict(lhs_str, rhs_str) == TRUE);
		
		lhs_str = "PROgram12Files";
		rhs_str = "PROgram12Files";
		ensure("compareDict 2 failed", LLString::compareDict(lhs_str, rhs_str) == 0);
		ensure("precedesDict 2 failed", LLString::precedesDict(lhs_str, rhs_str) == FALSE);

		lhs_str = "PROgram12Files";
		rhs_str = "PROgRAM12FILES";
		ensure("compareDict 3 failed", LLString::compareDict(lhs_str, rhs_str) > 0);
		ensure("precedesDict 3 failed", LLString::precedesDict(lhs_str, rhs_str) == FALSE);
	}

	template<> template<>
	void string_index_object_t::test<28>()
	{
		char str1[] = "First String...";
		char str2[100];

		LLString::copy(str2, str1, 100);
		ensure("LLString::copy with enough dest lenght failed", strcmp(str2, str1) == 0);
		LLString::copy(str2, str1, sizeof("First"));
		ensure("LLString::copy with less dest lenght failed", strcmp(str2, "First") == 0);
	}

	template<> template<>
	void string_index_object_t::test<29>()
	{
		LLString str1 = "This is the sentence...";
		LLString str2 = "This is the ";
		LLString str3 = "first ";
		LLString str4 = "This is the first sentence...";
		LLString str5 = "This is the sentence...first ";
		LLString dest;

		dest = str1;
		LLString::copyInto(dest, str3, str2.length());
		ensure("LLString::copyInto insert failed", dest == str4);

		dest = str1;
		LLString::copyInto(dest, str3, dest.length());
		ensure("LLString::copyInto append failed", dest == str5);
	}
}

