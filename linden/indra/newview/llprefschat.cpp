/** 
 * @file llprefschat.cpp
 * @author James Cook, Richard Nelson
 * @brief Chat preferences panel
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

#include "llcombobox.h"
#include "llpanel.h"
#include "llstylemap.h"
#include "lluictrlfactory.h"

#include "floatercommandline.h"
#include "llagent.h"
#include "llchatbar.h"
#include "llfloaterchat.h"
#include "lgghunspell_wrapper.h"
#include "lggautocorrectfloater.h"
#include "llprefschat.h"
#include "llviewercontrol.h"
#include "llviewermessage.h"

class LLPrefsChatImpl : public LLPanel
{
public:
	LLPrefsChatImpl();
	/*virtual*/ ~LLPrefsChatImpl(){};

	/*virtual*/ BOOL postBuild();

	void apply();
	void cancel();
	void refresh();
	void setPersonalInfo(bool im_via_email, const std::string& email);
	void initHelpBtn(const std::string& name, const std::string& xml_alert);

	static void onAutoCorrectButton(void * data);
	static void onClickHelp(void* data);
	static void onClickCommandLine(void* data);
	static void onCommitCheckBox(LLUICtrl* ctrl, void* user_data);
	static void onSpellAdd(void* data);
	static void onSpellRemove(void* data);
	static void onSpellGetMore(void* data);
	static void onSpellEditCustom(void* data);
	static void onSpellBaseComboBoxCommit(LLUICtrl* ctrl, void* userdata);

protected:
	bool mOriginalIMViaEmail;
	bool mGotPersonalInfo;

private:
	bool mChatChannel;
};

LLPrefsChatImpl::LLPrefsChatImpl()
	:	LLPanel(std::string("Chat Panel")),
	mChatChannel(false),
	mOriginalIMViaEmail(false),
	mGotPersonalInfo(false)
{
	LLUICtrlFactory::getInstance()->buildPanel(this, "panel_preferences_chat.xml");
}

BOOL LLPrefsChatImpl::postBuild()
{
	requires("send_im_to_email");
	childSetLabelArg("send_im_to_email", "[EMAIL]", getString("log_in_to_change"));

	// Don't enable these until we get personal data
	childSetEnabled("send_im_to_email", false);

	getChild<LLComboBox>("chat_font_size")->setValue(gSavedSettings.getS32("ChatFontSize"));
	childSetValue("fade_chat_time", gSavedSettings.getF32("ChatPersistTime"));
	childSetValue("max_chat_count", gSavedSettings.getS32("ConsoleMaxLines"));

	childSetValue("bubble_text_chat", gSavedSettings.getBOOL("UseChatBubbles"));
	childSetValue("local_bubble_text_chat", gSavedSettings.getBOOL("UseLocalChatWithBubbles"));
	childSetValue("chat_full_width_check", gSavedSettings.getBOOL("ChatFullWidth"));
	childSetValue("close_chat_on_return_check", gSavedSettings.getBOOL("CloseChatOnReturn"));
	childSetValue("play_typing_animation", gSavedSettings.getBOOL("PlayTypingAnim"));
	childSetValue("toggle_channel_control", gSavedSettings.getBOOL("ChatChannelSelect"));
	childSetValue("console_opacity", gSavedSettings.getF32("ConsoleBackgroundOpacity"));
	childSetValue("bubble_chat_opacity", gSavedSettings.getF32("ChatBubbleOpacity"));
	childSetValue("translate_language_combobox", 	gSavedSettings.getString("TranslateLanguage"));
	childSetValue("translate_chat", 	gSavedSettings.getBOOL("TranslateChat"));

	mChatChannel = gSavedSettings.getBOOL("ChatChannelSelect");

	childSetValue("command_line_check", gSavedSettings.getBOOL("CmdLineChatbarEnabled"));
	childSetCommitCallback("command_line_check", onCommitCheckBox, this);
	childSetAction("command_line_btn", onClickCommandLine, this);

	getChild<LLComboBox>("EmeraldSpellBase")->setCommitCallback(onSpellBaseComboBoxCommit);
	getChild<LLButton>("EmSpell_EditCustom")->setClickedCallback(onSpellEditCustom, this);
	getChild<LLButton>("EmSpell_GetMore")->setClickedCallback(onSpellGetMore, this);
	getChild<LLButton>("EmSpell_Add")->setClickedCallback(onSpellAdd, this);
	getChild<LLButton>("EmSpell_Remove")->setClickedCallback(onSpellRemove, this);

	getChild<LLButton>("ac_button")->setClickedCallback(onAutoCorrectButton, this);
	initHelpBtn("EmeraldHelp_SpellCheck", "EmeraldHelp_SpellCheck");

	childSetValue("include_im_in_chat_console", gSavedSettings.getBOOL("IMInChatConsole"));
	childSetValue("include_im_in_chat_history", gSavedSettings.getBOOL("IMInChatHistory"));
	childSetValue("vertical-imtabs-toggle", gSavedSettings.getBOOL("VerticalIMTabs"));

	refresh();

	return TRUE;
}

