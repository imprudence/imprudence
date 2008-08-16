/** 
 * @file llpanelaudiovolume.cpp
 * @brief A remote control for media (video and music)
 *
 * $LicenseInfo:firstyear=2005&license=viewergpl$
 * 
 * Copyright (c) 2005-2008, Linden Research, Inc.
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
 * $/LicenseInfo$
 */

#include "llviewerprecompiledheaders.h"

#include "llpanelaudiovolume.h"

#include "lloverlaybar.h"
#include "llviewercontrol.h"
#include "llvieweruictrlfactory.h"

////////////////////////////////////////////////////////////////////////////////
// Floater version of audio panel
//

//static
void* LLFloaterAudioVolume::createVolumePanel(void* data)
{
	LLPanelAudioVolume* panel = new LLPanelAudioVolume();
	return panel;
}

LLFloaterAudioVolume::LLFloaterAudioVolume(const LLSD& seed)
{
	mFactoryMap["Volume Panel"]	= LLCallbackMap(createVolumePanel, NULL);
	gUICtrlFactory->buildFloater(this, "floater_audio_volume.xml", &getFactoryMap());

	S32 pos_x = mRect.mLeft;
	S32 pos_y = mRect.mBottom;
	LLView* volume_panel_view = gOverlayBar->getChildByName("master_volume");
	if (volume_panel_view)
	{
		pos_x = volume_panel_view->getRect().mLeft;
		pos_y = volume_panel_view->getRect().mTop;
	}

	setOrigin(pos_x, pos_y);
	gFloaterView->adjustToFitScreen(this, FALSE);
}

////////////////////////////////////////////////////////////////////////////////
//
//
LLPanelAudioVolume::LLPanelAudioVolume()
{
}

BOOL LLPanelAudioVolume::postBuild()
{
	return TRUE;
}

LLPanelAudioVolume::~LLPanelAudioVolume ()
{
}

////////////////////////////////////////////////////////////////////////////////
//
//
void LLPanelAudioVolume::draw()
{
// 	LLOverlayBar::enableMusicButtons(this);
// 	LLOverlayBar::enableMediaButtons(this);
	BOOL mute = gSavedSettings.getBOOL("MuteAudio");
	bool enable = mute ? false : true;
	childSetEnabled("System Volume", enable);
	childSetEnabled("Music Volume", enable);
	childSetEnabled("Media Volume", enable);
	childSetEnabled("Voice Volume", enable);
	childSetEnabled("SFX Volume", enable);
	childSetEnabled("UI Volume", enable);
	childSetEnabled("Wind Volume", enable);
	LLPanel::draw();
}


