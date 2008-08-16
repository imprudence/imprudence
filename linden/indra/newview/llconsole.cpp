/** 
 * @file llconsole.cpp
 * @brief a scrolling console output device
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

#include "llconsole.h"

// linden library includes
#include "llmath.h"
#include "llviewercontrol.h"
#include "llcriticaldamp.h"
#include "llfontgl.h"
#include "llgl.h"
#include "llui.h"
#include "llviewerimage.h"
#include "llviewerimagelist.h"
#include "llviewerwindow.h"
#include "llfontgl.h"
#include "llmath.h"

#include "llstartup.h"
#include "viewer.h"

LLConsole* gConsole = NULL;  // Created and destroyed in LLViewerWindow.

const F32 FADE_DURATION = 2.f;
const S32 MIN_CONSOLE_WIDTH = 200;
 
LLConsole::LLConsole(const std::string& name, const U32 max_lines, const LLRect &rect, 
					 S32 font_size_index, F32 persist_time ) 
	: 
	LLFixedBuffer(max_lines),
	LLView(name, rect, FALSE),
	mLastBoxHeight(0),
	mLastBoxWidth(0)
{
	mLinePersistTime = persist_time;		// seconds
	mFadeTime = persist_time - FADE_DURATION;

	setFontSize( font_size_index );
	setMaxLines(gSavedSettings.getS32("ConsoleMaxLines"));
}

LLConsole::~LLConsole()
{
	mColors.clear();
}

EWidgetType LLConsole::getWidgetType() const
{
	return WIDGET_TYPE_CONSOLE;
}

LLString LLConsole::getWidgetTag() const
{
	return LL_CONSOLE_TAG;
}

void LLConsole::setLinePersistTime(F32 seconds)
{
	mLinePersistTime = seconds;
	mFadeTime = mLinePersistTime - FADE_DURATION;
}

void LLConsole::reshape(S32 width, S32 height, BOOL called_from_parent)
{
	S32 new_width = llmax(50, llmin(mRect.getWidth(), gViewerWindow->getWindowWidth()));
	S32 new_height = llmax(llfloor(mFont->getLineHeight()) + 15, llmin(mRect.getHeight(), gViewerWindow->getWindowHeight()));

	LLView::reshape(new_width, new_height, called_from_parent);
}

void LLConsole::setFontSize(S32 size_index)
{
	if (-1 == size_index)
	{
		mFont = LLFontGL::sMonospace;
	}
	else if (0 == size_index)
	{
		mFont = LLFontGL::sSansSerif;
	}
	else if (1 == size_index)
	{
		mFont = LLFontGL::sSansSerifBig;
	}
	else
	{
		mFont = LLFontGL::sSansSerifHuge;
	}
}

void LLConsole::draw()
{
	LLGLSUIDefault gls_ui;

	addQueuedLines();

	// skip lines added more than mLinePersistTime ago
	F32 cur_time = mTimer.getElapsedTimeF32();
	
	if( gStartupState != STATE_STARTED )
	{
		S32 count = mLines.size();
		S32 i = 0;
		while( count-- )
		{
			mAddTimes[i] = cur_time;
			i = (i+1) % mMaxLines;
		}
	}

	F32 skip_time = cur_time - mLinePersistTime;
	F32 fade_time = cur_time - mFadeTime;

	// draw remaining lines
	F32 x_pos = 0.f;
	F32 y_pos = 0.f;

	S32 line_count = mLines.size();

	// remove stale lines
	for (S32 line_num = 0; line_num < line_count; line_num++)
	{
		if((mLinePersistTime > 0.f) && (mAddTimes[0] - skip_time)/(mLinePersistTime - mFadeTime) <= 0.f)
		{
			mLines.pop_front();
			mAddTimes.pop_front();
			mLineLengths.pop_front();
			mColors.pop_front();
		}
	}

	line_count = mLines.size();

	S32 i;
	if (line_count == 0)
	{
		mLastBoxHeight = 0;
		mLastBoxWidth = 0;
		return;
	}
	else
	{
		LLUUID image_id;
		image_id.set(gViewerArt.getString("rounded_square.tga"));
		LLViewerImage* imagep = gImageList.getImage(image_id, MIPMAP_FALSE, TRUE);

		F32 console_opacity = llclamp(gSavedSettings.getF32("ConsoleBackgroundOpacity"), 0.f, 1.f);
		LLColor4 color(0.f, 0.f, 0.f, console_opacity);

		S32 max_width = 0;
		for (i = 0; i < line_count; i++)
		{
			max_width = llmax(max_width, mFont->getWidth(mLines[i].c_str()) + 30);
		}
		max_width = llmin(max_width, gViewerWindow->getWindowWidth());

		F32 u = 1.f;//LLCriticalDamp::getInterpolant(0.1f);
		S32 target_height = llfloor(line_count * mFont->getLineHeight() + 15);
		S32 target_width = max_width;
		mLastBoxHeight = llmax(target_height, (S32)lerp((F32)mLastBoxHeight, (F32)target_height, u));
		mLastBoxWidth = llmax(MIN_CONSOLE_WIDTH, llmax(target_width, (S32)lerp((F32)mLastBoxWidth, (F32)target_width, u)));
		gl_draw_scaled_image_with_border(-15, -10, 16, 16, mLastBoxWidth + 15, mLastBoxHeight,
								imagep, color, TRUE );
	}

	y_pos += (line_count-1) * mFont->getLineHeight();


	for (i = 0; i < line_count; i++)
	{
		F32 alpha;

		if ((mLinePersistTime > 0.f) && (mAddTimes[i] < fade_time))
		{
			alpha = (mAddTimes[i] - skip_time)/(mLinePersistTime - mFadeTime);
		}
		else
		{
			alpha = 1.0f;
		}

		if( alpha > 0.f )
		{
			// text line itself
			mFont->render(mLines[i], 0, x_pos, y_pos,
				LLColor4(
					mColors[i].mV[VRED], 
					mColors[i].mV[VGREEN], 
					mColors[i].mV[VBLUE], 
					mColors[i].mV[VALPHA]*alpha),
				LLFontGL::LEFT, 
				LLFontGL::BASELINE,
				LLFontGL::DROP_SHADOW,
				S32_MAX,
				mLastBoxWidth
				);
		}

		y_pos -= mFont->getLineHeight();
	}
}

void LLConsole::addLine(const LLString& utf8line)
{
	LLWString wline = utf8str_to_wstring(utf8line);
	addLine(wline, 0.f, LLColor4(1.f, 1.f, 1.f, 1.f));
}

void LLConsole::addLine(const LLWString& wline)
{
	addLine(wline, 0.f, LLColor4(1.f, 1.f, 1.f, 1.f));
}

void LLConsole::addLine(const LLString& utf8line, F32 size, const LLColor4 &color)
{
	LLWString wline = utf8str_to_wstring(utf8line);
	addLine(wline, size, color);
}

void LLConsole::addLine(const LLWString& wline, F32 size, const LLColor4 &color)
{
	while (mLineQueue.size() >= mMaxLines)
	{
		mLineQueue.pop_front();
	}
	mLineQueue.push_back(LineInfo(wline, size, color, mTimer.getElapsedTimeF32()));
}

void LLConsole::addQueuedLines()
{
	for (line_queue_t::iterator iter = mLineQueue.begin();
		 iter != mLineQueue.end(); ++iter)
	{
		LineInfo& line_info = *iter;
		LLWString wline = line_info.wline;
		//F32 size = line_info.size;
		LLColor4 color = line_info.color;
		if (!wline.empty() && mFont != NULL)
		{
			// Wrap lines that are longer than the view is wide.
			S32 offset = 0;
			while( offset < (S32)wline.length() )
			{
				S32 skip_chars; // skip '\n'
				// Figure out if a word-wrapped line fits here.
				LLWString::size_type line_end = wline.find_first_of(llwchar('\n'), offset);
				if (line_end != LLWString::npos)
				{
					skip_chars = 1; // skip '\n'
				}
				else
				{
					line_end = wline.size();
					skip_chars = 0;
				}
				U32 drawable = mFont->maxDrawableChars(wline.c_str()+offset, (F32)mRect.getWidth(), line_end-offset, TRUE);
				if (drawable != 0)
				{
					LLFixedBuffer::addLine(wline.substr(offset, drawable));
					mAddTimes[mAddTimes.size()-1] = line_info.add_time;
				}
				else
				{
					// force a blank line
					LLFixedBuffer::addLine(" ");
				}
				mColors.push_back(color);
				offset += (drawable + skip_chars);
			}
		}
	}
	mLineQueue.clear();
}

void LLConsole::removeExtraLines()
{
	while((S32)mColors.size() > llmax(0, (S32)(mMaxLines - 1)))
	{
		mColors.pop_front();
	}
	LLFixedBuffer::removeExtraLines();
}
