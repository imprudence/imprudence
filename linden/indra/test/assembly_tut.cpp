/** 
 * @file assembly_tut.cpp
 * @brief Tests for lscript/lscript_execute_mono
 *
 * $LicenseInfo:firstyear=2007&license=internal$
 * 
 * Copyright (c) 2007-2009, Linden Research, Inc.
 * 
 * The following source code is PROPRIETARY AND CONFIDENTIAL. Use of
 * this source code is governed by the Linden Lab Source Code Disclosure
 * Agreement ("Agreement") previously entered between you and Linden
 * Lab. By accessing, using, copying, modifying or distributing this
 * software, you acknowledge that you have been informed of your
 * obligations under the Agreement and agree to abide by those obligations.
 * 
 * ALL LINDEN LAB SOURCE CODE IS PROVIDED "AS IS." LINDEN LAB MAKES NO
 * WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
 * COMPLETENESS OR PERFORMANCE.
 * $/LicenseInfo$
 */

#include "linden_common.h"
#include "lltut.h"

#include "lscript_execute_mono/assembly.cpp" 

#include <string>
#include <map>


namespace tut
{
	struct assembly_data
	{
	};
	typedef test_group<assembly_data> assembly_test;
	typedef assembly_test::object assembly_object;
	tut::assembly_test assembly("assembly");

	// Create some random junk, its not important at the moment.
	U8* createTestBuffer(U32 size)
	{
		U8* result = new U8[size];	
		memset(result, 0, size);	
		return result;
	}

	template<> template<>
	void assembly_object::test<1>()
	{
		ensure("Assembly map not empty before creation", sAssemblyMap.empty());
		{
			U32 size = 10;
			U8* buffer = createTestBuffer(size);
			boost::intrusive_ptr<LLAssembly> assembly1 = LLAssembly::create("Assembly1",
				buffer, size);

			ensure("created assembly with ref count of 1",
				assembly1->mRefCount == 1);
				
			ensure("sAssemblyMap.size() == 1", sAssemblyMap.size() == 1);
			
			size = 200;
			buffer = createTestBuffer(size);
			boost::intrusive_ptr<LLAssembly> assembly2 = LLAssembly::create("Assembly2",
				buffer, size);

			ensure("created second assembly with ref count == 1",
				assembly2->mRefCount == 1);
			
			ensure("sAssemblyMap.size() == 2", sAssemblyMap.size() == 2);
		
			size = 10;
			buffer = createTestBuffer(size);
			boost::intrusive_ptr<LLAssembly> assembly3 = LLAssembly::create("Assembly1",
				buffer, size);
			
			ensure("created with ref count of 2", 
				assembly3->mRefCount == 2);
		
			ensure("sAssemblyMap.size() == 2", sAssemblyMap.size() == 2);
		}

		ensure("Assembly map not empty after creation", sAssemblyMap.empty());
	}
}
