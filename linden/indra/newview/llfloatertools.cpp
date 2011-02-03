/** 
 * @file llfloatertools.cpp
 * @brief The edit tools, including move, position, land, etc.
 *
 * $LicenseInfo:firstyear=2002&license=viewergpl$
 * 
 * Copyright (c) 2002-2010, Linden Research, Inc.
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

#include "llfloatertools.h"

#include "llfontgl.h"
#include "llcoord.h"
#include "llgl.h"

#include "llagent.h"
#include "llbutton.h"
#include "llcheckboxctrl.h"
#include "llcombobox.h"
#include "lldraghandle.h"
#include "llfloaterbuildoptions.h"
#include "llfloateropenobject.h"
#include "llfocusmgr.h"
#include "llmenugl.h"
#include "llnotify.h"
#include "llpanelcontents.h"
#include "llpanelface.h"
#include "llpanelland.h"
#include "llpanelinventory.h"
#include "llpanelobject.h"
#include "llpanelvolume.h"
#include "llpanelpermissions.h"
#include "llresmgr.h"
#include "llselectmgr.h"
#include "llslider.h"
#include "llspinctrl.h"
#include "llstatusbar.h"
#include "lltabcontainer.h"
#include "lltextbox.h"
#include "lltoolbrush.h"
#include "lltoolcomp.h"
#include "lltooldraganddrop.h"
#include "lltoolface.h"
#include "lltoolfocus.h"
#include "lltoolgrab.h"
#include "lltoolgrab.h"
#include "lltoolindividual.h"
#include "lltoolmgr.h"
#include "lltoolpie.h"
#include "lltoolpipette.h"
#include "lltoolplacer.h"
#include "lltoolselectland.h"
#include "llui.h"
#include "llviewermenu.h"
#include "llviewerparcelmgr.h"
#include "llviewerwindow.h"
#include "llviewercontrol.h"
#include "llviewerjoystick.h"
#include "llvograss.h"
#include "llvotree.h"
#include "lluictrlfactory.h"
#include "qtoolalign.h"
#include "hippolimits.h"

// Globals
LLFloaterTools *gFloaterTools = NULL;


const std::string PANEL_NAMES[LLFloaterTools::PANEL_COUNT] =
{
	std::string("General"), 	// PANEL_GENERAL,
	std::string("Object"), 	// PANEL_OBJECT,
	std::string("Features"),	// PANEL_FEATURES,
	std::string("Texture"),	// PANEL_FACE,
	std::string("Content"),	// PANEL_CONTENTS,
};

// Local prototypes
void commit_select_tool(LLUICtrl *ctrl, void *data);
void commit_select_component(LLUICtrl *ctrl, void *data);
void click_show_more(void*);
void click_popup_info(void*);
void click_popup_done(void*);
void click_popup_minimize(void*);
void click_popup_grab_drag(LLUICtrl *, void*);
void click_popup_grab_lift(LLUICtrl *, void*);
void click_popup_grab_spin(LLUICtrl *, void*);
void click_popup_rotate_left(void*);
void click_popup_rotate_reset(void*);
void click_popup_rotate_right(void*);
void click_popup_dozer_mode(LLUICtrl *, void *user);
void commit_slider_dozer_size(LLUICtrl *, void*);
void commit_slider_dozer_force(LLUICtrl *, void*);
void click_apply_to_selection(void*);
void commit_radio_zoom(LLUICtrl *, void*);
void commit_radio_orbit(LLUICtrl *, void*);
void commit_radio_pan(LLUICtrl *, void*);
void commit_grid_mode(LLUICtrl *, void*);
void commit_slider_zoom(LLUICtrl *, void*);


//static
void*	LLFloaterTools::createPanelPermissions(void* data)
{
	LLFloaterTools* floater = (LLFloaterTools*)data;
	floater->mPanelPermissions = new LLPanelPermissions("General");
	return floater->mPanelPermissions;
}
//static
void*	LLFloaterTools::createPanelObject(void* data)
{
	LLFloaterTools* floater = (LLFloaterTools*)data;
	floater->mPanelObject = new LLPanelObject("Object");
	return floater->mPanelObject;
}

//static
void*	LLFloaterTools::createPanelVolume(void* data)
{
	LLFloaterTools* floater = (LLFloaterTools*)data;
	floater->mPanelVolume = new LLPanelVolume("Features");
	return floater->mPanelVolume;
}

//static
void*	LLFloaterTools::createPanelFace(void* data)
{
	LLFloaterTools* floater = (LLFloaterTools*)data;
	floater->mPanelFace = new LLPanelFace("Texture");
	return floater->mPanelFace;
}

//static
void*	LLFloaterTools::createPanelContents(void* data)
{
	LLFloaterTools* floater = (LLFloaterTools*)data;
	floater->mPanelContents = new LLPanelContents("Contents");
	return floater->mPanelContents;
}

//static
void*	LLFloaterTools::createPanelContentsInventory(void* data)
{
	LLFloaterTools* floater = (LLFloaterTools*)data;
	floater->mPanelContents->mPanelInventory = new LLPanelInventory(std::string("ContentsInventory"), LLRect());
	return floater->mPanelContents->mPanelInventory;
}

//static
void*	LLFloaterTools::createPanelLandInfo(void* data)
{
	LLFloaterTools* floater = (LLFloaterTools*)data;
	floater->mPanelLandInfo = new LLPanelLandInfo(std::string("land info panel"));
	return floater->mPanelLandInfo;
}

void LLFloaterTools::updateToolsSizeLimits()
{
	getChild<LLSpinCtrl>("Scale X")->setMinValue(gHippoLimits->getMinPrimScale());
	getChild<LLSpinCtrl>("Scale Y")->setMinValue(gHippoLimits->getMinPrimScale());
	getChild<LLSpinCtrl>("Scale Z")->setMinValue(gHippoLimits->getMinPrimScale());

	getChild<LLSpinCtrl>("Scale X")->setMaxValue(gHippoLimits->getMaxPrimScale());
	getChild<LLSpinCtrl>("Scale Y")->setMaxValue(gHippoLimits->getMaxPrimScale());
	getChild<LLSpinCtrl>("Scale Z")->setMaxValue(gHippoLimits->getMaxPrimScale());

	getChild<LLSpinCtrl>("Pos X")->setMinValue(gHippoLimits->getMinPrimXPos());
	getChild<LLSpinCtrl>("Pos Y")->setMinValue(gHippoLimits->getMinPrimYPos());
	getChild<LLSpinCtrl>("Pos Z")->setMinValue(gHippoLimits->getMinPrimZPos());

	getChild<LLSpinCtrl>("Pos X")->setMaxValue(gHippoLimits->getMaxPrimXPos());
	getChild<LLSpinCtrl>("Pos Y")->setMaxValue(gHippoLimits->getMaxPrimYPos());
	getChild<LLSpinCtrl>("Pos Z")->setMaxValue(gHippoLimits->getMaxPrimZPos());

	getChild<LLCheckBoxCtrl>("Physical Checkbox Ctrl")->setEnabled(gHippoLimits->mAllowPhysicalPrims);
}

void LLFloaterTools::updateToolsPrecision()
{
	U32 decimals = gSavedSettings.getU32("DecimalsForTools");
	if (decimals != mPrecision)
	{
		if (decimals > 5)
		{
			decimals = 5;
		}
		getChild<LLSpinCtrl>("Pos X")->setPrecision(decimals);
		getChild<LLSpinCtrl>("Pos Y")->setPrecision(decimals);
		getChild<LLSpinCtrl>("Pos Z")->setPrecision(decimals);
		getChild<LLSpinCtrl>("Scale X")->setPrecision(decimals);
		getChild<LLSpinCtrl>("Scale Y")->setPrecision(decimals);
		getChild<LLSpinCtrl>("Scale Z")->setPrecision(decimals);
		getChild<LLSpinCtrl>("Rot X")->setPrecision(decimals);
		getChild<LLSpinCtrl>("Rot Y")->setPrecision(decimals);
		getChild<LLSpinCtrl>("Rot Z")->setPrecision(decimals);
		mPrecision = decimals;
	}
}

BOOL	LLFloaterTools::postBuild()
{
	
	// Hide until tool selected
	setVisible(FALSE);

	// Since we constantly show and hide this during drags, don't
	// make sounds on visibility changes.
	setSoundFlags(LLView::SILENT);

	getDragHandle()->setEnabled( !gSavedSettings.getBOOL("ToolboxAutoMove") );

	LLRect rect;
	mBtnFocus = getChild<LLButton>("button focus");//btn;
	childSetAction("button focus",LLFloaterTools::setEditTool, (void*)LLToolCamera::getInstance());
	mBtnMove = getChild<LLButton>("button move");
	childSetAction("button move",LLFloaterTools::setEditTool, (void*)LLToolGrab::getInstance());
	mBtnEdit = getChild<LLButton>("button edit");
	childSetAction("button edit",LLFloaterTools::setEditTool, (void*)LLToolCompTranslate::getInstance());
	mBtnCreate = getChild<LLButton>("button create");
	childSetAction("button create",LLFloaterTools::setEditTool, (void*)LLToolCompCreate::getInstance());
	mBtnLand = getChild<LLButton>("button land" );
	childSetAction("button land",LLFloaterTools::setEditTool, (void*)LLToolSelectLand::getInstance());
	mTextStatus = getChild<LLTextBox>("text status");

	childSetCommitCallback("slider zoom",commit_slider_zoom,this);

	mRadioZoom = getChild<LLCheckBoxCtrl>("radio zoom");
	childSetCommitCallback("radio zoom",commit_radio_zoom,this);
	mRadioOrbit = getChild<LLCheckBoxCtrl>("radio orbit");
	childSetCommitCallback("radio orbit",commit_radio_orbit,this);
	mRadioPan = getChild<LLCheckBoxCtrl>("radio pan");
	childSetCommitCallback("radio pan",commit_radio_pan,this);

	mRadioMove = getChild<LLCheckBoxCtrl>("radio move");
	childSetCommitCallback("radio move",click_popup_grab_drag,this);
	mRadioLift = getChild<LLCheckBoxCtrl>("radio lift");
	childSetCommitCallback("radio lift",click_popup_grab_lift,this);
	mRadioSpin = getChild<LLCheckBoxCtrl>("radio spin");
	childSetCommitCallback("radio spin",click_popup_grab_spin,NULL);
	mRadioPosition = getChild<LLCheckBoxCtrl>("radio position");
	childSetCommitCallback("radio position",commit_select_tool,LLToolCompTranslate::getInstance());
	mRadioRotate = getChild<LLCheckBoxCtrl>("radio rotate");
	childSetCommitCallback("radio rotate",commit_select_tool,LLToolCompRotate::getInstance());
	mRadioStretch = getChild<LLCheckBoxCtrl>("radio stretch");
	childSetCommitCallback("radio stretch",commit_select_tool,LLToolCompScale::getInstance());
	mRadioSelectFace = getChild<LLCheckBoxCtrl>("radio select face");
	childSetCommitCallback("radio select face",commit_select_tool,LLToolFace::getInstance());
	mRadioAlign = getChild<LLCheckBoxCtrl>("radio align");
	childSetCommitCallback("radio align",commit_select_tool,QToolAlign::getInstance());
	mCheckSelectIndividual = getChild<LLCheckBoxCtrl>("checkbox edit linked parts");
	childSetValue("checkbox edit linked parts",(BOOL)gSavedSettings.getBOOL("EditLinkedParts"));
	childSetCommitCallback("checkbox edit linked parts",commit_select_component,this);
	mCheckSnapToGrid = getChild<LLCheckBoxCtrl>("checkbox snap to grid");
	childSetValue("checkbox snap to grid",(BOOL)gSavedSettings.getBOOL("SnapEnabled"));
	mBtnGridOptions = getChild<LLButton>("Options...");
	childSetAction("Options...",onClickGridOptions, this);
	mCheckStretchUniform = getChild<LLCheckBoxCtrl>("checkbox uniform");
	childSetValue("checkbox uniform",(BOOL)gSavedSettings.getBOOL("ScaleUniform"));
	mCheckStretchTexture = getChild<LLCheckBoxCtrl>("checkbox stretch textures");
	childSetValue("checkbox stretch textures",(BOOL)gSavedSettings.getBOOL("ScaleStretchTextures"));
	mTextGridMode = getChild<LLTextBox>("text ruler mode");
	mComboGridMode = getChild<LLComboBox>("combobox grid mode");
	childSetCommitCallback("combobox grid mode",commit_grid_mode, this);
	mBtnLink = getChild<LLButton>("link_btn");
	childSetAction("link_btn",onClickLink, this);
	mBtnUnlink = getChild<LLButton>("unlink_btn");
	childSetAction("unlink_btn",onClickUnlink, this);

	// Set the default size limits for spinners -- MC
	updateToolsSizeLimits();

	// Set the default decimal precision for spinners -- MC
	updateToolsPrecision();

	//
	// Create Buttons
	//

	static	const std::string	toolNames[]={
			"ToolCube",
			"ToolPrism",
			"ToolPyramid",
			"ToolTetrahedron",
			"ToolCylinder",
			"ToolHemiCylinder",
			"ToolCone",
			"ToolHemiCone",
			"ToolSphere",
			"ToolHemiSphere",
			"ToolTorus",
			"ToolTube",
			"ToolRing",
			"ToolTree",
			"ToolGrass"};
	void*	toolData[]={
			&LLToolPlacerPanel::sCube,
			&LLToolPlacerPanel::sPrism,
			&LLToolPlacerPanel::sPyramid,
			&LLToolPlacerPanel::sTetrahedron,
			&LLToolPlacerPanel::sCylinder,
			&LLToolPlacerPanel::sCylinderHemi,
			&LLToolPlacerPanel::sCone,
			&LLToolPlacerPanel::sConeHemi,
			&LLToolPlacerPanel::sSphere,
			&LLToolPlacerPanel::sSphereHemi,
			&LLToolPlacerPanel::sTorus,
			&LLToolPlacerPanel::sSquareTorus,
			&LLToolPlacerPanel::sTriangleTorus,
			&LLToolPlacerPanel::sTree,
			&LLToolPlacerPanel::sGrass};
	for(size_t t=0; t<LL_ARRAY_SIZE(toolNames); ++t)
	{
		LLButton *found = getChild<LLButton>(toolNames[t]);
		if(found)
		{
			found->setClickedCallback(setObjectType,toolData[t]);
			mButtons.push_back( found );
		}
		else
		{
			llwarns << "Tool button not found! DOA Pending." << llendl;
		}
	}
	mComboTreesGrass = getChild<LLComboBox>("trees_grass");
	childSetCommitCallback("trees_grass", onSelectTreesGrass, (void*)0);
	mCheckCopySelection = getChild<LLCheckBoxCtrl>("checkbox copy selection");
	childSetValue("checkbox copy selection",(BOOL)gSavedSettings.getBOOL("CreateToolCopySelection"));
	mCheckSticky = getChild<LLCheckBoxCtrl>("checkbox sticky");
	childSetValue("checkbox sticky",(BOOL)gSavedSettings.getBOOL("CreateToolKeepSelected"));
	mCheckCopyCenters = getChild<LLCheckBoxCtrl>("checkbox copy centers");
	childSetValue("checkbox copy centers",(BOOL)gSavedSettings.getBOOL("CreateToolCopyCenters"));
	mCheckCopyRotates = getChild<LLCheckBoxCtrl>("checkbox copy rotates");
	childSetValue("checkbox copy rotates",(BOOL)gSavedSettings.getBOOL("CreateToolCopyRotates"));
	mRadioSelectLand = getChild<LLCheckBoxCtrl>("radio select land");
	childSetCommitCallback("radio select land",commit_select_tool, LLToolSelectLand::getInstance());
	mRadioDozerFlatten = getChild<LLCheckBoxCtrl>("radio flatten");
	childSetCommitCallback("radio flatten",click_popup_dozer_mode,  (void*)0);
	mRadioDozerRaise = getChild<LLCheckBoxCtrl>("radio raise");
	childSetCommitCallback("radio raise",click_popup_dozer_mode,  (void*)1);
	mRadioDozerLower = getChild<LLCheckBoxCtrl>("radio lower");
	childSetCommitCallback("radio lower",click_popup_dozer_mode,  (void*)2);
	mRadioDozerSmooth = getChild<LLCheckBoxCtrl>("radio smooth");
	childSetCommitCallback("radio smooth",click_popup_dozer_mode,  (void*)3);
	mRadioDozerNoise = getChild<LLCheckBoxCtrl>("radio noise");
	childSetCommitCallback("radio noise",click_popup_dozer_mode,  (void*)4);
	mRadioDozerRevert = getChild<LLCheckBoxCtrl>("radio revert");
	childSetCommitCallback("radio revert",click_popup_dozer_mode,  (void*)5);
	mBtnApplyToSelection = getChild<LLButton>("button apply to selection");
	childSetAction("button apply to selection",click_apply_to_selection,  (void*)0);

	mSliderDozerSize = getChild<LLSlider>("slider brush size");
	childSetCommitCallback("slider brush size", commit_slider_dozer_size,  (void*)0);
	childSetValue( "slider brush size", gSavedSettings.getF32("LandBrushSize"));
	
	mSliderDozerForce = getChild<LLSlider>("slider force");
	childSetCommitCallback("slider force",commit_slider_dozer_force,  (void*)0);
	// the setting stores the actual force multiplier, but the slider is logarithmic, so we convert here
	childSetValue( "slider force", log10(gSavedSettings.getF32("LandBrushForce")));

	mTab = getChild<LLTabContainer>("Object Info Tabs");
	if(mTab)
	{
		mTab->setFollows(FOLLOWS_TOP | FOLLOWS_LEFT);
		mTab->setBorderVisible(FALSE);
		mTab->selectFirstTab();
	}

	mStatusText["rotate"] = getString("status_rotate");
	mStatusText["scale"] = getString("status_scale");
	mStatusText["move"] = getString("status_move");
	mStatusText["align"] = getString("status_align");
	mStatusText["modifyland"] = getString("status_modifyland");
	mStatusText["camera"] = getString("status_camera");
	mStatusText["grab"] = getString("status_grab");
	mStatusText["place"] = getString("status_place");
	mStatusText["selectland"] = getString("status_selectland");
	
	return TRUE;
}

// Create the popupview with a dummy center.  It will be moved into place
// during LLViewerWindow's per-frame hover processing.
LLFloaterTools::LLFloaterTools()
:	LLFloater(std::string("toolbox floater")),
	mBtnFocus(NULL),
	mBtnMove(NULL),
	mBtnEdit(NULL),
	mBtnCreate(NULL),
	mBtnLand(NULL),
	mTextStatus(NULL),

	mRadioOrbit(NULL),
	mRadioZoom(NULL),
	mRadioPan(NULL),

	mRadioMove(NULL),
	mRadioLift(NULL),
	mRadioSpin(NULL),

	mRadioPosition(NULL),
	mRadioRotate(NULL),
	mRadioStretch(NULL),
	mRadioSelectFace(NULL),
	mRadioAlign(NULL),
	mCheckSelectIndividual(NULL),

	mCheckSnapToGrid(NULL),
	mBtnGridOptions(NULL),
	mTextGridMode(NULL),
	mComboGridMode(NULL),
	mCheckStretchUniform(NULL),
	mCheckStretchTexture(NULL),

	mBtnRotateLeft(NULL),
	mBtnRotateReset(NULL),
	mBtnRotateRight(NULL),

	mBtnDelete(NULL),
	mBtnDuplicate(NULL),
	mBtnDuplicateInPlace(NULL),

	mBtnLink(NULL),
	mBtnUnlink(NULL),

	mComboTreesGrass(NULL),
	mCheckSticky(NULL),
	mCheckCopySelection(NULL),
	mCheckCopyCenters(NULL),
	mCheckCopyRotates(NULL),
	mRadioSelectLand(NULL),
	mRadioDozerFlatten(NULL),
	mRadioDozerRaise(NULL),
	mRadioDozerLower(NULL),
	mRadioDozerSmooth(NULL),
	mRadioDozerNoise(NULL),
	mRadioDozerRevert(NULL),
	mSliderDozerSize(NULL),
	mSliderDozerForce(NULL),
	mBtnApplyToSelection(NULL),

	mTab(NULL),
	mPanelPermissions(NULL),
	mPanelObject(NULL),
	mPanelVolume(NULL),
	mPanelContents(NULL),
	mPanelFace(NULL),
	mPanelLandInfo(NULL),

	mTabLand(NULL),
	mDirty(TRUE),
	mPrecision(3)
{
	setAutoFocus(FALSE);
	LLCallbackMap::map_t factory_map;
	factory_map["General"] = LLCallbackMap(createPanelPermissions, this);//LLPanelPermissions
	factory_map["Object"] = LLCallbackMap(createPanelObject, this);//LLPanelObject
	factory_map["Features"] = LLCallbackMap(createPanelVolume, this);//LLPanelVolume
	factory_map["Texture"] = LLCallbackMap(createPanelFace, this);//LLPanelFace
	factory_map["Contents"] = LLCallbackMap(createPanelContents, this);//LLPanelContents
	factory_map["ContentsInventory"] = LLCallbackMap(createPanelContentsInventory, this);//LLPanelContents
	factory_map["land info panel"] = LLCallbackMap(createPanelLandInfo, this);//LLPanelLandInfo

	LLUICtrlFactory::getInstance()->buildFloater(this,"floater_tools.xml",&factory_map,FALSE);
}

LLFloaterTools::~LLFloaterTools()
{
	// children automatically deleted
}

void LLFloaterTools::setStatusText(const std::string& text)
{
	std::map<std::string, std::string>::iterator iter = mStatusText.find(text);
	if (iter != mStatusText.end())
	{
		mTextStatus->setText(iter->second);
	}
	else
	{
		mTextStatus->setText(text);
	}
}

void LLFloaterTools::refresh()
{
	const S32 INFO_WIDTH = getRect().getWidth();
	const S32 INFO_HEIGHT = 384;
	LLRect object_info_rect(0, 0, INFO_WIDTH, -INFO_HEIGHT);
	BOOL all_volume = LLSelectMgr::getInstance()->selectionAllPCode( LL_PCODE_VOLUME );

	S32 idx_features = mTab->getPanelIndexByTitle(PANEL_NAMES[PANEL_FEATURES]);
	S32 idx_face = mTab->getPanelIndexByTitle(PANEL_NAMES[PANEL_FACE]);
	S32 idx_contents = mTab->getPanelIndexByTitle(PANEL_NAMES[PANEL_CONTENTS]);

	S32 selected_index = mTab->getCurrentPanelIndex();

	if (!all_volume && (selected_index == idx_features || selected_index == idx_face ||
		selected_index == idx_contents))
	{
		mTab->selectFirstTab();
	}

	mTab->enableTabButton(idx_features, all_volume);
	mTab->enableTabButton(idx_face, all_volume);
	mTab->enableTabButton(idx_contents, all_volume);

	// Added in Link Num value -HgB
    if (gSavedSettings.getBOOL("EditLinkedParts") && LLSelectMgr::getInstance()->getEditSelection()->getObjectCount() == 1) //Selecting a single prim in "Edit Linked" mode, show link number
    {
		childSetVisible("obj_count", FALSE);
		childSetVisible("prim_count", FALSE);
		childSetVisible("link_num", TRUE);

		std::string value_string = "";
		LLViewerObject* selected = LLSelectMgr::getInstance()->getSelection()->getFirstObject();
		if (selected && selected->getRootEdit())
		{
			LLViewerObject::child_list_t children = selected->getRootEdit()->getChildren();
			if (children.empty())
			{
				// An unlinked prim is "link 0".
				value_string = "0";
			}
			else 
			{
				children.push_front(selected->getRootEdit()); // need root in the list too
				S32 index = 0;
				for (LLViewerObject::child_list_t::iterator iter = children.begin(); iter != children.end(); ++iter)
				{
					index++;
					if ((*iter)->isSelected())
					{
						LLResMgr::getInstance()->getIntegerString(value_string, index);
						break;
					}
				}
			}
		}
		childSetTextArg("link_num", "[NUMBER]", value_string);
	}
	else
	{
		// Refresh object and prim count labels
		childSetVisible("obj_count", TRUE);
		childSetVisible("prim_count", TRUE);
		childSetVisible("link_num", FALSE);
		LLLocale locale(LLLocale::USER_LOCALE);
		std::string obj_count_string;
		LLResMgr::getInstance()->getIntegerString(obj_count_string, LLSelectMgr::getInstance()->getSelection()->getRootObjectCount());
		childSetTextArg("obj_count",  "[COUNT]", obj_count_string);	
		std::string prim_count_string;
		LLResMgr::getInstance()->getIntegerString(prim_count_string, LLSelectMgr::getInstance()->getSelection()->getObjectCount());
		childSetTextArg("prim_count", "[COUNT]", prim_count_string);
	}

	updateToolsPrecision();

	// Refresh child tabs
	mPanelPermissions->refresh();
	mPanelObject->refresh();
	mPanelVolume->refresh();
	mPanelFace->refresh();
	mPanelContents->refresh();
	mPanelLandInfo->refresh();
}

void LLFloaterTools::draw()
{
	if (mDirty)
	{
		refresh();
		mDirty = FALSE;
	}

	//	mCheckSelectIndividual->set(gSavedSettings.getBOOL("EditLinkedParts"));
	LLFloater::draw();
}

void LLFloaterTools::dirty()
{
	mDirty = TRUE; 
	LLFloaterOpenObject::dirty();
}

// Clean up any tool state that should not persist when the
// floater is closed.
void LLFloaterTools::resetToolState()
{
	gCameraBtnZoom = TRUE;
	gCameraBtnOrbit = FALSE;
	gCameraBtnPan = FALSE;

	gGrabBtnSpin = FALSE;
	gGrabBtnVertical = FALSE;
}

void LLFloaterTools::updatePopup(LLCoordGL center, MASK mask)
{
	LLTool *tool = LLToolMgr::getInstance()->getCurrentTool();

	// HACK to allow seeing the buttons when you have the app in a window.
	// Keep the visibility the same as it 
	if (tool == gToolNull)
	{
		return;
	}

	if ( isMinimized() )
	{	// SL looks odd if we draw the tools while the window is minimized
		return;
	}
	
	// Focus buttons
	BOOL focus_visible = (	tool == LLToolCamera::getInstance() );

	mBtnFocus	->setToggleState( focus_visible );

	mRadioZoom	->setVisible( focus_visible );
	mRadioOrbit	->setVisible( focus_visible );
	mRadioPan	->setVisible( focus_visible );
	childSetVisible("slider zoom", focus_visible);
	childSetEnabled("slider zoom", gCameraBtnZoom);

	mRadioZoom	->set(	!gCameraBtnOrbit &&
						!gCameraBtnPan &&
						!(mask == MASK_ORBIT) &&
						!(mask == (MASK_ORBIT | MASK_ALT)) &&
						!(mask == MASK_PAN) &&
						!(mask == (MASK_PAN | MASK_ALT)) );

	mRadioOrbit	->set(	gCameraBtnOrbit || 
						(mask == MASK_ORBIT) ||
						(mask == (MASK_ORBIT | MASK_ALT)) );

	mRadioPan	->set(	gCameraBtnPan ||
						(mask == MASK_PAN) ||
						(mask == (MASK_PAN | MASK_ALT)) );

	// multiply by correction factor because volume sliders go [0, 0.5]
	childSetValue( "slider zoom", gAgent.getCameraZoomFraction() * 0.5f);

	// Move buttons
	BOOL move_visible = (tool == LLToolGrab::getInstance());

	if (mBtnMove) mBtnMove	->setToggleState( move_visible );

	// HACK - highlight buttons for next click
	if (mRadioMove)
	{
		mRadioMove	->setVisible( move_visible );
		mRadioMove	->set(	!gGrabBtnSpin && 
							!gGrabBtnVertical &&
							!(mask == MASK_VERTICAL) && 
							!(mask == MASK_SPIN) );
	}

	if (mRadioLift)
	{
		mRadioLift	->setVisible( move_visible );
		mRadioLift	->set(	gGrabBtnVertical || 
							(mask == MASK_VERTICAL) );
	}

	if (mRadioSpin)
	{
		mRadioSpin	->setVisible( move_visible );
		mRadioSpin	->set(	gGrabBtnSpin || 
							(mask == MASK_SPIN) );
	}

	// Edit buttons
	BOOL edit_visible = tool == LLToolCompTranslate::getInstance() ||
						tool == LLToolCompRotate::getInstance() ||
						tool == LLToolCompScale::getInstance() ||
						tool == LLToolFace::getInstance() ||
						tool == LLToolIndividual::getInstance() ||
						tool == QToolAlign::getInstance() ||
						tool == LLToolPipette::getInstance();

	mBtnEdit	->setToggleState( edit_visible );

	mRadioPosition	->setVisible( edit_visible );
	mRadioRotate	->setVisible( edit_visible );
	mRadioStretch	->setVisible( edit_visible );
	mRadioAlign		->setVisible( edit_visible );
	if (mRadioSelectFace)
	{
		mRadioSelectFace->setVisible( edit_visible );
		mRadioSelectFace->set( tool == LLToolFace::getInstance() );
	}

	if (mCheckSelectIndividual)
	{
		mCheckSelectIndividual->setVisible(edit_visible);
		//mCheckSelectIndividual->set(gSavedSettings.getBOOL("EditLinkedParts"));
	}

	mRadioPosition	->set( tool == LLToolCompTranslate::getInstance() );
	mRadioRotate	->set( tool == LLToolCompRotate::getInstance() );
	mRadioStretch	->set( tool == LLToolCompScale::getInstance() );
	mRadioAlign->set( tool == QToolAlign::getInstance() );

	if (mComboGridMode) 
	{
		mComboGridMode->setVisible( edit_visible );
		S32 index = mComboGridMode->getCurrentIndex();
		mComboGridMode->removeall();

		switch (mObjectSelection->getSelectType())
		{
		case SELECT_TYPE_HUD:
		  mComboGridMode->add(getString("grid_screen_text"));
		  mComboGridMode->add(getString("grid_local_text"));
		  //mComboGridMode->add(getString("grid_reference_text"));
		  break;
		case SELECT_TYPE_WORLD:
		  mComboGridMode->add(getString("grid_world_text"));
		  mComboGridMode->add(getString("grid_local_text"));
		  mComboGridMode->add(getString("grid_reference_text"));
		  break;
		case SELECT_TYPE_ATTACHMENT:
		  mComboGridMode->add(getString("grid_attachment_text"));
		  mComboGridMode->add(getString("grid_local_text"));
		  mComboGridMode->add(getString("grid_reference_text"));
		  break;
		}

		mComboGridMode->setCurrentByIndex(index);
	}
	if (mTextGridMode) mTextGridMode->setVisible( edit_visible );

	// Snap to grid disabled for grab tool - very confusing
	if (mCheckSnapToGrid) mCheckSnapToGrid->setVisible( edit_visible /* || tool == LLToolGrab::getInstance() */ );
	if (mBtnGridOptions) mBtnGridOptions->setVisible( edit_visible /* || tool == LLToolGrab::getInstance() */ );

	//mCheckSelectLinked	->setVisible( edit_visible );
	if (mCheckStretchUniform) mCheckStretchUniform->setVisible( edit_visible );
	if (mCheckStretchTexture) mCheckStretchTexture->setVisible( edit_visible );

	if (mBtnLink) mBtnLink->setVisible( edit_visible );
	if (mBtnUnlink) mBtnUnlink->setVisible( edit_visible );

	//TODO: Move these into llselectmgr
	// Check to see if we can link things
	bool can_link = false;
	if (!gSavedSettings.getBOOL("EditLinkedParts"))
	{
		if(LLSelectMgr::getInstance()->selectGetAllRootsValid() && LLSelectMgr::getInstance()->getSelection()->getRootObjectCount() >= 2)
		{
			struct f : public LLSelectedObjectFunctor
			{
				virtual bool apply(LLViewerObject* object)
				{
					return object->permModify();
				}
			}
			func;
			const bool firstonly = true;
			can_link = LLSelectMgr::getInstance()->getSelection()->applyToRootObjects(&func, firstonly);
		}
	}
	mBtnLink->setEnabled(can_link);

	// Check to see if we can unlink things
	bool can_unlink = false;
	if (tool != LLToolFace::getInstance())
	{
		if (LLSelectMgr::getInstance()->selectGetAllRootsValid() &&
			LLSelectMgr::getInstance()->getSelection()->getFirstEditableObject() &&
			!LLSelectMgr::getInstance()->getSelection()->isAttachment())
		{
			// LL's viewer unlinks the last linkset selected, 
			// regardless of how many linksets or prims are selected total. 
			// Preserve that behavior when enabling the unlink option.  
			if (gSavedSettings.getBOOL("EditLinkedParts"))
			{
				struct f : public LLSelectedNodeFunctor
				{
					virtual bool apply(LLSelectNode* pNode)
					{
						// Return the first selection node that is
						//    1) not a root prim
						//    2) or a root prim that has child prims
						// or in other words: any prim that is part of a linkset
						return (pNode->getObject() != pNode->getObject()->getRootEdit()) || 
								(pNode->getObject()->numChildren() != 0);
					}
				} func;

				if (LLSelectMgr::getInstance()->getSelection()->getFirstRootNode(&func, TRUE))
				{
					// the selection contains at least one prim (child or root) that is part of a linkset
					can_unlink = true;
				}
			}
			else
			{
				if (LLSelectMgr::getInstance()->getSelection()->getRootObjectCount() != 
					LLSelectMgr::getInstance()->getSelection()->getObjectCount())
				{
					can_unlink = true;
				}
			}
		}
	}
	mBtnUnlink->setEnabled(can_unlink);


	// Create buttons
	BOOL create_visible = (tool == LLToolCompCreate::getInstance());

	mBtnCreate	->setToggleState(	tool == LLToolCompCreate::getInstance() );

	updateTreeGrassCombo(create_visible);

	if (mCheckCopySelection
		&& mCheckCopySelection->get())
	{
		// don't highlight any placer button
		for (std::vector<LLButton*>::size_type i = 0; i < mButtons.size(); i++)
		{
			mButtons[i]->setToggleState(FALSE);
			mButtons[i]->setVisible( create_visible );
		}
	}
	else
	{
		// Highlight the correct placer button
		for( std::vector<LLButton*>::size_type i = 0; i < mButtons.size(); i++ )
		{
			LLPCode pcode = LLToolPlacer::getObjectType();
			void *userdata = mButtons[i]->getCallbackUserData();
			LLPCode *cur = (LLPCode*) userdata;

			BOOL state = (pcode == *cur);
			mButtons[i]->setToggleState( state );
			mButtons[i]->setVisible( create_visible );
		}
	}

	if (mCheckSticky) mCheckSticky		->setVisible( create_visible );
	if (mCheckCopySelection) mCheckCopySelection	->setVisible( create_visible );
	if (mCheckCopyCenters) mCheckCopyCenters	->setVisible( create_visible );
	if (mCheckCopyRotates) mCheckCopyRotates	->setVisible( create_visible );

	if (mCheckCopyCenters) mCheckCopyCenters->setEnabled( mCheckCopySelection->get() );
	if (mCheckCopyRotates) mCheckCopyRotates->setEnabled( mCheckCopySelection->get() );

	// Land buttons
	BOOL land_visible = (tool == LLToolBrushLand::getInstance() || tool == LLToolSelectLand::getInstance() );

	if (mBtnLand)	mBtnLand	->setToggleState( land_visible );

	//	mRadioEditLand	->set( tool == LLToolBrushLand::getInstance() );
	if (mRadioSelectLand)	mRadioSelectLand->set( tool == LLToolSelectLand::getInstance() );

	//	mRadioEditLand	->setVisible( land_visible );
	if (mRadioSelectLand)	mRadioSelectLand->setVisible( land_visible );

	S32 dozer_mode = gSavedSettings.getS32("RadioLandBrushAction");

	if (mRadioDozerFlatten)
	{
		mRadioDozerFlatten	->set( tool == LLToolBrushLand::getInstance() && dozer_mode == 0);
		mRadioDozerFlatten	->setVisible( land_visible );
	}
	if (mRadioDozerRaise)
	{
		mRadioDozerRaise	->set( tool == LLToolBrushLand::getInstance() && dozer_mode == 1);
		mRadioDozerRaise	->setVisible( land_visible );
	}
	if (mRadioDozerLower)
	{
		mRadioDozerLower	->set( tool == LLToolBrushLand::getInstance() && dozer_mode == 2);
		mRadioDozerLower	->setVisible( land_visible );
	}
	if (mRadioDozerSmooth)
	{
		mRadioDozerSmooth	->set( tool == LLToolBrushLand::getInstance() && dozer_mode == 3);
		mRadioDozerSmooth	->setVisible( land_visible );
	}
	if (mRadioDozerNoise)
	{
		mRadioDozerNoise	->set( tool == LLToolBrushLand::getInstance() && dozer_mode == 4);
		mRadioDozerNoise	->setVisible( land_visible );
	}
	if (mRadioDozerRevert)
	{
		mRadioDozerRevert	->set( tool == LLToolBrushLand::getInstance() && dozer_mode == 5);
		mRadioDozerRevert	->setVisible( land_visible );
	}
	if (mBtnApplyToSelection)
	{
		mBtnApplyToSelection->setVisible( land_visible );
		mBtnApplyToSelection->setEnabled( land_visible && !LLViewerParcelMgr::getInstance()->selectionEmpty() && tool != LLToolSelectLand::getInstance());
	}
	if (mSliderDozerSize)
	{
		mSliderDozerSize	->setVisible( land_visible );
		childSetVisible("Bulldozer:", land_visible);
		childSetVisible("Dozer Size:", land_visible);
	}
	if (mSliderDozerForce)
	{
		mSliderDozerForce	->setVisible( land_visible );
		childSetVisible("Strength:", land_visible);
	}

	if (gSavedSettings.getBOOL("EditLinkedParts") && LLSelectMgr::getInstance()->getEditSelection()->getObjectCount() == 1)
	{
		childSetVisible("link_num", !land_visible);
	}
	else
	{
		childSetVisible("obj_count", !land_visible);
		childSetVisible("prim_count", !land_visible);
	}
	mTab->setVisible(!land_visible);
	mPanelLandInfo->setVisible(land_visible);
}


