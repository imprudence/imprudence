/** 
 * @file llrandom_tut.cpp
 * @author Phoenix
 * @date 2007-01-25
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

#include "linden_common.h"
#include "llrand.h"
#include "lltut.h"


namespace tut
{
	struct random
	{
	};

	typedef test_group<random> random_t;
	typedef random_t::object random_object_t;
	tut::random_t tut_random("random");

	template<> template<>
	void random_object_t::test<1>()
	{
		F32 number = 0.0f;
		for(S32 ii = 0; ii < 100000; ++ii)
		{
			number = ll_frand();
			ensure("frand >= 0", (number >= 0.0f));
			ensure("frand < 1", (number < 1.0f));
		}
	}

	template<> template<>
	void random_object_t::test<2>()
	{
		F32 number = 0.0f;
		for(S32 ii = 0; ii < 100000; ++ii)
		{
			number = ll_frand(2.0f) - 1.0f;
			ensure("frand >= 0", (number >= -1.0f));
			ensure("frand < 1", (number <= 1.0f));
		}
	}
}
