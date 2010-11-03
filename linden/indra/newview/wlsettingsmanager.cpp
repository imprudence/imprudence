/** 
* @file wlfloaterwindlightsend.cpp
* @brief WLFloaterWindLightSend class definition
*
* $LicenseInfo:firstyear=2007&license=viewergpl$
* 
* Copyright (c) 2007-2009, Linden Research, Inc.
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

#include "llfloaterwindlight.h"

#include "pipeline.h"
#include "llsky.h"

#include "llsliderctrl.h"
#include "llmultislider.h"
#include "llmultisliderctrl.h"
#include "llspinctrl.h"
#include "llcheckboxctrl.h"
#include "lluictrlfactory.h"
#include "llviewercamera.h"
#include "llcombobox.h"
#include "lllineeditor.h"
#include "llfloaterdaycycle.h"
#include "lltabcontainer.h"
#include "llboost.h"

#include "llagent.h"
#include "llinventorymodel.h"
#include "llviewerinventory.h"

#include "v4math.h"
#include "llviewerdisplay.h"
#include "llviewercontrol.h"
#include "llviewerwindow.h"
#include "llsavedsettingsglue.h"

#include "llwlparamset.h"
#include "llwlparammanager.h"
#include "llwaterparammanager.h"
#include "llpostprocess.h"
#include "hippolimits.h"
#include "wlfloaterwindlightsend.h"
#include "llviewerregion.h"
#include "wlsettingsmanager.h"
#include "lightshare.h"

#include "linden_common.h"
#include "llviewercontrol.h"
#include "message.h"
#include "meta7windlight.h"
#include "llworld.h"

#undef max

const std::string WLSettingsManager::wlWaterPresetName = "(Region settings)";
const std::string WLSettingsManager::wlSkyPresetName   = "(Region settings)";

LLTimer* WLSettingsManager::wlIgnoreTimer = new LLTimer();
bool WLSettingsManager::wlIgnoreRegion = false;
LLWaterParamSet* WLSettingsManager::mWater = NULL;
LLWLParamSet* WLSettingsManager::mSky = NULL;
LLUUID* WLSettingsManager::mWaterNormal = NULL;

void WLSettingsManager::Apply( LLWLParamSet* Sky, LLWaterParamSet* Water, LLUUID* WaterNormal )
{
	if( gSavedSettings.getU32("LightShareAllowed") <= WindlightMessage::LIGHTSHARE_NEVER )
		return;

	std::string water = LLWaterParamManager::instance()->mCurParams.mName;
	std::string sky = LLWLParamManager::instance()->mCurParams.mName;

	// If they are using region settings already, or LightShare is
	// always allowed, just apply the new settings, don't bother asking.
	if( gSavedSettings.getU32("LightShareAllowed") == WindlightMessage::LIGHTSHARE_ALWAYS ||
	    (sky == wlSkyPresetName && water == wlWaterPresetName) )
	{
		mSky = Sky;
		mWater = Water;
		mWaterNormal = WaterNormal;
		Apply();
		return;
	}

	if( !wlignoreTimerHasExpired() )
	{
		// The user recently ignored a windlight message, so ignore
		// this one too, and restart the timer.
		wlrestartIgnoreTimer();
		return;
	}

	if(wlIgnoreRegion)
	{
		// We are ignoring new settings until user enters a new region.
		return;
	}

	if( gSavedSettings.getU32("LightShareAllowed") == WindlightMessage::LIGHTSHARE_ASK &&
	    mSky == NULL && mWater == NULL)
	{
		// No most recent, so store this and create notification
		// asking the user whether to apply or not.
		mSky = Sky;
		mWater = Water;
		mWaterNormal = WaterNormal;
		LLNotifications::instance().add("ConfirmLightShare", LLSD(), LLSD(), 
		                                boost::bind(&wlapplyCallback, _1, _2));
	}
	else
	{
		// No new notification (to avoid spamming the user, we do keep the saves from above)
		mSky = Sky;
		mWater = Water;
		mWaterNormal = WaterNormal;
	}
}

// static
bool WLSettingsManager::wlapplyCallback(const LLSD& notification,
                                     const LLSD& response)
{
	S32 option = LLNotification::getSelectedOption(notification, response);

	switch(option)
	{
		case 0:{
			// "Apply"
			Apply();
			break;
		}
		case 1:{
			// "Not Now", ignore until the region stops spamming
			wlrestartIgnoreTimer();
			break;
		}
		case 2:{
			// "Ignore", ignore all until user leaves the region
			wlIgnoreRegion = true;
			break;
		}
	}
	return false;
}

//static
void WLSettingsManager::Apply()
{
	LLWaterParamManager* water_mgr = LLWaterParamManager::instance();
	LLWLParamManager* sky_mgr = LLWLParamManager::instance();

	F32 fade = 0; //Instant
	bool error;
	fade = mSky->getFloat("fade", error);

	mWater->mName = wlWaterPresetName;
	if(fade != 0 && water_mgr->mCurParams.mName == wlWaterPresetName)//Load the settings forcefully the first time
	{
		LLWaterParamSet oldWset = water_mgr->mCurParams;
		//This still needs done so that we update right, but load it to the old
		water_mgr->removeParamSet( wlWaterPresetName, false );
		water_mgr->addParamSet( wlWaterPresetName, oldWset );
		water_mgr->savePreset( wlWaterPresetName );
		water_mgr->loadPreset( wlWaterPresetName, true );
		water_mgr->setNormalMapID( *mWaterNormal );
		//Then mix with the new
		water_mgr->SetMixTime(mWater, fade);
	}
	else
	{
		//Instant if fade is 0
		water_mgr->removeParamSet( wlWaterPresetName, false );
		water_mgr->addParamSet( wlWaterPresetName, *mWater );
		water_mgr->savePreset( wlWaterPresetName );
		water_mgr->loadPreset( wlWaterPresetName, true );
		water_mgr->setNormalMapID( *mWaterNormal );
	}

	mSky->mName = wlSkyPresetName;
	if(fade != 0 && sky_mgr->mCurParams.mName == wlSkyPresetName)//Load the settings forcefully the first time
	{
		LLWLParamSet oldset = sky_mgr->mCurParams;
		//This still needs done so that we update right, but load it to the old
		sky_mgr->removeParamSet( wlSkyPresetName, true );
		sky_mgr->addParamSet( wlSkyPresetName, oldset );
		sky_mgr->savePreset( wlSkyPresetName );
		sky_mgr->loadPreset( wlSkyPresetName, true );
		//Then mix with the new
		sky_mgr->SetMixTime(mSky, fade);
	}
	else
	{
		//Instant if fade is 0
		sky_mgr->mAnimator.mIsRunning = false;
		sky_mgr->mAnimator.mUseLindenTime = false;
		sky_mgr->removeParamSet( wlSkyPresetName, false );
		sky_mgr->addParamSet( wlSkyPresetName, *mSky );
		sky_mgr->savePreset( wlSkyPresetName );
		sky_mgr->loadPreset( wlSkyPresetName, true );
	}

	LLWorld::getInstance()->rebuildClouds(gAgent.getRegion());

	mSky = NULL;
	mWater = NULL;
	mWaterNormal = NULL;
}

// static
void WLSettingsManager::wlresetRegion()
{
	wlIgnoreRegion = false;
	LLWorld::getInstance()->rebuildClouds(gAgent.getRegion());
}

// static
void WLSettingsManager::wlrestartIgnoreTimer()
{
	F32 time = gSavedSettings.getF32("LightShareIgnoreTimer");
	wlIgnoreTimer->start();
	wlIgnoreTimer->setTimerExpirySec( (time < 0) ? 0 : time );
}

// static
bool WLSettingsManager::wlignoreTimerHasExpired()
{
	return wlIgnoreTimer->hasExpired();
}