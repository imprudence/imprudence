/** 
 * @file llpanelaudioprefs.cpp
 * @brief Audio preference implementation
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

#include "llviewerprecompiledheaders.h"

// file include
#include "llpanelaudioprefs.h"

// linden library includes
#include "llerror.h"
#include "llrect.h"
#include "llstring.h"
#include "llfontgl.h"

// project includes
#include "llviewerwindow.h"
#include "llui.h"
#include "llspinctrl.h"
#include "llslider.h"
#include "llradiogroup.h"
#include "llsliderctrl.h"
#include "llcheckboxctrl.h"
#include "lltextbox.h"
#include "llbutton.h"
#include "llcombobox.h"
#include "audioengine.h"
#include "llmediaengine.h"
#include "llfirstuse.h"
#include "llparcel.h"
#include "llviewerparcelmgr.h"
#include "llnotify.h"
#include "llvieweruictrlfactory.h"
#include "llresmgr.h"
#include "viewer.h"		// do_disconnect

//
// Imported globals
//

extern LLAudioEngine *gAudiop;

//
// Globals
//


//
// Static functions
//




LLPanelAudioPrefs::LLPanelAudioPrefs()
{

	gUICtrlFactory->buildPanel(this, "panel_preferences_audio.xml");


}


LLPanelAudioPrefs::~LLPanelAudioPrefs()
{
	// Children all cleaned up by default view destructor.
}




BOOL LLPanelAudioPrefs::postBuild()
{

	
	mPreviousMediaVolume = gSavedSettings.getF32("MediaAudioVolume");
	mPreviousVolume = gSavedSettings.getF32("AudioLevelMaster");
	mPreviousUI = gSavedSettings.getF32("AudioLevelUI");
	mPreviousFootsteps = gSavedSettings.getF32("AudioLevelFootsteps");
	mPreviousWind = gSavedSettings.getF32("AudioLevelWind");
	mPreviousDoppler = gSavedSettings.getF32("AudioLevelDoppler");
	mPreviousDistance = gSavedSettings.getF32("AudioLevelDistance");
	mPreviousRolloff = gSavedSettings.getF32("AudioLevelRolloff");

	mPreviousBitrate = gSavedSettings.getS32("AudioDefaultBitrate");

	mPreviousMoneyThreshold = gSavedSettings.getF32("UISndMoneyChangeThreshold");
	mPreviousHealthThreshold = gSavedSettings.getF32("UISndHealthReductionThreshold");

	mPreviousStreamingMusic = gSavedSettings.getBOOL("AudioStreamingMusic");
	mPreviousStreamingVideo = gSavedSettings.getBOOL("AudioStreamingVideo");

	mPreviousMuteAudio = gSavedSettings.getBOOL("MuteAudio");
	mPreviousMuteWhenMinimized = gSavedSettings.getBOOL("MuteWhenMinimized");

	

	childSetCommitCallback("disable audio", onMuteAudio, this);

	enable(!gSavedSettings.getBOOL("MuteAudio"));

	return TRUE;

}


void LLPanelAudioPrefs::enable(BOOL b)
{
	childSetEnabled("mute_when_minimized", b);

	childSetEnabled("streaming_text", b);
	childSetEnabled("streaming_music", b);
	childSetEnabled("streaming_video", b);

	childSetEnabled("System Volume", b);
	childSetEnabled("system_volume_text", b);

	childSetEnabled("Wind Volume", b);
	childSetEnabled("wind_volume_text", b);

	childSetEnabled("Footsteps Volume", b);
	childSetEnabled("footsteps_volume_text", b);

	childSetEnabled("UI Volume", b);
	childSetEnabled("ui_volume", b);

	childSetEnabled("Doppler Effect", b);
	childSetEnabled("doppler_effect_text", b);

	childSetEnabled("Distance Factor", b);
	childSetEnabled("distance_factor_text", b);

	childSetEnabled("Rolloff Factor", b);
	childSetEnabled("rolloff_factor_text", b);

	childSetEnabled("L$ Change Threshold", b);

	childSetEnabled("Health Change Threshold", b);

	childSetEnabled("bitrate", b);
	childSetEnabled("default_upload_bitrate_text", b);
}

struct LLPARestartData
{
	BOOL mMuteAudio;
};



void LLPanelAudioPrefs::apply()
{
	
}



void LLPanelAudioPrefs::onMuteAudio(LLUICtrl* ctrl, void* userdata)
{
	LLPanelAudioPrefs* self = (LLPanelAudioPrefs*)userdata;
	LLCheckBoxCtrl* check = (LLCheckBoxCtrl*)ctrl;

	self->enable(!check->get());

}


void LLPanelAudioPrefs::cancel()
{
	
	gSavedSettings.setS32("AudioDefaultBitrate", mPreviousBitrate);
	gSavedSettings.setF32("MediaAudioVolume", mPreviousMediaVolume);
	gSavedSettings.setF32("AudioLevelMaster", mPreviousVolume );
	gSavedSettings.setF32("AudioLevelUI", mPreviousUI );
	gSavedSettings.setF32("AudioLevelFootsteps", mPreviousFootsteps );
	gSavedSettings.setF32("AudioLevelWind", mPreviousWind );
	gSavedSettings.setF32("AudioLevelDoppler", mPreviousDoppler );
	gSavedSettings.setF32("AudioLevelDistance", mPreviousDistance );
	gSavedSettings.setF32("AudioLevelRolloff", mPreviousRolloff );

	gSavedSettings.setF32("UISndMoneyChangeThreshold", mPreviousMoneyThreshold );
	gSavedSettings.setF32("UISndHealthReductionThreshold", mPreviousHealthThreshold );

	gSavedSettings.setBOOL("AudioStreamingMusic", mPreviousStreamingMusic );
	gSavedSettings.setBOOL("AudioStreamingVideo", mPreviousStreamingVideo );

	
	gSavedSettings.setBOOL("MuteAudio", mPreviousMuteAudio );
	gSavedSettings.setBOOL("MuteWhenMinimized", mPreviousMuteWhenMinimized );



}