// virtual
BOOL LLFloaterTools::canClose()
{
	// don't close when quitting, so camera will stay put
	return !LLApp::isExiting();
}

// virtual
void LLFloaterTools::onOpen()
{
	mParcelSelection = LLViewerParcelMgr::getInstance()->getFloatingParcelSelection();
	mObjectSelection = LLSelectMgr::getInstance()->getEditSelection();
	
	// gMenuBarView->setItemVisible(std::string("Tools"), TRUE);
	// gMenuBarView->arrange();
}

// virtual
void LLFloaterTools::onClose(bool app_quitting)
{
	setMinimized(FALSE);
	setVisible(FALSE);
	mTab->setVisible(FALSE);

	LLViewerJoystick::getInstance()->moveAvatar(false);

	// Different from handle_reset_view in that it doesn't actually 
	//   move the camera if EditCameraMovement is not set.
	gAgent.resetView(gSavedSettings.getBOOL("EditCameraMovement"));
	
	// exit component selection mode
	LLSelectMgr::getInstance()->promoteSelectionToRoot();
	gSavedSettings.setBOOL("EditLinkedParts", FALSE);

	gViewerWindow->showCursor();

	resetToolState();

	mParcelSelection = NULL;
	mObjectSelection = NULL;

	// Switch back to basic toolset
	LLToolMgr::getInstance()->setCurrentToolset(gBasicToolset);
	// we were already in basic toolset, using build tools
	// so manually reset tool to default (pie menu tool)
	LLToolMgr::getInstance()->getCurrentToolset()->selectFirstTool();

	// gMenuBarView->setItemVisible(std::string("Tools"), FALSE);
	// gMenuBarView->arrange();
}

