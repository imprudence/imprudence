/** 
 * @file llinventoryview.cpp
 * @brief Implementation of the inventory view and associated stuff.
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
 * online at
 * http://secondlifegrid.net/programs/open_source/licensing/flossexception
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

#include <utility> // for std::pair<>

#include "llinventoryview.h"
#include "llinventorybridge.h"

#include "message.h"

#include "llagent.h"
#include "llcallingcard.h"
#include "llcheckboxctrl.h"		// for radio buttons
#include "llradiogroup.h"
#include "llspinctrl.h"
#include "lltextbox.h"
#include "llcombobox.h"
#include "llui.h"

#include "llfirstuse.h"
#include "llfloateravatarinfo.h"
#include "llfloaterchat.h"
#include "llfloatercustomize.h"
#include "llfocusmgr.h"
#include "llfolderview.h"
#include "llgesturemgr.h"
#include "lliconctrl.h"
#include "llinventorymodel.h"
#include "llinventoryclipboard.h"
#include "lllineeditor.h"
#include "llmenugl.h"
#include "llpreviewanim.h"
#include "llpreviewgesture.h"
#include "llpreviewlandmark.h"
#include "llpreviewnotecard.h"
#include "llpreviewscript.h"
#include "llpreviewsound.h"
#include "llpreviewtexture.h"
#include "llresmgr.h"
#include "llscrollcontainer.h"
#include "llscrollbar.h"
#include "llimview.h"
#include "lltooldraganddrop.h"
#include "llviewerimagelist.h"
#include "llviewerinventory.h"
#include "llviewerobjectlist.h"
#include "llviewerwindow.h"
#include "llwearablelist.h"
#include "llappviewer.h"
#include "llviewermessage.h"
#include "llviewerregion.h"
#include "lltabcontainer.h"
#include "lluictrlfactory.h"
#include "llselectmgr.h"

#include "llsdserialize.h"

static LLRegisterWidget<LLInventoryPanel> r("inventory_panel");

LLDynamicArray<LLInventoryView*> LLInventoryView::sActiveViews;

//BOOL LLInventoryView::sOpenNextNewItem = FALSE;
BOOL LLInventoryView::sWearNewClothing = FALSE;
LLUUID LLInventoryView::sWearNewClothingTransactionID;

///----------------------------------------------------------------------------
/// Local function declarations, constants, enums, and typedefs
///----------------------------------------------------------------------------

const S32 INV_MIN_WIDTH = 240;
const S32 INV_MIN_HEIGHT = 150;
const S32 INV_FINDER_WIDTH = 160;
const S32 INV_FINDER_HEIGHT = 408;

///----------------------------------------------------------------------------
/// LLInventoryViewFinder
///----------------------------------------------------------------------------

LLInventoryViewFinder::LLInventoryViewFinder(const std::string& name,
						const LLRect& rect,
						LLInventoryView* inventory_view) :
	LLFloater(name, rect, std::string("Filters"), RESIZE_NO,
				INV_FINDER_WIDTH, INV_FINDER_HEIGHT, DRAG_ON_TOP,
				MINIMIZE_NO, CLOSE_YES),
	mInventoryView(inventory_view),
	mFilter(inventory_view->mActivePanel->getFilter())
{

	LLUICtrlFactory::getInstance()->buildFloater(this, "floater_inventory_view_finder.xml");


	childSetCommitCallback("check_animation",    onCheckFilterType, this);
	childSetCommitCallback("check_bodypart",     onCheckFilterType, this);
	childSetCommitCallback("check_calling_card", onCheckFilterType, this);
	childSetCommitCallback("check_clothing",     onCheckFilterType, this);
	childSetCommitCallback("check_gesture",      onCheckFilterType, this);
	childSetCommitCallback("check_landmark",     onCheckFilterType, this);
	childSetCommitCallback("check_notecard",     onCheckFilterType, this);
	childSetCommitCallback("check_object",       onCheckFilterType, this);
	childSetCommitCallback("check_script",       onCheckFilterType, this);
	childSetCommitCallback("check_sound",        onCheckFilterType, this);
	childSetCommitCallback("check_texture",      onCheckFilterType, this);
	childSetCommitCallback("check_snapshot",     onCheckFilterType, this);


	childSetAction("All", selectAllTypes, this);
	childSetAction("None", selectNoTypes, this);

	mSpinSinceHours = getChild<LLSpinCtrl>("spin_hours_ago");
	childSetCommitCallback("spin_hours_ago", onTimeAgo, this);

	mSpinSinceDays = getChild<LLSpinCtrl>("spin_days_ago");
	childSetCommitCallback("spin_days_ago", onTimeAgo, this);

	childSetCommitCallback("check_show_empty", onCheckShowEmptyFolders, this);

//	mCheckSinceLogoff   = getChild<LLSpinCtrl>("check_since_logoff");
	childSetCommitCallback("check_since_logoff", onCheckSinceLogoff, this);

	childSetAction("Close", onCloseBtn, this);

	updateElementsFromFilter();
}


// Callback when an inventory type checkbox is changed.
void LLInventoryViewFinder::onCheckFilterType(LLUICtrl *ctrl, void *user_data)
{
	LLInventoryViewFinder *self = (LLInventoryViewFinder *)user_data;
	if (!self) return;

	self->rebuildFilter();
}


void LLInventoryViewFinder::onCheckSinceLogoff(LLUICtrl *ctrl, void *user_data)
{
	LLInventoryViewFinder *self = (LLInventoryViewFinder *)user_data;
	if (!self) return;

	bool since_logoff= self->childGetValue("check_since_logoff");
	
	if (!since_logoff && 
	    !(  self->mSpinSinceDays->get() ||  self->mSpinSinceHours->get() ) )
	{
		self->mSpinSinceHours->set(1.0f);
	}	

	self->rebuildFilter();
}

void LLInventoryViewFinder::onTimeAgo(LLUICtrl *ctrl, void *user_data)
{
	LLInventoryViewFinder *self = (LLInventoryViewFinder *)user_data;
	if (!self) return;
	
	bool since_logoff=true;
	if ( self->mSpinSinceDays->get() ||  self->mSpinSinceHours->get() )
	{
		since_logoff = false;
	}
	self->childSetValue("check_since_logoff", since_logoff);

	self->rebuildFilter();
}


void LLInventoryViewFinder::onCheckShowEmptyFolders(LLUICtrl *ctrl, void *user_data)
{
	LLInventoryViewFinder *self = (LLInventoryViewFinder *)user_data;
	if (!self) return;

	self->rebuildFilter();
}


void LLInventoryViewFinder::changeFilter(LLInventoryFilter* filter)
{
	mFilter = filter;
	updateElementsFromFilter();
}


void LLInventoryViewFinder::updateElementsFromFilter()
{
	if (!mFilter)
		return;

	// Get data needed for filter display
	U32 filter_types = mFilter->getFilterTypes();
	std::string filter_string = mFilter->getFilterSubString();
	LLInventoryFilter::EFolderShow show_folders = mFilter->getShowFolderState();
	U32 hours = mFilter->getHoursAgo();

	// Update floater title
	LLFloater::setTitle(mFilter->getName());

	// Update type check boxes
	childSetValue("check_animation",
	              (S32)(filter_types & LLInventoryType::NIT_ANIMATION));
	childSetValue("check_bodypart",
	              (S32)(filter_types & LLInventoryType::NIT_BODYPART));
	childSetValue("check_calling_card",
	              (S32)(filter_types & LLInventoryType::NIT_CALLCARD));
	childSetValue("check_clothing",
	              (S32)(filter_types & LLInventoryType::NIT_CLOTHING));
	childSetValue("check_gesture",
	              (S32)(filter_types & LLInventoryType::NIT_GESTURE));
	childSetValue("check_landmark",
	              (S32)(filter_types & LLInventoryType::NIT_LANDMARK));
	childSetValue("check_notecard",
	              (S32)(filter_types & LLInventoryType::NIT_NOTECARD));
	childSetValue("check_object",
	              (S32)(filter_types & LLInventoryType::NIT_OBJECT));
	childSetValue("check_script",
	              (S32)(filter_types & LLInventoryType::NIT_SCRIPT_LSL2));
	childSetValue("check_sound",
	              (S32)(filter_types & LLInventoryType::NIT_SOUND));
	childSetValue("check_texture",
	              (S32)(filter_types & LLInventoryType::NIT_TEXTURE));
	childSetValue("check_snapshot",
	              (S32)(filter_types & LLInventoryType::NIT_SNAPSHOT));

	// Update other check boxes
	childSetValue("check_show_empty",
	              show_folders == LLInventoryFilter::SHOW_ALL_FOLDERS);
	childSetValue("check_since_logoff",
	              mFilter->isSinceLogoff());

	// Update hours and days spinners
	mSpinSinceHours->set( (F32)(hours % 24) );
	mSpinSinceDays->set(  (F32)(hours / 24) );
}


void LLInventoryViewFinder::rebuildFilter()
{
	U32 filter = LLInventoryType::NIT_ALL;

	if (!childGetValue("check_animation"))
	{
		filter &= ~(LLInventoryType::NIT_ANIMATION);
	}

	if (!childGetValue("check_bodypart"))
	{
		filter &= ~(LLInventoryType::NIT_BODYPART);
	}

	if (!childGetValue("check_calling_card"))
	{
		filter &= ~(LLInventoryType::NIT_CALLCARD);
	}

	if (!childGetValue("check_clothing"))
	{
		filter &= ~(LLInventoryType::NIT_CLOTHING);
	}

	if (!childGetValue("check_gesture"))
	{
		filter &= ~(LLInventoryType::NIT_GESTURE);
	}

	if (!childGetValue("check_landmark"))
	{
		filter &= ~(LLInventoryType::NIT_LANDMARK);
	}

	if (!childGetValue("check_notecard"))
	{
		filter &= ~(LLInventoryType::NIT_NOTECARD);
	}

	if (!childGetValue("check_object"))
	{
		filter &= ~(LLInventoryType::NIT_OBJECT);
	}

	if (!childGetValue("check_script"))
	{
		filter &= ~(LLInventoryType::NIT_SCRIPT_LSL2);
	}

	if (!childGetValue("check_sound"))
	{
		filter &= ~(LLInventoryType::NIT_SOUND);
	}

	if (!childGetValue("check_texture"))
	{
		filter &= ~(LLInventoryType::NIT_TEXTURE);
	}

	if (!childGetValue("check_snapshot"))
	{
		filter &= ~(LLInventoryType::NIT_SNAPSHOT);
	}

	if (filter != LLInventoryType::NIT_ALL)
	{
		// don't include folders in filter, unless I've selected everything
		filter &= ~(LLInventoryType::NIT_FOLDER);
	}

	// update the panel, panel will update the filter
	mInventoryView->mActivePanel->setShowFolderState(getCheckShowEmpty() ?
		LLInventoryFilter::SHOW_ALL_FOLDERS : LLInventoryFilter::SHOW_NON_EMPTY_FOLDERS);
	mInventoryView->mActivePanel->setFilterTypes(filter);
	if (getCheckSinceLogoff())
	{
		mSpinSinceDays->set(0);
		mSpinSinceHours->set(0);
	}
	U32 days = (U32)mSpinSinceDays->get();
	U32 hours = (U32)mSpinSinceHours->get();
	if (hours > 24)
	{
		days += hours / 24;
		hours = (U32)hours % 24;
		mSpinSinceDays->set((F32)days);
		mSpinSinceHours->set((F32)hours);
	}
	hours += days * 24;
	mInventoryView->mActivePanel->setHoursAgo(hours);
	mInventoryView->mActivePanel->setSinceLogoff(getCheckSinceLogoff());
	mInventoryView->setFilterTextFromFilter();
}

void  LLInventoryViewFinder::onClose(bool app_quitting)
{
	if (mInventoryView) mInventoryView->getControl("Inventory.ShowFilters")->setValue(FALSE);
	// If you want to reset the filter on close, do it here.  This functionality was
	// hotly debated - Paulm
#if 0
	if (mInventoryView)
	{
		LLInventoryView::onResetFilter((void *)mInventoryView);
	}
#endif
	destroy();
}


BOOL LLInventoryViewFinder::getCheckShowEmpty()
{
	return childGetValue("check_show_empty");
}

BOOL LLInventoryViewFinder::getCheckSinceLogoff()
{
	return childGetValue("check_since_logoff");
}

void LLInventoryViewFinder::onCloseBtn(void* user_data)
{
	LLInventoryViewFinder* finderp = (LLInventoryViewFinder*)user_data;
	finderp->close();
}

// static
void LLInventoryViewFinder::selectAllTypes(void* user_data)
{
	LLInventoryViewFinder* self = (LLInventoryViewFinder*)user_data;
	if(!self) return;

	self->childSetValue("check_animation", TRUE);
	self->childSetValue("check_bodypart", TRUE);
	self->childSetValue("check_calling_card", TRUE);
	self->childSetValue("check_clothing", TRUE);
	self->childSetValue("check_gesture", TRUE);
	self->childSetValue("check_landmark", TRUE);
	self->childSetValue("check_notecard", TRUE);
	self->childSetValue("check_object", TRUE);
	self->childSetValue("check_script", TRUE);
	self->childSetValue("check_sound", TRUE);
	self->childSetValue("check_texture", TRUE);
	self->childSetValue("check_snapshot", TRUE);

	self->rebuildFilter();
}

//static
void LLInventoryViewFinder::selectNoTypes(void* user_data)
{
	LLInventoryViewFinder* self = (LLInventoryViewFinder*)user_data;
	if(!self) return;

	self->childSetValue("check_animation", FALSE);
	self->childSetValue("check_bodypart", FALSE);
	self->childSetValue("check_calling_card", FALSE);
	self->childSetValue("check_clothing", FALSE);
	self->childSetValue("check_gesture", FALSE);
	self->childSetValue("check_landmark", FALSE);
	self->childSetValue("check_notecard", FALSE);
	self->childSetValue("check_object", FALSE);
	self->childSetValue("check_script", FALSE);
	self->childSetValue("check_sound", FALSE);
	self->childSetValue("check_texture", FALSE);
	self->childSetValue("check_snapshot", FALSE);

	self->rebuildFilter();
}


///----------------------------------------------------------------------------
/// LLInventoryView
///----------------------------------------------------------------------------
void LLSaveFolderState::setApply(BOOL apply)
{
	mApply = apply; 
	// before generating new list of open folders, clear the old one
	if(!apply) 
	{
		clearOpenFolders(); 
	}
}

void LLSaveFolderState::doFolder(LLFolderViewFolder* folder)
{
	if(mApply)
	{
		// we're applying the open state
		LLInvFVBridge* bridge = (LLInvFVBridge*)folder->getListener();
		if(!bridge) return;
		LLUUID id(bridge->getUUID());
		if(mOpenFolders.find(id) != mOpenFolders.end())
		{
			folder->setOpen(TRUE);
		}
		else
		{
			// keep selected filter in its current state, this is less jarring to user
			if (!folder->isSelected())
			{
				folder->setOpen(FALSE);
			}
		}
	}
	else
	{
		// we're recording state at this point
		if(folder->isOpen())
		{
			LLInvFVBridge* bridge = (LLInvFVBridge*)folder->getListener();
			if(!bridge) return;
			mOpenFolders.insert(bridge->getUUID());
		}
	}
}

// Default constructor
LLInventoryView::LLInventoryView(const std::string& name,
								 const std::string& rect,
								 LLInventoryModel* inventory) :
	LLFloater(name, rect, std::string("Inventory"), RESIZE_YES,
			  INV_MIN_WIDTH, INV_MIN_HEIGHT, DRAG_ON_TOP,
			  MINIMIZE_NO, CLOSE_YES)
	//LLHandle<LLFloater> mFinderHandle takes care of its own initialization
{
	init(inventory);
}

LLInventoryView::LLInventoryView(const std::string& name,
								 const LLRect& rect,
								 LLInventoryModel* inventory) :
	LLFloater(name, rect, std::string("Inventory"), RESIZE_YES,
			  INV_MIN_WIDTH, INV_MIN_HEIGHT, DRAG_ON_TOP,
			  MINIMIZE_NO, CLOSE_YES)
	//LLHandle<LLFloater> mFinderHandle takes care of its own initialization
{
	init(inventory);
	setRect(rect); // override XML
}


void LLInventoryView::init(LLInventoryModel* inventory)
{
	// Callbacks
	init_inventory_actions(this);

	// Controls
	U32 sort_order = gSavedSettings.getU32("InventorySortOrder");
	BOOL sort_by_name = ! ( sort_order & LLInventoryFilter::SO_DATE );
	BOOL sort_folders_by_name = ( sort_order & LLInventoryFilter::SO_FOLDERS_BY_NAME );
	BOOL sort_system_folders_to_top = ( sort_order & LLInventoryFilter::SO_SYSTEM_FOLDERS_TO_TOP );

	addBoolControl("Inventory.ShowFilters", FALSE);
	addBoolControl("Inventory.SortByName", sort_by_name );
	addBoolControl("Inventory.SortByDate", ! sort_by_name );
	addBoolControl("Inventory.FoldersAlwaysByName", sort_folders_by_name );
	addBoolControl("Inventory.SystemFoldersToTop", sort_system_folders_to_top );

	//Search Controls
	U32 search_type = gSavedSettings.getU32("InventorySearchType");
	BOOL search_by_name = (search_type == 0);

	addBoolControl("Inventory.SearchByName", search_by_name);
	addBoolControl("Inventory.SearchByCreator", !search_by_name);
	addBoolControl("Inventory.SearchByDesc", !search_by_name);

	addBoolControl("Inventory.SearchByAll", !search_by_name);

	mSavedFolderState = new LLSaveFolderState();
	mSavedFolderState->setApply(FALSE);

	LLUICtrlFactory::getInstance()->buildFloater(this, "floater_inventory.xml", NULL);

	mFilterTabs = (LLTabContainer*)getChild<LLTabContainer>("inventory filter tabs");

	// Set up the default inv. panel/filter settings.
	mActivePanel = getChild<LLInventoryPanel>("All Items");
	if (mActivePanel)
	{
		// "All Items" is the previous only view, so it gets the InventorySortOrder
		mActivePanel->setSortOrder(gSavedSettings.getU32("InventorySortOrder"));
		mActivePanel->getFilter()->markDefault();
		mActivePanel->getRootFolder()->applyFunctorRecursively(*mSavedFolderState);
		mActivePanel->setSelectCallback(onSelectionChange, mActivePanel);
	}
	LLInventoryPanel* recent_items_panel = getChild<LLInventoryPanel>("Recent Items");
	if (recent_items_panel)
	{
		recent_items_panel->setSinceLogoff(TRUE);
		recent_items_panel->setSortOrder(LLInventoryFilter::SO_DATE);
		recent_items_panel->setShowFolderState(LLInventoryFilter::SHOW_NON_EMPTY_FOLDERS);
		recent_items_panel->getFilter()->markDefault();
		recent_items_panel->setSelectCallback(onSelectionChange, recent_items_panel);
	}
	LLInventoryPanel* worn_items_panel = getChild<LLInventoryPanel>("Worn Items");
	if (worn_items_panel)
	{
		worn_items_panel->setSortOrder(gSavedSettings.getU32("InventorySortOrder"));
		worn_items_panel->setShowFolderState(LLInventoryFilter::SHOW_NON_EMPTY_FOLDERS);
		worn_items_panel->getFilter()->markDefault();
		worn_items_panel->setFilterWorn(true);
		worn_items_panel->setSelectCallback(onSelectionChange, worn_items_panel);
	}

	// Now load the stored settings from disk, if available.
	std::ostringstream filterSaveName;
	filterSaveName << gDirUtilp->getExpandedFilename(LL_PATH_PER_SL_ACCOUNT, "filters.xml");
	llinfos << "LLInventoryView::init: reading from " << filterSaveName << llendl;
    llifstream file(filterSaveName.str());
	LLSD savedFilterState;
    if (file.is_open())
    {
        LLSDSerialize::fromXML(savedFilterState, file);
		file.close();

		// Load the persistent "Recent Items" settings.
		// Note that the "All Items" settings do not persist.
		if(recent_items_panel)
		{
			if(savedFilterState.has(recent_items_panel->getFilter()->getName()))
			{
				LLSD recent_items = savedFilterState.get(
					recent_items_panel->getFilter()->getName());
				recent_items_panel->getFilter()->fromLLSD(recent_items);
			}
		}

    }

	//Initialize item count - rkeast
	mItemCount = gSavedPerAccountSettings.getS32("InventoryPreviousCount");


	mSearchEditor = getChild<LLSearchEditor>("inventory search editor");
	if (mSearchEditor)
	{
		mSearchEditor->setSearchCallback(onSearchEdit, this);
	}

	mQuickFilterCombo = getChild<LLComboBox>("Quick Filter");
	if (mQuickFilterCombo)
	{
		mQuickFilterCombo->setCommitCallback(onQuickFilterCommit);
	}

	sActiveViews.put(this);

	gInventory.addObserver(this);
}

BOOL LLInventoryView::postBuild()
{
	childSetTabChangeCallback("inventory filter tabs", "All Items", onFilterSelected, this);
	childSetTabChangeCallback("inventory filter tabs", "Recent Items", onFilterSelected, this);
	childSetTabChangeCallback("inventory filter tabs", "Worn Items", onFilterSelected, this);
	//panel->getFilter()->markDefault();
	return TRUE;
}

// Destroys the object
LLInventoryView::~LLInventoryView( void )
{
	// Save the filters state.
	LLSD filterRoot;
	LLInventoryPanel* all_items_panel = getChild<LLInventoryPanel>("All Items");
	if (all_items_panel)
	{
		LLInventoryFilter* filter = all_items_panel->getFilter();
		LLSD filterState;
		filter->toLLSD(filterState);
		filterRoot[filter->getName()] = filterState;
	}

	LLInventoryPanel* recent_items_panel = getChild<LLInventoryPanel>("Recent Items");
	if (recent_items_panel)
	{
		LLInventoryFilter* filter = recent_items_panel->getFilter();
		LLSD filterState;
		filter->toLLSD(filterState);
		filterRoot[filter->getName()] = filterState;
	}
	
	LLInventoryPanel* worn_items_panel = getChild<LLInventoryPanel>("Worn Items");
	if (worn_items_panel)
	{
		LLInventoryFilter* filter = worn_items_panel->getFilter();
		LLSD filterState;
		filter->toLLSD(filterState);
		filterRoot[filter->getName()] = filterState;
	}

	std::ostringstream filterSaveName;
	filterSaveName << gDirUtilp->getExpandedFilename(LL_PATH_PER_SL_ACCOUNT, "filters.xml");
	llofstream filtersFile(filterSaveName.str());
	if(!LLSDSerialize::toPrettyXML(filterRoot, filtersFile))
	{
		llwarns << "Could not write to filters save file " << filterSaveName << llendl;
	}
	else
		filtersFile.close();

	sActiveViews.removeObj(this);
	gInventory.removeObserver(this);
	delete mSavedFolderState;
}

void LLInventoryView::draw()
{
 	if (LLInventoryModel::isEverythingFetched())
	{
		LLLocale locale(LLLocale::USER_LOCALE);
		std::ostringstream title;
		title << "Inventory";
		std::string item_count_string;
		LLResMgr::getInstance()->getIntegerString(item_count_string, gInventory.getItemCount());
		title << " (" << item_count_string << " items)";
		title << mFilterText;
		setTitle(title.str());
	}
	if (mActivePanel && mSearchEditor)
	{
		mSearchEditor->setText(mActivePanel->getFilterSubString());
	}
	if (mActivePanel && mQuickFilterCombo)
	{
		refreshQuickFilter( mQuickFilterCombo );
	}

	LLFloater::draw();
}

void LLOpenFilteredFolders::doItem(LLFolderViewItem *item)
{
	if (item->getFiltered())
	{
		item->getParentFolder()->setOpenArrangeRecursively(TRUE, LLFolderViewFolder::RECURSE_UP);
	}
}

void LLOpenFilteredFolders::doFolder(LLFolderViewFolder* folder)
{
	if (folder->getFiltered() && folder->getParentFolder())
	{
		folder->getParentFolder()->setOpenArrangeRecursively(TRUE, LLFolderViewFolder::RECURSE_UP);
	}
	// if this folder didn't pass the filter, and none of its descendants did
	else if (!folder->getFiltered() && !folder->hasFilteredDescendants())
	{
		folder->setOpenArrangeRecursively(FALSE, LLFolderViewFolder::RECURSE_NO);
	}
}

void LLSelectFirstFilteredItem::doItem(LLFolderViewItem *item)
{
	if (item->getFiltered() && !mItemSelected)
	{
		item->getRoot()->setSelection(item, FALSE, FALSE);
		if (item->getParentFolder())
		{
			item->getParentFolder()->setOpenArrangeRecursively(TRUE, LLFolderViewFolder::RECURSE_UP);
		}
		item->getRoot()->scrollToShowSelection();
		mItemSelected = TRUE;
	}
}

void LLSelectFirstFilteredItem::doFolder(LLFolderViewFolder* folder)
{
	if (folder->getFiltered() && !mItemSelected)
	{
		folder->getRoot()->setSelection(folder, FALSE, FALSE);
		if (folder->getParentFolder())
		{
			folder->getParentFolder()->setOpenArrangeRecursively(TRUE, LLFolderViewFolder::RECURSE_UP);
		}
		folder->getRoot()->scrollToShowSelection();
		mItemSelected = TRUE;
	}
}

void LLOpenFoldersWithSelection::doItem(LLFolderViewItem *item)
{
	if (item->getParentFolder() && item->isSelected())
	{
		item->getParentFolder()->setOpenArrangeRecursively(TRUE, LLFolderViewFolder::RECURSE_UP);
	}
}

void LLOpenFoldersWithSelection::doFolder(LLFolderViewFolder* folder)
{
	if (folder->getParentFolder() && folder->isSelected())
	{
		folder->getParentFolder()->setOpenArrangeRecursively(TRUE, LLFolderViewFolder::RECURSE_UP);
	}
}

void LLInventoryView::startSearch()
{
	// this forces focus to line editor portion of search editor
	if (mSearchEditor)
	{
		mSearchEditor->focusFirstItem(TRUE);
	}
}

// virtual, from LLView
void LLInventoryView::setVisible( BOOL visible )
{
	gSavedSettings.setBOOL("ShowInventory", visible);
	LLFloater::setVisible(visible);
}

// Destroy all but the last floater, which is made invisible.
void LLInventoryView::onClose(bool app_quitting)
{
//	S32 count = sActiveViews.count();
// [RLVa:KB] - Checked: 2009-07-10 (RLVa-1.0.0g)
	// See LLInventoryView::closeAll() on why we're doing it this way
	S32 count = 0;
	for (S32 idx = 0, cnt = sActiveViews.count(); idx < cnt; idx++)
	{
		if (!sActiveViews.get(idx)->isDead())
			count++;
	}
// [/RLVa:KB]

	if (count > 1)
	{
		destroy();
	}
	else
	{
		if (!app_quitting)
		{
			gSavedSettings.setBOOL("ShowInventory", FALSE);
		}
		// clear filters, but save user's folder state first
		if (!mActivePanel->getRootFolder()->isFilterModified())
		{
			mSavedFolderState->setApply(FALSE);
			mActivePanel->getRootFolder()->applyFunctorRecursively(*mSavedFolderState);
		}
		
		// onClearSearch(this);

		// pass up
		LLFloater::setVisible(FALSE);
	}
}

BOOL LLInventoryView::handleKeyHere(KEY key, MASK mask)
{
	LLFolderView* root_folder = mActivePanel ? mActivePanel->getRootFolder() : NULL;
	if (root_folder)
	{
		// first check for user accepting current search results
		if (mSearchEditor 
			&& mSearchEditor->hasFocus()
		    && (key == KEY_RETURN 
		    	|| key == KEY_DOWN)
		    && mask == MASK_NONE)
		{
			// move focus to inventory proper
			root_folder->setFocus(TRUE);
			root_folder->scrollToShowSelection();
			return TRUE;
		}

		if (root_folder->hasFocus() && key == KEY_UP)
		{
			startSearch();
		}
	}

	return LLFloater::handleKeyHere(key, mask);

}

void LLInventoryView::changed(U32 mask)
{
	std::ostringstream title;
	title << "Inventory";
 	if (LLInventoryModel::backgroundFetchActive())
	{
		LLLocale locale(LLLocale::USER_LOCALE);
		std::string item_count_string;
		LLResMgr::getInstance()->getIntegerString(item_count_string, gInventory.getItemCount());

		//Displays a progress indication for loading the inventory, but not if it hasn't been loaded before on this PC, or we load more than expected - rkeast
		if(mItemCount == -1)
		{
			title << " (Fetched " << item_count_string << " items...)";
		}
		else
		{
			S32 remaining = mItemCount - gInventory.getItemCount();
			std::string total_items;
			std::string items_remaining;
			LLResMgr::getInstance()->getIntegerString(total_items, mItemCount);
			LLResMgr::getInstance()->getIntegerString(items_remaining, remaining);
			if(remaining < 0) title << " (Fetched " << item_count_string << " items...)";
			else title << " (Fetched " << item_count_string << " items of ~" << total_items << " - ~" << items_remaining << " remaining...)";
		}
	}
	else
	{
		gSavedPerAccountSettings.setS32("InventoryPreviousCount", gInventory.getItemCount());
	}
	title << mFilterText;
	setTitle(title.str());

}

// static
// *TODO: remove take_keyboard_focus param
LLInventoryView* LLInventoryView::showAgentInventory(BOOL take_keyboard_focus)
{
	if (gDisconnected || gNoRender)
	{
		return NULL;
	}

// [RLVa:KB] - Checked: 2009-07-10 (RLVa-1.0.0g)
	if (gRlvHandler.hasBehaviour(RLV_BHVR_SHOWINV))
	{
		return NULL;
	}
// [/RLVa:KB]

	LLInventoryView* iv = LLInventoryView::getActiveInventory();
#if 0 && !LL_RELEASE_FOR_DOWNLOAD
	if (sActiveViews.count() == 1)
	{
		delete iv;
		iv = NULL;
	}
#endif
	if(!iv && !gAgent.cameraMouselook())
	{
		// create one.
		iv = new LLInventoryView(std::string("Inventory"),
								 std::string("FloaterInventoryRect"),
								 &gInventory);
		iv->open();
		// keep onscreen
		gFloaterView->adjustToFitScreen(iv, FALSE);

		gSavedSettings.setBOOL("ShowInventory", TRUE);
	}
	if(iv)
	{
		// Make sure it's in front and it makes a noise
		iv->setTitle(std::string("Inventory"));
		iv->open();		/*Flawfinder: ignore*/
	}
	//if (take_keyboard_focus)
	//{
	//	iv->startSearch();
	//	gFocusMgr.triggerFocusFlash();
	//}
	return iv;
}

