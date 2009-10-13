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
#include "llchat.h"
#include "llfirstuse.h"
#include "llfloateravatarinfo.h"
#include "llfloaterchat.h"
#include "llfloaterfriends.h"
#include "llfloatergroupinvite.h"
#include "llfloatergroups.h"
#include "llfloaterreporter.h"
#include "llimview.h"
#include "llmutelist.h"
#include "llparcel.h"
#include "llregionposition.h"
#include "roles_constants.h"
#include "llscrolllistctrl.h"
#include "lltracker.h"
#include "llviewerobjectlist.h"
#include "llviewermenu.h"
#include "llviewermessage.h"
#include "llviewerparcelmgr.h"
#include "llviewerregion.h"
#include "llviewerwindow.h"
#include "llvoavatar.h"
#include "llworld.h"

LLFloaterMap::LLFloaterMap(const LLSD& key)
	:
	LLFloater(std::string("minimap")),
	mPanelMap(NULL),
	mUpdate(TRUE),
	mSelectedAvatar(LLUUID::null)

{
	LLCallbackMap::map_t factory_map;
	factory_map["mini_mapview"] = LLCallbackMap(createPanelMiniMap, this);
	LLUICtrlFactory::getInstance()->buildFloater(this, "floater_mini_map.xml", &factory_map, FALSE);

	mChatAvatars.clear();
	mTypingAvatars.clear();
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
	sendChildToFront(getChild<LLButton>("llfloater_minimize_btn"));
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
		LLFirstUse::useMiniMap();
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
bool LLFloaterMap::isImpDev(LLUUID agent_id)
{
	// We use strings here as avatar keys change across grids. 
	// Feel free to add/remove yourself.
	std::string agent_name = getSelectedName(agent_id);
	if (agent_name == "McCabe Maxsted" || 
		agent_name == "Jacek Antonelli" ||
		agent_name == "Armin Weatherwax")
	{
		return true;
	}
	return false;
}

//static
void LLFloaterMap::updateRadar()
{
	LLFloaterMap::getInstance()->populateRadar();
}

void LLFloaterMap::populateRadar()
{
	if (!mUpdate || !LLFloaterMap::getInstance()->getVisible())
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

	// clear count
	std::stringstream avatar_count; 
	avatar_count.str("");

	// find what avatars you can see
	F32 range = gSavedSettings.getF32("NearMeRange");
	LLVector3d current_pos = gAgent.getPositionGlobal();
	std::vector<LLUUID> avatar_ids;
	std::vector<LLVector3d> positions;
	LLWorld::getInstance()->getAvatars(&avatar_ids, &positions);

	LLSD element;

	mRadarList->deleteAllItems();

	if (!avatar_ids.empty())
	{
		for (U32 i=0; i<avatar_ids.size(); i++)
		{
			if (avatar_ids[i] == gAgent.getID() ||
				avatar_ids[i].isNull())
			{
				continue;
			}

			// Add to list only if we get their name
			std::string fullname = getSelectedName(avatar_ids[i]);
			if (!fullname.empty())
			{
				bool notify_chat = gSavedSettings.getBOOL("MiniMapNotifyChatRange");
				bool notify_sim = gSavedSettings.getBOOL("MiniMapNotifySimRange");
	// [RLVa:KB] - Alternate: Imprudence-1.2.0
				if (gRlvHandler.hasBehaviour(RLV_BHVR_SHOWNAMES))
				{
					fullname = gRlvHandler.getAnonym(fullname);
					notify_chat = false;
					notify_sim = false;
				}
	// [/RLVa:KB]

				// check if they're in certain ranges and notify user if we've enabled that
				LLVector3d temp = positions[i] - current_pos;
				F32 distance = llround((F32)temp.magVec(), 0.1f);
				/*char dist[32];
				sprintf(dist, "%.1f", distance);
				std::string dist_string = dist;*/
				std::string dist_string = llformat("%.1f", distance);

				if (notify_chat)
				{
					if (distance < 20.0f)
					{
						if (!isInChatList(avatar_ids[i]))
						{
							addToChatList(avatar_ids[i], dist_string);
						}
					}
					else
					{
						if (isInChatList(avatar_ids[i]))
						{
							removeFromChatList(avatar_ids[i]);
						}
					}
					updateChatList(avatar_ids);
				}
				else if (!mChatAvatars.empty())
				{
					mChatAvatars.clear();
				}

				if (notify_sim)
				{
					if (!isInChatList(avatar_ids[i]) && !getInSimAvList(avatar_ids[i]))
					{
						LLViewerObject *av_obj = gObjectList.findObject(avatar_ids[i]);
						if (av_obj != NULL && av_obj->isAvatar())
						{
							LLVOAvatar* avatarp = (LLVOAvatar*)av_obj;
							if (avatarp != NULL)
							{
								if (avatarp->getRegion() == gAgent.getRegion())
								{
									addToSimAvList(avatar_ids[i], dist_string);
								}
							}
						}
					}
					updateSimAvList(avatar_ids);
				}
				else if (!mSimAvatars.empty())
				{
					mSimAvatars.clear();
				}

				// only display avatars in range
				if (distance <= range)
				{
					// append typing string
					std::string typing = "";
					if (isTyping(avatar_ids[i]))
					{
						typing = getString("is_typing")+ " ";
					}

					std::string mute_text = LLMuteList::getInstance()->isMuted(avatar_ids[i]) ? getString("is_muted") : "";
					element["id"] = avatar_ids[i];
					element["columns"][0]["column"] = "avatar_name";
					element["columns"][0]["type"] = "text";
					element["columns"][0]["value"] = typing + fullname + " " + mute_text;
					element["columns"][1]["column"] = "avatar_distance";
					element["columns"][1]["type"] = "text";
					element["columns"][1]["value"] = dist_string+"m";

					mRadarList->addElement(element, ADD_BOTTOM);
				}
			}
		}

		mRadarList->sortItems();
		mRadarList->setScrollPos(scroll_pos);
		if (mSelectedAvatar.notNull())
		{
			mRadarList->selectByID(mSelectedAvatar);
		}
		avatar_count << (int)avatar_ids.size();
	}
	else
	{
		mTypingAvatars.clear();
		mRadarList->addCommentText(getString("no_one_near"), ADD_TOP);
		avatar_count << "0";
	}

	childSetText("lblAvatarCount", avatar_count.str());

	toggleButtons();

	//llinfos << "mSelectedAvatar: " << mSelectedAvatar.asString() << llendl;
}

void LLFloaterMap::updateChatList(std::vector<LLUUID> agent_ids)
{
	std::set<LLUUID>::iterator it;
	std::vector<LLUUID>::iterator result;
	for (it = mChatAvatars.begin(); it != mChatAvatars.end(); )
	{
		result = find(agent_ids.begin(), agent_ids.end(), *it);
		if (result == agent_ids.end())
		{
			mChatAvatars.erase(it++);
		}
		else
		{
			it++;
		}
	}
}

bool LLFloaterMap::isInChatList(LLUUID agent_id)
{
	if (mChatAvatars.count(agent_id) > 0)
	{
		return true;
	}
	return false;
}

void LLFloaterMap::addToChatList(LLUUID agent_id, std::string distance)
{
	mChatAvatars.insert(agent_id);
	LLChat chat;

	LLUIString notify = getString("entering_chat_range");
	notify.setArg("[NAME]", getSelectedName(agent_id));
	notify.setArg("[DISTANCE]", distance);

	chat.mText = notify;
	chat.mSourceType = CHAT_SOURCE_SYSTEM;
	LLFloaterChat::addChat(chat, FALSE, FALSE);	
}

void LLFloaterMap::removeFromChatList(LLUUID agent_id)
{
	// Do we want to add a notice?
	mChatAvatars.erase(agent_id);
}

bool LLFloaterMap::isTyping(LLUUID agent_id)
{
	if (mTypingAvatars.count(agent_id) > 0)
	{
		return true;
	}
	return false;
}

void LLFloaterMap::updateTypingList(LLUUID agent_id, bool remove)
{
	if (remove)
	{
		if (isTyping(agent_id))
		{
			mTypingAvatars.erase(agent_id);	
		}
	}
	else
	{
		mTypingAvatars.insert(agent_id);
	}
}

void LLFloaterMap::updateSimAvList(std::vector<LLUUID> agent_ids)
{
	std::set<LLUUID>::iterator it;
	std::vector<LLUUID>::iterator result;
	for (it = mSimAvatars.begin(); it != mSimAvatars.end(); )
	{
		result = find(agent_ids.begin(), agent_ids.end(), *it);
		if (result == agent_ids.end())
		{
			mSimAvatars.erase(it++);
		}
		else
		{
			it++;
		}
	}
}

void LLFloaterMap::addToSimAvList(LLUUID agent_id, std::string distance)
{
	mSimAvatars.insert(agent_id);
	LLChat chat;

	LLUIString notify = getString("entering_sim_range");
	notify.setArg("[NAME]", getSelectedName(agent_id));
	notify.setArg("[DISTANCE]", distance);

	chat.mText = notify;
	chat.mSourceType = CHAT_SOURCE_SYSTEM;
	LLFloaterChat::addChat(chat, FALSE, FALSE);	
}

bool LLFloaterMap::getInSimAvList(LLUUID agent_id)
{
	if (mSimAvatars.count(agent_id) > 0)
	{
		return true;
	}
	return false;
}

void LLFloaterMap::toggleButtons()
{
	BOOL enable = FALSE;
	BOOL enable_unmute = FALSE;
	BOOL enable_track = FALSE;
	BOOL enable_estate = FALSE;
	BOOL enable_friend = FALSE;
	if (childHasFocus("RadarPanel"))
	{
		enable = mSelectedAvatar.notNull() ? visibleItemsSelected() : FALSE;
		enable_unmute = mSelectedAvatar.notNull() ? LLMuteList::getInstance()->isMuted(mSelectedAvatar) : FALSE;
		enable_track = gAgent.isGodlike() || is_agent_mappable(mSelectedAvatar);
		enable_estate = isKickable(mSelectedAvatar);
		enable_friend = !is_agent_friend(mSelectedAvatar);
	}
	else
	{
		mRadarList->deselect();
	}

	childSetEnabled("im_btn", enable);
	childSetEnabled("profile_btn", enable);
	childSetEnabled("offer_teleport_btn", enable);
	childSetEnabled("track_btn", enable_track);
	childSetEnabled("invite_btn", enable);
	childSetEnabled("add_btn", enable);
	childSetEnabled("freeze_btn", enable_estate);
	childSetEnabled("eject_btn", enable_estate);
	childSetEnabled("mute_btn", enable);
	childSetEnabled("ar_btn", enable);
	childSetEnabled("estate_eject_btn", enable_estate);

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
	// Bit clumsy, but this way the RLV stuff is in its own separate block and keeps the code above clean - Kitty
	if ( (rlv_handler_t::isEnabled()) && (mSelectedAvatar.notNull()) )
	{
		if (gRlvHandler.hasBehaviour(RLV_BHVR_SHOWNAMES))
		{
			childSetEnabled("im_btn", FALSE);
			childSetEnabled("profile_btn", FALSE);
			childSetEnabled("invite_btn", FALSE);
			childSetEnabled("add_btn", FALSE);
			childSetEnabled("mute_btn", FALSE);
			childSetEnabled("unmute_btn", FALSE);
		}

		// Even though the avie is in the same sim (so they already know where we are) the tp would just get blocked by different code
		// so it's actually less confusing to the user if we just disable the teleport button here so they'll at least have a visual cue
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

BOOL LLFloaterMap::isKickable(const LLUUID &agent_id)
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
				const LLVector3d& pos_global = avatar->getPositionGlobal();
				if (LLWorld::getInstance()->positionRegionValidGlobal(pos_global))
				{
					LLParcel* parcel = LLViewerParcelMgr::getInstance()->selectParcelAt(pos_global)->getParcel();
					LLViewerParcelMgr::getInstance()->deselectLand();
					
					BOOL new_value = (region != NULL);
								
					if (new_value)
					{
						new_value = region->isOwnedSelf(pos);
						if (!new_value || region->isOwnedGroup(pos))
						{
							new_value = LLViewerParcelMgr::getInstance()->isParcelOwnedByAgent(parcel,GP_LAND_ADMIN);
						}
					}
					return new_value;
				}
			}
		}
	}
	return FALSE;	
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
		gIMMgr->setFloaterOpen(TRUE);
		gIMMgr->addSession(getSelectedName(agent_id), IM_NOTHING_SPECIAL, agent_id);
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
			LLTracker::trackAvatar(agent_id, getSelectedName(agent_id));
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
		LLPanelFriends::requestFriendshipDialog(agent_id, getSelectedName(agent_id));
	}
}

