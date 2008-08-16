/** 
 * @file llcountdown.h
 * @brief LLCountdown class header file
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

#ifndef LL_LLCOUNTDOWN_H
#define LL_LLCOUNTDOWN_H

#include "lltimer.h"
#include "lltextbox.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Class LLCountdown
//
// This class draws a text box that counts down from seconds to zero,
// with a little bit of slop.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class LLCountdown : public LLTextBox
{
protected:
	LLString mBasePrefix;
	LLString mBaseSuffix;
	LLTimer mExpired;
	F32 mSeconds;
	F32 mHalfway;

	void buildLabel();

public:
	LLCountdown(const std::string& name, const LLRect& rect, F32 seconds,
				const char* base_prefix = NULL,
				const char* base_suffix = NULL);
	virtual ~LLCountdown();

	// cannot be const because our timers are not const. Damn
	// const-incorrectness.
	BOOL isExpired();

	virtual void draw();
};


#endif // LL_LLCOUNTDOWN_H
