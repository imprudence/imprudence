/**
 * @file lightshare.cpp
 * @brief Handler for Meta7 Lightshare (region-side Windlight settings).
 *
 * Copyright (c) 2010, Tom Grimshaw (Tom Meta)
 * Copyright (c) 2010, Jacek Antonelli
 *
 * The source code in this file ("Source Code") is provided to you
 * under the terms of the GNU General Public License, version 2.0
 * ("GPL"). Terms of the GPL can be found in doc/GPL-license.txt in
 * this distribution, or online at
 * http://secondlifegrid.net/programs/open_source/licensing/gplv2
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
 * ALL SOURCE CODE IS PROVIDED "AS IS." THE AUTHOR MAKES NO
 * WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
 * COMPLETENESS OR PERFORMANCE.
 */


#include "lightshare.h"

#include "linden_common.h"
#include "llviewercontrol.h"
#include "llwaterparammanager.h"
#include "llwaterparamset.h"
#include "llwlparammanager.h"
#include "llwlparamset.h"
#include "message.h"
#include "meta7windlight.h"


const std::string WindlightMessage::sWaterPresetName = "(Region settings)";
const std::string WindlightMessage::sSkyPresetName   = "(Region settings)";


WindlightMessage* WindlightMessage::sMostRecent = NULL;
LLTimer* WindlightMessage::sIgnoreTimer = new LLTimer();
bool WindlightMessage::sIgnoreRegion = false;


WindlightMessage::WindlightMessage( LLMessageSystem* msg ) :
	mPacket(NULL),
	mWater(NULL),
	mSky(NULL),
	mWaterNormal(NULL),
	mIsValid(false)
{
	std::string method;
	msg->getStringFast(_PREHASH_MethodData, _PREHASH_Method, method);

	if( method != "Windlight" )
	{
		return; // Wrong message type, somehow.
	}

	S32 size = msg->getSizeFast(_PREHASH_ParamList, 0, _PREHASH_Parameter);

	if( size < 0 || 250 < size )
	{
		return; // Too small or too big.
	}

	// Unpack and process the message's binary payload.
	char buf[250];
	msg->getBinaryDataFast(_PREHASH_ParamList,
				 _PREHASH_Parameter,
				 buf, size, 0, 249);

	mWater = new LLWaterParamSet();
// 	mSky = new LLWLParamSet();
	LLWLParamManager * wl_param_mgr = LLWLParamManager::instance();
	static LLWLParamSet & sSky = wl_param_mgr->mCurParams;
	mSky = &sSky;

	mWaterNormal = new LLUUID();

	process_packet(&buf[0]);
	process_water();
	process_sky();

	// *TODO: Actually validate the settings.
	mIsValid = true;
}


WindlightMessage::~WindlightMessage()
{
	delete mWater;
//	delete mSky;
	delete mWaterNormal;
}


// static
void WindlightMessage::processWindlight(LLMessageSystem* msg, void**)
{
	if( gSavedSettings.getU32("LightShareAllowed") <= LIGHTSHARE_NEVER )
		return;

	WindlightMessage* wl = new WindlightMessage(msg);

	if (!wl)
		return;

	if(!wl->isValid() )
	{
		delete wl;
		return;
	}

	std::string water = LLWaterParamManager::instance()->mCurParams.mName;
	std::string sky = LLWLParamManager::instance()->mCurParams.mName;

	// If they are using region settings already, or LightShare is
	// always allowed, just apply the new settings, don't bother asking.
	if( gSavedSettings.getU32("LightShareAllowed") == LIGHTSHARE_ALWAYS ||
	    (sky == sSkyPresetName && water == sWaterPresetName) )
	{
		wl->apply();
		delete wl;
		return;
	}

	if( !ignoreTimerHasExpired() )
	{
		// The user recently ignored a windlight message, so ignore
		// this one too, and restart the timer.
		restartIgnoreTimer();
		delete wl;
		return;
	}

	if(sIgnoreRegion)
	{
		// We are ignoring new settings until user enters a new region.
		delete wl;
		return;
	}

	if( gSavedSettings.getU32("LightShareAllowed") == LIGHTSHARE_ASK &&
	    sMostRecent == NULL )
	{
		// No most recent, so store this and create notification
		// asking the user whether to apply or not.
		sMostRecent = wl;
		LLNotifications::instance().add("ConfirmLightShare", LLSD(), LLSD(), 
		                                boost::bind(&applyCallback, _1, _2));
		return;
	}
	else
	{
		// No new notification (to avoid spamming the user), just
		// store this as most recent.
		delete sMostRecent;
		sMostRecent = wl;
		return;
	}
}


