/** 
 * @file llmediaremotectrl.cpp
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

#include "llmediaremotectrl.h"

#include "lloverlaybar.h"
#include "llvieweruictrlfactory.h"
#include "llpanelaudiovolume.h"
#include "llviewercontrol.h"
#include "llbutton.h"

////////////////////////////////////////////////////////////////////////////////
//
//
LLMediaRemoteCtrl::LLMediaRemoteCtrl()
{
	setIsChrome(TRUE);
	mIsFocusRoot = TRUE;

	mFactoryMap["Volume Panel"]	= LLCallbackMap(createVolumePanel, NULL);
	build();
}

void LLMediaRemoteCtrl::build()
{
	//HACK: only works because we don't have any implicit children (i.e. titlebars, close button, etc)
	deleteAllChildren();
	if (gSavedSettings.getBOOL("ShowVolumeSettingsPopup"))
	{
		gUICtrlFactory->buildPanel(this, "panel_media_remote_expanded.xml", &getFactoryMap());
	}
	else
	{
		gUICtrlFactory->buildPanel(this, "panel_media_remote.xml", &getFactoryMap());
	}
}

BOOL LLMediaRemoteCtrl::postBuild()
{
	childSetAction("media_play",LLOverlayBar::mediaPlay,this);
	childSetAction("music_play",LLOverlayBar::musicPlay,this);

	childSetAction("expand", onClickExpandBtn, this);	
	return TRUE;
}

void LLMediaRemoteCtrl::draw()
{
	LLButton* music_play_btn = LLUICtrlFactory::getButtonByName(this, "music_play");
	if (music_play_btn)
	{
		if (gOverlayBar->musicPlaying())
		{
			music_play_btn->setValue(TRUE);
			music_play_btn->setImageOverlay("icn_music-pause.tga");
		}
		else
		{
			music_play_btn->setValue(FALSE);
			music_play_btn->setImageOverlay("icn_music-play.tga");
		}
	}

	LLButton* media_play_btn = LLUICtrlFactory::getButtonByName(this, "media_play");
	if (media_play_btn)
	{
		if (gOverlayBar->mediaPlaying())
		{
			media_play_btn->setValue(TRUE);
			media_play_btn->setImageOverlay("icn_media-pause.tga");
		}
		else
		{
			media_play_btn->setValue(FALSE);
			media_play_btn->setImageOverlay("icn_media-play.tga");
		}
	}

	LLButton* expand_button = LLUICtrlFactory::getButtonByName(this, "expand");
	if (expand_button)
	{
		if (expand_button->getToggleState())
		{
			expand_button->setImageOverlay("arrow_down.tga");
		}
		else
		{
			expand_button->setImageOverlay("arrow_up.tga");
		}
	}

	LLPanel::draw();
}

LLMediaRemoteCtrl::~LLMediaRemoteCtrl ()
{
}

//static 
void LLMediaRemoteCtrl::onClickExpandBtn(void* user_data)
{
	LLMediaRemoteCtrl* remotep = (LLMediaRemoteCtrl*)user_data;

	remotep->build();
	gOverlayBar->layoutButtons();

}

//static
void* LLMediaRemoteCtrl::createVolumePanel(void* data)
{
	LLPanelAudioVolume* panel = new LLPanelAudioVolume();
	return panel;
}
