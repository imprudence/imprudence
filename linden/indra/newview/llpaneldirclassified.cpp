/** 
 * @file llpaneldirclassified.cpp
 * @brief Classified panel in the Find directory.
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

#include "llenum.h"

#include "llpaneldirclassified.h"

#include "llclassifiedflags.h"

// linden library includes
#include "llfontgl.h"
#include "message.h"
#include "llqueryflags.h"

// viewer project includes
#include "llagent.h"
#include "llbutton.h"
#include "llcontrol.h"
#include "llcombobox.h"
#include "llclassifiedinfo.h"
#include "lluiconstants.h"
#include "llpaneldirbrowser.h"
#include "lltextbox.h"

#include "llcheckboxctrl.h"
#include "llfloaterdirectory.h"
#include "lllineeditor.h"
#include "llpaneldirbrowser.h"
#include "lltextbox.h"
#include "llviewermessage.h"
#include "lluictrlfactory.h"
#include "llworldmap.h"

//
#include "llfloateravatarinfo.h"
#include "llviewermenu.h"

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

LLPanelDirClassified::LLPanelDirClassified(const std::string& name, LLFloaterDirectory* floater)
:	LLPanelDirBrowser(name, floater)
{
}

BOOL LLPanelDirClassified::postBuild()
{
	LLPanelDirBrowser::postBuild();

	// Teens don't get mature checkbox
	if (gAgent.isTeen())
	{
		childSetValue("incmature", FALSE);
		childHide("incmature");
	}

	// 0 or 3+ character searches allowed, exciting
	childSetKeystrokeCallback("name", onKeystrokeNameClassified, this);
	
	childSetAction("Search", onClickSearchCore, this);
	childSetAction("Browse", onClickSearchCore, this);
	setDefaultBtn( "Browse" );

	childSetAction("Place an Ad...", onClickCreateNewClassified, this);

	childSetAction("Delete", onClickDelete, this);
	childDisable("Delete");
	childHide("Delete");

	// Don't do this every time we open find, it's expensive; require clicking 'search'
	//requestClassified();

	return TRUE;
}


LLPanelDirClassified::~LLPanelDirClassified()
{
	// Children all cleaned up by default view destructor.
}


void LLPanelDirClassified::draw()
{
	refresh();

	LLPanelDirBrowser::draw();
}

void LLPanelDirClassified::refresh()
{
	BOOL godlike = gAgent.isGodlike();
	childSetVisible("Delete", godlike);
	childSetEnabled("Delete", godlike);
}

//Open Profile to Classifieds tab
void LLPanelDirClassified::onClickCreateNewClassified(void *userdata)
{
	LLFloaterAvatarInfo::showFromObject(gAgent.getID(), "Classified");
}

// static
void LLPanelDirClassified::onClickDelete(void *userdata)
{
	LLPanelDirClassified *self = (LLPanelDirClassified *)userdata;

	LLUUID classified_id;
	S32 type;

	self->getSelectedInfo(&classified_id, &type);

	// Clear out the list.  Deleting a classified will cause a refresh to be
	// sent.
	self->setupNewSearch();

	LLMessageSystem* msg = gMessageSystem;

	msg->newMessageFast(_PREHASH_ClassifiedGodDelete);
	msg->nextBlockFast(_PREHASH_AgentData);
	msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
	msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
	msg->nextBlockFast(_PREHASH_Data);
	msg->addUUIDFast(_PREHASH_ClassifiedID, classified_id);
	msg->addUUIDFast(_PREHASH_QueryID, self->mSearchID);
	gAgent.sendReliableMessage();
}


void LLPanelDirClassified::performQuery()
{
	lldebugs << "LLPanelDirClassified::performQuery()" << llendl;
	// This sets mSearchID and clears the list of results
	setupNewSearch();

	// send the message
	LLMessageSystem *msg = gMessageSystem;
	msg->newMessageFast(_PREHASH_DirClassifiedQuery);
	msg->nextBlockFast(_PREHASH_AgentData);
	msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID() );
	msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());

	BOOL filter_mature = !childGetValue("incmature").asBoolean();
	BOOL filter_auto_renew = FALSE;
	U32 query_flags = pack_classified_flags(filter_mature, filter_auto_renew);
	//if (gAgent.isTeen()) query_flags |= DFQ_PG_SIMS_ONLY;

	U32 category = childGetValue("Category").asInteger();
	
	msg->nextBlockFast(_PREHASH_QueryData);
	msg->addUUIDFast(_PREHASH_QueryID, mSearchID );
	msg->addStringFast(_PREHASH_QueryText, childGetValue("name").asString());
	msg->addU32Fast(_PREHASH_QueryFlags, query_flags);
	msg->addU32Fast(_PREHASH_Category, category);
	msg->addS32Fast(_PREHASH_QueryStart,mSearchStart);

	gAgent.sendReliableMessage();
}

void LLPanelDirClassified::onKeystrokeNameClassified(LLLineEditor* line, void* data)
{
	LLPanelDirClassified *self = (LLPanelDirClassified*)data;

	S32 len = line->getLength();
	if (len == 0
		|| len >= 3)
	{
		// no text searches are cheap, as are longer searches
		self->setDefaultBtn( "Search" );
		self->childEnable("Search");
	}
	else
	{
		self->setDefaultBtn();
		self->childDisable("Search");
	}

	// Change the Browse to Search or vice versa
	if (len > 0)
	{
		self->childSetVisible("Search", TRUE);
		self->childSetVisible("Browse", FALSE);
	}
	else
	{
		self->setDefaultBtn( "Browse" );
		self->childSetVisible("Search", FALSE);
		self->childSetVisible("Browse", TRUE);
	}
}
