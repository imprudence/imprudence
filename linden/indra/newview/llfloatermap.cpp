/** 
 * @file llfloatermap.cpp
 * @brief The "mini-map" or radar in the upper right part of the screen.
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

#include "llfloatermap.h"

#include "llagent.h"
#include "llcolorscheme.h"
#include "llviewercontrol.h"
#include "lldraghandle.h"
#include "llnetmap.h"
#include "llregionhandle.h"
#include "llresizebar.h"
#include "lluictrlfactory.h"

// radar
#include "llfloateravatarinfo.h"
#include "llfloaterfriends.h"
#include "llfloatergroupinvite.h"
#include "llfloatergroups.h"
#include "llimview.h"
#include "roles_constants.h"
#include "llscrolllistctrl.h"
#include "lltracker.h"
#include "llviewermessage.h"
#include "llworld.h"

LLFloaterMap* LLFloaterMap::sInstance = NULL;

LLFloaterMap::LLFloaterMap(const LLSD& key)
	:
	LLFloater(std::string("minimap")),
	mPanelMap(NULL)
{
	LLCallbackMap::map_t factory_map;
	factory_map["mini_mapview"] = LLCallbackMap(createPanelMiniMap, this);
	LLUICtrlFactory::getInstance()->buildFloater(this, "floater_mini_map.xml", &factory_map, FALSE);
	sInstance = this;
}


// static
void* LLFloaterMap::createPanelMiniMap(void* data)
{
	LLFloaterMap* self = (LLFloaterMap*)data;
	self->mPanelMap = new LLNetMap("Mapview");
	self->mSelectedAvatar.setNull();
	return self->mPanelMap;
}

BOOL LLFloaterMap::postBuild()
{
	// Send the drag handle to the back, but make sure close stays on top
	sendChildToBack(getDragHandle());
	sendChildToFront(getChild<LLButton>("llfloater_close_btn"));
	setIsChrome(TRUE);
	
	mRadarList = getChild<LLScrollListCtrl>("RadarList");
	childSetCommitCallback("RadarList", onList, this);

	childSetAction("im_btn", onClickIM, this);
	childSetAction("profile_btn", onClickProfile, this);
	childSetAction("offer_teleport_btn", onClickOfferTeleport, this);
	childSetAction("track_btn", onClickTrack, this);
	childSetAction("invite_btn", onClickInvite, this);
	childSetAction("add_btn", onClickAddFriend, this);

	setDefaultBtn("im_btn");

	populateRadar();

	return TRUE;
}


LLFloaterMap::~LLFloaterMap()
{
	sInstance = NULL;
}


// virtual 
void LLFloaterMap::onOpen()
{
	gFloaterView->adjustToFitScreen(this, FALSE);

	gSavedSettings.setBOOL("ShowMiniMap", TRUE);
}


// virtual
void LLFloaterMap::onClose(bool app_quitting)
{
	LLFloater::setVisible(FALSE);

	if (!app_quitting)
	{
		gSavedSettings.setBOOL("ShowMiniMap", FALSE);
	}
}

BOOL LLFloaterMap::canClose()
{
	return !LLApp::isExiting();
}


// virtual
void LLFloaterMap::draw()
{
	// Note: we can't just gAgent.check cameraMouselook() because the transition states are wrong.
	if( gAgent.cameraMouselook())
	{
		setMouseOpaque(FALSE);
		getDragHandle()->setMouseOpaque(FALSE);

		drawChild(mPanelMap);
	}
	else
	{
		setMouseOpaque(TRUE);
		getDragHandle()->setMouseOpaque(TRUE);

		LLFloater::draw();
	}
}


//
// Radar
//

//static
void LLFloaterMap::updateRadar()
{
	LLFloaterMap* self = sInstance;
	self->populateRadar();
}

void LLFloaterMap::populateRadar()
{
	BOOL empty = TRUE;
	std::stringstream avatar_count_string;
	LLScrollListCtrl* radar_scroller = getChild<LLScrollListCtrl>("RadarList");
	radar_scroller->deleteAllItems();

	LLVector3d current_pos = gAgent.getPositionGlobal();

	std::vector<LLUUID> avatar_ids;
	std::vector<LLVector3d> positions;
	LLWorld::getInstance()->getAvatars(&avatar_ids, &positions, current_pos, gSavedSettings.getF32("NearMeRange"));
	for(U32 i=0; i<avatar_ids.size(); i++)
	{
		LLUUID& av = avatar_ids[i];

		if(av == gAgent.getID()) 
			continue;

		LLVector3d avatar_pos = positions[i];
		F64 distance = dist_vec(current_pos, avatar_pos);
		std::stringstream dist_formatted;
		dist_formatted << (double)((int)((distance + 0.05)*10.0))/10.0 << "m";

		LLSD element;
		element["id"] = av; // value
		element["columns"][0]["column"] = "avatar_name";
		element["columns"][0]["type"] = "text";

		std::string fullname;
		if(!gCacheName->getFullName(av, fullname))
		{
			element["columns"][0]["value"] = LLCacheName::getDefaultName();
		}			
		else
		{
			element["columns"][0]["value"] = fullname;
		}

		element["columns"][1]["column"] = "avatar_distance";
		element["columns"][1]["type"] = "text";
		element["columns"][1]["value"] = dist_formatted.str();
		radar_scroller->addElement(element);
		empty = FALSE;
	}
	
	avatar_count_string.str("");
	if (empty)
	{
		childSetEnabled("RadarList", false);childSetEnabled("im_btn", false);
		radar_scroller->addCommentText(getString("no_one_near"));
		avatar_count_string << "0";
	}
	else 
	{
		childSetEnabled("RadarList", true);
		radar_scroller->selectFirstItem();		
		avatar_count_string << (int)avatar_ids.size();
	}
	LLTextBox* lblAvatarCount = getChild<LLTextBox>("lblAvatarCount");
	lblAvatarCount->setText(avatar_count_string.str());

	onList(radar_scroller, this);
}

// static
void LLFloaterMap::onList(LLUICtrl* ctrl, void* user_data)
{
	LLFloaterMap* self = (LLFloaterMap*)user_data;
	if (self)
	{
		self->childSetEnabled("im_btn", self->visibleItemsSelected());
		self->childSetEnabled("profile_btn", self->visibleItemsSelected());
		self->childSetEnabled("offer_teleport_btn", self->visibleItemsSelected());
		self->childSetEnabled("track_btn", self->visibleItemsSelected());
		self->childSetEnabled("invite_btn", self->visibleItemsSelected());
		self->childSetEnabled("add_btn", self->visibleItemsSelected());

		if (self->visibleItemsSelected())
		{
			self->mSelectedAvatar = self->mRadarList->getFirstSelected()->getUUID();
		}
		else
		{	
			self->mSelectedAvatar.setNull();
		}
	}
}

BOOL LLFloaterMap::visibleItemsSelected() const
{
	if (mRadarList->getFirstSelectedIndex() >= 0)
	{
		return TRUE;
	}
	return FALSE;
}

// static
BOOL LLFloaterMap::isSelected(LLUUID agent)
{
	if (sInstance->mSelectedAvatar == agent)
	{
		return TRUE;
	}
	return FALSE;
}

// static
void LLFloaterMap::onClickIM(void* user_data)
{
	LLFloaterMap* self = (LLFloaterMap*) user_data;
	LLScrollListItem *item = self->mRadarList->getFirstSelected();
	if (item != NULL)
	{
		LLUUID agent_id = item->getUUID();
		std::string fullname;
		if(gCacheName->getFullName(agent_id, fullname))
		{
			gIMMgr->setFloaterOpen(TRUE);
			gIMMgr->addSession(fullname, IM_NOTHING_SPECIAL, agent_id);
		}
	}
}

// static
void LLFloaterMap::onClickProfile(void* user_data)
{
	LLFloaterMap* self = (LLFloaterMap*) user_data;
	LLScrollListItem *item = self->mRadarList->getFirstSelected();
	if (item != NULL)
	{
		LLUUID agent_id = item->getUUID();
		LLFloaterAvatarInfo::show(agent_id);
	}
}

// static
void LLFloaterMap::onClickOfferTeleport(void* user_data)
{
	LLFloaterMap* self = (LLFloaterMap*) user_data;
	LLScrollListItem *item = self->mRadarList->getFirstSelected();
	if (item != NULL)
	{
		LLUUID agent_id = item->getUUID();
		handle_lure(agent_id);
	}
}

// static
void LLFloaterMap::onClickTrack(void* user_data)
{
	LLFloaterMap* self = (LLFloaterMap*) user_data;
	LLTracker::ETrackingStatus tracking_status = LLTracker::getTrackingStatus();
	if (LLTracker::TRACKING_AVATAR == tracking_status)
	{
		LLTracker::stopTracking(NULL);
	}
	else
	{
 		LLScrollListItem *item = self->mRadarList->getFirstSelected();
		if (item != NULL)
		{
			LLUUID agent_id = item->getUUID();
			std::string fullname;
			gCacheName->getFullName(agent_id, fullname);
			LLTracker::trackAvatar(agent_id, fullname);
		}
	}
}

// static
void LLFloaterMap::onClickInvite(void* user_data)
{
	LLFloaterMap* self = (LLFloaterMap*) user_data;
	LLScrollListItem *item = self->mRadarList->getFirstSelected();
	if (item != NULL)
	{
		LLUUID agent_id = item->getUUID();
		LLFloaterGroupPicker* widget;
		widget = LLFloaterGroupPicker::showInstance(LLSD(gAgent.getID()));
		if (widget)
		{
			widget->center();
			widget->setPowersMask(GP_MEMBER_INVITE);
			widget->setSelectCallback(callback_invite_to_group, (void *)&agent_id);
		}
	}
}

// static
void LLFloaterMap::callback_invite_to_group(LLUUID group_id, void *user_data)
{
	std::vector<LLUUID> agent_ids;
	agent_ids.push_back(*(LLUUID *)user_data);
	
	LLFloaterGroupInvite::showForGroup(group_id, &agent_ids);
}

// static
void LLFloaterMap::onClickAddFriend(void* user_data)
{
	LLFloaterMap* self = (LLFloaterMap*) user_data;
	LLScrollListItem *item = self->mRadarList->getFirstSelected();
	if (item != NULL)
	{
		LLUUID agent_id = item->getUUID();
		std::string fullname;
		gCacheName->getFullName(agent_id, fullname);
		LLPanelFriends::requestFriendshipDialog(agent_id, fullname);
	}
}
