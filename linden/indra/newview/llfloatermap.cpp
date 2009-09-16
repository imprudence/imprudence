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
#include "llfloaterreporter.h"
#include "llimview.h"
#include "llmutelist.h"
#include "roles_constants.h"
#include "llscrolllistctrl.h"
#include "lltracker.h"
#include "llviewerobjectlist.h"
#include "llviewermessage.h"
#include "llviewerregion.h"
#include "llviewerwindow.h"
#include "llworld.h"

LLFloaterMap::LLFloaterMap(const LLSD& key)
	:
	LLFloater(std::string("minimap")),
	mPanelMap(NULL),
	mUpdate(TRUE)
{
	LLCallbackMap::map_t factory_map;
	factory_map["mini_mapview"] = LLCallbackMap(createPanelMiniMap, this);
	LLUICtrlFactory::getInstance()->buildFloater(this, "floater_mini_map.xml", &factory_map, FALSE);

	mSelectedAvatar.setNull();
	mAvatars.clear();
}


// static
void* LLFloaterMap::createPanelMiniMap(void* data)
{
	LLFloaterMap* self = (LLFloaterMap*)data;
	self->mPanelMap = new LLNetMap("Mapview");
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
	mRadarList->setDoubleClickCallback(onClickIM);

	childSetFocusChangedCallback("near_me_range", onRangeChange, this);

	childSetAction("im_btn", onClickIM, this);
	childSetAction("profile_btn", onClickProfile, this);
	childSetAction("offer_teleport_btn", onClickOfferTeleport, this);
	childSetAction("track_btn", onClickTrack, this);
	childSetAction("invite_btn", onClickInvite, this);
	childSetAction("add_btn", onClickAddFriend, this);
	childSetAction("freeze_btn", onClickFreeze, this);
	childSetAction("eject_btn", onClickEject, this);
	childSetAction("mute_btn", onClickMute, this);
	childSetAction("unmute_btn", onClickUnmute, this);
	childSetAction("ar_btn", onClickAR, this);
	childSetAction("estate_eject_btn", onClickEjectFromEstate, this);

	setDefaultBtn("im_btn");

	populateRadar();

	return TRUE;
}


