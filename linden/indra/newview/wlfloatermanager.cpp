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
#include "wlfloatermanager.h"
#include "llviewerregion.h"
#include "llviewerparcelmgr.h"
#include "llparcel.h"
#include "wlretrievesettings.h"
#include "wlsettingsmanager.h"
#include "wlfloaterwindlightsend.h"

#undef max
WLFloaterManager* WLFloaterManager::sWindLight;
std::map<std::string, LLWLParamSet*> WLFloaterManager::mWLParamList;	
std::map<std::string, LLWaterParamSet*> WLFloaterManager::mWaterParamList;	
std::map<std::string, LLUUID*> WLFloaterManager::mWaterNormalParamList;	
std::map<std::string, LLSD> WLFloaterManager::mMinAltParamList;	
std::map<std::string, LLSD> WLFloaterManager::mMaxAltParamList;	
std::map<std::string, LLSD> WLFloaterManager::mFadeParamList;	

WLFloaterManager::WLFloaterManager() : LLFloater(std::string("windlight manager floater"))
{
	LLUICtrlFactory::getInstance()->buildFloater(this, "floater_windlight_manager.xml");

	// load it up
	initCallbacks();
}

WLFloaterManager::~WLFloaterManager()
{
}

void WLFloaterManager::initCallbacks(void) {

	// help buttons
	childSetAction("this_parcel", onGetThisParcel, this);
	childSetAction("all_parcels", onGetAllParcels, this);
	childSetAction("this_region", onGetThisRegion, this);
	childSetAction("show", onShow, this);
	childSetAction("set_to_current", onSetToCurrent, this);
	childSetAction("remove", onRemove, this);
}

void WLFloaterManager::onClickHelp(void* data)
{
	LLFloaterWindLight* self = LLFloaterWindLight::instance();

	const std::string xml_alert = *(std::string*)data;
	LLNotifications::instance().add(self->contextualNotification(xml_alert));
}

void WLFloaterManager::initHelpBtn(const std::string& name, const std::string& xml_alert)
{
	childSetAction(name, onClickHelp, new std::string(xml_alert));
}

// static
WLFloaterManager* WLFloaterManager::instance()
{
	if (!sWindLight)
	{
		sWindLight = new WLFloaterManager();
		sWindLight->open();
		sWindLight->setFocus(TRUE);
	}
	return sWindLight;
}

void WLFloaterManager::show()
{
	if (!sWindLight)
	{
		WLFloaterManager::instance();
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

bool WLFloaterManager::isOpen()
{
	if (sWindLight != NULL) {
		return true;
	}
	return false;
}

// virtual
void WLFloaterManager::onClose(bool app_quitting)
{
	if (sWindLight)
	{
		sWindLight->setVisible(FALSE);
	}
}

void WLFloaterManager::onGetThisRegion(void* userData)
{
	LLSD body;

	//Send the update CAPS to the server
	std::string url = gAgent.getRegion()->getCapability("RetrieveWindLightSettings");
	if (!url.empty())
	{
		body["RegionID"] = gAgent.getRegion()->getRegionID();
		LLHTTPClient::post(url, body, new retrieveWindlightSettings(body));
	}
}

void WLFloaterManager::onGetThisParcel(void* userData)
{
	LLSD body;

	//Send the update CAPS to the server
	std::string url = gAgent.getRegion()->getCapability("RetrieveWindLightSettings");
	if (!url.empty())
	{
		body["ParcelID"] = LLViewerParcelMgr::getInstance()->getAgentParcel()->getLocalID();
		LLHTTPClient::post(url, body, new retrieveWindlightSettings(body));
	}
}
void WLFloaterManager::onGetAllParcels(void* userData)
{
	LLSD body;

	//Send the update CAPS to the server
	std::string url = gAgent.getRegion()->getCapability("RetrieveWindLightSettings");
	if (!url.empty())
	{
		body["ParcelID"] = -1;
		LLHTTPClient::post(url, body, new retrieveWindlightSettings(body));
	}
}

void WLFloaterManager::onShow(void* userData)
{
	WLFloaterManager* mgr = WLFloaterManager::instance();
	LLComboBox* comboBox = mgr->getChild<LLComboBox>("WLSettingsCombo");
	std::string name = comboBox->getSelectedItemLabel();
	//Set the new settings up
	LLWLParamSet* mSky = mgr->mWLParamList[name];
	LLWaterParamSet* mWater = mgr->mWaterParamList[name];
	LLUUID* mWaterNormal = mgr->mWaterNormalParamList[name];
	if(mSky != NULL && mWater != NULL && mWaterNormal != NULL)
		WLSettingsManager::Apply(mSky, mWater, mWaterNormal);
}
void WLFloaterManager::onSetToCurrent(void* userData)
{
	WLFloaterManager* mgr = WLFloaterManager::instance();
	LLComboBox* comboBox = mgr->getChild<LLComboBox>("WLSettingsCombo");
	std::string name = comboBox->getSelectedItemLabel();

	LLWLParamSet* mSky = mgr->mWLParamList[name];
	LLWaterParamSet* mWater = mgr->mWaterParamList[name];
	LLUUID* mWaterNormal = mgr->mWaterNormalParamList[name];
	LLSD fade = mgr->mFadeParamList[name];
	LLSD minAlt = mgr->mMinAltParamList[name];
	LLSD maxAlt = mgr->mMaxAltParamList[name];

	int type = 1;
	if(name == "(Region Settings)")
	{
		type = 0;
	}	
	
	if(mSky != NULL && mWater != NULL && mWaterNormal != NULL)
		WLFloaterWindLightSend::SendSettings(false, type, NULL, *mSky, *mWater, fade, minAlt, maxAlt, *mWaterNormal); 
}
void WLFloaterManager::onRemove(void* userData)
{
	WLFloaterManager* mgr = WLFloaterManager::instance();
	LLComboBox* comboBox = mgr->getChild<LLComboBox>("WLSettingsCombo");
	std::string name = comboBox->getSelectedItemLabel();

	LLWLParamSet* mSky = mgr->mWLParamList[name];
	LLWaterParamSet* mWater = mgr->mWaterParamList[name];
	LLUUID* mWaterNormal = mgr->mWaterNormalParamList[name];
	LLSD fade = mgr->mFadeParamList[name];
	LLSD minAlt = mgr->mMinAltParamList[name];
	LLSD maxAlt = mgr->mMaxAltParamList[name];

	int type = 1;
	if(name == "(Region Settings)")
	{
		type = 0;
	}

	if(mSky != NULL && mWater != NULL && mWaterNormal != NULL)
		WLFloaterWindLightSend::SendSettings(true, type, false, *mSky, *mWater, fade, minAlt, maxAlt, *mWaterNormal); 
}


//static
void WLFloaterManager::UpdateFloater()
{
	WLFloaterManager* mgr = WLFloaterManager::instance();
	LLComboBox* comboBox = mgr->getChild<LLComboBox>("WLSettingsCombo");
	comboBox->clear();
	comboBox->removeall();
	std::map<std::string, LLWLParamSet*>::iterator mIt = 
		WLFloaterManager::instance()->mWLParamList.begin();
	for(; mIt != WLFloaterManager::instance()->mWLParamList.end(); mIt++) 
	{
		comboBox->add(mIt->first);
	}
	//Reorder them
	comboBox->sortByName();
}