// static
bool WindlightMessage::applyCallback(const LLSD& notification,
                                     const LLSD& response)
{
	S32 option = LLNotification::getSelectedOption(notification, response);

	switch(option)
	{
		case 0:{
			// "Apply"
			sMostRecent->apply();
			break;
		}
		case 1:{
			// "Not Now", ignore until the region stops spamming
			restartIgnoreTimer();
			break;
		}
		case 2:{
			// "Ignore", ignore all until user leaves the region
			sIgnoreRegion = true;
			break;
		}
	}

	delete sMostRecent;
	sMostRecent = NULL;

	return false;
}


// static
void WindlightMessage::resetRegion()
{
	sIgnoreRegion = false;
}


// static
void WindlightMessage::restartIgnoreTimer()
{
	F32 time = gSavedSettings.getF32("LightShareIgnoreTimer");
	sIgnoreTimer->start();
	sIgnoreTimer->setTimerExpirySec( (time < 0) ? 0 : time );
}

// static
bool WindlightMessage::ignoreTimerHasExpired()
{
	return sIgnoreTimer->hasExpired();
}


bool WindlightMessage::apply()
{
	LLWaterParamManager* water_mgr = LLWaterParamManager::instance();
	LLWLParamManager* sky_mgr = LLWLParamManager::instance();

	mWater->mName = sWaterPresetName;
	water_mgr->removeParamSet( sWaterPresetName, false );
	water_mgr->addParamSet( sWaterPresetName, *mWater );
	water_mgr->savePreset( sWaterPresetName );
	water_mgr->loadPreset( sWaterPresetName, true );
	water_mgr->setNormalMapID( *mWaterNormal );

	mSky->mName = sSkyPresetName;
	sky_mgr->mAnimator.mIsRunning = false;
	sky_mgr->mAnimator.mUseLindenTime = false;
	sky_mgr->removeParamSet( sSkyPresetName, false );
	sky_mgr->addParamSet( sSkyPresetName, *mSky );
	sky_mgr->savePreset( sSkyPresetName );
	sky_mgr->loadPreset( sSkyPresetName, true );

	return true;
}


bool WindlightMessage::isValid()
{
	return mIsValid;
}


void WindlightMessage::process_packet( char* buf )
{
	// *FIXME: Horrible idea, fragile, not byte-order or endian
	//         safe, no validation, etc. etc. -Jacek
	mPacket = (Meta7WindlightPacket*)buf;
}


void WindlightMessage::process_water()
{
	mWater->set("waterFogColor",
							mPacket->waterColor.red   / 256.f,
							mPacket->waterColor.green / 256.f,
							mPacket->waterColor.blue  / 256.f);

	mWater->set("waterFogDensity",
							pow(2.0f, mPacket->waterFogDensityExponent));

	mWater->set("underWaterFogMod", mPacket->underwaterFogModifier);

	mWater->set("normScale",
							mPacket->reflectionWaveletScale.X,
							mPacket->reflectionWaveletScale.Y,
							mPacket->reflectionWaveletScale.Z);

	mWater->set("fresnelScale",   mPacket->fresnelScale);
	mWater->set("fresnelOffset",  mPacket->fresnelOffset);
	mWater->set("scaleAbove",     mPacket->refractScaleAbove);
	mWater->set("scaleBelow",     mPacket->refractScaleBelow);
	mWater->set("blurMultiplier", mPacket->blurMultiplier);

	mWater->set("wave1Dir",
							mPacket->littleWaveDirection.X,
							mPacket->littleWaveDirection.Y);

	mWater->set("wave2Dir",
							mPacket->bigWaveDirection.X,
							mPacket->bigWaveDirection.Y);


	// Format a UUID string from a block of raw bytes. Ugh.
	std::string uuid = llformat(
		"%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
		(U8)(mPacket->normalMapTexture[0]),
		(U8)(mPacket->normalMapTexture[1]),
		(U8)(mPacket->normalMapTexture[2]),
		(U8)(mPacket->normalMapTexture[3]),
		(U8)(mPacket->normalMapTexture[4]),
		(U8)(mPacket->normalMapTexture[5]),
		(U8)(mPacket->normalMapTexture[6]),
		(U8)(mPacket->normalMapTexture[7]),
		(U8)(mPacket->normalMapTexture[8]),
		(U8)(mPacket->normalMapTexture[9]),
		(U8)(mPacket->normalMapTexture[10]),
		(U8)(mPacket->normalMapTexture[11]),
		(U8)(mPacket->normalMapTexture[12]),
		(U8)(mPacket->normalMapTexture[13]),
		(U8)(mPacket->normalMapTexture[14]),
		(U8)(mPacket->normalMapTexture[15]));

	mWaterNormal->set(uuid);
}