void LLFloaterTools::showPanel(EInfoPanel panel)
{
	llassert(panel >= 0 && panel < PANEL_COUNT);
	mTab->selectTabByName(PANEL_NAMES[panel]);
}

void click_popup_info(void*)
{
//	gBuildView->setPropertiesPanelOpen(TRUE);
}

void click_popup_done(void*)
{
	handle_reset_view();
}

void click_popup_grab_drag(LLUICtrl*, void*)
{
	gGrabBtnVertical = FALSE;
	gGrabBtnSpin = FALSE;
}

void click_popup_grab_lift(LLUICtrl*, void*)
{
	gGrabBtnVertical = TRUE;
	gGrabBtnSpin = FALSE;
}

void click_popup_grab_spin(LLUICtrl*, void*)
{
	gGrabBtnVertical = FALSE;
	gGrabBtnSpin = TRUE;
}

void commit_radio_zoom(LLUICtrl *, void*)
{
	gCameraBtnZoom = TRUE;
	gCameraBtnOrbit = FALSE;
	gCameraBtnPan = FALSE;
}

void commit_radio_orbit(LLUICtrl *, void*)
{
	gCameraBtnZoom = FALSE;
	gCameraBtnOrbit = TRUE;
	gCameraBtnPan = FALSE;
}

