/** 
 * @file llprefsim.cpp
 * @author James Cook, Richard Nelson
 * @brief Instant messsage preferences panel
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

#include "llprefsim.h"

#include "floaterbusy.h"
#include "llpanel.h"
#include "llcheckboxctrl.h"
#include "llstring.h"
#include "lltexteditor.h"
#include "llavatarconstants.h"
#include "llagent.h"
#include "llviewercontrol.h"
#include "llviewernetwork.h"
#include "lluictrlfactory.h"
#include "llstartup.h"

#include "lldirpicker.h"

#include "hippogridmanager.h"

// [RLVa:KB]
#include "rlvhandler.h"
// [/RLVa:KB]

class LLPrefsIMImpl : public LLPanel
{
public:
	LLPrefsIMImpl();
	/*virtual*/ ~LLPrefsIMImpl(){};

	/*virtual*/ BOOL postBuild();

	void apply();
	void cancel();
	void setPersonalInfo(const std::string& visibility, bool im_via_email, const std::string& email);
	void preparePerAccountPrefs(bool enable);
	void enableHistory();
	
	static void onClickLogPath(void* user_data);
	static void onCommitLogging(LLUICtrl* ctrl, void* user_data);

	static void onClickBusyAdvanced(void* user_data);

protected:
 
	bool mGotPersonalInfo;
	bool mGotPerAccountSettings;
	bool mOriginalIMViaEmail;

	bool mOriginalHideOnlineStatus;
	std::string mDirectoryVisibility;
};


LLPrefsIMImpl::LLPrefsIMImpl()
	: LLPanel(std::string("IM Prefs Panel")),
	  mGotPersonalInfo(false),
	  mGotPerAccountSettings(false),
	  mOriginalIMViaEmail(false),
	  mOriginalHideOnlineStatus(false)
{
	LLUICtrlFactory::getInstance()->buildPanel(this, "panel_preferences_im.xml");
}

void LLPrefsIMImpl::cancel()
{
}

BOOL LLPrefsIMImpl::postBuild()
{
	requires("online_visibility");
	requires("send_im_to_email");
	if (!checkRequirements())
	{
		return FALSE;
	}

	childSetLabelArg("send_im_to_email", "[EMAIL]", getString("log_in_to_change"));

	// Don't enable these until we get personal data
	childSetEnabled("online_visibility", false);
	childSetEnabled("send_im_to_email", false);

	// These are safe to enable
	childSetEnabled("include_im_in_chat_console", true);
	childSetEnabled("include_im_in_chat_history", true);
	childSetEnabled("show_timestamps_check", true);
	childSetEnabled("friends_online_notify_checkbox", true);
	childSetEnabled("vertical-imtabs-toggle", true);
	childSetValue("include_im_in_chat_console", gSavedSettings.getBOOL("IMInChatConsole"));
	childSetValue("include_im_in_chat_history", gSavedSettings.getBOOL("IMInChatHistory"));
	childSetValue("show_timestamps_check", gSavedSettings.getBOOL("IMShowTimestamps"));
	childSetValue("friends_online_notify_checkbox", gSavedSettings.getBOOL("ChatOnlineNotification"));
	childSetValue("vertical-imtabs-toggle", gSavedSettings.getBOOL("VerticalIMTabs"));

	childSetAction("log_path_button", onClickLogPath, this);
	childSetCommitCallback("log_chat",onCommitLogging,this);
	childSetCommitCallback("log_instant_messages",onCommitLogging,this);

	childSetAction("busy_adv_btn", onClickBusyAdvanced, this);
	
	preparePerAccountPrefs(LLStartUp::isLoggedIn());

	return TRUE;
}

void LLPrefsIMImpl::enableHistory()
{
	
	if  (mGotPerAccountSettings &&
		(childGetValue("log_instant_messages").asBoolean() || childGetValue("log_chat").asBoolean()))
	{
		childEnable("log_show_history");
		childEnable("log_path_button");
	}
	else
	{
		childDisable("log_show_history");
		childDisable("log_path_button");
	}
}

