/** 
 * @file llpanelmsgs.cpp
 * @brief Message popup preferences panel
 *
 * $LicenseInfo:firstyear=2003&license=viewergpl$
 * 
 * Copyright (c) 2003-2009, Linden Research, Inc.
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

#include "llpanelmsgs.h"

#include "llscrolllistctrl.h"
#include "llviewerwindow.h"
#include "llviewercontrol.h"
#include "lluictrlfactory.h"
#include "llfirstuse.h"

//-----------------------------------------------------------------------------
LLPanelMsgs::LLPanelMsgs() : 
	LLPanel(std::string("Messages Panel")),
	mDisabledPopups( NULL ),
	mEnabledPopups( NULL )
{
	LLUICtrlFactory::getInstance()->buildPanel(this, "panel_preferences_popups.xml");
}


LLPanelMsgs::~LLPanelMsgs()
{ }

//-----------------------------------------------------------------------------
// postBuild()
//-----------------------------------------------------------------------------
BOOL LLPanelMsgs::postBuild()
{
	mDisabledPopups = getChild<LLScrollListCtrl>("disabled_popups");
	mEnabledPopups = getChild<LLScrollListCtrl>("enabled_popups");
	childSetAction("enable_popup", onClickEnablePopup, this);
	childSetAction("reset_dialogs_btn", onClickResetDialogs, this);
	childSetAction("skip_dialogs_btn", onClickSkipDialogs, this);
	buildLists();

	childSetValue("accept_new_inventory", gSavedSettings.getBOOL("AutoAcceptNewInventory"));
	childSetValue("show_new_inventory", gSavedSettings.getBOOL("ShowNewInventory"));
	childSetValue("show_in_inventory", gSavedSettings.getBOOL("ShowInInventory"));

	return TRUE;
}

void LLPanelMsgs::buildLists()
{
	if ( mDisabledPopups )
		mDisabledPopups->deleteAllItems();

	if ( mEnabledPopups )
		mEnabledPopups->deleteAllItems();

	for (LLAlertDialog::template_map_t::iterator iter = LLAlertDialog::sIgnorableTemplates.begin();
		 iter != LLAlertDialog::sIgnorableTemplates.end(); ++iter)
	{
		LLAlertDialogTemplate* alert_temp = iter->second;
		S32 ignore = alert_temp->getIgnore();

		LLSD row;
		row["columns"][0]["value"] = alert_temp->mIgnoreListText;
		row["columns"][0]["font"] = "SANSSERIF_SMALL";
		row["columns"][0]["width"] = 300;

		LLScrollListItem* item = NULL;


		if (ignore)
		{
			if (ignore == LLAlertDialog::IGNORE_USE_SAVED)
			{
				S32 arg = LLUI::sConfigGroup->getS32("Default" + alert_temp->mIgnoreLabel);
				row["columns"][1]["value"] = alert_temp->mOptionDefaultText[arg];
				row["columns"][1]["font"] = "SANSSERIF_SMALL";
				row["columns"][1]["width"] = 160;
			}
			if (mDisabledPopups)
			{
				item = mDisabledPopups->addElement(row,
								   ADD_SORTED);
			}
			else
			{
				llwarns << "(ignore) but also (!mDisabledPopups)" << llendl;
			}
		}
		else
		{
			if (mEnabledPopups)
			{
				item = mEnabledPopups->addElement(row,
								  ADD_SORTED);
			}
			else
			{
				llwarns << "(!ignore) but also (!mEnabledPopups)" << llendl;
			}
		}

		if (item)
		{
			item->setUserdata((void*)&iter->first);
		}
	}	
}

void LLPanelMsgs::draw()
{
	if (mDisabledPopups->getFirstSelected())
	{
		childEnable("enable_popup");
	}
	else
	{
		childDisable("enable_popup");
	}

	LLPanel::draw();
}


void LLPanelMsgs::apply()
{
	gSavedSettings.setBOOL("AutoAcceptNewInventory", childGetValue("accept_new_inventory"));
	gSavedSettings.setBOOL("ShowNewInventory", childGetValue("show_new_inventory"));
	gSavedSettings.setBOOL("ShowInInventory", childGetValue("show_in_inventory"));
}

void LLPanelMsgs::cancel()
{
}

void LLPanelMsgs::resetAllIgnored()
{
	for(LLAlertDialog::template_map_t::iterator iter = LLAlertDialog::sIgnorableTemplates.begin();
		iter != LLAlertDialog::sIgnorableTemplates.end(); ++iter)
	{
		LLAlertDialogTemplate* alert_temp = iter->second;
		S32 ignore = alert_temp->getIgnore();
		if(ignore)
			alert_temp->setIgnore(false);
	}
}

void LLPanelMsgs::setAllIgnored()
{
	for(LLAlertDialog::template_map_t::iterator iter = LLAlertDialog::sIgnorableTemplates.begin();
		iter != LLAlertDialog::sIgnorableTemplates.end(); ++iter)
	{
		LLAlertDialogTemplate* alert_temp = iter->second;
		alert_temp->setIgnore(true);
	}
}

//static 
void LLPanelMsgs::onClickEnablePopup(void* user_data)
{
	LLPanelMsgs* panelp = (LLPanelMsgs*)user_data;

	std::vector<LLScrollListItem*> items = panelp->mDisabledPopups->getAllSelected();
	std::vector<LLScrollListItem*>::iterator itor;
	for (itor = items.begin(); itor != items.end(); ++itor)
	{
		LLAlertDialog::template_map_t::iterator found_alert = LLAlertDialog::sAlertTemplates.find(*(std::string*)((*itor)->getUserdata()));
		if (found_alert != LLAlertDialog::sAlertTemplates.end())
		{
			LLAlertDialogTemplate* alert_temp = LLAlertDialog::sAlertTemplates[*(std::string*)((*itor)->getUserdata())];
			gSavedSettings.setWarning(alert_temp->mIgnoreLabel, TRUE);
		}
	}

	panelp->buildLists();
}

void callback_reset_dialogs(S32 option, void* data)
{
	if (0 == option)
	{
		LLPanelMsgs* panelp = (LLPanelMsgs*)data;
		if ( panelp )
		{
			panelp->resetAllIgnored();
			LLFirstUse::resetFirstUse();
			panelp->buildLists();
		}
	}
}

// static
void LLPanelMsgs::onClickResetDialogs(void* user_data)
{
	gViewerWindow->alertXml("ResetShowNextTimeDialogs",callback_reset_dialogs,user_data);
}

void callback_skip_dialogs(S32 option, void* data)
{
	if (0 == option)
	{
		LLPanelMsgs* panelp = (LLPanelMsgs*)data;
		if ( panelp )
		{
			panelp->setAllIgnored();
			LLFirstUse::disableFirstUse();
			panelp->buildLists();
		}
	}
}

// static
void LLPanelMsgs::onClickSkipDialogs(void* user_data)
{
	gViewerWindow->alertXml("SkipShowNextTimeDialogs", callback_skip_dialogs, user_data);
}
