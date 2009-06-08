/** 
 * @file llfloaterhtmlhelp.cpp
 * @brief HTML Help floater - uses embedded web browser control
 *
 * $LicenseInfo:firstyear=2006&license=viewergpl$
 * 
 * Copyright (c) 2006-2009, Linden Research, Inc.
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

#include "llfloaterhtmlhelp.h"

#include "llparcel.h"
#include "lluictrlfactory.h"
#include "llwebbrowserctrl.h"
#include "llviewerwindow.h"
#include "llviewercontrol.h"
#include "llviewerparcelmgr.h"
#include "llweb.h"
#include "llui.h"
#include "roles_constants.h"

#include "llurlhistory.h"
#include "llwebbrowserctrl.h"
#include "llviewermedia.h"
#include "llviewerparcelmedia.h"
#include "llcombobox.h"

#include "llchat.h"
#include "lllineeditor.h"
#include "llfloaterchat.h"
#include "lltrans.h"

// static
LLFloaterMediaBrowser* LLFloaterMediaBrowser::sInstance = NULL;

LLFloaterMediaBrowser::LLFloaterMediaBrowser(const LLSD& media_data)
{
	LLUICtrlFactory::getInstance()->buildFloater(this, "floater_media_browser.xml");
}

void LLFloaterMediaBrowser::draw()
{
	BOOL url_exists = !mAddressCombo->getValue().asString().empty();
	childSetEnabled("go", url_exists);
	childSetEnabled("set_home", url_exists);
	LLParcel* parcel = LLViewerParcelMgr::getInstance()->getAgentParcel();
	if(parcel)
	{
		childSetVisible("parcel_owner_controls", LLViewerParcelMgr::isParcelModifiableByAgent(parcel, GP_LAND_CHANGE_MEDIA));
		childSetEnabled("assign", !mAddressCombo->getValue().asString().empty());
	}
	LLFloater::draw();
}

BOOL LLFloaterMediaBrowser::postBuild()
{
	mBrowser = getChild<LLWebBrowserCtrl>("browser");
	mBrowser->addObserver(this);

	mAddressCombo = getChild<LLComboBox>("address");
	mAddressCombo->setCommitCallback(onEnterAddress);
	mAddressCombo->setCallbackUserData(this);

	childSetAction("back", onClickBack, this);
	childSetAction("forward", onClickForward, this);
	childSetAction("reload", onClickRefresh, this);
	childSetAction("go", onClickGo, this);
	childSetAction("close", onClickClose, this);
	childSetAction("open_browser", onClickOpenWebBrowser, this);
	childSetAction("assign", onClickAssign, this);
	childSetAction("home", onClickHome, this);
	childSetAction("set_home", onClickSetHome, this);

	buildURLHistory();
	return TRUE;
}

void LLFloaterMediaBrowser::buildURLHistory()
{
	LLCtrlListInterface* url_list = childGetListInterface("address");
	if (url_list)
	{
		url_list->operateOnAll(LLCtrlListInterface::OP_DELETE);
	}

	// Get all of the entries in the "parcel" collection
	LLSD parcel_history = LLURLHistory::getURLHistory("browser");

	LLSD::array_iterator iter_history =
		parcel_history.beginArray();
	LLSD::array_iterator end_history =
		parcel_history.endArray();
	for(; iter_history != end_history; ++iter_history)
	{
		std::string url = (*iter_history).asString();
		if(! url.empty())
			url_list->addSimpleElement(url);
	}
}

void LLFloaterMediaBrowser::onClose(bool app_quitting)
{
	//setVisible(FALSE)
	destroy();
}

void LLFloaterMediaBrowser::onLocationChange( const EventType& eventIn )
{
	// hitting the refresh button will navigate to same URL, so don't add to address history
	mCurrentURL = eventIn.getStringValue();
	std::string::size_type string_start = mCurrentURL.find("://");
	std::string truncated_url;
	if ((string_start == std::string::npos) || (1)) // NOTE: this conditional is forced true to disable truncation DEV-9834
	{
		truncated_url = mCurrentURL;
	}
	else
	{
		truncated_url = mCurrentURL.substr(string_start + 3);
	}
	// redirects will navigate momentarily to about:blank, don't add to history
	if (truncated_url != "about:blank")
	{
		mAddressCombo->remove(truncated_url);
		mAddressCombo->add(truncated_url, ADD_SORTED);
		mAddressCombo->selectByValue(truncated_url);

		// Serialize url history
		LLURLHistory::removeURL("browser", truncated_url);
		LLURLHistory::addURL("browser", truncated_url);
	}
	childSetEnabled("back", mBrowser->canNavigateBack());
	childSetEnabled("forward", mBrowser->canNavigateForward());
	childSetEnabled("reload", TRUE);
	gSavedSettings.setString("BrowserLastVisited", truncated_url);
}

LLFloaterMediaBrowser* LLFloaterMediaBrowser::showInstance(const LLSD& media_url)
{
	LLFloaterMediaBrowser* sInstance = LLUISingleton<LLFloaterMediaBrowser, VisibilityPolicy<LLFloater> >::showInstance(media_url);

	sInstance->openMedia(media_url.asString());
	return sInstance;
}

//static
void LLFloaterMediaBrowser::toggle()
{
	bool visible = LLFloaterMediaBrowser::instanceVisible();
	LLFloaterMediaBrowser* self = sInstance->getInstance();

	if(visible && self)
	{
		self->close();
	}
	else
	{
		//Show home url if new session, last visited if not
		std::string last_url = gSavedSettings.getString("BrowserLastVisited");
		if(last_url.empty()) 
			last_url = gSavedSettings.getString("BrowserHome");
		showInstance(last_url);
	}
}

//static
void LLFloaterMediaBrowser::helpF1()
{
	std::string url = gSavedSettings.getString("HelpSupportURL");
	std::string* url_copy = new std::string(url);
 
    gViewerWindow->alertXml("ClickOpenF1Help", onClickF1HelpLoadURL, url_copy);
}
 
// static
void LLFloaterMediaBrowser::onClickF1HelpLoadURL(S32 option, void* userdata)
{
  std::string* urlp = (std::string*)userdata;
  if (0 == option)
  {
    showInstance(urlp->c_str());
  }
  delete urlp;
}

//static 
void LLFloaterMediaBrowser::onEnterAddress(LLUICtrl* ctrl, void* user_data)
{
	LLFloaterMediaBrowser* self = (LLFloaterMediaBrowser*)user_data;
	self->mBrowser->navigateTo(self->mAddressCombo->getValue().asString());
}

//static 
void LLFloaterMediaBrowser::onClickRefresh(void* user_data)
{
	LLFloaterMediaBrowser* self = (LLFloaterMediaBrowser*)user_data;

	self->mAddressCombo->remove(0);
	self->mBrowser->navigateTo(self->mCurrentURL);
}

//static 
void LLFloaterMediaBrowser::onClickForward(void* user_data)
{
	LLFloaterMediaBrowser* self = (LLFloaterMediaBrowser*)user_data;

	self->mBrowser->navigateForward();
}

//static 
void LLFloaterMediaBrowser::onClickBack(void* user_data)
{
	LLFloaterMediaBrowser* self = (LLFloaterMediaBrowser*)user_data;

	self->mBrowser->navigateBack();
}

//static 
void LLFloaterMediaBrowser::onClickGo(void* user_data)
{
	LLFloaterMediaBrowser* self = (LLFloaterMediaBrowser*)user_data;

	self->mBrowser->navigateTo(self->mAddressCombo->getValue().asString());
}

//static 
void LLFloaterMediaBrowser::onClickClose(void* user_data)
{
	LLFloaterMediaBrowser* self = (LLFloaterMediaBrowser*)user_data;

	self->close();
}

//static 
void LLFloaterMediaBrowser::onClickOpenWebBrowser(void* user_data)
{
	LLFloaterMediaBrowser* self = (LLFloaterMediaBrowser*)user_data;

	std::string url = self->mCurrentURL.empty() ? 
		self->mBrowser->getHomePageUrl() :
		self->mCurrentURL;
	LLWeb::loadURLExternal(url);
}

void LLFloaterMediaBrowser::onClickAssign(void* user_data)
{
	LLFloaterMediaBrowser* self = (LLFloaterMediaBrowser*)user_data;

	LLParcel* parcel = LLViewerParcelMgr::getInstance()->getAgentParcel();
	if (!parcel)
	{
		return;
	}
	std::string media_url = self->mAddressCombo->getValue().asString();
	LLStringUtil::trim(media_url);

	parcel->setMediaURL(media_url);
	parcel->setMediaType(std::string("text/html"));

	// Send current parcel data upstream to server 
	LLViewerParcelMgr::getInstance()->sendParcelPropertiesUpdate( parcel, true );
	// now check for video
	LLViewerParcelMedia::update( parcel );
}

void LLFloaterMediaBrowser::onClickHome(void* user_data)
{
	LLFloaterMediaBrowser* self = (LLFloaterMediaBrowser*)user_data;
	if (self)
	{
		if (self->mBrowser)
		{
			std::string home_url = gSavedSettings.getString("BrowserHome");
			self->mBrowser->navigateTo(home_url);
		}
	}
}

void LLFloaterMediaBrowser::onClickSetHome(void* user_data)
{
	LLFloaterMediaBrowser* self = (LLFloaterMediaBrowser*)user_data;
	std::string url = self->mCurrentURL;
	if(!url.empty())
	{
		LLChat chat;
		std::string log_message = LLTrans::getString("new_home_page") + " ";
		log_message += url;
		chat.mText = log_message;
		LLFloaterChat::addChat(chat, FALSE, FALSE);
		gSavedSettings.setString("BrowserHome", url);
	}
}

void LLFloaterMediaBrowser::openMedia(const std::string& media_url)
{
	mBrowser->setHomePageUrl(media_url);
	mBrowser->navigateTo(media_url);
}