// static
LLInventoryView* LLInventoryView::getActiveInventory()
{
	LLInventoryView* iv = NULL;
	S32 count = sActiveViews.count();
	if(count > 0)
	{
		iv = sActiveViews.get(0);
		S32 z_order = gFloaterView->getZOrder(iv);
		S32 z_next = 0;
		LLInventoryView* next_iv = NULL;
		for(S32 i = 1; i < count; ++i)
		{
			next_iv = sActiveViews.get(i);
			z_next = gFloaterView->getZOrder(next_iv);
			if(z_next < z_order)
			{
				iv = next_iv;
				z_order = z_next;
			}
		}
	}
	return iv;
}

// static
void LLInventoryView::toggleVisibility()
{
	S32 count = sActiveViews.count();
	if (0 == count)
	{
		showAgentInventory(TRUE);
	}
	else if (1 == count)
	{
		if (sActiveViews.get(0)->getVisible())
		{
			sActiveViews.get(0)->close();
			gSavedSettings.setBOOL("ShowInventory", FALSE);
		}
		else
		{
			showAgentInventory(TRUE);
		}
	}
	else
	{
		// With more than one open, we know at least one
		// is visible.

		// Close all the last one spawned.
		S32 last_index = sActiveViews.count() - 1;
		sActiveViews.get(last_index)->close();
	}
}

