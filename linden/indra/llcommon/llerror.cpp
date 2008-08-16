/** 
 * @file llerror.cpp
 * @brief Function to crash.
 *
 * Copyright (c) 2001-2007, Linden Research, Inc.
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

#include "llerror.h"

LLErrorBuffer gErrorBuffer;
LLErrorStream gErrorStream(&gErrorBuffer);


void _llcrash_and_loop()
{
	// Now, we go kaboom!
	U32* crash = NULL;

	*crash = 0;

	while(TRUE)
	{
	
		// Loop forever, in case the crash didn't work?
	}
}

LLScopedErrorLevel::LLScopedErrorLevel(LLErrorBuffer::ELevel error_level)
{
	mOrigErrorLevel = gErrorStream.getErrorLevel();
	gErrorStream.setErrorLevel(error_level);
}


LLScopedErrorLevel::~LLScopedErrorLevel()
{
	gErrorStream.setErrorLevel(mOrigErrorLevel);
}
