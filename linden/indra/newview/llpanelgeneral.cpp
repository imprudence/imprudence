/** 
 * @file llpanelgeneral.cpp
 * @brief General preferences panel in preferences floater
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

#include "llpanelgeneral.h"

// linden library includes
#include "llerror.h"
#include "llrect.h"
#include "llfontgl.h"
#include "message.h"
#include "llvieweruictrlfactory.h"

// project includes
#include "llagent.h"
#include "llviewerwindow.h"
#include "llcolorswatch.h"
#include "llcombobox.h"
#include "llconsole.h"
#include "lllineeditor.h"
#include "llpanellogin.h"
#include "llnetmap.h"
#include "llresmgr.h"
#include "llspinctrl.h"
#include "lltextbox.h"
#include "llui.h"
#include "llurlsimstring.h"
#include "llviewercontrol.h"
#include "llappviewer.h"
#include "llurlsimstring.h"
#include "llappviewer.h"
//
// Imported globals
//

void set_crash_behavior(LLUICtrl* ctrl, void* data);
void set_start_location(LLUICtrl* ctrl, void* data);


//
// Globals
//

//
// Static functions
//
void set_crash_behavior(LLUICtrl* ctrl, void* data)
{
	LLAppViewer::instance()->setCrashBehavior(((LLComboBox*) ctrl)->getCurrentIndex());
}

void set_language(LLUICtrl* ctrl, void* data)
{
    gSavedSettings.setString("Language", ctrl->getValue().asString());
}

void LLPanelGeneral::set_start_location(LLUICtrl* ctrl, void* data)
{
    LLURLSimString::setString(ctrl->getValue().asString());
}

void LLPanelGeneral::set_specific_start_location(LLLineEditor* line_editor, void* data)
{
    LLURLSimString::setString(line_editor->getValue().asString());
}

LLPanelGeneral::LLPanelGeneral()
{
	gUICtrlFactory->buildPanel(this, "panel_preferences_general.xml");
}

BOOL LLPanelGeneral::postBuild()
{
	requires("location_combobox", WIDGET_TYPE_COMBO_BOX);
	requires("region_name_prompt", WIDGET_TYPE_TEXT_BOX);
	requires("show_location_checkbox", WIDGET_TYPE_CHECKBOX);
	requires("fade_out_radio", WIDGET_TYPE_RADIO_GROUP);
	requires("show_my_name_checkbox", WIDGET_TYPE_CHECKBOX);
	requires("show_my_title_checkbox", WIDGET_TYPE_CHECKBOX);
	requires("small_avatar_names_checkbox", WIDGET_TYPE_CHECKBOX);
	requires("effect_color_swatch", WIDGET_TYPE_COLOR_SWATCH);
	requires("afk_timeout_spinner", WIDGET_TYPE_SPINNER);
	requires("rotate_mini_map_checkbox", WIDGET_TYPE_CHECKBOX);
	requires("friends_online_notify_checkbox", WIDGET_TYPE_CHECKBOX);
	requires("notify_money_change_checkbox", WIDGET_TYPE_CHECKBOX);
	requires("use_system_color_picker_checkbox", WIDGET_TYPE_CHECKBOX);
	requires("crash_behavior_combobox", WIDGET_TYPE_COMBO_BOX);

	if (!checkRequirements())
	{
		return FALSE;
	}

	LLString region_name_prompt = childGetText("region_name_prompt");


	// location combobox
	LLComboBox* combo = LLUICtrlFactory::getComboBoxByName(this, "location_combobox");
	if (combo)
	{
		if (!LLURLSimString::sInstance.mSimString.empty())
		{
			combo->setTextEntry(LLURLSimString::sInstance.mSimString);
		}
	
		BOOL login_last = gSavedSettings.getBOOL("LoginLastLocation");
		if (!LLURLSimString::sInstance.mSimString.empty())
		{
			combo->add( LLURLSimString::sInstance.mSimString );
			combo->setCurrentByIndex( 2 );
		}
		else
		{
			combo->add( region_name_prompt );
			combo->setCurrentByIndex( login_last ? 1 : 0 );
		}
		combo->setCommitCallback( &set_start_location );
		combo->setTextEntryCallback( &set_specific_start_location );
	}
	
	// Show location on login screen
	childSetCommitCallback("show_location_checkbox", &LLPanelGeneral::clickShowStartLocation);

	combo = LLUICtrlFactory::getComboBoxByName(this, "crash_behavior_combobox");
	if (combo)
	{
		combo->setCurrentByIndex( LLAppViewer::instance()->getCrashBehavior() );
		combo->setCommitCallback( &set_crash_behavior );
	}
	
	childSetCommitCallback("language_combobox", set_language );
	childSetValue("language_combobox", 	gSavedSettings.getString("Language"));

	refresh();

	return TRUE;
}

LLPanelGeneral::~LLPanelGeneral()
{
	// Children all cleaned up by default view destructor.
}

void LLPanelGeneral::refresh()
{
	LLPanel::refresh();
	BOOL login_last = gSavedSettings.getBOOL("LoginLastLocation");
	LLComboBox* combo = LLUICtrlFactory::getComboBoxByName(this, "location_combobox");
	if (combo)
	{
		if (!LLURLSimString::sInstance.mSimString.empty())
		{
			combo->setCurrentByIndex( 2 );
		}
		else
		{
			combo->setCurrentByIndex( login_last ? 1 : 0 );
		}
		//save current settings in case cancel is clicked
		mLoginLocation = combo->getValue().asString();
	}
	
	mCrashBehavior = LLAppViewer::instance()->getCrashBehavior();
	combo = LLUICtrlFactory::getComboBoxByName(this, "crash_behavior_combobox");
	if (combo)
	{
		combo->setCurrentByIndex( LLAppViewer::instance()->getCrashBehavior() );
	}
	
	mRenderName = gSavedSettings.getS32("RenderName");
	mRenderNameHideSelf = gSavedSettings.getBOOL("RenderNameHideSelf");
	mSmallAvatarNames = gSavedSettings.getBOOL("SmallAvatarNames");
	mRenderHideGroupTitle = gSavedSettings.getBOOL("RenderHideGroupTitle");
	mChatOnlineNotification = gSavedSettings.getBOOL("ChatOnlineNotification");
	mAFKTimeout = gSavedSettings.getF32("AFKTimeout");
	mMiniMapRotate = gSavedSettings.getBOOL("MiniMapRotate");
	mNotifyMoney = gSavedSettings.getBOOL("NotifyMoneyChange");
	mUseDefaultColor = gSavedSettings.getBOOL("UseDefaultColorPicker");
	mEffectColor = gSavedSettings.getColor4("EffectColor");

	mLanguage = gSavedSettings.getString("Language");
}

void LLPanelGeneral::apply()
{
}

void LLPanelGeneral::cancel()
{
	gSavedSettings.setS32("RenderName", mRenderName);
	gSavedSettings.setBOOL("RenderNameHideSelf", mRenderNameHideSelf );
	gSavedSettings.setBOOL("SmallAvatarNames", mSmallAvatarNames );
	gSavedSettings.setBOOL("RenderHideGroupTitle", mRenderHideGroupTitle );
	gSavedSettings.setBOOL("ChatOnlineNotification", mChatOnlineNotification );
	gSavedSettings.setF32("AFKTimeout", mAFKTimeout );
	gSavedSettings.setBOOL("MiniMapRotate", mMiniMapRotate );
	gSavedSettings.setBOOL("NotifyMoneyChange", mNotifyMoney );
	gSavedSettings.setBOOL("UseDefaultColorPicker", mUseDefaultColor );
	gSavedSettings.setColor4("EffectColor", mEffectColor );
	gSavedSettings.setString("Language", mLanguage);
	
	LLURLSimString::setString(mLoginLocation);

	LLAppViewer::instance()->setCrashBehavior(mCrashBehavior);

	// make listener
	//gAFKTimeout = away_timeout;
}

void LLPanelGeneral::clickShowStartLocation(LLUICtrl*, void* user_data)
{
	LLPanelLogin::refreshLocation( false ); // in case LLPanelLogin is visible
}
