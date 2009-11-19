/** 
 * @file llpanellcd.cpp
 * @brief lcd options panel
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

#include "llpanelLCD.h"

// linden library includes
#include "llerror.h"
#include "llrect.h"
#include "llfontgl.h"
#include "message.h"
#include "lluictrlfactory.h"

// project includes
#include "llviewerwindow.h"
#include "llcheckboxctrl.h"
#include "llradiogroup.h"
#include "llresmgr.h"
#include "lltextbox.h"
#include "llui.h"
#include "llviewercontrol.h"

//Ventrella
#include "llagent.h"
//end Ventrella

// for Logitech LCD keyboards / speakers
#ifndef LL_LCD_H
#include "lllcd.h"
#endif


//
// Globals
//

//
// Static functions
//


LLPanelLCD::LLPanelLCD() 
{
	LLUICtrlFactory::getInstance()->buildPanel(this, "panel_preferences_lcd.xml");
}

BOOL LLPanelLCD::postBuild()
{
	requires<LLRadioGroup>("LCDDestination");
	requires<LLCheckBoxCtrl>("DisplayLinden");
	requires<LLCheckBoxCtrl>("DisplayDebug");
	requires<LLCheckBoxCtrl>("DisplayDebugConsole");
	requires<LLCheckBoxCtrl>("DisplayRegion");
	requires<LLCheckBoxCtrl>("DisplayChat");
	requires<LLCheckBoxCtrl>("DisplayIM");

	if (!checkRequirements())
	{
		return FALSE;
	}	

	refresh();

	return TRUE;
}


LLPanelLCD::~LLPanelLCD()
{
	// Children all cleaned up by default view destructor.
}

void LLPanelLCD::refresh()
{
	mLCDDestination = gSavedSettings.getS32("LCDDestination");
	mDisplayChat = gSavedSettings.getBOOL("DisplayChat");
	mDisplayIM = gSavedSettings.getBOOL("DisplayIM");
	mDisplayRegion = gSavedSettings.getBOOL("DisplayRegion");
	mDisplayDebug = gSavedSettings.getBOOL("DisplayDebug");
	mDisplayDebugConsole = gSavedSettings.getBOOL("DisplayDebugConsole");
	mDisplayLinden = gSavedSettings.getBOOL("DisplayLinden");

	LLPanel::refresh();
}

void LLPanelLCD::apply()
{
	// nothing really to do here.
}


void LLPanelLCD::cancel()
{
	// doing this to restore situation when we entered this function
	gSavedSettings.setS32("LCDDestination", mLCDDestination);
	gSavedSettings.setBOOL("DisplayChat", mDisplayChat);
	gSavedSettings.setBOOL("DisplayIM", mDisplayIM);
	gSavedSettings.setBOOL("DisplayRegion", mDisplayRegion);
	gSavedSettings.setBOOL("DisplayDebug", mDisplayDebug);
	gSavedSettings.setBOOL("DisplayDebugConsole", mDisplayDebugConsole);
	gSavedSettings.setBOOL("DisplayLinden", mDisplayLinden);
}