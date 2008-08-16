/** 
 * @file llcameraview.cpp
 * @brief Container for movement buttons like forward, left, fly
 *
 * Copyright (c) 2001-2007, Linden Research, Inc.
 * 
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

#include "llcameraview.h"

// Library includes
#include "llfontgl.h"

// Viewer includes
#include "llagent.h"
#include "lljoystickbutton.h"
#include "llviewercontrol.h"

// Constants
const char *CAMERA_TITLE = "";
const F32 CAMERA_BUTTON_DELAY = 0.0f;

// Globals
LLFloaterCamera* gFloaterCamera = NULL;


//
// Member functions
//

LLFloaterCamera::LLFloaterCamera(const std::string& name)
:	LLFloater(name, "FloaterCameraRect", CAMERA_TITLE, FALSE, 100, 100, DRAG_ON_TOP,
			  MINIMIZE_NO)
{
	setIsChrome(TRUE);
	
	S32 top = mRect.getHeight();
	S32 bottom = 0;
	S32 left = 16;

	const S32 ROTATE_WIDTH = 64;
	mRotate = new LLJoystickCameraRotate("cam rotate stick", 
		LLRect( left, top, left + ROTATE_WIDTH, bottom ),
		"cam_rotate_out.tga",
		"cam_rotate_in.tga" );
	mRotate->setFollows(FOLLOWS_TOP | FOLLOWS_LEFT);
	mRotate->setHeldDownDelay(CAMERA_BUTTON_DELAY);
	mRotate->setToolTip( "Rotate Camera Around Focus" );
	mRotate->setSoundFlags(MOUSE_DOWN | MOUSE_UP);
	addChild(mRotate);

	left += ROTATE_WIDTH;

	const S32 ZOOM_WIDTH = 16;
	mZoom = new LLJoystickCameraZoom( 
		"zoom",
		LLRect( left, top, left + ZOOM_WIDTH, bottom ),
		"cam_zoom_out.tga",
		"cam_zoom_plus_in.tga",
		"cam_zoom_minus_in.tga");
	mZoom->setFollows(FOLLOWS_TOP | FOLLOWS_LEFT);
	mZoom->setHeldDownDelay(CAMERA_BUTTON_DELAY);
	mZoom->setToolTip( "Zoom Camera Towards Focus" );
	mZoom->setSoundFlags(MOUSE_DOWN | MOUSE_UP);
	addChild(mZoom);

	left += ZOOM_WIDTH;
	
	const S32 TRACK_WIDTH = 64;
	mTrack = new LLJoystickCameraTrack("cam track stick", 
		LLRect( left, top, left + TRACK_WIDTH, bottom ),
		"cam_tracking_out.tga",
		"cam_tracking_in.tga");
	mTrack->setFollows(FOLLOWS_TOP | FOLLOWS_LEFT);
	mTrack->setHeldDownDelay(CAMERA_BUTTON_DELAY);
	mTrack->setToolTip( "Move Camera Up and Down, Left and Right" );
	mTrack->setSoundFlags(MOUSE_DOWN | MOUSE_UP);
	addChild(mTrack);
}


LLFloaterCamera::~LLFloaterCamera()
{
	// children all deleted by LLView destructor
	gFloaterCamera = NULL;
}

// virtual
void LLFloaterCamera::onClose(bool app_quitting)
{
	LLFloater::onClose(app_quitting);
	
	if (!app_quitting)
	{
		gSavedSettings.setBOOL("ShowCameraControls", FALSE);
	}
}

//
// Static member functions
//

// static
void LLFloaterCamera::show(void*)
{
	if (gFloaterCamera)
	{
		gFloaterCamera->open();
	}
	else
	{
		gFloaterCamera = new LLFloaterCamera("camera floater");

		gFloaterCamera->open();
	}
	
	gSavedSettings.setBOOL("ShowCameraControls", TRUE);
}

// static
void LLFloaterCamera::toggle(void*)
{
	if (gFloaterCamera)
	{
		gFloaterCamera->close();
	}
	else
	{
		show(NULL);
	}
}

// static
BOOL LLFloaterCamera::visible(void*)
{
	if (gFloaterCamera)
	{
		return gFloaterCamera->getVisible();
	}
	else
	{
		return FALSE;
	}
}

// EOF
