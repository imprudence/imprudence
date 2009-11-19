/** 
 * @file llcolorscheme.cpp
 * @brief Implementation of colors used for map
 *
 * $LicenseInfo:firstyear=2003&license=viewergpl$
 * 
 * Copyright (c) 2003-2009, Linden Research, Inc.
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

#include "llviewerprecompiledheaders.h"

#include "llcolorscheme.h"

#include "llviewercontrol.h"

LLColor4 gTrackColor;
LLColor4 gLandmarkMapColor;
LLColor4 gLocationMapColor;
LLColor4 gTelehubMapColor;
LLColor4 gEventColor;
LLColor4 gPopularColor;
LLColor4 gPickColor;
LLColor4 gHomeColor;
LLColor4 gDisabledTrackColor;

void init_colors()
{
	gTrackColor	= LLColor4::red;
	gDisabledTrackColor.setVec(		0.5f, 0.f, 0.f, 1.f );
	gLandmarkMapColor = LLColor4::red;
	gLocationMapColor.setVec(		0.f, 0.72f, 1.f, 1.f);
	gTelehubMapColor.setVec(		0.9f, 0.9f, 0.f, 1.f); // dk yellow
	gEventColor.setVec(				1.f, 0.5f, 1.f, 1.f );
	gPopularColor.setVec(			1.f, 0.0f, 0.f, 1.f );
	gPickColor.setVec(				1.f, 0.0f, 0.f, 1.f );
	gHomeColor.setVec(				0.f, 0.5f, 1.f, 1.f );
}
