/**
* @file llprefsadvanced.cpp
* @brief Advanced preferences options for Imprudence
*
* $LicenseInfo:firstyear=2009&license=viewergpl$
*
* Copyright (c) 2010, McCabe Maxsted
*
* Imprudence Viewer Source Code
* The source code in this file ("Source Code") is provided to you
* under the terms of the GNU General Public License, version 2.0
* ("GPL"). Terms of the GPL can be found in doc/GPL-license.txt in
* this distribution, or online at
* http://secondlifegrid.net/programs/open_source/licensing/gplv2
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
* ALL SOURCE CODE IS PROVIDED "AS IS." THE AUTHOR MAKES NO
* WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
* COMPLETENESS OR PERFORMANCE.
* $/LicenseInfo$
*/

#include "llviewerprecompiledheaders.h"

#include "llcombobox.h"

#include "llagent.h"
#include "llprefsadvanced.h"
#include "llviewercontrol.h"
#include "llviewermenu.h"
#include "llvoavatar.h"

#include "lluictrlfactory.h"

LLPrefsAdvanced::LLPrefsAdvanced()
{
	LLUICtrlFactory::getInstance()->buildPanel(this, "panel_preferences_advanced.xml");
	childSetCommitCallback("speed_rez_check", onCommitCheckBox, this);

	childSetAction("reset_btn", onClickResetPrefs, this);
}

LLPrefsAdvanced::~LLPrefsAdvanced()
{
	// Children all cleaned up by default view destructor.
}

BOOL LLPrefsAdvanced::postBuild()
{
	childSetValue("disable_log_screen_check", gSavedSettings.getBOOL("DisableLoginLogoutScreens"));
	childSetValue("disable_tp_screen_check", gSavedSettings.getBOOL("DisableTeleportScreens"));
	childSetValue("client_name_tag_check", gSavedSettings.getBOOL("ShowClientNameTag"));
	childSetValue("client_name_color_check", gSavedSettings.getBOOL("ShowClientColor"));
	childSetValue("client_name_hover_check", gSavedSettings.getBOOL("ShowClientNameHoverTip"));
	childSetValue("client_name_tag_broadcast_check", gSavedSettings.getBOOL("ShowMyClientTagToOthers"));
	childSetValue("http_texture_check", gSavedSettings.getBOOL("ImagePipelineUseHTTP"));
	childSetValue("speed_rez_check", gSavedSettings.getBOOL("SpeedRez"));
	childSetValue("speed_rez_interval_spinner", (F32)gSavedSettings.getU32("SpeedRezInterval"));
	childSetValue("appearance_anim_check", gSavedSettings.getBOOL("AppearanceAnimate"));
	childSetValue("legacy_pie_menu_checkbox", gSavedSettings.getBOOL("LegacyPieEnabled"));
	childSetValue("language_is_public", gSavedSettings.getBOOL("LanguageIsPublic"));
	childSetValue("allow_mupose", gSavedSettings.getBOOL("AllowMUpose"));
	childSetValue("auto_close_ooc", gSavedSettings.getBOOL("AutoCloseOOC"));
	childSetValue("shadows_check", gSavedSettings.getBOOL("ShadowsEnabled"));

	childSetValue("lightshare_combo",
	              LLSD((S32)gSavedSettings.getU32("LightShareAllowed")));

	LLComboBox* crash_behavior_combobox = getChild<LLComboBox>("crash_behavior_combobox");
	crash_behavior_combobox->setCurrentByIndex(gCrashSettings.getS32(CRASH_BEHAVIOR_SETTING));

	refresh();

	return TRUE;
}

