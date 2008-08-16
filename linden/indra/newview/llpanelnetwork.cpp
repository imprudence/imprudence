/** 
 * @file llpanelnetwork.cpp
 * @brief Network preferences panel
 *
 * Copyright (c) 2001-2007, Linden Research, Inc.
 * 
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
#include "llui.h"
#include "lluictrlfactory.h"
#include "llresmgr.h"
#include "llsliderctrl.h"
#include "lltextbox.h"
#include "llviewerregion.h"
#include "llviewerthrottle.h"
#include "llworld.h"
#include "llviewercontrol.h"
#include "llvieweruictrlfactory.h"
#include "llmozlib.h"
#include "llviewerwindow.h"


LLPanelNetwork::LLPanelNetwork()
{
	gUICtrlFactory->buildPanel(this, "panel_preferences_network.xml");
}

BOOL LLPanelNetwork::postBuild()
{
	requires("disk cache", WIDGET_TYPE_RADIO_GROUP);
	requires("max_bandwidth", WIDGET_TYPE_SLIDER);
	requires("clear_cache", WIDGET_TYPE_BUTTON);

	if (!checkRequirements())
	{
		return FALSE;
	}

	// retrieve controls
	mDiskCacheRadio = LLUICtrlFactory::getRadioGroupByName(this, "disk cache");
	mCtrlBandwidth = LLUICtrlFactory::getSliderByName(this, "max_bandwidth");

	mClearCacheBtn = LLUICtrlFactory::getButtonByName(this, "clear_cache");
	mClearCacheBtn->setClickedCallback(onClickClearCache);
	mClearCacheBtn->setCallbackUserData(this);

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

	mCacheSetting = gSavedSettings.getU32("VFSSize");
	mBandwidthBPS = gSavedSettings.getF32("ThrottleBandwidthKBPS")*1024;
}

void LLPanelNetwork::cancel()
{
	gSavedSettings.setU32("VFSSize", mCacheSetting);
	gSavedSettings.setF32("ThrottleBandwidthKBPS", mBandwidthBPS/1024);
}

// static
void LLPanelNetwork::onClickClearCache(void*)
{
#if LL_LIBXUL_ENABLED
	// clear Mozilla cache
	LLMozLib::getInstance()->clearCache();
#endif // LL_LIBXUL_ENABLED

	// flag client cache for clearing next time the client runs
	gSavedSettings.setBOOL("PurgeCacheOnNextStartup", TRUE);
	gViewerWindow->alertXml("CacheWillClear");
}
