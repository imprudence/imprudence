/** 
 * @file llprefsvoice.cpp
 * @author Richard Nelson
 * @brief Voice chat preferences panel
 *
 * $LicenseInfo:firstyear=2003&license=viewergpl$
 * 
 * Copyright (c) 2003-2008, Linden Research, Inc.
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

#include "llprefsvoice.h"

#include "llcheckboxctrl.h"
#include "llcombobox.h"

#include "llviewercontrol.h"
#include "llvieweruictrlfactory.h"

#include "llmodaldialog.h"
#include "llkeyboard.h"
#include "llfocusmgr.h"
#include "llfloatervoicewizard.h"

#include "llappviewer.h"

#include "llvoiceclient.h"

class LLVoiceHotkeySelectDialog : public LLModalDialog
{
private:
	LLPrefsVoiceLogic	*mParent;
	LLFloater			*mOldFrontmost;

public:
	LLVoiceHotkeySelectDialog( LLPrefsVoiceLogic *parent )
		: LLModalDialog( "", 240, 100 ),
		  mParent( parent )
	{
		mOldFrontmost = gFloaterView->getFrontmost();

		gUICtrlFactory->buildFloater(this, "floater_select_key.xml");
		
		childSetAction("Cancel", LLVoiceHotkeySelectDialog::onCancel, this );
		childSetFocus("Cancel");
	}

	/*virtual*/ void setFocus( BOOL b )
	{
		LLFloater::setFocus(b);
		
		// This forces keyboard processing to happen at the raw key level instead of going through handleUnicodeChar.
		gFocusMgr.removeKeyboardFocusWithoutCallback(gFocusMgr.getKeyboardFocus());
	}

	static void onCancel( void* userdata );

	BOOL handleKey(KEY key, MASK mask, BOOL called_from_parent );

};

LLPrefsVoiceLogic::LLPrefsVoiceLogic(LLPanel* panelp) : 
	mPanel(panelp)
{
	init();
}
 
void LLPrefsVoiceLogic::init()
{
	mEnableVoice = gSavedSettings.getBOOL("EnableVoiceChat");

	mVoiceCallsFriendsOnly = gSavedSettings.getBOOL("VoiceCallsFriendsOnly");
	mEnablePushToTalk = gSavedSettings.getBOOL("EnablePushToTalk");
	mModifier = gSavedSettings.getString("PushToTalkButton");
	mPushToTalkToggle = gSavedSettings.getBOOL("PushToTalkToggle");
	mEarLocation = gSavedSettings.getS32("VoiceEarLocation");

	mCtrlEarLocation = LLUICtrlFactory::getSelectionInterfaceByName(mPanel, "ear_location");
	mCtrlEarLocation->selectByValue(LLSD(gSavedSettings.getS32("VoiceEarLocation")));
	mPanel->childSetCommitCallback("ear_location", onEarLocationCommit, this );

	mPanel->childSetAction("launch_voice_wizard_button", onClickLaunchWizard, mPanel);
	mPanel->childSetAction("set_voice_hotkey_button", onClickSetKey, this);
	mPanel->childSetAction("set_voice_middlemouse_button", onClickSetMiddleMouse, this);

	refresh();
	mEatNextSetKeyClick = FALSE;
}

void LLPrefsVoiceLogic::refresh()
{
	mPanel->childSetVisible("voice_unavailable", gDisableVoice);
	mPanel->childSetVisible("enable_voice_check", !gDisableVoice);
	mPanel->childSetEnabled("enable_voice_check", !gDisableVoice);
	
	bool enable = !gDisableVoice && gSavedSettings.getBOOL("EnableVoiceChat");
	
	mPanel->childSetEnabled("push_to_talk_check", enable);
	mPanel->childSetEnabled("push_to_talk_label", enable);
	mPanel->childSetEnabled("voice_call_friends_only_check", enable);
	mPanel->childSetEnabled("push_to_talk_toggle_check", enable /*&& gSavedSettings.getBOOL("EnablePushToTalk")*/);
	mPanel->childSetEnabled("ear_location", enable);
	mPanel->childSetEnabled("set_voice_hotkey_button", enable /*&& gSavedSettings.getBOOL("EnablePushToTalk")*/);
	mPanel->childSetEnabled("set_voice_middlemouse_button", enable /*&& gSavedSettings.getBOOL("EnablePushToTalk")*/);
}

void LLPrefsVoiceLogic::cancel()
{
	gSavedSettings.setBOOL("EnableVoiceChat", mEnableVoice);
	gSavedSettings.setBOOL("VoiceCallsFriendsOnly", mVoiceCallsFriendsOnly);
	gSavedSettings.setBOOL("EnablePushToTalk", mEnablePushToTalk );
	gSavedSettings.setString("PushToTalkButton", mModifier);
	gSavedSettings.setBOOL("PushToTalkToggle", mPushToTalkToggle );
	gSavedSettings.setS32("VoiceEarLocation", mEarLocation);
}