LLFloaterMap::~LLFloaterMap()
{
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

// [RLVa:KB] - Version: 1.22.11 | Checked: 2009-07-05 (RLVa-1.0.0c)
void LLFloaterMap::open()
{
	if (!gRlvHandler.hasBehaviour(RLV_BHVR_SHOWMINIMAP))
	{
		LLFloater::open();
	}
}
// [/RLVa:KB]

// TODO: make this detachable
// TODO: make this expand/collapse

/*
* Imprudence Radar
* @brief inworld radar integrated with the minimap
* by McCabe Maxsted
* Estate tab portion by Dale Glass
*/

//static
void LLFloaterMap::updateRadar()
{
	LLFloaterMap::getInstance()->populateRadar();
}

void LLFloaterMap::populateRadar()
{
	if (!mUpdate)
	{
		return;
	}

	if (visibleItemsSelected())
	{
		mSelectedAvatar = mRadarList->getFirstSelected()->getUUID();
	}
	else
	{
		mSelectedAvatar.setNull();
	}

	S32 scroll_pos = mRadarList->getScrollPos();

	LLVector3d current_pos = gAgent.getPositionGlobal();

	// find what avatars you can see
	std::vector<LLUUID> avatar_ids;
	std::vector<LLVector3d> positions;
	LLWorld::getInstance()->getAvatars(&avatar_ids, &positions, current_pos, gSavedSettings.getF32("NearMeRange"));

	// Add avatars to the list. If they're already there, update positions
	std::pair<std::map<LLUUID, LLVector3d>::iterator, bool> ret;
	for (U32 i=0; i<avatar_ids.size(); i++)
	{
		if (avatar_ids[i] == gAgent.getID() ||
			avatar_ids[i].isNull())
		{
			continue;
		}

		ret = mAvatars.insert(std::pair<LLUUID, LLVector3d>(avatar_ids[i], positions[i]));
		if (ret.second == false)
		{
			mAvatars[avatar_ids[i]] = positions[i];	
		}
	}

	LLSD element;

	mRadarList->deleteAllItems();
	
	// if an avatar's not in range anymore, kill it. Otherwise, populate radar
	std::map<LLUUID, LLVector3d>::iterator mIt;
	std::vector<LLUUID>::iterator result;
	for (mIt = mAvatars.begin(); mIt != mAvatars.end(); )
	{
		result = find(avatar_ids.begin(), avatar_ids.end(), mIt->first);
		if (result == avatar_ids.end())
		{
			mAvatars.erase(mIt++);
		}
		else
		{
			// Add to list only if we get their name
			std::string fullname = getSelectedName(mIt->first);
			if (!fullname.empty() && fullname != " ")
			{
				std::string mute_text = LLMuteList::getInstance()->isMuted(mIt->first) ? getString("muted") : "";
				element["id"] = mIt->first;
				element["columns"][0]["column"] = "avatar_name";
				element["columns"][0]["type"] = "text";
				element["columns"][0]["value"] = fullname + " " + mute_text;
				element["columns"][1]["column"] = "avatar_distance";
				element["columns"][1]["type"] = "text";

				F64 distance = dist_vec(current_pos, mIt->second);
				std::stringstream dist_formatted;
				dist_formatted.str("");
				dist_formatted << (double)((int)((distance + 0.05)*10.0))/10.0 << "m";
				element["columns"][1]["value"] = dist_formatted.str();

				mRadarList->addElement(element, ADD_BOTTOM);
			}
			++mIt;
		}
	}

	mRadarList->sortItems();
	mRadarList->setScrollPos(scroll_pos);
	mRadarList->selectByID(mSelectedAvatar);

	// set count
	std::stringstream avatar_count; 
	avatar_count.str("");
	if (mAvatars.empty())
	{
		mRadarList->addCommentText(getString("no_one_near"), ADD_TOP);
		avatar_count << "0";
	}
	else
	{
		avatar_count << (int)mAvatars.size();
	}
	childSetText("lblAvatarCount", avatar_count.str());

	toggleButtons();

	//llinfos << "mSelectedAvatar: " << mSelectedAvatar.asString() << llendl;
}	

void LLFloaterMap::toggleButtons()
{
	BOOL enabled = mSelectedAvatar.notNull() ? visibleItemsSelected() : FALSE;
	BOOL unmute_enabled = mSelectedAvatar.notNull() ? LLMuteList::getInstance()->isMuted(mSelectedAvatar) : FALSE;

	childSetEnabled("im_btn", enabled);
	childSetEnabled("profile_btn", enabled);
	childSetEnabled("offer_teleport_btn", enabled);
	childSetEnabled("track_btn", enabled);
	childSetEnabled("invite_btn", enabled);
	childSetEnabled("add_btn", enabled);
	childSetEnabled("freeze_btn", enabled);
	childSetEnabled("eject_btn", enabled);
	childSetEnabled("mute_btn", enabled);
	childSetEnabled("unmute_btn", unmute_enabled);
	childSetEnabled("ar_btn", enabled);
	childSetEnabled("estate_eject_btn", enabled);
}

// static
void LLFloaterMap::onList(LLUICtrl* ctrl, void* user_data)
{
	LLFloaterMap* self = (LLFloaterMap*)user_data;
	if (self)
	{
		self->toggleButtons();
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
void LLFloaterMap::onRangeChange(LLFocusableElement* focus, void* user_data)
{
	LLFloaterMap* self = (LLFloaterMap*)user_data;
	if (self)
	{
		self->mUpdate = !(self->childHasFocus("near_me_range"));
	}
}

// static
LLUUID LLFloaterMap::getSelected()
{
	return LLFloaterMap::getInstance()->mSelectedAvatar;
}

//
// Avatar tab
//

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

//
// Estate tab
//

//static 
std::string LLFloaterMap::getSelectedName(LLUUID agent_id)
{
	std::string agent_name;
	if(agent_id.notNull() && gCacheName->getFullName(agent_id, agent_name))
	{
		return agent_name;
	}
	return LLStringUtil::null;
}

//static 
void LLFloaterMap::callbackFreeze(S32 option, void *user_data)
{
	LLFloaterMap *self = (LLFloaterMap*)user_data;

	if ( option == 0 )
	{
		sendFreeze(self->mSelectedAvatar, true);
	}
	else if ( option == 1 )
	{
		sendFreeze(self->mSelectedAvatar, false);
	}
}

//static 
void LLFloaterMap::callbackEject(S32 option, void *user_data)
{
	LLFloaterMap *self = (LLFloaterMap*)user_data;
 
	if ( option == 0 )
	{
		sendEject(self->mSelectedAvatar, false);
	}
	else if ( option == 1 )
	{
		sendEject(self->mSelectedAvatar, true);
	}
}

//static 
void LLFloaterMap::callbackEjectFromEstate(S32 option, void *user_data)
{
	LLFloaterMap *self = (LLFloaterMap*)user_data;

	if ( option == 0 )
	{
		cmdEstateEject(self->mSelectedAvatar);
	} 
	else if ( option == 1 )
	{
		cmdEstateBan(self->mSelectedAvatar);
	}
}

void LLFloaterMap::onClickFreeze(void *user_data)
{
	LLFloaterMap *self = (LLFloaterMap*)user_data;
	LLStringUtil::format_map_t args;
	LLSD payload;
	args["[AVATAR_NAME]"] = self->getSelectedName(self->mSelectedAvatar);
	gViewerWindow->alertXml("FreezeAvatarFullname",	args, callbackFreeze, user_data);
}

//static
void LLFloaterMap::onClickEject(void *user_data)
{
	LLFloaterMap *self = (LLFloaterMap*)user_data;
	LLStringUtil::format_map_t args;
	LLSD payload;
	args["AVATAR_NAME"] = self->getSelectedName(self->mSelectedAvatar);
	gViewerWindow->alertXml("EjectAvatarFullName", args, callbackEject, user_data);
}

//static
void LLFloaterMap::onClickMute(void *user_data)
{
	LLFloaterMap *self = (LLFloaterMap*)user_data;

	LLUUID agent_id = self->mSelectedAvatar;
	std::string agent_name = self->getSelectedName(agent_id);
	if(!agent_name.empty())
	{
		if (LLMuteList::getInstance()->isMuted(agent_id))
		{
			//LLMute mute(agent_id, agent_name, LLMute::AGENT);
			//LLMuteList::getInstance()->remove(mute);	
			//LLFloaterMute::getInstance()->selectMute(agent_id);
		}
		else
		{
			LLMute mute(agent_id, agent_name, LLMute::AGENT);
			LLMuteList::getInstance()->add(mute);
		}
	}
}

//static
void LLFloaterMap::onClickUnmute(void *user_data)
{
	LLFloaterMap *self = (LLFloaterMap*)user_data;

	LLUUID agent_id = self->mSelectedAvatar;
	std::string agent_name = self->getSelectedName(agent_id);
	if(!agent_name.empty())
	{
		if (LLMuteList::getInstance()->isMuted(agent_id))
		{
			LLMute mute(agent_id, agent_name, LLMute::AGENT);
			LLMuteList::getInstance()->remove(mute);	
			//LLFloaterMute::getInstance()->selectMute(agent_id);
		}
		else
		{
			//LLMute mute(agent_id, agent_name, LLMute::AGENT);
			//LLMuteList::getInstance()->add(mute);
		}
	}
}

//static
void LLFloaterMap::onClickEjectFromEstate(void *user_data)
{
	LLFloaterMap *self = (LLFloaterMap*)user_data;
	LLStringUtil::format_map_t args;
	LLSD payload;
	args["EVIL_USER"] = self->getSelectedName(self->mSelectedAvatar);
	gViewerWindow->alertXml("EstateKickUser", args, callbackEjectFromEstate, user_data);
}

//static
void LLFloaterMap::onClickAR(void *user_data)
{
	LLFloaterMap *self = (LLFloaterMap*)user_data;
	LLUUID agent_id = self->mSelectedAvatar;
		
	if (agent_id.notNull())
	{
			LLFloaterReporter::showFromObject(agent_id);
	}
}

// static 
void LLFloaterMap::cmdEstateEject(const LLUUID &avatar)
{ 
	sendEstateMessage("teleporthomeuser", avatar); 
}

// static 
void LLFloaterMap::cmdEstateBan(const LLUUID &avatar)
{
	sendEstateMessage("teleporthomeuser", avatar); // Kick first, just to be sure
	sendEstateBan(avatar);
}

// static 
void LLFloaterMap::sendFreeze(const LLUUID& avatar_id, bool freeze)
{
	U32 flags = 0x0;
	if (!freeze)
	{
		// unfreeze
		flags |= 0x1;
	}

	LLMessageSystem* msg = gMessageSystem;
	LLViewerObject* avatar = gObjectList.findObject(avatar_id);

	if (avatar)
	{
		msg->newMessage("FreezeUser");
		msg->nextBlock("AgentData");
		msg->addUUID("AgentID", gAgent.getID());
		msg->addUUID("SessionID", gAgent.getSessionID());
		msg->nextBlock("Data");
		msg->addUUID("TargetID", avatar_id );
		msg->addU32("Flags", flags );
		msg->sendReliable( avatar->getRegion()->getHost() );
	}
}

// static 
void LLFloaterMap::sendEject(const LLUUID& avatar_id, bool ban)
{	
	LLMessageSystem* msg = gMessageSystem;
	LLViewerObject* avatar = gObjectList.findObject(avatar_id);

	if (avatar)
	{
		U32 flags = 0x0;
		if ( ban )
		{
			// eject and add to ban list
			flags |= 0x1;
		}

		msg->newMessage("EjectUser");
		msg->nextBlock("AgentData");
		msg->addUUID("AgentID", gAgent.getID() );
		msg->addUUID("SessionID", gAgent.getSessionID() );
		msg->nextBlock("Data");
		msg->addUUID("TargetID", avatar_id );
		msg->addU32("Flags", flags );
		msg->sendReliable( avatar->getRegion()->getHost() );
	}
}

// static 
void LLFloaterMap::sendEstateMessage(const char* request, const LLUUID &target)
{

	LLMessageSystem* msg = gMessageSystem;
	LLUUID invoice;

	// This seems to provide an ID so that the sim can say which request it's
	// replying to. I think this can be ignored for now.
	invoice.generate();

	llinfos << "Sending estate request '" << request << "'" << llendl;
	msg->newMessage("EstateOwnerMessage");
	msg->nextBlockFast(_PREHASH_AgentData);
	msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
	msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
	msg->addUUIDFast(_PREHASH_TransactionID, LLUUID::null); //not used
	msg->nextBlock("MethodData");
	msg->addString("Method", request);
	msg->addUUID("Invoice", invoice);

	// Agent id
	msg->nextBlock("ParamList");
	msg->addString("Parameter", gAgent.getID().asString().c_str());

	// Target
	msg->nextBlock("ParamList");
	msg->addString("Parameter", target.asString().c_str());

	msg->sendReliable(gAgent.getRegion()->getHost());
}

// static 
void LLFloaterMap::sendEstateBan(const LLUUID& agent)
{
	LLUUID invoice;
	U32 flags = ESTATE_ACCESS_BANNED_AGENT_ADD;

	invoice.generate();

	LLMessageSystem* msg = gMessageSystem;
	msg->newMessage("EstateOwnerMessage");
	msg->nextBlockFast(_PREHASH_AgentData);
	msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
	msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
	msg->addUUIDFast(_PREHASH_TransactionID, LLUUID::null); //not used

	msg->nextBlock("MethodData");
	msg->addString("Method", "estateaccessdelta");
	msg->addUUID("Invoice", invoice);

	char buf[MAX_STRING];		/* Flawfinder: ignore*/
	gAgent.getID().toString(buf);
	msg->nextBlock("ParamList");
	msg->addString("Parameter", buf);

	snprintf(buf, MAX_STRING, "%u", flags);			/* Flawfinder: ignore */
	msg->nextBlock("ParamList");
	msg->addString("Parameter", buf);

	agent.toString(buf);
	msg->nextBlock("ParamList");
	msg->addString("Parameter", buf);

	gAgent.sendReliableMessage();
}
