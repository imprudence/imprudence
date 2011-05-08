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

#undef max


WLFloaterWindLightSend* WLFloaterWindLightSend::sWindLight = NULL;

WLFloaterWindLightSend::WLFloaterWindLightSend() : LLFloater(std::string("windlight send floater"))
{
	LLUICtrlFactory::getInstance()->buildFloater(this, "floater_windlight_remote_save.xml");

	// load it up
	initCallbacks();
}

WLFloaterWindLightSend::~WLFloaterWindLightSend()
{
}

void WLFloaterWindLightSend::initCallbacks(void) {

	// help buttons
	initHelpBtn("fade_help", "HelpBlueHorizon");
	initHelpBtn("override_parcel_default_help", "HelpHazeHorizon");
	initHelpBtn("override_parcel_help", "HelpBlueDensity");
	initHelpBtn("max_altitude_help", "HelpHazeDensity");
	initHelpBtn("min_altitude_help", "HelpDensityMult");
	childSetAction("button_region_send_to_server", onSaveRegionPreset, this);
	childSetAction("button_parcel_send_to_server", onSaveParcelPreset, this);
}

void WLFloaterWindLightSend::onClickHelp(void* data)
{
	LLFloaterWindLight* self = LLFloaterWindLight::instance();

	const std::string xml_alert = *(std::string*)data;
	LLNotifications::instance().add(self->contextualNotification(xml_alert));
}

void WLFloaterWindLightSend::initHelpBtn(const std::string& name, const std::string& xml_alert)
{
	childSetAction(name, onClickHelp, new std::string(xml_alert));
}

// static
WLFloaterWindLightSend* WLFloaterWindLightSend::instance()
{
	if (!sWindLight)
	{
		sWindLight = new WLFloaterWindLightSend();
		sWindLight->open();
		sWindLight->setFocus(TRUE);
	}
	return sWindLight;
}

void WLFloaterWindLightSend::show()
{
	if (!sWindLight)
	{
		WLFloaterWindLightSend::instance();
	}
	else
	{
		if (sWindLight->getVisible())
		{
			sWindLight->close();
		}
		else
		{
			sWindLight->open();
		}
	}
}

bool WLFloaterWindLightSend::isOpen()
{
	if (sWindLight != NULL) {
		return true;
	}
	return false;
}

// virtual
void WLFloaterWindLightSend::onClose(bool app_quitting)
{
	if (sWindLight)
	{
		sWindLight->setVisible(FALSE);
	}
}

