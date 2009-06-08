/** 
 * @file llprefsvoice.cpp
 * @author Richard Nelson
 * @brief Voice chat preferences panel
 *
 * $LicenseInfo:firstyear=2003&license=viewergpl$
 * 
 * Copyright (c) 2003-2009, Linden Research, Inc.
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
#include "llfloatervoicedevicesettings.h"
#include "llfocusmgr.h"
#include "llkeyboard.h"
#include "llmodaldialog.h"
#include "llviewercontrol.h"
#include "lluictrlfactory.h"


class LLVoiceSetKeyDialog : public LLModalDialog
{
public:
	LLVoiceSetKeyDialog(LLPrefsVoice* parent);
	~LLVoiceSetKeyDialog();

	BOOL handleKeyHere(KEY key, MASK mask);

	static void onCancel(void* user_data);

private:
	LLPrefsVoice* mParent;
};

LLVoiceSetKeyDialog::LLVoiceSetKeyDialog(LLPrefsVoice* parent)
	: LLModalDialog(LLStringUtil::null, 240, 100), mParent(parent)
{
	LLUICtrlFactory::getInstance()->buildFloater(this, "floater_select_key.xml");
	childSetAction("Cancel", onCancel, this);
	childSetFocus("Cancel");

	gFocusMgr.setKeystrokesOnly(TRUE);
}

LLVoiceSetKeyDialog::~LLVoiceSetKeyDialog()
{
}

BOOL LLVoiceSetKeyDialog::handleKeyHere(KEY key, MASK mask)
{
	BOOL result = TRUE;
	
	if(key == 'Q' && mask == MASK_CONTROL)
	{
		result = FALSE;
	}
	else
	{
		mParent->setKey(key);
	}

	close();
	return result;
}

//static
void LLVoiceSetKeyDialog::onCancel(void* user_data)
{
	LLVoiceSetKeyDialog* self = (LLVoiceSetKeyDialog*)user_data;
	self->close();
}

//--------------------------------------------------------------------
//LLPrefsVoice
LLPrefsVoice::LLPrefsVoice()
	:	LLPanel(std::string("Voice Chat Panel"))
{ 
	LLUICtrlFactory::getInstance()->buildPanel(this, "panel_preferences_voice.xml");
}

LLPrefsVoice::~LLPrefsVoice()
{
}

BOOL LLPrefsVoice::postBuild()
{
	childSetCommitCallback("enable_voice_check", onCommitEnableVoiceChat, this);
	childSetAction("set_voice_hotkey_button", onClickSetKey, this);
	childSetAction("set_voice_middlemouse_button", onClickSetMiddleMouse, this);
	childSetAction("device_settings_btn", onClickVoiceDeviceSettings, NULL);

	BOOL voice_disabled = gSavedSettings.getBOOL("CmdLineDisableVoice");
	childSetVisible("voice_unavailable", voice_disabled);
	childSetVisible("enable_voice_check", !voice_disabled);
	childSetEnabled("enable_voice_check", !voice_disabled);

	bool enable = !voice_disabled && gSavedSettings.getBOOL("EnableVoiceChat");
	childSetValue("enable_voice_check", enable);
	onCommitEnableVoiceChat(getChild<LLCheckBoxCtrl>("enable_voice_check"), this);

	childSetValue("modifier_combo", gSavedSettings.getString("PushToTalkButton"));
	childSetValue("voice_call_friends_only_check", gSavedSettings.getBOOL("VoiceCallsFriendsOnly"));
	childSetValue("push_to_talk_toggle_check", gSavedSettings.getBOOL("PushToTalkToggle"));
	childSetValue("ear_location", gSavedSettings.getS32("VoiceEarLocation"));

	return TRUE;
}

void LLPrefsVoice::apply()
{
	gSavedSettings.setBOOL("EnableVoiceChat", childGetValue("enable_voice_check"));

	gSavedSettings.setString("PushToTalkButton", childGetValue("modifier_combo"));
	gSavedSettings.setBOOL("VoiceCallsFriendsOnly", childGetValue("voice_call_friends_only_check"));
	gSavedSettings.setBOOL("PushToTalkToggle", childGetValue("push_to_talk_toggle_check"));
	gSavedSettings.setS32("VoiceEarLocation", childGetValue("ear_location"));
	
	LLFloaterVoiceDeviceSettings* voice_device_settings = LLFloaterVoiceDeviceSettings::getInstance();
	if(voice_device_settings)
	{
		voice_device_settings->apply();
	}
}

void LLPrefsVoice::cancel()
{
	LLFloaterVoiceDeviceSettings* voice_device_settings = LLFloaterVoiceDeviceSettings::getInstance();
	if(voice_device_settings)
	{
		voice_device_settings->cancel();
	}
}

void LLPrefsVoice::setKey(KEY key)
{
	childSetValue("modifier_combo", LLKeyboard::stringFromKey(key));
}

//static
void LLPrefsVoice::onCommitEnableVoiceChat(LLUICtrl* ctrl, void* user_data)
{
	LLPrefsVoice* self = (LLPrefsVoice*)user_data;
	LLCheckBoxCtrl* enable_voice_chat = (LLCheckBoxCtrl*)ctrl;

	bool enable = enable_voice_chat->getValue();

	self->childSetEnabled("modifier_combo", enable);
	self->childSetEnabled("friends_only_check", enable);
	self->childSetEnabled("push_to_talk_label", enable);
	self->childSetEnabled("voice_call_friends_only_check", enable);
	self->childSetEnabled("push_to_talk_toggle_check", enable);
	self->childSetEnabled("ear_location", enable);
	self->childSetEnabled("set_voice_hotkey_button", enable);
	self->childSetEnabled("set_voice_middlemouse_button", enable);
	self->childSetEnabled("device_settings_btn", enable);
}

//static
void LLPrefsVoice::onClickSetKey(void* user_data)
{
	LLPrefsVoice* self = (LLPrefsVoice*)user_data;
	LLVoiceSetKeyDialog* dialog = new LLVoiceSetKeyDialog(self);
	dialog->startModal();
}

//static
void LLPrefsVoice::onClickSetMiddleMouse(void* user_data)
{
	LLPrefsVoice* self = (LLPrefsVoice*)user_data;
	self->childSetValue("modifier_combo", "MiddleMouse");
}

//static
void LLPrefsVoice::onClickVoiceDeviceSettings(void* user_data)
{
	LLPrefsVoice* voice_prefs = (LLPrefsVoice*)user_data;
	LLFloaterVoiceDeviceSettings* device_settings_floater = LLFloaterVoiceDeviceSettings::showInstance();
	LLFloater* parent_floater = gFloaterView->getParentFloater(voice_prefs);
	if(parent_floater)
	{
		parent_floater->addDependentFloater(device_settings_floater, FALSE);
	}
}

