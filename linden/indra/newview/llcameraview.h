/** 
 * @file llcameraview.h
 * @brief Container for buttons for walking, turning, flying
 *
 * Copyright (c) 2001-2007, Linden Research, Inc.
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

// llcameraview.h
//
// Copyright 2001-2003, Linden Reserach, Inc.
// 

#ifndef LL_LLCAMERAVIEW_H
#define LL_LLCAMERAVIEW_H

// Library includes
#include "llfloater.h"
#include "llstatview.h"

class LLJoystickCameraRotate;
class LLJoystickCameraZoom;
class LLJoystickCameraTrack;

//
// Classes
//
class LLFloaterCamera
:	public LLFloater
{
public:
	LLFloaterCamera(const std::string& name);
	~LLFloaterCamera();

	/*virtual*/ void onClose(bool app_quitting);
	
	static void show(void*);
	static void toggle(void*);
	static BOOL visible(void*);

public:
	LLJoystickCameraRotate* mRotate;
	LLJoystickCameraZoom*	mZoom;
	LLJoystickCameraTrack*	mTrack;
};

class LLFloaterJoystick
:	public LLFloater
{
protected:
	LLFloaterJoystick();
	~LLFloaterJoystick();

public:
	static void		show(void*);
	static LLFloaterJoystick* getInstance();
	static BOOL		visible(void*);
	virtual void draw();
	
protected:
	static LLFloaterJoystick*	sInstance;
	LLStatView*		mAxisStats;
	LLStat*			mAxis[6];
	LLStatBar*		mAxisBar[6];
};

extern LLFloaterCamera *gFloaterCamera;

#endif
