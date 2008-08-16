/** 
 * @file llwindowlinux.cpp
 * @brief Platform-dependent implementation of llwindow
 *
 * Copyright (c) 2001-2007, Linden Research, Inc.
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

#if LL_LINUX

#include "linden_common.h"
#include "indra_constants.h"

#include "llwindowlinux.h"
#include "llgl.h"
#include "llglheaders.h"

//
// LLWindowLinux
//
LLWindowLinux::LLWindowLinux(char *title, char *name, S32 x, S32 y, S32 width, S32 height,
							 U32 flags,  BOOL fullscreen, BOOL clearBg,
							 BOOL disable_vsync, BOOL use_gl, BOOL ignore_pixel_depth)
	: LLWindow(fullscreen, flags)
{
	llerrs << "Linux window not yet supported" << llendl;
}


LLWindowLinux::~LLWindowLinux()
{
}

void LLWindowLinux::swapBuffers()
{
}

#endif // LL_LINUX
