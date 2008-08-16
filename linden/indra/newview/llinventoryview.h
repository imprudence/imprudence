/** 
 * @file llinventoryview.h
 * @brief LLInventoryView, LLInventoryFolder, and LLInventoryItem
 * class definition
 *
 * Copyright (c) 2001-2007, Linden Research, Inc.
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

#ifndef LL_LLINVENTORYVIEW_H
#define LL_LLINVENTORYVIEW_H

#include "llassetstorage.h"
#include "lldarray.h"
#include "llfloater.h"
#include "llinventory.h"
#include "llfolderview.h"
#include "llinventorymodel.h"
#include "llmemberlistener.h"
#include "lluictrlfactory.h"
#include <set>

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Class LLInventoryView
//
// This is the controller class specific for handling agent
// inventory. It deals with the buttons and views used to navigate as
// well as controls the behavior of the overall object.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class LLInventoryModel;
class LLInvFVBridge;
class LLMenuBarGL;
class LLCheckBoxCtrl;
class LLSpinCtrl;
class LLScrollableContainerView;
class LLTextBox;
class LLIconCtrl;
class LLSaveFolderState;
class LLSearchEditor;


class LLInventoryPanel : public LLPanel
{
public:
	static const LLString DEFAULT_SORT_ORDER;
	static const LLString RECENTITEMS_SORT_ORDER;
	static const LLString INHERIT_SORT_ORDER;

	LLInventoryPanel(const LLString& name,
			const LLString& sort_order_setting,
			const LLRect& rect,
			LLInventoryModel* inventory,
			LLFolderSearchFunction search,
			BOOL allow_multi_select,
			LLView *parent_view = NULL);
	~LLInventoryPanel();

	LLInventoryModel* getModel() { return mInventory; }

	BOOL postBuild();

	virtual EWidgetType getWidgetType() const { return WIDGET_TYPE_INVENTORY_PANEL; }
	virtual LLString getWidgetTag() const { return LL_INVENTORY_PANEL_TAG; }
	virtual LLXMLNodePtr getXML(bool save_children = true) const;
	static LLView* fromXML(LLXMLNodePtr node, LLView *parent, LLUICtrlFactory *factory);

	// LLView methods
	BOOL handleHover(S32 x, S32 y, MASK mask);
	BOOL handleDragAndDrop(S32 x, S32 y, MASK mask, BOOL drop,
								   EDragAndDropType cargo_type,
								   void* cargo_data,
								   EAcceptance* accept,
								   LLString& tooltip_msg);

	// Call this method to set the selection.
	void openAllFolders();
	void closeAllFolders();
	void openDefaultFolderForType(LLAssetType::EType);
	void setSelection(const LLUUID& obj_id, BOOL take_keyboard_focus);
	void setSelectCallback(LLFolderView::SelectCallback callback, void* user_data) { if (mFolders) mFolders->setSelectCallback(callback, user_data); }
	void clearSelection();
	LLInventoryFilter* getFilter() { return mFolders->getFilter(); }
	void setFilterTypes(U32 filter);
	U32 getFilterTypes() const { return mFolders->getFilterTypes(); }
	void setFilterPermMask(PermissionMask filter_perm_mask);
	U32 getFilterPermMask() const { return mFolders->getFilterPermissions(); }
	void setFilterSubString(const LLString& string);
	const LLString getFilterSubString() { return mFolders->getFilterSubString(); }
	void setSortOrder(U32 order);
	U32 getSortOrder() { return mFolders->getSortOrder(); }
	void setSinceLogoff(BOOL sl);
	void setHoursAgo(U32 hours);
	BOOL getSinceLogoff() { return mFolders->getFilter()->isSinceLogoff(); }
	
	void setShowFolderState(LLInventoryFilter::EFolderShow show);
	LLInventoryFilter::EFolderShow getShowFolderState();
	void setAllowMultiSelect(BOOL allow) { mFolders->setAllowMultiSelect(allow); }
	// This method is called when something has changed about the inventory.
	void modelChanged(U32 mask);
	LLFolderView* getRootFolder() { return mFolders; }

	// DEBUG ONLY:
	static void dumpSelectionInformation(void* user_data);

	void openSelected();

	void unSelectAll()	{ mFolders->setSelection(NULL, FALSE, FALSE); }

protected:
	// Given the id and the parent, build all of the folder views.
	void rebuildViewsFor(const LLUUID& id, U32 mask);
	void buildNewViews(const LLUUID& id);

public:
	// TomY TODO: Move this elsewhere?
	// helper method which creates an item with a good description,
	// updates the inventory, updates the server, and pushes the
	// inventory update out to other observers.
	void createNewItem(const char* name,
					   const LLUUID& parent_id,
					   LLAssetType::EType asset_type,
					   LLInventoryType::EType inv_type,
					   U32 next_owner_perm = 0);

protected:
	LLInventoryModel*			mInventory;
	LLInventoryObserver*		mInventoryObserver;
	LLFolderView*				mFolders;
	LLScrollableContainerView*	mScroller;
	BOOL 						mAllowMultiSelect;
	const LLString				mSortOrderSetting;
	LLFolderSearchFunction 		mSearchFunction;
};

class LLInventoryView;

class LLInventoryViewFinder : public LLFloater
{
public:
	LLInventoryViewFinder(const LLString& name,
						const LLRect& rect,
						LLInventoryView* inventory_view);
	virtual void draw();
	virtual void onClose(bool app_quitting);
	void changeFilter(LLInventoryFilter* filter);
	void updateElementsFromFilter();
	BOOL getCheckShowEmpty();
	BOOL getCheckSinceLogoff();

	static void onTimeAgo(LLUICtrl*, void *);
	static void onCloseBtn(void* user_data);
	static void selectAllTypes(void* user_data);
	static void selectNoTypes(void* user_data);

protected:
	LLInventoryView*	mInventoryView;
	LLSpinCtrl*			mSpinSinceDays;
	LLSpinCtrl*			mSpinSinceHours;
	LLInventoryFilter*	mFilter;
};

class LLInventoryView : public LLFloater, LLInventoryObserver
{
friend class LLInventoryViewFinder;

public:
	LLInventoryView(const LLString& name, const LLString& rect,
			LLInventoryModel* inventory);
	LLInventoryView(const LLString& name, const LLRect& rect,
					LLInventoryModel* inventory);
	~LLInventoryView();

	/*virtual*/ void changed(U32 mask);

	 BOOL postBuild();

	//
	// Misc functions
	//
	void setFilterTextFromFilter() { mFilterText = mActivePanel->getFilter()->getFilterText(); }
	void startSearch();
	// This method makes sure that an inventory view exists, is
	// visible, and has focus. The view chosen is returned.
	static LLInventoryView* showAgentInventory(BOOL take_keyboard_focus = FALSE);

	// Return the active inventory view if there is one. Active is
	// defined as the inventory that is the closest to the front, and
	// is visible.
	static LLInventoryView* getActiveInventory();

	// This method calls showAgentInventory() if no views are visible,
	// or hides/destroyes them all if any are visible.
	static void toggleVisibility();
	static void toggleVisibility(void*) { toggleVisibility(); }

	// Final cleanup, destroy all open inventory views.
	static void cleanup();

	// LLView & LLFloater functionality
	virtual void onClose(bool app_quitting);
	virtual void setVisible(BOOL visible);
	virtual void draw();
	virtual BOOL handleKeyHere(KEY key, MASK mask, BOOL called_from_parent);

	LLInventoryPanel* getPanel() { return mActivePanel; }
	LLInventoryPanel* getActivePanel() { return mActivePanel; }

	static BOOL filtersVisible(void* user_data);
	static void onClearSearch(void* user_data);
	static void onFoldersByName(void *user_data);
	static BOOL checkFoldersByName(void *user_data);
	static void onSearchEdit(const LLString& search_string, void* user_data );
	//static void onSearchCommit(LLUICtrl* caller, void* user_data );
	static void onFilterSelected(void* userdata, bool from_click);
	static BOOL incrementalFind(LLFolderViewItem* first_item, const char* find_text, BOOL backward);

	const LLString getFilterSubString() { return mActivePanel->getFilterSubString(); }
	void setFilterSubString(const LLString& string) { mActivePanel->setFilterSubString(string); }

	// HACK: Until we can route this info through the instant message hierarchy
	//static BOOL sOpenNextNewItem;
	static BOOL sWearNewClothing;
	static LLUUID sWearNewClothingTransactionID;	// wear all clothing in this transaction

	void toggleFindOptions();

	LLInventoryViewFinder* getFinder() { return (LLInventoryViewFinder*)LLFloater::getFloaterByHandle(mFinderHandle); }

