/** 
 * @file llpreviewlandmark.cpp
 * @brief LLPreviewLandmark class implementation
 *
 * Copyright (c) 2002-2007, Linden Research, Inc.
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
#include "llresmgr.h"
#include "llstatusbar.h"
#include "lltextbox.h"
#include "llui.h"
#include "llviewercontrol.h"
#include "llviewerregion.h"
#include "llviewerstats.h"
#include "viewer.h"
#include "llvieweruictrlfactory.h"




////////////////////////////////////////////////////////////////////////////
// LLPreviewLandmark

// static
//LLDoubleLinkedList<LLPreviewLandmark> LLPreviewLandmark::sOrderedInstances;
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
	
	if (show_keep_discard)
	{
		gUICtrlFactory->buildFloater(this,"floater_preview_new_landmark.xml");
		childSetAction("Discard btn",onDiscardBtn,this);
	}	
	else 
	{
		gUICtrlFactory->buildFloater(this,"floater_preview_existing_landmark.xml");
		childSetVisible("desc txt", FALSE);
		childSetEnabled("desc", FALSE);
	}

	childSetAction("Teleport btn", onTeleportBtn,this);
	childSetAction("Show on Map btn", onMapBtn,this);

	const LLInventoryItem* item = getItem();
	
	childSetCommitCallback("desc", LLPreview::onText, this);
	childSetText("desc", item->getDescription());
	childSetPrevalidate("desc", &LLLineEditor::prevalidatePrintableNotPipe);

	setTitle(title);
	
	if (!getHost())
	{
		LLRect curRect = getRect();
		translate(rect.mLeft - curRect.mLeft, rect.mTop - curRect.mTop);
	}

	LLPreviewLandmark::sOrderedInstances.push_back( this );
}

// static
void LLPreviewLandmark::onMapBtn( void* userdata )
{
	LLPreviewLandmark* self = (LLPreviewLandmark*) userdata;
	gFocusMgr.setKeyboardFocus(NULL, NULL);
	
	if( gFloaterWorldMap )
	{
		gFloaterWorldMap->trackLandmark( self->mItemUUID );
		LLFloaterWorldMap::show(NULL, TRUE);

		self->close();
	}
}

// static
void LLPreviewLandmark::onTeleportBtn( void* userdata )
{
	LLPreviewLandmark* self = (LLPreviewLandmark*) userdata;
	gFocusMgr.setKeyboardFocus(NULL, NULL);
	
	const LLInventoryItem *item = self->getItem();
	if(item)
	{
		gAgent.teleportViaLandmark(item->getAssetUUID());

		// we now automatically track the landmark you're teleporting to
		// because you'll probably arrive at a telehub instead
		if( gFloaterWorldMap )
		{
			gFloaterWorldMap->trackLandmark( self->mItemUUID );
		}
	}
	self->close();
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

const LLString& LLPreviewLandmark::getName() const
{
	const LLInventoryItem *item = getItem();
	if (item)
	{
		return item->getName();
	}
	return LLString::null;
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
	if( getVisible() )
	{
		const LLInventoryItem *item = getItem();

		if( item && !mLandmark )
		{
			mLandmark = gLandmarkList.getAsset( item->getAssetUUID() );
		}

		if(mLandmark)
		{
			F32 degrees = 0;
			F64 horiz_dist = 0;
			F64 vert_dist = 0;
			getDegreesAndDist( &degrees, &horiz_dist, &vert_dist);
			S32 offset = 180 + (360/16);
			if( S32(degrees) + offset >= 360 )
			{
				offset -= 360;
			}
#if _DEBUG
			S32 dir_index = (S32(degrees) + offset) / (360/8);
			llassert( 0 <= dir_index && dir_index <= 7 );
#endif
			// See also llfloatermap.cpp -> onLandmarkGo
			
			childSetEnabled("Teleport btn", TRUE);
			LLButton* teleportBtn = LLUICtrlFactory::getButtonByName(this, "Teleport btn");
			teleportBtn->setLabelUnselected("Teleport");
		}

		BOOL in_prelude = gAgent.inPrelude();
		childSetEnabled("Show on Map btn", !in_prelude);

		LLFloater::draw();
	}
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
