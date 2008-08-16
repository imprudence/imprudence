/** 
 * @file llpanelinput.cpp
 * @brief Input preferences panel
 *
 * Copyright (c) 2004-2007, Linden Research, Inc.
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

#include "llpanelinput.h"

// linden library includes
#include "llerror.h"
#include "llrect.h"
#include "llfontgl.h"
#include "message.h"
#include "llvieweruictrlfactory.h"

// project includes
#include "llviewerwindow.h"
#include "llcheckboxctrl.h"
#include "llradiogroup.h"
#include "llresmgr.h"
#include "llspinctrl.h"
#include "llslider.h"
#include "llsliderctrl.h"
#include "lltextbox.h"
#include "llui.h"
#include "viewer.h"

//Ventrella
#include "llagent.h"
//end Ventrella


//
// Imported globals
//

extern F32 gMouseSensitivity;
extern BOOL gInvertMouse;

//
// Globals
//

//
// Static functions
//


LLPanelInput::LLPanelInput() 
{
	gUICtrlFactory->buildPanel(this, "panel_preferences_input.xml");
}

BOOL LLPanelInput::postBuild()
{
	requires("Mouse Sensitivity", WIDGET_TYPE_SLIDER_BAR);
	requires("invert mouse", WIDGET_TYPE_CHECKBOX);
	requires("automatic fly", WIDGET_TYPE_CHECKBOX);
	requires("dynamic camera", WIDGET_TYPE_SLIDER);
	requires("edit camera movement", WIDGET_TYPE_CHECKBOX);
	requires("appearance camera movement", WIDGET_TYPE_CHECKBOX);
	
	if (!checkRequirements())
	{
		return FALSE;
	}	

	refresh();

	return TRUE;
}


LLPanelInput::~LLPanelInput()
{
	// Children all cleaned up by default view destructor.
}

void LLPanelInput::refresh()
{
	LLPanel::refresh();

	mMouseSensitivityVal = gSavedSettings.getF32("MouseSensitivity");
	mAutomaticFly =gSavedSettings.getBOOL("AutomaticFly");
	mInvertMouse = gSavedSettings.getBOOL("InvertMouse");
	mEditCameraMovement = gSavedSettings.getBOOL("EditCameraMovement");
	mAppearanceCameraMovement = gSavedSettings.getBOOL("AppearanceCameraMovement");
	mDynamicCameraStrengthVal = gSavedSettings.getF32("DynamicCameraStrength");
	mNumpadControlVal = gSavedSettings.getS32("NumpadControl");
}

void LLPanelInput::apply()
{

}

void LLPanelInput::cancel()
{
	gSavedSettings.setF32("MouseSensitivity", mMouseSensitivityVal);
	gSavedSettings.setBOOL("AutomaticFly", mAutomaticFly);
	gSavedSettings.setBOOL("InvertMouse", mInvertMouse);
	gSavedSettings.setBOOL("EditCameraMovement", mEditCameraMovement);
	gSavedSettings.setBOOL("AppearanceCameraMovement", mAppearanceCameraMovement);
	gSavedSettings.setF32("DynamicCameraStrength", mDynamicCameraStrengthVal);
	gSavedSettings.setS32("NumpadControl", mNumpadControlVal);
}

