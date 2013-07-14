/**
 * @file llettherebelight.cpp
 * @brief Handler for Meta7 Lightshare (region-side Windlight settings), and other methods of sharing WindLight.
 *
 * Copyright (c) 2010, Tom Grimshaw (Tom Meta)
 * Copyright (c) 2010, Jacek Antonelli
 * Copyright (c) 2012, David Seikel
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



#include "linden_common.h"
#include "llviewercontrol.h"
#include "llwaterparammanager.h"
#include "llwaterparamset.h"
#include "llwlparammanager.h"
#include "llwlparamset.h"
#include "message.h"
#include "meta7windlight.h"

#include "llettherebelight.h"

#include "llagent.h"
#include "llworld.h"


const std::string LightShare::sRegionPresetName = "(Region settings)";
const std::string LightShare::sParcelPresetName = "(Parcel settings)";
const std::string LightShare::sRLVPresetName = "(RLV settings)";


LLWaterParamSet* LightShare::mWater = NULL;
LLWLParamSet* LightShare::mSky = NULL;
LLUUID* LightShare::mWaterNormal = NULL;
LLTimer* LightShare::sIgnoreTimer = new LLTimer();
bool LightShare::sIgnoreRegion = false;


LightShare::LightShare( LLMessageSystem* msg ) :
	mPacket(NULL),
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
 	mSky = new LLWLParamSet();

	mWaterNormal = new LLUUID();

	process_packet(&buf[0]);
	process_water();
	process_sky();

	// *TODO: Actually validate the settings.
	mIsValid = true;
}


LightShare::~LightShare()
{
	delete mWater;
	delete mSky;
	delete mWaterNormal;
}


// static
void LightShare::processWindlight(LLMessageSystem* msg, void**)
{
	if( gSavedSettings.getU32("LightShareAllowed") <= LIGHTSHARE_NEVER )
		return;

	LightShare* wl = new LightShare(msg);

	if (!wl)
		return;

	if(!wl->isValid() )
	{
		delete wl;
		return;
	}

	applyMaybe(wl->mWater, wl->mWaterNormal, wl->mSky);
}


//static
void LightShare::applyMaybe(LLWaterParamSet* thisWater, LLUUID* thisWaterNormal, LLWLParamSet* thisSky)
{
	if( gSavedSettings.getU32("LightShareAllowed") <= LIGHTSHARE_NEVER )
		return;

	std::string water = LLWaterParamManager::instance()->mCurParams.mName;
	std::string sky = LLWLParamManager::instance()->mCurParams.mName;

	// If they are using region settings already, or LightShare is
	// always allowed, just apply the new settings, don't bother asking.
	if( gSavedSettings.getU32("LightShareAllowed") == LIGHTSHARE_ALWAYS ||
	    (sky == sRegionPresetName && water == sRegionPresetName) )
	{
		mSky = thisSky;
		mWater = thisWater;
		mWaterNormal = thisWaterNormal;
		apply(mWater, mWaterNormal, mSky, WL_SCOPE_REGION);
		return;
	}

	if( !ignoreTimerHasExpired() )
	{
		// The user recently ignored a windlight message, so ignore
		// this one too, and restart the timer.
		restartIgnoreTimer();
		return;
	}

	if(sIgnoreRegion)
	{
		// We are ignoring new settings until user enters a new region.
		return;
	}

	if( gSavedSettings.getU32("LightShareAllowed") == LIGHTSHARE_ASK && mSky == NULL && mWater == NULL)
	{
		// No most recent, so store this and create notification
		// asking the user whether to apply or not.
		mSky = thisSky;
		mWater = thisWater;
		mWaterNormal = thisWaterNormal;
		LLNotifications::instance().add("ConfirmLightShare", LLSD(), LLSD(),
		                                boost::bind(&applyCallback, _1, _2));
	}
	else
	{
		// No new notification (to avoid spamming the user, we do keep the saves from above)
		mSky = thisSky;
		mWater = thisWater;
		mWaterNormal = thisWaterNormal;
	}
}

// static
bool LightShare::applyCallback(const LLSD& notification, const LLSD& response)
{
	S32 option = LLNotification::getSelectedOption(notification, response);

	switch(option)
	{
		case 0:{
			// "Apply"
			apply(mWater, mWaterNormal, mSky, WL_SCOPE_REGION);

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

	return false;
}

// static
void LightShare::resetRegion()
{
	sIgnoreRegion = false;
	apply(NULL, NULL, NULL, WL_SCOPE_REGION);
}

// static
void LightShare::restartIgnoreTimer()
{
	F32 time = gSavedSettings.getF32("LightShareIgnoreTimer");
	sIgnoreTimer->start();
	sIgnoreTimer->setTimerExpirySec( (time < 0) ? 0 : time );
}

// static
bool LightShare::ignoreTimerHasExpired()
{
	return sIgnoreTimer->hasExpired();
}

// TODO - have regionSet and parcelSet be arrays, so we can deal with height zones.
static struct WLCombined userSet, regionSet, parcelSet, RLVSet;

// TODO - should spread this merging stuff around,
//        so that eventually we can get rid of almost identical code for water and sky.
//        Then one of these two methods goes away.

//static
void LightShare::mergeWaterSets(LLWaterParamSet* thisSet, LLWaterParamSet* oldSet)
{
	for(LLSD::map_const_iterator i = thisSet->mParamValues.beginMap();
		i != thisSet->mParamValues.endMap();
		++i)
	{
		const std::string& param = i->first;

		if(i->second.isArray())
		{
			for (int j = 0; j < i->second.size(); j++)
			{
				oldSet->mParamValues[param][j] = i->second[j].asReal();
			}
		}
		else if(i->second.isReal())
		  oldSet->mParamValues[param] = i->second.asReal();
	}
}

//static
void LightShare::mergeWLSets(LLWLParamSet* thisSet, LLWLParamSet* oldSet)
{
	for(LLSD::map_const_iterator i = thisSet->mParamValues.beginMap();
		i != thisSet->mParamValues.endMap();
		++i)
	{
		const std::string& param = i->first;

		if(i->second.isArray())
		{
			for (int j = 0; j < i->second.size(); j++)
			{
				oldSet->mParamValues[param][j] = i->second[j].asReal();
			}
		}
		else if(i->second.isReal())
		  oldSet->mParamValues[param] = i->second.asReal();
	}
}

//static
void LightShare::apply(LLWaterParamSet * newWater, LLUUID *newWaterNormal, LLWLParamSet *newSky, WLScope scope)
// TODO - Deal with day cycle stuff.
{
	LLWaterParamManager* waterMgr = LLWaterParamManager::instance();
	LLWLParamManager* skyMgr = LLWLParamManager::instance();
	LLWaterParamSet oldWaterSet = waterMgr->mCurParams;
	LLWLParamSet oldWLSet = skyMgr->mCurParams;
	struct WLCombined* thisSet = &userSet;
	bool user = true;

	switch(scope)
	{
		case WL_SCOPE_USER :
		{
			thisSet = &userSet;
			thisSet->water.mName = waterMgr->mCurParams.mName;
			thisSet->sky.mName = skyMgr->mCurParams.mName;
			thisSet->enabled = true;
			// Check if user selected to show the saved region or parcel settings.
			if (newSky && (sRegionPresetName == skyMgr->mCurParams.mName))
			  thisSet->enabled = false;
			if (newWater && (sParcelPresetName == skyMgr->mCurParams.mName))
			  thisSet->enabled = false;
			break;
		}
		case WL_SCOPE_REGION :
		{
			thisSet = &regionSet;
			thisSet->water.mName = sRegionPresetName;
			thisSet->sky.mName = sRegionPresetName;
			thisSet->enabled = (gSavedSettings.getU32("LightShareAllowed") != LIGHTSHARE_NEVER);
			break;
		}
		case WL_SCOPE_PARCEL :
		{
			thisSet = &parcelSet;
			thisSet->water.mName = sParcelPresetName;
			thisSet->sky.mName = sParcelPresetName;
			thisSet->enabled = (gSavedSettings.getU32("LightShareAllowed") != LIGHTSHARE_NEVER);
			break;
		}
		case WL_SCOPE_RLV :
		{
			thisSet = &RLVSet;
			thisSet->water.mName = sRLVPresetName;
			thisSet->sky.mName = sRLVPresetName;
			// TODO set enabled properly.
			break;
		}
	}

	if (newWater)
		thisSet->water.setAll(newWater->getAll());
	if (newWaterNormal)
		thisSet->water.mParamValues["normalMap"] = *newWaterNormal;
	if (newSky)
		thisSet->sky.setAll(newSky->getAll());

	if ((NULL == newWater) && (NULL == newSky))
		thisSet->enabled = false;

	F32 fade = 0; //Instant
	bool error;
	fade = thisSet->sky.getFloat("fade", error);

	if (fade)
	{
		// TODO - should copy the original, then set that here.
		// The fade should delete this copy once it's done fading.
		// Dunno if we actually need to do any of this anyway.
		waterMgr->removeParamSet( oldWaterSet.mName, false );
		waterMgr->addParamSet( oldWaterSet.mName, oldWaterSet );
		waterMgr->setNormalMapID( *newWaterNormal );
		waterMgr->getParamSet(oldWaterSet.mName, waterMgr->mCurParams);
		waterMgr->propagateParameters();

		skyMgr->removeParamSet( oldWLSet.mName, false );
		skyMgr->addParamSet( oldWLSet.mName, oldWLSet );
		skyMgr->getParamSet(oldWLSet.mName, skyMgr->mCurParams);
		skyMgr->propagateParameters();
	}

	if (regionSet.enabled)
	{
		waterMgr->setParamSet( regionSet.water.mName, regionSet.water );
		skyMgr->setParamSet( regionSet.sky.mName, regionSet.sky );
		mergeWaterSets(&(regionSet.water), &oldWaterSet);
		mergeWLSets(&(regionSet.sky), &oldWLSet);
	}
	else
	{
		waterMgr->removeParamSet( regionSet.water.mName, false );
		skyMgr->removeParamSet( regionSet.sky.mName, false );
	}
	if (parcelSet.enabled)
	{
		waterMgr->setParamSet( parcelSet.water.mName, parcelSet.water );
		skyMgr->setParamSet( parcelSet.sky.mName, parcelSet.sky );
		mergeWaterSets(&(parcelSet.water), &oldWaterSet);
		mergeWLSets(&(parcelSet.sky), &oldWLSet);
	}
	else
	{
		waterMgr->removeParamSet( parcelSet.water.mName, false );
		skyMgr->removeParamSet( parcelSet.sky.mName, false );
	}
	if (userSet.enabled)
	{
		mergeWaterSets(&(userSet.water), &oldWaterSet);
		mergeWLSets(&(userSet.sky), &oldWLSet);
	}
	if (RLVSet.enabled)
	{
		mergeWaterSets(&(RLVSet.water), &oldWaterSet);
		mergeWLSets(&(RLVSet.sky), &oldWLSet);
	}

	skyMgr->mAnimator.mIsRunning = false;
	skyMgr->mAnimator.mUseLindenTime = false;
	if (fade)
	{
		waterMgr->SetMixTime(&oldWaterSet, fade);
		skyMgr->SetMixTime(&oldWLSet, fade);
	}
	else
	{
		if (newWater)
		{
			waterMgr->setParamSet( thisSet->water.mName, oldWaterSet );
			waterMgr->setNormalMapID( *newWaterNormal );
			waterMgr->getParamSet(thisSet->water.mName, waterMgr->mCurParams);
			waterMgr->propagateParameters();
		}

		if (newSky)
		{
			  skyMgr->setParamSet( thisSet->sky.mName, oldWLSet );
			  skyMgr->getParamSet(thisSet->sky.mName, skyMgr->mCurParams);
			  skyMgr->propagateParameters();
		}
	}

	LLWorld::getInstance()->rebuildClouds(gAgent.getRegion());
}

bool LightShare::isValid()
{
	return mIsValid;
}

void LightShare::process_packet( char* buf )
{
	// *FIXME: Horrible idea, fragile, not byte-order or endian
	//         safe, no validation, etc. etc. -Jacek
	mPacket = (Meta7WindlightPacket*)buf;
}

void LightShare::process_water()
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


void LightShare::process_sky()
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
