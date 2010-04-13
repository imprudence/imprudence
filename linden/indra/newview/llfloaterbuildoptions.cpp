/** 
 * @file llfloaterbuildoptions.cpp
 * @brief LLFloaterBuildOptions class implementation
 *
 * $LicenseInfo:firstyear=2002&license=viewergpl$
 * 
 * Copyright (c) 2002-2009, Linden Research, Inc.
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

/**
 * Panel for setting global object-editing options, specifically
 * grid size and spacing.
 */ 

#include "llviewerprecompiledheaders.h"

#include "llfloaterbuildoptions.h"
#include "lluictrlfactory.h"

// library includes
#include "llfontgl.h"
#include "llcheckboxctrl.h"
#include "llcombobox.h"
#include "llspinctrl.h"
#include "llsliderctrl.h"
#include "lltexturectrl.h"

// newview includes
#include "llinventorymodel.h"
#include "llresmgr.h"
#include "llstartup.h" // not really necessary here, but just in case
#include "llviewercontrol.h"
#include "llviewerinventory.h"
#include "floaterbusy.h" // class InvDropTarget

//
// Globals
//
LLFloaterBuildOptions	*LLFloaterBuildOptions::sInstance = NULL;
InvDropTarget * LLFloaterBuildOptions::mBuildObjectDropTarget;

//
// Methods
//
LLFloaterBuildOptions::LLFloaterBuildOptions( )
: LLFloater(std::string("build options floater"))
{
	sInstance = this;
}

LLFloaterBuildOptions::~LLFloaterBuildOptions()
{
	sInstance = NULL;
	delete mBuildObjectDropTarget;
	mBuildObjectDropTarget = NULL;
}

BOOL LLFloaterBuildOptions::postBuild()
{
	getChild<LLComboBox>("material")->setSimple(gSavedSettings.getString("BuildPrefs_Material"));
	getChild<LLComboBox>("combobox shininess")->setSimple(gSavedSettings.getString("BuildPrefs_Shiny"));

	getChild<LLComboBox>("material")->setCommitCallback(onComboBoxCommit);
	getChild<LLComboBox>("combobox shininess")->setCommitCallback(onComboBoxCommit);

	getChild<LLTextureCtrl>("texture control")->setDefaultImageAssetID(LLUUID("89556747-24cb-43ed-920b-47caed15465f"));
	getChild<LLTextureCtrl>("texture control")->setCommitCallback(onTexturePickerCommit);

	LLView *target_view = getChild<LLView>("build_item_add_disp_rect");
	if (target_view)
	{
		if (mBuildObjectDropTarget) 
		{
			delete mBuildObjectDropTarget;
		}
		mBuildObjectDropTarget = new InvDropTarget("build drop target", target_view->getRect(),BuildAutoResponseItemDrop);
		addChild(mBuildObjectDropTarget);
	}

	if (LLStartUp::getStartupState() == STATE_STARTED)
	{
		LLUUID itemid = (LLUUID)gSavedSettings.getString("BuildPrefs_Item");
		LLViewerInventoryItem* item = gInventory.getItem(itemid);
		if (item)
		{
			childSetValue("build_item_add_disp_rect_txt", "Currently set to: "+item->getName());
		}
		else if (itemid.isNull())
		{
			childSetValue("build_item_add_disp_rect_txt", "Currently not set");
		}
		else
		{
			childSetValue("build_item_add_disp_rect_txt", "Currently set to a item not on this account");
		}
	}
	else
	{
		childSetValue("build_item_add_disp_rect_txt", "Not logged in");
	}

	return TRUE;
}

// static
void LLFloaterBuildOptions::show(void*)
{
	if (sInstance)
	{
		sInstance->open();	/*Flawfinder: ignore*/
	}
	else
	{
		LLFloaterBuildOptions* floater = new LLFloaterBuildOptions();

		LLUICtrlFactory::getInstance()->buildFloater(floater, "floater_build_options.xml");
		floater->open();	/*Flawfinder: ignore*/
	}
}

LLFloaterBuildOptions* LLFloaterBuildOptions::getInstance()
{
	return sInstance;
}

// static
BOOL LLFloaterBuildOptions::visible(void*)
{
	return (sInstance != NULL);
}

void LLFloaterBuildOptions::BuildAutoResponseItemDrop(LLViewerInventoryItem* item)
{
	gSavedSettings.setString("BuildPrefs_Item", item->getUUID().asString());
	LLFloaterBuildOptions::getInstance()->childSetValue("build_item_add_disp_rect_txt", "Currently set to: "+item->getName());
}

void LLFloaterBuildOptions::onComboBoxCommit(LLUICtrl* ctrl, void* userdata)
{
	
	LLComboBox* box = (LLComboBox*)ctrl;
	if(box)
	{
		gSavedSettings.setString(box->getControlName(), box->getValue().asString());
	}	
}

void LLFloaterBuildOptions::onTexturePickerCommit(LLUICtrl* ctrl, void* userdata)
{
	LLTextureCtrl*	image_ctrl = (LLTextureCtrl*)ctrl;
	if(image_ctrl)
	{
		gSavedSettings.setString("BuildPrefs_Texture", image_ctrl->getImageAssetID().asString());
	}
}
