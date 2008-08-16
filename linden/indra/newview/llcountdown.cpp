/** 
 * @file llcountdown.cpp
 * @brief Implementation of the countdown box.
 *
 * Copyright (c) 2002-2007, Linden Research, Inc.
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

#include "llviewerprecompiledheaders.h"

#include "llcountdown.h"

///----------------------------------------------------------------------------
/// Local function declarations, constants, enums, and typedefs
///----------------------------------------------------------------------------

const LLString DEFAULT_BASE_PREFIX("This dialag will close in ");
const LLString DEFAULT_BASE_SUFFIX(" seconds");

///----------------------------------------------------------------------------
/// Class llcountdown
///----------------------------------------------------------------------------

// Default constructor
LLCountdown::LLCountdown(const std::string& name, const LLRect& rect, F32 seconds,
						 const char* base_prefix, const char* base_suffix) :
	LLTextBox(name, rect),
	mSeconds(seconds),
	mHalfway(seconds/2-0.99f)
{
	mExpired.reset();
	if(base_prefix)
	{
		mBasePrefix.assign(base_prefix);
	}
	else
	{
		mBasePrefix = DEFAULT_BASE_PREFIX;
	}
	if(base_suffix)
	{
		mBaseSuffix.assign(base_suffix);
	}
	else
	{
		mBaseSuffix = DEFAULT_BASE_SUFFIX;
	}
	S32 s = llfloor(mSeconds);
	LLString buffer = mBasePrefix + llformat("%d",s) + mBaseSuffix;
	setText(buffer);
}

// Destroys the object
LLCountdown::~LLCountdown()
{
}

BOOL LLCountdown::isExpired()
{
	if(mExpired.getElapsedTimeF32() > mSeconds)
	{
		return TRUE;
	}
	return FALSE;
}

void LLCountdown::draw()
{
	F32 elapsed = mExpired.getElapsedTimeF32();
	if(elapsed > mSeconds)
	{
		LLString buffer = mBasePrefix + "0" + mBaseSuffix;
		setText(buffer);
	}
	else if(elapsed > mHalfway)
	{
		S32 seconds = llfloor(llmax((mSeconds - elapsed), 0.0f));
		LLString buffer = mBasePrefix + llformat("%d",seconds) + mBaseSuffix;
		setText(buffer);
	}
	LLTextBox::draw();
}

///----------------------------------------------------------------------------
/// Local function definitions
///----------------------------------------------------------------------------
