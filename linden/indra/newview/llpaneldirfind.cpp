/**
 * @file llpaneldirfind.cpp
 * @brief The "Find All" panel in the Find directory.
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

#include "llpaneldirfind.h"

// linden library includes
#include "llclassifiedflags.h"
#include "llfontgl.h"
#include "llparcel.h"
#include "llqueryflags.h"
#include "message.h"

// viewer project includes
#include "llagent.h"
#include "llbutton.h"
#include "llcheckboxctrl.h"
#include "llcombobox.h"
#include "llviewercontrol.h"
#include "lllineeditor.h"
#include "llmenucommands.h"
#include "llmenugl.h"
#include "llscrolllistctrl.h"
#include "lltextbox.h"
#include "lluiconstants.h"
#include "llviewerimagelist.h"
#include "llviewermessage.h"
#include "llfloateravatarinfo.h"
#include "lldir.h"
#include "llvieweruictrlfactory.h"
#include "llfloaterdirectory.h"
#include "llpaneldirbrowser.h"

LLPanelDirFind::LLPanelDirFind(const std::string& name, LLFloaterDirectory* floater)
	:	LLPanelDirBrowser(name, floater)
{
	mMinSearchChars = 3;
}

BOOL LLPanelDirFind::postBuild()
{
	LLPanelDirBrowser::postBuild();

	childSetKeystrokeCallback("name", &LLPanelDirBrowser::onKeystrokeName, this);

	childSetAction("Search", onClickSearch, this);
	childDisable("Search");
	setDefaultBtn( "Search" );

	return TRUE;
}

// virtual
void LLPanelDirFind::draw()
{
	// You only have a choice if you are mature
	childSetVisible("incmature", gAgent.mAccess >= SIM_ACCESS_MATURE);
	childSetValue("incmature", gSavedSettings.getBOOL("ShowMatureFindAll"));

	LLPanelDirBrowser::draw();
}

LLPanelDirFind::~LLPanelDirFind()
{
	// Children all cleaned up by default view destructor.
}

// static
void LLPanelDirFind::onCommitScope(LLUICtrl* ctrl, void* data)
{
	LLPanelDirFind* self = (LLPanelDirFind*)data;
	self->setFocus(TRUE);
}

// static
void LLPanelDirFind::onClickSearch(void *userdata)
{
	LLPanelDirFind *self = (LLPanelDirFind *)userdata;

	if (self->childGetValue("name").asString().length() < self->mMinSearchChars)
	{
		return;
	};

	self->setupNewSearch();

	// Figure out scope
	U32 scope = 0x0;
	scope |= DFQ_PEOPLE;	// people (not just online = 0x01 | 0x02)
	// places handled below
	scope |= DFQ_EVENTS;	// events
	scope |= DFQ_GROUPS;	// groups

	if (!gSavedSettings.getBOOL("ShowMatureFindAll") || gAgent.mAccess <= SIM_ACCESS_PG) scope |= DFQ_PG_SIMS_ONLY;

	// send the message
	LLMessageSystem *msg = gMessageSystem;
	S32 start_row = 0;
	sendDirFindQuery(msg, self->mSearchID, self->childGetValue("name").asString(), scope, start_row);

	// Also look up classified ads. JC 12/2005
	BOOL filter_mature = !gSavedSettings.getBOOL("ShowMatureFindAll");
	if (gAgent.mAccess <= SIM_ACCESS_PG) filter_mature = TRUE;
	BOOL filter_auto_renew = FALSE;
	U32 classified_flags = pack_classified_flags(filter_mature, filter_auto_renew);
	msg->newMessage("DirClassifiedQuery");
	msg->nextBlock("AgentData");
	msg->addUUID("AgentID", gAgent.getID());
	msg->addUUID("SessionID", gAgent.getSessionID());
	msg->nextBlock("QueryData");
	msg->addUUID("QueryID", self->mSearchID);
	msg->addString("QueryText", self->childGetValue("name").asString());
	msg->addU32("QueryFlags", classified_flags);
	msg->addU32("Category", 0);	// all categories
	msg->addS32("QueryStart", 0);
	gAgent.sendReliableMessage();

	U32 query_flags = DFQ_DWELL_SORT;
	if (!gSavedSettings.getBOOL("ShowMatureFindAll") || gAgent.mAccess <= SIM_ACCESS_PG) query_flags |= DFQ_PG_SIMS_ONLY;

	// Need to use separate find places query because places are
	// sent using the more compact DirPlacesReply message.
	msg->newMessage("DirPlacesQuery");
	msg->nextBlock("AgentData");
	msg->addUUID("AgentID", gAgent.getID() );
	msg->addUUID("SessionID", gAgent.getSessionID());
	msg->nextBlock("QueryData");
	msg->addUUID("QueryID", self->mSearchID );
	msg->addString("QueryText", self->childGetValue("name").asString());
	msg->addU32("QueryFlags", query_flags );
	msg->addS32("QueryStart", 0 ); // Always get the first 100 when using find ALL
	msg->addS8("Category", LLParcel::C_ANY);
	msg->addString("SimName", NULL);
	gAgent.sendReliableMessage();
}
