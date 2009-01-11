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

#include "llprefschat.h"
#include "lltexteditor.h"
#include "llviewercontrol.h"
#include "lluictrlfactory.h"
#include "llcolorswatch.h"
#include "llradiogroup.h"
#include "llstylemap.h"

class LLPrefsChatImpl : public LLPanel
{
public:
	LLPrefsChatImpl();
	/*virtual*/ ~LLPrefsChatImpl(){};

	void apply();
	void cancel();

protected:
	S32	mChatSize;
	F32	mChatPersist;
	S32	mChatMaxLines;
	LLColor4 mSystemChatColor;
	LLColor4 mUserChatColor;
	LLColor4 mAgentChatColor;
	LLColor4 mIMChatColor;
	LLColor4 mObjectChatColor;
	LLColor4 mOwnerSayChatColor;
	LLColor4 mBGChatColor;
	LLColor4 mScriptErrorColor;
	LLColor4 mHTMLLinkColor;
	BOOL mChatFullWidth;
	BOOL mCloseChatOnReturn;
	BOOL mArrowKeysMoveAvatar;
	BOOL mShowTimestamps;
	BOOL mPlayTypingAnim;
	BOOL mChatBubbles;
	BOOL mScriptErrorAsChat;
	F32	mConsoleOpacity;
	F32	mBubbleOpacity;
};


LLPrefsChatImpl::LLPrefsChatImpl()
	:	LLPanel(std::string("Chat Panel"))
{
	LLUICtrlFactory::getInstance()->buildPanel(this, "panel_preferences_chat.xml");

	getChild<LLRadioGroup>("chat_font_size")->setSelectedIndex(gSavedSettings.getS32("ChatFontSize"));
	childSetValue("fade_chat_time", gSavedSettings.getF32("ChatPersistTime"));
	childSetValue("max_chat_count", gSavedSettings.getS32("ConsoleMaxLines"));

	getChild<LLColorSwatchCtrl>("system")->set(gSavedSettings.getColor4("SystemChatColor"));
	getChild<LLColorSwatchCtrl>("user")->set(gSavedSettings.getColor4("UserChatColor"));
	getChild<LLColorSwatchCtrl>("agent")->set(gSavedSettings.getColor4("AgentChatColor"));
	getChild<LLColorSwatchCtrl>("im")->set(gSavedSettings.getColor4("IMChatColor"));
	getChild<LLColorSwatchCtrl>("script_error")->set(gSavedSettings.getColor4("ScriptErrorColor"));
	getChild<LLColorSwatchCtrl>("objects")->set(gSavedSettings.getColor4("ObjectChatColor"));
	getChild<LLColorSwatchCtrl>("owner")->set(gSavedSettings.getColor4("llOwnerSayChatColor"));
	getChild<LLColorSwatchCtrl>("background")->set(gSavedSettings.getColor4("BackgroundChatColor"));
	getChild<LLColorSwatchCtrl>("links")->set(gSavedSettings.getColor4("HTMLLinkColor"));

	childSetValue("arrow_keys_move_avatar_check", gSavedSettings.getBOOL("ArrowKeysMoveAvatar"));
	childSetValue("show_timestamps_check", gSavedSettings.getBOOL("ChatShowTimestamps"));
	childSetValue("script_errors_as_chat", gSavedSettings.getBOOL("ScriptErrorsAsChat"));
 
	childSetValue("bubble_text_chat", gSavedSettings.getBOOL("UseChatBubbles"));
	childSetValue("chat_full_width_check", gSavedSettings.getBOOL("ChatFullWidth"));
	childSetValue("close_chat_on_return_check", gSavedSettings.getBOOL("CloseChatOnReturn"));
	childSetValue("play_typing_animation", gSavedSettings.getBOOL("PlayTypingAnim"));
	childSetValue("console_opacity", gSavedSettings.getF32("ConsoleBackgroundOpacity"));
	childSetValue("bubble_chat_opacity", gSavedSettings.getF32("ChatBubbleOpacity"));

	//set values
	mChatSize = gSavedSettings.getS32("ChatFontSize");
	mChatPersist = gSavedSettings.getF32("ChatPersistTime");
	mChatMaxLines = gSavedSettings.getS32("ConsoleMaxLines");
	mSystemChatColor = gSavedSettings.getColor4("SystemChatColor");
	mUserChatColor = gSavedSettings.getColor4("UserChatColor");
	mAgentChatColor = gSavedSettings.getColor4("AgentChatColor");
	mIMChatColor = gSavedSettings.getColor4("IMChatColor");
	mObjectChatColor = gSavedSettings.getColor4("ObjectChatColor");
	mOwnerSayChatColor = gSavedSettings.getColor4("llOwnerSayChatColor");
	mBGChatColor = gSavedSettings.getColor4("BackgroundChatColor");
	mScriptErrorColor = gSavedSettings.getColor4("ScriptErrorColor");
	mHTMLLinkColor = gSavedSettings.getColor4("HTMLLinkColor");
	mArrowKeysMoveAvatar = gSavedSettings.getBOOL("ArrowKeysMoveAvatar");
	mShowTimestamps = gSavedSettings.getBOOL("ChatShowTimestamps");
	mScriptErrorAsChat = gSavedSettings.getBOOL("ScriptErrorsAsChat");
	mChatBubbles = gSavedSettings.getBOOL("UseChatBubbles");
	mChatFullWidth = gSavedSettings.getBOOL("ChatFullWidth");
	mCloseChatOnReturn = gSavedSettings.getBOOL("CloseChatOnReturn");
	mPlayTypingAnim = gSavedSettings.getBOOL("PlayTypingAnim"); 
	mConsoleOpacity = gSavedSettings.getF32("ConsoleBackgroundOpacity");
	mBubbleOpacity = gSavedSettings.getF32("ChatBubbleOpacity");
}

