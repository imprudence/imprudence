/** 
 * @file llpanelinput.cpp
 * @brief Input preferences panel
 *
 * $LicenseInfo:firstyear=2004&license=viewergpl$
 * 
 * Copyright (c) 2004-2009, Linden Research, Inc.
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
#include "llpanelinput.h"
#include "lluictrlfactory.h"
#include "llviewercontrol.h"
#include "llfloaterjoystick.h"


LLPanelInput::LLPanelInput() 
{
	LLUICtrlFactory::getInstance()->buildPanel(this, "panel_preferences_input.xml");
}

BOOL LLPanelInput::postBuild()
{
	childSetAction("joystick_setup_button", onClickJoystickSetup, (void*)this);

	childSetValue("mouse_sensitivity", gSavedSettings.getF32("MouseSensitivity"));
	childSetValue("automatic_fly", gSavedSettings.getBOOL("AutomaticFly"));
	childSetValue("invert_mouse", gSavedSettings.getBOOL("InvertMouse"));
	childSetValue("edit_camera_movement", gSavedSettings.getBOOL("EditCameraMovement"));
	childSetValue("appearance_camera_movement", gSavedSettings.getBOOL("AppearanceCameraMovement"));
	childSetValue("dynamic_camera_strength", gSavedSettings.getF32("DynamicCameraStrength"));
	childSetValue("zoom_time", gSavedSettings.getF32("ZoomTime"));
	childSetValue("camera_position_smoothing", gSavedSettings.getF32("CameraPositionSmoothing"));
	childSetValue("first_person_avatar_visible", gSavedSettings.getBOOL("FirstPersonAvatarVisible"));

	childSetValue("double_click_action", gSavedSettings.getString("DoubleClickAction"));
	childSetValue("go_action", gSavedSettings.getString("GoAction"));

	return TRUE;
}

LLPanelInput::~LLPanelInput()
{
	// Children all cleaned up by default view destructor.
}

void LLPanelInput::apply()
{
	gSavedSettings.setF32("MouseSensitivity", childGetValue("mouse_sensitivity").asReal());
	gSavedSettings.setBOOL("AutomaticFly", childGetValue("automatic_fly"));
	gSavedSettings.setBOOL("InvertMouse", childGetValue("invert_mouse"));
	gSavedSettings.setBOOL("EditCameraMovement", childGetValue("edit_camera_movement"));
	gSavedSettings.setBOOL("AppearanceCameraMovement", childGetValue("appearance_camera_movement"));
	gSavedSettings.setF32("DynamicCameraStrength", childGetValue("dynamic_camera_strength").asReal());
	gSavedSettings.setF32("ZoomTime", childGetValue("zoom_time").asReal());
	gSavedSettings.setF32("CameraPositionSmoothing", childGetValue("camera_position_smoothing").asReal());
	gSavedSettings.setBOOL("FirstPersonAvatarVisible", childGetValue("first_person_avatar_visible"));

	gSavedSettings.setString("DoubleClickAction", childGetValue("double_click_action"));
	gSavedSettings.setString("GoAction", childGetValue("go_action"));

}

void LLPanelInput::cancel()
{
}

//static
void LLPanelInput::onClickJoystickSetup(void* user_data)
{
	LLPanelInput* prefs = (LLPanelInput*)user_data;
	LLFloaterJoystick* floaterp = LLFloaterJoystick::showInstance();
	LLFloater* parent_floater = gFloaterView->getParentFloater(prefs);
	if (parent_floater)
	{
		parent_floater->addDependentFloater(floaterp, FALSE);
	}
}

