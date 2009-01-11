/**
 * @file llassembly.cpp
 * @brief keeps track of user scripts, ensures that only one copy exists for each
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


#include "llassembly.h"
#include <map>


static std::map<std::string, LLAssembly*> sAssemblyMap;

boost::intrusive_ptr<LLAssembly> LLAssembly::create(const std::string& name, 
													const U8* buffer, U32 size)
{
	std::map<std::string, LLAssembly*>::iterator it = sAssemblyMap.find(name);
	if (it != sAssemblyMap.end())
	{
		return it->second;
	}
	else
	{
		LLAssembly* a = new LLAssembly(name, buffer, size);
		sAssemblyMap[name] = a;
		return a;
	}
}


LLAssembly::LLAssembly(const std::string &name, const U8* buffer, U32 size) : 
	mName(name), mRefCount(0), mBuffer(buffer, buffer+size)
{
}

LLAssembly::~LLAssembly() 
{
	std::map<std::string, LLAssembly*>::iterator it = sAssemblyMap.find(mName);
	if (it != sAssemblyMap.end())
	{
		sAssemblyMap.erase(it);
	}
}


void intrusive_ptr_add_ref(LLAssembly* p)
{
	++(p->mRefCount);
}

void intrusive_ptr_release(LLAssembly* p)
{
	if (0 == --(p->mRefCount))
	{
		delete p;
	}
}


