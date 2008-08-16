/** 
 * @file math.cpp
 * @author Phoenix
 * @date 2005-09-26
 * @brief Tests for the llmath library.
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
#include "lltut.h"

#include "llmath.h"
#include "lluuid.h"
#include "llcrc.h"

namespace tut
{
	struct math_data
	{
	};
	typedef test_group<math_data> math_test;
	typedef math_test::object math_object;
	tut::math_test tm("basic_linden_math");

	template<> template<>
	void math_object::test<1>()
	{
		S32 val = 89543;
		val = llabs(val);
		ensure("integer absolute value 1", (89543 == val));
		val = -500;
		val = llabs(val);
		ensure("integer absolute value 2", (500 == val));
	}

	template<> template<>
	void math_object::test<2>()
	{
		F32 val = -2583.4f;
		val = llabs(val);
		ensure("float absolute value 1", (2583.4f == val));
		val = 430903.f;
		val = llabs(val);
		ensure("float absolute value 2", (430903.f == val));
	}

	template<> template<>
	void math_object::test<3>()
	{
		F64 val = 387439393.987329839;
		val = llabs(val);
		ensure("double absolute value 1", (387439393.987329839 == val));
		val = -8937843.9394878;
		val = llabs(val);
		ensure("double absolute value 2", (8937843.9394878 == val));
	}

	template<> template<>
	void math_object::test<4>()
	{
		F32 val = 430903.9f;
		S32 val1 = lltrunc(val);
		ensure("float truncate value 1", (430903 == val1));
		val = -2303.9f;
		val1 = lltrunc(val);
		ensure("float truncate value 2", (-2303  == val1));
	}

	template<> template<>
	void math_object::test<5>()
	{
		F64 val = 387439393.987329839 ;
		S32 val1 = lltrunc(val);
		ensure("float truncate value 1", (387439393 == val1));
		val = -387439393.987329839;
		val1 = lltrunc(val);
		ensure("float truncate value 2", (-387439393  == val1));
	}

	template<> template<>
	void math_object::test<6>()
	{
		F32 val = 430903.2f;
		S32 val1 = llfloor(val);
		ensure("float llfloor value 1", (430903 == val1));
		val = -430903.9f;
		val1 = llfloor(val);
		ensure("float llfloor value 2", (-430904 == val1));
	}

	template<> template<>
	void math_object::test<7>()
	{
		F32 val = 430903.2f;
		S32 val1 = llceil(val);
		ensure("float llceil value 1", (430904 == val1));
		val = -430903.9f;
		val1 = llceil(val);
		ensure("float llceil value 2", (-430903 == val1));
	}

	template<> template<>
	void math_object::test<8>()
	{
		F32 val = 430903.2f;
		S32 val1 = llround(val);
		ensure("float llround value 1", (430903 == val1));
		val = -430903.9f;
		val1 = llround(val);
		ensure("float llround value 2", (-430904 == val1));
	}

	template<> template<>
	void math_object::test<9>()
	{
		F32 val = 430905.2654f, nearest = 100.f;
		val = llround(val, nearest);
		ensure("float llround value 1", (430900 == val));
		val = -430905.2654f, nearest = 10.f;
		val = llround(val, nearest);
		ensure("float llround value 1", (-430910 == val));
	}

	template<> template<>
	void math_object::test<10>()
	{
		F64 val = 430905.2654, nearest = 100.0;
		val = llround(val, nearest);
		ensure("double llround value 1", (430900 == val));
		val = -430905.2654, nearest = 10.0;
		val = llround(val, nearest);
		ensure("double llround value 1", (-430910.00000 == val));
	}

	template<> template<>
	void math_object::test<11>()
	{
		const F32	F_PI		= 3.1415926535897932384626433832795f;
		F32 angle = 3506.f;
		angle =  llsimple_angle(angle);
		ensure("llsimple_angle  value 1", (angle <=F_PI && angle >= -F_PI));
		angle = -431.f;
		angle =  llsimple_angle(angle);
		ensure("llsimple_angle  value 1", (angle <=F_PI && angle >= -F_PI));
	}
}

namespace tut
{
	struct uuid_data
	{
		LLUUID id;
	};
	typedef test_group<uuid_data> uuid_test;
	typedef uuid_test::object uuid_object;
	tut::uuid_test tu("uuid");

	template<> template<>
	void uuid_object::test<1>()
	{
		ensure("uuid null", id.isNull());
		id.generate();
		ensure("generate not null", id.notNull());
		id.setNull();
		ensure("set null", id.isNull());
	}

	template<> template<>
	void uuid_object::test<2>()
	{
		id.generate();
		LLUUID a(id);
		ensure_equals("copy equal", id, a);
		a.generate();
		ensure_not_equals("generate not equal", id, a);
		a = id;
		ensure_equals("assignment equal", id, a);
	}

	template<> template<>
	void uuid_object::test<3>()
	{
		id.generate();
		LLUUID copy(id);
		LLUUID mask;
		mask.generate();
		copy ^= mask;
		ensure_not_equals("mask not equal", id, copy);
		copy ^= mask;
		ensure_equals("mask back", id, copy);
	}

	template<> template<>
	void uuid_object::test<4>()
	{
		id.generate();
		std::string id_str = id.asString();
		LLUUID copy(id_str.c_str());
		ensure_equals("string serialization", id, copy);
	}
	
}

namespace tut
{
	struct crc_data
	{
	};
	typedef test_group<crc_data> crc_test;
	typedef crc_test::object crc_object;
	tut::crc_test tc("crc");

	template<> template<>
	void crc_object::test<1>()
	{
		/* Test buffer update and individual char update */
		const char TEST_BUFFER[] = "hello &#$)$&Nd0";
		LLCRC c1, c2;
		c1.update((U8*)TEST_BUFFER, sizeof(TEST_BUFFER) - 1);
		char* rh = (char*)TEST_BUFFER;
		while(*rh != '\0')
		{
			c2.update(*rh);
			++rh;
		}
		ensure_equals("crc update 1", c1.getCRC(), c2.getCRC());
	}

	template<> template<>
	void crc_object::test<2>()
	{
		/* Test mixing of buffer and individual char update */
		const char TEST_BUFFER1[] = "Split Buffer one $^%$%#@$";
		const char TEST_BUFFER2[] = "Split Buffer two )(8723#5dsds";
		LLCRC c1, c2;
		c1.update((U8*)TEST_BUFFER1, sizeof(TEST_BUFFER1) - 1);
		char* rh = (char*)TEST_BUFFER2;
		while(*rh != '\0')
		{
			c1.update(*rh);
			++rh;
		}

		rh = (char*)TEST_BUFFER1;
		while(*rh != '\0')
		{
			c2.update(*rh);
			++rh;
		}
		c2.update((U8*)TEST_BUFFER2, sizeof(TEST_BUFFER2) - 1);

		ensure_equals("crc update 2", c1.getCRC(), c2.getCRC());
	}
}
