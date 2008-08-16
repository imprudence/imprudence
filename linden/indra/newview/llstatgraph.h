/** 
 * @file llstatgraph.h
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

#ifndef LL_LLSTATGRAPH_H
#define LL_LLSTATGRAPH_H

#include "llview.h"
#include "llframetimer.h"
#include "v4color.h"

class LLStat;

class LLStatGraph : public LLView
{
public:
	LLStatGraph(const std::string& name, const LLRect& rect);

	virtual EWidgetType getWidgetType() const;
	virtual LLString getWidgetTag() const;
	virtual void draw();

	void setLabel(const char *label);
	void setUnits(const char *units);
	void setPrecision(const S32 precision);
	void setStat(LLStat *statp);
	void setThreshold(const S32 i, F32 value);
	void setMin(const F32 min);
	void setMax(const F32 max);

	void setValue(const F32 value);
	LLStat *mStatp;
	BOOL mPerSec;
private:
	F32 mValue;

	F32 mMin;
	F32 mMax;
	LLFrameTimer mUpdateTimer;
	LLString mLabel;
	LLString mUnits;
	S32 mPrecision; // Num of digits of precision after dot

	S32 mNumThresholds;
	F32 mThresholds[4];
	LLColor4 mThresholdColors[4];
};

#endif  // LL_LLSTATGRAPH_H
