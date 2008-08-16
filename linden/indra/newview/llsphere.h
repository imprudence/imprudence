/** 
 * @file llsphere.h
 * @brief interface for the LLSphere class.
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

#ifndef LL_LLSPHERE_H
#define LL_LLSPHERE_H

//#include "vmath.h"
#include "llmath.h"
#include "v3math.h"
#include "v4math.h"
#include "m3math.h"
#include "m4math.h"
#include "v4color.h"
#include "llgl.h"

void lat2xyz(LLVector3 * result, F32 lat, F32 lon);			// utility routine

class LLSphere  
{
public:
	LLGLuint	mDList[5];

	void prerender();
	void cleanupGL();
	void render(F32 pixel_area);		// of a box of size 1.0 at that position
	void render();						// render at highest LOD
};

extern LLSphere gSphere;
#endif