void LLPrefsChatImpl::cancel()
{
	gSavedSettings.setS32("ChatFontSize", mChatSize);
	gSavedSettings.setF32("ChatPersistTime", mChatPersist);
	gSavedSettings.setS32("ConsoleMaxLines", mChatMaxLines);
	gSavedSettings.setColor4("SystemChatColor", mSystemChatColor);
	gSavedSettings.setColor4("UserChatColor", mUserChatColor);
	gSavedSettings.setColor4("AgentChatColor", mAgentChatColor);
	gSavedSettings.setColor4("IMChatColor", mIMChatColor);
	gSavedSettings.setColor4("ObjectChatColor", mObjectChatColor);
	gSavedSettings.setColor4("llOwnerSayChatColor", mOwnerSayChatColor);
	gSavedSettings.setColor4("BackgroundChatColor", mBGChatColor);
	gSavedSettings.setColor4("ScriptErrorColor", mScriptErrorColor);
	gSavedSettings.setColor4("HTMLLinkColor", mHTMLLinkColor);
	gSavedSettings.setBOOL("ArrowKeysMoveAvatar", mArrowKeysMoveAvatar);
	gSavedSettings.setBOOL("ChatShowTimestamps", mShowTimestamps);
	gSavedSettings.setBOOL("ScriptErrorsAsChat", mScriptErrorAsChat);
	gSavedSettings.setBOOL("UseChatBubbles", mChatBubbles);
	gSavedSettings.setBOOL("ChatFullWidth", mChatFullWidth);
	gSavedSettings.setBOOL("CloseChatOnReturn", mCloseChatOnReturn);
	gSavedSettings.setBOOL("PlayTypingAnim", mPlayTypingAnim); 
	gSavedSettings.setF32("ConsoleBackgroundOpacity", mConsoleOpacity);
	gSavedSettings.setF32("ChatBubbleOpacity", mBubbleOpacity);	
}

void LLPrefsChatImpl::apply()
{
	gSavedSettings.setS32("ChatFontSize", getChild<LLRadioGroup>("chat_font_size")->getSelectedIndex());
	gSavedSettings.setF32("ChatPersistTime", childGetValue("fade_chat_time").asReal());
	gSavedSettings.setS32("ConsoleMaxLines", childGetValue("max_chat_count"));

	gSavedSettings.setColor4("SystemChatColor", childGetValue("system"));
	gSavedSettings.setColor4("UserChatColor", childGetValue("user"));
	gSavedSettings.setColor4("AgentChatColor", childGetValue("agent"));
	gSavedSettings.setColor4("IMChatColor", childGetValue("im"));
	gSavedSettings.setColor4("ScriptErrorColor", childGetValue("script_error"));
	gSavedSettings.setColor4("ObjectChatColor", childGetValue("objects"));
	gSavedSettings.setColor4("llOwnerSayChatColor", childGetValue("owner"));
	gSavedSettings.setColor4("BackgroundChatColor", childGetValue("background"));

	gSavedSettings.setColor4("HTMLLinkColor", childGetValue("links"));
	LLTextEditor::setLinkColor(childGetValue("links"));

	gSavedSettings.setBOOL("ArrowKeysMoveAvatar", childGetValue("arrow_keys_move_avatar_check"));
	gSavedSettings.setBOOL("ChatShowTimestamps", childGetValue("show_timestamps_check"));
	gSavedSettings.setBOOL("ScriptErrorsAsChat", childGetValue("script_errors_as_chat"));
	gSavedSettings.setBOOL("UseChatBubbles", childGetValue("bubble_text_chat"));
	gSavedSettings.setBOOL("ChatFullWidth", childGetValue("chat_full_width_check"));
	gSavedSettings.setBOOL("CloseChatOnReturn", childGetValue("close_chat_on_return_check"));
	gSavedSettings.setBOOL("PlayTypingAnim", childGetValue("play_typing_animation"));

	gSavedSettings.setF32("ConsoleBackgroundOpacity", childGetValue("console_opacity").asReal());
	gSavedSettings.setF32("ChatBubbleOpacity", childGetValue("bubble_chat_opacity").asReal());
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