void commit_radio_pan(LLUICtrl *, void*)
{
	gCameraBtnZoom = FALSE;
	gCameraBtnOrbit = FALSE;
	gCameraBtnPan = TRUE;
}

void commit_slider_zoom(LLUICtrl *ctrl, void*)
{
	// renormalize value, since max "volume" level is 0.5 for some reason
	F32 zoom_level = (F32)ctrl->getValue().asReal() * 2.f; // / 0.5f;
	gAgent.setCameraZoomFraction(zoom_level);
}

void click_popup_rotate_left(void*)
{
	LLSelectMgr::getInstance()->selectionRotateAroundZ( 45.f );
	dialog_refresh_all();
}

void click_popup_rotate_reset(void*)
{
	LLSelectMgr::getInstance()->selectionResetRotation();
	dialog_refresh_all();
}

void click_popup_rotate_right(void*)
{
	LLSelectMgr::getInstance()->selectionRotateAroundZ( -45.f );
	dialog_refresh_all();
}


void click_popup_dozer_mode(LLUICtrl *, void *user)
{
	S32 mode = (S32)(intptr_t) user;
	gFloaterTools->setEditTool( LLToolBrushLand::getInstance() );
	gSavedSettings.setS32("RadioLandBrushAction", mode);
}

void commit_slider_dozer_size(LLUICtrl *ctrl, void*)
{
	F32 size = (F32)ctrl->getValue().asReal();
	gSavedSettings.setF32("LandBrushSize", size);
}

