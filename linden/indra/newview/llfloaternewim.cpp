/** 
 * @file llfloaternewim.cpp
 * @brief Panel allowing the user to create a new IM session.
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

#include "llfloaternewim.h"

#include "llvieweruictrlfactory.h"
#include "llinstantmessage.h"
#include "sound_ids.h"
#include "audioengine.h"
#include "llfontgl.h"
#include "llnamevalue.h"

#include "llagent.h"
#include "llbutton.h"
#include "llfloater.h"
#include "llfontgl.h"
#include "llimpanel.h"
#include "llkeyboard.h"
#include "llnamelistctrl.h"
#include "llresmgr.h"
#include "lltabcontainer.h"
#include "llimview.h"
#include "lltextbox.h"

extern LLAudioEngine*   gAudiop;
extern BOOL gInitializationComplete;
S32 COL_1_WIDTH = 200;

static LLString sOnlineDescriptor = "*";
static LLString sNameFormat = "[FIRST] [LAST]";

LLFloaterNewIM::LLFloaterNewIM()
{
	gUICtrlFactory->buildFloater(this, "floater_new_im.xml");
}

BOOL LLFloaterNewIM::postBuild()
{
	requires("start_btn", WIDGET_TYPE_BUTTON);
	requires("close_btn", WIDGET_TYPE_BUTTON);
	requires("user_list", WIDGET_TYPE_NAME_LIST);
	requires("online_descriptor", WIDGET_TYPE_TEXT_BOX);
	requires("name_format", WIDGET_TYPE_TEXT_BOX);

	if (checkRequirements())
	{
		childSetAction("start_btn", &LLFloaterNewIM::onStart, this);
		childSetAction("close_btn", &LLFloaterNewIM::onClickClose, this);
		mSelectionList = LLViewerUICtrlFactory::getNameListByName(this, "user_list");
		if (mSelectionList)
		{
			mSelectionList->setDoubleClickCallback(&LLFloaterNewIM::onStart);
			mSelectionList->setCallbackUserData(this);
		}
		else
		{
			llwarns << "LLViewerUICtrlFactory::getNameListByName() returned NULL for 'user_list'" << llendl;
		}
		sOnlineDescriptor = childGetValue("online_descriptor").asString();
		sNameFormat = childGetValue("name_format").asString();
		setDefaultBtn("start_btn");
		return TRUE;
	}	

	return FALSE;
}


LLFloaterNewIM::~LLFloaterNewIM()
{
	clearAllTargets();
}


void LLFloaterNewIM::clearAllTargets()
{
	mSelectionList->deleteAllItems();
}

void LLFloaterNewIM::addSpecial(const LLUUID& uuid, const std::string& name,
							 void* data, BOOL bold, BOOL online)
{
	LLSD row;
	row["id"] = uuid;
	row["name"] = name;
	row["target"] = "SPECIAL";
	row["columns"][0]["value"] = name;
	row["columns"][0]["width"] = COL_1_WIDTH;
	row["columns"][0]["font"] = "SANSSERIF";
	row["columns"][0]["font-style"] = bold ? "BOLD" : "NORMAL";
	row["columns"][1]["value"] = online ? sOnlineDescriptor : "";
	row["columns"][1]["font"] = "SANSSERIF";
	row["columns"][1]["font-style"] = "BOLD";
	LLScrollListItem* itemp = mSelectionList->addElement(row);
	itemp->setUserdata(data);

	if (mSelectionList->getFirstSelectedIndex() == -1)
	{
		mSelectionList->selectFirstItem();
	}
}

void LLFloaterNewIM::addGroup(const LLUUID& uuid, void* data, BOOL bold, BOOL online)
{
	LLSD row;
	row["id"] = uuid;
	row["target"] = "GROUP";
	row["columns"][0]["value"] = ""; // name will be looked up
	row["columns"][0]["width"] = COL_1_WIDTH;
	row["columns"][0]["font"] = "SANSSERIF";
	row["columns"][0]["font-style"] = bold ? "BOLD" : "NORMAL";
	row["columns"][1]["value"] = online ? sOnlineDescriptor : "";
	row["columns"][1]["font"] = "SANSSERIF";
	row["columns"][1]["font-style"] = "BOLD";
	LLScrollListItem* itemp = mSelectionList->addElement(row);
	itemp->setUserdata(data);

	if (mSelectionList->getFirstSelectedIndex() == -1)
	{
		mSelectionList->selectFirstItem();
	}
}

void LLFloaterNewIM::addAgent(const LLUUID& uuid, void* data, BOOL online)
{
	char first[DB_FIRST_NAME_BUF_SIZE];		/* Flawfinder: ignore */
	first[0] = '\0';
	char last[DB_LAST_NAME_BUF_SIZE];		/* Flawfinder: ignore */
	last[0] = '\0';
	gCacheName->getName(uuid, first, last);
	LLUIString fullname = sNameFormat;
	fullname.setArg("[FIRST]", first);
	fullname.setArg("[LAST]", last);

	LLSD row;
	row["id"] = uuid;
	row["columns"][0]["value"] = fullname;
	row["columns"][0]["width"] = COL_1_WIDTH;
	row["columns"][0]["font"] = "SANSSERIF";
	row["columns"][0]["font-style"] = online ? "BOLD" : "NORMAL";
	row["columns"][1]["value"] = online ? sOnlineDescriptor : "";
	row["columns"][1]["font"] = "SANSSERIF";
	row["columns"][1]["font-style"] = "BOLD";
	LLScrollListItem* itemp = mSelectionList->addElement(row);
	itemp->setUserdata(data);

	if (mSelectionList->getFirstSelectedIndex() == -1)
	{
		mSelectionList->selectFirstItem();
	}
}