// static
void LLInventoryView::cleanup()
{
	S32 count = sActiveViews.count();
	for (S32 i = 0; i < count; i++)
	{
		sActiveViews.get(i)->destroy();
	}
	gInventory.empty();
}

void LLInventoryView::toggleFindOptions()
{
	LLFloater *floater = getFinder();
	if (!floater)
	{
		LLInventoryViewFinder * finder = new LLInventoryViewFinder(std::string("Inventory Finder"),
										LLRect(getRect().mLeft - INV_FINDER_WIDTH, getRect().mTop, getRect().mLeft, getRect().mTop - INV_FINDER_HEIGHT),
										this);
		mFinderHandle = finder->getHandle();
		finder->open();		/*Flawfinder: ignore*/
		addDependentFloater(mFinderHandle);

		// start background fetch of folders
		gInventory.startBackgroundFetch();

		mFloaterControls[std::string("Inventory.ShowFilters")]->setValue(TRUE);
	}
	else
	{
		floater->close();

		mFloaterControls[std::string("Inventory.ShowFilters")]->setValue(FALSE);
	}
}

// static
BOOL LLInventoryView::filtersVisible(void* user_data)
{
	LLInventoryView* self = (LLInventoryView*)user_data;
	if(!self) return FALSE;

	return self->getFinder() != NULL;
}

