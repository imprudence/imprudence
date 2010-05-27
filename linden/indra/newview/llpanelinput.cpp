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
#include "llpanelinput.h"
#include "lluictrlfactory.h"
#include "llviewercamera.h"
#include "llviewercontrol.h"
#include "llfloaterjoystick.h"
#include "llsliderctrl.h"
#include "llfloaterjoystick.h"


LLPanelInput::LLPanelInput() 
{
	LLUICtrlFactory::getInstance()->buildPanel(this, "panel_preferences_input.xml");
}

static void onFOVAdjust(LLUICtrl* source, void* data)
{
	LLSliderCtrl* slider = dynamic_cast<LLSliderCtrl*>(source);
	LLViewerCamera::getInstance()->setDefaultFOV(slider->getValueF32());
}

BOOL LLPanelInput::postBuild()
{
	childSetAction("joystick_setup_button", onClickJoystickSetup, (void*)this);

	// cache values in case user cancels
	mPreAdjustFOV = gSavedSettings.getF32("CameraAngle");
	mPreAdjustCameraOffsetScale = gSavedSettings.getF32("CameraOffsetScale");

	childSetValue("mouse_sensitivity", gSavedSettings.getF32("MouseSensitivity"));
	childSetValue("automatic_fly", gSavedSettings.getBOOL("AutomaticFly"));
	childSetValue("invert_mouse", gSavedSettings.getBOOL("InvertMouse"));
	childSetValue("edit_camera_movement", gSavedSettings.getBOOL("EditCameraMovement"));
	childSetValue("appearance_camera_movement", gSavedSettings.getBOOL("AppearanceCameraMovement"));
	childSetValue("zoom_time", gSavedSettings.getF32("ZoomTime"));
	childSetValue("camera_position_smoothing", gSavedSettings.getF32("CameraPositionSmoothing"));
	childSetValue("first_person_avatar_visible", gSavedSettings.getBOOL("FirstPersonAvatarVisible"));

	LLSliderCtrl* fov_slider = getChild<LLSliderCtrl>("camera_fov");
	fov_slider->setCommitCallback(&onFOVAdjust);
	fov_slider->setMinValue(LLViewerCamera::getInstance()->getMinView());
	fov_slider->setMaxValue(LLViewerCamera::getInstance()->getMaxView());
	fov_slider->setValue(LLViewerCamera::getInstance()->getView());
	
	childSetValue("double_click_action", gSavedSettings.getString("DoubleClickAction"));
	childSetValue("go_action", gSavedSettings.getString("GoAction"));

	childSetValue("Disable camera constraints", gSavedSettings.getBOOL("DisableCameraConstraints"));
	childSetValue("disable_min_zoom_check", gSavedSettings.getBOOL("DisableMinZoomDist"));

	return TRUE;
}

LLPanelInput::~LLPanelInput()
{
	// Children all cleaned up by default view destructor.
}

void LLPanelInput::apply()
{
	// any cancel after this point will use these new values
	mPreAdjustFOV = childGetValue("camera_fov").asReal();
	mPreAdjustCameraOffsetScale = childGetValue("camera_offset_scale").asReal();

	gSavedSettings.setF32("MouseSensitivity", childGetValue("mouse_sensitivity").asReal());
	gSavedSettings.setBOOL("AutomaticFly", childGetValue("automatic_fly"));
	gSavedSettings.setBOOL("InvertMouse", childGetValue("invert_mouse"));
	gSavedSettings.setBOOL("EditCameraMovement", childGetValue("edit_camera_movement"));
	gSavedSettings.setBOOL("AppearanceCameraMovement", childGetValue("appearance_camera_movement"));
	gSavedSettings.setF32("CameraAngle", mPreAdjustFOV);
	gSavedSettings.setF32("ZoomTime", childGetValue("zoom_time").asReal());
	gSavedSettings.setF32("CameraPositionSmoothing", childGetValue("camera_position_smoothing").asReal());
	gSavedSettings.setBOOL("FirstPersonAvatarVisible", childGetValue("first_person_avatar_visible"));

	gSavedSettings.setString("DoubleClickAction", childGetValue("double_click_action"));
	gSavedSettings.setString("GoAction", childGetValue("go_action"));

	gSavedSettings.setBOOL("DisableCameraConstraints", childGetValue("Disable camera constraints"));
	gSavedSettings.setBOOL("DisableMinZoomDist", childGetValue("disable_min_zoom_check"));
}

void LLPanelInput::cancel()
{
	LLViewerCamera::getInstance()->setView(mPreAdjustFOV);
	gSavedSettings.setF32("CameraAngle", LLViewerCamera::getInstance()->getView());
	gSavedSettings.setF32("CameraOffsetScale", mPreAdjustCameraOffsetScale);
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