void commit_slider_dozer_force(LLUICtrl *ctrl, void*)
{
	// the slider is logarithmic, so we exponentiate to get the actual force multiplier
	F32 dozer_force = pow(10.f, (F32)ctrl->getValue().asReal());
	gSavedSettings.setF32("LandBrushForce", dozer_force);
}




void click_apply_to_selection(void* user)
{
	LLToolBrushLand::getInstance()->modifyLandInSelectionGlobal();
}

void commit_select_tool(LLUICtrl *ctrl, void *data)
{
	static BOOL* sShowParcelOwners = rebind_llcontrol<BOOL>("ShowParcelOwners", &gSavedSettings, true);
	
	S32 show_owners = *sShowParcelOwners;
	gFloaterTools->setEditTool(data);
	gSavedSettings.setBOOL("ShowParcelOwners", show_owners);
}

void commit_select_component(LLUICtrl *ctrl, void *data)
{
	LLFloaterTools* floaterp = (LLFloaterTools*)data;

	//forfeit focus
	if (gFocusMgr.childHasKeyboardFocus(floaterp))
	{
		gFocusMgr.setKeyboardFocus(NULL);
	}

	BOOL select_individuals = floaterp->mCheckSelectIndividual->get();
	gSavedSettings.setBOOL("EditLinkedParts", select_individuals);
	floaterp->dirty();

	if (select_individuals)
	{
		LLSelectMgr::getInstance()->demoteSelectionToIndividuals();
	}
	else
	{
		LLSelectMgr::getInstance()->promoteSelectionToRoot();
	}
}

