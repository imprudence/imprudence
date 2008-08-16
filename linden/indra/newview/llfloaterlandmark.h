/** 
 * @file lltexturectrl.h
 * @author Richard Nelson, James Cook
 * @brief LLTextureCtrl class header file including related functions
 *
 * $LicenseInfo:firstyear=2002&license=internal$
 * 
 * Copyright (c) 2002-2007, Linden Research, Inc.
 * 
 * The following source code is PROPRIETARY AND CONFIDENTIAL. Use of
 * this source code is governed by the Linden Lab Source Code Disclosure
 * Agreement ("Agreement") previously entered between you and Linden
 * Lab. By accessing, using, copying, modifying or distributing this
 * software, you acknowledge that you have been informed of your
 * obligations under the Agreement and agree to abide by those obligations.
 * 
 * ALL LINDEN LAB SOURCE CODE IS PROVIDED "AS IS." LINDEN LAB MAKES NO
 * WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
 * COMPLETENESS OR PERFORMANCE.
 * $/LicenseInfo$
 */

#ifndef LL_LLFLOATERLANDMARK_H
#define LL_LLFLOATERLANDMARK_H

#include "llcoord.h"
#include "llfloater.h"
#include "llstring.h"
#include "lluictrl.h"
#include "llpermissionsflags.h"

class LLButton;
class LLFloaterTexturePicker;
class LLInventoryItem;
class LLTextBox;
class LLViewBorder;
class LLFolderViewItem;
class LLSearchEditor;
class LLInventoryPanel;
class LLSaveFolderState;

// used for setting drag & drop callbacks.
typedef BOOL (*drag_n_drop_callback)(LLUICtrl*, LLInventoryItem*, void*);


//////////////////////////////////////////////////////////////////////////////////////////
// LLFloaterLandmark

class LLFloaterLandmark: public LLFloater, public LLUISingleton<LLFloaterLandmark>
{
public:
	LLFloaterLandmark(const LLSD& data);
	virtual ~LLFloaterLandmark();

	// LLView overrides
	virtual BOOL	handleDragAndDrop(S32 x, S32 y, MASK mask,
		BOOL drop, EDragAndDropType cargo_type, void *cargo_data, 
		EAcceptance *accept,
		LLString& tooltip_msg);
	virtual BOOL	handleKeyHere(KEY key, MASK mask, BOOL called_from_parent);

	// LLFloater overrides
	virtual void	onClose(bool app_quitting);

	const LLUUID& getAssetID() { return mImageAssetID; }
	const LLUUID& findItemID(const LLUUID& asset_id, BOOL copyable_only);

	void			setDirty( BOOL b ) { mIsDirty = b; }
	BOOL			isDirty() { return mIsDirty; }
	void			setActive( BOOL active );

	static void		onBtnClose( void* userdata );
	static void		onBtnNew( void* userdata );
	static void		onBtnEdit( void* userdata );
	static void		onBtnDelete( void* userdata );
	static void		onBtnNewFolder( void* userdata );
	static void		onBtnRename( void* userdata );
	static void		onSelectionChange(const std::deque<LLFolderViewItem*> &items, BOOL user_action, void* data);
	static void		onShowFolders(LLUICtrl* ctrl, void* userdata);
	static void		onSearchEdit(const LLString& search_string, void* user_data );

protected:
	LLPointer<LLViewerImage> mLandmarkp;

	LLUUID				mImageAssetID; // Currently selected texture

	LLUUID				mWhiteImageAssetID;
	LLUUID				mSpecialCurrentImageAssetID;  // Used when the asset id has no corresponding texture in the user's inventory.
	LLUUID				mOriginalImageAssetID;

	LLTextBox*			mTentativeLabel;
	LLTextBox*			mResolutionLabel;

	LLString			mPendingName;
	BOOL				mIsDirty;
	BOOL				mActive;

	LLSearchEditor*		mSearchEdit;
	LLInventoryPanel*	mInventoryPanel;
	PermissionMask		mImmediateFilterPermMask;
	PermissionMask		mNonImmediateFilterPermMask;
	BOOL				mNoCopyLandmarkSelected;
	F32					mContextConeOpacity;
	LLSaveFolderState*	mSavedFolderState;
};

#endif  // LL_FLOATERLANDMARK_H