void WLFloaterWindLightSend::onSaveRegionPreset(void* userData)
{
	int RegionType = 0;
	SendSettings(false, RegionType, 
		WLFloaterWindLightSend::instance()->childGetValue("override_parcel"),
		LLWLParamManager::instance()->mCurParams,
		LLWaterParamManager::instance()->mCurParams,
		WLFloaterWindLightSend::instance()->childGetValue("Fade"),
		WLFloaterWindLightSend::instance()->childGetValue("min_altitude"),
		WLFloaterWindLightSend::instance()->childGetValue("max_altitude"),
		LLWaterParamManager::instance()->getNormalMapID());
}
void WLFloaterWindLightSend::onSaveParcelPreset(void* userData)
{
	int ParcelType = 1;
	SendSettings(false, ParcelType, false, LLWLParamManager::instance()->mCurParams,
		LLWaterParamManager::instance()->mCurParams,
		WLFloaterWindLightSend::instance()->childGetValue("Fade"),
		WLFloaterWindLightSend::instance()->childGetValue("min_altitude"),
		WLFloaterWindLightSend::instance()->childGetValue("max_altitude"),
		LLWaterParamManager::instance()->getNormalMapID());
}
void WLFloaterWindLightSend::SendSettings(bool remove, int type, bool overrideParcels,
										  LLWLParamSet mSky, LLWaterParamSet mWater,
										  LLSD fade, LLSD minAlt, LLSD maxAlt,
										  LLUUID normalMap)
{
	LLSD body;
	std::string url = gAgent.getRegion()->getCapability("DispatchWindLightSettings");
	if (!url.empty())
	{
		bool error;

		body["type"] = type;
		body["remove"] = remove;
		body["fade"] = fade;
		body["maxEffectiveAltitude"] = maxAlt;
		body["minEffectiveAltitude"] = minAlt;
		if(overrideParcels == true || overrideParcels == false)
			body["overrideParcels"] = overrideParcels;

		LLVector4 v = mSky.getVector("ambient", error);
		body["ambientX"] = v[0] / 3.0;
		body["ambientY"] = v[1] / 3.0;
		body["ambientZ"] = v[2] / 3.0;
		body["ambientW"] = v[3] / 3.0;

		body["eastAngle"] = mSky.getEastAngle();
		body["sunMoonPosition"] = mSky.getSunAngle();

		v = mSky.getVector("sunlight_color",error);
		body["sunMoonColorX"] = v[0] / 3.0;
		body["sunMoonColorY"] = v[1] / 3.0;
		body["sunMoonColorZ"] = v[2] / 3.0;
		body["sunMoonColorW"] = v[3] / 3.0;

		v = mSky.getVector("blue_horizon",error);
		body["horizonX"] = v[0] / 2.0;
		body["horizonY"] = v[1] / 2.0;
		body["horizonZ"] = v[2] / 2.0;
		body["horizonW"] = v[3] / 2.0;

		v = mSky.getVector("blue_density",error);
		body["blueDensityX"] = v[0];
		body["blueDensityY"] = v[1];
		body["blueDensityZ"] = v[2];

		v = mSky.getVector("haze_horizon",error);
		body["hazeHorizon"] = v[0];

		body["hazeDensity"] = mSky.getFloat("haze_density",error);
		body["cloudCoverage"] = mSky.getFloat("cloud_shadow",error);
		body["densityMultiplier"] = mSky.getFloat("density_multiplier",error) * 1000;
		body["distanceMultiplier"] = mSky.getFloat("distance_multiplier",error);
		body["maxAltitude"] = mSky.getFloat("max_y",error);

		v = mSky.getVector("cloud_color",error);
		body["cloudColorX"] = v[0];
		body["cloudColorY"] = v[1];
		body["cloudColorZ"] = v[2];
		body["cloudColorW"] = v[3];

		v = mSky.getVector("cloud_pos_density1",error);
		body["cloudXYDensityX"] = v[0];
		body["cloudXYDensityY"] = v[1];
		body["cloudXYDensityZ"] = v[2];

		v = mSky.getVector("cloud_pos_density2",error);
		body["cloudDetailXYDensityX"] = v[0];
		body["cloudDetailXYDensityY"] = v[1];
		body["cloudDetailXYDensityZ"] = v[2];

		v = mSky.getVector("glow",error);
		body["sunGlowSize"] = -((v[0]/ 20) - 2);
		body["sunGlowFocus"] = -v[2] / 5;

		body["cloudScale"] = mSky.getFloat("cloud_scale",error);
		body["sceneGamma"] = mSky.getFloat("gamma",error);
		body["cloudScrollX"] = mSky.getCloudScrollX() - 10;
		body["cloudScrollY"] = mSky.getCloudScrollY() - 10;
		body["cloudScrollXLock"] = !mSky.getEnableCloudScrollX();
		body["cloudScrollYLock"] = !mSky.getEnableCloudScrollY();
		body["starBrightness"] = mSky.getStarBrightness();
		body["drawClassicClouds"] = gHippoLimits->skyUseClassicClouds;
		body["classicCloudHeight"] = gSavedSettings.getF32("ClassicCloudHeight");
		body["classicCloudRange"] = gSavedSettings.getF32("ClassicCloudRange");

		LLVector3 vvv = mWater.getVector3("normScale",error);
		body["reflectionWaveletScaleX"] = vvv[0];
		body["reflectionWaveletScaleY"] = vvv[1];
		body["reflectionWaveletScaleZ"] = vvv[2];

		v = mWater.getVector4("waterFogColor",error);
		body["waterColorX"] = v[0] * 256.0;
		body["waterColorY"] = v[1] * 256.0;
		body["waterColorZ"] = v[2] * 256.0;
		body["waterColorW"] = v[3] * 256.0;

		body["waterFogDensityExponent"] = mWater.getFloat("waterFogDensity", error);
		body["underwaterFogModifier"] = mWater.getFloat("underWaterFogMod", error);

		body["fresnelScale"] = mWater.getFloat("fresnelScale", error);
		body["fresnelOffset"] = mWater.getFloat("fresnelOffset", error);
		body["refractScaleAbove"] = mWater.getFloat("scaleAbove", error);
		body["refractScaleBelow"] = mWater.getFloat("scaleBelow", error);
		body["blurMultiplier"] = mWater.getFloat("blurMultiplier", error);

		LLVector2 vv = mWater.getVector2("wave1Dir",error);
		body["littleWaveDirectionX"] = vv[0];
		body["littleWaveDirectionY"] = vv[1];

		vv = mWater.getVector2("wave2Dir",error);
		body["bigWaveDirectionX"] = vv[0];
		body["bigWaveDirectionY"] = vv[1];

		body["normalMapTexture"] = normalMap;

		LLHTTPClient::post(url, body, new LLHTTPClient::Responder());
	}
}