void commit_grid_mode(LLUICtrl *ctrl, void *data)   
{   
	LLComboBox* combo = (LLComboBox*)ctrl;   
    
	LLSelectMgr::getInstance()->setGridMode((EGridMode)combo->getCurrentIndex());
} 

// static 
void LLFloaterTools::setObjectType( void* data )
{
	LLPCode pcode = *(LLPCode*) data;
	LLToolPlacer::setObjectType( pcode );
	gSavedSettings.setBOOL("CreateToolCopySelection", FALSE);
	gFloaterTools->updateTreeGrassCombo(true);
	gFocusMgr.setMouseCapture(NULL);
}

// static
void LLFloaterTools::onClickGridOptions(void* data)
{
	//LLFloaterTools* floaterp = (LLFloaterTools*)data;
	LLFloaterBuildOptions::show(NULL);
	// RN: this makes grid options dependent on build tools window
	//floaterp->addDependentFloater(LLFloaterBuildOptions::getInstance(), FALSE);
}

void LLFloaterTools::setEditTool(void* tool_pointer)
{
	select_tool(tool_pointer);
}

void LLFloaterTools::onFocusReceived()
{
	LLToolMgr::getInstance()->setCurrentToolset(gBasicToolset);
	LLFloater::onFocusReceived();
}

