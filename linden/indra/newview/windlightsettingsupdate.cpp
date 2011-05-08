/*
 * @file kowopenregionsettings.cpp
 * @brief Handler for OpenRegionInfo event queue message.
 *
 * Copyright (c) 2010, Patrick Sapinski
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

#include "llviewerprecompiledheaders.h"
#include "llhttpnode.h"
#include "hippolimits.h"
#include "llfloatertools.h"
#include "llviewercontrol.h"
#include "llagent.h"
#include "llsurface.h"
#include "llviewerregion.h"
#include "llviewerobject.h"

#include "linden_common.h"
#include "llwaterparammanager.h"
#include "llwaterparamset.h"
#include "llwlparammanager.h"
#include "llwlparamset.h"
#include "message.h"
#include "meta7windlight.h"
#include "lightshare.h"
#include "wlsettingsmanager.h"

//DEBUG includes
//#include "llsdserialize.h" //LLSDNotationStreamer - for dumping LLSD to string

class WindLightSettingsUpdate : public LLHTTPNode
{
	/*virtual*/ void post(
		LLHTTPNode::ResponsePtr response,
		const LLSD& context,
		const LLSD& input) const
	{
		if (!input.isMap() || !input.has("body"))
		{
			llinfos << "malformed WindLightSettingsUpdate update!" << llendl;	 
			return;
		}
		LLWaterParamSet* mWater;
		LLWLParamSet* mSky;
		LLUUID* mWaterNormal;

		mWater = new LLWaterParamSet();
		mSky = new LLWLParamSet();
		mWaterNormal = new LLUUID();

		LLSD body = input["body"];
		
		mWater->set("waterFogColor",
			body["waterColorX"].asReal()   / 256.f,
			body["waterColorY"].asReal()   / 256.f,
			body["waterColorZ"].asReal()   / 256.f,
			body["waterColorW"].asReal()   / 256.f);
		mWater->set("waterFogDensity", body["waterFogDensityExponent"].asReal());
		mWater->set("underWaterFogMod", body["underwaterFogModifier"].asReal());
		mWater->set("normScale", body["reflectionWaveletScaleX"].asReal(),
			body["reflectionWaveletScaleY"].asReal(),
			body["reflectionWaveletScaleZ"].asReal());
		mWater->set("fresnelScale", body["fresnelScale"].asReal());
		mWater->set("fresnelOffset", body["fresnelOffset"].asReal());
		mWater->set("scaleAbove", body["refractScaleAbove"].asReal());
		mWater->set("scaleBelow", body["refractScaleBelow"].asReal());
		mWater->set("blurMultiplier", body["blurMultiplier"].asReal());
		mWater->set("wave2Dir", body["bigWaveDirectionX"].asReal(),
			body["bigWaveDirectionY"].asReal());
		mWater->set("wave1Dir", body["littleWaveDirectionX"].asReal(),
			body["littleWaveDirectionY"].asReal());
		mWaterNormal->parseUUID(body["normalMapTexture"].asUUID().asString(), mWaterNormal);

		mSky->setSunAngle(body["sunMoonPosition"].asReal());
		mSky->setEastAngle(body["eastAngle"].asReal());

		mSky->set("sunlight_color",
						body["sunMoonColorX"].asReal() * 3.0f,
						body["sunMoonColorY"].asReal() * 3.0f,
						body["sunMoonColorZ"].asReal() * 3.0f,
						body["sunMoonColorW"].asReal() * 3.0f);

		mSky->set("ambient",
						body["ambientX"].asReal() * 3.0f,
						body["ambientY"].asReal() * 3.0f,
						body["ambientZ"].asReal() * 3.0f,
						body["ambientW"].asReal() * 3.0f);

		mSky->set("blue_horizon",
						body["horizonX"].asReal() * 2.0f,
						body["horizonY"].asReal() * 2.0f,
						body["horizonZ"].asReal() * 2.0f,
						body["horizonW"].asReal() * 2.0f);

		mSky->set("blue_density",
						body["blueDensityX"].asReal(),
						body["blueDensityY"].asReal(),
						body["blueDensityZ"].asReal(),
						1.0);

		mSky->set("haze_horizon",
						body["hazeHorizon"].asReal(),
						body["hazeHorizon"].asReal(),
						body["hazeHorizon"].asReal(),
						1.f);

		mSky->set("haze_density",
						body["hazeDensity"].asReal(),
						0.f, 0.f, 1.f);

		mSky->set("cloud_shadow",
						body["cloudCoverage"].asReal(),
						0.f, 0.f, 1.f);

		mSky->set("density_multiplier",
						body["densityMultiplier"].asReal() / 1000.0f,
						0.f, 0.f, 1.f);

		mSky->set("distance_multiplier",
						body["distanceMultiplier"].asReal(),
						0.f, 0.f, 1.f);

		mSky->set("max_y",
						body["maxAltitude"].asReal(),
						0.f, 0.f, 1.f);

		mSky->set("cloud_color",
						body["cloudColorX"].asReal(),
						body["cloudColorY"].asReal(),
						body["cloudColorZ"].asReal(),
						body["cloudColorW"].asReal());

		mSky->set("cloud_pos_density1",
						body["cloudXYDensityX"].asReal(),
						body["cloudXYDensityY"].asReal(),
						body["cloudXYDensityZ"].asReal(),
						1.f);

		mSky->set("cloud_pos_density2",
						body["cloudDetailXYDensityX"].asReal(),
						body["cloudDetailXYDensityY"].asReal(),
						body["cloudDetailXYDensityZ"].asReal(),
						1.f);

		mSky->set("cloud_scale",
						body["cloudScale"].asReal(),
						0.f, 0.f, 1.f);

		mSky->set("gamma",
						body["sceneGamma"].asReal(),
						0.f, 0.f, 1.f);

		mSky->set("glow",
						(2 - body["sunGlowSize"].asReal()) * 20,
						0.f,
						-body["sunGlowFocus"].asReal() * 5,
						1.f);

	mSky->setCloudScrollX(body["cloudScrollX"].asReal() + 10.0f);
	mSky->setCloudScrollY(body["cloudScrollY"].asReal() + 10.0f);

	mSky->setEnableCloudScrollX(!body["cloudScrollXLock"].asBoolean());
	mSky->setEnableCloudScrollY(!body["cloudScrollYLock"].asBoolean());

	mSky->setStarBrightness(body["starBrightness"].asReal());

	mSky->set("fade", body["fade"].asReal());

	//Update this here.. since it isn't a part of WL... go figure
	gHippoLimits->skyUseClassicClouds = body["drawClassicClouds"].asBoolean();
	gSavedSettings.setF32("ClassicCloudHeight",body["classicCloudHeight"].asReal());
	gSavedSettings.setF32("ClassicCloudRange",body["classicCloudRange"].asReal());

	WLSettingsManager::Apply(mSky, mWater, mWaterNormal);
}
};

LLHTTPRegistration<WindLightSettingsUpdate>
gHTTPRegistrationWindLightSettingsUpdate(
	"/message/WindLightSettingsUpdate");