protected:
	// internal initialization code
	void init(LLInventoryModel* inventory);

protected:
	LLSearchEditor*				mSearchEditor;
	LLTabContainer*				mFilterTabs;
	LLViewHandle				mFinderHandle;
	LLInventoryPanel*			mActivePanel;
	LLSaveFolderState*			mSavedFolderState;

	LLString					mFilterText;


	// This container is used to hold all active inventory views. This
	// is here to support the inventory toggle show button.
	static LLDynamicArray<LLInventoryView*> sActiveViews;
};

class LLSelectFirstFilteredItem : public LLFolderViewFunctor
{
public:
	LLSelectFirstFilteredItem() : mItemSelected(FALSE) {}
	virtual ~LLSelectFirstFilteredItem() {}
	virtual void doFolder(LLFolderViewFolder* folder);
	virtual void doItem(LLFolderViewItem* item);
	BOOL wasItemSelected() { return mItemSelected; }
protected:
	BOOL	mItemSelected;
};

class LLOpenFilteredFolders : public LLFolderViewFunctor
{
public:
	LLOpenFilteredFolders()  {}
	virtual ~LLOpenFilteredFolders() {}
	virtual void doFolder(LLFolderViewFolder* folder);
	virtual void doItem(LLFolderViewItem* item);
};

