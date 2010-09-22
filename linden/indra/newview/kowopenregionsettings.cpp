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
#include "hippoLimits.h"
#include "llfloatertools.h"
#include "llviewercontrol.h"

//DEBUG includes
//#include "llsdserialize.h" //LLSDNotationStreamer - for dumping LLSD to string

class OpenRegionInfoUpdate : public LLHTTPNode
{
	/*virtual*/ void post(
		LLHTTPNode::ResponsePtr response,
		const LLSD& context,
		const LLSD& input) const
	{
		if (!input.isMap() || !input.has("body"))
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
			//IMPLEMENT ME
		}
		if ( body.has("AllowPhysicalPrims") )
		{
			//IMPLEMENT ME
		}
		if ( body.has("DrawDistance") )
		{
			//IMPLEMENT ME
		}
		if ( body.has("ForceDrawDistance") )
		{
			//IMPLEMENT ME
		}
		if ( body.has("LSLFunctions") )
		{
			//IMPLEMENT ME
		}
		if ( body.has("MaxDragDistance") )
		{
			//IMPLEMENT ME
		}
		if ( body.has("MinHoleSize") )
		{
			gHippoLimits->mMinHoleSize = body["MinHoleSize"].asReal();
		}
		if ( body.has("MaxHollowSize") )
		{
			gHippoLimits->mMaxHollow = body["MaxHollowSize"].asReal();
		}
		if ( body.has("MaxInventoryItemsTransfer") )
		{
			//IMPLEMENT ME
		}
		if ( body.has("MaxLinkCount") )
		{
			gHippoLimits->mMaxLinkedPrims = body["MaxLinkCount"].asInteger();
		}
		if ( body.has("MaxLinkCountPhys") )
		{
			//IMPLEMENT ME
		}
		if ( body.has("MaxPhysPrimScale") )
		{
			//IMPLEMENT ME
		}
		if ( body.has("MaxPos") )
		{
			//IMPLEMENT ME
		}
		if ( body.has("MaxPrimScale") )
		{
			gHippoLimits->mMaxPrimScale = body["MaxPrimScale"].asReal();
			limitschanged = TRUE;
		}
		if ( body.has("MinPos") )
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
			//IMPLEMENT ME
		}
		if ( body.has("RenderWater") )
		{
			gSavedSettings.setBOOL("RenderWater", body["RenderWater"].asBoolean());
		}
		if ( body.has("SayDistance") )
		{
			//IMPLEMENT ME
		}
		if ( body.has("ShoutDistance") )
		{
			//IMPLEMENT ME
		}
		if ( body.has("ToggleTeenMode") )
		{
			gSavedSettings.setBOOL("ToggleTeenMode", body["ToggleTeenMode"].asBoolean());

		}
		if ( body.has("WhisperDistance") )
		{
			//IMPLEMENT ME
		}

		if (limitschanged)
			gFloaterTools->updateToolsSizeLimits();
	}
};

LLHTTPRegistration<OpenRegionInfoUpdate>
gHTTPRegistrationOpenRegionInfoUpdate(
	"/message/OpenRegionInfo");