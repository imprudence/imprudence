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
#include "llcombobox.h"
#include "llcolorswatch.h"
#include "llstartup.h"
#include "lluictrlfactory.h"
#include "lltexturectrl.h"

#include "boost/algorithm/string.hpp"

LLPrefsAdvanced* LLPrefsAdvanced::sInstance;

LLPrefsAdvanced::LLPrefsAdvanced()
{
	LLUICtrlFactory::getInstance()->buildPanel(this, "panel_preferences_advanced.xml");
	if(sInstance)delete sInstance;
	sInstance = this;

	childSetCommitCallback("speed_rez_check", onCommitCheckBox, this);
}

LLPrefsAdvanced::~LLPrefsAdvanced()
{
	// Children all cleaned up by default view destructor.
	sInstance = NULL;
}

BOOL LLPrefsAdvanced::postBuild()
{
	childSetValue("disable_log_screen_check", gSavedSettings.getBOOL("DisableLoginLogoutScreens"));
	childSetValue("disable_tp_screen_check", gSavedSettings.getBOOL("DisableTeleportScreens"));

	static BOOL* sShowClientNameTag = rebind_llcontrol<BOOL>("ShowClientNameTag", &gSavedSettings, true);
	childSetValue("client_name_tag_check", (*sShowClientNameTag));
	childSetValue("client_name_hover_check", gSavedSettings.getBOOL("ShowClientNameHoverTip"));
	childSetValue("client_name_tag_broadcast_check", gSavedSettings.getBOOL("ShowMyClientTagToOthers"));

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

	getChild<LLButton>("reset_cloud_this_account")->setClickedCallback(onResetThisCloudButton,this);
	getChild<LLButton>("save_cloud_this_account")->setClickedCallback(onSaveThisCloudButton,this);
	getChild<LLButton>("save_cloud_any_account")->setClickedCallback(onSaveAnyoneCloudButton,this);

	childSetValue("automatic_fly", gSavedSettings.getBOOL("AutomaticFly"));

	static BOOL* sEnableWindlightRemote = rebind_llcontrol<BOOL>("EnableWindlightRemote", &gSavedSettings, true);
	childSetValue("toggle_windlight_control", (*sEnableWindlightRemote));
	mWLControl = (*sEnableWindlightRemote);

	childSetValue("script_errors_as_chat", gSavedSettings.getBOOL("ScriptErrorsAsChat"));
	childSetValue("show_timestamps_check", gSavedSettings.getBOOL("ShowTimestamps"));
	childSetValue("arrow_keys_move_avatar_check", gSavedSettings.getBOOL("ArrowKeysMoveAvatar"));

	refresh();

	return TRUE;
}		

void LLPrefsAdvanced::apply()
{
	gSavedSettings.setBOOL("DisableLoginLogoutScreens", childGetValue("disable_log_screen_check"));
	gSavedSettings.setBOOL("DisableTeleportScreens", childGetValue("disable_tp_screen_check"));
	gSavedSettings.setBOOL("ShowClientNameTag", childGetValue("client_name_tag_check"));
	gSavedSettings.setBOOL("ShowClientNameHoverTip", childGetValue("client_name_hover_check"));

	gSavedSettings.setBOOL("AppearanceAnimate", childGetValue("appearance_anim_check"));
	gSavedSettings.setBOOL("LanguageIsPublic", childGetValue("language_is_public"));

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
	
	if (LLStartUp::isLoggedIn() && LLVOAvatar::sHasCloud)
	{
		onSaveThisCloudButton(NULL);
	}

	gSavedSettings.setBOOL("AutomaticFly", childGetValue("automatic_fly"));

	// windlight remote
	static BOOL* sEnableWindlightRemote = rebind_llcontrol<BOOL>("EnableWindlightRemote", &gSavedSettings, true);
	mWLControl = (*sEnableWindlightRemote);

	gSavedSettings.setBOOL("AllowMUpose", childGetValue("allow_mupose"));
	gSavedSettings.setBOOL("AutoCloseOOC", childGetValue("auto_close_ooc"));
	gSavedSettings.setBOOL("ArrowKeysMoveAvatar", childGetValue("arrow_keys_move_avatar_check"));
	gSavedSettings.setBOOL("ScriptErrorsAsChat", childGetValue("script_errors_as_chat"));
	gSavedSettings.setBOOL("ShowTimestamps", childGetValue("show_timestamps_check"));
}

void LLPrefsAdvanced::cancel()
{
// 	llwarns << "cancel" << llendl;
//	cool - "ok" is also cancel
// 	LLVOAvatar::sCloud.mPartData.mStartColor = mCloudStartColor;
// 	LLVOAvatar::sCloud.mPartData.mEndColor = mCloudEndColor;

	gSavedSettings.setBOOL("EnableWindlightRemote", mWLControl);
}