// static
void LLInventoryView::onClearSearch(void* user_data)
{
	LLInventoryView* self = (LLInventoryView*)user_data;
	if(!self) return;

	if (self->mActivePanel)
	{
		self->mActivePanel->setFilterSubString(LLStringUtil::null);
	}

	// re-open folders that were initially open
	if (self->mActivePanel)
	{
		self->mSavedFolderState->setApply(TRUE);
		self->mActivePanel->getRootFolder()->applyFunctorRecursively(*self->mSavedFolderState);
		LLOpenFoldersWithSelection opener;
		self->mActivePanel->getRootFolder()->applyFunctorRecursively(opener);
		self->mActivePanel->getRootFolder()->scrollToShowSelection();
	}
}

//static
void LLInventoryView::onSearchEdit(const std::string& search_string, void* user_data )
{
	if (search_string == "")
	{
		onClearSearch(user_data);
	}
	LLInventoryView* self = (LLInventoryView*)user_data;
	if (!self->mActivePanel)
	{
		return;
	}

	gInventory.startBackgroundFetch();

	std::string filter_text = search_string;
	std::string uppercase_search_string = filter_text;
	LLStringUtil::toUpper(uppercase_search_string);
	if (self->mActivePanel->getFilterSubString().empty() && uppercase_search_string.empty())
	{
			// current filter and new filter empty, do nothing
			return;
	}

	// save current folder open state if no filter currently applied
	if (!self->mActivePanel->getRootFolder()->isFilterModified())
	{
		self->mSavedFolderState->setApply(FALSE);
		self->mActivePanel->getRootFolder()->applyFunctorRecursively(*self->mSavedFolderState);
	}

	// set new filter string
	self->mActivePanel->setFilterSubString(uppercase_search_string);
}


