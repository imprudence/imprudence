/** 
 * @file llstatview.cpp
 * @brief Container for all statistics info.
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

#include "llstatview.h"

#include "llerror.h"
#include "llstatbar.h"
#include "llfontgl.h"
#include "llgl.h"
#include "llui.h"

#include "llstatbar.h"
#include "llviewercontrol.h"

LLStatView::LLStatView(const std::string& name, const std::string& label, const std::string& setting, const LLRect& rect)
	:	LLContainerView(name, rect),
		mNumStatBars(0),
		mSetting(setting)
{
	setFollows(FOLLOWS_TOP | FOLLOWS_LEFT);
	setLabel(label);
	BOOL open = FALSE;
	if (mSetting.length() > 0)
	{
		open = gSavedSettings.getBOOL(mSetting);
	}
	setDisplayChildren(open);		/* Flawfinder: ignore */
}

LLStatView::~LLStatView()
{
	// Children all cleaned up by default view destructor.
	if (mSetting.length() > 0)
	{
		BOOL open = getDisplayChildren();
		gSavedSettings.setBOOL(mSetting, open);		/* Flawfinder: ignore */
	}
}

LLStatBar *LLStatView::addStat(const std::string& name, LLStat *statp,
							   const std::string& setting, BOOL default_bar, BOOL default_history)
{
	LLStatBar *stat_barp;
	LLRect r;

//	if (getStatBar(name))
//	{
//		llinfos << "LLStatView::addStat - Stat already exists!" << llendl;
//		return NULL;
//	}

	mNumStatBars++;

	stat_barp = new LLStatBar(name, r, setting, default_bar, default_history);
	stat_barp->mStatp = statp;

	stat_barp->setVisible(mDisplayChildren);
	addChildAtEnd(stat_barp);
	mStatBars.push_back(stat_barp);

	// Rearrange all child bars.
	reshape(getRect().getWidth(), getRect().getHeight());
	return stat_barp;
}

LLStatView *LLStatView::addStatView(const std::string& name, const std::string& label, const std::string& setting, const LLRect& rect)
{
	LLStatView *statview = new LLStatView(name, label, setting, rect);
	statview->setVisible(mDisplayChildren);
	addChildAtEnd(statview);
	return statview;
}


LLStatBar *LLStatView::getStatBar(const std::string& name)
{
	sb_vector_t::iterator iter;
	for(iter = mStatBars.begin(); iter != mStatBars.end(); ++iter)
	{
		LLStatBar *stat_barp = *iter;
		if (stat_barp->getLabel() == name)
		{
			return stat_barp;
		}
	}

	// Not found!
	return NULL;
}
