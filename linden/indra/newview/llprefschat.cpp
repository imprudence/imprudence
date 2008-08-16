/** 
 * @file llprefschat.cpp
 * @author James Cook, Richard Nelson
 * @brief Chat preferences panel
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

#include "llprefschat.h"

#include "llcheckboxctrl.h"
#include "llcolorswatch.h"
#include "llpanel.h"
#include "llradiogroup.h"

#include "llchatbar.h"
#include "llconsole.h"
#include "llspinctrl.h"
#include "llsliderctrl.h"
#include "llviewercontrol.h"
#include "llvieweruictrlfactory.h"
#include "message.h"
#include "viewer.h"
#include "lltexteditor.h"

class LLPrefsChatImpl : public LLPanel
{
public:
	LLPrefsChatImpl();
	/*virtual*/ ~LLPrefsChatImpl(){};
	
	virtual void refresh();

	void apply();
	void cancel();

protected:
	S32	mChatSize;
	F32	mChatPersist;
	S32	mChatMaxLines;
	LLColor4 mSystemChatColor;
	LLColor4 mAgentChatColor;
	LLColor4 mObjectChatColor;
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
:	LLPanel("Chat Panel")
{
	gUICtrlFactory->buildPanel(this, "panel_preferences_chat.xml");

	refresh();
}
 
void LLPrefsChatImpl::refresh()
{
	LLPanel::refresh();

	//set values
	mChatSize = gSavedSettings.getS32("ChatFontSize");
	mChatPersist = gSavedSettings.getF32("ChatPersistTime");
	mChatMaxLines = gSavedSettings.getS32("ConsoleMaxLines");
	mSystemChatColor = gSavedSettings.getColor4("SystemChatColor");
	mAgentChatColor = gSavedSettings.getColor4("AgentChatColor");
	mObjectChatColor = gSavedSettings.getColor4("ObjectChatColor");
	mBGChatColor = gSavedSettings.getColor4("BackgroundChatColor");
	mScriptErrorColor = gSavedSettings.getColor4("ScriptErrorColor");
	mHTMLLinkColor = gSavedSettings.getColor4("HTMLLinkColor");
	mArrowKeysMoveAvatar = gSavedSettings.getBOOL("ArrowKeysMoveAvatar");
	mShowTimestamps = gSavedSettings.getBOOL("ChatShowTimestamps");
	mScriptErrorAsChat = gSavedSettings.getBOOL("ScriptErrorsAsChat");
	mChatBubbles = gSavedSettings.getBOOL("UseChatBubbles");
	mChatFullWidth = gSavedSettings.getBOOL("ChatFullWidth");
	mCloseChatOnReturn = gSavedSettings.getBOOL("CloseChatOnReturn");
	mConsoleOpacity = gSavedSettings.getF32("ConsoleBackgroundOpacity");
	mBubbleOpacity = gSavedSettings.getF32("ChatBubbleOpacity");
}

void LLPrefsChatImpl::cancel()
{
	gSavedSettings.setS32("ChatFontSize", mChatSize);
	gSavedSettings.setF32("ChatPersistTime", mChatPersist);
	gSavedSettings.setS32("ConsoleMaxLines", mChatMaxLines);
	gSavedSettings.setColor4("SystemChatColor", mSystemChatColor);
	gSavedSettings.setColor4("AgentChatColor", mAgentChatColor);
	gSavedSettings.setColor4("ObjectChatColor", mObjectChatColor);
	gSavedSettings.setColor4("BackgroundChatColor", mBGChatColor);
	gSavedSettings.setColor4("ScriptErrorColor", mScriptErrorColor);
	gSavedSettings.setColor4("HTMLLinkColor", mHTMLLinkColor);
	gSavedSettings.setBOOL("ArrowKeysMoveAvatar", mArrowKeysMoveAvatar);
	gSavedSettings.setBOOL("ChatShowTimestamps", mShowTimestamps);
	gSavedSettings.setBOOL("ScriptErrorsAsChat", mScriptErrorAsChat);
	gSavedSettings.setBOOL("UseChatBubbles", mChatBubbles);
	gSavedSettings.setBOOL("ChatFullWidth", mChatFullWidth);
	gSavedSettings.setBOOL("CloseChatOnReturn", mCloseChatOnReturn);
	gSavedSettings.setF32("ConsoleBackgroundOpacity", mConsoleOpacity);
	gSavedSettings.setF32("ChatBubbleOpacity", mBubbleOpacity);	
}

void LLPrefsChatImpl::apply()
{
		
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
	LLTextEditor::setLinkColor( gSavedSettings.getColor4("HTMLLinkColor") );
}

void LLPrefsChat::cancel()
{
	impl.cancel();
}

LLPanel* LLPrefsChat::getPanel()
{
	return &impl;
}
