/** 
 * @file llpaneldirpopular.cpp
 * @brief Popular places as measured by dwell.
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

#include "llpaneldirpopular.h"

// linden library includes
#include "llfontgl.h"
#include "message.h"
#include "llqueryflags.h"

// viewer project includes
#include "llagent.h"
#include "llcheckboxctrl.h"
#include "llviewercontrol.h"
#include "lluiconstants.h"
#include "lltextbox.h"

LLPanelDirPopular::LLPanelDirPopular(const std::string& name, LLFloaterDirectory* floater)
	:	LLPanelDirBrowser(name, floater),
		mRequested(false)
{
}

BOOL LLPanelDirPopular::postBuild()
{
	LLPanelDirBrowser::postBuild();

	childSetCommitCallback("incpictures", onCommitAny, this);
	childSetCommitCallback("incmature", onCommitAny, this);

	mCurrentSortColumn = "dwell";
	mCurrentSortAscending = FALSE;

	// Don't request popular until first drawn.  JC
	// requestPopular();

	return TRUE;
}

LLPanelDirPopular::~LLPanelDirPopular()
{
	// Children all cleaned up by default view destructor.
}


// virtual
void LLPanelDirPopular::draw()
{
	// You only have a choice if you are mature]
	childSetVisible("incmature", gAgent.mAccess >= SIM_ACCESS_MATURE);
	childSetValue("incmature", gSavedSettings.getBOOL("ShowMatureSims"));

	LLPanelDirBrowser::draw();
	
	if (!mRequested)
	{
		requestPopular();
		mRequested = true;
	}
}


void LLPanelDirPopular::requestPopular()
{
	LLMessageSystem* msg = gMessageSystem;
	BOOL pg_only = !childGetValue("incmature").asBoolean() || gAgent.mAccess <= SIM_ACCESS_PG;
	BOOL pictures_only = childGetValue("incpictures").asBoolean();

	U32 flags = 0x0;
	if (pg_only)
	{
		flags |= DFQ_PG_SIMS_ONLY;
	}
	if (pictures_only)
	{
		flags |= DFQ_PICTURES_ONLY;
	}

	setupNewSearch();

	msg->newMessage("DirPopularQuery");
	msg->nextBlock("AgentData");
	msg->addUUID("AgentID", gAgent.getID());
	msg->addUUID("SessionID", gAgent.getSessionID());
	msg->nextBlock("QueryData");
	msg->addUUID("QueryID", getSearchID());
	msg->addU32("QueryFlags", flags);
	gAgent.sendReliableMessage();
}


// static
void LLPanelDirPopular::onClickSearch(void* data)
{
	LLPanelDirPopular* self = (LLPanelDirPopular*)data;
	self->requestPopular();
}

// static
void LLPanelDirPopular::onCommitAny(LLUICtrl* ctrl, void* data)
{
	LLPanelDirPopular* self = (LLPanelDirPopular*)data;
	self->requestPopular();
}