//static
void LLInventoryView::onQuickFilterCommit(LLUICtrl* ctrl, void* user_data)
{

	LLComboBox* quickfilter = (LLComboBox*)ctrl;


	LLInventoryView* view = (LLInventoryView*)(quickfilter->getParent());
	if (!view->mActivePanel)
	{
		return;
	}


	std::string item_type = quickfilter->getSimple();
	U32 filter_type;

	if (view->getString("filter_type_animation") == item_type)
	{
		filter_type = LLInventoryType::NIT_ANIMATION;
	}

	else if (view->getString("filter_type_bodypart") == item_type)
	{
		filter_type = LLInventoryType::NIT_BODYPART;
	}

	else if (view->getString("filter_type_callingcard") == item_type)
	{
		filter_type = LLInventoryType::NIT_CALLCARD;
	}

	else if (view->getString("filter_type_clothing") == item_type)
	{
		filter_type = LLInventoryType::NIT_CLOTHING;
	}

	else if (view->getString("filter_type_gesture") == item_type)
	{
		filter_type = LLInventoryType::NIT_GESTURE;
	}

	else if (view->getString("filter_type_landmark") == item_type)
	{
		filter_type = LLInventoryType::NIT_LANDMARK;
	}

	else if (view->getString("filter_type_notecard") == item_type)
	{
		filter_type = LLInventoryType::NIT_NOTECARD;
	}

	else if (view->getString("filter_type_object") == item_type)
	{
		filter_type = LLInventoryType::NIT_OBJECT;
	}

	else if (view->getString("filter_type_script") == item_type)
	{
		filter_type = LLInventoryType::NIT_SCRIPT_LSL2;
	}

	else if (view->getString("filter_type_sound") == item_type)
	{
		filter_type = LLInventoryType::NIT_SOUND;
	}

	else if (view->getString("filter_type_texture") == item_type)
	{
		filter_type = LLInventoryType::NIT_TEXTURE;
	}

	else if (view->getString("filter_type_snapshot") == item_type)
	{
		filter_type = LLInventoryType::NIT_SNAPSHOT;
	}

	else if (view->getString("filter_type_custom") == item_type)
	{
		// When they select custom, show the floater then return
		if( !(view->filtersVisible(view)) )
		{
			view->toggleFindOptions();
		}
		return;
	}

	else if (view->getString("filter_type_all") == item_type)
	{
		// Show all types
		filter_type = LLInventoryType::NIT_ALL;
	}

	else
	{
		llwarns << "Ignoring unknown filter: " << item_type << llendl;
		return;
	}

	view->mActivePanel->setFilterTypes( filter_type );

	// Start fetching inventory in the background, so we have
	// some items to show the user.
	gInventory.startBackgroundFetch();

	// Update the Inventory window text
	view->setFilterTextFromFilter();

	// Force the filters window to update itself, if it's open.
	LLInventoryViewFinder* finder = view->getFinder();
	if( finder )
	{
		finder->updateElementsFromFilter();
	}

	// llinfos << "Quick Filter: " << item_type << llendl;

}



//static
void LLInventoryView::refreshQuickFilter(LLUICtrl* ctrl)
{

	LLInventoryView* view = (LLInventoryView*)(ctrl->getParent());
	if (!view->mActivePanel)
	{
		return;
	}

	LLComboBox* quickfilter = view->getChild<LLComboBox>("Quick Filter");
	if (!quickfilter)
	{
		return;
	}


	U32 filter_type = view->mActivePanel->getFilterTypes();
  filter_type &= LLInventoryType::NIT_ALL;


  //llinfos << "filter_type: " << filter_type << llendl;

	std::string selection;


	if (filter_type == LLInventoryType::NIT_ALL)
	{
		selection = view->getString("filter_type_all");
	}

  else if (filter_type == LLInventoryType::NIT_NONE)
  {
		selection = view->getString("filter_type_custom");
  }

	else if (filter_type == (filter_type & LLInventoryType::NIT_ANIMATION))
	{
		selection = view->getString("filter_type_animation");
	}

	else if (filter_type == (filter_type & LLInventoryType::NIT_BODYPART))
	{
		selection = view->getString("filter_type_bodypart");
	}

	else if (filter_type == (filter_type & LLInventoryType::NIT_CALLCARD))
	{
		selection = view->getString("filter_type_callingcard");
	}

	else if (filter_type == (filter_type & LLInventoryType::NIT_CLOTHING))
	{
		selection = view->getString("filter_type_clothing");
	}

	else if (filter_type == (filter_type & LLInventoryType::NIT_GESTURE))
	{
		selection = view->getString("filter_type_gesture");
	}

	else if (filter_type == (filter_type & LLInventoryType::NIT_LANDMARK))
	{
		selection = view->getString("filter_type_landmark");
	}

	else if (filter_type == (filter_type & LLInventoryType::NIT_NOTECARD))
	{
		selection = view->getString("filter_type_notecard");
	}

	else if (filter_type == (filter_type & LLInventoryType::NIT_OBJECT))
	{
		selection = view->getString("filter_type_object");
	}

	else if (filter_type == (filter_type & LLInventoryType::NIT_SCRIPT_LSL2))
	{
		selection = view->getString("filter_type_script");
	}

	else if (filter_type == (filter_type & LLInventoryType::NIT_SOUND))
	{
		selection = view->getString("filter_type_sound");
	}

	else if (filter_type == (filter_type & LLInventoryType::NIT_TEXTURE))
	{
		selection = view->getString("filter_type_texture");
	}

	else if (filter_type == (filter_type & LLInventoryType::NIT_SNAPSHOT))
	{
		selection = view->getString("filter_type_snapshot");
	}

	else
	{
		selection = view->getString("filter_type_custom");
	}


	// Select the chosen item by label text
	BOOL result = quickfilter->setSimple( (selection) );

  if( !result )
  {
    llinfos << "The item didn't exist: " << selection << llendl;
  }

}



// static
// BOOL LLInventoryView::incrementalFind(LLFolderViewItem* first_item, const char *find_text, BOOL backward)
// {
// 	LLInventoryView* active_view = NULL;

// 	for (S32 i = 0; i < sActiveViews.count(); i++)
// 	{
// 		if (gFocusMgr.childHasKeyboardFocus(sActiveViews[i]))
// 		{
// 			active_view = sActiveViews[i];
// 			break;
// 		}
// 	}

