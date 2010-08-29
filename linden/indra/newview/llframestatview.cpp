/** 
 * @file llframestatview.cpp
 * @brief LLFrameStatView class implementation
 *
 * $LicenseInfo:firstyear=2001&license=viewergpl$
 * 
 * Copyright (c) 2001-2009, Linden Research, Inc.
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

#include "llframestatview.h"

#include "llframestats.h"
#include "llrect.h"
#include "llerror.h"
#include "llgl.h"
#include "llmath.h"
#include "llfontgl.h"

#include "llui.h"
#include "llviewercontrol.h"
#include "llstat.h"


LLFrameStatView::LLFrameStatView(const std::string& name, const LLRect& rect)
:	LLView(name, rect, TRUE)
{
	setVisible(FALSE);

	mNumStats     = 0;
	mTotalTime    = 0.05f;
	mTickSpacing  = 0.01f;
	mLabelSpacing = 0.05f;
	mShowPercent  = TRUE;
	mCenterOn     = 0;
}

LLFrameStatView::~LLFrameStatView()
{
	// Children all cleaned up by default view destructor.
}

void LLFrameStatView::setup(const LLFrameStats &frame_stats)
{
	S32 i;
	for (i = 0; i < LLFrameStats::NUM_STATS; i++)
	{
		addStat(&gFrameStats.getStat(i),
				gFrameStats.getStatLabel(i),
				gFrameStats.getStatColor(i));
	}
}

BOOL LLFrameStatView::handleMouseDown(S32 x, S32 y, MASK mask)
{
	mShowPercent = !mShowPercent;
	return TRUE;
}

BOOL LLFrameStatView::handleMouseUp(S32 x, S32 y, MASK mask)
{
	return TRUE;
}


BOOL LLFrameStatView::handleHover(S32 x, S32 y, MASK mask)
{
	return FALSE;
}


void LLFrameStatView::draw()
{
	S32 i;
	S32 j;
	LLStat *statp;

	gGL.getTexUnit(0)->unbind(LLTexUnit::TT_TEXTURE);

	statp = mStats[0];
	if (!statp)
	{
		return;
	}
	S32 num_bins = llmin(statp->getNumValues(), (U32)32);

	F32 total_width = getRect().getWidth() - 20.f;
	F32 total_height = getRect().getHeight() - 30.f;
	S32 left, top, right, bottom;
	F32 key_height = 16;
	S32 bar_spacing = 2;
	F32 time_scale = total_width / mTotalTime;

	// Draw the window background
	gl_rect_2d(0, getRect().getHeight(), getRect().getWidth(), 0, LLColor4(0.f, 0.f, 0.f, 0.25f));

	// Render the key
	left = 10;
	top = getRect().getHeight() - 10;

	BOOL is_active[MAX_STATS];
	S32 bin = mStats[0]->getCurBin() - 1;
	if (bin < 0)
	{
		bin += mStats[0]->getNumBins();
	}

	F64 min_last_time = mStats[0]->getBinTime(bin);

	for (i = 0; i < mNumStats; i++)
	{
		if (mStats[i]->getLastTime() >= min_last_time)
		{
			is_active[i] = TRUE;
		}
		else
		{
			is_active[i] = FALSE;
		}
	}

	S32 cur_center = mCenterOn;
	if (!is_active[mCenterOn])
	{
		for (i = 0; i < mNumStats; i++)
		{
			if (is_active[i])
			{
				cur_center = i;
				break;
			}
		}
	}


	for (i = 0; i < mNumStats; i++)
	{
		// Draw the color key.
		gGL.getTexUnit(0)->unbind(LLTexUnit::TT_TEXTURE);
		gl_rect_2d(left, top - 2, left + 10, top - 12, mColors[i]);

		left = left + 15;
		// Draw the label.
		LLFontGL::getFontMonospace()->renderUTF8(mLabels[i], 0, left, top,
			LLColor4(1.f, 1.f, 1.f, 1.f),
			LLFontGL::LEFT, LLFontGL::TOP);
	
		left = left + LLFontGL::getFontMonospace()->getWidth(mLabels[i]) + 10;
		if ((i + 1) < mNumStats)
		{
			if ((left + LLFontGL::getFontMonospace()->getWidth(mLabels[i+1])) > (getRect().getWidth() - 10))
			{
				left = 10;
				top -= llfloor(LLFontGL::getFontMonospace()->getLineHeight());
				key_height += LLFontGL::getFontMonospace()->getLineHeight();
			}
		}
	}

	top -= llfloor(LLFontGL::getFontMonospace()->getLineHeight());
	bottom = top - llfloor(LLFontGL::getFontMonospace()->getLineHeight());
	F32 total_time = 0.f;
	F32 bin_means[32];

	key_height += 20;

	// Draw the "Average" bar
	
	// Draw ticks for the "Average bar"
	F32 tick_value;
	std::string tick_label;
	for (tick_value = 0; tick_value <= 100; tick_value += 10)
	{
		left = 10 + llfloor(tick_value*(total_width/100.f));
		right = left + 1;
		gl_rect_2d(left, top, right, bottom, LLColor4(1.f, 1.f, 1.f, 0.2f));
	}

	// Draw the tick labels (and big ticks).
	bottom = bottom - 4;
	for (tick_value = 0; tick_value <= 100; tick_value += 20)
	{
		gGL.getTexUnit(0)->unbind(LLTexUnit::TT_TEXTURE);
		left = 10 + llfloor(tick_value*(total_width/100.f));
		right = left + 1;
		gl_rect_2d(left, top, right, bottom, LLColor4(1.f, 1.f, 1.f, 0.2f));

		tick_label = llformat("%.2f", tick_value);
		// draw labels for the tick marks
		LLFontGL::getFontMonospace()->renderUTF8(tick_label, 0, left, bottom,
			LLColor4(1.f, 1.f, 1.f, 0.5f),
			LLFontGL::LEFT, LLFontGL::TOP);
	}

	for (i = 0; i < mNumStats; i++)
	{
		bin_means[i] = mStats[i]->getMeanDuration();
		total_time += bin_means[i];
	}

	F32 time_frac = total_width / total_time;
	F32 cur_left = .0f;
	for (i = 0; i < mNumStats; i++)
	{
		F32 cur_right = cur_left + bin_means[i] * time_frac;
		left = llfloor(cur_left) + 10;
		right = llfloor(cur_right) + 10;
		gl_rect_2d(left, top, right, bottom, mColors[i]);
		cur_left = cur_right;
	}

	S32 bar_height = llmax(1, (S32)((total_height - bar_spacing*(num_bins-1) - key_height) / num_bins));
	S32 graph_height = bar_height * num_bins + bar_spacing * num_bins - 1;

	// Draw the tick marks.
	top = getRect().getHeight() - llfloor(key_height) - 20;

	bottom = top - graph_height - 2;
	// Render the ticks and labels
	if (mShowPercent)
	{
		for (tick_value = 0; tick_value <= 100; tick_value += 10)
		{
			left = 10 + llfloor(tick_value*(total_width/100.f));
			right = left + 1;
			gl_rect_2d(left, top, right, bottom, LLColor4(1.f, 1.f, 1.f, 0.1f));
		}

		// Draw the tick labels (and big ticks).
		bottom = top - graph_height - 4;
		for (tick_value = 0; tick_value <= 100; tick_value += 20)
		{
			gGL.getTexUnit(0)->unbind(LLTexUnit::TT_TEXTURE);
			left = 10 + llfloor(tick_value*(total_width/100.f));
			right = left + 1;
			gl_rect_2d(left, top, right, bottom, LLColor4(1.f, 1.f, 1.f, 0.25f));

			tick_label = llformat("%.2f", tick_value);
			// draw labels for the tick marks
			LLFontGL::getFontMonospace()->renderUTF8(tick_label, 0, left, bottom,
				LLColor4(1.f, 1.f, 1.f, 0.5f),
				LLFontGL::LEFT, LLFontGL::TOP);
		}
	}
	else
	{
		for (tick_value = 0; tick_value <= mTotalTime; tick_value += mTickSpacing)
		{
			left = 10 + llfloor(tick_value*time_scale);
			right = left + 1;
			gl_rect_2d(left, top, right, bottom, LLColor4(1.f, 1.f, 1.f, 0.1f));
		}

		// Draw the tick labels (and big ticks).
		bottom = top - graph_height - 4;
		for (tick_value = 0; tick_value <= mTotalTime; tick_value += mLabelSpacing)
		{
			gGL.getTexUnit(0)->unbind(LLTexUnit::TT_TEXTURE);
			left = 10 + llfloor(tick_value*time_scale);
			right = left + 1;
			gl_rect_2d(left, top, right, bottom, LLColor4(1.f, 1.f, 1.f, 0.25f));

			tick_label = llformat("%.2f", tick_value);
			// draw labels for the tick marks
			LLFontGL::getFontMonospace()->renderUTF8(tick_label, 0, left, bottom,
				LLColor4(1.f, 1.f, 1.f, 0.5f),
				LLFontGL::LEFT, LLFontGL::TOP);
		}
	}

	// Draw the graph background
	bottom = top - graph_height;
	left = 10;
	right = left + llfloor(total_width);

	gl_rect_2d(left, top, right, bottom, LLColor4(1.f, 1.f, 1.f, 0.25f));

	F64 min_time = 0.f;
	F64 max_time = 0.f;

	// Determine which stats are active.

	top = getRect().getHeight() - llfloor(key_height) - 20;
	bottom = top - bar_height;
	for (i = 0; i < 32; i++)
	{
		// Find the min time for this set of bins.
		min_time = mStats[0]->getPrevBeginTime(i);
		max_time = mStats[0]->getPrevBeginTime(i);
		for (j = 0; j < mNumStats; j++)
		{
			if (is_active[j])
			{
				min_time = llmin(min_time, mStats[j]->getPrevBeginTime(i));
				max_time = llmax(max_time, mStats[j]->getPrevTime(i));
			}
		}

		if (mShowPercent)
		{
			for (j = 0; j < mNumStats; j++)
			{
				if (is_active[j])
				{
					statp = mStats[j];
					F64 begin = statp->getPrevBeginTime(i) - min_time;
					F64 end = statp->getPrevTime(i) - min_time;

					F32 begin_frac = (F32)(begin/(max_time - min_time));
					F32 end_frac   = (F32)(end/(max_time - min_time));

					left = llfloor(begin_frac * total_width) + 10;
					right = llfloor(end_frac * total_width) + 10;
					gl_rect_2d(left, top, right, bottom, mColors[j]);
				}
			}
		}
		else
		{
			F64 center_time = mStats[cur_center]->getPrevBeginTime(i);
			center_time = min_time;

			// Now, draw the bars for each stat
			for (j = 0; j < mNumStats; j++)
			{
				if (is_active[j])
				{
					statp = mStats[j];
					F64 begin = statp->getPrevBeginTime(i);
					F64 end   = statp->getPrevTime(i);

					F32 begin_frac = (F32)(begin - center_time);
					F32 end_frac   = (F32)(end - center_time);

					left  = llfloor(begin_frac * time_scale) + 10;
					right = llfloor(end_frac * time_scale) + 10;

					gl_rect_2d(left, top, right, bottom, mColors[j]);
				}
			}
		}
		top = top - bar_height - bar_spacing;
		bottom = top - bar_height;
	}
	LLView::draw();
}

void LLFrameStatView::addStat(LLStat *statp, const std::string& label, const LLColor4 &color)
{
	if( mNumStats >= MAX_STATS )
	{
		llwarns << "LLFrameStatView::addStat - too many stats!" << llendl;
		return;
	}

	mStats[mNumStats] = statp;
	mColors[mNumStats] = color;
	mLabels[mNumStats] = label;
	mNumStats++;
}
