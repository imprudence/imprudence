/**
* @file llwindlightremotectrl.cpp
* @brief toolbar remote for windlight options and presets
*
* $LicenseInfo:firstyear=2009&license=viewergpl$
*
* Copyright (c) 2009, McCabe Maxsted
*
* Imprudence Viewer Source Code
* The source code in this file ("Source Code") is provided to you
* under the terms of the GNU General Public License, version 2.0
* ("GPL"). Terms of the GPL can be found in doc/GPL-license.txt in
* this distribution, or online at
* http://secondlifegrid.net/programs/open_source/licensing/gplv2
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
* ALL SOURCE CODE IS PROVIDED "AS IS." THE AUTHOR MAKES NO
* WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
* COMPLETENESS OR PERFORMANCE.
* $/LicenseInfo$
*/

#include "llviewerprecompiledheaders.h"

#include "llwindlightremotectrl.h"

#include "llbutton.h"
#include "llcombobox.h"
#include "llfloaterenvsettings.h"
#include "llfloaterwater.h"
#include "llfloaterwindlight.h"
#include "lloverlaybar.h"
#include "lluictrlfactory.h"
#include "llwlparammanager.h"
#include "llviewercontrol.h"

// [RLVa:KB] - Alternate: Imprudence-1.2.0
#include "rlvhandler.h"
// [/RLVa:KB]

class LLWindlightRemoteObserver : public LLWLPresetsObserver
{
public:
	LLWindlightRemoteObserver(LLWindlightRemoteCtrl* wind_rem) : mWindlightRemote(wind_rem){}
	virtual ~LLWindlightRemoteObserver() {}
	virtual void changed() { mWindlightRemote->refreshPresets(); }
private:
	LLWindlightRemoteCtrl* mWindlightRemote;
};


LLWindlightRemoteCtrl::LLWindlightRemoteCtrl() 	
{
	mPresetsCombo = NULL;
	mObserver = NULL;
	setIsChrome(TRUE);

	build();

	setFocusRoot(TRUE);
}

LLWindlightRemoteCtrl::~LLWindlightRemoteCtrl()
{
	delete mObserver;
	mObserver = NULL;
}

void LLWindlightRemoteCtrl::draw()
{
	LLButton* expand_button = getChild<LLButton>("Popup");
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

// [RLVa:KB] - Alternate: Imprudence-1.2.0
	if (rlv_handler_t::isEnabled())
	{
		childSetEnabled("Environment", !gRlvHandler.hasBehaviour(RLV_BHVR_SETENV));
		mPresetsCombo->setEnabled(!gRlvHandler.hasBehaviour(RLV_BHVR_SETENV));
	}
// [/RLVA:KB]

	LLPanel::draw();
}

void LLWindlightRemoteCtrl::build()
{
	if (gSavedSettings.getBOOL("ShowWindlightSettingsPopup"))
	{
		LLUICtrlFactory::getInstance()->buildPanel(this, "panel_windlight_remote_expanded.xml");
	}
	else
	{
		LLUICtrlFactory::getInstance()->buildPanel(this, "panel_windlight_remote.xml");
	}
}

BOOL LLWindlightRemoteCtrl::postBuild()
{
	
	childSetAction("Environment", onClickToggleEnvironment, this);
	childSetAction("Popup", onClickPopupBtn, this);

	mPresetsCombo = getChild<LLComboBox>("Presets");
	if (mPresetsCombo)
	{
		mPresetsCombo->setCommitCallback(onCommitPreset);
		mPresetsCombo->setCallbackUserData(this);

		// set up observer to follow changes
		mObserver = new LLWindlightRemoteObserver(this);
		LLWLParamManager::addObserver(mObserver);

		// refresh list from current presets
		refreshPresets();
	}

	return TRUE;
}

void LLWindlightRemoteCtrl::refreshPresets()
{
	// Just in case, let's not ever crash here
	mPresetsCombo = getChild<LLComboBox>("Presets");
	if (mPresetsCombo)
	{
		// snag current preset
		LLWLParamManager * param_mgr = LLWLParamManager::instance();
		LLWLParamSet& currentParams = param_mgr->mCurParams;
		
		// clear in case presets names have changed
		mPresetsCombo->clearRows();

		std::map<std::string, LLWLParamSet>::iterator mIt = 
			param_mgr->mParamList.begin();
		for(; mIt != param_mgr->mParamList.end(); mIt++) 
		{
			mPresetsCombo->add(mIt->first);
		}
		
		// insert separator and add World menu options
		// mPresetsCombo->addSeparator(ADD_BOTTOM);
		// mPresetsCombo->addSimpleElement(getString("atmosphere"), ADD_BOTTOM);
		// mPresetsCombo->addSimpleElement(getString("lighting"), ADD_BOTTOM);
		// mPresetsCombo->addSimpleElement(getString("clouds"), ADD_BOTTOM);
		// mPresetsCombo->addSimpleElement(getString("advanced_water"), ADD_BOTTOM);
		mPresetsCombo->addSeparator(ADD_BOTTOM);
		mPresetsCombo->addSimpleElement(getString("sunrise"), ADD_BOTTOM);
		mPresetsCombo->addSimpleElement(getString("noon"), ADD_BOTTOM);
		mPresetsCombo->addSimpleElement(getString("sunset"), ADD_BOTTOM);
		mPresetsCombo->addSimpleElement(getString("midnight"), ADD_BOTTOM);
		mPresetsCombo->addSimpleElement(getString("revert_region"), ADD_BOTTOM);

		if (mPresetsCombo->getSelectedItemLabel() != currentParams.mName &&
			!currentParams.mName.empty())
		{
			mPresetsCombo->selectByValue(LLSD(currentParams.mName));
		}
		else
		{
			mPresetsCombo->selectByValue(LLSD("Default"));
		}
	}
}

