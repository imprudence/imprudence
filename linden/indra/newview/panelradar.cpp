/**
 * @file panelradar.cpp
 * @brief PanelRadar class (list of nearby agents)
 *
 * Copyright (c) 2009, McCabe Maxsted, Jacek Antonelli, Dale Glass
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

#include "panelradar.h"

#include "llagent.h"
#include "llchat.h"
#include "llfloateravatarinfo.h"
#include "llfloaterchat.h"
#include "llfloaterfriends.h"
#include "llfloatergroupinvite.h"
#include "llfloatergroups.h"
#include "llfloatermap.h"
#include "llfloaterregioninfo.h"
#include "llfloaterreporter.h"
#include "llimview.h"
#include "llmutelist.h"
#include "llparcel.h"
#include "llregionposition.h"
#include "roles_constants.h"
#include "llscrolllistctrl.h"
#include "lltracker.h"
#include "lluictrlfactory.h"
#include "llviewercontrol.h"
#include "llviewerobjectlist.h"
#include "llviewermenu.h"
#include "llviewermessage.h"
#include "llviewerparcelmgr.h"
#include "llviewerregion.h"
#include "llviewerwindow.h"
#include "llvoavatar.h"
#include "llworld.h"
#include "panelradarentry.h"
// [RLVa:KB] - Alternate: Imprudence-1.2.0
#include "rlvhandler.h"
// [/RLVa:KB]

PanelRadar::PanelRadar()
	:
	LLPanel(),
	mSelectedAvatar(LLUUID::null),
	mSelectedDistance(-1.0f)
{
	LLUICtrlFactory::getInstance()->buildPanel(this, "panel_radar.xml");
}


BOOL PanelRadar::postBuild()
{
	mRadarTabs = getChild<LLTabContainer>("radar_tab_container");

	mRadarList = getChild<LLScrollListCtrl>("RadarList");
	childSetCommitCallback("RadarList", onUseRadarList, this);
	mRadarList->setDoubleClickCallback(onClickIM);

	childSetAction("im_btn", onClickIM, this);
	childSetAction("profile_btn", onClickProfile, this);
	childSetAction("offer_teleport_btn", onClickOfferTeleport, this);
	childSetAction("teleport_btn", onClickTeleport, this);
	childSetAction("track_btn", onClickTrack, this);
	childSetAction("invite_btn", onClickInvite, this);
	childSetAction("add_btn", onClickAddFriend, this);
	childSetAction("cam_btn", onClickCam, this);
	childSetAction("freeze_btn", onClickFreeze, this);
	childSetAction("eject_btn", onClickEject, this);
	childSetAction("mute_btn", onClickMute, this);
	childSetAction("unmute_btn", onClickUnmute, this);
	childSetAction("ar_btn", onClickAR, this);
	//childSetAction("estate_eject_btn", onClickEjectFromEstate, this);
	//childSetAction("estate_ban_btn", onClickBanFromEstate, this);
	childSetAction("ban_btn", onClickBan, this);

	setDefaultBtn("im_btn");

	updateRadarInfo();

	return TRUE;
}

PanelRadar::~PanelRadar()
{
}

bool PanelRadar::isImpDev(const LLUUID& agent_id)
{
	// We use strings here as avatar keys change across grids. 
	// Feel free to add/remove yourself.
	std::string agent_name = getSelectedName(agent_id);
	return (agent_name == "McCabe Maxsted" || 
	        agent_name == "Jacek Antonelli" ||
	        agent_name == "Armin Weatherwax" ||
			agent_name == "Elektra Hesse" || 
			agent_name == "CodeBastard Redgrave");
}

void PanelRadar::updateRadarInfo()
{
	if (!getVisible())
	{
		return;
	}

	// find what avatars we can know about
	std::vector<LLUUID> avatar_ids;
	std::vector<LLVector3d> positions;
	LLWorld::getInstance()->getAvatars(&avatar_ids, &positions);

	if (!avatar_ids.empty())
	{
		for (U32 i=0; i<avatar_ids.size(); i++)
		{
			// This actually happens sometimes O.o
			if (avatar_ids[i] == gAgent.getID() || avatar_ids[i].isNull())
			{
				continue;
			}

			// Determine if they're in the list already--getEntry checks for null keys
			PanelRadarEntry* entry = getEntry(avatar_ids[i]);

			// If they aren't, create a new entry
			// If they are, see if we need to update any values
			// List them as "(Unknown)" if we can't get their name
			if (!entry)
			{
				mAvatars.insert(std::pair<LLUUID, PanelRadarEntry>(avatar_ids[i], PanelRadarEntry(avatar_ids[i], 
																	getSelectedName(avatar_ids[i]),
																	calculateDistance(avatar_ids[i], positions[i]),
																	positions[i],
																	RADAR_STATUS_NONE,
																	RADAR_NOTIFIED_NONE)
																	));
			}
			else
			{
				if (entry->getName() == getString("unknown_avatar"))
				{
					// Try to find the name again
					entry->setName(getSelectedName(avatar_ids[i]));
				}
				if (entry->getPosition() != positions[i])
				{
					entry->setPosition(positions[i]);
				}
				if (entry->getStatusTimer().hasExpired())
				{
					entry->setStatus(RADAR_STATUS_NONE);
				}
				entry->setDistance(calculateDistance(avatar_ids[i], positions[i]));
			}
		}
		removeDeadEntries(avatar_ids);
	}
	else // avatar_ids empty
	{
		// Just in case
		if (!mAvatars.empty())
		{
			mAvatars.clear();
		}
		mRadarList->deleteAllItems();
		mRadarList->addCommentText(getString("no_one_near"), ADD_TOP);
		LLUIString av_count_string = getString("avatars_in_plural");
		av_count_string.setArg("[COUNT]", "0");
		childSetText("avatar_count", av_count_string.getString());
		return;
	}

	updateRadarDisplay();
}

void PanelRadar::updateRadarDisplay()
{
	if (visibleItemsSelected())
	{
		mSelectedAvatar = mRadarList->getFirstSelected()->getUUID();
		//TODO: as we expand columns, make these numbers enums
		mSelectedDistance = mRadarList->getFirstSelected()->getColumn(1)->getValue().asReal(); 
	}
	else
	{
		mSelectedAvatar.setNull();
		mSelectedDistance = -1.0f;
	}

	S32 scroll_pos = mRadarList->getScrollPos();

	LLSD element;

	// Zap all the avie names. Zap zap zap!
	mRadarList->deleteAllItems();

	U32 chat_distance = gSavedSettings.getU32("ChatDistance");
	F32 range = gSavedSettings.getF32("NearMeRange");
	bool notify_chat = gSavedSettings.getBOOL("MiniMapNotifyChatRange");
	bool notify_sim = gSavedSettings.getBOOL("MiniMapNotifySimRange");
	// We show avatars outside the estate even if you can't manage it in case griefers are lying on the border
	bool is_manager = gAgent.getRegion()->canManageEstate();
// [RLVa:KB] - Alternate: Imprudence-1.2.0
	if (gRlvHandler.hasBehaviour(RLV_BHVR_SHOWNAMES))
	{
		notify_chat = false;
		notify_sim = false;
	}
// [/RLVa:KB]

	std::map<LLUUID, PanelRadarEntry>::iterator mIt;
	for (mIt = mAvatars.begin(); mIt != mAvatars.end(); ++mIt)
	{
		PanelRadarEntry* entry = &mIt->second;

		LL_DEBUGS("Radar") << "Avatar :" << entry->getName()
						   << " Position: " << entry->getPosition()
						   << " Your Position: " << gAgent.getPositionGlobal()
						   << " Distance: " << entry->getDistance()
						   << " Status: " << entry->getStatus()
						   << " Notified: " << entry->getNotified()
						   << " Timer Seconds: " << entry->getStatusTimer().getElapsedTimeF32()
						   << LL_ENDL;

		// Check if they're in certain ranges and notify user if we've enabled that, starting with chat range
		// We round for accuracy when avs tp in
		std::string dist_string = llformat("%.1f", llround(entry->getDistance(), 0.1f));

		// Don't notify if we don't know their name. It takes a few seconds for names to load on OpenSim, anyway
		if (entry->getName() != getString("unknown_avatar"))
		{
			if (notify_sim && 
				(entry->getNotified() < RADAR_NOTIFIED_SIM))
			{
				LLViewerObject *av_obj = gObjectList.findObject(entry->getID());
				if (av_obj != NULL && av_obj->isAvatar())
				{
					LLVOAvatar* avatarp = (LLVOAvatar*)av_obj;
					if (avatarp != NULL)
					{
						if (avatarp->getRegion() == gAgent.getRegion())
						{
							LLChat chat;
							LLUIString notify = getString("entering_sim_range");
							notify.setArg("[NAME]", entry->getName());
							notify.setArg("[DISTANCE]", dist_string);
							chat.mText = notify;
							chat.mSourceType = CHAT_SOURCE_SYSTEM;
							LLFloaterChat::addChat(chat, FALSE, FALSE);
							entry->setNotified(RADAR_NOTIFIED_SIM);
						}
					}
				}
			}
			else if (notify_chat && 
					(entry->getDistance() < chat_distance) && 
					(entry->getNotified() < RADAR_NOTIFIED_CHAT))
			{
				LLChat chat;
				LLUIString notify = getString("entering_chat_range");
				notify.setArg("[NAME]", entry->getName());
				notify.setArg("[DISTANCE]", dist_string);
				chat.mText = notify;
				chat.mSourceType = CHAT_SOURCE_SYSTEM;
				LLFloaterChat::addChat(chat, FALSE, FALSE);
				entry->setNotified(RADAR_NOTIFIED_CHAT);
			}
		}

		// Only display avatars in range
		if (is_manager || entry->getDistance() <= range)
		{
			// Append typing string
			std::string typing = "";
			if (entry->getStatus() == RADAR_STATUS_TYPING)
			{
				typing = getString("is_typing")+ " ";
			}

			std::string mute_text = LLMuteList::getInstance()->isMuted(entry->getID()) ? getString("is_muted") : "";
			element["id"] = entry->getID();
			element["columns"][0]["column"] = "avatar_name";
			element["columns"][0]["type"] = "text";
//			element["columns"][0]["value"] = typing + entry->getName() + " " + mute_text;
// [RLVa:KB] - Alternate: Imprudence-1.2.0
			element["columns"][0]["value"] =
				(gRlvHandler.hasBehaviour(RLV_BHVR_SHOWNAMES))
					? RlvStrings::getAnonym(entry->getName())
					: typing + entry->getName() + " " + mute_text;
// [/RLVa:KB]
			element["columns"][1]["column"] = "avatar_distance";
			element["columns"][1]["type"] = "text";
			element["columns"][1]["value"] = dist_string+"m";

			mRadarList->addElement(element, ADD_BOTTOM);
		}
	}

	mRadarList->sortItems();
	mRadarList->setScrollPos(scroll_pos);
	if (mSelectedAvatar.notNull())
	{
		mRadarList->selectByID(mSelectedAvatar);
	}

	LLUIString av_count_string = (mRadarList->getItemCount() == 1) ? getString("avatars_in_singular") : getString("avatars_in_plural");
	av_count_string.setArg("[COUNT]", llformat("%d", mRadarList->getItemCount()));
	childSetText("avatar_count", av_count_string.getString());

	updateButtonStates();

	LL_DEBUGS("Radar") << "mSelectedAvatar: " << mSelectedAvatar.asString() << LL_ENDL;
}

void PanelRadar::removeDeadEntries(const std::vector<LLUUID>& agent_ids)
{
	if (agent_ids.empty())
	{
		return;
	}

	// TODO: this should really be a part of updateRadarDisplay
	std::vector<LLUUID>::const_iterator vIt;
	std::map<LLUUID, PanelRadarEntry>::iterator mIt;
	for (mIt = mAvatars.begin(); mIt != mAvatars.end();)
	{
		vIt = std::find(agent_ids.begin(), agent_ids.end(), mIt->first);
		if (vIt == agent_ids.end())
		{
			// Remember, removing from the map triggers the entry's default dtor
			mAvatars.erase(mIt++);
		}
		else
		{
			mIt++;
		}
	}
}
			
PanelRadarEntry* PanelRadar::getEntry(const LLUUID& agent_id)
{
	if (agent_id.isNull())
	{
		return NULL;
	}
	else
	{
		std::map<LLUUID, PanelRadarEntry>::iterator mIt;
		mIt = mAvatars.find(agent_id);
		if (mIt != mAvatars.end())
		{
			return &mIt->second;
		}
		return NULL;
	}
}

F32 PanelRadar::calculateDistance(const LLUUID& agent_id, LLVector3d agent_position)
{
	// LL only sends height value up to 1024m, try to work around it if we can by using draw distance
	if (agent_position.mdV[VZ] == 0.0f)
	{
		LLViewerObject *av_obj = gObjectList.findObject(agent_id);
		if (av_obj != NULL && av_obj->isAvatar())
		{
			LLVOAvatar* avatarp = (LLVOAvatar*)av_obj;
			if (avatarp != NULL)
			{
				agent_position = avatarp->getPositionGlobal();
			}
		}
	}
	return F32(dist_vec(agent_position, gAgent.getPositionGlobal())); // don't need F64 here (what dist_vec returns)
}

bool PanelRadar::isKnown(const LLUUID& agent_id)
{
	return (mAvatars.count(agent_id) > 0);
}

void PanelRadar::updateButtonStates()
{
	static bool enable = false;
	static bool enable_unmute = false;
	static bool enable_track = false;
	static bool enable_estate = false;
	static bool enable_friend = false;
	static bool enable_cam = false;

	if (mRadarTabs->getCurrentPanelIndex() == 0) // Avatar tab
	{
		mRadarList->setDoubleClickCallback(onClickIM);
	}
	else // Estate tab
	{
		mRadarList->setDoubleClickCallback(onClickCam);
	}
	
	if (hasFocus() && mSelectedAvatar.notNull())
	{
		enable = visibleItemsSelected();
		enable_estate = isKickable(mSelectedAvatar);
		enable_unmute = LLMuteList::getInstance()->isMuted(mSelectedAvatar);
		enable_track = gAgent.isGodlike() || is_agent_mappable(mSelectedAvatar);
		enable_friend = !is_agent_friend(mSelectedAvatar);
		enable_cam = mSelectedDistance >= 0 && mSelectedDistance <= gSavedSettings.getF32("NearMeRange");
	}
	else
	{
		mRadarList->deselect();
		enable = false;
		enable_estate = false;
		enable_unmute = false;
		enable_track = false;
		enable_friend = false;
		enable_cam = false;
	}

	childSetEnabled("im_btn", enable);
	childSetEnabled("profile_btn", enable);
	childSetEnabled("offer_teleport_btn", enable);
	childSetEnabled("teleport_btn", enable);
	childSetEnabled("track_btn", enable_track);
	childSetEnabled("invite_btn", enable);
	childSetEnabled("add_btn", enable);
	childSetEnabled("cam_btn", enable_cam);
	childSetEnabled("freeze_btn", enable_estate);
	childSetEnabled("eject_btn", enable_estate);
	childSetEnabled("ban_btn", enable_estate);
	childSetEnabled("mute_btn", enable);
	childSetEnabled("ar_btn", enable);
	//childSetEnabled("estate_eject_btn", enable_estate);
	//childSetEnabled("estate_ban_btn", enable_estate);

	if (enable_unmute)
	{
		childSetVisible("mute_btn", false);
		childSetEnabled("unmute_btn", true);
		childSetVisible("unmute_btn", true);
	}
	else
	{
		childSetVisible("mute_btn", true);
		childSetVisible("unmute_btn", false);
	}

// [RLVa:KB] - Imprudence-1.2.0
	// Bit clumsy, but this way the RLV stuff is in its own separate
	// block and keeps the code above clean - Kitty
	if ( (rlv_handler_t::isEnabled()) && (mSelectedAvatar.notNull()) )
	{
		if (gRlvHandler.hasBehaviour(RLV_BHVR_SHOWNAMES))
		{
			childSetEnabled("im_btn", false);
			childSetEnabled("profile_btn", false);
			childSetEnabled("invite_btn", false);
			childSetEnabled("add_btn", false);
			childSetEnabled("mute_btn", false);
			childSetEnabled("unmute_btn", false);
			childSetEnabled("cam_btn", false);
			childSetEnabled("teleport_btn", false);
		}

		// Even though the avie is in the same sim (so they already know
		// where we are) the tp would just get blocked by different code
		// so it's actually less confusing to the user if we just disable
		// the teleport button here so they'll at least have a visual cue
		BOOL rlv_enable_tp = (!gRlvHandler.hasBehaviour(RLV_BHVR_TPLURE)) || (gRlvHandler.isException(RLV_BHVR_TPLURE, mSelectedAvatar));
		if ( (rlv_enable_tp) && (gRlvHandler.hasBehaviour(RLV_BHVR_SHOWLOC)) )
		{
			const LLRelationship* pBuddyInfo = LLAvatarTracker::instance().getBuddyInfo(mSelectedAvatar);
			if ( ((!pBuddyInfo) || (!pBuddyInfo->isOnline()) || (!pBuddyInfo->isRightGrantedTo(LLRelationship::GRANT_MAP_LOCATION))) )
				rlv_enable_tp = FALSE;
		}
		childSetEnabled("offer_teleport_btn", rlv_enable_tp);
	}
// [/RLVa:KB]
}


bool PanelRadar::isKickable(const LLUUID& agent_id)
{
	if (agent_id.notNull())
	{
		LLViewerObject* av_obj = gObjectList.findObject(agent_id);
		if (av_obj != NULL && av_obj->isAvatar())
		{
			LLVOAvatar* avatar = (LLVOAvatar*)av_obj;
			LLViewerRegion* region = avatar->getRegion();
			if (region)
			{
				const LLVector3& pos = avatar->getPositionRegion();

				if (region->isOwnedSelf(pos) || 
					region->canManageEstate())
				{
					return true;
				}

				const LLVector3d& pos_global = avatar->getPositionGlobal();
				if (LLWorld::getInstance()->positionRegionValidGlobal(pos_global))
				{
					LLParcel* parcel = LLViewerParcelMgr::getInstance()->selectParcelAt(pos_global)->getParcel();
					LLViewerParcelMgr::getInstance()->deselectLand();

					if (parcel)
					{
						if (region->isOwnedGroup(pos) &&
								(LLViewerParcelMgr::getInstance()->isParcelOwnedByAgent(parcel,GP_LAND_ADMIN) ||
								 LLViewerParcelMgr::getInstance()->isParcelOwnedByAgent(parcel,GP_LAND_MANAGE_BANNED)))
						{
							return true;
						}
					}
				}
			}
		}
	}
	return false;	
}


// static
void PanelRadar::onUseRadarList(LLUICtrl* ctrl, void* user_data)
{
	PanelRadar* self = (PanelRadar*)user_data;
	if (self)
	{
		self->updateButtonStates();
	}
}


bool PanelRadar::visibleItemsSelected() const
{
	return (mRadarList->getFirstSelectedIndex() >= 0);
}


LLUUID PanelRadar::getSelected()
{
	return mSelectedAvatar;
}


std::string PanelRadar::getSelectedName(const LLUUID& agent_id)
{
	std::string agent_name;
	if(!(gCacheName->getFullName(agent_id, agent_name) && agent_name != " "))
	{
		agent_name = getString("unknown_avatar");
	}
	return agent_name;
}

void PanelRadar::sendAvatarPropertiesRequest(const LLUUID& agent_id)
{
	LL_DEBUGS("Radar") << "PanelRadar::sendAvatarPropertiesRequest()" << LL_ENDL; 
	LLMessageSystem *msg = gMessageSystem;

	msg->newMessageFast(_PREHASH_AvatarPropertiesRequest);
	msg->nextBlockFast( _PREHASH_AgentData);
	msg->addUUIDFast(   _PREHASH_AgentID, gAgent.getID() );
	msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
	msg->addUUIDFast(   _PREHASH_AvatarID, agent_id);
	gAgent.sendReliableMessage();
}


//////////////////////////////////////////////////////////////
//															//
//						* AVATAR TAB*						//
//															//
//////////////////////////////////////////////////////////////

// static
void PanelRadar::onClickIM(void* user_data)
{
	PanelRadar* self = (PanelRadar*) user_data;
	LLScrollListItem *item = self->mRadarList->getFirstSelected();
	if (item != NULL)
	{
		LLUUID agent_id = item->getUUID();
		gIMMgr->setFloaterOpen(TRUE);
		gIMMgr->addSession(self->getSelectedName(agent_id), IM_NOTHING_SPECIAL, agent_id);
	}
}

// static
void PanelRadar::onClickProfile(void* user_data)
{
	PanelRadar* self = (PanelRadar*) user_data;
	LLScrollListItem *item = self->mRadarList->getFirstSelected();
	if (item != NULL)
	{
		LLUUID agent_id = item->getUUID();
		LLFloaterAvatarInfo::show(agent_id);
	}
}

// static
void PanelRadar::onClickOfferTeleport(void* user_data)
{
	PanelRadar* self = (PanelRadar*) user_data;
	LLScrollListItem *item = self->mRadarList->getFirstSelected();
	if (item != NULL)
	{
		LLUUID agent_id = item->getUUID();
		handle_lure(agent_id);
	}
}

//static
void PanelRadar::onClickTeleport(void* user_data)
{
	PanelRadar* self = (PanelRadar*)user_data;
 	LLScrollListItem *item = self->mRadarList->getFirstSelected();

	if (item)
	{
		LLUUID agent_id = item->getUUID();
		std::string agent_name = self->getSelectedName(agent_id);
		if (!agent_name.empty())
		{
			LLViewerObject *av_obj = gObjectList.findObject(agent_id);
			if (av_obj != NULL && av_obj->isAvatar())
			{
				LLVOAvatar* avatarp = (LLVOAvatar*)av_obj;
				if (avatarp != NULL)
				{
					LLVector3d pos = avatarp->getPositionGlobal();
					gAgent.teleportViaLocation(pos);
				}
			}
		}
	}
}

// static
void PanelRadar::onClickTrack(void* user_data)
{
	PanelRadar* self = (PanelRadar*) user_data;
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
			LLTracker::trackAvatar(agent_id, self->getSelectedName(agent_id));
		}
	}
}

// static
void PanelRadar::onClickInvite(void* user_data)
{
	PanelRadar* self = (PanelRadar*) user_data;
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
void PanelRadar::callback_invite_to_group(LLUUID group_id, void* user_data)
{
	std::vector<LLUUID> agent_ids;
	agent_ids.push_back(*(LLUUID *)user_data);
	
	LLFloaterGroupInvite::showForGroup(group_id, &agent_ids);
}

// static
void PanelRadar::onClickAddFriend(void* user_data)
{
	PanelRadar* self = (PanelRadar*) user_data;
	LLScrollListItem *item = self->mRadarList->getFirstSelected();
	if (item != NULL)
	{
		LLUUID agent_id = item->getUUID();
		LLPanelFriends::requestFriendshipDialog(agent_id, self->getSelectedName(agent_id));
	}
}


//////////////////////////////////////////////////////////////
//															//
//						* ESTATE TAB *						//
//															//
//////////////////////////////////////////////////////////////

// static 
void PanelRadar::onClickCam(void* user_data)
{
	PanelRadar* self = (PanelRadar*)user_data;
	self->lookAtAvatar(self->getSelected());
}

void PanelRadar::lookAtAvatar(const LLUUID& agent_id)
{
    LLViewerObject* voavatar = gObjectList.findObject(agent_id);
    if (voavatar && voavatar->isAvatar())
    {
        gAgent.setFocusOnAvatar(FALSE, FALSE);
        gAgent.changeCameraToThirdPerson();
        gAgent.setFocusGlobal(voavatar->getPositionGlobal(), agent_id);
        gAgent.setCameraPosAndFocusGlobal(voavatar->getPositionGlobal() 
                + LLVector3d(3.5,1.35,0.75) * voavatar->getRotation(), 
                                                voavatar->getPositionGlobal(), 
                                                agent_id );
    }
}

//static
bool PanelRadar::callbackFreeze(const LLSD& notification, const LLSD& response)
{
	S32 option = LLNotification::getSelectedOption(notification, response);
	if (option == 0)
	{
		sendFreeze(notification["payload"]["avatar_id"].asUUID(), true);
	}
	else if (option == 1)
	{
		sendFreeze(notification["payload"]["avatar_id"].asUUID(), false);
	}
	return false;
}

//static
bool PanelRadar::callbackEjectBan(const LLSD& notification, const LLSD& response)
{
	S32 option = LLNotification::getSelectedOption(notification, response);
	if (2 == option)
	{
		// Cancel button.
		return false;
	}
	LLUUID avatar_id = notification["payload"]["avatar_id"].asUUID();
	bool ban_enabled = notification["payload"]["ban_enabled"].asBoolean();

	if (0 == option)
	{
		// Eject button
		LLMessageSystem* msg = gMessageSystem;
		LLViewerObject* avatar = gObjectList.findObject(avatar_id);

		if (avatar)
		{
			U32 flags = 0x0;
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
	else if (ban_enabled)
	{
		// This is tricky. It is similar to say if it is not an 'Eject' button,
		// and it is also not an 'Cancle' button, and ban_enabled==ture, 
		// it should be the 'Eject and Ban' button.
		LLMessageSystem* msg = gMessageSystem;
		LLViewerObject* avatar = gObjectList.findObject(avatar_id);

		if (avatar)
		{
			U32 flags = 0x1;
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
	return false;
}

//static
// Don't use until the UI can be worked out
//bool PanelRadar::callbackEjectFromEstate(const LLSD& notification, const LLSD& response)
//{
//	S32 option = LLNotification::getSelectedOption(notification, response);
//	if (option == 0)
//	{
//		strings_t strings;
//		strings.push_back(notification["payload"]["avatar_id"].asString());
//		sendEstateOwnerMessage(gMessageSystem, "kickestate", LLFloaterRegionInfo::getLastInvoice(), strings);
//	} 
//	return false;
//}

// static
bool PanelRadar::callbackBanFromEstate(const LLSD& notification, const LLSD& response)
{
	S32 option = LLNotification::getSelectedOption(notification, response);
	if (option == 0)
	{
		LLPanelEstateInfo::sendEstateAccessDelta(ESTATE_ACCESS_BANNED_AGENT_ADD | ESTATE_ACCESS_ALLOWED_AGENT_REMOVE | ESTATE_ACCESS_NO_REPLY, notification["payload"]["avatar_id"].asUUID());
	}
	else if (option == 1)
	{
		LLViewerRegion* regionp = gAgent.getRegion();
		if (regionp)
		{
			U32 flags = ESTATE_ACCESS_BANNED_AGENT_ADD | ESTATE_ACCESS_ALLOWED_AGENT_REMOVE | ESTATE_ACCESS_NO_REPLY;
			if (regionp->getOwner() == gAgent.getID() || gAgent.isGodlike())
			{
				flags |= ESTATE_ACCESS_APPLY_TO_ALL_ESTATES;
			}
			else if (regionp->isEstateManager())
			{
				flags |= ESTATE_ACCESS_APPLY_TO_MANAGED_ESTATES;
			}
			LLPanelEstateInfo::sendEstateAccessDelta(flags, notification["payload"]["avatar_id"].asUUID());
		}
	}
	return false;
}

// static
void PanelRadar::onClickFreeze(void* user_data)
{
	PanelRadar* self = (PanelRadar*)user_data;
	LLSD payload;
	payload["avatar_id"] = self->getSelected();
	LLSD args;
	args["AVATAR_NAME"] = self->getSelectedName(self->getSelected());
	LLNotifications::instance().add("FreezeAvatarFullname", 
		args, 
		payload, 
		callbackFreeze);
}


// static
void PanelRadar::onClickUnfreeze(void* user_data)
{
	PanelRadar* self = (PanelRadar*)user_data;
	sendFreeze(self->getSelected(), false);
}


//static
// Don't use until we can work out the UI
//void PanelRadar::onClickEjectFromEstate(void* user_data)
//{
//	PanelRadar* self = (PanelRadar*)user_data;
//	LLSD args;
//	args["AVATAR_NAME"] = self->getSelectedName(self->getSelected());
//	LLNotifications::instance().add("EjectAvatarFullnameNoBan", 
//		args, 
//		LLSD(), 
//		boost::bind(&callbackEjectFromEstate, _1, _2, self));
//}


//static
void PanelRadar::onClickMute(void* user_data)
{
	PanelRadar* self = (PanelRadar*)user_data;
	LLScrollListItem *item = self->mRadarList->getFirstSelected();
	if (item != NULL)
	{
		LLUUID agent_id = item->getUUID();
		std::string agent_name = self->getSelectedName(agent_id);
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
void PanelRadar::onClickUnmute(void* user_data)
{
	PanelRadar* self = (PanelRadar*)user_data;
	LLScrollListItem *item = self->mRadarList->getFirstSelected();
	if (item != NULL)
	{
		LLUUID agent_id = item->getUUID();
		std::string agent_name = self->getSelectedName(agent_id);
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
void PanelRadar::onClickEject(void* user_data)
{
	PanelRadar* self = (PanelRadar*)user_data;
	LLSD payload;
	payload["avatar_id"] = self->getSelected();
	payload["ban_enabled"] = false;
	std::string fullname = self->getSelectedName(self->getSelected());

	if (!fullname.empty())
	{
		LLSD args;
		args["AVATAR_NAME"] = fullname;
		LLNotifications::instance().add("EjectAvatarFullnameNoBan",
					args,
					payload,
					callbackEjectBan);
	}
	else
	{
		LLNotifications::instance().add("EjectAvatarNoBan",
					LLSD(),
					payload,
					callbackEjectBan);
	}
}


//static
void PanelRadar::onClickBan(void* user_data)
{
	// Ban for EMs
	PanelRadar* self = (PanelRadar*)user_data;
	LLSD payload;
	payload["avatar_id"] = self->getSelected();

	if (gAgent.canManageEstate())
	{
		LLSD args;
		args["ALL_ESTATES"] = "all estates";
		LLNotifications::instance().add("EstateBannedAgentAdd", args, payload, callbackBanFromEstate);
	}
	else // Ban for parcel owners
	{
		payload["ban_enabled"] = true;
		std::string fullname = self->getSelectedName(self->getSelected());

		if (!fullname.empty())
		{
			LLSD args;
			args["AVATAR_NAME"] = fullname;
			LLNotifications::instance().add("EjectAvatarFullname",
						args,
						payload,
						callbackEjectBan);
		}
		else
		{
			LLNotifications::instance().add("EjectAvatarFullname",
						LLSD(),
						payload,
						callbackEjectBan);
		}
	}
}


//static
void PanelRadar::onClickAR(void* user_data)
{
	PanelRadar* self = (PanelRadar*)user_data;
	LLUUID agent_id = self->getSelected();
		
	if (agent_id.notNull())
	{
			LLFloaterReporter::showFromObject(agent_id);
	}
}


//static
/* Don't use until the UI can be worked out
void PanelRadar::onClickBanFromEstate(void* user_data)
{
	PanelRadar* self = (PanelRadar*)user_data;
	LLSD payload;
	payload["avatar_id"] = self->getSelected();
	LLSD args;
	args["ALL_ESTATES"] = "all estates";
	LLNotifications::instance().add("EstateBannedAgentAdd", args, payload, callbackBanFromEstate);
}
*/


// static 
void PanelRadar::sendFreeze(const LLUUID& avatar_id, bool freeze)
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