void WindlightMessage::process_sky()
{
	mSky->setSunAngle(F_TWO_PI * mPacket->sunMoonPosiiton);
	mSky->setEastAngle(F_TWO_PI * mPacket->eastAngle);

	mSky->set("sunlight_color",
						mPacket->sunMoonColor.red * 3.0f,
						mPacket->sunMoonColor.green * 3.0f,
						mPacket->sunMoonColor.blue * 3.0f,
						mPacket->sunMoonColor.alpha * 3.0f);

	mSky->set("ambient",
						mPacket->ambient.red * 3.0f,
						mPacket->ambient.green * 3.0f,
						mPacket->ambient.blue * 3.0f,
						mPacket->ambient.alpha * 3.0f);

	mSky->set("blue_horizon",
						mPacket->horizon.red * 2.0f,
						mPacket->horizon.green *2.0f,
						mPacket->horizon.blue * 2.0f,
						mPacket->horizon.alpha * 2.0f);

	mSky->set("blue_density",
						mPacket->blueDensity.red * 2.0f,
						mPacket->blueDensity.green * 2.0f,
						mPacket->blueDensity.blue * 2.0f,
						mPacket->blueDensity.alpha * 2.0f);

	mSky->set("haze_horizon",
						mPacket->hazeHorizon,
						mPacket->hazeHorizon,
						mPacket->hazeHorizon,
						1.f);

	mSky->set("haze_density",
						mPacket->hazeDensity,
						0.f, 0.f, 1.f);

	mSky->set("cloud_shadow",
						mPacket->cloudCoverage,
						0.f, 0.f, 1.f);

	mSky->set("density_multiplier",
	          mPacket->densityMultiplier / 1000.0f,
	          0.f, 0.f, 1.f);

	mSky->set("distance_multiplier",
						mPacket->distanceMultiplier,
						0.f, 0.f, 1.f);

	mSky->set("max_y",
	          (F32)mPacket->maxAltitude,
	          0.f, 0.f, 1.f);

	mSky->set("cloud_color",
						mPacket->cloudColor.red,
						mPacket->cloudColor.green,
						mPacket->cloudColor.blue,
						mPacket->cloudColor.alpha);

	mSky->set("cloud_pos_density1",
						mPacket->cloudXYDensity.X,
						mPacket->cloudXYDensity.Y,
	          mPacket->cloudXYDensity.Z,
	          1.f);

	mSky->set("cloud_pos_density2",
						mPacket->cloudDetailXYDensity.X,
						mPacket->cloudDetailXYDensity.Y,
						mPacket->cloudDetailXYDensity.Z,
	          1.f);

	mSky->set("cloud_scale",
	          mPacket->cloudScale,
	          0.f, 0.f, 1.f);

	mSky->set("gamma",
						mPacket->sceneGamma,
						0.f, 0.f, 1.f);

	mSky->set("glow",
						(2 - mPacket->sunGlowSize) * 20,
						0.f,
	          -mPacket->sunGlowFocus * 5,
	          1.f);

	mSky->setCloudScrollX(mPacket->cloudScrollX + 10.0f);
	mSky->setCloudScrollY(mPacket->cloudScrollY + 10.0f);

	mSky->setEnableCloudScrollX(!mPacket->cloudScrollXLock);
	mSky->setEnableCloudScrollY(!mPacket->cloudScrollYLock);

	mSky->setStarBrightness(mPacket->starBrightness);
}


