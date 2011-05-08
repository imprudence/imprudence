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

#include "llchatbar.h"
#include "llfloaterchat.h"
#include "llprefschat.h"
#include "llviewercontrol.h"
#include "lluictrlfactory.h"
#include "llradiogroup.h"
#include "llstylemap.h"

class LLPrefsChatImpl : public LLPanel
{
public:
	LLPrefsChatImpl();
	/*virtual*/ ~LLPrefsChatImpl(){};

	void apply();
	void cancel();

private:
	void refreshValues();
	S32	mChatSize;
	F32	mChatPersist;
	S32	mChatMaxLines;
	BOOL mChatFullWidth;
	BOOL mCloseChatOnReturn;
	BOOL mArrowKeysMoveAvatar;
	BOOL mShowTimestamps;
	BOOL mPlayTypingAnim;
	BOOL mChatBubbles;
	BOOL mLocalChatBubbles;
	BOOL mScriptErrorAsChat;
	BOOL mChatChannel;
	F32	mConsoleOpacity;
	F32	mBubbleOpacity;
	std::string mTranslateLanguage;
	BOOL mTranslateChat;
};

LLPrefsChatImpl::LLPrefsChatImpl()
	:	LLPanel(std::string("Chat Panel"))
{
	refreshValues(); // initialize member data from saved settings

	LLUICtrlFactory::getInstance()->buildPanel(this, "panel_preferences_chat.xml");

	getChild<LLRadioGroup>("chat_font_size")->setSelectedIndex(gSavedSettings.getS32("ChatFontSize"));
	childSetValue("fade_chat_time", gSavedSettings.getF32("ChatPersistTime"));
	childSetValue("max_chat_count", gSavedSettings.getS32("ConsoleMaxLines"));

	childSetValue("arrow_keys_move_avatar_check", gSavedSettings.getBOOL("ArrowKeysMoveAvatar"));
	childSetValue("show_timestamps_check", gSavedSettings.getBOOL("ChatShowTimestamps"));
	childSetValue("script_errors_as_chat", gSavedSettings.getBOOL("ScriptErrorsAsChat"));
 
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
}

void LLPrefsChatImpl::refreshValues()
{
	//set values
	mChatSize = gSavedSettings.getS32("ChatFontSize");
	mChatPersist = gSavedSettings.getF32("ChatPersistTime");
	mChatMaxLines = gSavedSettings.getS32("ConsoleMaxLines");
	mArrowKeysMoveAvatar = gSavedSettings.getBOOL("ArrowKeysMoveAvatar");
	mShowTimestamps = gSavedSettings.getBOOL("ChatShowTimestamps");
	mScriptErrorAsChat = gSavedSettings.getBOOL("ScriptErrorsAsChat");
	mChatBubbles = gSavedSettings.getBOOL("UseChatBubbles");
	mLocalChatBubbles = gSavedSettings.getBOOL("UseLocalChatWithBubbles");
	mChatFullWidth = gSavedSettings.getBOOL("ChatFullWidth");
	mCloseChatOnReturn = gSavedSettings.getBOOL("CloseChatOnReturn");
	mPlayTypingAnim = gSavedSettings.getBOOL("PlayTypingAnim"); 
	mChatChannel = gSavedSettings.getBOOL("ChatChannelSelect");
	mConsoleOpacity = gSavedSettings.getF32("ConsoleBackgroundOpacity");
	mTranslateLanguage = gSavedSettings.getString("TranslateLanguage");
	mTranslateChat = gSavedSettings.getBOOL("TranslateChat");
	static F32* sChatBubbleOpacity = rebind_llcontrol<F32>("ChatBubbleOpacity", &gSavedSettings, true);
	mBubbleOpacity = *sChatBubbleOpacity;
}

void LLPrefsChatImpl::cancel()
{
	gSavedSettings.setS32("ChatFontSize", mChatSize);
	gSavedSettings.setF32("ChatPersistTime", mChatPersist);
	gSavedSettings.setS32("ConsoleMaxLines", mChatMaxLines);
	gSavedSettings.setBOOL("ArrowKeysMoveAvatar", mArrowKeysMoveAvatar);
	gSavedSettings.setBOOL("ChatShowTimestamps", mShowTimestamps);
	gSavedSettings.setBOOL("ScriptErrorsAsChat", mScriptErrorAsChat);
	gSavedSettings.setBOOL("UseChatBubbles", mChatBubbles);
	gSavedSettings.setBOOL("UseLocalChatWithBubbles", mLocalChatBubbles);
	gSavedSettings.setBOOL("ChatFullWidth", mChatFullWidth);
	gSavedSettings.setBOOL("CloseChatOnReturn", mCloseChatOnReturn);
	gSavedSettings.setBOOL("PlayTypingAnim", mPlayTypingAnim); 
	gSavedSettings.setBOOL("ChatChannelSelect", mChatChannel); 
	gSavedSettings.setF32("ConsoleBackgroundOpacity", mConsoleOpacity);
	gSavedSettings.setF32("ChatBubbleOpacity", mBubbleOpacity);	
	gSavedSettings.setString("TranslateLanguage", mTranslateLanguage);	
	gSavedSettings.setBOOL("TranslateChat", mTranslateChat);
}

void LLPrefsChatImpl::apply()
{
	gSavedSettings.setS32("ChatFontSize", getChild<LLRadioGroup>("chat_font_size")->getSelectedIndex());
	gSavedSettings.setF32("ChatPersistTime", childGetValue("fade_chat_time").asReal());
	gSavedSettings.setS32("ConsoleMaxLines", childGetValue("max_chat_count"));

	gSavedSettings.setBOOL("ArrowKeysMoveAvatar", childGetValue("arrow_keys_move_avatar_check"));
	gSavedSettings.setBOOL("ChatShowTimestamps", childGetValue("show_timestamps_check"));
	gSavedSettings.setBOOL("ScriptErrorsAsChat", childGetValue("script_errors_as_chat"));
	gSavedSettings.setBOOL("UseChatBubbles", childGetValue("bubble_text_chat"));
	gSavedSettings.setBOOL("UseLocalChatWithBubbles", childGetValue("local_bubble_text_chat"));
	gSavedSettings.setBOOL("ChatFullWidth", childGetValue("chat_full_width_check"));
	gSavedSettings.setBOOL("CloseChatOnReturn", childGetValue("close_chat_on_return_check"));
	gSavedSettings.setBOOL("PlayTypingAnim", childGetValue("play_typing_animation"));

	gSavedSettings.setF32("ConsoleBackgroundOpacity", childGetValue("console_opacity").asReal());
	gSavedSettings.setF32("ChatBubbleOpacity", childGetValue("bubble_chat_opacity").asReal());

	gSavedSettings.setString("TranslateLanguage", childGetValue("translate_language_combobox"));
	gSavedSettings.setBOOL("TranslateChat", childGetValue("translate_chat"));

	BOOL chan_check = childGetValue("toggle_channel_control");
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

	refreshValues(); // member values become the official values and cancel becomes a no-op.
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

LLPanel* LLPrefsChat::getPanel()
{
	return &impl;
}
