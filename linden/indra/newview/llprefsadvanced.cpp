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

#include "llagent.h"
#include "llprefsadvanced.h"
#include "llviewercontrol.h"
#include "llvoavatar.h"

#include "lluictrlfactory.h"

LLPrefsAdvanced::LLPrefsAdvanced()
{
	LLUICtrlFactory::getInstance()->buildPanel(this, "panel_preferences_advanced.xml");
	childSetCommitCallback("speed_rez_check", onCommitCheckBox, this);
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
	childSetValue("client_name_tag_broadcast_check", gSavedSettings.getBOOL("ClothingLayerProtection"));
	childSetValue("http_texture_check", gSavedSettings.getBOOL("ImagePipelineUseHTTP"));
	childSetValue("speed_rez_check", gSavedSettings.getBOOL("SpeedRez"));
	childSetValue("speed_rez_interval_spinner", (F32)gSavedSettings.getU32("SpeedRezInterval"));
	childSetValue("appearance_stand_check", gSavedSettings.getBOOL("AppearanceForceStand"));
	childSetValue("mute_wind_check", !gSavedSettings.getBOOL("MuteWind"));

	refresh();

	return TRUE;
}

void LLPrefsAdvanced::apply()
{
	gSavedSettings.setBOOL("DisableLoginLogoutScreens", childGetValue("disable_log_screen_check"));
	gSavedSettings.setBOOL("DisableTeleportScreens", childGetValue("disable_tp_screen_check"));
	gSavedSettings.setBOOL("ShowClientNameTag", childGetValue("client_name_tag_check"));
	gSavedSettings.setBOOL("ImagePipelineUseHTTP", childGetValue("http_texture_check"));
	gSavedSettings.setBOOL("SpeedRez", childGetValue("speed_rez_check"));
	gSavedSettings.setU32("SpeedRezInterval", childGetValue("speed_rez_interval_spinner").asReal());
	gSavedSettings.setBOOL("AppearanceForceStand", childGetValue("appearance_stand_check"));
	gSavedSettings.setBOOL("MuteWind", !(BOOL)(childGetValue("mute_wind_check").asBoolean()));

	// Need to force a rebake when ClothingLayerProtection toggled for it take effect -- MC
	if (gSavedSettings.getBOOL("ClothingLayerProtection") != (BOOL)childGetValue("client_name_tag_broadcast_check"))
	{
		LLVOAvatar* avatar = gAgent.getAvatarObject();
		if (avatar)
		{
			// Slam pending upload count to "unstick" things
			bool slam_for_debug = true;
			avatar->forceBakeAllTextures(slam_for_debug);
		}
	}
	gSavedSettings.setBOOL("ClothingLayerProtection", childGetValue("client_name_tag_broadcast_check"));

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
		}
		else if ( (gSavedSettings.getBOOL("WindLightUseAtmosShaders") // If we do, toggle shadows in the correct order
				&& gSavedSettings.getBOOL("VertexShaderEnable")) )
		{
			gSavedSettings.setBOOL("RenderUseFBO", childGetValue("shadows_check").asBoolean());
			gSavedSettings.setBOOL("RenderDeferred", childGetValue("shadows_check").asBoolean());
			llinfos << "Shadow rendering enabled" << llendl;
		}
	}
	else if (!childGetValue("shadows_check").asBoolean()) 
	{
		if (gSavedSettings.getBOOL("RenderDeferred"))
		{
			gSavedSettings.setBOOL("RenderDeferred", childGetValue("shadows_check").asBoolean());
			gSavedSettings.setBOOL("RenderUseFBO", childGetValue("shadows_check").asBoolean());
			llinfos << "Shadow rendering disabled" << llendl;
		}
	}
	gSavedSettings.setBOOL("ShadowsEnabled", childGetValue("shadows_check").asBoolean());
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
