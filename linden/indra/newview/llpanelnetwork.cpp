/** 
 * @file llpanelnetwork.cpp
 * @brief Network preferences panel
 *
 * $LicenseInfo:firstyear=2001&license=viewergpl$
 * 
 * Copyright (c) 2001-2009, Linden Research, Inc.
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

//file include
#include "llpanelnetwork.h"
#include "llstartup.h"

// project includes
#include "llcheckboxctrl.h"
#include "llradiogroup.h"
#include "lldirpicker.h"
#include "lluictrlfactory.h"
#include "llviewercontrol.h"
#include "llviewerwindow.h"

// project includes
#include "llcheckboxctrl.h"
#include "hippogridmanager.h"
#include "lluictrlfactory.h"
#include "llviewercontrol.h"
#include "llviewermedia.h"
#include "llviewerwindow.h"
#include "llpluginclassmedia.h"

#include "hippogridmanager.h"
#include "llpluginclassmedia.h"
#include "llviewermedia.h"

// helper functions for getting/freeing the web browser media
// if creating/destroying these is too slow, we'll need to create
// a static member and update all our static callbacks
viewer_media_t get_web_media()
{

	viewer_media_t media_source = LLViewerMedia::newMediaImpl("", LLUUID::null, 0, 0, 0, 0, "text/html");

	return media_source;
}

bool LLPanelNetwork::sSocksSettingsChanged;

LLPanelNetwork::LLPanelNetwork()
{
	LLUICtrlFactory::getInstance()->buildPanel(this, "panel_preferences_network.xml");
}

BOOL LLPanelNetwork::postBuild()
{
	std::string cache_location = gDirUtilp->getExpandedFilename(LL_PATH_CACHE, "");
	childSetText("disk_cache_location", cache_location);
		
	childSetAction("clear_disk_cache", onClickClearDiskCache, this);
	childSetAction("set_disk_cache", onClickSetDiskCache, this);
	childSetAction("reset_disk_cache", onClickResetDiskCache, this);
	
	childSetEnabled("connection_port", gSavedSettings.getBOOL("ConnectionPortEnabled"));
	childSetCommitCallback("connection_port_enabled", onCommitPort, this);

	childSetValue("disk_cache_size", (F32)gSavedSettings.getU32("CacheSize"));
	childSetValue("max_bandwidth", gSavedSettings.getF32("ThrottleBandwidthKBPS"));
	childSetValue("connection_port_enabled", gSavedSettings.getBOOL("ConnectionPortEnabled"));
	childSetValue("connection_port", (F32)gSavedSettings.getU32("ConnectionPort"));

	childSetCommitCallback("xmlrpc_proxy_enabled", onCommitXMLRPCProxyEnabled, this);
	childSetValue("xmlrpc_proxy_enabled", gSavedSettings.getBOOL("XMLRPCProxyEnabled"));
	childSetValue("xmlrpc_proxy_editor", gSavedSettings.getString("XMLRPCProxyAddress"));
	childSetValue("xmlrpc_proxy_port", gSavedSettings.getS32("XMLRPCProxyPort"));
	childSetEnabled("xmlrpc_proxy_text_label", gSavedSettings.getBOOL("XMLRPCProxyEnabled"));
	childSetEnabled("xmlrpc_proxy_editor", gSavedSettings.getBOOL("XMLRPCProxyEnabled"));
	childSetEnabled("xmlrpc_proxy_port", gSavedSettings.getBOOL("XMLRPCProxyEnabled"));

	// Socks 5 proxy settings, commit callbacks
	childSetCommitCallback("socks5_proxy_enabled", onCommitSocks5ProxyEnabled, this);
	childSetCommitCallback("socks5_auth", onSocksAuthChanged, this);

	//Socks 5 proxy settings, saved data
	childSetValue("socks5_proxy_enabled",   gSavedSettings.getBOOL("Socks5ProxyEnabled"));
	childSetValue("socks5_http_proxy_type", gSavedSettings.getString("Socks5HttpProxyType"));

	childSetValue("socks5_proxy_host",     gSavedSettings.getString("Socks5ProxyHost"));
	childSetValue("socks5_proxy_port",     (F32)gSavedSettings.getU32("Socks5ProxyPort"));
	childSetValue("socks5_proxy_username", gSavedSettings.getString("Socks5Username"));
	childSetValue("socks5_proxy_password", gSavedSettings.getString("Socks5Password"));
	childSetValue("socks5_auth", gSavedSettings.getString("Socks5AuthType"));

	// Socks 5 proxy settings, check if settings modified callbacks
	childSetCommitCallback("socks5_proxy_host", onSocksSettingsModified,this);
	childSetCommitCallback("socks5_proxy_port", onSocksSettingsModified,this);
	childSetCommitCallback("socks5_proxy_username", onSocksSettingsModified,this);
	childSetCommitCallback("socks5_proxy_password", onSocksSettingsModified,this);
	
	// Socks 5 settings, Set all controls and labels enabled state
	updateProxyEnabled(this, gSavedSettings.getBOOL("Socks5ProxyEnabled"), gSavedSettings.getString("Socks5AuthType"));

	childSetEnabled("xmlrpc_proxy_editor", gSavedSettings.getBOOL("XMLRPCProxyEnabled"));
	childSetEnabled("xmlrpc_proxy_port", gSavedSettings.getBOOL("XMLRPCProxyEnabled"));
	childSetEnabled("xmlrpc_proxy_text_label", gSavedSettings.getBOOL("XMLRPCProxyEnabled"));

	childSetValue("http_texture_check", gSavedSettings.getBOOL("ImagePipelineUseHTTP"));
	childSetValue("speed_rez_check", gSavedSettings.getBOOL("SpeedRez"));
	childSetValue("speed_rez_interval_spinner", (F32)gSavedSettings.getU32("SpeedRezInterval"));
	childSetCommitCallback("speed_rez_check", onCommitSpeedRezCheckBox, this);

	sSocksSettingsChanged = false;

	// formerly the Web panel -- MC
	childSetAction("clear_web_cache", onClickClearWebCache, this);
	childSetCommitCallback("web_proxy_enabled", onCommitWebProxyEnabled, this);

	std::string value = gSavedSettings.getBOOL("UseExternalBrowser") ? "external" : "internal";
	childSetValue("use_external_browser", value);

	childSetValue("cookies_enabled", gSavedSettings.getBOOL("BrowserCookiesEnabled"));
	childSetAction("clear_cookies", onClickClearCookies,this);

	childSetValue("web_proxy_enabled", gSavedSettings.getBOOL("BrowserProxyEnabled"));
	childSetValue("web_proxy_editor", gSavedSettings.getString("BrowserProxyAddress"));
	childSetValue("web_proxy_port", gSavedSettings.getS32("BrowserProxyPort"));

	if (gHippoGridManager->getConnectedGrid()->isSecondLife()) 
	{
		childSetValue("world_search_editor", gSavedSettings.getString("SearchURLQuery")) ;
	}
	else
	{	
		childSetValue("world_search_editor", gSavedSettings.getString("SearchURLQueryOpenSim")) ;
	}
	childSetAction("world_search_reset_default", onClickSearchDefault, this);
	childSetAction("world_search_clear", onClickSearchClear, this);

	childSetEnabled("proxy_text_label", gSavedSettings.getBOOL("BrowserProxyEnabled"));
	childSetEnabled("web_proxy_editor", gSavedSettings.getBOOL("BrowserProxyEnabled"));
	childSetEnabled("web_proxy_port", gSavedSettings.getBOOL("BrowserProxyEnabled"));

	refresh();

	return TRUE;
}

LLPanelNetwork::~LLPanelNetwork()
{
	// Children all cleaned up by default view destructor.
}


void LLPanelNetwork::apply()
{
	gSavedSettings.setU32("CacheSize", childGetValue("disk_cache_size").asInteger());
	gSavedSettings.setF32("ThrottleBandwidthKBPS", childGetValue("max_bandwidth").asReal());
	gSavedSettings.setBOOL("ConnectionPortEnabled", childGetValue("connection_port_enabled"));
	gSavedSettings.setU32("ConnectionPort", childGetValue("connection_port").asInteger());

	gSavedSettings.setBOOL("XMLRPCProxyEnabled", childGetValue("xmlrpc_proxy_enabled"));
	gSavedSettings.setString("XMLRPCProxyAddress", childGetValue("xmlrpc_proxy_editor"));
	gSavedSettings.setS32("XMLRPCProxyPort", childGetValue("xmlrpc_proxy_port"));

	gSavedSettings.setBOOL("Socks5ProxyEnabled", childGetValue("socks5_proxy_enabled"));		
	gSavedSettings.setString("Socks5HttpProxyType", childGetValue("socks5_http_proxy_type"));
	gSavedSettings.setString("Socks5ProxyHost", childGetValue("socks5_proxy_host"));
	gSavedSettings.setU32("Socks5ProxyPort", childGetValue("socks5_proxy_port").asInteger());

	gSavedSettings.setString("Socks5AuthType", childGetValue("socks5_auth"));
	gSavedSettings.setString("Socks5Username", childGetValue("socks5_proxy_username"));
	gSavedSettings.setString("Socks5Password", childGetValue("socks5_proxy_password"));

	gSavedSettings.setBOOL("ImagePipelineUseHTTP", childGetValue("http_texture_check"));
	gSavedSettings.setBOOL("SpeedRez", childGetValue("speed_rez_check"));
	gSavedSettings.setU32("SpeedRezInterval", childGetValue("speed_rez_interval_spinner").asReal());

	if (sSocksSettingsChanged)
	{
		if (LLStartUp::getStartupState() != STATE_LOGIN_WAIT)
		{
			LLNotifications::instance().add("ProxyNeedRestart");
		}
		else
		{
			// Mark the socks class that it needs to update its connection
			LLSocks::getInstance()->updated();
		}
	}

	// formerly the Web panel -- MC
	gSavedSettings.setBOOL("BrowserCookiesEnabled", childGetValue("cookies_enabled"));

	bool proxy_enable = childGetValue("web_proxy_enabled");
	std::string proxy_address = childGetValue("web_proxy_editor");
	int proxy_port = childGetValue("web_proxy_port");
	gSavedSettings.setBOOL("BrowserProxyEnabled", proxy_enable);
	gSavedSettings.setString("BrowserProxyAddress", proxy_address);
	gSavedSettings.setS32("BrowserProxyPort", proxy_port);
	LLViewerMedia::setProxyConfig(proxy_enable, proxy_address, proxy_port);

	if (gHippoGridManager->getConnectedGrid()->isSecondLife()) 
	{
		gSavedSettings.setString("SearchURLQuery", childGetValue("world_search_editor"));
	}
	else
	{
		gSavedSettings.setString("SearchURLQueryOpenSim", childGetValue("world_search_editor"));
	}

	bool value = childGetValue("use_external_browser").asString() == "external" ? true : false;
	gSavedSettings.setBOOL("UseExternalBrowser", value);
	
	viewer_media_t media_source = get_web_media();
	if (media_source && media_source->hasMedia())
	{
		media_source->getMediaPlugin()->enable_cookies(childGetValue("cookies_enabled"));

		bool proxy_enable = childGetValue("web_proxy_enabled");
		std::string proxy_address = childGetValue("web_proxy_editor");
		int proxy_port = childGetValue("web_proxy_port");
		media_source->getMediaPlugin()->proxy_setup(proxy_enable, proxy_address, proxy_port);
	}
}

void LLPanelNetwork::cancel()
{
}

void LLPanelNetwork::refresh()
{
	if (childGetValue("speed_rez_check").asBoolean())
	{
		childEnable("speed_rez_interval_spinner");
		childEnable("speed_rez_seconds_text");
	}
	else
	{
		childDisable("speed_rez_interval_spinner");
		childDisable("speed_rez_seconds_text");
	}
}

// static
void LLPanelNetwork::onClickClearDiskCache(void*)
{
	// flag client cache for clearing next time the client runs
	gSavedSettings.setBOOL("PurgeCacheOnNextStartup", TRUE);
	LLNotifications::instance().add("CacheWillClear");
}

// static
void LLPanelNetwork::onClickSetDiskCache(void* user_data)
{
	LLPanelNetwork* self = (LLPanelNetwork*)user_data;

	std::string cur_name(gSavedSettings.getString("CacheLocation"));
	std::string proposed_name(cur_name);
	
	LLDirPicker& picker = LLDirPicker::instance();
	if (! picker.getDir(&proposed_name ) )
	{
		return; //Canceled!
	}

	std::string dir_name = picker.getDirName();
	if (!dir_name.empty() && dir_name != cur_name)
	{
		self->childSetText("disk_cache_location", dir_name);
		LLNotifications::instance().add("CacheWillBeMoved");
		gSavedSettings.setString("NewCacheLocation", dir_name);
	}
	else
	{
		std::string cache_location = gDirUtilp->getCacheDir();
		self->childSetText("disk_cache_location", cache_location);
	}
}

// static
void LLPanelNetwork::onClickResetDiskCache(void* user_data)
{
 	LLPanelNetwork* self = (LLPanelNetwork*)user_data;
	if (!gSavedSettings.getString("CacheLocation").empty())
	{
		gSavedSettings.setString("NewCacheLocation", "");
		LLNotifications::instance().add("CacheWillBeMoved");
	}
	std::string cache_location = gDirUtilp->getCacheDir(true);
	self->childSetText("disk_cache_location", cache_location);
}

// static
void LLPanelNetwork::onCommitPort(LLUICtrl* ctrl, void* data)
{
  LLPanelNetwork* self = (LLPanelNetwork*)data;
  LLCheckBoxCtrl* check = (LLCheckBoxCtrl*)ctrl;

  if (!self || !check) return;
  self->childSetEnabled("connection_port", check->get());
  LLNotifications::instance().add("ChangeConnectionPort");
}

// static
void LLPanelNetwork::onCommitXMLRPCProxyEnabled(LLUICtrl* ctrl, void* data)
{
	LLPanelNetwork* self = (LLPanelNetwork*)data;
	LLCheckBoxCtrl* check = (LLCheckBoxCtrl*)ctrl;

	if (!self || !check) return;
	self->childSetEnabled("xmlrpc_proxy_editor", check->get());
	self->childSetEnabled("xmlrpc_proxy_port", check->get());
	self->childSetEnabled("xmlrpc_proxy_text_label", check->get());

	self->childSetEnabled("socks5_proxy_enabled", !check->get());
}

// static
void LLPanelNetwork::onCommitSocks5ProxyEnabled(LLUICtrl* ctrl, void* data)
{
	LLPanelNetwork* self  = (LLPanelNetwork*)data;
	LLCheckBoxCtrl* check = (LLCheckBoxCtrl*)ctrl;

	if (!self || !check) return;

	sSocksSettingsChanged = true;
	
	updateProxyEnabled(self, check->get(), self->childGetValue("socks5_auth"));
}

// static
void LLPanelNetwork::onSocksSettingsModified(LLUICtrl* ctrl, void* data)
{
	sSocksSettingsChanged = true;
}

// static
void LLPanelNetwork::onSocksAuthChanged(LLUICtrl* ctrl, void* data)
{
	LLRadioGroup* radio  = static_cast<LLRadioGroup*>(ctrl);
	LLPanelNetwork* self = static_cast<LLPanelNetwork*>(data);

	sSocksSettingsChanged = true;

	std::string selection = radio->getValue().asString();
	updateProxyEnabled(self, true, selection);
}

// static
void LLPanelNetwork::updateProxyEnabled(LLPanelNetwork * self, bool enabled, std::string authtype)
{
	// Manage all the enable/disable of the socks5 options from this single function
	// to avoid code duplication

	// Update all socks labels and controls except auth specific ones
	self->childSetEnabled("socks5_proxy_port",	enabled);
	self->childSetEnabled("socks5_proxy_host",	enabled);
	self->childSetEnabled("socks5_host_label",	enabled);
	self->childSetEnabled("socks5_proxy_label",	enabled);
	self->childSetEnabled("socks5_proxy_port",	enabled);
	self->childSetEnabled("socks5_auth_label",	enabled);
	self->childSetEnabled("socks5_auth",		enabled);

	// disable the web option if the web proxy has not been configured
	// this is still not ideal as apply or ok is needed for this to be saved to the preferences
	self->childSetEnabled("Web", gSavedSettings.getBOOL("BrowserProxyEnabled"));

	self->childSetEnabled("Socks", enabled);

	// Hide the auth specific labels if authtype is none or
	// we are not enabled.
	if ((authtype.compare("None") == 0) || (enabled == false))
	{
		self->childSetEnabled("socks5_username_label", false);
		self->childSetEnabled("socks5_password_label", false);
		self->childSetEnabled("socks5_proxy_username", false);
		self->childSetEnabled("socks5_proxy_password", false);
	}

	// Only show the username and password boxes if we are enabled
	// and authtype is username password.
	if ((authtype.compare("UserPass") == 0) && (enabled == true))
	{
		self->childSetEnabled("socks5_username_label", true);
		self->childSetEnabled("socks5_password_label", true);
		self->childSetEnabled("socks5_proxy_username", true);
		self->childSetEnabled("socks5_proxy_password", true);
	}

	// Disable the XMLRPC proxy if it's enabled and we enable SOCKS5
	self->childSetEnabled("xmlrpc_proxy_enabled", !enabled);
	self->childSetEnabled("xmlrpc_proxy_editor", !enabled);
	self->childSetEnabled("xmlrpc_proxy_port", !enabled);
	self->childSetEnabled("xmlrpc_proxy_text_label", !enabled);
}

// static
void LLPanelNetwork::onClickClearWebCache(void*)
{
	LLNotifications::instance().add("ConfirmClearBrowserCache", LLSD(), LLSD(), callback_clear_browser_cache);
}

//static
bool LLPanelNetwork::callback_clear_browser_cache(const LLSD& notification, const LLSD& response)
{
	S32 option = LLNotification::getSelectedOption(notification, response);
	if ( option == 0 ) // YES
	{
		viewer_media_t media_source = get_web_media();
		if (media_source && media_source->hasMedia())
			media_source->getMediaPlugin()->clear_cache();
	}
	return false;
}

// static
void LLPanelNetwork::onClickClearCookies(void*)
{
	LLNotifications::instance().add("ConfirmClearCookies", LLSD(), LLSD(), callback_clear_cookies);
}

//static
bool LLPanelNetwork::callback_clear_cookies(const LLSD& notification, const LLSD& response)
{
	S32 option = LLNotification::getSelectedOption(notification, response);
	if ( option == 0 ) // YES
	{
		viewer_media_t media_source = get_web_media();
		if (media_source && media_source->hasMedia())
			media_source->getMediaPlugin()->clear_cookies();
	}
	return false;
}

// static
void LLPanelNetwork::onCommitCookies(LLUICtrl* ctrl, void* data)
{
  LLPanelNetwork* self = (LLPanelNetwork*)data;
  LLCheckBoxCtrl* check = (LLCheckBoxCtrl*)ctrl;

  if (!self || !check) return;

  viewer_media_t media_source = get_web_media();
		if (media_source && media_source->hasMedia())
	  media_source->getMediaPlugin()->enable_cookies(check->get());
}

// static
void LLPanelNetwork::onCommitWebProxyEnabled(LLUICtrl* ctrl, void* data)
{
	LLPanelNetwork* self = (LLPanelNetwork*)data;
	LLCheckBoxCtrl* check = (LLCheckBoxCtrl*)ctrl;

	if (!self || !check) return;
	self->childSetEnabled("web_proxy_editor", check->get());
	self->childSetEnabled("web_proxy_port", check->get());
	self->childSetEnabled("proxy_text_label", check->get());
}

// static
void LLPanelNetwork::onClickSearchDefault(void* user_data)
{
	LLPanelNetwork* self = (LLPanelNetwork*)user_data;
	LLControlVariable* controlp = 
		(gHippoGridManager->getConnectedGrid()->isSecondLife()) 
		? 
		gSavedSettings.getControl("SearchURLQuery")
		:
		gSavedSettings.getControl("SearchURLQueryOpenSim");

	if (controlp)
	{
		self->childSetValue("world_search_editor",controlp->getDefault().asString()) ;
	}
	else
	{
		llwarns << "SearchURLQuery or SearchURLQueryOpenSim missing from settings.xml - thats bad!" << llendl;
	}
}

// static
void LLPanelNetwork::onClickSearchClear(void* user_data)
{
	LLPanelNetwork* self = (LLPanelNetwork*)user_data;
	self->childSetValue("world_search_editor","") ;
}

//static
void LLPanelNetwork::onCommitSpeedRezCheckBox(LLUICtrl* ctrl, void* user_data)
{
	LLPanelNetwork* self = (LLPanelNetwork*)user_data;
	if (self)
	{
		self->refresh();
	}
}