class LLSaveFolderState : public LLFolderViewFunctor
{
public:
	LLSaveFolderState() : mApply(FALSE) {}
	virtual ~LLSaveFolderState() {}
	virtual void doFolder(LLFolderViewFolder* folder);
	virtual void doItem(LLFolderViewItem* item) {}
	void setApply(BOOL apply);
	void clearOpenFolders() { mOpenFolders.clear(); }
protected:
	std::set<LLUUID> mOpenFolders;
	BOOL mApply;
};

class LLOpenFoldersWithSelection : public LLFolderViewFunctor
{
public:
	LLOpenFoldersWithSelection() {}
	virtual ~LLOpenFoldersWithSelection() {}
	virtual void doFolder(LLFolderViewFolder* folder);
	virtual void doItem(LLFolderViewItem* item);
};

///----------------------------------------------------------------------------
/// Function declarations, constants, enums, and typedefs
///----------------------------------------------------------------------------

// useful functions with the inventory view
// *FIX: move these methods.

void init_inventory_actions(LLInventoryView *floater);
void init_inventory_panel_actions(LLInventoryPanel *panel);

class LLInventoryCategory;
class LLInventoryItem;
//void wear_inventory_category_on_avatar(LLInventoryCategory* category);

void wear_inventory_item_on_avatar(LLInventoryItem* item);
void wear_outfit_by_name(const char* name);
void wear_inventory_category(LLInventoryCategory* category, bool copy, bool append);

// These methods can open items without the inventory being visible
void open_notecard(LLViewerInventoryItem* inv_item, const LLString& title, const LLUUID& object_id, BOOL show_keep_discard, const LLUUID& source_id = LLUUID::null, BOOL take_focus = TRUE);
void open_landmark(LLViewerInventoryItem* inv_item, const LLString& title,                          BOOL show_keep_discard, const LLUUID& source_id = LLUUID::null, BOOL take_focus = TRUE);
void open_texture(const LLUUID& item_id, const LLString& title, BOOL show_keep_discard, const LLUUID& source_id = LLUUID::null, BOOL take_focus = TRUE);

LLUUID get_item_icon_uuid(LLAssetType::EType asset_type,
							 LLInventoryType::EType inventory_type,
							 U32 attachment_point, 
							 BOOL item_is_multi );

LLViewerImage* get_item_icon(LLAssetType::EType asset_type,
							 LLInventoryType::EType inventory_type,
							 U32 attachment_point, 
							 BOOL item_is_multi );

// Move items from an in-world object's "Contents" folder to a specified
// folder in agent inventory.
BOOL move_inv_category_world_to_agent(const LLUUID& object_id, 
									  const LLUUID& category_id,
									  BOOL drop,
									  void (*callback)(S32, void*) = NULL,
									  void* user_data = NULL);

const BOOL TAKE_FOCUS_YES = TRUE;
const BOOL TAKE_FOCUS_NO  = FALSE;

void rez_attachment(LLViewerInventoryItem* item, LLViewerJointAttachment* attachment);

extern const char* FIND_HINT;
extern const char* NAME_SEARCH_DESC;
extern const char* NEW_LSL_NAME;
extern const char* NEW_NOTECARD_NAME;
extern const char* NEW_GESTURE_NAME;

#endif // LL_LLINVENTORYVIEW_H

