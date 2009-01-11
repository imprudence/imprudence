/** 
 * @file llrendersphere.h
 * @brief interface for the LLRenderSphere class.
 *
 * $LicenseInfo:firstyear=2001&license=internal$
 * 
 * Copyright (c) 2001-2009, Linden Research, Inc.
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

#ifndef LL_LLRENDERSPHERE_H
#define LL_LLRENDERSPHERE_H

#include "llmath.h"
#include "v3math.h"
#include "v4math.h"
#include "m3math.h"
#include "m4math.h"
#include "v4color.h"
#include "llgl.h"

void lat2xyz(LLVector3 * result, F32 lat, F32 lon);			// utility routine

class LLRenderSphere  
{
public:
	LLGLuint	mDList[5];

	void prerender();
	void cleanupGL();
	void render(F32 pixel_area);		// of a box of size 1.0 at that position
	void render();						// render at highest LOD
};

extern LLRenderSphere gSphere;
#endif
