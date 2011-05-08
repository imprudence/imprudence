/**
* @file llprefscolors.cpp
* @brief Color-specific preferences for Imprudence
*
* $LicenseInfo:firstyear=2009&license=viewergpl$
*
* Copyright (c) 2010, McCabe Maxsted
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

#include "llprefscolors.h"
#include "lltexteditor.h"
#include "llviewercontrol.h"

#include "lluictrlfactory.h"
#include "llcolorswatch.h"

#include "boost/algorithm/string.hpp"

LLPrefsColors::LLPrefsColors()
{
	refreshColors(); // initialize member data from saved settings

	LLUICtrlFactory::getInstance()->buildPanel(this, "panel_preferences_colors.xml");
}

LLPrefsColors::~LLPrefsColors()
{
	// Children all cleaned up by default view destructor.
}

BOOL LLPrefsColors::postBuild()
{
	getChild<LLColorSwatchCtrl>("system")->set(gSavedSettings.getColor4("SystemChatColor"));
	getChild<LLColorSwatchCtrl>("user")->set(gSavedSettings.getColor4("UserChatColor"));
	getChild<LLColorSwatchCtrl>("agent")->set(gSavedSettings.getColor4("AgentChatColor"));
	getChild<LLColorSwatchCtrl>("im")->set(gSavedSettings.getColor4("IMChatColor"));
	getChild<LLColorSwatchCtrl>("script_error")->set(gSavedSettings.getColor4("ScriptErrorColor"));
	getChild<LLColorSwatchCtrl>("objects")->set(gSavedSettings.getColor4("ObjectChatColor"));
	getChild<LLColorSwatchCtrl>("owner")->set(gSavedSettings.getColor4("llOwnerSayChatColor"));
	getChild<LLColorSwatchCtrl>("background")->set(gSavedSettings.getColor4("BackgroundChatColor"));
	getChild<LLColorSwatchCtrl>("links")->set(gSavedSettings.getColor4("HTMLLinkColor"));
	getChild<LLColorSwatchCtrl>("FriendsChatColor")->set(gSavedSettings.getColor4("FriendsChatColor"));
	getChild<LLColorSwatchCtrl>("OwnNameChatColor")->set(gSavedSettings.getColor4("OwnNameChatColor"));

	childSetValue("HighlightOwnNameInIM", gSavedSettings.getBOOL("HighlightOwnNameInIM"));
	childSetValue("HighlightFriendsChat", gSavedSettings.getBOOL("HighlightFriendsChat"));
	childSetValue("HighlightOwnNameInChat", gSavedSettings.getBOOL("HighlightOwnNameInChat"));
	updateSelfCheck();
	updateFriendsCheck();
	
	// All three of these settings must exist, they are read by LLFloaterChat::isOwnNameInText.
	childSetValue("nick01", gSavedSettings.getString("HighlightNickname01"));
	childSetValue("nick02", gSavedSettings.getString("HighlightNickname02"));
	childSetValue("nick03", gSavedSettings.getString("HighlightNickname03"));

	childSetCommitCallback("HighlightOwnNameInIM", onCommitCheckSelfName, this);
	childSetCommitCallback("HighlightOwnNameInChat", onCommitCheckSelfName, this);
	childSetCommitCallback("HighlightFriendsChat", onCommitCheckFriends, this);

	return TRUE;
}

void LLPrefsColors::refreshColors()
{
	mSystemChatColor = gSavedSettings.getColor4("SystemChatColor");
	mUserChatColor = gSavedSettings.getColor4("UserChatColor");
	mAgentChatColor = gSavedSettings.getColor4("AgentChatColor");
	mIMChatColor = gSavedSettings.getColor4("IMChatColor");
	mObjectChatColor = gSavedSettings.getColor4("ObjectChatColor");
	mOwnerSayChatColor = gSavedSettings.getColor4("llOwnerSayChatColor");
	mBGChatColor = gSavedSettings.getColor4("BackgroundChatColor");
	mScriptErrorColor = gSavedSettings.getColor4("ScriptErrorColor");
	mHTMLLinkColor = gSavedSettings.getColor4("HTMLLinkColor");
	mFriendsChatColor = gSavedSettings.getColor4("FriendsChatColor");
	mOwnNameChatColor = gSavedSettings.getColor4("OwnNameChatColor");
}

// static
void LLPrefsColors::onCommitCheckSelfName(LLUICtrl* ctrl, void* userdata)
{
	LLPrefsColors* self = (LLPrefsColors*)userdata;
	self->updateSelfCheck();
}

void LLPrefsColors::updateSelfCheck()
{
	bool highlight_names_enabled = (childGetValue("HighlightOwnNameInIM") || childGetValue("HighlightOwnNameInChat"));

	getChild<LLColorSwatchCtrl>("OwnNameChatColor")->setEnabled(highlight_names_enabled);
	childSetEnabled("nick01", highlight_names_enabled);
	childSetEnabled("nick02", highlight_names_enabled);
	childSetEnabled("nick03", highlight_names_enabled);
	childSetEnabled("nick01_text", highlight_names_enabled);
	childSetEnabled("nick02_text", highlight_names_enabled);
	childSetEnabled("nick03_text", highlight_names_enabled);
}

// static
void LLPrefsColors::onCommitCheckFriends(LLUICtrl* ctrl, void* userdata)
{
	LLPrefsColors* self = (LLPrefsColors*)userdata;
	self->updateFriendsCheck();
}

void LLPrefsColors::updateFriendsCheck()
{
	getChild<LLColorSwatchCtrl>("FriendsChatColor")->setEnabled(childGetValue("HighlightFriendsChat"));
}

void LLPrefsColors::cancel()
{
	gSavedSettings.setColor4("SystemChatColor", mSystemChatColor);
	gSavedSettings.setColor4("UserChatColor", mUserChatColor);
	gSavedSettings.setColor4("AgentChatColor", mAgentChatColor);
	gSavedSettings.setColor4("IMChatColor", mIMChatColor);
	gSavedSettings.setColor4("ObjectChatColor", mObjectChatColor);
	gSavedSettings.setColor4("llOwnerSayChatColor", mOwnerSayChatColor);
	gSavedSettings.setColor4("BackgroundChatColor", mBGChatColor);
	gSavedSettings.setColor4("ScriptErrorColor", mScriptErrorColor);
	gSavedSettings.setColor4("HTMLLinkColor", mHTMLLinkColor);
	gSavedSettings.setColor4("FriendsChatColor", mFriendsChatColor);
	gSavedSettings.setColor4("OwnNameChatColor", mOwnNameChatColor);
}

void LLPrefsColors::apply()
{
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

	gSavedSettings.setBOOL("HighlightOwnNameInIM", childGetValue("HighlightOwnNameInIM"));
	gSavedSettings.setBOOL("HighlightFriendsChat", childGetValue("HighlightFriendsChat"));
	gSavedSettings.setColor4("FriendsChatColor", getChild<LLColorSwatchCtrl>("FriendsChatColor")->get());
	gSavedSettings.setBOOL("HighlightOwnNameInChat", childGetValue("HighlightOwnNameInChat"));
	gSavedSettings.setColor4("OwnNameChatColor", getChild<LLColorSwatchCtrl>("OwnNameChatColor")->get());

	std::string nick01 = childGetValue("nick01");
	boost::trim(nick01);
	gSavedSettings.setString("HighlightNickname01", nick01);

	std::string nick02 = childGetValue("nick02");
	boost::trim(nick02);
	gSavedSettings.setString("HighlightNickname02", nick02);

	std::string nick03 = childGetValue("nick03");
	boost::trim(nick03);
	gSavedSettings.setString("HighlightNickname03", nick03);

	refreshColors(); // member values become the official values and cancel becomes a no-op.
}
