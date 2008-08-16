/** 
 * @file llfloatermute.cpp
 * @brief Container for mute list
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

#include "llfloatermute.h"

#include "llfontgl.h"
#include "llrect.h"
#include "llerror.h"
#include "llstring.h"
#include "message.h"

// project include
#include "llagent.h"
#include "llfloateravatarpicker.h"
#include "llbutton.h"
#include "lllineeditor.h"
#include "llmutelist.h"
#include "llresizehandle.h"
#include "llscrolllistctrl.h"
#include "lltextbox.h"
#include "llviewertexteditor.h"
#include "llviewerwindow.h"
#include "llvieweruictrlfactory.h"

//
// Constants
//
const char FLOATER_TITLE[] = "Muted Residents & Objects";
const F32 INSTANT_MSG_SIZE = 8.0f;
const LLColor4 INSTANT_MSG_COLOR(1, 1, 1, 1);
const LLColor4 MUTED_MSG_COLOR(0.5f, 0.5f, 0.5f, 1.f);

const S32 LINE = 16;
const S32 LEFT = 2;
const S32 VPAD = 4;
const S32 HPAD = 4;
//
// Global statics
//
LLFloaterMute* gFloaterMute = NULL;


//
// Member Functions
//

//-----------------------------------------------------------------------------
// LLFloaterMute()
//-----------------------------------------------------------------------------
LLFloaterMute::LLFloaterMute()
:	LLFloater("mute floater", "FloaterMuteRect3", FLOATER_TITLE, 
			  RESIZE_YES, 220, 140, DRAG_ON_TOP, MINIMIZE_YES, CLOSE_YES)
{

	gUICtrlFactory->buildFloater(this, "floater_mute.xml");
	
	setVisible(FALSE);

	childSetCommitCallback("mutes", onSelectName, this);
	childSetAction("Mute resident...", onClickPick, this);
	childSetAction("Mute object by name...", onClickMuteByName, this);
	childSetAction("Unmute", onClickRemove, this);

	mMuteList = LLUICtrlFactory::getScrollListByName(this, "mutes");
	
	refreshMuteList();
}

//-----------------------------------------------------------------------------
// ~LLFloaterMute()
//-----------------------------------------------------------------------------
LLFloaterMute::~LLFloaterMute()
{
}


//-----------------------------------------------------------------------------
// show()
//-----------------------------------------------------------------------------
void LLFloaterMute::show()
{
	// Make sure we make a noise.
	open();		/* Flawfinder: ignore */
}

//-----------------------------------------------------------------------------
// toggle()
//-----------------------------------------------------------------------------
void LLFloaterMute::toggle(void*)
{
	if (gFloaterMute->getVisible())
	{
		gFloaterMute->close();
	}
	else
	{
		gFloaterMute->show();
	}
}

//-----------------------------------------------------------------------------
// visible()
//-----------------------------------------------------------------------------
BOOL LLFloaterMute::visible(void*)
{
	return (gFloaterMute && gFloaterMute->getVisible());
}

//-----------------------------------------------------------------------------
// refreshMuteList()
//-----------------------------------------------------------------------------
void LLFloaterMute::refreshMuteList()
{
	mMuteList->deleteAllItems();

	if (gMuteListp)
	{
		std::vector<LLMute> mutes = gMuteListp->getMutes();
		std::vector<LLMute>::iterator it;
		for (it = mutes.begin(); it != mutes.end(); ++it)
		{
			LLString display_name = it->getDisplayName();
			mMuteList->addStringUUIDItem(display_name, it->mID);
		}
	}

	updateButtons();
}

void LLFloaterMute::selectMute(const LLUUID& mute_id)
{
	mMuteList->selectByID(mute_id);
	updateButtons();
}

//-----------------------------------------------------------------------------
// updateButtons()
//-----------------------------------------------------------------------------
void LLFloaterMute::updateButtons()
{
	if (mMuteList->getFirstSelected())
	{
		childSetEnabled("Unmute", TRUE);
	}
	else
	{
		childSetEnabled("Unmute", FALSE);
	}
}

//-----------------------------------------------------------------------------
// onSelectName()
//-----------------------------------------------------------------------------
void LLFloaterMute::onSelectName(LLUICtrl *caller, void *data)
{
	LLFloaterMute *floater = (LLFloaterMute*)data;

	floater->updateButtons();
}

//-----------------------------------------------------------------------------
// onClickRemove()
//-----------------------------------------------------------------------------
void LLFloaterMute::onClickRemove(void *data)
{
	LLFloaterMute* floater = (LLFloaterMute *)data;

	LLString name = floater->mMuteList->getSimpleSelectedItem();
	LLUUID id = floater->mMuteList->getStringUUIDSelectedItem();
	LLMute mute(id);
	mute.setFromDisplayName(name);
	// now mute.mName has the suffix trimmed off
	
	S32 last_selected = floater->mMuteList->getFirstSelectedIndex();
	if (gMuteListp->remove(mute))
	{
		// Above removals may rebuild this dialog.
		
		if (last_selected == floater->mMuteList->getItemCount())
		{
			// we were on the last item, so select the last item again
			floater->mMuteList->selectNthItem(last_selected - 1);
		}
		else
		{
			// else select the item after the last item previously selected
			floater->mMuteList->selectNthItem(last_selected);
		}
	}
	floater->updateButtons();
}

//-----------------------------------------------------------------------------
// onClickPick()
//-----------------------------------------------------------------------------
void LLFloaterMute::onClickPick(void *data)
{
	LLFloaterMute* floaterp = (LLFloaterMute*)data;
	const BOOL allow_multiple = FALSE;
	const BOOL close_on_select = TRUE;
	LLFloaterAvatarPicker* picker = LLFloaterAvatarPicker::show(onPickUser, data, allow_multiple, close_on_select);
	floaterp->addDependentFloater(picker);
}

//-----------------------------------------------------------------------------
// onPickUser()
//-----------------------------------------------------------------------------
void LLFloaterMute::onPickUser(const std::vector<std::string>& names, const std::vector<LLUUID>& ids, void* user_data)
{
	LLFloaterMute* floaterp = (LLFloaterMute*)user_data;
	if (!floaterp) return;
	if (names.empty() || ids.empty()) return;

	LLMute mute(ids[0], names[0], LLMute::AGENT);
	gMuteListp->add(mute);
	floaterp->updateButtons();
}


void LLFloaterMute::onClickMuteByName(void* data)
{


	LLString::format_map_t args;
	gViewerWindow->alertXmlEditText("MuteByName", args,
									NULL, NULL,
									callbackMuteByName, data);


}


void LLFloaterMute::callbackMuteByName(S32 option, const LLString& text, void* data)
{
	if (option != 0) return;
	if (text.empty()) return;

	LLMute mute(LLUUID::null, text, LLMute::BY_NAME);
	BOOL success = gMuteListp->add(mute);
	if (!success)
	{
		gViewerWindow->alertXml("MuteByNameFailed");
	}
}
