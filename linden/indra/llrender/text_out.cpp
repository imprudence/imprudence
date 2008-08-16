/** 
 * @file text_out.cpp
 * @brief Text rendering implementation
 *
 * Copyright (c) 2000-2007, Linden Research, Inc.
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

/* DEPRECATED - Use LLFontGL for raw rendering, or make an LLTextBox to hold the text */

#include "linden_common.h"

#include "text_out.h"
#include "llfontgl.h"
//#include "vmath.h"
#include "v4color.h"

#define MAX_TEXT_LINE_LENGTH	(256)
#define MAX_TEXT_LINES			(128)

extern S32 gWindowWidth;
extern S32 gWindowHeight;

S32 gNumTextLines = 0;

LLColor4 gCurrentColor;
//U8	gCurrentBGColor[] = { 32, 32, 32 };

//F32 gCurrentScale = 10.f;

typedef struct s_text_line
{
//	const LLFontGL	*font;
//	U8		color[3];
//	U8		bg_color[3];
	char	line[MAX_TEXT_LINE_LENGTH];
	S32		x, y;
//	ETextModifiers	tm;
//	ETextColorModifiers fcm;
//	ETextColorModifiers bcm;
} TEXT_LINE;

TEXT_LINE	gTextLine[MAX_TEXT_LINES];

void set_text_color( const LLColor4& color )
{
	gCurrentColor = color;
}
/*
void set_text_bg_color(S32 red, S32 green, S32 blue)
{
	gCurrentBGColor[0] = red;
	gCurrentBGColor[1] = green;
	gCurrentBGColor[2] = blue;
}
*/
void reset_num_text_lines(void)
{
	gNumTextLines = 0;
}

void add_text(S32 x, S32 y, char *text) // , ETextModifiers tm, ETextColorModifiers fcm, ETextColorModifiers bcm)
{
	if (gNumTextLines > MAX_TEXT_LINES) return;

	TEXT_LINE *linep = &gTextLine[gNumTextLines++];	

	strcpy(linep->line, text);
	linep->x = x;
	linep->y = y;
}


// Writes text on the screen.  Deprecated, don't write new code using this.
void show_text_gl(void)
{
	S32 i;

	TEXT_LINE *linep;

	if (gNumTextLines > MAX_TEXT_LINES)
	{
		gNumTextLines = MAX_TEXT_LINES;
	}

	for (i = 0; i < gNumTextLines; i++)
	{
		linep = &gTextLine[i];

		LLFontGL::sMonospace->renderUTF8(linep->line, 0, (F32)linep->x, (F32)linep->y, gCurrentColor,
										 LLFontGL::LEFT, LLFontGL::TOP,
										 LLFontGL::NORMAL, S32_MAX, S32_MAX, NULL, FALSE);
	}
	gNumTextLines = 0;
}

