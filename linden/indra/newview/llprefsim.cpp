/** 
 * @file llprefsim.cpp
 * @author James Cook, Richard Nelson
 * @brief Instant messsage preferences panel
 *
 * Copyright (c) 2003-2007, Linden Research, Inc.
 * 
 * Second Life Viewer Source Code
 * The source code in this file ("Source Code") is provided by Linden Lab
 * to you under the terms of the GNU General Public License, version 2.0
 * ("GPL"), unless you have obtained a separate licensing agreement
 * ("Other License"), formally executed by you and Linden Lab.  Terms of
 * the GPL can be found in doc/GPL-license.txt in this distribution, or
 * online at http://secondlife.com/developers/opensource/gplv2
 * 
 * There are special exceptions to the terms and conditions of the GPL as
 * it is applied to this Source Code. View the full text of the exception
 * in the file doc/FLOSS-exception.txt in this software distribution, or
 * online at http://secondlife.com/developers/opensource/flossexception
 * 
 * By copying, modifying or distributing this software, you acknowledge
 * that you have read and understood your obligations described above,
 * and agree to abide by those obligations.
 * 
 * ALL LINDEN LAB SOURCE CODE IS PROVIDED "AS IS." LINDEN LAB MAKES NO
 * WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
 * COMPLETENESS OR PERFORMANCE.
 */

#include "llviewerprecompiledheaders.h"

#include "llprefsim.h"

#include "llpanel.h"
#include "llcheckboxctrl.h"
#include "llstring.h"
#include "lltexteditor.h"
#include "llavatarconstants.h"
#include "llagent.h"
#include "llviewercontrol.h"
#include "llviewernetwork.h"
#include "llvieweruictrlfactory.h"

#include "lldirpicker.h"

class LLPrefsIMImpl : public LLPanel
{
public:
	LLPrefsIMImpl();
	/*virtual*/ ~LLPrefsIMImpl(){};

	/*virtual*/ BOOL postBuild();

	void apply();
	void refresh();
	void cancel();
	void setPersonalInfo(
		const std::string& visibility,
		bool im_via_email,
		const std::string& email);
	void enableHistory();
	static void onClickLogPath(void* user_data);
	static void onCommitLogging( LLUICtrl* ctrl, void* user_data);

protected:
	BOOL mIMInChatHistory;
	BOOL mLogInstantMessages;
	BOOL mLogChat;
	BOOL mLogShowHistory;
	BOOL mShowTimestamps;
	LLString mIMBusyResponse;
	LLString mLogPath;

	bool mGotPersonalInfo;
	bool mOriginalIMViaEmail;

	// online status info
	bool mOriginalHideOnlineStatus;
	std::string mDirectoryVisibility;
};


LLPrefsIMImpl::LLPrefsIMImpl()
 : LLPanel("IM Prefs Panel")
{
	gUICtrlFactory->buildPanel(this, "panel_preferences_im.xml");
}

void LLPrefsIMImpl::refresh()
{
	mIMInChatHistory   = gSavedSettings.getBOOL("IMInChatHistory");
	mShowTimestamps    = gSavedSettings.getBOOL("IMShowTimestamps");
	mIMBusyResponse    = gSavedPerAccountSettings.getString("BusyModeResponse");
	mLogPath           = gSavedPerAccountSettings.getString("InstantMessageLogPath");
	mLogInstantMessages= gSavedPerAccountSettings.getBOOL("LogInstantMessages");
	mLogChat           = gSavedPerAccountSettings.getBOOL("LogChat");
	mLogShowHistory    = gSavedPerAccountSettings.getBOOL("LogShowHistory");
}

void LLPrefsIMImpl::cancel()
{
	gSavedSettings.setBOOL("IMInChatHistory", mIMInChatHistory);
	gSavedSettings.setBOOL("IMShowTimestamps", mShowTimestamps);
	gSavedPerAccountSettings.setString("BusyModeResponse", mIMBusyResponse);
	gSavedPerAccountSettings.setString("InstantMessageLogPath",mLogPath);
	gSavedPerAccountSettings.setBOOL("LogInstantMessages",mLogInstantMessages);
	gSavedPerAccountSettings.setBOOL("LogChat",mLogChat);
	gSavedPerAccountSettings.setBOOL("LogShowHistory",mLogShowHistory);
}

