/** 
 * @file llpaneldirgroups.cpp
 * @brief Groups panel in the Find directory.
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

#include "llpaneldirgroups.h"
#include "llnotifications.h"

#include "llwebbrowserctrl.h"

LLPanelDirGroups::LLPanelDirGroups(const std::string& name, LLFloaterDirectory* floater)
	:	LLPanelDirFind(name, floater, "groups_browser")
{
}


void LLPanelDirGroups::search(const std::string& search_text)
{
	if (!search_text.empty())
	{
		BOOL inc_pg = childGetValue("incpg").asBoolean();
		BOOL inc_mature = childGetValue("incmature").asBoolean();
		BOOL inc_adult = childGetValue("incadult").asBoolean();
		if (!(inc_pg || inc_mature || inc_adult))
		{
			LLNotifications::instance().add("NoContentToSearch");
			return;
		}
		
		std::string selected_collection = "Groups";
		std::string url = buildSearchURL(search_text, selected_collection, inc_pg, inc_mature, inc_adult);
		if (mWebBrowser)
		{
			mWebBrowser->navigateTo(url);
		}
	}
	else
	{
		// empty search text
		navigateToDefaultPage();
	}

	childSetText("search_editor", search_text);
}

void LLPanelDirGroups::draw()
{
	updateMaturityCheckbox();
	LLPanelDirBrowser::draw();
}
