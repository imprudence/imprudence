/** 
 * @file llpreviewlandmark.cpp
 * @brief LLFloaterURLDisplayList class implementation
 *
 * $LicenseInfo:firstyear=2002&license=internal$
 * 
 * Copyright (c) 2002-2008, Linden Research, Inc.
 * 
 * The following source code is PROPRIETARY AND CONFIDENTIAL. Use of
 * this source code is governed by the Linden Lab Source Code Disclosure
 * Agreement ("Agreement") previously entered between you and Linden
 * Lab. By accessing, using, copying, modifying or distributing this
 * software, you acknowledge that you have been informed of your
 * obligations under the Agreement and agree to abide by those obligations.
 * 
 * ALL LINDEN LAB SOURCE CODE IS PROVIDED "AS IS." LINDEN LAB MAKES NO
 * WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
 * COMPLETENESS OR PERFORMANCE.
 * $/LicenseInfo$
 */

#include "llviewerprecompiledheaders.h"

#include "llfloaterurldisplay.h"

#include "llpanelplace.h"
#include "llvieweruictrlfactory.h"

#include "v3dmath.h"

////////////////////////////////////////////////////////////////////////////
// LLFloaterURLDisplay


LLFloaterURLDisplay::LLFloaterURLDisplay(const LLSD& sd)
{	
	mFactoryMap["place_details_panel"] = LLCallbackMap(LLFloaterURLDisplay::createPlaceDetail, this);
	gUICtrlFactory->buildFloater(this, "floater_preview_url.xml", &getFactoryMap());
	this->setVisible(false);
}

LLFloaterURLDisplay::~LLFloaterURLDisplay()
{
}

void LLFloaterURLDisplay::displayParcelInfo(U64 region_handle, const LLVector3& pos_local)
{
	mRegionHandle = region_handle;
	mRegionPosition = pos_local;
	LLVector3d pos_global = from_region_handle(region_handle);
	pos_global += (LLVector3d)pos_local;

	LLUUID region_id;			// don't know this
	LLUUID landmark_asset_id;	// don't know this either
	mPlacePanel->displayParcelInfo(pos_local, landmark_asset_id, region_id, pos_global);

	this->setVisible(true);
	this->setFrontmost(true);
}

void LLFloaterURLDisplay::setSnapshotDisplay(const LLUUID& snapshot_id)
{
	mPlacePanel->setSnapshot(snapshot_id);
}

void LLFloaterURLDisplay::setName(const std::string& name)
{
	mPlacePanel->setName(name);
}

void LLFloaterURLDisplay::setLocationString(const std::string& name)
{
	mPlacePanel->setLocationString(name);
}

// static
void* LLFloaterURLDisplay::createPlaceDetail(void* userdata)
{
	LLFloaterURLDisplay *self = (LLFloaterURLDisplay*)userdata;
	self->mPlacePanel = new LLPanelPlace();
	gUICtrlFactory->buildPanel(self->mPlacePanel, "panel_place.xml");

	return self->mPlacePanel;
}