void LLPrefsIMImpl::apply()
{
	if (mGotPerAccountSettings)
	{ 
		LLTextEditor* busy = getChild<LLTextEditor>("busy_response");
		LLWString busy_response;
		if (busy) busy_response = busy->getWText(); 
		LLWStringUtil::replaceTabsWithSpaces(busy_response, 4);
		LLWStringUtil::replaceChar(busy_response, '\n', '^');
		LLWStringUtil::replaceChar(busy_response, ' ', '%');
		gSavedPerAccountSettings.setString("BusyModeResponse", std::string(wstring_to_utf8str(busy_response)));

		gSavedPerAccountSettings.setString("InstantMessageLogPath", childGetText("log_path_string"));
		gSavedPerAccountSettings.setBOOL("LogInstantMessages",childGetValue("log_instant_messages").asBoolean());
		gSavedPerAccountSettings.setBOOL("LogChat",childGetValue("log_chat").asBoolean());
		gSavedPerAccountSettings.setBOOL("LogShowHistory",childGetValue("log_show_history").asBoolean());
		gSavedPerAccountSettings.setBOOL("IMLogTimestamp",childGetValue("log_instant_messages_timestamp").asBoolean());
		gSavedPerAccountSettings.setBOOL("LogChatTimestamp",childGetValue("log_chat_timestamp").asBoolean());
		gSavedPerAccountSettings.setBOOL("LogChatIM",childGetValue("log_chat_IM").asBoolean());
		gSavedPerAccountSettings.setBOOL("LogTimestampDate",childGetValue("log_date_timestamp").asBoolean());

		gDirUtilp->setChatLogsDir(gSavedPerAccountSettings.getString("InstantMessageLogPath"));

		if (gSavedSettings.getBOOL("UseLegacyChatLogsFolder"))
		{
			gDirUtilp->setPerAccountChatLogsDir(LLStringUtil::null, 
				gHippoGridManager->getCurrentGrid()->getFirstName(), gHippoGridManager->getCurrentGrid()->getLastName() );
		}
		else
		{
			gDirUtilp->setPerAccountChatLogsDir(gHippoGridManager->getCurrentGridNick(), 
				gHippoGridManager->getCurrentGrid()->getFirstName(), gHippoGridManager->getCurrentGrid()->getLastName() );
		}
		LLFile::mkdir(gDirUtilp->getPerAccountChatLogsDir());
	}

	if (mGotPersonalInfo)
	{
		bool new_im_via_email = childGetValue("send_im_to_email").asBoolean();
		bool new_hide_online = childGetValue("online_visibility").asBoolean();		

		if((new_im_via_email != mOriginalIMViaEmail)
		   ||(new_hide_online != mOriginalHideOnlineStatus))
		{
			LLMessageSystem* msg = gMessageSystem;
			msg->newMessageFast(_PREHASH_UpdateUserInfo);
			msg->nextBlockFast(_PREHASH_AgentData);
			msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
			msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
			msg->nextBlockFast(_PREHASH_UserData);
			msg->addBOOLFast(_PREHASH_IMViaEMail, new_im_via_email);
			// This hack is because we are representing several different 	 
			// possible strings with a single checkbox. Since most users 	 
			// can only select between 2 values, we represent it as a 	 
			// checkbox. This breaks down a little bit for liaisons, but 	 
			// works out in the end. 	 
			if(new_hide_online != mOriginalHideOnlineStatus) 	 
			{ 	 
				if(new_hide_online) mDirectoryVisibility = VISIBILITY_HIDDEN;
				else mDirectoryVisibility = VISIBILITY_DEFAULT;
				//Update showonline value, otherwise multiple applys won't work
				mOriginalHideOnlineStatus = new_hide_online;
			} 	 
			msg->addString("DirectoryVisibility", mDirectoryVisibility);
			gAgent.sendReliableMessage();
		}
	}

	gSavedSettings.setBOOL("VerticalIMTabs", childGetValue("vertical-imtabs-toggle").asBoolean());
	gSavedSettings.setBOOL("IMInChatConsole", childGetValue("include_im_in_chat_console").asBoolean());
	gSavedSettings.setBOOL("IMInChatHistory", childGetValue("include_im_in_chat_history").asBoolean());
	gSavedSettings.setBOOL("IMShowTimestamps", childGetValue("show_timestamps_check").asBoolean());
	gSavedSettings.setBOOL("ChatOnlineNotification", childGetValue("friends_online_notify_checkbox").asBoolean());
}

