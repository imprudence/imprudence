/** 
 * @file llpaneldirplaces.cpp
 * @brief "Places" panel in the Find directory (not popular places, just places)
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

#include "llpaneldirplaces.h"

// linden library includes
#include "llfontgl.h"
#include "message.h"
#include "lldir.h"
#include "llparcel.h"
#include "llregionflags.h"
#include "llqueryflags.h"

// viewer project includes
#include "llagent.h"
#include "llbutton.h"
#include "llcheckboxctrl.h"
#include "llcombobox.h"
#include "llfloaterdirectory.h"
#include "lllineeditor.h"
#include "llpaneldirbrowser.h"
#include "lltextbox.h"
#include "lluiconstants.h"
#include "llviewercontrol.h"
#include "llviewermessage.h"
#include "llworldmap.h"

LLPanelDirPlaces::LLPanelDirPlaces(const std::string& name, LLFloaterDirectory* floater)
	:	LLPanelDirBrowser(name, floater)
{
	mMinSearchChars = 3;
}

BOOL LLPanelDirPlaces::postBuild()
{
	LLPanelDirBrowser::postBuild();

	childSetKeystrokeCallback("name", &LLPanelDirBrowser::onKeystrokeName, this);

	childSetAction("Search", &LLPanelDirBrowser::onClickSearchCore, this);
	childDisable("Search");

	mCurrentSortColumn = "dwell";
	mCurrentSortAscending = FALSE;

	// Don't prepopulate the places list, as it hurts the database as of 2006-12-04. JC
	// initialQuery();

	return TRUE;
}

LLPanelDirPlaces::~LLPanelDirPlaces()
{
	// Children all cleaned up by default view destructor.
}


// virtual
void LLPanelDirPlaces::draw()
{
	// You only have a choice if you are mature
	childSetVisible("incmature", gAgent.mAccess >= SIM_ACCESS_MATURE);
	childSetValue("incmature", gSavedSettings.getBOOL("ShowMatureSims"));
	
	LLPanelDirBrowser::draw();
}

// virtual
void LLPanelDirPlaces::performQuery()
{
	LLString name = childGetValue("name").asString();
	if (name.length() < mMinSearchChars)
	{
		return;
	}

	LLString catstring = childGetValue("Category").asString();
	
	// Because LLParcel::C_ANY is -1, must do special check
	S32 category = 0;
	if (catstring == "any")
	{
		category = LLParcel::C_ANY;
	}
	else
	{
		category = LLParcel::getCategoryFromString(catstring.c_str());
	}

	BOOL pg_only = !gSavedSettings.getBOOL("ShowMatureSims") 
				   || gAgent.mAccess <= SIM_ACCESS_PG;

	queryCore(name, category, pg_only);
}

void LLPanelDirPlaces::initialQuery()
{
	// All Linden locations in PG/Mature sims, any name.
	const BOOL pg_only = FALSE;
	queryCore("", LLParcel::C_LINDEN, pg_only);
}

void LLPanelDirPlaces::queryCore(const LLString& name, 
								 S32 category, 
								 BOOL pg_only)
{
	setupNewSearch();

	// send the message
	U32 flags = 0x0;

	if (pg_only)
	{
		flags |= DFQ_PG_PARCELS_ONLY;
	}

// JC: Sorting by dwell severely impacts the performance of the query.
// Instead of sorting on the dataserver, we sort locally once the results
// are received.
// IW: Re-enabled dwell sort based on new 3-character minimum description
// Hopefully we'll move to next-gen Find before this becomes a big problem

	flags |= DFQ_DWELL_SORT;

	LLMessageSystem* msg = gMessageSystem;

	msg->newMessage("DirPlacesQuery");
	msg->nextBlock("AgentData");
	msg->addUUID("AgentID", gAgent.getID());
	msg->addUUID("SessionID", gAgent.getSessionID());
	msg->nextBlock("QueryData");
	msg->addUUID("QueryID", getSearchID());
	msg->addString("QueryText", name);
	msg->addU32("QueryFlags", flags);
	msg->addS8("Category", (S8)category);
	// No longer support queries by region name, too many regions
	// for combobox, no easy way to do autocomplete. JC
	msg->addString("SimName", "");
	msg->addS32Fast(_PREHASH_QueryStart,mSearchStart);
	gAgent.sendReliableMessage();
}

