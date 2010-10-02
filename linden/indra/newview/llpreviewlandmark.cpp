/** 
 * @file llpreviewlandmark.cpp
 * @brief LLPreviewLandmark class implementation
 *
 * $LicenseInfo:firstyear=2002&license=viewergpl$
 * 
 * Copyright (c) 2002-2009, Linden Research, Inc.
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

#include "llpreviewlandmark.h"

#include "llassetstorage.h"
#include "llfocusmgr.h"
#include "llfontgl.h"
#include "llgl.h"
#include "llinventory.h"
#include "message.h"

#include "llagent.h"
#include "llbutton.h"
#include "lleconomy.h"
#include "llfloaterworldmap.h"
#include "llinventoryview.h"
#include "lliconctrl.h"
#include "lllandmarklist.h"
#include "lllineeditor.h"
#include "llpanelplace.h"
#include "llresmgr.h"
#include "llstatusbar.h"
#include "lltextbox.h"
#include "llui.h"
#include "llviewercontrol.h"
#include "llviewerregion.h"
#include "llviewerstats.h"
#include "lluictrlfactory.h"
#include "lltexturectrl.h"




////////////////////////////////////////////////////////////////////////////
// LLPreviewLandmark

// static
LLPreviewLandmarkList LLPreviewLandmark::sOrderedInstances;


LLPreviewLandmark::LLPreviewLandmark(const std::string& name,
									 const LLRect& rect,
									 const std::string& title,
									 const LLUUID& item_uuid,
									 BOOL show_keep_discard,
									 LLViewerInventoryItem* inv_item)
:	LLPreview(name,
			  LLRect(rect.mLeft, 
					rect.mTop, 
					(show_keep_discard ? rect.mRight+70 : rect.mRight), 
					rect.mBottom), 
			  title, 
			  item_uuid, 
			  LLUUID::null, // object id
			  FALSE,  // allow resize
			  0, 0, // min dimensions
			  inv_item),
	mLandmark( NULL )
{
	mFactoryMap["place_details_panel"] = LLCallbackMap(LLPreviewLandmark::createPlaceDetail, this);
	if (show_keep_discard)
	{
		// Move the notification in llviewermessage in the if/else there please.
		// This class is no more called if show_keep_discard != TRUE -- Ele

		// Decided against this, a text notification is a better fix for this -- McCabe
		//LLUICtrlFactory::getInstance()->buildFloater(this, "floater_preview_new_landmark.xml", &getFactoryMap());
		//childSetAction("Discard btn",onDiscardBtn,this);
	}
	else
	{
		LLUICtrlFactory::getInstance()->buildFloater(this, "floater_preview_existing_landmark.xml", &getFactoryMap());
	}


	/*
	childSetCommitCallback("desc_editor", LLPreview::onText, this);
	childSetText("desc_editor", item->getDescription());
	childSetText("name_editor", item->getName());
	childSetPrevalidate("desc_editor", &LLLineEditor::prevalidatePrintableNotPipe);

	setTitle(title);
	
	if (!getHost())
	{
		LLRect curRect = getRect();
		translate(rect.mLeft - curRect.mLeft, rect.mTop - curRect.mTop);
	}
	*/
	LLPreviewLandmark::sOrderedInstances.push_back( this );
}

LLPreviewLandmark::~LLPreviewLandmark()
{
	LLPreviewLandmarkList::iterator this_itr;
	this_itr = std::find(LLPreviewLandmark::sOrderedInstances.begin(), 
			LLPreviewLandmark::sOrderedInstances.end(), this);
	if (this_itr != LLPreviewLandmark::sOrderedInstances.end())
	{
		LLPreviewLandmark::sOrderedInstances.erase(this_itr);
	}
}


// Distance and direction from avatar to landmark.
// Distance is in meters; degrees is angle from east (north = 90)
void LLPreviewLandmark::getDegreesAndDist( F32* degrees, F64* horiz_dist, F64* vert_dist) const
{
	if( mLandmark )
	{
		LLVector3d pos;
		if(mLandmark->getGlobalPos(pos))
		{
			LLVector3d to_vec = pos - gAgent.getPositionGlobal();
			*horiz_dist = sqrt(to_vec.mdV[VX] * to_vec.mdV[VX] + to_vec.mdV[VY] * to_vec.mdV[VY]);
			*vert_dist = to_vec.mdV[VZ];
			*degrees = F32( RAD_TO_DEG * atan2( to_vec.mdV[VY], to_vec.mdV[VX] ) );
		}
	}
}

const std::string& LLPreviewLandmark::getName() const
{
	const LLInventoryItem *item = getItem();
	if (item)
	{
		return item->getName();
	}
	return LLStringUtil::null;
}

LLVector3d LLPreviewLandmark::getPositionGlobal() const
{
	LLVector3d pos;
	if( mLandmark )
	{
		// we can safely ignore the return value here.
		(void)mLandmark->getGlobalPos(pos);
	}
	return pos;
}


const LLColor4& LLPreviewLandmark::getMarkerColor() const
{
	return mMarkerColor;
}

void LLPreviewLandmark::draw()
{
	const LLInventoryItem *item = getItem();

	if( item && !mLandmark )
	{
		mLandmark = gLandmarkList.getAsset( item->getAssetUUID() );
		if(mLandmark && mPlacePanel)
		{
			LLVector3 pos_region = mLandmark->getRegionPos();	// always have this
			LLUUID landmark_asset_id = item->getAssetUUID();	// always have this
			LLUUID region_id;
			mLandmark->getRegionID(region_id);		// might find null?
			LLVector3d pos_global = getPositionGlobal();	// might be 0
			mPlacePanel->displayParcelInfo(pos_region, landmark_asset_id, region_id, pos_global);
		}
	}

	LLPreview::draw();
}

void LLPreviewLandmark::loadAsset()
{
	const LLInventoryItem *item = getItem();

	if( item && !mLandmark )
	{
		mLandmark = gLandmarkList.getAsset( item->getAssetUUID() );
	}
	mAssetStatus = PREVIEW_ASSET_LOADING;
}

LLPreview::EAssetStatus LLPreviewLandmark::getAssetStatus()
{
	const LLInventoryItem *item = getItem();
	if (item && gLandmarkList.assetExists(item->getAssetUUID()))
	{
		mAssetStatus = PREVIEW_ASSET_LOADED;
	}
	return mAssetStatus;
}
// static
void* LLPreviewLandmark::createPlaceDetail(void* userdata)
{
	LLPreviewLandmark *self = (LLPreviewLandmark*)userdata;
	self->mPlacePanel = new LLPanelPlace();
	LLUICtrlFactory::getInstance()->buildPanel(self->mPlacePanel, "panel_place.xml");
	const LLInventoryItem* item = self->getItem();
	self->mPlacePanel->displayItemInfo(item);

	return self->mPlacePanel;
}
