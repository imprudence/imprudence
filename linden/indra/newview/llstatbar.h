/** 
 * @file llstatbar.h
 * @brief A little map of the world with network information
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

#ifndef LL_LLSTATBAR_H
#define LL_LLSTATBAR_H

#include "llview.h"
#include "llframetimer.h"
#include "llviewercontrol.h"

class LLStat;

class LLStatBar : public LLView
{
	enum STAT_MODE_FLAG
	{
		STAT_BAR_FLAG = 1,
		STAT_HISTORY_FLAG = 2
	};
	
public:
	LLStatBar(const std::string& name, const LLRect& rect, const std::string& setting = std::string(),
			  BOOL default_bar = FALSE, BOOL default_history = FALSE);

	virtual void draw();
	virtual BOOL handleMouseDown(S32 x, S32 y, MASK mask);

	const std::string& getLabel() const;
	void setLabel(const std::string& label);
	void setUnitLabel(const std::string& unit_label);
	/*virtual*/ LLRect getRequiredRect();	// Return the height of this object, given the set options.

	F32 mMinBar;
	F32 mMaxBar;
	F32 mTickSpacing;
	F32 mLabelSpacing;
	U32 mPrecision;
	BOOL mPerSec;				// Use the per sec stats.
	BOOL mDisplayBar;			// Display the bar graph.
	BOOL mDisplayHistory;
	BOOL mDisplayMean;			// If true, display mean, if false, display current value

	F32 mUpdatesPerSec;
	LLStat *mStatp;
private:
	LLFrameTimer mUpdateTimer;
	std::string mLabel;
	std::string mUnitLabel;
	F32 mValue;
	std::string mSetting;
};

#endif