void LLFloaterTools::updateTreeGrassCombo(bool visible)
{
	LLTextBox* tree_grass_label = getChild<LLTextBox>("tree_grass_label");
	if (visible) 
	{
		LLPCode pcode = LLToolPlacer::getObjectType();
		std::map<std::string, S32>::iterator it, end;
		std::string selected;
		if (pcode == LLToolPlacerPanel::sTree) 
		{
			tree_grass_label->setVisible(visible);
			LLButton* button = getChild<LLButton>("ToolTree");
			tree_grass_label->setText(button->getToolTip());

			selected = gSavedSettings.getString("LastTree");
			it = LLVOTree::sSpeciesNames.begin();
			end = LLVOTree::sSpeciesNames.end();
		} 
		else if (pcode == LLToolPlacerPanel::sGrass) 
		{
			tree_grass_label->setVisible(visible);
			LLButton* button = getChild<LLButton>("ToolGrass");
			tree_grass_label->setText(button->getToolTip());

			selected = gSavedSettings.getString("LastGrass");
			it = LLVOGrass::sSpeciesNames.begin();
			end = LLVOGrass::sSpeciesNames.end();
		} 
		else 
		{
			mComboTreesGrass->removeall();
			mComboTreesGrass->setLabel(LLStringExplicit(""));  // LLComboBox::removeall() does not clear the label
			mComboTreesGrass->setEnabled(false);
			mComboTreesGrass->setVisible(false);
			tree_grass_label->setVisible(false);
			return;
		}

		mComboTreesGrass->removeall();
		mComboTreesGrass->add("Random");

		int select = 0, i = 0;

		while (it != end) 
		{
			const std::string &species = it->first;
			mComboTreesGrass->add(species);  ++i;
			if (species == selected) select = i;
			++it;
		}
		// if saved species not found, default to "Random"
		mComboTreesGrass->selectNthItem(select);
		mComboTreesGrass->setEnabled(true);
	}
	
	mComboTreesGrass->setVisible(visible);
	tree_grass_label->setVisible(visible);
}

