/** 
 * @file llpanelgeneral.cpp
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

//file include
#include "llpanelgeneral.h"

// project includes
#include "llcolorswatch.h"
#include "llcombobox.h"
#include "lluictrlfactory.h"
#include "llurlsimstring.h"
#include "llviewercontrol.h"

LLPanelGeneral::LLPanelGeneral()
{
	LLUICtrlFactory::getInstance()->buildPanel(this, "panel_preferences_general.xml");
}

BOOL LLPanelGeneral::postBuild()
{
	LLComboBox* fade_out_combobox = getChild<LLComboBox>("fade_out_combobox");
	fade_out_combobox->setCurrentByIndex(gSavedSettings.getS32("RenderName"));

	childSetValue("default_start_location", gSavedSettings.getBOOL("LoginLastLocation") ? "MyLastLocation" : "MyHome");
	childSetValue("show_location_checkbox", gSavedSettings.getBOOL("ShowStartLocation"));
	childSetValue("show_all_title_checkbox", gSavedSettings.getBOOL("RenderHideGroupTitleAll"));
	childSetValue("language_is_public", gSavedSettings.getBOOL("LanguageIsPublic"));

	childSetValue("show_my_name_checkbox", gSavedSettings.getBOOL("RenderNameHideSelf"));
	childSetValue("small_avatar_names_checkbox", gSavedSettings.getBOOL("SmallAvatarNames"));
	childSetValue("show_my_title_checkbox", gSavedSettings.getBOOL("RenderHideGroupTitle"));
	childSetValue("afk_timeout_spinner", gSavedSettings.getF32("AFKTimeout"));
	childSetValue("mini_map_notify_chat", gSavedSettings.getBOOL("MiniMapNotifyChatRange"));
	childSetValue("mini_map_notify_sim", gSavedSettings.getBOOL("MiniMapNotifySimRange"));
	childSetValue("notify_money_change_checkbox", gSavedSettings.getBOOL("NotifyMoneyChange"));
	childSetValue("use_system_color_picker_checkbox", gSavedSettings.getBOOL("UseDefaultColorPicker"));
	childSetValue("show_search_panel", gSavedSettings.getBOOL("ShowSearchBar"));

	getChild<LLColorSwatchCtrl>("effect_color_swatch")->set(gSavedSettings.getColor4("EffectColor"));

	childSetValue("ui_scale_slider", gSavedSettings.getF32("UIScaleFactor"));
	childSetValue("ui_auto_scale", gSavedSettings.getBOOL("UIAutoScale"));

	LLComboBox* crash_behavior_combobox = getChild<LLComboBox>("crash_behavior_combobox");
	crash_behavior_combobox->setCurrentByIndex(gCrashSettings.getS32(CRASH_BEHAVIOR_SETTING));
	
	childSetValue("language_combobox", 	gSavedSettings.getString("Language"));

	childSetAction("reset_ui_size", onClickResetUISize, this);

	return TRUE;
}

LLPanelGeneral::~LLPanelGeneral()
{
	// Children all cleaned up by default view destructor.
}

void LLPanelGeneral::apply()
{
	LLComboBox* fade_out_combobox = getChild<LLComboBox>("fade_out_combobox");
	gSavedSettings.setS32("RenderName", fade_out_combobox->getCurrentIndex());
	
	gSavedSettings.setBOOL("LoginLastLocation", childGetValue("default_start_location").asString() == "MyLastLocation");
	gSavedSettings.setBOOL("ShowStartLocation", childGetValue("show_location_checkbox"));
	gSavedSettings.setBOOL("RenderHideGroupTitleAll", childGetValue("show_all_title_checkbox"));
	gSavedSettings.setBOOL("LanguageIsPublic", childGetValue("language_is_public"));
	gSavedSettings.setBOOL("RenderNameHideSelf", childGetValue("show_my_name_checkbox"));
	gSavedSettings.setBOOL("SmallAvatarNames", childGetValue("small_avatar_names_checkbox"));
	gSavedSettings.setBOOL("RenderHideGroupTitle", childGetValue("show_my_title_checkbox"));
	gSavedSettings.setF32("AFKTimeout", childGetValue("afk_timeout_spinner").asReal());
	gSavedSettings.setBOOL("MiniMapNotifyChatRange", childGetValue("mini_map_notify_chat"));
	gSavedSettings.setBOOL("MiniMapNotifySimRange", childGetValue("mini_map_notify_sim"));
	gSavedSettings.setBOOL("NotifyMoneyChange", childGetValue("notify_money_change_checkbox"));
	gSavedSettings.setBOOL("UseDefaultColorPicker", childGetValue("use_system_color_picker_checkbox"));
	gSavedSettings.setBOOL("ShowSearchBar", childGetValue("show_search_panel"));
	gSavedSettings.setColor4("EffectColor", childGetValue("effect_color_swatch"));
	gSavedSettings.setF32("UIScaleFactor", childGetValue("ui_scale_slider").asReal());
	gSavedSettings.setBOOL("UIAutoScale", childGetValue("ui_auto_scale"));
	gSavedSettings.setString("Language", childGetValue("language_combobox"));

	LLURLSimString::setString(childGetValue("location_combobox"));

	LLComboBox* crash_behavior_combobox = getChild<LLComboBox>("crash_behavior_combobox");
	gCrashSettings.setS32(CRASH_BEHAVIOR_SETTING, crash_behavior_combobox->getCurrentIndex());
}

void LLPanelGeneral::cancel()
{
}

// static
void LLPanelGeneral::onClickResetUISize(void* user_data)
{
	LLPanelGeneral* self = (LLPanelGeneral*)user_data;
	self->childSetValue("ui_scale_slider", 1.002f);
}
