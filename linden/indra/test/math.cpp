/** 
 * @file math.cpp
 * @author Phoenix
 * @date 2005-09-26
 * @brief Tests for the llmath library.
 *
 * Copyright (c) 2005-2007, Linden Research, Inc.
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
#include "lltut.h"

#include "llmath.h"
#include "lluuid.h"

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
