/** 
 * @file llxfer_tut.cpp
 * @author Moss
 * @date 2007-04-17
 *
 * Copyright (c) 2007-2007, Linden Research, Inc.
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

#include "llxfer_file.h"

#include <string>

namespace tut
{
	struct llxfer_data
	{
	};
	typedef test_group<llxfer_data> llxfer_test;
	typedef llxfer_test::object llxfer_object;
	tut::llxfer_test llxfer("llxfer");

	template<> template<>
	void llxfer_object::test<1>()
	{
		// test that he handle an oversized filename correctly.
		LLString oversized_filename;
		U32 i;
		for (i=0; i<LL_MAX_PATH*2; ++i) // create oversized filename
		{
			oversized_filename += 'X';
		}

		LLXfer_File xff(oversized_filename, FALSE, 1);
		ensure("oversized local_filename nul-terminated",
		       strnlen(xff.getName(), LL_MAX_PATH) < LL_MAX_PATH);
	}
}
