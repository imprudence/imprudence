/**
 * @file llfloaterstats.cpp
 * @brief Container for statistics view
 *
 * $LicenseInfo:firstyear=2001&license=viewergpl$
 * 
 * Copyright (c) 2001-2008, Linden Research, Inc.
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
 * online at http://secondlifegrid.net/programs/open_source/licensing/flossexception
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

#include "llfloaterstats.h"
#include "llcontainerview.h"
#include "llfloater.h"
#include "llstatview.h"
#include "llscrollcontainer.h"

const S32 LL_SCROLL_BORDER = 1;

LLFloaterStats::LLFloaterStats(const LLRect& rect)
	:   LLFloater(std::string("floater_stats"), rect, std::string("Statistics"), TRUE, rect.getWidth())

{
	LLRect stats_rect(0, rect.getHeight() - LLFLOATER_HEADER_SIZE,
					rect.getWidth() - LLFLOATER_CLOSE_BOX_SIZE, 0);
	mStatsContainer = new LLContainerView("statistics_view", stats_rect);
	mStatsContainer->showLabel(FALSE);

	LLRect scroll_rect(LL_SCROLL_BORDER, rect.getHeight() - LLFLOATER_HEADER_SIZE - LL_SCROLL_BORDER,
					   rect.getWidth() - LL_SCROLL_BORDER, LL_SCROLL_BORDER);
		mScrollContainer = new LLScrollableContainerView(std::string("statistics_scroll"), scroll_rect, mStatsContainer);
	mScrollContainer->setFollowsAll();
	mScrollContainer->setReserveScrollCorner(TRUE);

	mStatsContainer->setScrollContainer(mScrollContainer);
	
	addChild(mScrollContainer);
}


LLFloaterStats::~LLFloaterStats()
{
}

void LLFloaterStats::reshape(S32 width, S32 height, BOOL called_from_parent)
{
	LLRect rect = mStatsContainer->getRect();

	mStatsContainer->reshape(rect.getWidth() - 2, rect.getHeight(), TRUE);

	LLFloater::reshape(width, height, called_from_parent);
}


void LLFloaterStats::addStatView(LLStatView* stat)
{
	mStatsContainer->addChildAtEnd(stat);
}

void LLFloaterStats::onClose(bool app_quitting)
{
	setVisible(FALSE);
}
