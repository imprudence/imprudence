/** 
 * @file llgroupnotify.h
 * @brief Non-blocking notification that doesn't take keyboard focus.
 *
 * Copyright (c) 2006-2007, Linden Research, Inc.
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

#ifndef LL_LLGROUPNOTIFY_H
#define LL_LLGROUPNOTIFY_H

#include "llfontgl.h"
#include "llpanel.h"
#include "lldarray.h"
#include "lltimer.h"
#include "llviewermessage.h"

class LLButton;

// NotifyBox - for notifications that require a response from the
// user.  Replaces LLMessageBox.
class LLGroupNotifyBox
:	public LLPanel
{
public:
	static LLGroupNotifyBox* show(const char* subject,
									const char* message,
									const char* from_name,
									const LLUUID& group_id,
									const U32& time_stamp,
									const bool& has_inventory = FALSE,
									const char* inventory_name = NULL,
									LLOfferInfo* inventory_offer = NULL);
	void close();

protected:
	// Non-transient messages.  You can specify non-default button
	// layouts (like one for script dialogs) by passing various
	// numbers in for "layout".
	LLGroupNotifyBox(const char* subject,
						const char* message,
						const char* from_name,
						const LLUUID& group_id,
						const LLUUID& group_insignia,
						const char* group_name,
						const U32& t,
						const bool& has_inventory = FALSE,
						const char* inventory_name = NULL,
						LLOfferInfo* inventory_offer = NULL);

	/*virtual*/ ~LLGroupNotifyBox();

// JC - removed support for clicking in background to dismiss
// the dialogs.
//	/*virtual*/ BOOL handleMouseDown(S32 x, S32 y, MASK mask);
//	/*virtual*/ BOOL handleDoubleClick(S32 x, S32 y, MASK mask);
//	/*virtual*/ BOOL handleMouseUp(S32 x, S32 y, MASK mask);
	/*virtual*/ BOOL handleRightMouseDown(S32 x, S32 y, MASK mask);

	// Animate as sliding onto the screen.
	/*virtual*/ void draw();

	void moveToBack();

	// Returns the rect, relative to gNotifyView, where this
	// notify box should be placed.
	static LLRect getGroupNotifyRect();

	// internal handler for button being clicked
	static void onClickOk(void* data);
	static void onClickGroupInfo(void* data);
	static void onClickSaveInventory(void* data);

	// for "next" button
	static void onClickNext(void* data);

protected:
	// Are we sliding onscreen?
	BOOL mAnimating;

	// Time since this notification was displayed.
	// This is an LLTimer not a frame timer because I am concerned
	// that I could be out-of-sync by one frame in the animation.
	LLTimer mTimer;

	LLButton* mNextBtn;
	LLButton* mSaveInventoryBtn;

	static S32 sGroupNotifyBoxCount;

	LLUUID mGroupID;
	BOOL mHasInventory;
	LLOfferInfo* mInventoryOffer;
};

// This view contains the stack of notification windows.
//extern LLView* gGroupNotifyBoxView;

const S32 GROUP_LAYOUT_DEFAULT = 0;
const S32 GROUP_LAYOUT_SCRIPT_DIALOG = 1;

#endif