BOOL LLFloaterNewIM::isUUIDAvailable(const LLUUID& uuid)
{
	std::vector<LLScrollListItem*> data_list = mSelectionList->getAllData();
	std::vector<LLScrollListItem*>::iterator data_itor;
	for (data_itor = data_list.begin(); data_itor != data_list.end(); ++data_itor)
	{
		LLScrollListItem* item = *data_itor;
		if(item->getUUID() == uuid)
		{
			return TRUE;
		}
	}
	return FALSE;
}

void LLFloaterNewIM::onStart(void* userdata)
{
	LLFloaterNewIM* self = (LLFloaterNewIM*) userdata;

	LLScrollListItem* item = self->mSelectionList->getFirstSelected();
	if(item)
	{
		const LLScrollListCell* cell = item->getColumn(0);
		LLString name(cell->getText());

		// *NOTE: Do a live detrmination of what type of session it
		// should be. If we restrict the new im panel to online users,
		// then we can remove some of this code.
		EInstantMessage type;
		EInstantMessage* t = (EInstantMessage*)item->getUserdata();
		if(t) type = (*t);
		else type = LLIMView::defaultIMTypeForAgent(item->getUUID());
		gIMView->addSession(name, type, item->getUUID());

		make_ui_sound("UISndStartIM");
	}
	else
	{
		make_ui_sound("UISndInvalidOp");
	}
}


// static
void LLFloaterNewIM::onClickClose(void *userdata)
{
	gIMView->setFloaterOpen(FALSE);
}


BOOL LLFloaterNewIM::handleKeyHere(KEY key, MASK mask, BOOL called_from_parent)
{
	BOOL handled = LLFloater::handleKeyHere(key, mask, called_from_parent);
	if (getVisible() && mEnabled && !called_from_parent)
	{
		if ( KEY_ESCAPE == key )
		{
			handled = TRUE;
			// Close talk panel on escape
			gIMView->toggle(NULL);
		}
	}

	// Might need to call base class here if not handled
	return handled;
}

BOOL LLFloaterNewIM::canClose()
{
	if (getHost())
	{
		LLMultiFloater* hostp = (LLMultiFloater*)getHost();
		// if we are the only tab in the im view, go ahead and close
		return hostp->getFloaterCount() == 1;
	}
	return TRUE;
}

void LLFloaterNewIM::close(bool app_quitting)
{
	if (getHost())
	{
		LLMultiFloater* hostp = (LLMultiFloater*)getHost();
		hostp->close();
	}
	else
	{
		LLFloater::close(app_quitting);
	}
}

S32 LLFloaterNewIM::getScrollPos()
{
	return mSelectionList->getScrollPos();
}

void LLFloaterNewIM::setScrollPos( S32 pos )
{
	mSelectionList->setScrollPos( pos );
}
