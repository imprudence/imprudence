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
#include "llscrolllistctrl.h"


class PanelRadar : public LLPanel
{
public:
	PanelRadar();
	virtual ~PanelRadar();

	BOOL postBuild();

	// returns true if agent_id belongs to an Imprudence developer
	static bool isImpDev(LLUUID agent_id);

	LLUUID getSelected();

	void addToTypingList(LLUUID agent_id);
	void removeFromTypingList(LLUUID agent_id);

	void updateButtonStates();
	void populateRadar();

private:

	// TODO: move all this info into its own object. It's stupid 
	// and bug-prone to keep it all in separate containers, but 
	// I want to get this out for 1.2 -- McCabe
	std::set<LLUUID>    mChatAvatars;
	std::set<LLUUID>    mTypingAvatars;
	std::set<LLUUID>    mSimAvatars;

	LLScrollListCtrl*   mRadarList;
	LLUUID              mSelectedAvatar;

	void updateChatList(std::vector<LLUUID> agent_ids);
	bool isInChatList(LLUUID agent_id);
	void addToChatList(LLUUID agent_id, std::string distance);
	void removeFromChatList(LLUUID agent_id);

	bool isInSimAvList(LLUUID agent_id);
	void addToSimAvList(LLUUID agent_id, std::string distance);
	void updateSimAvList(std::vector<LLUUID> agent_ids);

	bool isTyping(LLUUID agent_id);

	bool visibleItemsSelected() const;
	bool isKickable(const LLUUID &agent_id);

	static std::string getSelectedName(const LLUUID &agent_id);

	static void onUseRadarList(LLUICtrl* ctrl, void* user_data);
	static void onRangeChange(LLFocusableElement* focus, void* user_data);

	static void onClickProfile(void* user_data);
	static void onClickIM(void* user_data);
	static void onClickAddFriend(void* user_data);
	static void onClickOfferTeleport(void* user_data);
	static void onClickTrack(void* user_data);
	static void onClickInvite(void* user_data);
	static void callback_invite_to_group(LLUUID group_id, void *user_data);

	static void onClickFreeze(void *user_data);
	static void onClickEject(void *user_data);
	static void onClickMute(void *user_data);
	static void onClickUnmute(void *user_data);
	static void onClickAR(void *user_data);
	static void onClickEjectFromEstate(void *user_data);

	static void callbackFreeze(S32 option, void *user_data);
	static void callbackEject(S32 option, void *user_data);
	static void callbackAR(void *user_data);
	static void callbackEjectFromEstate(S32 option, void *user_data);

	static void sendFreeze(const LLUUID &avatar, bool);
	static void sendEject(const LLUUID &avatar, bool);
	static void cmdEstateEject(const LLUUID &avatar);
	static void cmdEstateBan(const LLUUID &avatar);
	static void sendEstateBan(const LLUUID& agent);
	static void sendEstateMessage(const char* request, const LLUUID &target);

};


#endif // PANELRADAR_H

