/** 
 * @file llfloaterbuildoptions.cpp
 * @brief LLFloaterBuildOptions class implementation
 *
 * $LicenseInfo:firstyear=2002&license=viewergpl$
 * 
 * Copyright (c) 2002-2009, Linden Research, Inc., 
 * 2010, McCabe Maxsted
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

// library includes
#include "llfontgl.h"
#include "llcheckboxctrl.h"
#include "llcombobox.h"
#include "llspinctrl.h"
#include "llsliderctrl.h"
#include "lltexturectrl.h"
#include "lluictrlfactory.h"

// newview includes
#include "llcolorswatch.h"
#include "llinventorymodel.h"
#include "llresmgr.h"
#include "llselectmgr.h"
#include "llstartup.h" // not really necessary here, but just in case
#include "llviewercontrol.h"
#include "llviewerinventory.h"
#include "floaterbusy.h" // class InvDropTarget

//
// Globals
//
LLFloaterBuildOptions	*LLFloaterBuildOptions::sInstance = NULL;
InvDropTarget * LLFloaterBuildOptions::sBuildObjectDropTarget;

//
// Methods
//
LLFloaterBuildOptions::LLFloaterBuildOptions()
: LLFloater(std::string("build options floater")),
mBuildItemUUID(NULL),
mBuildTextureUUID(NULL)
{
	sInstance = this;
}

LLFloaterBuildOptions::~LLFloaterBuildOptions()
{
	sInstance = NULL;
	delete sBuildObjectDropTarget;
	sBuildObjectDropTarget = NULL;
}

BOOL LLFloaterBuildOptions::postBuild()
{
	childSetAction("btn_ok", onClickOK, this);
	childSetAction("btn_apply", onClickApply, this);
	childSetAction("btn_cancel", onClickCancel, this);
	childSetAction("btn_clear", onClickClear, this);
	childSetAction("btn_reset", onClickReset, this);

	getChild<LLComboBox>("material")->setSimple(gSavedPerAccountSettings.getString("BuildPrefs_Material"));
	getChild<LLComboBox>("combobox shininess")->setSimple(gSavedPerAccountSettings.getString("BuildPrefs_Shiny"));

	childSetValue("BuildPrefsActualRoot_toggle", gSavedPerAccountSettings.getBOOL("BuildPrefs_ActualRoot") );
	childSetValue("PivotPercToggle", gSavedPerAccountSettings.getBOOL("BuildPrefs_PivotIsPercent") );
	childSetValue("FBToggle", gSavedPerAccountSettings.getBOOL("BuildPrefs_FullBright") );
	childSetValue("BuildPrefsEmbedItem", gSavedPerAccountSettings.getBOOL("BuildPrefs_EmbedItem") );
	childSetValue("PhysicalToggle", gSavedPerAccountSettings.getBOOL("BuildPrefs_Physical") );
	childSetValue("TemporaryToggle", gSavedPerAccountSettings.getBOOL("BuildPrefs_Temporary") );
	childSetValue("PhantomToggle", gSavedPerAccountSettings.getBOOL("BuildPrefs_Phantom") );

	childSetValue("Z pos", gSavedPerAccountSettings.getF32("BuildPrefs_PivotZ") );
	childSetValue("Y pos", gSavedPerAccountSettings.getF32("BuildPrefs_PivotY") );
	childSetValue("X pos", gSavedPerAccountSettings.getF32("BuildPrefs_PivotX") );
	childSetValue("glow", gSavedPerAccountSettings.getF32("BuildPrefs_Glow") );
	childSetValue("alpha", gSavedPerAccountSettings.getF32("BuildPrefs_Alpha") );
	childSetValue("Z size", gSavedPerAccountSettings.getF32("BuildPrefs_Zsize") );
	childSetValue("Y size", gSavedPerAccountSettings.getF32("BuildPrefs_Ysize") );
	childSetValue("X size", gSavedPerAccountSettings.getF32("BuildPrefs_Xsize") );

	getChild<LLColorSwatchCtrl>("colorswatch")->set(gSavedPerAccountSettings.getColor4("BuildPrefs_Color"), TRUE );
	getChild<LLTextureCtrl>("texture control")->setImageAssetID(LLUUID(gSavedPerAccountSettings.getString("BuildPrefs_Texture")));

	childSetValue("BuildPrefsRenderHighlight_toggle", gSavedSettings.getBOOL("RenderHighlightSelections") );
	childSetValue("grouplandrez", gSavedSettings.getBOOL("RezWithLandGroup") );
	childSetValue("GridSubUnit", gSavedSettings.getBOOL("GridSubUnit") );
	childSetValue("GridCrossSection", gSavedSettings.getBOOL("GridCrossSections") );

	childSetValue("GridResolution", gSavedSettings.getF32("GridResolution") );
	childSetValue("GridDrawSize", gSavedSettings.getF32("GridDrawSize") );
	childSetValue("GridOpacity", gSavedSettings.getF32("GridOpacity") );

	childSetValue("spinner_decimal", (F32)gSavedSettings.getU32("DecimalsForTools") );

	getChild<LLComboBox>("material")->setCommitCallback(onComboBoxCommit);
	getChild<LLComboBox>("combobox shininess")->setCommitCallback(onComboBoxCommit);

	getChild<LLTextureCtrl>("texture control")->setDefaultImageAssetID(LLUUID("89556747-24cb-43ed-920b-47caed15465f"));
	getChild<LLTextureCtrl>("texture control")->setCommitCallback(onTexturePickerCommit);
	getChild<LLTextureCtrl>("texture control")->setCallbackUserData(this);

	LLView *target_view = getChild<LLView>("build_item_add_disp_rect");
	if (target_view)
	{
		if (sBuildObjectDropTarget) 
		{
			delete sBuildObjectDropTarget;
		}
		sBuildObjectDropTarget = new InvDropTarget("build drop target", target_view->getRect(),BuildAutoResponseItemDrop);
		addChild(sBuildObjectDropTarget);
	}

	mBuildItemUUID = (LLUUID)gSavedPerAccountSettings.getString("BuildPrefs_Item");
	mBuildTextureUUID = (LLUUID)gSavedPerAccountSettings.getString("BuildPrefs_Texture");

	refreshItem(); // Refresh the item UI bits after everything's loaded -- MC

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

// static
void LLFloaterBuildOptions::BuildAutoResponseItemDrop(LLViewerInventoryItem* item)
{
	LLFloaterBuildOptions::getInstance()->setItem(item->getUUID());
	LLFloaterBuildOptions::getInstance()->refreshItem();
	/*LLFloaterBuildOptions::getInstance()->childSetValue("build_item_add_disp_rect_txt", "Currently set to: "+item->getName());
	gSavedPerAccountSettings.setString("BuildPrefs_Item", item->getUUID().asString());*/
}

