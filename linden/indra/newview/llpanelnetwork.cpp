/** 
 * @file llpanelnetwork.cpp
 * @brief Network preferences panel
 *
 * Copyright (c) 2001-2007, Linden Research, Inc.
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

// file include
#include "llpanelnetwork.h"

// linden library includes
#include "llerror.h"
#include "llrect.h"
#include "llstring.h"

// project includes
#include "llbutton.h"
#include "lldirpicker.h"
#include "llui.h"
#include "lluictrlfactory.h"
#include "llresmgr.h"
#include "llsliderctrl.h"
#include "llspinctrl.h"
#include "llcheckboxctrl.h"
#include "lltextbox.h"
#include "llviewerregion.h"
#include "llviewerthrottle.h"
#include "llworld.h"
#include "llviewercontrol.h"
#include "llvieweruictrlfactory.h"
#include "llviewerwindow.h"

LLPanelNetwork::LLPanelNetwork()
{
	gUICtrlFactory->buildPanel(this, "panel_preferences_network.xml");
}

BOOL LLPanelNetwork::postBuild()
{
	LLString cache_location = gDirUtilp->getExpandedFilename(LL_PATH_CACHE, "");
	childSetText("cache_location", cache_location);
		
	childSetAction("clear_cache", onClickClearCache, this);
	childSetAction("set_cache", onClickSetCache, this);
	childSetAction("reset_cache", onClickResetCache, this);
	
	childSetEnabled("connection_port", 
			gSavedSettings.getBOOL("ConnectionPortEnabled"));
	childSetCommitCallback("connection_port_enabled", onCommitPort, this);


	refresh();

	return TRUE;
}

LLPanelNetwork::~LLPanelNetwork()
{
	// Children all cleaned up by default view destructor.
}


void LLPanelNetwork::apply()
{
}

void LLPanelNetwork::refresh()
{
	LLPanel::refresh();

	mCacheSetting = gSavedSettings.getU32("CacheSize");
	mBandwidthBPS = gSavedSettings.getF32("ThrottleBandwidthKBPS")*1024;
	mConnectionPortEnabled = gSavedSettings.getBOOL("ConnectionPortEnabled");
	mConnectionPort = gSavedSettings.getU32("ConnectionPort");
}

void LLPanelNetwork::cancel()
{
	gSavedSettings.setU32("CacheSize", mCacheSetting);
	gSavedSettings.setF32("ThrottleBandwidthKBPS", mBandwidthBPS/1024);
	gSavedSettings.setBOOL("ConnectionPortEnabled", mConnectionPortEnabled);
	gSavedSettings.setU32("ConnectionPort", mConnectionPort);
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

	LLString cur_name(gSavedSettings.getString("CacheLocation"));
	LLString proposed_name(cur_name);
	
	LLDirPicker& picker = LLDirPicker::instance();
	if (! picker.getDir(&proposed_name ) )
	{
		return; //Canceled!
	}

	LLString dir_name = picker.getDirName();
	if (!dir_name.empty() && dir_name != cur_name)
	{
		self->childSetText("cache_location", dir_name);
		gViewerWindow->alertXml("CacheWillBeMoved");
		gSavedSettings.setString("NewCacheLocation", dir_name);
	}
	else
	{
		LLString cache_location = gDirUtilp->getCacheDir();
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
	LLString cache_location = gDirUtilp->getCacheDir(true);
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