// static
void LLWindlightRemoteCtrl::onCommitPreset(LLUICtrl* ctrl, void* data)
{
	LLWindlightRemoteCtrl* self = (LLWindlightRemoteCtrl*)data;

	LLCtrlListInterface* presets = self->mPresetsCombo ? self->mPresetsCombo->getListInterface() : NULL;
	if (presets)
	{
		S32 index = presets->getFirstSelectedIndex();
		if (index < 0)
		{
			// Open Advanced Sky
			LLFloaterWindLight::show();
			return;
		}

		// check for World menu options; if none, apply preset
		std::string selected = self->mPresetsCombo->getSelectedValue().asString();

		if (selected == self->getString("atmosphere"))
		{
			if (!LLFloaterWindLight::instance()->getVisible())
			{
				LLFloaterWindLight::show();
			}
			LLFloaterWindLight::instance()->selectTab("Atmosphere");
		}
		else if (selected == self->getString("lighting"))
		{
			if (!LLFloaterWindLight::instance()->getVisible())
			{
				LLFloaterWindLight::show();
			}
			LLFloaterWindLight::instance()->selectTab("Lighting");
		}
		else if (selected == self->getString("clouds"))
		{
			if (!LLFloaterWindLight::instance()->getVisible())
			{
				LLFloaterWindLight::show();
			}
			LLFloaterWindLight::instance()->selectTab("Clouds");
		}
		else if (selected == self->getString("advanced_water"))
		{
			LLFloaterWater::show();
		}
		else if (selected == self->getString("sunrise"))
		{
			// set the value, turn off animation
			LLWLParamManager::instance()->mAnimator.setDayTime(0.25);
			LLWLParamManager::instance()->mAnimator.mIsRunning = false;
			LLWLParamManager::instance()->mAnimator.mUseLindenTime = false;

			// then call update once
			LLWLParamManager::instance()->mAnimator.update(
				LLWLParamManager::instance()->mCurParams);
		}
		else if (selected == self->getString("noon"))
		{
			// set the value, turn off animation
			LLWLParamManager::instance()->mAnimator.setDayTime(0.567);
			LLWLParamManager::instance()->mAnimator.mIsRunning = false;
			LLWLParamManager::instance()->mAnimator.mUseLindenTime = false;

			// then call update once
			LLWLParamManager::instance()->mAnimator.update(
				LLWLParamManager::instance()->mCurParams);
		}
		else if (selected == self->getString("sunset"))
		{
			// set the value, turn off animation
			LLWLParamManager::instance()->mAnimator.setDayTime(0.75);
			LLWLParamManager::instance()->mAnimator.mIsRunning = false;
			LLWLParamManager::instance()->mAnimator.mUseLindenTime = false;

			// then call update once
			LLWLParamManager::instance()->mAnimator.update(
				LLWLParamManager::instance()->mCurParams);
		}
		else if (selected == self->getString("midnight"))
		{
			// set the value, turn off animation
			LLWLParamManager::instance()->mAnimator.setDayTime(0.0);
			LLWLParamManager::instance()->mAnimator.mIsRunning = false;
			LLWLParamManager::instance()->mAnimator.mUseLindenTime = false;

			// then call update once
			LLWLParamManager::instance()->mAnimator.update(
				LLWLParamManager::instance()->mCurParams);
		}
		else if (selected == self->getString("revert_region"))
		{
			LLWLParamManager::instance()->mAnimator.mIsRunning = true;
			LLWLParamManager::instance()->mAnimator.mUseLindenTime = true;	
		}
		else
		{
			// Apply preset
			LLWLParamManager::instance()->mAnimator.mIsRunning = false;
			LLWLParamManager::instance()->mAnimator.mUseLindenTime = false;
			LLWLParamManager::instance()->loadPreset(selected, true);
		}
	}
}

// static
void LLWindlightRemoteCtrl::onClickToggleEnvironment(void* data)
{
	// if evn settings not there or is hidden, show it
	if(	!LLFloaterEnvSettings::isOpen() || 
		!LLFloaterEnvSettings::instance()->getVisible()) 
	{
		LLFloaterEnvSettings::show();
	
	// otherwise, close it button acts like a toggle
	} 
	else 
	{
		LLFloaterEnvSettings::instance()->close();
	}
}

//static
void LLWindlightRemoteCtrl::onClickPopupBtn(void* data)
{
	LLWindlightRemoteCtrl* remotep = (LLWindlightRemoteCtrl*)data;

	remotep->deleteAllChildren();
	remotep->build();
	gOverlayBar->layoutButtons();
}