// 	if (!active_view)
// 	{
// 		return FALSE;
// 	}

// 	std::string search_string(find_text);

// 	if (search_string.empty())
// 	{
// 		return FALSE;
// 	}

// 	if (active_view->mActivePanel &&
// 		active_view->mActivePanel->getRootFolder()->search(first_item, search_string, backward))
// 	{
// 		return TRUE;
// 	}

// 	return FALSE;
// }

//static
void LLInventoryView::onFilterSelected(void* userdata, bool from_click)
{
	LLInventoryView* self = (LLInventoryView*) userdata;
	LLInventoryFilter* filter;

	LLInventoryViewFinder *finder = self->getFinder();
	// Find my index
	self->mActivePanel = (LLInventoryPanel*)self->childGetVisibleTab("inventory filter tabs");

	if (!self->mActivePanel)
	{
		return;
	}
	filter = self->mActivePanel->getFilter();
	if (finder)
	{
		finder->changeFilter(filter);
	}
	if (filter->isActive())
	{
		// If our filter is active we may be the first thing requiring a fetch so we better start it here.
		gInventory.startBackgroundFetch();
	}
	self->setFilterTextFromFilter();
}

// static
void LLInventoryView::onSelectionChange(const std::deque<LLFolderViewItem*> &items, BOOL user_action, void* data)
{
	LLInventoryPanel* panel = (LLInventoryPanel*)data;
	LLFolderView* fv = panel->getRootFolder();
	if (fv->needsAutoRename()) // auto-selecting a new user-created asset and preparing to rename
	{
		fv->setNeedsAutoRename(FALSE);
		if (items.size()) // new asset is visible and selected
		{
			fv->startRenamingSelectedItem();
		}
	}
}

BOOL LLInventoryView::handleDragAndDrop(S32 x, S32 y, MASK mask, BOOL drop,
										 EDragAndDropType cargo_type,
										 void* cargo_data,
										 EAcceptance* accept,
										 std::string& tooltip_msg)
{
	// Check to see if we are auto scrolling from the last frame
	LLInventoryPanel* panel = (LLInventoryPanel*)this->getActivePanel();
	BOOL needsToScroll = panel->getScrollableContainer()->needsToScroll(x, y, LLScrollableContainerView::VERTICAL);
	if(mFilterTabs)
	{
		if(needsToScroll)
		{
			mFilterTabs->startDragAndDropDelayTimer();
		}
	}
	
	BOOL handled = LLFloater::handleDragAndDrop(x, y, mask, drop, cargo_type, cargo_data, accept, tooltip_msg);

	return handled;
}

std::string get_item_icon_name(LLAssetType::EType asset_type,
                               LLInventoryType::EType inv_type,
                               U32 flags,
                               BOOL item_is_multi )
{
	return get_item_icon_name( calc_ntype(inv_type, asset_type, flags),
	                           item_is_multi );
}

std::string get_item_icon_name(LLInventoryType::NType inv_ntype,
                               BOOL item_is_multi )
{
	EInventoryIcon idx = OBJECT_ICON_NAME;

	switch( inv_ntype )
	{
		// BODY PARTS

		case LLInventoryType::NIT_SHAPE:
			idx = BODYPART_SHAPE_ICON_NAME;
			break;
		case LLInventoryType::NIT_SKIN:
			idx = BODYPART_SKIN_ICON_NAME;
			break;
		case LLInventoryType::NIT_HAIR:
			idx = BODYPART_HAIR_ICON_NAME;
			break;
		case LLInventoryType::NIT_EYES:
			idx = BODYPART_EYES_ICON_NAME;
			break;

		case LLInventoryType::NIT_BODYPART:
			idx = BODYPART_ICON_NAME;
			break;


		// CLOTHING

		case LLInventoryType::NIT_SHIRT:
			idx = CLOTHING_SHIRT_ICON_NAME;
			break;
		case LLInventoryType::NIT_PANTS:
			idx = CLOTHING_PANTS_ICON_NAME;
			break;
		case LLInventoryType::NIT_SHOES:
			idx = CLOTHING_SHOES_ICON_NAME;
			break;
		case LLInventoryType::NIT_SOCKS:
			idx = CLOTHING_SOCKS_ICON_NAME;
			break;
		case LLInventoryType::NIT_JACKET:
			idx = CLOTHING_JACKET_ICON_NAME;
			break;
		case LLInventoryType::NIT_GLOVES:
			idx = CLOTHING_GLOVES_ICON_NAME;
			break;
		case LLInventoryType::NIT_UNDERSHIRT:
			idx = CLOTHING_UNDERSHIRT_ICON_NAME;
			break;
		case LLInventoryType::NIT_UNDERPANTS:
			idx = CLOTHING_UNDERPANTS_ICON_NAME;
			break;
		case LLInventoryType::NIT_SKIRT:
			idx = CLOTHING_SKIRT_ICON_NAME;
			break;

		case LLInventoryType::NIT_CLOTHING:
			idx = CLOTHING_ICON_NAME;
			break;


		// TEXTURES / SNAPSHOTS

		case LLInventoryType::NIT_SNAPSHOT:
			idx = SNAPSHOT_ICON_NAME;
			break;
		case LLInventoryType::NIT_TEXTURE:
		case LLInventoryType::NIT_IMAGE:
			idx = TEXTURE_ICON_NAME;
			break;


		// CALLING CARDS

		case LLInventoryType::NIT_CALLCARD_ON:
			idx = CALLINGCARD_ONLINE_ICON_NAME;
			break;
		case LLInventoryType::NIT_CALLCARD_OFF:
		case LLInventoryType::NIT_CALLCARD:
			idx = CALLINGCARD_OFFLINE_ICON_NAME;
			break;


		// LANDMARKS

		case LLInventoryType::NIT_LANDMARK_USED:
			idx = LANDMARK_VISITED_ICON_NAME;
			break;
		case LLInventoryType::NIT_LANDMARK_UNUSED:
		case LLInventoryType::NIT_LANDMARK:
			idx = LANDMARK_ICON_NAME;
			break;


		// SOUNDS

		case LLInventoryType::NIT_SOUND:
			idx = SOUND_ICON_NAME;
			break;


		// ANIMATIONS

		case LLInventoryType::NIT_ANIMATION:
			idx = ANIMATION_ICON_NAME;
			break;


		// GESTURES

		case LLInventoryType::NIT_GESTURE:
			idx = GESTURE_ICON_NAME;
			break;


		// NOTECARD

		case LLInventoryType::NIT_NOTECARD:
			idx = NOTECARD_ICON_NAME;
			break;


		// SCRIPTS

		case LLInventoryType::NIT_SCRIPT_LSL2:
			idx = SCRIPT_ICON_NAME;
			break;


		// OBJECTS

		case LLInventoryType::NIT_OBJECT:
			if( item_is_multi )
			{
				idx = OBJECT_MULTI_ICON_NAME;
			}
			else
			{
				idx = OBJECT_ICON_NAME;
			}
			break;

		default:
			break;
	}

	return ICON_NAME[idx];
}

LLUIImagePtr get_item_icon(LLAssetType::EType asset_type,
							 LLInventoryType::EType inventory_type,
							 U32 attachment_point,
							 BOOL item_is_multi)
{
	const std::string& icon_name = get_item_icon_name(asset_type, inventory_type, attachment_point, item_is_multi );
	return LLUI::getUIImage(icon_name);
}

const std::string LLInventoryPanel::DEFAULT_SORT_ORDER = std::string("InventorySortOrder");
const std::string LLInventoryPanel::RECENTITEMS_SORT_ORDER = std::string("RecentItemsSortOrder");
const std::string LLInventoryPanel::INHERIT_SORT_ORDER = std::string("");

LLInventoryPanel::LLInventoryPanel(const std::string& name,
								    const std::string& sort_order_setting,
									const LLRect& rect,
									LLInventoryModel* inventory,
									BOOL allow_multi_select,
									LLView *parent_view) :
	LLPanel(name, rect, TRUE),
	mInventory(inventory),
	mInventoryObserver(NULL),
	mFolders(NULL),
	mScroller(NULL),
	mAllowMultiSelect(allow_multi_select),
	mSortOrderSetting(sort_order_setting)
{
	setBackgroundColor(gColors.getColor("InventoryBackgroundColor"));
	setBackgroundVisible(TRUE);
	setBackgroundOpaque(TRUE);
}

