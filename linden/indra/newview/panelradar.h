/**
 * @file panelradar.h
 * @brief PanelRadar class header (list of nearby agents)
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


#ifndef PANELRADAR_H
#define PANELRADAR_H


#include "llpanel.h"

class PanelRadarEntry;
class LLScrollListCtrl;
class LLTabContainer;

class PanelRadar : public LLPanel
{
public:
	PanelRadar();
	virtual ~PanelRadar();

	BOOL postBuild();

	// Returns true if agent_id belongs to an Imprudence developer
	bool isImpDev(const LLUUID& agent_id);
	// Change the camera focus to an avatar
	void lookAtAvatar(const LLUUID& agent_id);
	
	// Returns UUID of currently selected avatar
	LLUUID getSelected();

	// Updates the radar button states
	void updateButtonStates();
	// Update the list of known avatars
	void updateRadarInfo();
	// Update the radar UI. Call updateRadarInfo first
	void updateRadarDisplay();

	// Returns PanelRadarEntry* if key is found, NULL if not
	PanelRadarEntry* getEntry(const LLUUID& agent_id);
	// Returns true if avatar is in the radar's list
	bool isKnown(const LLUUID& agent_id);

private:

	std::map<LLUUID, PanelRadarEntry> mAvatars;

	// Returns your distance from an avatar's position
	F32 calculateDistance(const LLUUID& agent_id, LLVector3d agent_position);
	// Removes avatar IDs no longer known to the viewer
	void removeDeadEntries(const std::vector<LLUUID>& agent_ids);

	LLTabContainer*		mRadarTabs;
	LLScrollListCtrl*   mRadarList;
	LLUUID              mSelectedAvatar;
	F32					mSelectedDistance;

	bool visibleItemsSelected() const;
	bool isKickable(const LLUUID& agent_id);

	std::string getSelectedName(const LLUUID& agent_id);
	F32			getSelectedDistance() { return mSelectedDistance; }

	void sendAvatarPropertiesRequest(const LLUUID& agent_id);

	static void onUseRadarList(LLUICtrl* ctrl, void* user_data);
	static void onRangeChange(LLFocusableElement* focus, void* user_data);

	static void onClickProfile(void* user_data);
	static void onClickIM(void* user_data);
	static void onClickAddFriend(void* user_data);
	static void onClickOfferTeleport(void* user_data);
	static void onClickTeleport(void* user_data);
	static void onClickTrack(void* user_data);
	static void onClickInvite(void* user_data);
	static void callback_invite_to_group(LLUUID group_id, void* user_data);

	static void onClickCam(void* user_data);
	static void onClickFreeze(void* user_data);
	static void onClickUnfreeze(void* user_data);
	static void onClickEject(void* user_data);
	static void onClickBan(void* user_data);
	static void onClickMute(void* user_data);
	static void onClickUnmute(void* user_data);
	static void onClickAR(void* user_data);
	//static void onClickEjectFromEstate(void* user_data); Not working yet
	//static void onClickBanFromEstate(void* user_data);
	static void onClickFullRadar(void* user_data);

	static void callbackAR(void* user_data);
	static bool	callbackFreeze(const LLSD& notification, const LLSD& response);
	static bool	callbackEjectBan(const LLSD& notification, const LLSD& response);
	//static bool callbackEjectFromEstate(const LLSD& notification, const LLSD& response);
	static bool callbackBanFromEstate(const LLSD& notification, const LLSD& response);

	static void sendFreeze(const LLUUID& avatar, bool freeze);
};


#endif // PANELRADAR_H