//
// Estate tab
//

//static 
std::string LLFloaterMap::getSelectedName(const LLUUID &agent_id)
{
	std::string agent_name;
	if(gCacheName->getFullName(agent_id, agent_name) && agent_name != " ")
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
	args["[AVATAR_NAME]"] = getSelectedName(self->mSelectedAvatar);
	gViewerWindow->alertXml("FreezeAvatarFullname",	args, callbackFreeze, user_data);
}

//static
void LLFloaterMap::onClickEject(void *user_data)
{
	LLFloaterMap *self = (LLFloaterMap*)user_data;
	LLStringUtil::format_map_t args;
	LLSD payload;
	args["AVATAR_NAME"] = getSelectedName(self->mSelectedAvatar);
	gViewerWindow->alertXml("EjectAvatarFullName", args, callbackEject, user_data);
}

//static
void LLFloaterMap::onClickMute(void *user_data)
{
	LLFloaterMap *self = (LLFloaterMap*)user_data;
	LLScrollListItem *item = self->mRadarList->getFirstSelected();
	if (item != NULL)
	{
		LLUUID agent_id = item->getUUID();
		std::string agent_name = getSelectedName(agent_id);
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
	LLScrollListItem *item = self->mRadarList->getFirstSelected();
	if (item != NULL)
	{
		LLUUID agent_id = item->getUUID();
		std::string agent_name = getSelectedName(agent_id);
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
	args["EVIL_USER"] = getSelectedName(self->mSelectedAvatar);
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