// static
void LLFloaterBuildOptions::onComboBoxCommit(LLUICtrl* ctrl, void* userdata)
{
	LLComboBox* box = (LLComboBox*)ctrl;
	if(box)
	{
		gSavedPerAccountSettings.setString(box->getControlName(), box->getValue().asString());
	}
}

// static
void LLFloaterBuildOptions::onTexturePickerCommit(LLUICtrl* ctrl, void* userdata)
{
	LLFloaterBuildOptions* self = (LLFloaterBuildOptions*)userdata;

	LLTextureCtrl*	image_ctrl = (LLTextureCtrl*)ctrl;
	if(image_ctrl)
	{
		self->setTexture(image_ctrl->getImageAssetID());
		/*gSavedPerAccountSettings.setString("BuildPrefs_Texture", image_ctrl->getImageAssetID().asString());*/
	}
}

// static
void LLFloaterBuildOptions::onClickOK(void* userdata)
{
	LLFloaterBuildOptions* self = (LLFloaterBuildOptions*)userdata;
	self->apply();
	self->close();
}

// static
void LLFloaterBuildOptions::onClickApply(void* userdata)
{
	LLFloaterBuildOptions* self = (LLFloaterBuildOptions*)userdata;
	self->apply();
}

// static
void LLFloaterBuildOptions::onClickCancel(void* userdata)
{
	LLFloaterBuildOptions* self = (LLFloaterBuildOptions*)userdata;
	self->cancel();
}

// static
void LLFloaterBuildOptions::onClickClear(void* userdata)
{
	// Clear the inventory item
	LLFloaterBuildOptions* self = (LLFloaterBuildOptions*)userdata;
	//gSavedPerAccountSettings.setString("BuildPrefs_Item", LLUUID::null);
	//gSavedPerAccountSettings.getControl("BuildPrefs_Item")->resetToDefault(false);

	self->setItem(LLUUID::null);
	self->refreshItem();
}

// static
void LLFloaterBuildOptions::onClickReset(void* userdata)
{
	// Reset to defaults
	LLFloaterBuildOptions* self = (LLFloaterBuildOptions*)userdata;
	self->reset();
	self->refresh();
}

