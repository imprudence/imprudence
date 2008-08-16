/** 
 * @file llvelocitybar.cpp
 * @brief A user interface widget that displays user energy level
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

#include "llviewerprecompiledheaders.h"

#include "llui.h"
#include "llgl.h"
#include "llfontgl.h"

#include "llagent.h"

#include "llvelocitybar.h"

EWidgetType LLVelocityBar::getWidgetType() const
{
	return WIDGET_TYPE_VELOCITY_BAR;
}

LLString LLVelocityBar::getWidgetTag() const
{
	return LL_VELOCITY_BAR_TAG;
}

const char tab = '\t';

void LLVelocityBar::draw()
{
	const S32 BAR_TOP = 24;
	const S32 BAR_BOTTOM = 20;
	const S32 TICK_BOTTOM = 15;
	const S32 TICK_WIDTH = 2;
	const S32 MAGIC_CHAR_WIDTH = 6;

	S32 left, top, right, bottom;

	if (!getVisible())
	{
		return;
	}

//	const F32 MS_TO_SECONDS = 0.001f;
//	const S32 WIDTH_IN_MS = 66;
//	const F32 WIDGET_TIME_FOR_WIDTH = WIDTH_IN_MS * MS_TO_SECONDS;

	F32 velocity = gAgent.getVelocity().magVec();

	LLColor4 bar_color(0.0f, 1.0f, 0.0f, 1.f);

	// 
	//  Set bar color 
	//

	LLGLSNoTexture gls_no_texture;

	// draw background box
	// glColor4f(0.f, 0.f, 0.f, 0.3f);
	// gl_rect_2d(0, mRect.getHeight(), mRect.getWidth(), 0);

	// draw white lines for special times 
	// (60 hz = 16 ms, 30 hz = 33 ms, 15 hz = 66 ms)
	LLColor4 color(1.f, 1.f, 1.f, 1.f);

	top = BAR_BOTTOM - 1;
	bottom = TICK_BOTTOM;

	left = 0;
	right = left + TICK_WIDTH;
	gl_rect_2d(left, top, right, bottom, color);

	left = (S32) (mRect.getWidth() * 1 / 6);
	right = left + TICK_WIDTH;
	gl_rect_2d(left, top, right, bottom, color);

	left = (S32) (mRect.getWidth() * 2 / 6);
	right = left + TICK_WIDTH;
	gl_rect_2d(left, top, right, bottom, color);

	left = (S32) (mRect.getWidth() * 3 / 6);
	right = left + TICK_WIDTH;
	gl_rect_2d(left, top, right, bottom, color);

	left = (S32) (mRect.getWidth() * 4 / 6);
	right = left + TICK_WIDTH;
	gl_rect_2d(left, top, right, bottom, color);

	left = (S32) (mRect.getWidth() * 5 / 6);
	right = left + TICK_WIDTH;
	gl_rect_2d(left, top, right, bottom, color);

	left = (S32) (mRect.getWidth() * 6 / 6);
	right = left + TICK_WIDTH;
	gl_rect_2d(left, top, right, bottom, color);


	// draw labels for the bar
	LLGLSTexture gls_texture;

	top = BAR_TOP + 15;
	left = 0;
	char str[80];		/* Flawfinder: ignore */
	snprintf(str, sizeof(str), "Velocity %.3fm/s", velocity);		/* Flawfinder: ignore */
	LLFontGL::sMonospace->renderUTF8(str, 0, left, top, color, LLFontGL::LEFT, LLFontGL::TOP);

	top = TICK_BOTTOM;

	left = - MAGIC_CHAR_WIDTH/2;
	LLFontGL::sMonospace->renderUTF8("0", 0, left, top, color, LLFontGL::LEFT, LLFontGL::TOP);

	left = (mRect.getWidth()*1 / 6) - MAGIC_CHAR_WIDTH * 2;
	LLFontGL::sMonospace->renderUTF8("1", 0, left, top, color, LLFontGL::LEFT, LLFontGL::TOP);

	left = (mRect.getWidth()*2 / 6) - MAGIC_CHAR_WIDTH;
	LLFontGL::sMonospace->renderUTF8("2", 0, left, top, color, LLFontGL::LEFT, LLFontGL::TOP);

	left = (mRect.getWidth()*3 / 6) - MAGIC_CHAR_WIDTH * 2;
	LLFontGL::sMonospace->renderUTF8("3", 0, left, top, color, LLFontGL::LEFT, LLFontGL::TOP);

	left = (mRect.getWidth()*4 / 6) - MAGIC_CHAR_WIDTH;
	LLFontGL::sMonospace->renderUTF8("4", 0, left, top, color, LLFontGL::LEFT, LLFontGL::TOP);

	left = (mRect.getWidth()*5 / 6) - MAGIC_CHAR_WIDTH * 2;
	LLFontGL::sMonospace->renderUTF8("5", 0, left, top, color, LLFontGL::LEFT, LLFontGL::TOP);

	left = (mRect.getWidth()*6 / 6) - MAGIC_CHAR_WIDTH * 3;
	LLFontGL::sMonospace->renderUTF8("6 m/s", 0, left, top, color, LLFontGL::LEFT, LLFontGL::TOP);

	// draw idle time
	top = BAR_TOP;
	bottom = BAR_BOTTOM;

	//  Draw energy level
	left = 0;
	right = (S32) (left + velocity * 0.33333f * mRect.getWidth() / 2.f);
	gl_rect_2d(left, top, right, bottom, bar_color);
}