void LLPrefsVoiceLogic::apply()
{
}

//static 
void LLPrefsVoiceLogic::onEarLocationCommit(LLUICtrl* ctrl, void* user_data)
{
	LLCtrlSelectionInterface* interfacep = ctrl->getSelectionInterface();
	if (interfacep)
	{
		gSavedSettings.setS32("VoiceEarLocation", interfacep->getSelectedValue().asInteger());
	}
}

//static
void LLPrefsVoiceLogic::onClickLaunchWizard(void* user_data)
{
	LLPrefsVoice* prefs = (LLPrefsVoice*)user_data;
	LLFloaterVoiceWizard* floaterp = LLFloaterVoiceWizard::showInstance();
	LLFloater* parent_floater = gFloaterView->getParentFloater(prefs);
	if (parent_floater)
	{
		parent_floater->addDependentFloater(floaterp, FALSE);
	}
}

// static
void LLPrefsVoiceLogic::onClickSetKey(void* user_data)
{
	LLPrefsVoiceLogic* self=(LLPrefsVoiceLogic*)user_data;	
	if(self->mEatNextSetKeyClick)
	{
		self->mEatNextSetKeyClick = false;
	}
	else
	{
		LLVoiceHotkeySelectDialog* dialog = new LLVoiceHotkeySelectDialog( self );
		dialog->startModal();
		// dialog will delete itself
	}
}


// static
void LLPrefsVoiceLogic::onClickSetMiddleMouse(void* user_data)
{
//	LLPrefsVoiceLogic* self=(LLPrefsVoiceLogic*)user_data;
	
	gSavedSettings.setString("PushToTalkButton", "MiddleMouse");
}

void LLPrefsVoiceLogic::setKey(KEY key, MASK mask)
{
	std::string keystring = LLKeyboard::stringFromKey(key);
	gSavedSettings.setString("PushToTalkButton", keystring);
	
	if(key == ' ')
	{
		// This will cause the select dialog to immediately reopen.
		// Eat the next click event.
		mEatNextSetKeyClick = TRUE;
	}
}

void LLVoiceHotkeySelectDialog::onCancel( void* userdata )
{
	LLVoiceHotkeySelectDialog* self = (LLVoiceHotkeySelectDialog*) userdata;
	self->close(); // destroys this object
	self->mOldFrontmost->setFrontmost(TRUE);
}

BOOL LLVoiceHotkeySelectDialog::handleKey(KEY key, MASK mask, BOOL called_from_parent )
{
	BOOL result = TRUE;
	
	// Suck up all keystokes except CTRL-Q.
	BOOL is_quit = ('Q' == key) && (MASK_CONTROL == mask);
	if(is_quit)
	{
		result = FALSE;
	}
	else
	{
		mParent->setKey(key, mask);
	}

	close(); // destroys this object
	mOldFrontmost->setFrontmost(TRUE);
	
	return result;
}

//---------------------------------------------------------------------------


LLPrefsVoice::LLPrefsVoice()
:	LLPanel("Voice Chat Panel")
{ 
	gUICtrlFactory->buildPanel(this, "panel_preferences_voice.xml");
	mLogic = new LLPrefsVoiceLogic(this);
	childSetAction("device_settings_btn", onClickDeviceSettingsBtn, this);

	// create floater immediately and keep it hidden
	// since it stores preference state for audio devices
	mDeviceSettings = LLFloaterDeviceSettings::getInstance();
}

LLPrefsVoice::~LLPrefsVoice()
{
	delete mLogic;
}

void LLPrefsVoice::draw()
{
	bool enable = !gDisableVoice && gSavedSettings.getBOOL("EnableVoiceChat");
	childSetEnabled("device_settings_btn", enable);

	mLogic->refresh();
	mDeviceSettings->refresh();
	LLPanel::draw();
}

void LLPrefsVoice::apply()
{
	mLogic->apply();
	mDeviceSettings->apply();
}

void LLPrefsVoice::cancel()
{
	mLogic->cancel();
	mDeviceSettings->cancel();
}

//static 
void LLPrefsVoice::onClickDeviceSettingsBtn(void* user_data)
{
	LLPrefsVoice* prefs = (LLPrefsVoice*)user_data;
	prefs->mDeviceSettings->open();
	LLFloater* parent_floater = gFloaterView->getParentFloater(prefs);
	if (parent_floater)
	{
		parent_floater->addDependentFloater(prefs->mDeviceSettings, FALSE);
	}
}
