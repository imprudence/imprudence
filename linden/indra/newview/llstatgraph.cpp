/** 
 * @file llstatgraph.cpp
 * @brief Simpler compact stat graph with tooltip
 *
 * Copyright (c) 2002-2007, Linden Research, Inc.
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

#include "llstatgraph.h"

#include "llmath.h"
#include "llui.h"
#include "llstat.h"
#include "llgl.h"
#include "llglheaders.h"
#include "llviewercontrol.h"

///////////////////////////////////////////////////////////////////////////////////

LLStatGraph::LLStatGraph(const std::string& name, const LLRect& rect)
		:	LLView(name, rect, TRUE)
{
	mStatp = NULL;
	setToolTip(name);
	mNumThresholds = 3;
	mThresholdColors[0] = LLColor4(0.f, 1.f, 0.f, 1.f);
	mThresholdColors[1] = LLColor4(1.f, 1.f, 0.f, 1.f);
	mThresholdColors[2] = LLColor4(1.f, 0.f, 0.f, 1.f);
	mThresholdColors[3] = LLColor4(1.f, 0.f, 0.f, 1.f);
	mThresholds[0] = 50.f;
	mThresholds[1] = 75.f;
	mThresholds[2] = 100.f;
	mMin = 0.f;
	mMax = 125.f;
	mPerSec = TRUE;
	mValue = 0.f;
	mPrecision = 0;
}

EWidgetType LLStatGraph::getWidgetType() const
{
	return WIDGET_TYPE_STAT_GRAPH;
}

LLString LLStatGraph::getWidgetTag() const
{
	return LL_STAT_GRAPH_TAG;
}

void LLStatGraph::draw()
{
	if (getVisible())
	{
		F32 range, frac;
		range = mMax - mMin;
		if (mStatp)
		{
			if (mPerSec)
			{
				mValue = mStatp->getMeanPerSec();
			}
			else
			{
				mValue = mStatp->getMean();
			}
		}
		frac = (mValue - mMin) / range;
		frac = llmax(0.f, frac);
		frac = llmin(1.f, frac);

		if (mUpdateTimer.getElapsedTimeF32() > 0.5f)
		{
			char format_str[256];		/* Flawfinder: ignore */
			char tmp_str[256];		/* Flawfinder: ignore */
			snprintf(format_str, sizeof(format_str), "%%s%%.%df%%s", mPrecision);		/* Flawfinder: ignore */
			snprintf(tmp_str, sizeof(tmp_str), format_str, mLabel.c_str(), mValue, mUnits.c_str());		/* Flawfinder: ignore */
			setToolTip(tmp_str);

			mUpdateTimer.reset();
		}

		LLColor4 color;

		S32 i;
		for (i = 0; i < mNumThresholds - 1; i++)
		{
			if (mThresholds[i] > mValue)
			{
				break;
			}
		}

		//gl_drop_shadow(0,  mRect.getHeight(), mRect.getWidth(), 0,
		//				gColors.getColor("ColorDropShadow"), 
		//				(S32) gSavedSettings.getF32("DropShadowFloater") );

		
		color = gColors.getColor( "MenuDefaultBgColor" );
		glColor4fv(color.mV);
		gl_rect_2d(0, mRect.getHeight(), mRect.getWidth(), 0, TRUE);

		glColor4fv(LLColor4::black.mV);
		gl_rect_2d(0, mRect.getHeight(), mRect.getWidth(), 0, FALSE);
		
		color = mThresholdColors[i];
		glColor4fv(color.mV);
		gl_rect_2d(1, llround(frac*mRect.getHeight()), mRect.getWidth() - 1, 0, TRUE);
	}
}

void LLStatGraph::setValue(const F32 value)
{
	mValue = value;
}

void LLStatGraph::setMin(const F32 min)
{
	mMin = min;
}

void LLStatGraph::setMax(const F32 max)
{
	mMax = max;
}

void LLStatGraph::setStat(LLStat *statp)
{
	mStatp = statp;
}

void LLStatGraph::setLabel(const char *label)
{
	mLabel = label;
}

void LLStatGraph::setUnits(const char *units)
{
	mUnits = units;
}

void LLStatGraph::setPrecision(const S32 precision)
{
	mPrecision = precision;
}

void LLStatGraph::setThreshold(const S32 i, F32 value)
{
	mThresholds[i] = value;
}
