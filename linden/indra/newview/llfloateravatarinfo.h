/** 
 * @file llfloateravatarinfo.h
 * @brief LLFloaterAvatarInfo class definition
 *
 * Copyright (c) 2002-2007, Linden Research, Inc.
 * 
 * Second Life Viewer Source Code
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

/**
 * Avatar information as shown in a floating window from right-click
 * Profile.  Used for editing your own avatar info.  Just a wrapper
 * for LLPanelAvatar, shared with the Find directory.
 */

#ifndef LL_LLFLOATERAVATARINFO_H
#define LL_LLFLOATERAVATARINFO_H

#include "llfloater.h"
#include "llpreview.h"
#include "lluuid.h"
#include "llpanelavatar.h"

class LLButton;
class LLCheckBoxCtrl;
class LLDropTarget;
class LLInventoryItem;
class LLLineEditor;
class LLMessageSystem;
class LLScrollListCtrl;
class LLTabContainer;
class LLTextBox;
class LLTextEditor;
class LLTextureCtrl;
class LLUICtrl;
class LLViewerImage;
class LLViewerObject;

class LLFloaterAvatarInfo
:	public LLPreview
{
public:
	
	static	void*	createPanelAvatar(void*	data);

	virtual	BOOL	postBuild();

	LLFloaterAvatarInfo(const std::string& name, const LLRect &rect, const LLUUID &avatar_id );
	/*virtual*/ ~LLFloaterAvatarInfo();

	/*virtual*/ void draw();

	/*virtual*/ BOOL canClose();

	/*virtual*/ void loadAsset();
	/*virtual*/ EAssetStatus getAssetStatus();

	// Enables rate button, enables IM
	static void showFromObject(const LLUUID &avatar_id, std::string tab_name = "");

	// Disables the rate button, enables IM
	static void showFromDirectory(const LLUUID &avatar_id);

	// Enables all buttons
	static void showFromAvatar(LLViewerObject *object);

	// Enables most buttons
	static void showFromFriend(const LLUUID& agent_id, BOOL online);

	static LLFloaterAvatarInfo* getInstance(const LLUUID &id);
	static void showProfileCallback(S32 option, void *userdata);
	void resetGroupList();

private:

protected:
	LLUUID			mAvatarID;			// for which avatar is this window?
	LLPanelAvatar*	mPanelAvatarp;
	EOnlineStatus	mSuggestedOnlineStatus;
};


#endif
