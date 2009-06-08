/** 
 * @file llfloaterclothing.cpp
 * @author James Cook
 * @brief Read-only list of clothing from your inventory.
 *
 * $LicenseInfo:firstyear=2004&license=viewergpl$
 * 
 * Copyright (c) 2004-2009, Linden Research, Inc.
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

#include "llfloaterclothing.h"

#include "lldir.h"
#include "llinventory.h"

#include "llagent.h"
#include "llinventorymodel.h"
#include "llinventoryview.h"
#include "llkeyboard.h"
#include "llscrollbar.h"
#include "llscrolllistctrl.h"
#include "lluictrlfactory.h"
#include "llviewerinventory.h"
#include "llvoavatar.h"
#include "llviewercontrol.h"

const std::string LOADING_STRING("Loading...");

// static
LLFloaterClothing* LLFloaterClothing::sInstance = NULL;
LLFloaterClothingObserver* LLFloaterClothing::sObserver = NULL;

class LLFloaterClothingObserver : public LLInventoryObserver
{
public:
	LLFloaterClothingObserver() {}
	virtual ~LLFloaterClothingObserver() {}

	virtual void changed(U32 mask)
	{
		LLFloaterClothing::refreshAll();
	}
};

//---------------------------------------------------------------------------
// LLFloaterClothing
//---------------------------------------------------------------------------
LLFloaterClothing::LLFloaterClothing()
:	LLFloater(std::string("floater_clothing"), std::string("FloaterClothingRect"), LLStringUtil::null),
	mSelectedID(),
	mAllowSelection(FALSE)
{
	LLUICtrlFactory::getInstance()->buildFloater(this, "floater_clothing.xml");

	sInstance = this;

	sObserver = new LLFloaterClothingObserver;
	gInventory.addObserver(sObserver);

	childSetAction("take_off_btn", onClickTakeOff, this);
	childSetAction("wear_btn", onClickWear, this);

	childSetDoubleClickCallback("clothing_list", onClickWear);
	childSetCommitCallback("clothing_list", onCommitList, this);

	LLScrollListCtrl* list = getChild<LLScrollListCtrl>("clothing_list");
	if (list)
	{
		list->addCommentText(LOADING_STRING);
	}

	setDefaultBtn("wear_btn");

	gInventory.startBackgroundFetch();
}

// virtual
LLFloaterClothing::~LLFloaterClothing()
{
	gInventory.removeObserver(sObserver);
	delete sObserver;
	sObserver = NULL;

	sInstance = NULL;
}


// virtual
void LLFloaterClothing::onClose(bool app_quitting)
{
	gSavedSettings.setBOOL("ClothingBtnState", FALSE);

	LLFloater::onClose(app_quitting);
}


// static
void LLFloaterClothing::show(void*)
{
	if (sInstance)
	{
		sInstance->setVisibleAndFrontmost();
	}
	else
	{
		LLFloaterClothing *self = new LLFloaterClothing();

		self->buildClothingList();

		if (self->mAllowSelection)
		{
			LLCtrlSelectionInterface* iface = self->childGetSelectionInterface("clothing_list");
			if (iface) iface->selectFirstItem();
		}
		self->childSetFocus("clothing_list");
		
		self->mSelectedID = LLUUID::null;

		// Update button labels
		onCommitList(NULL, self);
	}

	gSavedSettings.setBOOL("ClothingBtnState", TRUE);
}

// static
void LLFloaterClothing::toggleVisibility()
{
	if(sInstance && sInstance->getVisible())
	{
		sInstance->close();
	}
	else
	{
		show();
	}
}

// static
void LLFloaterClothing::refreshAll()
{
	if (sInstance)
	{
		sInstance->buildClothingList();

		LLCtrlListInterface* list = sInstance->childGetListInterface("clothing_list");
		if (list)
		{
			if (!sInstance->mAllowSelection)
			{
				// no selection, no commit on change
				list->operateOnSelection(LLCtrlListInterface::OP_DESELECT);
			}
			else if (sInstance->mSelectedID.isNull())
			{
				list->selectFirstItem();
			}
			else
			{
				if (list->setCurrentByID(sInstance->mSelectedID))
				{
					LLCtrlScrollInterface *scroll = sInstance->childGetScrollInterface("clothing_list");
					if (scroll)
					{
						scroll->scrollToShowSelected();
					}
				}
				else
				{
					list->selectFirstItem();
				}
			}

			// Update button labels
			onCommitList(NULL, sInstance);
		}
	}
}

class LLIsClothing : public LLInventoryCollectFunctor
{
public:
	LLIsClothing() {}
	virtual ~LLIsClothing() {}
	virtual bool operator()(LLInventoryCategory* cat,
							LLInventoryItem* item)
	{
		if (item)
		{
			LLAssetType::EType at = item->getType();
			if (at == LLAssetType::AT_CLOTHING
				|| at == LLAssetType::AT_BODYPART)
			{
				U32 flags = item->getFlags();
				switch(flags)
				{
				case WT_SHAPE:
				case WT_SKIN:
				case WT_HAIR:
				case WT_EYES:
					return FALSE;
				case WT_SHIRT:
				case WT_PANTS:
				case WT_SKIRT:
				case WT_UNDERSHIRT:
				case WT_UNDERPANTS:
					return TRUE;
				default:
					return TRUE;
				}
			}
		}
		return FALSE;
	}
};



void LLFloaterClothing::buildClothingList()
{
	//llinfos << "buildClothingList" << llendl;

	LLScrollListCtrl* list = getChild<LLScrollListCtrl>("clothing_list");

	list->operateOnAll(LLCtrlListInterface::OP_DELETE);

	LLInventoryModel::cat_array_t cats;
	LLInventoryModel::item_array_t items;

	LLIsClothing is_clothing;
	gInventory.collectDescendentsIf(gAgent.getInventoryRootID(),
									cats,
									items,
									LLInventoryModel::EXCLUDE_TRASH,
									is_clothing);

	S32 count = items.count();
	for(S32 i = 0; i < count; ++i)
	{
		LLInventoryItem* item = items.get(i);

		LLSD row;
		row["id"] = item->getUUID();

		BOOL item_is_multi = FALSE;
		if ( item->getFlags() & LLInventoryItem::II_FLAGS_OBJECT_HAS_MULTIPLE_ITEMS )
		{
			item_is_multi = TRUE;
		}

		std::string icon_name = get_item_icon_name(item->getType(),
											 item->getInventoryType(),
											 item->getFlags(), item_is_multi);		// flags = wearable type
		row["columns"][0]["column"] = "icon";
		row["columns"][0]["type"] = "icon";
		row["columns"][0]["value"] = icon_name;

		std::string text = item->getName();
		std::string style = "NORMAL";
		if( gAgent.isWearingItem( item->getUUID() ) )
		{
			text.append(" (worn)");
			style = "BOLD";
		}
		row["columns"][1]["column"] = "name";
		row["columns"][1]["value"] = text;
		row["columns"][1]["font"] = "SANSSERIFSMALL";
		row["columns"][1]["font-style"] = style;

		// hidden column for sorting
		U32 flags = item->getFlags();	// flags = wearable type
		enum EWearableType wearable_type = (enum EWearableType)flags;
		const std::string& wearable_label = LLWearable::typeToTypeLabel(wearable_type);
		//line->addColumn(wearable_label, FONT, -1);	// invisible
		row["columns"][2]["column"] = "sort";
		row["columns"][2]["value"] = wearable_label;

		list->addElement(row);
	}

	if (count > 0)
	{
		mAllowSelection = TRUE;
	}
	else if (LLInventoryModel::backgroundFetchActive())
	{
		// We're loading
		list->addCommentText(LOADING_STRING);
		mAllowSelection = FALSE;
	}
	else
	{
		// Weird case, we're done loading but have no clothing
		list->addCommentText(std::string("No clothing found.")); // *TODO: Translate
		mAllowSelection = FALSE;
	}
}

// static
void LLFloaterClothing::onClickWear(void* data)
{
	LLFloaterClothing* self = (LLFloaterClothing*)data;

	const LLUUID& item_id = self->childGetValue("clothing_list").asUUID();

	LLViewerInventoryItem* inv_item = gInventory.getItem(item_id);
	if (!inv_item)
	{
		llwarns << "Can't find inventory item to wear" << llendl;
		return;
	}

	wear_inventory_item_on_avatar(inv_item);
}

BOOL wearable_can_take_off(EWearableType wearable_type)
{
	switch(wearable_type)
	{
	default:
	case WT_SHAPE:
	case WT_SKIN:
	case WT_HAIR:
	case WT_EYES:
		return FALSE;
	case WT_SHIRT:
	case WT_PANTS:
	case WT_SHOES:
	case WT_SOCKS:
	case WT_JACKET:
	case WT_GLOVES:
	case WT_SKIRT:
		return TRUE;
	case WT_UNDERSHIRT:
	case WT_UNDERPANTS:
		// can't take off underpants with PG accounts
		return (!gAgent.isTeen());
	}
}

// static
void LLFloaterClothing::onClickTakeOff(void* data)
{
	LLFloaterClothing* self = (LLFloaterClothing*)data;

	const LLUUID& item_id = self->childGetValue("clothing_list").asUUID();

	LLInventoryItem* inv_item = gInventory.getItem(item_id);
	if (!inv_item)
	{
		llwarns << "Can't find inventory item to wear" << llendl;
		return;
	}

	LLVOAvatar* avatar = gAgent.getAvatarObject();
	if( !avatar ) return;

	EWearableType wearable_type = (EWearableType)inv_item->getFlags();

	// Can only take off certain types
	if (!wearable_can_take_off(wearable_type)) return;

	LLWearable* wearable = gAgent.getWearable(wearable_type);
	if( !wearable ) return;

	gAgent.removeWearable(wearable_type);
}


// static
void LLFloaterClothing::onCommitList(LLUICtrl* ctrl, void* data)
{
	LLFloaterClothing* self = (LLFloaterClothing*)data;

	const LLUUID& item_id = self->childGetValue("clothing_list").asUUID();
	self->mSelectedID = item_id;

	LLVOAvatar* avatar = gAgent.getAvatarObject();
	if( !avatar ) return;

	if(gAgent.isWearingItem( item_id ) )
	{
		// If already wearing, can't wear again
		self->childDisable("wear_btn");

		LLInventoryItem* inv_item = gInventory.getItem(item_id);
		if (!inv_item) return;

		EWearableType wearable_type = (EWearableType)inv_item->getFlags();
		BOOL can_take_off = wearable_can_take_off(wearable_type);
		self->childSetEnabled("take_off_btn", can_take_off);
	}
	else
	{
		// Can always wear something you're not wearing now
		self->childEnable("wear_btn");
		self->childEnable("take_off_btn");
	}
}
