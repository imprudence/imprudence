/** 
 * @file llfloatervoicewizard.cpp
 * @author Richard Nelson
 * @brief Voice communication set-up wizard
 *
 * $LicenseInfo:firstyear=2007&license=viewergpl$
 * 
 * Copyright (c) 2007-2008, Linden Research, Inc.
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
 * online at http://secondlifegrid.net/programs/open_source/licensing/flossexception
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

#include "llfloatervoicewizard.h"

#include "llagent.h"
#include "llbutton.h"
#include "llcombobox.h"
#include "llfocusmgr.h"
#include "lliconctrl.h"
#include "llprefsvoice.h"
#include "llsliderctrl.h"
#include "llviewercontrol.h"
#include "llvieweruictrlfactory.h"
#include "llvoiceclient.h"
#include "llimpanel.h"

LLFloaterVoiceWizard::LLFloaterVoiceWizard(const LLSD& seed) : LLFloater("floater_voice_wizard"), mDevicePanel(NULL)
{
	mFactoryMap["device_settings"] = LLCallbackMap(createPanelDeviceSettings, this);
	// do not automatically open singleton floaters (as result of getInstance())
	BOOL no_open = FALSE;
	gUICtrlFactory->buildFloater(this, "floater_voice_wizard.xml", &mFactoryMap, no_open);

	mLogic = new LLPrefsVoiceLogic(this);
	center();
}

LLFloaterVoiceWizard::~LLFloaterVoiceWizard()
{
	delete mLogic;
	mLogic = NULL;
}

BOOL LLFloaterVoiceWizard::postBuild()
{
	childSetAction("next_btn", onClickNext, this);
	childSetAction("back_btn", onClickBack, this);
	childSetAction("ok_btn", onClickOK, this);
	childSetAction("cancel_btn", onClickCancel, this);

	childSetCommitCallback("voice_enable", onCommitVoiceEnable, this);

	return TRUE;
}

void LLFloaterVoiceWizard::draw()
{
	mLogic->refresh();
	if (mDevicePanel)
	{
		mDevicePanel->refresh();
	}

	LLTabContainerCommon* tabs = LLUICtrlFactory::getTabContainerByName(this, "wizard_tabs");

	if (tabs)
	{
		// if on first tab, disable back button, etc
		if (tabs->getCurrentPanelIndex() == 0)
		{
			if (gSavedSettings.getBOOL("EnableVoiceChat"))
			{
				childSetEnabled("next_btn", TRUE);
				setDefaultBtn(gUICtrlFactory->getButtonByName(this, "next_btn"));
			}
			else
			{
				childSetEnabled("next_btn", FALSE);
				setDefaultBtn(gUICtrlFactory->getButtonByName(this, "ok_btn"));
			}
			childSetEnabled("back_btn", FALSE);
		}
		else
		{
			// if on any tab but the last, enable the next button
			if (tabs->getCurrentPanelIndex() < tabs->getTabCount() - 1)
			{
				childSetEnabled("next_btn", TRUE);
				setDefaultBtn(gUICtrlFactory->getButtonByName(this, "next_btn"));
			}
			else
			{
				childSetEnabled("next_btn", FALSE);
				setDefaultBtn(gUICtrlFactory->getButtonByName(this, "ok_btn"));
			}
			childSetEnabled("back_btn", TRUE);
		}
	}

	// because we can simultaneously change voice settings from the preferences UI
	// we need to stay in sync
	childSetValue("voice_enable", gSavedSettings.getBOOL("EnableVoiceChat") ? "1" : "0");

	// show appropriate text on first tab
	childSetVisible("voice_intro_text3", !gSavedSettings.getBOOL("EnableVoiceChat"));
	childSetVisible("voice_intro_text4", gSavedSettings.getBOOL("EnableVoiceChat"));

	LLFloater::draw();
}

void LLFloaterVoiceWizard::onOpen()
{
	LLFloaterDeviceSettings::hideInstance();

	if(mDevicePanel)
	{
		mDevicePanel->onOpen();
	}

	LLFloater::onOpen();
}

void LLFloaterVoiceWizard::onClose(bool app_quitting)
{
	if(mDevicePanel)
	{
		mDevicePanel->onClose(app_quitting);
	}

	LLFloater::onClose(app_quitting);
}


// static
void LLFloaterVoiceWizard::onClickNext(void *user_data)
{
	LLTabContainerCommon* tabs = LLUICtrlFactory::getTabContainerByName((LLFloater*)user_data, "wizard_tabs");
	if (tabs)
	{
		tabs->selectNextTab();
	}
}

// static
void LLFloaterVoiceWizard::onClickBack(void *user_data)
{
	LLTabContainerCommon* tabs = LLUICtrlFactory::getTabContainerByName((LLFloater*)user_data, "wizard_tabs");
	if (tabs)
	{
		tabs->selectPrevTab();
	}
}

// static
void LLFloaterVoiceWizard::onClickOK(void *user_data)
{
	LLFloaterVoiceWizard* self = (LLFloaterVoiceWizard*)user_data;

	// propagate tuning mic volume to actual mic volume
	self->mLogic->apply();
	if (self->mDevicePanel)
	{
		self->mDevicePanel->apply();
	}
	self->close();
}

// static
void LLFloaterVoiceWizard::onClickCancel(void *user_data)
{
	LLFloaterVoiceWizard* self = (LLFloaterVoiceWizard*)user_data;
	
	self->mLogic->cancel();
	if (self->mDevicePanel)
	{
		self->mDevicePanel->cancel();
	}
	self->close();
}

// static
void LLFloaterVoiceWizard::onCommitVoiceEnable(LLUICtrl* ctrl, void* user_data)
{
	gSavedSettings.setBOOL("EnableVoiceChat", ctrl->getValue().asInteger());
}

// static
void* LLFloaterVoiceWizard::createPanelDeviceSettings(void* user_data)
{
	LLFloaterVoiceWizard* floaterp = (LLFloaterVoiceWizard*)user_data;
	floaterp->mDevicePanel = new LLPanelDeviceSettings();
	return floaterp->mDevicePanel;
}


//
// LLPanelDeviceSettings
//

LLPanelDeviceSettings::LLPanelDeviceSettings()
{
	mCtrlInputDevices = NULL;
	mCtrlOutputDevices = NULL;
	mInputDevice = gSavedSettings.getString("VoiceInputAudioDevice");
	mOutputDevice = gSavedSettings.getString("VoiceOutputAudioDevice");
	mDevicesUpdated = FALSE;

	// grab "live" mic volume level
	mMicVolume = gSavedSettings.getF32("AudioLevelMic");

	// ask for new device enumeration
	// now do this in onOpen() instead...
	//gVoiceClient->refreshDeviceLists();
}

LLPanelDeviceSettings::~LLPanelDeviceSettings()
{
}

BOOL LLPanelDeviceSettings::postBuild()
{
	LLSlider* volume_slider = gUICtrlFactory->getSliderBarByName(this, "mic_volume_slider");
	if (volume_slider)
	{
		// set mic volume tuning slider based on last mic volume setting
		volume_slider->setValue(mMicVolume);
	}

	childSetCommitCallback("voice_input_device", onCommitInputDevice, this);
	childSetCommitCallback("voice_output_device", onCommitOutputDevice, this);
	
	return TRUE;
}

void LLPanelDeviceSettings::draw()
{
	// let user know that volume indicator is not yet available
	childSetVisible("wait_text", !gVoiceClient->inTuningMode());

	LLPanel::draw();

	F32 voice_power = gVoiceClient->tuningGetEnergy();
	S32 discrete_power = 0;

	if (!gVoiceClient->inTuningMode())
	{
		discrete_power = 0;
	}
	else
	{
		discrete_power = llmin(4, llfloor((voice_power / LLVoiceClient::OVERDRIVEN_POWER_LEVEL) * 4.f));
	}
	
	if (gVoiceClient->inTuningMode())
	{
		for(S32 power_bar_idx = 0; power_bar_idx < 5; power_bar_idx++)
		{
			LLString view_name = llformat("%s%d", "bar", power_bar_idx);
			LLView* bar_view = getChildByName(view_name, TRUE);
			if (bar_view)
			{
				if (power_bar_idx < discrete_power)
				{
					LLColor4 color = (power_bar_idx >= 3) ? gSavedSettings.getColor4("OverdrivenColor") : gSavedSettings.getColor4("SpeakingColor");
					gl_rect_2d(bar_view->getRect(), color, TRUE);
				}
				gl_rect_2d(bar_view->getRect(), LLColor4::grey, FALSE);
			}
		}
	}
}

void LLPanelDeviceSettings::apply()
{
	std::string s;
	if(mCtrlInputDevices)
	{
		s = mCtrlInputDevices->getSimple();
		gSavedSettings.setString("VoiceInputAudioDevice", s);
	}

	if(mCtrlOutputDevices)
	{
		s = mCtrlOutputDevices->getSimple();
		gSavedSettings.setString("VoiceOutputAudioDevice", s);
	}

	// assume we are being destroyed by closing our embedding window
	gSavedSettings.setF32("AudioLevelMic", mMicVolume);
}

void LLPanelDeviceSettings::cancel()
{
	gSavedSettings.setString("VoiceInputAudioDevice", mInputDevice);
	gSavedSettings.setString("VoiceOutputAudioDevice", mOutputDevice);

	if(mCtrlInputDevices)
		mCtrlInputDevices->setSimple(mInputDevice);

	if(mCtrlOutputDevices)
		mCtrlOutputDevices->setSimple(mOutputDevice);
}

void LLPanelDeviceSettings::refresh()
{
	//grab current volume
	LLSlider* volume_slider = gUICtrlFactory->getSliderBarByName(this, "mic_volume_slider");
	if (volume_slider)
	{
		// set mic volume tuning slider based on last mic volume setting
		mMicVolume = (F32)volume_slider->getValue().asReal();
		gVoiceClient->tuningSetMicVolume(mMicVolume);
	}

	// Fill in popup menus
	mCtrlInputDevices = LLUICtrlFactory::getComboBoxByName(this, "voice_input_device");
	mCtrlOutputDevices = LLUICtrlFactory::getComboBoxByName(this, "voice_output_device");

	if(!gVoiceClient->deviceSettingsAvailable())
	{
		// The combo boxes are disabled, since we can't get the device settings from the daemon just now.
		// Put the currently set default (ONLY) in the box, and select it.
		if(mCtrlInputDevices)
		{
			mCtrlInputDevices->removeall();
			mCtrlInputDevices->add( mInputDevice, ADD_BOTTOM );
			mCtrlInputDevices->setSimple(mInputDevice);
		}
		if(mCtrlOutputDevices)
		{
			mCtrlOutputDevices->removeall();
			mCtrlOutputDevices->add( mOutputDevice, ADD_BOTTOM );
			mCtrlOutputDevices->setSimple(mOutputDevice);
		}
	}
	else if (!mDevicesUpdated)
	{
		LLVoiceClient::deviceList *devices;
		
		LLVoiceClient::deviceList::iterator iter;
		
		if(mCtrlInputDevices)
		{
			mCtrlInputDevices->removeall();
			mCtrlInputDevices->add( childGetText("default_text"), ADD_BOTTOM );

			devices = gVoiceClient->getCaptureDevices();
			for(iter=devices->begin(); iter != devices->end(); iter++)
			{
				mCtrlInputDevices->add( *iter, ADD_BOTTOM );
			}

			if(!mCtrlInputDevices->setSimple(mInputDevice))
			{
				mCtrlInputDevices->setSimple(childGetText("default_text"));
			}
		}
		
		if(mCtrlOutputDevices)
		{
			mCtrlOutputDevices->removeall();
			mCtrlOutputDevices->add( childGetText("default_text"), ADD_BOTTOM );

			devices = gVoiceClient->getRenderDevices();
			for(iter=devices->begin(); iter != devices->end(); iter++)
			{
				mCtrlOutputDevices->add( *iter, ADD_BOTTOM );
			}

			if(!mCtrlOutputDevices->setSimple(mOutputDevice))
			{
				mCtrlOutputDevices->setSimple(childGetText("default_text"));
			}
		}
		mDevicesUpdated = TRUE;
	}	
}

void LLPanelDeviceSettings::onOpen()
{
	mInputDevice = gSavedSettings.getString("VoiceInputAudioDevice");
	mOutputDevice = gSavedSettings.getString("VoiceOutputAudioDevice");
	mMicVolume = gSavedSettings.getF32("AudioLevelMic");
	mDevicesUpdated = FALSE;

	// ask for new device enumeration
	gVoiceClient->refreshDeviceLists();

	// put voice client in "tuning" mode
	gVoiceClient->tuningStart();
	LLVoiceChannel::suspend();
}

void LLPanelDeviceSettings::onClose(bool app_quitting)
{
	gVoiceClient->tuningStop();
	LLVoiceChannel::resume();
}

// static
void LLPanelDeviceSettings::onCommitInputDevice(LLUICtrl* ctrl, void* user_data)
{
	gSavedSettings.setString("VoiceInputAudioDevice", ctrl->getValue().asString());
}

// static
void LLPanelDeviceSettings::onCommitOutputDevice(LLUICtrl* ctrl, void* user_data)
{
	gSavedSettings.setString("VoiceOutputAudioDevice", ctrl->getValue().asString());
}

//
// LLFloaterDeviceSettings
//

LLFloaterDeviceSettings::LLFloaterDeviceSettings(const LLSD& seed) : LLFloater("floater_device_settings"), mDevicePanel(NULL)
{
	mFactoryMap["device_settings"] = LLCallbackMap(createPanelDeviceSettings, this);
	// do not automatically open singleton floaters (as result of getInstance())
	BOOL no_open = FALSE;
	gUICtrlFactory->buildFloater(this, "floater_device_settings.xml", &mFactoryMap, no_open);
	center();
}

void LLFloaterDeviceSettings::onOpen()
{
	LLFloaterVoiceWizard::hideInstance();
	if(mDevicePanel)
	{
		mDevicePanel->onOpen();
	}

	LLFloater::onOpen();
}

void LLFloaterDeviceSettings::onClose(bool app_quitting)
{
	if(mDevicePanel)
	{
		mDevicePanel->onClose(app_quitting);
	}

	setVisible(FALSE);
}

void LLFloaterDeviceSettings::apply()
{
	if (mDevicePanel)
	{
		mDevicePanel->apply();
	}
}

void LLFloaterDeviceSettings::cancel()
{
	if (mDevicePanel)
	{
		mDevicePanel->cancel();
	}
}

void LLFloaterDeviceSettings::draw()
{
	if (mDevicePanel)
	{
		mDevicePanel->refresh();
	}
	LLFloater::draw();
}

// static
void* LLFloaterDeviceSettings::createPanelDeviceSettings(void* user_data)
{
	LLFloaterDeviceSettings* floaterp = (LLFloaterDeviceSettings*)user_data;
	floaterp->mDevicePanel = new LLPanelDeviceSettings();
	return floaterp->mDevicePanel;
}
