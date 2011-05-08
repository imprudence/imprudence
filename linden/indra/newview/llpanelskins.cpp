/** 
 * @file llpanelskins.cpp
 * @brief General preferences panel in preferences floater
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

#include "llpanelskins.h"

// linden library includes
#include "llradiogroup.h"
#include "llbutton.h"
#include "lluictrlfactory.h"

// project includes
#include "llviewercontrol.h"
#include "llviewerwindow.h"

LLPanelSkins::LLPanelSkins()
{
	LLUICtrlFactory::getInstance()->buildPanel(this, "panel_preferences_skins.xml");
}

LLPanelSkins::~LLPanelSkins()
{
}

BOOL LLPanelSkins::postBuild()
{
	LLRadioGroup* skin_select = getChild<LLRadioGroup>("skin_selection");
	skin_select->setCommitCallback(onSelectSkin);
	skin_select->setCallbackUserData(this);

	getChild<LLButton>("classic_preview")->setClickedCallback(onClickClassic, this);
	getChild<LLButton>("silver_preview")->setClickedCallback(onClickSilver, this);
	getChild<LLButton>("dark_preview")->setClickedCallback(onClickDark, this);
	getChild<LLButton>("gemini_preview")->setClickedCallback(onClickGemini, this);

	refresh();
	return TRUE;
}

void LLPanelSkins::refresh()
{
	mSkin = gSavedSettings.getString("SkinCurrent");
	getChild<LLRadioGroup>("skin_selection")->setValue(mSkin);
}

void LLPanelSkins::apply()
{
	if (mSkin != gSavedSettings.getString("SkinCurrent"))
	{
		  LLNotifications::instance().add("ChangeSkin");
		  refresh();
	}
}

void LLPanelSkins::cancel()
{
	// reverts any changes to current skin
	gSavedSettings.setString("SkinCurrent", mSkin);
}

//static
void LLPanelSkins::onSelectSkin(LLUICtrl* ctrl, void* data)
{
	std::string skin_selection = ctrl->getValue().asString();
	gSavedSettings.setString("SkinCurrent", skin_selection);
}

//static 
void LLPanelSkins::onClickClassic(void* data)
{
	LLPanelSkins* self = (LLPanelSkins*)data;
	gSavedSettings.setString("SkinCurrent", "default");
	self->getChild<LLRadioGroup>("skin_selection")->setValue("default");
}

//static 
void LLPanelSkins::onClickSilver(void* data)
{
	LLPanelSkins* self = (LLPanelSkins*)data;
	gSavedSettings.setString("SkinCurrent", "silver");
	self->getChild<LLRadioGroup>("skin_selection")->setValue("silver");
}

//static 
void LLPanelSkins::onClickDark(void* data)
{
	LLPanelSkins* self = (LLPanelSkins*)data;
	gSavedSettings.setString("SkinCurrent", "dark");
	self->getChild<LLRadioGroup>("skin_selection")->setValue("dark");
}

//static 
void LLPanelSkins::onClickGemini(void* data)
{
	LLPanelSkins* self = (LLPanelSkins*)data;
	gSavedSettings.setString("SkinCurrent", "gemini");
	self->getChild<LLRadioGroup>("skin_selection")->setValue("gemini");
}
