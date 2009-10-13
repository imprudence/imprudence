/** 
 * @file llfloatermap.h
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

#ifndef LL_LLFLOATERMAP_H
#define LL_LLFLOATERMAP_H

#include "llfloater.h"
#include "llscrolllistctrl.h"

class LLNetMap;

class LLFloaterMap :
	public LLFloater,
	public LLFloaterSingleton<LLFloaterMap>
{
	friend class LLUISingleton<LLFloaterMap, VisibilityPolicy<LLFloater> >;
public:
	virtual ~LLFloaterMap();

	static void* createPanelMiniMap(void* data);

	static void updateRadar();
	static LLUUID getSelected();
	// returns true if agent_id belongs to a developer listed in llfloatermap.cpp
	static bool isImpDev(LLUUID agent_id);

	bool isTyping(LLUUID agent_id);
	void updateTypingList(LLUUID agent_id, bool remove);

	BOOL postBuild();

	/*virtual*/ void	draw();
	/*virtual*/ void	onOpen();
	/*virtual*/ void	onClose(bool app_quitting);
	/*virtual*/ BOOL	canClose();
// [RLVa:KB] - Version: 1.22.11 | Checked: 2009-07-05 (RLVa-1.0.0c)
	/*virtual*/ void    open();
// [/RLVa:KB]


private:

	LLFloaterMap(const LLSD& key = LLSD());

	LLNetMap*				mPanelMap;
	LLScrollListCtrl*		mRadarList;
	LLUUID					mSelectedAvatar;

	// TODO: move all this info into its own object. It's stupid 
	// and bug-prone to keep it all in separate containers, but 
	// I want to get this out for 1.2 -- McCabe
	std::set<LLUUID>		mChatAvatars;
	std::set<LLUUID>		mTypingAvatars;
	std::set<LLUUID>		mSimAvatars;
	bool					mUpdate;
	
	static void onList(LLUICtrl* ctrl, void* user_data);
	static void onRangeChange(LLFocusableElement* focus, void* user_data);
	BOOL visibleItemsSelected() const;
	BOOL isKickable(const LLUUID &agent_id);
	void toggleButtons();
	void populateRadar();

	void updateChatList(std::vector<LLUUID> agent_ids);
	bool isInChatList(LLUUID agent_id);
	void addToChatList(LLUUID agent_id, std::string distance);
	void removeFromChatList(LLUUID agent_id);

	bool getInSimAvList(LLUUID agent_id);
	void addToSimAvList(LLUUID agent_id, std::string distance);
	void updateSimAvList(std::vector<LLUUID> agent_ids);

	static void onClickProfile(void* user_data);
	static void onClickIM(void* user_data);
	static void onClickAddFriend(void* user_data);
	static void onClickOfferTeleport(void* user_data);
	static void onClickTrack(void* user_data);
	static void onClickInvite(void* user_data);
	static void callback_invite_to_group(LLUUID group_id, void *user_data);

	static std::string getSelectedName(const LLUUID &agent_id);
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

#endif  // LL_LLFLOATERMAP_H
