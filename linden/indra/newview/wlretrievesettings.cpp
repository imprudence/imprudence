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
#include "wlretrievesettings.h"
#include "wlfloatermanager.h"

#undef max

//If we get back a normal response, handle it here
void retrieveWindlightSettings::result(const LLSD& content)
{
	//Clear the lists first
	WLFloaterManager::mMinAltParamList.clear();
	WLFloaterManager::mMaxAltParamList.clear();
	WLFloaterManager::mFadeParamList.clear();
	WLFloaterManager::mWLParamList.clear();
	WLFloaterManager::mWaterParamList.clear();
	WLFloaterManager::mWaterNormalParamList.clear();

	for(LLSD::array_const_iterator wls = content["WindLight"].beginArray();
		wls != content["WindLight"].endArray();
		++wls)
	{	
		LLSD windLightLLSD = *wls;
		LLWaterParamSet* mWater;
		LLWLParamSet* mSky;
		LLUUID* mWaterNormal;

		mWater = new LLWaterParamSet();
		mSky = new LLWLParamSet();
		mWaterNormal = new LLUUID();

		mWater->set("waterFogColor",
			windLightLLSD["waterColorX"].asReal()   / 256.f,
			windLightLLSD["waterColorY"].asReal()   / 256.f,
			windLightLLSD["waterColorZ"].asReal()   / 256.f,
			windLightLLSD["waterColorW"].asReal()   / 256.f);
		mWater->set("waterFogDensity", windLightLLSD["waterFogDensityExponent"].asReal());
		mWater->set("underWaterFogMod", windLightLLSD["underwaterFogModifier"].asReal());
		mWater->set("normScale", windLightLLSD["reflectionWaveletScaleX"].asReal(),
			windLightLLSD["reflectionWaveletScaleY"].asReal(),
			windLightLLSD["reflectionWaveletScaleZ"].asReal());
		mWater->set("fresnelScale", windLightLLSD["fresnelScale"].asReal());
		mWater->set("fresnelOffset", windLightLLSD["fresnelOffset"].asReal());
		mWater->set("scaleAbove", windLightLLSD["refractScaleAbove"].asReal());
		mWater->set("scaleBelow", windLightLLSD["refractScaleBelow"].asReal());
		mWater->set("blurMultiplier", windLightLLSD["blurMultiplier"].asReal());
		mWater->set("wave2Dir", windLightLLSD["bigWaveDirectionX"].asReal(),
			windLightLLSD["bigWaveDirectionY"].asReal());
		mWater->set("wave1Dir", windLightLLSD["littleWaveDirectionX"].asReal(),
			windLightLLSD["littleWaveDirectionY"].asReal());
		mWaterNormal->parseUUID(windLightLLSD["normalMapTexture"].asUUID().asString(), mWaterNormal);

		mSky->setSunAngle(windLightLLSD["sunMoonPosition"].asReal());
		mSky->setEastAngle(windLightLLSD["eastAngle"].asReal());

		mSky->set("sunlight_color",
			windLightLLSD["sunMoonColorX"].asReal() * 3.0f,
			windLightLLSD["sunMoonColorY"].asReal() * 3.0f,
			windLightLLSD["sunMoonColorZ"].asReal() * 3.0f,
			windLightLLSD["sunMoonColorW"].asReal() * 3.0f);

		mSky->set("ambient",
			windLightLLSD["ambientX"].asReal() * 3.0f,
			windLightLLSD["ambientY"].asReal() * 3.0f,
			windLightLLSD["ambientZ"].asReal() * 3.0f,
			windLightLLSD["ambientW"].asReal() * 3.0f);

		mSky->set("blue_horizon",
			windLightLLSD["horizonX"].asReal() * 2.0f,
			windLightLLSD["horizonY"].asReal() * 2.0f,
			windLightLLSD["horizonZ"].asReal() * 2.0f,
			windLightLLSD["horizonW"].asReal() * 2.0f);

		mSky->set("blue_density",
			windLightLLSD["blueDensityX"].asReal(),
			windLightLLSD["blueDensityY"].asReal(),
			windLightLLSD["blueDensityZ"].asReal(),
			1.0);

		mSky->set("haze_horizon",
			windLightLLSD["hazeHorizon"].asReal(),
			windLightLLSD["hazeHorizon"].asReal(),
			windLightLLSD["hazeHorizon"].asReal(),
			1.f);

		mSky->set("haze_density",
			windLightLLSD["hazeDensity"].asReal(),
			0.f, 0.f, 1.f);

		mSky->set("cloud_shadow",
			windLightLLSD["cloudCoverage"].asReal(),
			0.f, 0.f, 1.f);

		mSky->set("density_multiplier",
			windLightLLSD["densityMultiplier"].asReal() / 1000.0f,
			0.f, 0.f, 1.f);

		mSky->set("distance_multiplier",
			windLightLLSD["distanceMultiplier"].asReal(),
			0.f, 0.f, 1.f);

		mSky->set("max_y",
			windLightLLSD["maxAltitude"].asReal(),
			0.f, 0.f, 1.f);

		mSky->set("cloud_color",
			windLightLLSD["cloudColorX"].asReal(),
			windLightLLSD["cloudColorY"].asReal(),
			windLightLLSD["cloudColorZ"].asReal(),
			windLightLLSD["cloudColorW"].asReal());

		mSky->set("cloud_pos_density1",
			windLightLLSD["cloudXYDensityX"].asReal(),
			windLightLLSD["cloudXYDensityY"].asReal(),
			windLightLLSD["cloudXYDensityZ"].asReal(),
			1.f);

		mSky->set("cloud_pos_density2",
			windLightLLSD["cloudDetailXYDensityX"].asReal(),
			windLightLLSD["cloudDetailXYDensityY"].asReal(),
			windLightLLSD["cloudDetailXYDensityZ"].asReal(),
			1.f);

		mSky->set("cloud_scale",
			windLightLLSD["cloudScale"].asReal(),
			0.f, 0.f, 1.f);

		mSky->set("gamma",
			windLightLLSD["sceneGamma"].asReal(),
			0.f, 0.f, 1.f);

		mSky->set("glow",
			(2 - windLightLLSD["sunGlowSize"].asReal()) * 20,
			0.f,
			-windLightLLSD["sunGlowFocus"].asReal() * 5,
			1.f);

		mSky->setCloudScrollX(windLightLLSD["cloudScrollX"].asReal() + 10.0f);
		mSky->setCloudScrollY(windLightLLSD["cloudScrollY"].asReal() + 10.0f);

		mSky->setEnableCloudScrollX(!windLightLLSD["cloudScrollXLock"].asBoolean());
		mSky->setEnableCloudScrollY(!windLightLLSD["cloudScrollYLock"].asBoolean());

		mSky->setStarBrightness(windLightLLSD["starBrightness"].asReal());

		mSky->set("fade", windLightLLSD["fade"].asReal());

		WLFloaterManager::mMinAltParamList[windLightLLSD["Name"].asString()] = windLightLLSD["minEffectiveAltitude"].asReal();
		WLFloaterManager::mMaxAltParamList[windLightLLSD["Name"].asString()] = windLightLLSD["maxEffectiveAltitude"].asReal();
		WLFloaterManager::mFadeParamList[windLightLLSD["Name"].asString()] = windLightLLSD["fade"].asReal();

		WLFloaterManager::mWLParamList[windLightLLSD["Name"].asString()] = mSky;
		WLFloaterManager::mWaterParamList[windLightLLSD["Name"].asString()] = mWater;
		WLFloaterManager::mWaterNormalParamList[windLightLLSD["Name"].asString()] = mWaterNormal;
	}
	WLFloaterManager::UpdateFloater();
}

//If we get back an error (not found, etc...), handle it here
void retrieveWindlightSettings::error(U32 status, const std::string& reason)
{
	LL_INFOS("Inventory") << "retrieveWindlightSettings::error "
		<< status << ": " << reason << LL_ENDL;
}