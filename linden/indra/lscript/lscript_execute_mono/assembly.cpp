/**
 * @file llassembly.cpp
 * @brief keeps track of user scripts, ensures that only one copy exists for each
 *
 * $LicenseInfo:firstyear=2007&license=viewergpl$
 * 
 * Copyright (c) 2007-2009, Linden Research, Inc.
 * 
 * Second Life Viewer Source Code
 * The source code in this file ("Source Code") is provided by Linden Lab
 * to you under the terms of the GNU General Public License, version 2.0
 * ("GPL"), unless you have obtained a separate licensing agreement
 * ("Other License"), formally executed by you and Linden Lab.  Terms of
 * the GPL can be found in doc/GPL-license.txt in this distribution, or
 * online at http://secondlifegrid.net/programs/open_source/licensing/gplv2
 * 
 * There are special exceptions to the terms and conditions of the GPL as
 * it is applied to this Source Code. View the full text of the exception
 * in the file doc/FLOSS-exception.txt in this software distribution, or
 * online at
 * http://secondlifegrid.net/programs/open_source/licensing/flossexception
 * 
 * By copying, modifying or distributing this software, you acknowledge
 * that you have read and understood your obligations described above,
 * and agree to abide by those obligations.
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


