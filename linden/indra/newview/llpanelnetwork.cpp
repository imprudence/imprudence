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

//file include
#include "llpanelnetwork.h"

// project includes
#include "llcheckboxctrl.h"
#include "lldirpicker.h"
#include "lluictrlfactory.h"
#include "llviewercontrol.h"
#include "llviewerwindow.h"

LLPanelNetwork::LLPanelNetwork()
{
	LLUICtrlFactory::getInstance()->buildPanel(this, "panel_preferences_network.xml");
}

BOOL LLPanelNetwork::postBuild()
{
	std::string cache_location = gDirUtilp->getExpandedFilename(LL_PATH_CACHE, "");
	childSetText("cache_location", cache_location);
		
	childSetAction("clear_cache", onClickClearCache, this);
	childSetAction("set_cache", onClickSetCache, this);
	childSetAction("reset_cache", onClickResetCache, this);
	
	childSetEnabled("connection_port", gSavedSettings.getBOOL("ConnectionPortEnabled"));
	childSetCommitCallback("connection_port_enabled", onCommitPort, this);

	childSetValue("cache_size", (F32)gSavedSettings.getU32("CacheSize"));
	childSetValue("max_bandwidth", gSavedSettings.getF32("ThrottleBandwidthKBPS"));
	childSetValue("connection_port_enabled", gSavedSettings.getBOOL("ConnectionPortEnabled"));
	childSetValue("connection_port", (F32)gSavedSettings.getU32("ConnectionPort"));

	return TRUE;
}

LLPanelNetwork::~LLPanelNetwork()
{
	// Children all cleaned up by default view destructor.
}


void LLPanelNetwork::apply()
{
	gSavedSettings.setU32("CacheSize", childGetValue("cache_size").asInteger());
	gSavedSettings.setF32("ThrottleBandwidthKBPS", childGetValue("max_bandwidth").asReal());
	gSavedSettings.setBOOL("ConnectionPortEnabled", childGetValue("connection_port_enabled"));
	gSavedSettings.setU32("ConnectionPort", childGetValue("connection_port").asInteger());
}

void LLPanelNetwork::cancel()
{
}

// static
void LLPanelNetwork::onClickClearCache(void*)
{
	// flag client cache for clearing next time the client runs
	gSavedSettings.setBOOL("PurgeCacheOnNextStartup", TRUE);
	gViewerWindow->alertXml("CacheWillClear");
}

// static
void LLPanelNetwork::onClickSetCache(void* user_data)
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
		self->childSetText("cache_location", dir_name);
		gViewerWindow->alertXml("CacheWillBeMoved");
		gSavedSettings.setString("NewCacheLocation", dir_name);
	}
	else
	{
		std::string cache_location = gDirUtilp->getCacheDir();
		self->childSetText("cache_location", cache_location);
	}
}

// static
void LLPanelNetwork::onClickResetCache(void* user_data)
{
 	LLPanelNetwork* self = (LLPanelNetwork*)user_data;
	if (!gSavedSettings.getString("CacheLocation").empty())
	{
		gSavedSettings.setString("NewCacheLocation", "");
		gViewerWindow->alertXml("CacheWillBeMoved");
	}
	std::string cache_location = gDirUtilp->getCacheDir(true);
	self->childSetText("cache_location", cache_location);
}

// static
void LLPanelNetwork::onCommitPort(LLUICtrl* ctrl, void* data)
{
  LLPanelNetwork* self = (LLPanelNetwork*)data;
  LLCheckBoxCtrl* check = (LLCheckBoxCtrl*)ctrl;

  if (!self || !check) return;
  self->childSetEnabled("connection_port", check->get());
  gViewerWindow->alertXml("ChangeConnectionPort");
}
