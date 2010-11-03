/*
 * @file kowopenregionsettings.cpp
 * @brief Handler for OpenRegionInfo event queue message.
 *
 * Copyright (c) 2010, Patrick Sapinski
 *
 * The source code in this file ("Source Code") is provided to you
 * under the terms of the GNU General Public License, version 2.0
 * ("GPL"). Terms of the GPL can be found in doc/GPL-license.txt in
 * this distribution, or online at
 * http://secondlifegrid.net/programs/open_source/licensing/gplv2
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
 * ALL SOURCE CODE IS PROVIDED "AS IS." THE AUTHOR MAKES NO
 * WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
 * COMPLETENESS OR PERFORMANCE.
 */

#include "llviewerprecompiledheaders.h"
#include "llhttpnode.h"
#include "hippolimits.h"
#include "llfloatertools.h"
#include "llviewercontrol.h"
#include "llagent.h"
#include "llsurface.h"
#include "llviewerregion.h"
#include "llviewerobject.h"
#include "llfloaterregioninfo.h"
#include "llfloaterworldmap.h"

//DEBUG includes
//#include "llsdserialize.h" //LLSDNotationStreamer - for dumping LLSD to string

class OpenRegionInfoUpdate : public LLHTTPNode
{
	/*virtual*/ void post(
		LLHTTPNode::ResponsePtr response,
		const LLSD& context,
		const LLSD& input) const
	{
		if (!input || !context || !input.isMap() || !input.has("body"))
		{
			llinfos << "malformed OpenRegionInfo update!" << llendl;	 
			return;
		}

		LLSD body = input["body"];
		//llinfos << "data: " << LLSDNotationStreamer(body) << llendl;	 
		//llinfos << "data: " << LLSDXMLStreamer(body) << llendl;	 	 
		
		//set the default limits/settings for this simulator type, as limits from our
		//previous region may not exist in this one
		gHippoLimits->setLimits();

		//limits are also reset in llviewermessage.cpp when we detect a new server
		//version, in case the destination simulator does not support OpenRegionInfo.

		BOOL limitschanged = FALSE;

		if ( body.has("AllowMinimap") )
		{
			gHippoLimits->mAllowMinimap = body["AllowMinimap"].asInteger() == 1;
		}
		if ( body.has("AllowPhysicalPrims") )
		{
			gHippoLimits->mAllowPhysicalPrims = body["AllowPhysicalPrims"].asInteger() == 1;
			limitschanged = TRUE;
		}
		if ( body.has("DrawDistance") )
		{
			F32 distance = body["DrawDistance"].asReal();
			if (distance > 0)
			{
				gAgent.mDrawDistance = distance;
			}
		}
		if ( body.has("ForceDrawDistance") )
		{
			gAgent.mLockedDrawDistance = body["ForceDrawDistance"].asInteger() == 1;
		}
		if ( body.has("LSLFunctions") )
		{
			//IMPLEMENT ME
		}
		if ( body.has("MaxDragDistance") )
		{
			gHippoLimits->mMaxDragDistance = body["MaxDragDistance"].asReal();
		}
		if ( body.has("MinHoleSize") )
		{
			//Note: does NOT update correctly
			gHippoLimits->mMinHoleSize = body["MinHoleSize"].asReal();
			limitschanged = TRUE;
		}
		if ( body.has("MaxHollowSize") )
		{
			//Note: does NOT update correctly
			gHippoLimits->mMaxHollow = body["MaxHollowSize"].asReal();
			limitschanged = TRUE;
		}
		if ( body.has("MaxInventoryItemsTransfer") )
		{
			gHippoLimits->mMaxInventoryItemsTransfer = body["MaxInventoryItemsTransfer"].asReal();
		}
		if ( body.has("MaxLinkCount") )
		{
			gHippoLimits->mMaxLinkedPrims = body["MaxLinkCount"].asInteger();
		}
		if ( body.has("MaxLinkCountPhys") )
		{
			gHippoLimits->mMaxPhysLinkedPrims = body["MaxLinkCountPhys"].asInteger();
		}
		if ( body.has("MaxPos") )
		{
			gHippoLimits->mMaxPrimXPos = body["MaxPosX"].asReal();
			gHippoLimits->mMaxPrimYPos = body["MaxPosY"].asReal();
			gHippoLimits->mMaxPrimZPos = body["MaxPosZ"].asReal();
			limitschanged = TRUE;
		}
		if ( body.has("MinPos") )
		{
			gHippoLimits->mMinPrimXPos = body["MinPosX"].asReal();
			gHippoLimits->mMinPrimYPos = body["MinPosY"].asReal();
			gHippoLimits->mMinPrimZPos = body["MinPosZ"].asReal();
			limitschanged = TRUE;
		}
		if ( body.has("MaxPrimScale") )
		{
			gHippoLimits->mMaxPrimScale = body["MaxPrimScale"].asReal();
			limitschanged = TRUE;
		}
		if ( body.has("MaxPhysPrimScale") )
		{
			//IMPLEMENT ME
		}
		if ( body.has("MinPrimScale") )
		{
			gHippoLimits->mMinPrimScale = body["MinPrimScale"].asReal();
			limitschanged = TRUE;
		}
		if ( body.has("OffsetOfUTC") )
		{
			gSavedSettings.setS32("TimeOffset", body["OffsetOfUTC"].asReal());
			gSavedSettings.setBOOL("UseTimeOffset", true);
		}
		if ( body.has("OffsetOfUTCDST") )
		{
			gSavedSettings.setBOOL("TimeOffsetDST", body["OffsetOfUTCDST"].asInteger() == 1 ? TRUE : FALSE);
		}
		if ( body.has("RenderWater") )
		{
			gHippoLimits->mRenderWater = body["RenderWater"].asInteger() == 1 ? TRUE : FALSE;
			gAgent.getRegion()->rebuildWater();
		}
		if ( body.has("SayDistance") )
		{
			gSavedSettings.setU32("ChatDistance", body["SayDistance"].asReal());
		}
		if ( body.has("ShoutDistance") )
		{
			//IMPLEMENT ME
		}
		if ( body.has("WhisperDistance") )
		{
			//IMPLEMENT ME
		}
		if ( body.has("ToggleTeenMode") )
		{
			gHippoLimits->mEnableTeenMode = body["ToggleTeenMode"].asInteger() == 1 ? TRUE : FALSE;
		}
		if ( body.has("SetTeenMode") )
		{
			gAgent.setTeen( body["SetTeenMode"].asInteger() == 1 ? TRUE : FALSE );
			LLFloaterWorldMap::reloadIcons(NULL);
			llinfos << "PG status set to " << (S32)gAgent.isTeen() << llendl;
		}
		if ( body.has("ShowTags") )
		{
			gHippoLimits->mRenderName = body["ShowTags"].asReal();
		}
		if ( body.has("EnforceMaxBuild") )
		{
			gHippoLimits->mEnforceMaxBuild = body["EnforceMaxBuild"].asInteger() == 1 ? TRUE : FALSE;
			limitschanged = TRUE;
		}
		if ( body.has("MaxGroups") )
		{
			gHippoLimits->mMaxAgentGroups = body["MaxGroups"].asReal();
		}
		if ( body.has("AllowParcelWindLight") )
		{
			gHippoLimits->mAllowParcelWindLight = body["AllowParcelWindLight"].asInteger() == 1;
		}

		if (limitschanged)
			gFloaterTools->updateToolsSizeLimits();

		//Update the floater if its around
		LLPanelRegionOpenSettingsInfo* floater = LLFloaterRegionInfo::getPanelOpenSettings();

		if (floater != NULL)
		{
			floater->refreshFromRegion(gAgent.getRegion());
		}
	}
};

LLHTTPRegistration<OpenRegionInfoUpdate>
gHTTPRegistrationOpenRegionInfoUpdate(
	"/message/OpenRegionInfo");