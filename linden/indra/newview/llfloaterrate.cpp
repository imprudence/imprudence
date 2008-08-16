/** 
 * @file llfloaterrate.cpp
 * @brief Dialog for rating avatars.  She's a ten!
 *
 * Copyright (c) 2002-2007, Linden Research, Inc.
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

#include "llfloaterrate.h"

#include "llcachename.h"
#include "llfontgl.h"
#include "lluuid.h"
#include "message.h"

#include "llagent.h"
#include "lltextbox.h"
#include "llbutton.h"
#include "lllineeditor.h"
#include "llradiogroup.h"
#include "llselectmgr.h"
#include "llvieweruictrlfactory.h"
#include "llviewermessage.h"
#include "llviewernetwork.h"
#include "llviewerregion.h"
#include "llviewerreputation.h"
#include "llviewerwindow.h"

// Dollar cost to change your rating of someone
const S32 RATING_COST = 25;

//static
LLFloaterRate::instance_map_t LLFloaterRate::sInstanceMap;

//-----------------------------------------------------------------------------
// Methods
//-----------------------------------------------------------------------------
LLFloaterRate::LLFloaterRate(const std::string& name, const LLUUID &id)
:	LLFloater(name),
	mAvatarID( id ),
	mLastBehavior( 0.f ),
	mLastAppearance( 0.f ),
	mLastBuilding( 0.f ),
	mReputationRequested( FALSE )
{
	gUICtrlFactory->buildFloater(this, "floater_rate.xml");

	childSetAction("OK", onClickOK, this);
	childSetAction("Cancel", onClickCancel, this);
}


LLFloaterRate::~LLFloaterRate()
{
	sInstanceMap.erase(mAvatarID);
}


void LLFloaterRate::draw()
{
	LLString name;

	// Construct the name, if possible
	char firstname[MAX_STRING];
	char lastname[MAX_STRING];
	gCacheName->getName(mAvatarID, firstname, lastname);
	name.assign(firstname);
	name.append(" ");
	name.append(lastname);

	// Request the current data, if we haven't already
	if (!mReputationRequested)
	{
		sendReputationIndividualRequest(mAvatarID);
	}

	LLString title("Your Rating of ");
	title.append( name );

	setTitle(title);

	F32 behavior   = childGetValue("behavior").asString() == "Positive" ? 1.0f : 0.0f;
	F32 appearance   = childGetValue("appearance").asString() == "Positive" ? 1.0f : 0.0f;
	F32 building   = childGetValue("building").asString() == "Positive" ? 1.0f : 0.0f;

	S32 change_count = 0;
	if (behavior != mLastBehavior) change_count++;
	if (appearance != mLastAppearance) change_count++;
	if (building != mLastBuilding) change_count++;

	childSetTextArg("cost", "[COST]", llformat("%d",RATING_COST));
	childSetTextArg("cost", "[TOTAL]", llformat("%d",RATING_COST * change_count));

	BOOL can_afford = can_afford_transaction(RATING_COST * change_count);
	BOOL changed = (change_count > 0);
	childSetEnabled("OK",  changed && can_afford );

	LLFloater::draw();
}


// static
void LLFloaterRate::show(const LLUUID &avatar_id)
{
	instance_map_t::iterator iter = sInstanceMap.find(avatar_id);
	if (iter != sInstanceMap.end())
	{
		iter->second->setFocus(TRUE);
		iter->second->open();
	}
	else if (avatar_id != LLUUID::null)
	{
		LLFloaterRate *f = new LLFloaterRate("rate", avatar_id);
		sInstanceMap[avatar_id] = f;
		f->center();
		f->setFocus(TRUE);
		f->sendReputationIndividualRequest(avatar_id);
		f->open();
	}
}


// static
void LLFloaterRate::show(ERateSelection which)
{
	LLUUID avatarid;
	LLString avatarname;
	switch(which)
	{
	  case RS_CREATOR:
		gSelectMgr->selectGetCreator(avatarid, avatarname);
		break;

	  case RS_OWNER:
	  default:
		gSelectMgr->selectGetOwner(avatarid, avatarname);
		break;
	}

	if (avatarid != LLUUID::null)
	{
		show(avatarid);
	}
	else
	{
		gViewerWindow->alertXml("SelectSingleRate");
	}


	// Clean up selection
	gSelectMgr->deselectTransient();
}


// static
void LLFloaterRate::onClickOK(void *data)
{
	LLFloaterRate *self = (LLFloaterRate *)data;
	
	F32 behavior   = self->childGetValue("behavior").asString() == "Positive" ? 1.0f : 0.0f;
	F32 appearance   = self->childGetValue("appearance").asString() == "Positive" ? 1.0f : 0.0f;
	F32 building   = self->childGetValue("building").asString() == "Positive" ? 1.0f : 0.0f;
	std::string text = self->childGetValue("mesg editor").asString();

	S32 change_count = 0;
	if (behavior != self->mLastBehavior) change_count++;
	if (appearance != self->mLastAppearance) change_count++;
	if (building != self->mLastBuilding) change_count++;

	if (change_count > 0)
	{
		send_reputation_agent_assign(gAgent.getID(), self->mAvatarID, 
									 behavior, appearance, building, text.c_str() );

		give_money(LLUUID::null, gAgent.getRegion(), change_count * RATING_COST);

		self->mLastBehavior = behavior;
		self->mLastAppearance = appearance;
		self->mLastBuilding = building;
	}

	self->close();
}


// static
void LLFloaterRate::onClickCancel(void *data) 
{
	LLFloaterRate *self = (LLFloaterRate *)data;
	self->close();
}


void LLFloaterRate::sendReputationIndividualRequest(const LLUUID &avatar_id)
{
	mReputationRequested = TRUE;

	LLMessageSystem *msg = gMessageSystem;
	msg->newMessageFast(_PREHASH_ReputationIndividualRequest);
	msg->nextBlockFast(_PREHASH_ReputationData);
	msg->addUUIDFast(_PREHASH_FromID, gAgent.getID() );
	msg->addUUIDFast(_PREHASH_ToID, avatar_id );
	msg->sendReliable( gUserServer );
}


// static
void LLFloaterRate::processReputationIndividualReply(LLMessageSystem *msg, void** data)
{
	LLUUID from_id;
	LLUUID to_id;
	F32 behavior = 0.f;
	F32 appearance = 0.f;
	F32 building = 0.f;

	msg->getUUIDFast(_PREHASH_ReputationData, _PREHASH_FromID, from_id );
	msg->getUUIDFast(_PREHASH_ReputationData, _PREHASH_ToID, to_id );
	msg->getF32Fast(_PREHASH_ReputationData, _PREHASH_Behavior, behavior );
	msg->getF32Fast(_PREHASH_ReputationData, _PREHASH_Appearance, appearance );
	msg->getF32Fast(_PREHASH_ReputationData, _PREHASH_Building, building );

	instance_map_t::iterator iter = sInstanceMap.find(to_id);
	if (iter != sInstanceMap.end())
	{
		LLFloaterRate* f = iter->second;

		f->mLastBehavior = behavior;
		f->mLastAppearance = appearance;
		f->mLastBuilding = building;

		if (behavior > 0.f)
		{
			f->childSetValue("behavior", "Positive");
		}
		else
		{	
			f->childSetValue("behavior", "No Rating");
		}

		if (appearance > 0.f)
		{
			f->childSetValue("appearance", "Positive");
		}
		else
		{	
			f->childSetValue("appearance", "No Rating");
		}

		if (building > 0.f)
		{
			f->childSetValue("building", "Positive");
		}
		else
		{	
			f->childSetValue("building", "No Rating");
		}
	}
}