void LLPrefsAdvanced::apply()
{
	gSavedSettings.setBOOL("DisableLoginLogoutScreens", childGetValue("disable_log_screen_check"));
	gSavedSettings.setBOOL("DisableTeleportScreens", childGetValue("disable_tp_screen_check"));
	gSavedSettings.setBOOL("ShowClientNameTag", childGetValue("client_name_tag_check"));
	gSavedSettings.setBOOL("ShowClientColor", childGetValue("client_name_color_check"));
	gSavedSettings.setBOOL("ShowClientNameHoverTip", childGetValue("client_name_hover_check"));
	gSavedSettings.setBOOL("ImagePipelineUseHTTP", childGetValue("http_texture_check"));
	gSavedSettings.setBOOL("SpeedRez", childGetValue("speed_rez_check"));
	gSavedSettings.setU32("SpeedRezInterval", childGetValue("speed_rez_interval_spinner").asReal());
	gSavedSettings.setBOOL("AppearanceAnimate", childGetValue("appearance_anim_check"));
	gSavedSettings.setBOOL("LanguageIsPublic", childGetValue("language_is_public"));
	gSavedSettings.setBOOL("AllowMUpose", childGetValue("allow_mupose"));
	gSavedSettings.setBOOL("AutoCloseOOC", childGetValue("auto_close_ooc"));
	gSavedSettings.setU32("LightShareAllowed",
	                      (U32)childGetValue("lightshare_combo").asInteger());


	// Need to force a rebake when ClothingLayerProtection toggled for it take effect -- MC
	if (gSavedSettings.getBOOL("ShowMyClientTagToOthers") != (BOOL)childGetValue("client_name_tag_broadcast_check"))
	{
		if(gSavedSettings.getBOOL("ShowMyClientTagToOthers"))
		{
			//ShowMyClientTagToOthers works only with ClothingLayerProtection true,
			//while not showing also works with ClothingLayerProtection false.
			//since ClothingLayerProtection true is preferrable only switch ON
			gSavedSettings.setBOOL("ClothingLayerProtection", TRUE);
		}

		LLVOAvatar* avatar = gAgent.getAvatarObject();
		if (avatar)
		{
			// Slam pending upload count to "unstick" things
			bool slam_for_debug = true;
			avatar->forceBakeAllTextures(slam_for_debug);
		}
	}
	gSavedSettings.setBOOL("ShowMyClientTagToOthers", childGetValue("client_name_tag_broadcast_check"));

	// This is bad bad BAD UI from Emerald, I know. 
	// If anyone wants to do this better, please do -- MC
	if ( childGetValue("shadows_check").asBoolean() )
	{
		if ( (gSavedSettings.getU32("RenderQualityPerformance") < 3) // Make sure we have everything enabled
			|| !gSavedSettings.getBOOL("WindLightUseAtmosShaders")
			|| !gSavedSettings.getBOOL("VertexShaderEnable") ) 
		{
			childSetValue("shadows_check", FALSE);
			LLNotifications::instance().add("NoShadows");
			llwarns << "Attempting to enable shadow rendering while graphics settings less than Ultra or shaders are missing!" << llendl;
			gSavedSettings.setBOOL("ShadowsEnabled", FALSE);
		}
		else if ( (gSavedSettings.getBOOL("WindLightUseAtmosShaders") // If we do, toggle shadows in the correct order
				&& gSavedSettings.getBOOL("VertexShaderEnable")) )
		{
			gSavedSettings.setBOOL("RenderUseFBO", childGetValue("shadows_check").asBoolean());
			gSavedSettings.setBOOL("RenderDeferred", childGetValue("shadows_check").asBoolean());
			llinfos << "Shadow rendering enabled" << llendl;
			gSavedSettings.setBOOL("ShadowsEnabled", TRUE);
		}
	}
	else if (!childGetValue("shadows_check").asBoolean()) 
	{
		if (gSavedSettings.getBOOL("RenderDeferred"))
		{
			gSavedSettings.setBOOL("RenderDeferred", childGetValue("shadows_check").asBoolean());
			gSavedSettings.setBOOL("RenderUseFBO", childGetValue("shadows_check").asBoolean());
			llinfos << "Shadow rendering disabled" << llendl;
			gSavedSettings.setBOOL("ShadowsEnabled", FALSE);
		}
	}

	if (gSavedSettings.getBOOL("LegacyPieEnabled") == !((BOOL)childGetValue("legacy_pie_menu_checkbox")))
	{
		gSavedSettings.setBOOL("LegacyPieEnabled", childGetValue("legacy_pie_menu_checkbox"));
		build_pie_menus();
	}

	LLComboBox* crash_behavior_combobox = getChild<LLComboBox>("crash_behavior_combobox");
	gCrashSettings.setS32(CRASH_BEHAVIOR_SETTING, crash_behavior_combobox->getCurrentIndex());
}

void LLPrefsAdvanced::cancel()
{
}

void LLPrefsAdvanced::refresh()
{
	if (childGetValue("speed_rez_check").asBoolean())
	{
		childEnable("speed_rez_interval_spinner");
		childEnable("speed_rez_seconds_text");
	}
	else
	{
		childDisable("speed_rez_interval_spinner");
		childDisable("speed_rez_seconds_text");
	}
}

//static
void LLPrefsAdvanced::onCommitCheckBox(LLUICtrl* ctrl, void* user_data)
{
	LLPrefsAdvanced* self = (LLPrefsAdvanced*)user_data;
	self->refresh();
}

// static
void LLPrefsAdvanced::onClickResetPrefs(void* user_data)
{
	LLPrefsAdvanced* self = (LLPrefsAdvanced*)user_data;
	LLNotifications::instance().add("ConfirmResetAllPreferences", LLSD(), LLSD(), boost::bind(callbackReset, _1, _2, self));
}

// static
bool LLPrefsAdvanced::callbackReset(const LLSD& notification, const LLSD& response, LLPrefsAdvanced *self)
{
	S32 option = LLNotification::getSelectedOption(notification, response);
	if ( option == 0 )
	{
		gSavedSettings.setBOOL("ResetAllPreferences", TRUE);
	}
	return false;
}