void LLFloaterBuildOptions::apply()
{
	gSavedPerAccountSettings.setBOOL("BuildPrefs_ActualRoot", childGetValue("BuildPrefsActualRoot_toggle").asBoolean() );
	gSavedPerAccountSettings.setBOOL("BuildPrefs_PivotIsPercent", childGetValue("PivotPercToggle").asBoolean() );
	gSavedPerAccountSettings.setBOOL("BuildPrefs_FullBright", childGetValue("FBToggle").asBoolean() );
	gSavedPerAccountSettings.setBOOL("BuildPrefs_EmbedItem", childGetValue("BuildPrefsEmbedItem").asBoolean() );
	gSavedPerAccountSettings.setBOOL("BuildPrefs_Physical", childGetValue("PhysicalToggle").asBoolean() );
	gSavedPerAccountSettings.setBOOL("BuildPrefs_Temporary", childGetValue("TemporaryToggle").asBoolean() );
	gSavedPerAccountSettings.setBOOL("BuildPrefs_Phantom", childGetValue("PhantomToggle").asBoolean() );

	gSavedPerAccountSettings.setF32("BuildPrefs_PivotZ", childGetValue("Z pos").asReal() );
	gSavedPerAccountSettings.setF32("BuildPrefs_PivotY", childGetValue("Y pos").asReal() );
	gSavedPerAccountSettings.setF32("BuildPrefs_PivotX", childGetValue("X pos").asReal() );
	gSavedPerAccountSettings.setF32("BuildPrefs_Glow", childGetValue("glow").asReal() );
	gSavedPerAccountSettings.setF32("BuildPrefs_Alpha", childGetValue("alpha").asReal() );
	gSavedPerAccountSettings.setF32("BuildPrefs_Zsize", childGetValue("Z size").asReal() );
	gSavedPerAccountSettings.setF32("BuildPrefs_Ysize", childGetValue("Y size").asReal() );
	gSavedPerAccountSettings.setF32("BuildPrefs_Xsize", childGetValue("X size").asReal() );

	gSavedPerAccountSettings.setString("BuildPrefs_Shiny", getChild<LLComboBox>("combobox shininess")->getValue().asString() );
	gSavedPerAccountSettings.setString("BuildPrefs_Material", getChild<LLComboBox>("material")->getValue().asString() );

	gSavedPerAccountSettings.setColor4("BuildPrefs_Color", getChild<LLColorSwatchCtrl>("colorswatch")->get());

	if (mBuildItemUUID != LLSD(gSavedPerAccountSettings.getString("BuildPrefs_Item")) )
	{
		gSavedPerAccountSettings.setString("BuildPrefs_Item", mBuildItemUUID.asString() );
	}

	if (mBuildTextureUUID != LLSD(gSavedPerAccountSettings.getString("BuildPrefs_Texture")) )
	{
		gSavedPerAccountSettings.setString("BuildPrefs_Texture", mBuildTextureUUID.asString() );
	}

	if (gSavedSettings.getBOOL("RenderHighlightSelections") != (BOOL)(childGetValue("BuildPrefsRenderHighlight_toggle").asBoolean()))
	{
		LLSelectMgr::sRenderSelectionHighlights = !LLSelectMgr::sRenderSelectionHighlights;
		gSavedSettings.setBOOL("RenderHighlightSelections", LLSelectMgr::sRenderSelectionHighlights);
	}

	gSavedSettings.setBOOL("RezWithLandGroup", childGetValue("grouplandrez").asBoolean() );
	gSavedSettings.setBOOL("GridSubUnit", childGetValue("GridSubUnit").asBoolean() );
	gSavedSettings.setBOOL("GridCrossSections", childGetValue("GridCrossSection").asBoolean() );

	gSavedSettings.setF32("GridResolution", childGetValue("GridResolution").asReal() );
	gSavedSettings.setF32("GridDrawSize", childGetValue("GridDrawSize").asReal() );
	gSavedSettings.setF32("GridOpacity", childGetValue("GridOpacity").asReal() );

	gSavedSettings.setU32("DecimalsForTools", childGetValue("spinner_decimal").asReal() );
}

void LLFloaterBuildOptions::cancel()
{
	close();
}