// Enable and set the value of settings recieved from the sim in AgentInfoReply
void LLPrefsIMImpl::setPersonalInfo(const std::string& visibility, bool im_via_email, const std::string& email)
{
	mGotPersonalInfo = true;
	mOriginalIMViaEmail = im_via_email;
	mDirectoryVisibility = visibility;
	
	if(visibility == VISIBILITY_DEFAULT)
	{
		mOriginalHideOnlineStatus = false;
		childEnable("online_visibility"); 	 
	}
	else if(visibility == VISIBILITY_HIDDEN)
	{
		mOriginalHideOnlineStatus = true;
		childEnable("online_visibility"); 	 
	}
	else
	{
		mOriginalHideOnlineStatus = true;
	}

	childSetValue("online_visibility", mOriginalHideOnlineStatus); 	 
	childSetLabelArg("online_visibility", "[DIR_VIS]", mDirectoryVisibility);
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

// Enable and set the value of settings that need an account name
void LLPrefsIMImpl::preparePerAccountPrefs(bool enable)
{
	if (mGotPerAccountSettings && enable)
	{
		return; // prevent overwriting unsaved changes.
	}
	mGotPerAccountSettings = enable;

	childSetEnabled("log_chat", enable);
	childSetEnabled("log_chat_timestamp", enable);
	childSetEnabled("log_chat_IM", enable);
	childSetEnabled("log_instant_messages_timestamp", enable);
	childSetEnabled("log_instant_messages", enable);
	childSetEnabled("log_date_timestamp", enable);

	childSetValue("log_chat", gSavedPerAccountSettings.getBOOL("LogChat"));
	childSetValue("log_chat_timestamp", gSavedPerAccountSettings.getBOOL("LogChatTimestamp"));
	childSetValue("log_chat_IM", gSavedPerAccountSettings.getBOOL("LogChatIM"));
	childSetValue("log_instant_messages_timestamp", gSavedPerAccountSettings.getBOOL("IMLogTimestamp"));
	childSetValue("log_instant_messages", gSavedPerAccountSettings.getBOOL("LogInstantMessages"));
	childSetValue("log_date_timestamp", gSavedPerAccountSettings.getBOOL("LogTimestampDate"));

	childSetValue("log_show_history", gSavedPerAccountSettings.getBOOL("LogShowHistory"));
	enableHistory();
	childSetText("log_path_string", gSavedPerAccountSettings.getString("InstantMessageLogPath"));

	childSetEnabled("busy_response", enable);
	if (enable)
	{
		//RN: get wide string so replace char can work (requires fixed-width encoding)
		LLWString busy_response = utf8str_to_wstring( gSavedPerAccountSettings.getString("BusyModeResponse") );
		LLWStringUtil::replaceChar(busy_response, '^', '\n');
		LLWStringUtil::replaceChar(busy_response, '%', ' ');
		childSetText("busy_response", wstring_to_utf8str(busy_response));
	}
	else
	{
		childSetText("busy_response", getString("log_in_to_change"));
	}
// [RLVa:KB] - Checked: 2009-07-10 (RLVa-1.0.0g)
	if (gRlvHandler.hasBehaviour(RLV_BHVR_SENDIM))
	{
		childDisable("busy_response");
	}
// [/RLVa:KB]

	childSetEnabled("busy_adv_btn", enable);
}

// static
void LLPrefsIMImpl::onClickBusyAdvanced(void* user_data)
{
	FloaterBusy::getInstance()->open();
	FloaterBusy::getInstance()->center();
}

// static
void LLPrefsIMImpl::onClickLogPath(void* user_data)
{
	LLPrefsIMImpl* self=(LLPrefsIMImpl*)user_data;
	
	std::string proposed_name(self->childGetText("log_path_string"));	 
	
	LLDirPicker& picker = LLDirPicker::instance();
	if (!picker.getDir(&proposed_name ) )
	{
		return; //Canceled!
	}
	
	self->childSetText("log_path_string", picker.getDirName());	 
}


// static
void LLPrefsIMImpl::onCommitLogging( LLUICtrl* ctrl, void* user_data)
{
	LLPrefsIMImpl* self=(LLPrefsIMImpl*)user_data;
	self->enableHistory();
}


//---------------------------------------------------------------------------

LLPrefsIM::LLPrefsIM()
:	impl( * new LLPrefsIMImpl() )
{ }

LLPrefsIM::~LLPrefsIM()
{
	delete &impl;
}

void LLPrefsIM::apply()
{
	impl.apply();
}

void LLPrefsIM::cancel()
{
	impl.cancel();
}

void LLPrefsIM::setPersonalInfo(const std::string& visibility, bool im_via_email, const std::string& email)
{
	impl.setPersonalInfo(visibility, im_via_email, email);
}

void LLPrefsIM::preparePerAccountPrefs(bool enable)
{
	impl.preparePerAccountPrefs(enable);
}

LLPanel* LLPrefsIM::getPanel()
{
	return &impl;
}
