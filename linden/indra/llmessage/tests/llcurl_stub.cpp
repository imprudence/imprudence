/**
 * @file llcurl_stub.cpp
 * @brief stub class to allow unit testing
 *
 * $LicenseInfo:firstyear=2008&license=viewergpl$
 * 
 * Copyright (c) 2008-2009, Linden Research, Inc.
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

#include "linden_common.h"
#include "llcurl.h"

LLCurl::Responder::Responder() : mReferenceCount(0)
{
}

void LLCurl::Responder::completed(U32 status, std::basic_string<char, std::char_traits<char>, std::allocator<char> > const &reason,
								  LLSD const& mContent)
{
	if (isGoodStatus(status))
	{
		result(mContent);
	}
	else
	{
		error(status, reason, mContent);
	}
}

void LLCurl::Responder::completedHeader(unsigned,
										std::basic_string<char, std::char_traits<char>, std::allocator<char> > const&,
										LLSD const&)
{
}

void LLCurl::Responder::completedRaw(unsigned,
									 std::basic_string<char, std::char_traits<char>, std::allocator<char> > const&,
									 LLChannelDescriptors const&,
									 boost::shared_ptr<LLBufferArray> const&)
{
}

void LLCurl::Responder::error(unsigned,
							  std::basic_string<char, std::char_traits<char>, std::allocator<char> > const&,
							  LLSD const&)
{
}

LLCurl::Responder::~Responder ()
{
}

void LLCurl::Responder::error(unsigned,
							  std::basic_string<char, std::char_traits<char>, std::allocator<char> > const&)
{
}

void LLCurl::Responder::result(LLSD const&)
{
}

namespace boost
{
	void intrusive_ptr_add_ref(LLCurl::Responder* p)
	{
		++p->mReferenceCount;
	}

	void intrusive_ptr_release(LLCurl::Responder* p)
	{
		if(p && 0 == --p->mReferenceCount)
		{
			delete p;
		}
	}
};