void LLPrefsChatImpl::refresh()
{
		if (childGetValue("command_line_check").asBoolean())
	{
		childEnable("command_line_btn");
	}
	else
	{
		childDisable("command_line_btn");
	}

	LLComboBox* comboBox = getChild<LLComboBox>("EmeraldSpellBase");
	if (comboBox != NULL) 
	{
		comboBox->removeall();
		std::vector<std::string> names = glggHunSpell->getDicts();
		for (int i = 0; i < (int)names.size(); i++) 
		{
			comboBox->add(names[i]);
		}
		comboBox->setSimple(gSavedSettings.getString("EmeraldSpellBase"));
	}
	comboBox = getChild<LLComboBox>("EmSpell_Avail");
	if (comboBox != NULL) 
	{
		LLSD selected = comboBox->getSelectedValue();
		comboBox->removeall();
		std::vector<std::string> names = glggHunSpell->getAvailDicts();
		for (int i = 0; i < (int)names.size(); i++) 
		{
			comboBox->add(names[i]);
		}
		comboBox->selectByValue(selected);
	}
	comboBox = getChild<LLComboBox>("EmSpell_Installed");
	if (comboBox != NULL) 
	{
		LLSD selected = comboBox->getSelectedValue();
		comboBox->removeall();
		std::vector<std::string> names = glggHunSpell->getInstalledDicts();
		for (int i = 0; i < (int)names.size(); i++) 
		{
			comboBox->add(names[i]);
		}
		comboBox->selectByValue(selected);
	}
}

void LLPrefsChatImpl::cancel()
{
}

void LLPrefsChatImpl::apply()
{
	gSavedSettings.setS32("ChatFontSize", getChild<LLComboBox>("chat_font_size")->getValue().asInteger());
	gSavedSettings.setF32("ChatPersistTime", childGetValue("fade_chat_time").asReal());
	gSavedSettings.setS32("ConsoleMaxLines", childGetValue("max_chat_count"));

	gSavedSettings.setBOOL("UseChatBubbles", childGetValue("bubble_text_chat"));
	gSavedSettings.setBOOL("UseLocalChatWithBubbles", childGetValue("local_bubble_text_chat"));
	gSavedSettings.setBOOL("ChatFullWidth", childGetValue("chat_full_width_check"));
	gSavedSettings.setBOOL("CloseChatOnReturn", childGetValue("close_chat_on_return_check"));
	gSavedSettings.setBOOL("PlayTypingAnim", childGetValue("play_typing_animation"));

	gSavedSettings.setF32("ConsoleBackgroundOpacity", childGetValue("console_opacity").asReal());
	gSavedSettings.setF32("ChatBubbleOpacity", childGetValue("bubble_chat_opacity").asReal());

	gSavedSettings.setString("TranslateLanguage", childGetValue("translate_language_combobox"));
	gSavedSettings.setBOOL("TranslateChat", childGetValue("translate_chat"));

	bool chan_check = childGetValue("toggle_channel_control");
	gSavedSettings.setBOOL("ChatChannelSelect", chan_check);
	if (mChatChannel != chan_check)
	{
		if (gChatBar)
		{
			gChatBar->toggleChannelControl();
			//LLFloaterChat::toggleHistoryChannelControl(); temporarily disable until working
		}
		mChatChannel = chan_check;
	}

	gSavedSettings.setBOOL("CmdLineChatbarEnabled", childGetValue("command_line_check").asBoolean());

	gSavedSettings.setBOOL("VerticalIMTabs", childGetValue("vertical-imtabs-toggle").asBoolean());
	gSavedSettings.setBOOL("IMInChatConsole", childGetValue("include_im_in_chat_console").asBoolean());
	gSavedSettings.setBOOL("IMInChatHistory", childGetValue("include_im_in_chat_history").asBoolean());

	if (mGotPersonalInfo)
	{
		bool new_im_via_email = childGetValue("send_im_to_email").asBoolean();	

		if (new_im_via_email != mOriginalIMViaEmail)
		{
			LLMessageSystem* msg = gMessageSystem;
			msg->newMessageFast(_PREHASH_UpdateUserInfo);
			msg->nextBlockFast(_PREHASH_AgentData);
			msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
			msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
			msg->nextBlockFast(_PREHASH_UserData);
			msg->addBOOLFast(_PREHASH_IMViaEMail, new_im_via_email);
			gAgent.sendReliableMessage();
		}
	}

	// Spell checking prefs aren't here because Emerald never supported the cancel button. We should fix this -- MC
}