BOOL LLPrefsIMImpl::postBuild()
{
	requires("online_visibility");
	requires("send_im_to_email");
	if (!checkRequirements())
	{
		return FALSE;
	}

	mGotPersonalInfo = false;
	mOriginalIMViaEmail = false;
	mOriginalHideOnlineStatus = true;
	childSetLabelArg("send_im_to_email", "[EMAIL]", childGetText("log_in_to_change"));

	// Don't enable this until we get personal data
	childDisable("online_visibility");
	childDisable("send_im_to_email");
	childDisable("log_instant_messages");
	childDisable("log_chat");
	childDisable("log_show_history");
	childDisable("log_path_button");
	childDisable("busy_response");

	childSetText("busy_response", childGetText("log_in_to_change"));
	
	refresh(); 

	childSetText("log_path_string", mLogPath);
	childSetValue("log_instant_messages", mLogInstantMessages); 
	childSetValue("log_chat", mLogChat); 
	childSetValue("log_show_history", mLogShowHistory); 
	childSetAction("log_path_button", onClickLogPath, this);
	childSetCommitCallback("log_chat",onCommitLogging,this);
	childSetCommitCallback("log_instant_messages",onCommitLogging,this);
	
	return TRUE;
}

void LLPrefsIMImpl::enableHistory()
{
	
	if (childGetValue("log_instant_messages").asBoolean() || childGetValue("log_chat").asBoolean())
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
	LLTextEditor* busy = LLUICtrlFactory::getTextEditorByName(this, "busy_response");
	LLWString busy_response;
	if (busy) busy_response = busy->getWText(); 
	LLWString::replaceTabsWithSpaces(busy_response, 4);
	LLWString::replaceChar(busy_response, '\n', '^');
	LLWString::replaceChar(busy_response, ' ', '%');
	
	if(mGotPersonalInfo)
	{ 

		gSavedPerAccountSettings.setString("BusyModeResponse", LLString(wstring_to_utf8str(busy_response)));

		gSavedPerAccountSettings.setString("InstantMessageLogPath", childGetText("log_path_string").c_str());
		gSavedPerAccountSettings.setBOOL("LogInstantMessages",childGetValue("log_instant_messages").asBoolean());
		gSavedPerAccountSettings.setBOOL("LogChat",childGetValue("log_chat").asBoolean());
		gSavedPerAccountSettings.setBOOL("LogShowHistory",childGetValue("log_show_history").asBoolean());

		gDirUtilp->setChatLogsDir(gSavedPerAccountSettings.getString("InstantMessageLogPath"));

		gDirUtilp->setPerAccountChatLogsDir(gSavedSettings.getString("FirstName").c_str(), 
											gSavedSettings.getString("LastName").c_str() );
		LLFile::mkdir(gDirUtilp->getPerAccountChatLogsDir().c_str());
		
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
}

void LLPrefsIMImpl::setPersonalInfo(
	const std::string& visibility,
	bool im_via_email,
	const std::string& email)
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
	childEnable("log_instant_messages");
	childEnable("log_chat");
	childEnable("busy_response");
	
	//RN: get wide string so replace char can work (requires fixed-width encoding)
	LLWString busy_response = utf8str_to_wstring( gSavedPerAccountSettings.getString("BusyModeResponse") );
	LLWString::replaceChar(busy_response, '^', '\n');
	LLWString::replaceChar(busy_response, '%', ' ');
	childSetText("busy_response", wstring_to_utf8str(busy_response));

	enableHistory();

	// Truncate the e-mail address if it's too long (to prevent going off
	// the edge of the dialog).
	std::string display_email(email);
	if(display_email.size() > 30)
	{
		display_email.resize(30);
		display_email += "...";
	}

	childSetLabelArg("send_im_to_email", "[EMAIL]", display_email);
}


// static
void LLPrefsIMImpl::onClickLogPath(void* user_data)
{
	LLPrefsIMImpl* self=(LLPrefsIMImpl*)user_data;
	
	LLString proposed_name(self->childGetText("log_path_string"));	 
	
	LLDirPicker& picker = LLDirPicker::instance();
	if (! picker.getDir(&proposed_name ) )
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

void LLPrefsIM::setPersonalInfo(
	const char* visibility,
	bool im_via_email,
	const char* email)
{
	impl.setPersonalInfo(
		ll_safe_string(visibility),
		im_via_email,
		ll_safe_string(email));
}

LLPanel* LLPrefsIM::getPanel()
{
	return &impl;
}
