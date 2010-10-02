/** 
 * @file llpanelaudiovolume.cpp
 * @brief A remote control for media (video and music)
 *
 * $LicenseInfo:firstyear=2005&license=viewergpl$
 * 
 * Copyright (c) 2005-2009, Linden Research, Inc.
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

#include "llpanelaudiovolume.h"

#include "lloverlaybar.h"
#include "llviewercontrol.h"
#include "lluictrlfactory.h"

////////////////////////////////////////////////////////////////////////////////
//
//
LLPanelAudioVolume::LLPanelAudioVolume()
{
}

BOOL LLPanelAudioVolume::postBuild()
{
	childSetCommitCallback("System Volume", onCommitVolumeChange);
	childSetCommitCallback("Music Volume", onCommitVolumeChange);
	childSetCommitCallback("Media Volume", onCommitVolumeChange);
	childSetCommitCallback("Voice Volume", onCommitVolumeChange);
	childSetCommitCallback("SFX Volume", onCommitVolumeChange);
	childSetCommitCallback("UI Volume", onCommitVolumeChange);
	childSetCommitCallback("Wind Volume", onCommitVolumeChange);
	childSetCommitCallback("Gestures Volume", onCommitVolumeChange);
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

	static BOOL* sMuteAudio = rebind_llcontrol<BOOL>("MuteAudio", &gSavedSettings, true);
	bool enable = (*sMuteAudio) ? false : true;
	childSetEnabled("Music Volume", enable);
	childSetEnabled("Media Volume", enable);
	childSetEnabled("Voice Volume", enable);
	childSetEnabled("SFX Volume", enable);
	childSetEnabled("UI Volume", enable);
	childSetEnabled("Wind Volume", enable);
	childSetEnabled("Gestures Volume", enable);

	childSetEnabled("mute_music", enable);
	childSetEnabled("mute_media", enable);
	childSetEnabled("mute_voice", enable);
	childSetEnabled("mute_sfx", enable);
	childSetEnabled("mute_wind", enable);
	childSetEnabled("mute_ui", enable);
	childSetEnabled("mute_gestures", enable);

	LLPanel::draw();
}

//static
void LLPanelAudioVolume::onCommitVolumeChange(LLUICtrl* ctrl, void* user_data)
{
	// unmute various audio sources when user changes volume
	std::string control_name = ctrl->getControlName();
	if (control_name == "AudioLevelMaster")
	{
		gSavedSettings.setBOOL("MuteAudio", FALSE);
	}
	else if (control_name == "AudioLevelSFX")
	{
		gSavedSettings.setBOOL("MuteSounds", FALSE);
	}
	else if (control_name == "AudioLevelUI")
	{
		gSavedSettings.setBOOL("MuteUI", FALSE);
	}
	else if (control_name == "AudioLevelAmbient")
	{
		gSavedSettings.setBOOL("MuteAmbient", FALSE);
	}
	else if (control_name == "AudioLevelMusic")
	{
		gSavedSettings.setBOOL("MuteMusic", FALSE);
	}
	else if (control_name == "AudioLevelMedia")
	{
		gSavedSettings.setBOOL("MuteMedia", FALSE);
	}
	else if (control_name == "AudioLevelVoice")
	{
		gSavedSettings.setBOOL("MuteVoice", FALSE);
	}
	else if (control_name == "AudioLevelGestures")
	{
		gSavedSettings.setBOOL("MuteGestures", FALSE);
	}
}