BOOL LLInventoryPanel::postBuild()
{
	init_inventory_panel_actions(this);

	LLRect folder_rect(0,
					   0,
					   getRect().getWidth(),
					   0);
	mFolders = new LLFolderView(getName(), NULL, folder_rect, LLUUID::null, this);
	mFolders->setAllowMultiSelect(mAllowMultiSelect);

	// scroller
	LLRect scroller_view_rect = getRect();
	scroller_view_rect.translate(-scroller_view_rect.mLeft, -scroller_view_rect.mBottom);
	mScroller = new LLScrollableContainerView(std::string("Inventory Scroller"),
											   scroller_view_rect,
											  mFolders);
	mScroller->setFollowsAll();
	mScroller->setReserveScrollCorner(TRUE);
	addChild(mScroller);
	mFolders->setScrollContainer(mScroller);

	// set up the callbacks from the inventory we're viewing, and then
	// build everything.
	mInventoryObserver = new LLInventoryPanelObserver(this);
	mInventory->addObserver(mInventoryObserver);
	rebuildViewsFor(LLUUID::null, LLInventoryObserver::ADD);

	// bit of a hack to make sure the inventory is open.
	mFolders->openFolder(std::string("My Inventory"));

	if (mSortOrderSetting != INHERIT_SORT_ORDER)
	{
		setSortOrder(gSavedSettings.getU32(mSortOrderSetting));
	}
	else
	{
		setSortOrder(gSavedSettings.getU32(DEFAULT_SORT_ORDER));
	}
	mFolders->setSortOrder(mFolders->getFilter()->getSortOrder());


	return TRUE;
}

LLInventoryPanel::~LLInventoryPanel()
{
	// should this be a global setting?
	U32 sort_order = mFolders->getSortOrder();
	if (mSortOrderSetting != INHERIT_SORT_ORDER)
	{
		gSavedSettings.setU32(mSortOrderSetting, sort_order);
	}

	// LLView destructor will take care of the sub-views.
	mInventory->removeObserver(mInventoryObserver);
	delete mInventoryObserver;
	mScroller = NULL;
}

// virtual
LLXMLNodePtr LLInventoryPanel::getXML(bool save_children) const
{
	LLXMLNodePtr node = LLPanel::getXML(false); // Do not print out children

	node->setName(LL_INVENTORY_PANEL_TAG);

	node->createChild("allow_multi_select", TRUE)->setBoolValue(mFolders->getAllowMultiSelect());

	return node;
}

LLView* LLInventoryPanel::fromXML(LLXMLNodePtr node, LLView *parent, LLUICtrlFactory *factory)
{
	LLInventoryPanel* panel;

	std::string name("inventory_panel");
	node->getAttributeString("name", name);

	BOOL allow_multi_select = TRUE;
	node->getAttributeBOOL("allow_multi_select", allow_multi_select);

	LLRect rect;
	createRect(node, rect, parent, LLRect());

	std::string sort_order(INHERIT_SORT_ORDER);
	node->getAttributeString("sort_order", sort_order);

	panel = new LLInventoryPanel(name, sort_order,
								 rect, &gInventory,
								 allow_multi_select, parent);

	panel->initFromXML(node, parent);

	panel->postBuild();

	return panel;
}

void LLInventoryPanel::draw()
{
	// select the desired item (in case it wasn't loaded when the selection was requested)
	if (mSelectThisID.notNull())
	{
		setSelection(mSelectThisID, false);
	}
	LLPanel::draw();
}

void LLInventoryPanel::setFilterTypes(U32 filter_types)
{
	mFolders->getFilter()->setFilterTypes(filter_types);
}	

void LLInventoryPanel::setFilterPermMask(PermissionMask filter_perm_mask)
{
	mFolders->getFilter()->setFilterPermissions(filter_perm_mask);
}

void LLInventoryPanel::setFilterSubString(const std::string& string)
{
	mFolders->getFilter()->setFilterSubString(string);
}

void LLInventoryPanel::setFilterWorn(bool worn)
{
	mFolders->getFilter()->setFilterWorn(worn);
}

void LLInventoryPanel::setSortOrder(U32 order)
{
	mFolders->getFilter()->setSortOrder(order);
	if (mFolders->getFilter()->isModified())
	{
		mFolders->setSortOrder(order);
		// try to keep selection onscreen, even if it wasn't to start with
		mFolders->scrollToShowSelection();
	}
}

void LLInventoryPanel::setSinceLogoff(BOOL sl)
{
	mFolders->getFilter()->setDateRangeLastLogoff(sl);
}

void LLInventoryPanel::setHoursAgo(U32 hours)
{
	mFolders->getFilter()->setHoursAgo(hours);
}

void LLInventoryPanel::setShowFolderState(LLInventoryFilter::EFolderShow show)
{
	mFolders->getFilter()->setShowFolderState(show);
}

LLInventoryFilter::EFolderShow LLInventoryPanel::getShowFolderState()
{
	return mFolders->getFilter()->getShowFolderState();
}

void LLInventoryPanel::modelChanged(U32 mask)
{
	LLFastTimer t2(LLFastTimer::FTM_REFRESH);

	bool handled = false;
	if(mask & LLInventoryObserver::LABEL)
	{
		handled = true;
		// label change - empty out the display name for each object
		// in this change set.
		const std::set<LLUUID>& changed_items = gInventory.getChangedIDs();
		std::set<LLUUID>::const_iterator id_it = changed_items.begin();
		std::set<LLUUID>::const_iterator id_end = changed_items.end();
		LLFolderViewItem* view = NULL;
		LLInvFVBridge* bridge = NULL;
		for (;id_it != id_end; ++id_it)
		{
			view = mFolders->getItemByID(*id_it);
			if(view)
			{
				// request refresh on this item (also flags for filtering)
				bridge = (LLInvFVBridge*)view->getListener();
				if(bridge)
				{	// Clear the display name first, so it gets properly re-built during refresh()
					bridge->clearDisplayName();
				}
				view->refresh();
			}
		}
	}
	if((mask & (LLInventoryObserver::STRUCTURE
				| LLInventoryObserver::ADD
				| LLInventoryObserver::REMOVE)) != 0)
	{
		handled = true;
		// Record which folders are open by uuid.
		LLInventoryModel* model = getModel();
		if (model)
		{
			const std::set<LLUUID>& changed_items = gInventory.getChangedIDs();

			std::set<LLUUID>::const_iterator id_it = changed_items.begin();
			std::set<LLUUID>::const_iterator id_end = changed_items.end();
			for (;id_it != id_end; ++id_it)
			{
				// sync view with model
				LLInventoryObject* model_item = model->getObject(*id_it);
				LLFolderViewItem* view_item = mFolders->getItemByID(*id_it);

				if (model_item)
				{
					if (!view_item)
					{
						// this object was just created, need to build a view for it
						if ((mask & LLInventoryObserver::ADD) != LLInventoryObserver::ADD)
						{
							llwarns << *id_it << " is in model but not in view, but ADD flag not set" << llendl;
						}
						buildNewViews(*id_it);
						
						// select any newly created object
						// that has the auto rename at top of folder
						// root set
						if(mFolders->getRoot()->needsAutoRename())
						{
							setSelection(*id_it, FALSE);
						}
					}
					else
					{
						// this object was probably moved, check its parent
						if ((mask & LLInventoryObserver::STRUCTURE) != LLInventoryObserver::STRUCTURE)
						{
							llwarns << *id_it << " is in model and in view, but STRUCTURE flag not set" << llendl;
						}

						LLFolderViewFolder* new_parent = (LLFolderViewFolder*)mFolders->getItemByID(model_item->getParentUUID());
						if (view_item->getParentFolder() != new_parent)
						{
							view_item->getParentFolder()->extractItem(view_item);
							view_item->addToFolder(new_parent, mFolders);
						}
					}
				}
				else
				{
					if (view_item)
					{
						if ((mask & LLInventoryObserver::REMOVE) != LLInventoryObserver::REMOVE)
						{
							llwarns << *id_it << " is not in model but in view, but REMOVE flag not set" << llendl;
						}
						// item in view but not model, need to delete view
						view_item->destroyView();
					}
					else
					{
						llwarns << *id_it << "Item does not exist in either view or model, but notification triggered" << llendl;
					}
				}
			}
		}
	}

	if (!handled)
	{
		// it's a small change that only requires a refresh.
		// *TODO: figure out a more efficient way to do the refresh
		// since it is expensive on large inventories
		mFolders->refresh();
	}
}