void LLPrefsAdvanced::refresh()
{
	bool is_logged_in = LLStartUp::isLoggedIn();
	setParticleControls(is_logged_in);
}

void LLPrefsAdvanced::draw()
{

	bool is_logged_in = LLStartUp::isLoggedIn();
	if(LLVOAvatar::sHasCloud && mWasLoggedIn != is_logged_in)
	{


		setParticleControls(is_logged_in);

		mCloudStartColor = LLVOAvatar::sCloud.mPartData.mStartColor;
		mCloudEndColor = LLVOAvatar::sCloud.mPartData.mEndColor;


		getChild<LLColorSwatchCtrl>("part_start_color_swatch")->set(mCloudStartColor);
		getChild<LLColorSwatchCtrl>("part_end_color_swatch")->set(mCloudEndColor);


		mCloudTextureID = LLVOAvatar::sCloud.mPartImageID;
		LLTextureCtrl* texture_ctrl = getChild<LLTextureCtrl>("part_texture_picker");
		if (texture_ctrl)
		{
			texture_ctrl->setImageAssetID(mCloudTextureID);
			texture_ctrl->setAllowNoTexture( true );

			// Don't allow (no copy) or (no transfer) textures to be selected.
			// With that we are less permissive than the original LL code of
			// LLWaterParamManager, which allows picking (and saving) plain texture uuids
			// without any permission check.
			// If real absurdity is necessary we let the user save a windlight water setting
			// for the texture uuid.

			texture_ctrl->setImmediateFilterPermMask(PERM_NONE);//PERM_COPY | PERM_TRANSFER);
			texture_ctrl->setNonImmediateFilterPermMask(PERM_NONE);
		}


		mWasLoggedIn = is_logged_in;
	}

	if(LLVOAvatar::sHasCloud && is_logged_in)
	{

		LLVOAvatar::sCloud.mPartData.mStartColor = childGetValue("part_start_color_swatch");
		LLVOAvatar::sCloud.mPartData.mEndColor = childGetValue("part_end_color_swatch");
		LLVOAvatar::sCloud.mPartImageID  = childGetValue("part_texture_picker");
	}

	LLPanel::draw();
}

void LLPrefsAdvanced::setParticleControls(bool is_logged_in)
{
	childSetEnabled("reset_cloud_this_account", is_logged_in );
	childSetEnabled("save_cloud_this_account", is_logged_in);
	childSetEnabled("save_cloud_any_account", is_logged_in);
	childSetEnabled("part_start_color_swatch", is_logged_in);
	childSetEnabled("part_end_color_swatch", is_logged_in);
	childSetEnabled("part_texture_picker", is_logged_in );

	childSetEnabled("preview_cloud", is_logged_in);
	childSetVisible("preview_cloud", is_logged_in);
	childSetVisible("must_be_logged_in_textbox", !is_logged_in);
}

//static
void LLPrefsAdvanced::onCommitCheckBox(LLUICtrl* ctrl, void* user_data)
{
	LLPrefsAdvanced* self = (LLPrefsAdvanced*)user_data;
	if (self)
	{
		self->refresh();
	}
}

void LLPrefsAdvanced::onResetThisCloudButton(void * data)
{
	LLPrefsAdvanced* self = (LLPrefsAdvanced*)data;
	if (!self)
	{
		return;
	}

	// keep draw() from overriding the cloud with the values from the UI
	LLVOAvatar::sHasCloud = false; 
	std::string filename = gDirUtilp->getExpandedFilename(LL_PATH_APP_SETTINGS, "cloud.xml");
	LLVOAvatar::loadCloud(filename, LLVOAvatar::sCloud);

	// tell draw() to override values from the UI with the data from the new cloud
	self-> mWasLoggedIn = !(LLStartUp::isLoggedIn());

	LLVOAvatar::sHasCloud = true;
}

void LLPrefsAdvanced::onSaveThisCloudButton(void * data)
{
	std::string filename = gDirUtilp->getExpandedFilename(LL_PATH_PER_SL_ACCOUNT, "cloud.xml");
	LLVOAvatar::saveCloud(filename, LLVOAvatar::sCloud);
}

void LLPrefsAdvanced::onSaveAnyoneCloudButton(void * data)
{
	std::string filename = gDirUtilp->getExpandedFilename(LL_PATH_APP_SETTINGS, "cloud.xml");
	LLVOAvatar::saveCloud(filename, LLVOAvatar::sCloud);
}