void LLFloaterBuildOptions::reset()
{
	getChild<LLComboBox>("material")->setSimple(gSavedPerAccountSettings.getControl("BuildPrefs_Material")->getDefault().asString() );
	getChild<LLComboBox>("combobox shininess")->setSimple(gSavedPerAccountSettings.getControl("BuildPrefs_Shiny")->getDefault().asString() );

	childSetValue("BuildPrefsActualRoot_toggle", gSavedPerAccountSettings.getControl("BuildPrefs_ActualRoot")->getDefault() );
	childSetValue("PivotPercToggle", gSavedPerAccountSettings.getControl("BuildPrefs_PivotIsPercent")->getDefault() );
	childSetValue("FBToggle", gSavedPerAccountSettings.getControl("BuildPrefs_FullBright")->getDefault() );
	childSetValue("BuildPrefsEmbedItem", gSavedPerAccountSettings.getControl("BuildPrefs_EmbedItem")->getDefault() );
	childSetValue("PhysicalToggle", gSavedPerAccountSettings.getControl("BuildPrefs_Physical")->getDefault() );
	childSetValue("TemporaryToggle", gSavedPerAccountSettings.getControl("BuildPrefs_Temporary")->getDefault() );
	childSetValue("PhantomToggle", gSavedPerAccountSettings.getControl("BuildPrefs_Phantom")->getDefault() );
	
	childSetValue("Z pos", gSavedPerAccountSettings.getControl("BuildPrefs_PivotZ")->getDefault() );
	childSetValue("Y pos", gSavedPerAccountSettings.getControl("BuildPrefs_PivotY")->getDefault() );
	childSetValue("X pos", gSavedPerAccountSettings.getControl("BuildPrefs_PivotX")->getDefault() );
	childSetValue("glow", gSavedPerAccountSettings.getControl("BuildPrefs_Glow")->getDefault() );
	childSetValue("alpha", gSavedPerAccountSettings.getControl("BuildPrefs_Alpha")->getDefault() );
	childSetValue("Z size", gSavedPerAccountSettings.getControl("BuildPrefs_Zsize")->getDefault() );
	childSetValue("Y size", gSavedPerAccountSettings.getControl("BuildPrefs_Ysize")->getDefault() );
	childSetValue("X size", gSavedPerAccountSettings.getControl("BuildPrefs_Xsize")->getDefault() );

	getChild<LLColorSwatchCtrl>("colorswatch")->set(gSavedPerAccountSettings.getControl("BuildPrefs_Color")->getDefault(), TRUE);
	getChild<LLTextureCtrl>("texture control")->setImageAssetID( (LLUUID)gSavedPerAccountSettings.getControl("BuildPrefs_Texture")->getDefault().asString() );

	mBuildItemUUID = (LLUUID)gSavedPerAccountSettings.getControl("BuildPrefs_Item")->getDefault().asString();
	mBuildTextureUUID = (LLUUID)gSavedPerAccountSettings.getControl("BuildPrefs_Texture")->getDefault().asString();

	childSetValue("BuildPrefsRenderHighlight_toggle", gSavedSettings.getControl("RenderHighlightSelections")->getDefault() );
	childSetValue("grouplandrez", gSavedSettings.getControl("RezWithLandGroup")->getDefault() );
	childSetValue("GridSubUnit", gSavedSettings.getControl("GridSubUnit")->getDefault() );
	childSetValue("GridCrossSection", gSavedSettings.getControl("GridCrossSections")->getDefault() );

	childSetValue("GridResolution", gSavedSettings.getControl("GridResolution")->getDefault() );
	childSetValue("GridDrawSize", gSavedSettings.getControl("GridDrawSize")->getDefault() );
	childSetValue("GridOpacity", gSavedSettings.getControl("GridOpacity")->getDefault() );

	childSetValue("spinner_decimal", gSavedSettings.getControl("DecimalsForTools")->getDefault() );
}

void LLFloaterBuildOptions::refresh()
{
	
}

void LLFloaterBuildOptions::setItem(const LLUUID& item)
{
	mBuildItemUUID = item;
}

void LLFloaterBuildOptions::setTexture(const LLUUID& texture)
{
	mBuildTextureUUID = texture;
}

void LLFloaterBuildOptions::refreshItem()
{
	if (LLStartUp::getStartupState() == STATE_STARTED)
	{
		if (mBuildItemUUID.isNull())
		{
			childSetValue("build_item_add_disp_rect_txt", "Currently not set");
		}
		else
		{
			LLViewerInventoryItem* item = gInventory.getItem(mBuildItemUUID);
			if (item)
			{
				childSetValue("build_item_add_disp_rect_txt", "Currently set to: "+item->getName());
			}
			
			else
			{
				childSetValue("build_item_add_disp_rect_txt", "Currently set to a item not on this account");
			}
		}
	}
	else
	{
		childSetValue("build_item_add_disp_rect_txt", "Not logged in");
	}
}