void LLInventoryPanel::rebuildViewsFor(const LLUUID& id, U32 mask)
{
	LLFolderViewItem* old_view = NULL;

	// get old LLFolderViewItem
	old_view = mFolders->getItemByID(id);
	if (old_view && id.notNull())
	{
		old_view->destroyView();
	}

	buildNewViews(id);
}

void LLInventoryPanel::buildNewViews(const LLUUID& id)
{
	LLFolderViewItem* itemp = NULL;
	LLInventoryObject* objectp = gInventory.getObject(id);

	if (objectp)
	{		
		if (objectp->getType() <= LLAssetType::AT_NONE ||
			objectp->getType() >= LLAssetType::AT_COUNT)
		{
			llwarns << "LLInventoryPanel::buildNewViews called with objectp->mType == " 
				<< ((S32) objectp->getType())
				<< " (shouldn't happen)" << llendl;
		}
		else if (objectp->getType() == LLAssetType::AT_CATEGORY) // build new view for category
		{
			LLInvFVBridge* new_listener = LLInvFVBridge::createBridge(objectp->getType(),
													LLInventoryType::IT_CATEGORY,
													this,
													objectp->getUUID());

			if (new_listener)
			{
				LLFolderViewFolder* folderp = new LLFolderViewFolder(new_listener->getDisplayName(),
													new_listener->getIcon(),
													mFolders,
													new_listener);
				
				folderp->setItemSortOrder(mFolders->getSortOrder());
				itemp = folderp;
			}
		}
		else // build new view for item
		{
			LLInventoryItem* item = (LLInventoryItem*)objectp;
			LLInvFVBridge* new_listener = LLInvFVBridge::createBridge(
				item->getType(),
				item->getInventoryType(),
				this,
				item->getUUID(),
				item->getFlags());
			if (new_listener)
			{
				itemp = new LLFolderViewItem(new_listener->getDisplayName(),
												new_listener->getIcon(),
												new_listener->getCreationDate(),
												mFolders,
												new_listener);
			}
		}

		LLFolderViewFolder* parent_folder = (LLFolderViewFolder*)mFolders->getItemByID(objectp->getParentUUID());

		if (itemp)
		{
			if (parent_folder)
			{
				itemp->addToFolder(parent_folder, mFolders);
			}
			else
			{
				llwarns << "Couldn't find parent folder for child " << itemp->getLabel() << llendl;
				delete itemp;
			}
		}
	}

	if ((id.isNull() ||
		(objectp && objectp->getType() == LLAssetType::AT_CATEGORY)))
	{
		LLViewerInventoryCategory::cat_array_t* categories;
		LLViewerInventoryItem::item_array_t* items;

		mInventory->lockDirectDescendentArrays(id, categories, items);
		if(categories)
		{
			S32 count = categories->count();
			for(S32 i = 0; i < count; ++i)
			{
				LLInventoryCategory* cat = categories->get(i);
				buildNewViews(cat->getUUID());
			}
		}
		if(items)
		{
			S32 count = items->count();
			for(S32 i = 0; i < count; ++i)
			{
				LLInventoryItem* item = items->get(i);
				buildNewViews(item->getUUID());
			}
		}
		mInventory->unlockDirectDescendentArrays(id);
	}
}

struct LLConfirmPurgeData
{
	LLUUID mID;
	LLInventoryModel* mModel;
};

class LLIsNotWorn : public LLInventoryCollectFunctor
{
public:
	LLIsNotWorn() {}
	virtual ~LLIsNotWorn() {}
	virtual bool operator()(LLInventoryCategory* cat,
							LLInventoryItem* item)
	{
		return !gAgent.isWearingItem(item->getUUID());
	}
};

class LLOpenFolderByID : public LLFolderViewFunctor
{
public:
	LLOpenFolderByID(const LLUUID& id) : mID(id) {}
	virtual ~LLOpenFolderByID() {}
	virtual void doFolder(LLFolderViewFolder* folder)
		{
			if (folder->getListener() && folder->getListener()->getUUID() == mID) folder->setOpenArrangeRecursively(TRUE, LLFolderViewFolder::RECURSE_UP);
		}
	virtual void doItem(LLFolderViewItem* item) {}
protected:
	const LLUUID& mID;
};


void LLInventoryPanel::openSelected()
{
	LLFolderViewItem* folder_item = mFolders->getCurSelectedItem();
	if(!folder_item) return;
	LLInvFVBridge* bridge = (LLInvFVBridge*)folder_item->getListener();
	if(!bridge) return;
	bridge->openItem();
}

BOOL LLInventoryPanel::handleHover(S32 x, S32 y, MASK mask)
{
	BOOL handled = LLView::handleHover(x, y, mask);
	if(handled)
	{
		ECursorType cursor = getWindow()->getCursor();
		if (LLInventoryModel::backgroundFetchActive() && cursor == UI_CURSOR_ARROW)
		{
			// replace arrow cursor with arrow and hourglass cursor
			getWindow()->setCursor(UI_CURSOR_WORKING);
		}
	}
	else
	{
		getWindow()->setCursor(UI_CURSOR_ARROW);
	}
	return TRUE;
}

BOOL LLInventoryPanel::handleDragAndDrop(S32 x, S32 y, MASK mask, BOOL drop,
								   EDragAndDropType cargo_type,
								   void* cargo_data,
								   EAcceptance* accept,
								   std::string& tooltip_msg)
{

	BOOL handled = LLPanel::handleDragAndDrop(x, y, mask, drop, cargo_type, cargo_data, accept, tooltip_msg);

	if (handled)
	{
		mFolders->setDragAndDropThisFrame();
	}

	return handled;
}


void LLInventoryPanel::openAllFolders()
{
	mFolders->setOpenArrangeRecursively(TRUE, LLFolderViewFolder::RECURSE_DOWN);
	mFolders->arrangeAll();
}

void LLInventoryPanel::closeAllFolders()
{
	mFolders->setOpenArrangeRecursively(FALSE, LLFolderViewFolder::RECURSE_DOWN);
	mFolders->arrangeAll();
}

void LLInventoryPanel::openDefaultFolderForType(LLAssetType::EType type)
{
	LLUUID category_id = mInventory->findCategoryUUIDForType(type);
	LLOpenFolderByID opener(category_id);
	mFolders->applyFunctorRecursively(opener);
}

void LLInventoryPanel::setSelection(const LLUUID& obj_id, BOOL take_keyboard_focus)
{
	LLFolderViewItem* itemp = mFolders->getItemByID(obj_id);
	if(itemp && itemp->getListener())
	{
		itemp->getListener()->arrangeAndSet(itemp, TRUE, take_keyboard_focus);
		mSelectThisID.setNull();
		return;
	}
	else
	{
		// save the desired item to be selected later (if/when ready)
		mSelectThisID = obj_id;
	}
}

void LLInventoryPanel::clearSelection()
{
	mFolders->clearSelection();
	mSelectThisID.setNull();
}

void LLInventoryPanel::createNewItem(const std::string& name,
									const LLUUID& parent_id,
									LLAssetType::EType asset_type,
									LLInventoryType::EType inv_type,
									U32 next_owner_perm)
{
	std::string desc;
	LLAssetType::generateDescriptionFor(asset_type, desc);
	next_owner_perm = (next_owner_perm) ? next_owner_perm : PERM_MOVE | PERM_TRANSFER;

	
	if (inv_type == LLInventoryType::IT_GESTURE)
	{
		LLPointer<LLInventoryCallback> cb = new CreateGestureCallback();
		create_inventory_item(gAgent.getID(), gAgent.getSessionID(),
							  parent_id, LLTransactionID::tnull, name, desc, asset_type, inv_type,
							  NOT_WEARABLE, next_owner_perm, cb);
	}
	else
	{
		LLPointer<LLInventoryCallback> cb = NULL;
		create_inventory_item(gAgent.getID(), gAgent.getSessionID(),
							  parent_id, LLTransactionID::tnull, name, desc, asset_type, inv_type,
							  NOT_WEARABLE, next_owner_perm, cb);
	}
	
}	

// static DEBUG ONLY:
void LLInventoryPanel::dumpSelectionInformation(void* user_data)
{
	LLInventoryPanel* iv = (LLInventoryPanel*)user_data;
	iv->mFolders->dumpSelectionInformation();
}