// Enable and set the value of settings recieved from the sim in AgentInfoReply
void LLPrefsChatImpl::setPersonalInfo(bool im_via_email, const std::string& email)
{
	mGotPersonalInfo = true;
	mOriginalIMViaEmail = im_via_email;

	childEnable("send_im_to_email");
	childSetValue("send_im_to_email", im_via_email);

	// Truncate the e-mail address if it's too long (to prevent going off
	// the edge of the dialog).
	std::string display_email(email);
	if(display_email.size() > 30)
	{
		display_email.resize(30);
		display_email += "...";
	}
	else if (display_email.empty())
	{
		display_email = getString("default_email_used");
	}
	childSetLabelArg("send_im_to_email", "[EMAIL]", display_email);
}

// static
void LLPrefsChatImpl::onSpellAdd(void* data)
{
	LLPrefsChatImpl* panel = (LLPrefsChatImpl*)data;
	if (panel)
	{
		glggHunSpell->addButton(panel->childGetValue("EmSpell_Avail").asString());
		panel->refresh();
	}
}

// static
void LLPrefsChatImpl::onSpellRemove(void* data)
{
	LLPrefsChatImpl* panel = (LLPrefsChatImpl*)data;
	if (panel)
	{
		glggHunSpell->removeButton(panel->childGetValue("EmSpell_Installed").asString());
		panel->refresh();
	}
}

// static
void LLPrefsChatImpl::onSpellGetMore(void* data)
{
	glggHunSpell->getMoreButton(data);
}

// static
void LLPrefsChatImpl::onSpellEditCustom(void* data)
{
	glggHunSpell->editCustomButton();
}

// static
void LLPrefsChatImpl::onSpellBaseComboBoxCommit(LLUICtrl* ctrl, void* userdata)
{
	LLComboBox* box = (LLComboBox*)ctrl;
	if (box)
	{
		glggHunSpell->newDictSelection(box->getValue().asString());
	}
	LLPrefsChatImpl* panel = (LLPrefsChatImpl*)userdata;
	if (panel)
	{
		panel->refresh();
	}
}

// static
void LLPrefsChatImpl::onAutoCorrectButton(void * data)
{
	lggAutoCorrectFloaterStart::show(TRUE,data);
}

// static
void LLPrefsChatImpl::onClickCommandLine(void* data)
{
	FloaterCommandLine::getInstance()->open();
	FloaterCommandLine::getInstance()->center();
}

//static
void LLPrefsChatImpl::onCommitCheckBox(LLUICtrl* ctrl, void* user_data)
{
	LLPrefsChatImpl* self = (LLPrefsChatImpl*)user_data;
	if (self)
	{
		self->refresh();
	}
}

void LLPrefsChatImpl::initHelpBtn(const std::string& name, const std::string& xml_alert)
{
	childSetAction(name, onClickHelp, new std::string(xml_alert));
}

// static
void LLPrefsChatImpl::onClickHelp(void* data)
{
	std::string* xml_alert = (std::string*)data;
	LLNotifications::instance().add(*xml_alert);
}

//---------------------------------------------------------------------------

LLPrefsChat::LLPrefsChat()
:	impl( * new LLPrefsChatImpl() )
{ }

LLPrefsChat::~LLPrefsChat()
{
	delete &impl;
}

void LLPrefsChat::apply()
{
	impl.apply();
	LLStyleMap::instance().update();
}

void LLPrefsChat::cancel()
{
	impl.cancel();
}

void LLPrefsChat::setPersonalInfo(bool im_via_email, const std::string& email)
{
	impl.setPersonalInfo(im_via_email, email);
}

LLPanel* LLPrefsChat::getPanel()
{
	return &impl;
}