// static
void LLFloaterTools::onSelectTreesGrass(LLUICtrl*, void*)
{
	const std::string &selected = gFloaterTools->mComboTreesGrass->getValue();
	LLPCode pcode = LLToolPlacer::getObjectType();
	if (pcode == LLToolPlacerPanel::sTree) 
	{
		gSavedSettings.setString("LastTree", selected);
	} 
	else if (pcode == LLToolPlacerPanel::sGrass) 
	{
		gSavedSettings.setString("LastGrass", selected);
	}  
}

// static
void LLFloaterTools::onClickLink(void* data)
{
	if(!LLSelectMgr::getInstance()->selectGetAllRootsValid())
	{
		LLNotifications::instance().add("UnableToLinkWhileDownloading");
		return;
	}

	S32 max_linked_prims = 0;
	LLViewerObject* first_rootp = LLSelectMgr::getInstance()->getSelection()->getFirstRootObject();
	if(first_rootp && first_rootp->usePhysics())
	{
		//Physical - use phys prim limit
		max_linked_prims = gHippoLimits->getMaxPhysLinkedPrims();
	}
	else
	{
		//Non phys limit
		max_linked_prims = gHippoLimits->getMaxLinkedPrims();
	}
	if (max_linked_prims > -1) //-1 : no limits
	{
		S32 object_count = LLSelectMgr::getInstance()->getSelection()->getObjectCount();
		if (object_count > max_linked_prims + 1)
		{
			LLSD args;
			args["COUNT"] = llformat("%d", object_count);
			args["MAX"] = llformat("%d", max_linked_prims +1);
			LLNotifications::instance().add("UnableToLinkObjects", args);
			return;
		}
	}
	
	if(LLSelectMgr::getInstance()->getSelection()->getRootObjectCount() < 2)
	{
		LLNotifications::instance().add("CannotLinkIncompleteSet");
		return;
	}
	if(!LLSelectMgr::getInstance()->selectGetRootsModify())
	{
		LLNotifications::instance().add("CannotLinkModify");
		return;
	}
	LLUUID owner_id;
	std::string owner_name;
	if(!LLSelectMgr::getInstance()->selectGetOwner(owner_id, owner_name))
	{
	  // we don't actually care if you're the owner, but novices are
	  // the most likely to be stumped by this one, so offer the
	  // easiest and most likely solution.
	  LLNotifications::instance().add("CannotLinkDifferentOwners");
	  return;
	}
	LLSelectMgr::getInstance()->sendLink();
	return;
}

// static
void LLFloaterTools::onClickUnlink(void* data)
{
	LLSelectMgr::getInstance()->sendDelink();
}
