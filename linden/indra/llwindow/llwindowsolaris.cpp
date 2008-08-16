/** 
 * @file fmodwrapper.cpp
 * @brief dummy source file for building a shared library to wrap libfmod.a
 *
 * $LicenseInfo:firstyear=2005&license=viewergpl$
 * 
 * Copyright (c) 2005-2008, Linden Research, Inc.
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
 * online at http://secondlifegrid.net/programs/open_source/licensing/flossexception
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

#if LL_SOLARIS

#include "linden_common.h"
#include "indra_constants.h"

#include "llwindowsolaris.h"
#include "llgl.h"
#include "llglheaders.h"

//
// LLWindowSolaris
//
LLWindowSolaris::LLWindowSolaris(char *title, char *name, S32 x, S32 y, S32 width, S32 height,
							 U32 flags,  BOOL fullscreen, BOOL clearBg,
							 BOOL disable_vsync, BOOL use_gl, BOOL ignore_pixel_depth)
	: LLWindow(fullscreen, flags)
{
	llerrs << "Solaris window not yet supported" << llendl;
}


LLWindowSolaris::~LLWindowSolaris()
{
}

void LLWindowSolaris::swapBuffers()
{
}

#endif // LL_SOLARIS
