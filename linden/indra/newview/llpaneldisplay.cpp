/** 
 * @file llpaneldisplay.cpp
 * @brief Display preferences for the preferences floater
 *
 * $LicenseInfo:firstyear=2001&license=viewergpl$
 * 
 * Copyright (c) 2001-2008, Linden Research, Inc.
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

// file include
#include "llpaneldisplay.h"

// linden library includes
#include "llerror.h"
#include "llfontgl.h"
#include "llrect.h"
#include "llstring.h"

// project includes
#include "llagent.h"
#include "llbutton.h"
#include "llcheckboxctrl.h"
#include "llcombobox.h"
#include "llflexibleobject.h"
#include "lllineeditor.h"
#include "llradiogroup.h"
#include "llresmgr.h"
#include "llsliderctrl.h"
#include "llspinctrl.h"
#include "llstartup.h"
#include "lltextbox.h"
#include "llui.h"
#include "llviewercamera.h"
#include "llviewerimage.h"
#include "llviewerimagelist.h"
#include "llviewermessage.h"
#include "llviewerobjectlist.h"
#include "llviewerwindow.h"
#include "llvoavatar.h"
#include "llvovolume.h"
#include "llvotree.h"
#include "llvosky.h"
#include "llwindow.h"
#include "llworld.h"
#include "pipeline.h"
#include "llvieweruictrlfactory.h"
#include "llfeaturemanager.h"
#include "llglslshader.h"
#include "llfloaterhardwaresettings.h"
#include "llboost.h"

//RN temporary includes for resolution switching
#include "llglheaders.h"
#include "llviewercontrol.h"
#include "llsky.h"

// parent
#include "llfloaterpreference.h"

const F32 MAX_USER_FAR_CLIP = 512.f;
const F32 MIN_USER_FAR_CLIP = 64.f;

const S32 ASPECT_RATIO_STR_LEN = 100;

LLPanelDisplay::LLPanelDisplay()
{
	gUICtrlFactory->buildPanel(this, "panel_preferences_graphics1.xml");
}

BOOL LLPanelDisplay::postBuild()
{
	requires("windowed mode", WIDGET_TYPE_CHECKBOX);
	requires("fullscreen combo", WIDGET_TYPE_COMBO_BOX);
	requires("aspect_ratio", WIDGET_TYPE_COMBO_BOX);
	requires("aspect_auto_detect", WIDGET_TYPE_CHECKBOX);
	requires("AspectRatioLabel1", WIDGET_TYPE_TEXT_BOX);	
	requires("DisplayResLabel", WIDGET_TYPE_TEXT_BOX);
	requires("FullScreenInfo", WIDGET_TYPE_TEXT_EDITOR);

	requires("QualityPerformanceSelection", WIDGET_TYPE_SLIDER);
	requires("CustomSettings", WIDGET_TYPE_CHECKBOX);

	requires("GraphicsHardwareButton", WIDGET_TYPE_BUTTON);
	requires("Defaults", WIDGET_TYPE_BUTTON);
	
	requires("BumpShiny", WIDGET_TYPE_CHECKBOX);
	requires("BasicShaders", WIDGET_TYPE_CHECKBOX);
	requires("AvatarVertexProgram", WIDGET_TYPE_CHECKBOX);
	requires("WindLightUseAtmosShaders", WIDGET_TYPE_CHECKBOX);
	requires("Reflections", WIDGET_TYPE_CHECKBOX);
	
	requires("AvatarImpostors", WIDGET_TYPE_CHECKBOX);
	requires("AvatarCloth", WIDGET_TYPE_CHECKBOX);
	
	requires("DrawDistance", WIDGET_TYPE_SLIDER);
	requires("DrawDistanceMeterText1", WIDGET_TYPE_TEXT_BOX);
	requires("DrawDistanceMeterText2", WIDGET_TYPE_TEXT_BOX);

	requires("ObjectMeshDetail", WIDGET_TYPE_SLIDER);
	requires("FlexibleMeshDetail", WIDGET_TYPE_SLIDER);
	requires("TreeMeshDetail", WIDGET_TYPE_SLIDER);
	requires("AvatarMeshDetail", WIDGET_TYPE_SLIDER);
	requires("TerrainMeshDetail", WIDGET_TYPE_SLIDER);
	requires("SkyMeshDetail", WIDGET_TYPE_SLIDER);
	requires("MaxParticleCount", WIDGET_TYPE_SLIDER);
	requires("RenderPostProcess", WIDGET_TYPE_SLIDER);

	requires("ObjectMeshDetailText", WIDGET_TYPE_TEXT_BOX);
	requires("FlexibleMeshDetailText", WIDGET_TYPE_TEXT_BOX);
	requires("TreeMeshDetailText", WIDGET_TYPE_TEXT_BOX);
	requires("AvatarMeshDetailText", WIDGET_TYPE_TEXT_BOX);
	requires("TerrainMeshDetailText", WIDGET_TYPE_TEXT_BOX);
	requires("SkyMeshDetailText", WIDGET_TYPE_TEXT_BOX);
	requires("PostProcessText", WIDGET_TYPE_TEXT_BOX);

	requires("LightingDetailRadio", WIDGET_TYPE_RADIO_GROUP);
	requires("TerrainDetailRadio", WIDGET_TYPE_RADIO_GROUP);

	if (!checkRequirements())
	{
		return FALSE;
	}

	// return to default values
	childSetAction("Defaults", setHardwareDefaults, NULL);
	
	// Help button
	childSetAction("GraphicsPreferencesHelpButton", onOpenHelp, this);

	// Hardware settings button
	childSetAction("GraphicsHardwareButton", onOpenHardwareSettings, NULL);

	//============================================================================
	// Resolution
	
	// radio set for fullscreen size
	
	mCtrlWindowed = LLUICtrlFactory::getCheckBoxByName(this, "windowed mode");
	mCtrlWindowed->setCommitCallback(onCommitWindowedMode);
	mCtrlWindowed->setCallbackUserData(this);

	mAspectRatioLabel1 = LLUICtrlFactory::getTextBoxByName(this, "AspectRatioLabel1");
	mFullScreenInfo = LLUICtrlFactory::getTextEditorByName(this, "FullScreenInfo");
	mDisplayResLabel = LLUICtrlFactory::getTextBoxByName(this, "DisplayResLabel");

	S32 num_resolutions = 0;
	LLWindow::LLWindowResolution* supported_resolutions = gViewerWindow->getWindow()->getSupportedResolutions(num_resolutions);

	S32 fullscreen_mode = num_resolutions - 1;

	mCtrlFullScreen = LLUICtrlFactory::getComboBoxByName(this, "fullscreen combo");
	
	LLUIString resolution_label = getUIString("resolution_format");

	for (S32 i = 0; i < num_resolutions; i++)
	{
		resolution_label.setArg("[RES_X]", llformat("%d", supported_resolutions[i].mWidth));
		resolution_label.setArg("[RES_Y]", llformat("%d", supported_resolutions[i].mHeight));
		mCtrlFullScreen->add( resolution_label, ADD_BOTTOM );
	}

	{
		BOOL targetFullscreen;
		S32 targetWidth;
		S32 targetHeight;
		
		gViewerWindow->getTargetWindow(targetFullscreen, targetWidth, targetHeight);
		
		if (targetFullscreen)
		{
			fullscreen_mode = 0; // default to 800x600
			for (S32 i = 0; i < num_resolutions; i++)
			{
				if (targetWidth == supported_resolutions[i].mWidth
				&&  targetHeight == supported_resolutions[i].mHeight)
				{
					fullscreen_mode = i;
				}
			}
			mCtrlFullScreen->setCurrentByIndex(fullscreen_mode);
			mCtrlWindowed->set(FALSE);
			mCtrlFullScreen->setVisible(TRUE);
		}
		else
		{
			// set to windowed mode
			//fullscreen_mode = mCtrlFullScreen->getItemCount() - 1;
			mCtrlWindowed->set(TRUE);
			mCtrlFullScreen->setCurrentByIndex(0);
			mCtrlFullScreen->setVisible(FALSE);
		}
	}

	if (gSavedSettings.getBOOL("FullScreenAutoDetectAspectRatio"))
	{
		mAspectRatio = gViewerWindow->getDisplayAspectRatio();
	}
	else
	{
		mAspectRatio = gSavedSettings.getF32("FullScreenAspectRatio");
	}

	S32 numerator = 0;
	S32 denominator = 0;
	fractionFromDecimal(mAspectRatio, numerator, denominator);

	LLUIString aspect_ratio_text = getUIString("aspect_ratio_text");
	if (numerator != 0)
	{
		aspect_ratio_text.setArg("[NUM]", llformat("%d",  numerator));
		aspect_ratio_text.setArg("[DEN]", llformat("%d",  denominator));
	}
	else
	{
		aspect_ratio_text = llformat("%.3f", mAspectRatio);
	}

	mCtrlAspectRatio = LLUICtrlFactory::getComboBoxByName(this, "aspect_ratio");
	mCtrlAspectRatio->setTextEntryCallback(onKeystrokeAspectRatio);
	mCtrlAspectRatio->setCommitCallback(onSelectAspectRatio);
	mCtrlAspectRatio->setCallbackUserData(this);
	// add default aspect ratios
	mCtrlAspectRatio->add(aspect_ratio_text, &mAspectRatio, ADD_TOP);
	mCtrlAspectRatio->setCurrentByIndex(0);

	mCtrlAutoDetectAspect = LLUICtrlFactory::getCheckBoxByName(this, "aspect_auto_detect");
	mCtrlAutoDetectAspect->setCommitCallback(onCommitAutoDetectAspect);
	mCtrlAutoDetectAspect->setCallbackUserData(this);

	// radio performance box
	mCtrlSliderQuality = LLUICtrlFactory::getSliderByName(this, 
		"QualityPerformanceSelection");
	mCtrlSliderQuality->setSliderMouseUpCallback(onChangeQuality);
	mCtrlSliderQuality->setCallbackUserData(this);

	mCtrlCustomSettings = LLUICtrlFactory::getCheckBoxByName(this, "CustomSettings");
	mCtrlCustomSettings->setCommitCallback(onChangeCustom);
	mCtrlCustomSettings->setCallbackUserData(this);

	mGraphicsBorder = static_cast<LLViewBorder*>(getChildByName("GraphicsBorder"));
	llassert(mGraphicsBorder != NULL);

	//----------------------------------------------------------------------------
	// Enable Bump/Shiny
	mCtrlBumpShiny = LLUICtrlFactory::getCheckBoxByName(this, "BumpShiny");
	
	//----------------------------------------------------------------------------
	// Enable Reflections
	mCtrlReflections = LLUICtrlFactory::getCheckBoxByName(this, "Reflections");
	mCtrlReflections->setCommitCallback(&LLPanelDisplay::onVertexShaderEnable);
	mCtrlReflections->setCallbackUserData(this);
	mRadioReflectionDetail = LLUICtrlFactory::getRadioGroupByName(this, "ReflectionDetailRadio");
	
	// WindLight
	mCtrlWindLight = LLUICtrlFactory::getCheckBoxByName(this, "WindLightUseAtmosShaders");
	mCtrlWindLight->setCommitCallback(&LLPanelDisplay::onVertexShaderEnable);
	mCtrlWindLight->setCallbackUserData(this);

	//----------------------------------------------------------------------------
	// Enable Avatar Shaders
	mCtrlAvatarVP = LLUICtrlFactory::getCheckBoxByName(this, "AvatarVertexProgram");
	mCtrlAvatarVP->setCommitCallback(&LLPanelDisplay::onVertexShaderEnable);
	mCtrlAvatarVP->setCallbackUserData(this);

	//----------------------------------------------------------------------------
	// Avatar Render Mode
	mCtrlAvatarCloth = LLUICtrlFactory::getCheckBoxByName(this, "AvatarCloth");
	mCtrlAvatarImpostors = LLUICtrlFactory::getCheckBoxByName(this, "AvatarImpostors");

	//----------------------------------------------------------------------------
	// radio set for lighting detail
	mRadioLightingDetail2 = LLUICtrlFactory::getRadioGroupByName(this, "LightingDetailRadio");

	//----------------------------------------------------------------------------
	// radio set for terrain detail mode
	mRadioTerrainDetail = LLUICtrlFactory::getRadioGroupByName(this, "TerrainDetailRadio");

	//----------------------------------------------------------------------------
	// Global Shader Enable
	mCtrlShaderEnable = LLUICtrlFactory::getCheckBoxByName(this, "BasicShaders");
	mCtrlShaderEnable->setCommitCallback(&LLPanelDisplay::onVertexShaderEnable);
	mCtrlShaderEnable->setCallbackUserData(this);
	
	//============================================================================

	// Object detail slider
	mCtrlDrawDistance = LLUICtrlFactory::getSliderByName(this, "DrawDistance");
	mDrawDistanceMeterText1 = LLUICtrlFactory::getTextBoxByName(this, "DrawDistanceMeterText1");
	mDrawDistanceMeterText2 = LLUICtrlFactory::getTextBoxByName(this, "DrawDistanceMeterText2");
	mCtrlDrawDistance->setCommitCallback(&LLPanelDisplay::updateMeterText);
	mCtrlDrawDistance->setCallbackUserData(this);

	// Object detail slider
	mCtrlLODFactor = LLUICtrlFactory::getSliderByName(this, "ObjectMeshDetail");
	mLODFactorText = LLUICtrlFactory::getTextBoxByName(this, "ObjectMeshDetailText");
	mCtrlLODFactor->setCommitCallback(&LLPanelDisplay::updateSliderText);
	mCtrlLODFactor->setCallbackUserData(mLODFactorText);

	// Flex object detail slider
	mCtrlFlexFactor = LLUICtrlFactory::getSliderByName(this, "FlexibleMeshDetail");
	mFlexFactorText = LLUICtrlFactory::getTextBoxByName(this, "FlexibleMeshDetailText");
	mCtrlFlexFactor->setCommitCallback(&LLPanelDisplay::updateSliderText);
	mCtrlFlexFactor->setCallbackUserData(mFlexFactorText);

	// Tree detail slider
	mCtrlTreeFactor = LLUICtrlFactory::getSliderByName(this, "TreeMeshDetail");
	mTreeFactorText = LLUICtrlFactory::getTextBoxByName(this, "TreeMeshDetailText");
	mCtrlTreeFactor->setCommitCallback(&LLPanelDisplay::updateSliderText);
	mCtrlTreeFactor->setCallbackUserData(mTreeFactorText);

	// Avatar detail slider
	mCtrlAvatarFactor = LLUICtrlFactory::getSliderByName(this, "AvatarMeshDetail");
	mAvatarFactorText = LLUICtrlFactory::getTextBoxByName(this, "AvatarMeshDetailText");
	mCtrlAvatarFactor->setCommitCallback(&LLPanelDisplay::updateSliderText);
	mCtrlAvatarFactor->setCallbackUserData(mAvatarFactorText);

	// Terrain detail slider
	mCtrlTerrainFactor = LLUICtrlFactory::getSliderByName(this, "TerrainMeshDetail");
	mTerrainFactorText = LLUICtrlFactory::getTextBoxByName(this, "TerrainMeshDetailText");
	mCtrlTerrainFactor->setCommitCallback(&LLPanelDisplay::updateSliderText);
	mCtrlTerrainFactor->setCallbackUserData(mTerrainFactorText);

	// Terrain detail slider
	mCtrlSkyFactor = LLUICtrlFactory::getSliderByName(this, "SkyMeshDetail");
	mSkyFactorText = LLUICtrlFactory::getTextBoxByName(this, "SkyMeshDetailText");
	mCtrlSkyFactor->setCommitCallback(&LLPanelDisplay::updateSliderText);
	mCtrlSkyFactor->setCallbackUserData(mSkyFactorText);

	// Particle detail slider
	mCtrlMaxParticle = LLUICtrlFactory::getSliderByName(this, "MaxParticleCount");

	// Glow detail slider
	mCtrlPostProcess = LLUICtrlFactory::getSliderByName(this, "RenderPostProcess");
	mPostProcessText = LLUICtrlFactory::getTextBoxByName(this, "PostProcessText");
	mCtrlPostProcess->setCommitCallback(&LLPanelDisplay::updateSliderText);
	mCtrlPostProcess->setCallbackUserData(mPostProcessText);

	// Text boxes (for enabling/disabling)
	mShaderText = LLUICtrlFactory::getTextBoxByName(this, "ShadersText");
	mReflectionText = LLUICtrlFactory::getTextBoxByName(this, "ReflectionDetailText");
	mAvatarText = LLUICtrlFactory::getTextBoxByName(this, "AvatarRenderingText");
	mTerrainText = LLUICtrlFactory::getTextBoxByName(this, "TerrainDetailText");
	mLightingText = LLUICtrlFactory::getTextBoxByName(this, "LightingDetailText");
	mMeshDetailText = LLUICtrlFactory::getTextBoxByName(this, "MeshDetailText");

	refresh();

	return TRUE;
}


LLPanelDisplay::~LLPanelDisplay()
{
	// clean up user data
	for (S32 i = 0; i < mCtrlAspectRatio->getItemCount(); i++)
	{
		mCtrlAspectRatio->setCurrentByIndex(i);
	}
}

void LLPanelDisplay::refresh()
{
	LLPanel::refresh();
	
	mFSAutoDetectAspect = gSavedSettings.getBOOL("FullScreenAutoDetectAspectRatio");

	mQualityPerformance = gSavedSettings.getU32("RenderQualityPerformance");
	mCustomSettings = gSavedSettings.getBOOL("RenderCustomSettings");

	// shader settings
	mBumpShiny = gSavedSettings.getBOOL("RenderObjectBump");
	mShaderEnable = gSavedSettings.getBOOL("VertexShaderEnable");
	mWindLight = gSavedSettings.getBOOL("WindLightUseAtmosShaders");
	mReflections = gSavedSettings.getBOOL("RenderWaterReflections");
	mAvatarVP = gSavedSettings.getBOOL("RenderAvatarVP");

	// reflection radio
	mReflectionDetail = gSavedSettings.getS32("RenderReflectionDetail");

	// avatar settings
	mAvatarImpostors = gSavedSettings.getBOOL("RenderUseImpostors");
	mAvatarCloth = gSavedSettings.getBOOL("RenderAvatarCloth");

	// Draw distance
	mRenderFarClip = gSavedSettings.getF32("RenderFarClip");

	// sliders and their text boxes
	mPrimLOD = gSavedSettings.getF32("RenderVolumeLODFactor");
	mFlexLOD = gSavedSettings.getF32("RenderFlexTimeFactor");
	mTreeLOD = gSavedSettings.getF32("RenderTreeLODFactor");
	mAvatarLOD = gSavedSettings.getF32("RenderAvatarLODFactor");
	mTerrainLOD = gSavedSettings.getF32("RenderTerrainLODFactor");
	mSkyLOD = gSavedSettings.getU32("WLSkyDetail");
	mParticleCount = gSavedSettings.getS32("RenderMaxPartCount");
	mPostProcess = gSavedSettings.getS32("RenderGlowResolutionPow");
	
	// lighting and terrain radios
	mLightingDetail = gSavedSettings.getS32("RenderLightingDetail");
	mTerrainDetail =  gSavedSettings.getS32("RenderTerrainDetail");

	// slider text boxes
	updateSliderText(mCtrlLODFactor, mLODFactorText);
	updateSliderText(mCtrlFlexFactor, mFlexFactorText);
	updateSliderText(mCtrlTreeFactor, mTreeFactorText);
	updateSliderText(mCtrlAvatarFactor, mAvatarFactorText);
	updateSliderText(mCtrlTerrainFactor, mTerrainFactorText);
	updateSliderText(mCtrlPostProcess, mPostProcessText);
	updateSliderText(mCtrlSkyFactor, mSkyFactorText);

	refreshEnabledState();
}

void LLPanelDisplay::refreshEnabledState()
{
	// if in windowed mode, disable full screen options
	bool isFullScreen = !mCtrlWindowed->get();

	mDisplayResLabel->setVisible(isFullScreen);
	mCtrlFullScreen->setVisible(isFullScreen);
	mCtrlAspectRatio->setVisible(isFullScreen);
	mAspectRatioLabel1->setVisible(isFullScreen);
	mCtrlAutoDetectAspect->setVisible(isFullScreen);
	mFullScreenInfo->setVisible(!isFullScreen);

	// disable graphics settings and exit if it's not set to custom
	if(!gSavedSettings.getBOOL("RenderCustomSettings"))
	{
		setHiddenGraphicsState(true);
		return;
	}

	// otherwise turn them all on and selectively turn off others
	else
	{
		setHiddenGraphicsState(false);
	}

	// Reflections
	BOOL reflections = gSavedSettings.getBOOL("VertexShaderEnable") 
		&& gGLManager.mHasCubeMap 
		&& gFeatureManagerp->isFeatureAvailable("RenderCubeMap");
	mCtrlReflections->setEnabled(reflections);
	
	// Bump & Shiny
	bool bumpshiny = gGLManager.mHasCubeMap && gFeatureManagerp->isFeatureAvailable("RenderCubeMap") && gFeatureManagerp->isFeatureAvailable("RenderObjectBump");
	mCtrlBumpShiny->setEnabled(bumpshiny ? TRUE : FALSE);
	
	for (S32 i = 0; i < mRadioReflectionDetail->getItemCount(); ++i)
	{
		mRadioReflectionDetail->setIndexEnabled(i, mCtrlReflections->get() && reflections);
	}

	// Avatar Mode
	S32 max_avatar_shader = LLShaderMgr::sMaxAvatarShaderLevel;
	mCtrlAvatarVP->setEnabled((max_avatar_shader > 0) ? TRUE : FALSE);
	
	if (gSavedSettings.getBOOL("VertexShaderEnable") == FALSE || 
		gSavedSettings.getBOOL("RenderAvatarVP") == FALSE)
	{
		mCtrlAvatarCloth->setEnabled(false);
	} 
	else
	{
		mCtrlAvatarCloth->setEnabled(true);
	}

	// Vertex Shaders
	mCtrlShaderEnable->setEnabled(gFeatureManagerp->isFeatureAvailable("VertexShaderEnable"));

	BOOL shaders = mCtrlShaderEnable->get();
	if (shaders)
	{
		mRadioTerrainDetail->setValue(1);
		mRadioTerrainDetail->setEnabled(FALSE);
	}
	else
	{
		mRadioTerrainDetail->setEnabled(TRUE);
	}

	// *HACK just checks to see if we can use shaders... 
	// maybe some cards that use shaders, but don't support windlight
	mCtrlWindLight->setEnabled(mCtrlShaderEnable->getEnabled() && shaders);

	// turn off sky detail if atmostpherics isn't on
	mCtrlSkyFactor->setEnabled(gSavedSettings.getBOOL("WindLightUseAtmosShaders"));
	mSkyFactorText->setEnabled(gSavedSettings.getBOOL("WindLightUseAtmosShaders"));

	// now turn off any features that are unavailable
	disableUnavailableSettings();
}

void LLPanelDisplay::disableUnavailableSettings()
{	
	// if vertex shaders off, disable all shader related products
	if(!gFeatureManagerp->isFeatureAvailable("VertexShaderEnable"))
	{
		mCtrlShaderEnable->setEnabled(FALSE);
		mCtrlShaderEnable->setValue(FALSE);

		mCtrlWindLight->setEnabled(FALSE);
		mCtrlWindLight->setValue(FALSE);

		mCtrlReflections->setEnabled(FALSE);
		mCtrlReflections->setValue(FALSE);

		mCtrlAvatarVP->setEnabled(FALSE);
		mCtrlAvatarVP->setValue(FALSE);

		mCtrlAvatarCloth->setEnabled(FALSE);
		mCtrlAvatarCloth->setValue(FALSE);
	}

	// disabled windlight
	if(!gFeatureManagerp->isFeatureAvailable("WindLightUseAtmosShaders"))
	{
		mCtrlWindLight->setEnabled(FALSE);
		mCtrlWindLight->setValue(FALSE);
	}

	// disabled reflections
	if(!gFeatureManagerp->isFeatureAvailable("RenderWaterReflections"))
	{
		mCtrlReflections->setEnabled(FALSE);
		mCtrlReflections->setValue(FALSE);
	}

	// disabled av
	if(!gFeatureManagerp->isFeatureAvailable("RenderAvatarVP"))
	{
		mCtrlAvatarVP->setEnabled(FALSE);
		mCtrlAvatarVP->setValue(FALSE);

		mCtrlAvatarCloth->setEnabled(FALSE);
		mCtrlAvatarCloth->setValue(FALSE);
	}
	// disabled cloth
	if(!gFeatureManagerp->isFeatureAvailable("RenderAvatarCloth"))
	{
		mCtrlAvatarCloth->setEnabled(FALSE);
		mCtrlAvatarCloth->setValue(FALSE);
	}
	// disabled impostors
	if(!gFeatureManagerp->isFeatureAvailable("RenderUseImpostors"))
	{
		mCtrlAvatarImpostors->setEnabled(FALSE);
		mCtrlAvatarImpostors->setValue(FALSE);
	}
}

void LLPanelDisplay::setHiddenGraphicsState(bool isHidden)
{
	// quick check
	llassert(mGraphicsBorder != NULL);

	llassert(mCtrlDrawDistance != NULL);
	llassert(mCtrlLODFactor != NULL);
	llassert(mCtrlFlexFactor != NULL);
	llassert(mCtrlTreeFactor != NULL);
	llassert(mCtrlAvatarFactor != NULL);
	llassert(mCtrlTerrainFactor != NULL);
	llassert(mCtrlSkyFactor != NULL);
	llassert(mCtrlMaxParticle != NULL);
	llassert(mCtrlPostProcess != NULL);

	llassert(mLODFactorText != NULL);
	llassert(mFlexFactorText != NULL);
	llassert(mTreeFactorText != NULL);
	llassert(mAvatarFactorText != NULL);
	llassert(mTerrainFactorText != NULL);
	llassert(mSkyFactorText != NULL);
	llassert(mPostProcessText != NULL);

	llassert(mCtrlBumpShiny != NULL);
	llassert(mCtrlReflections != NULL);
	llassert(mCtrlWindLight != NULL);
	llassert(mCtrlAvatarVP != NULL);
	llassert(mCtrlShaderEnable != NULL);
	llassert(mCtrlAvatarImpostors != NULL);
	llassert(mCtrlAvatarCloth != NULL);
	llassert(mRadioLightingDetail2 != NULL);

	llassert(mRadioTerrainDetail != NULL);
	llassert(mRadioReflectionDetail != NULL);

	llassert(mMeshDetailText != NULL);
	llassert(mShaderText != NULL);
	llassert(mReflectionText != NULL);
	llassert(mAvatarText != NULL);
	llassert(mLightingText != NULL);
	llassert(mTerrainText != NULL);
	llassert(mDrawDistanceMeterText1 != NULL);
	llassert(mDrawDistanceMeterText2 != NULL);

	// enable/disable the states
	mGraphicsBorder->setVisible(!isHidden);
	/* 
	LLColor4 light(.45098f, .51765f, .6078f, 1.0f);
	LLColor4 dark(.10196f, .10196f, .10196f, 1.0f);
	b ? mGraphicsBorder->setColors(dark, light) : mGraphicsBorder->setColors(dark, dark);
	*/

	mCtrlDrawDistance->setVisible(!isHidden);
	mCtrlLODFactor->setVisible(!isHidden);	
	mCtrlFlexFactor->setVisible(!isHidden);	
	mCtrlTreeFactor->setVisible(!isHidden);	
	mCtrlAvatarFactor->setVisible(!isHidden);	
	mCtrlTerrainFactor->setVisible(!isHidden);
	mCtrlSkyFactor->setVisible(!isHidden);
	mCtrlMaxParticle->setVisible(!isHidden);
	mCtrlPostProcess->setVisible(!isHidden);

	mLODFactorText->setVisible(!isHidden);	
	mFlexFactorText->setVisible(!isHidden);	
	mTreeFactorText->setVisible(!isHidden);	
	mAvatarFactorText->setVisible(!isHidden);	
	mTerrainFactorText->setVisible(!isHidden);
	mSkyFactorText->setVisible(!isHidden);
	mPostProcessText->setVisible(!isHidden);

	mCtrlBumpShiny->setVisible(!isHidden);
	mCtrlReflections->setVisible(!isHidden);
	mCtrlWindLight->setVisible(!isHidden);
	mCtrlAvatarVP->setVisible(!isHidden);
	mCtrlShaderEnable->setVisible(!isHidden);
	mCtrlAvatarImpostors->setVisible(!isHidden);
	mCtrlAvatarCloth->setVisible(!isHidden);
	mRadioLightingDetail2->setVisible(!isHidden);

	mRadioTerrainDetail->setVisible(!isHidden);
	mRadioReflectionDetail->setVisible(!isHidden);

	// text boxes
	mShaderText->setVisible(!isHidden);
	mReflectionText->setVisible(!isHidden);
	mAvatarText->setVisible(!isHidden);
	mLightingText->setVisible(!isHidden);
	mTerrainText->setVisible(!isHidden);
	mDrawDistanceMeterText1->setVisible(!isHidden);
	mDrawDistanceMeterText2->setVisible(!isHidden);

	// hide one meter text if we're making things visible
	if(!isHidden)
	{
		updateMeterText(mCtrlDrawDistance, this);
	}

	mMeshDetailText->setVisible(!isHidden);
}

void LLPanelDisplay::cancel()
{
	gSavedSettings.setBOOL("FullScreenAutoDetectAspectRatio", mFSAutoDetectAspect);
	gSavedSettings.setF32("FullScreenAspectRatio", mAspectRatio);

	gSavedSettings.setU32("RenderQualityPerformance", mQualityPerformance);

	gSavedSettings.setBOOL("RenderCustomSettings", mCustomSettings);

	gSavedSettings.setBOOL("RenderObjectBump", mBumpShiny);
	gSavedSettings.setBOOL("VertexShaderEnable", mShaderEnable);
	gSavedSettings.setBOOL("WindLightUseAtmosShaders", mWindLight);
	gSavedSettings.setBOOL("RenderWaterReflections", mReflections);
	gSavedSettings.setBOOL("RenderAvatarVP", mAvatarVP);

	gSavedSettings.setS32("RenderReflectionDetail", mReflectionDetail);

	gSavedSettings.setBOOL("RenderUseImpostors", mAvatarImpostors);
	gSavedSettings.setBOOL("RenderAvatarCloth", mAvatarCloth);

	gSavedSettings.setS32("RenderLightingDetail", mLightingDetail);
	gSavedSettings.setS32("RenderTerrainDetail", mTerrainDetail);

	gSavedSettings.setF32("RenderFarClip", mRenderFarClip);
	gSavedSettings.setF32("RenderVolumeLODFactor", mPrimLOD);
	gSavedSettings.setF32("RenderFlexTimeFactor", mFlexLOD);
	gSavedSettings.setF32("RenderTreeLODFactor", mTreeLOD);
	gSavedSettings.setF32("RenderAvatarLODFactor", mAvatarLOD);
	gSavedSettings.setF32("RenderTerrainLODFactor", mTerrainLOD);
	gSavedSettings.setU32("WLSkyDetail", mSkyLOD);
	gSavedSettings.setS32("RenderMaxPartCount", mParticleCount);
	gSavedSettings.setS32("RenderGlowResolutionPow", mPostProcess);
}

void LLPanelDisplay::apply()
{
	applyResolution();
}

void LLPanelDisplay::onChangeQuality(LLUICtrl *ctrl, void *data)
{
	LLSliderCtrl* sldr = static_cast<LLSliderCtrl*>(ctrl);
	LLPanelDisplay* cur_panel = static_cast<LLPanelDisplay*>(data);

	if(sldr == NULL || cur_panel == NULL)
	{
		return;
	}

	U32 set = (U32)sldr->getValueF32();
	gFeatureManagerp->setGraphicsLevel(set, true);
	
	LLFloaterPreference::refreshEnabledGraphics();
	cur_panel->refresh();
}

void LLPanelDisplay::onChangeCustom(LLUICtrl *ctrl, void *data)
{
	LLFloaterPreference::refreshEnabledGraphics();
}

void LLPanelDisplay::onOpenHelp(void* user_data)
{
	LLPanelDisplay* self = static_cast<LLPanelDisplay*>(user_data);

	const char* xml_alert = "GraphicsPreferencesHelp";
	LLAlertDialog* dialogp = gViewerWindow->alertXml(xml_alert);
	if (dialogp)
	{
		LLFloater* root_floater = gFloaterView->getParentFloater(self);
		if (root_floater)
		{
			root_floater->addDependentFloater(dialogp);
		}
	}	
}

void LLPanelDisplay::onOpenHardwareSettings(void* user_data)
{
	LLFloaterHardwareSettings::show();
}

void LLPanelDisplay::onApplyResolution(LLUICtrl* src, void* user_data)
{
	((LLPanelDisplay*) src)->applyResolution();
}

void LLPanelDisplay::applyResolution()
{
	char aspect_ratio_text[ASPECT_RATIO_STR_LEN];		/*Flawfinder: ignore*/
	if (mCtrlAspectRatio->getCurrentIndex() == -1)
	{
		strncpy(aspect_ratio_text, mCtrlAspectRatio->getSimple().c_str(), sizeof(aspect_ratio_text) -1);	/*Flawfinder: ignore*/
                aspect_ratio_text[sizeof(aspect_ratio_text) -1] = '\0';
		char *element = strtok(aspect_ratio_text, ":/\\");
		if (!element)
		{
			mAspectRatio = 0.f; // will be clamped later
		}
		else
		{
			LLLocale locale(LLLocale::USER_LOCALE);
			mAspectRatio = (F32)atof(element);
		}

		// look for denominator
		element = strtok(NULL, ":/\\");
		if (element)
		{
			LLLocale locale(LLLocale::USER_LOCALE);

			F32 denominator = (F32)atof(element);
			if (denominator != 0.f)
			{
				mAspectRatio /= denominator;
			}
		}
	}
	else
	{
		mAspectRatio = (F32)mCtrlAspectRatio->getValue().asReal();
	}

	// presumably, user entered a non-numeric value if aspect_ratio == 0.f
	if (mAspectRatio != 0.f)
	{
		mAspectRatio = llclamp(mAspectRatio, 0.2f, 5.f);
		gSavedSettings.setF32("FullScreenAspectRatio", mAspectRatio);
		if (gSavedSettings.getBOOL("FullScreenAutoDetectAspectRatio"))
		{
			gViewerWindow->getWindow()->setNativeAspectRatio(0.f);
		}
		else
		{
			gViewerWindow->getWindow()->setNativeAspectRatio(mAspectRatio);
		}
	}
	gViewerWindow->reshape(gViewerWindow->getWindowDisplayWidth(), gViewerWindow->getWindowDisplayHeight());

	// Screen resolution
	S32 num_resolutions;
	LLWindow::LLWindowResolution* supported_resolutions = gViewerWindow->getWindow()->getSupportedResolutions(num_resolutions);

	// switching to windowed
	BOOL fullscreen = !mCtrlWindowed->get();

	// check if resolution has changed
	BOOL targetFullscreen;
	S32 targetWidth;
	S32 targetHeight;
	
	gViewerWindow->getTargetWindow(targetFullscreen, targetWidth, targetHeight);

	if ((fullscreen != targetFullscreen) ||
		(fullscreen &&
		 (supported_resolutions[mCtrlFullScreen->getCurrentIndex()].mWidth != targetWidth ||
		  supported_resolutions[mCtrlFullScreen->getCurrentIndex()].mHeight != targetHeight)
		 ))
	{
		// change fullscreen resolution or switch in/out of windowed mode
		BOOL result;

		BOOL logged_in = (LLStartUp::getStartupState() >= STATE_STARTED);
		if (fullscreen)
		{
			result = gViewerWindow->changeDisplaySettings(TRUE, 
				LLCoordScreen(	supported_resolutions[mCtrlFullScreen->getCurrentIndex()].mWidth, 
								supported_resolutions[mCtrlFullScreen->getCurrentIndex()].mHeight), 
				gSavedSettings.getBOOL("DisableVerticalSync"),
				logged_in);
		}
		else
		{
			result = gViewerWindow->changeDisplaySettings(FALSE, 
				LLCoordScreen(gSavedSettings.getS32("WindowWidth"), gSavedSettings.getS32("WindowHeight")), 
				TRUE,
				logged_in);
		}
		if (!result)
		{

			// GL is non-existent at this point, so we can't continue.
			llerrs << "LLPanelDisplay::apply() failed" << llendl;
		}
	}

	// force aspect ratio
	if (fullscreen && gCamera)
	{
		gCamera->setAspect( gViewerWindow->getDisplayAspectRatio() );
	}

	if (gWorldPointer)
	{
		send_agent_update(TRUE);
	}

	// Update enable/disable
	refresh();
}

//static 
void LLPanelDisplay::onCommitWindowedMode(LLUICtrl* ctrl, void *data)
{
	LLPanelDisplay *panel = (LLPanelDisplay*)data;

	panel->refresh();
}

//static
void LLPanelDisplay::onCommitAutoDetectAspect(LLUICtrl *ctrl, void *data)
{
	LLPanelDisplay *panel = (LLPanelDisplay*)data;

	BOOL auto_detect = ((LLCheckBoxCtrl*)ctrl)->get();
	F32 ratio;

	if (auto_detect)
	{
		S32 numerator = 0;
		S32 denominator = 0;
		
		// clear any aspect ratio override
		gViewerWindow->mWindow->setNativeAspectRatio(0.f);
		fractionFromDecimal(gViewerWindow->mWindow->getNativeAspectRatio(), numerator, denominator);

		LLString aspect;
		if (numerator != 0)
		{
			aspect = llformat("%d:%d", numerator, denominator);
		}
		else
		{
			aspect = llformat("%.3f", gViewerWindow->mWindow->getNativeAspectRatio());
		}

		panel->mCtrlAspectRatio->setLabel(aspect);

		ratio = gViewerWindow->mWindow->getNativeAspectRatio();
		gSavedSettings.setF32("FullScreenAspectRatio", ratio);
	}
}

//static 
void LLPanelDisplay::onKeystrokeAspectRatio(LLLineEditor* caller, void* user_data)
{
	LLPanelDisplay* panel = (LLPanelDisplay*)user_data;

	panel->mCtrlAutoDetectAspect->set(FALSE);
}

//static
void LLPanelDisplay::onSelectAspectRatio(LLUICtrl*, void* user_data)
{
	LLPanelDisplay* panel = (LLPanelDisplay*)user_data;

	panel->mCtrlAutoDetectAspect->set(FALSE);
}

//static
void LLPanelDisplay::fractionFromDecimal(F32 decimal_val, S32& numerator, S32& denominator)
{
	numerator = 0;
	denominator = 0;
	for (F32 test_denominator = 1.f; test_denominator < 30.f; test_denominator += 1.f)
	{
		if (fmodf((decimal_val * test_denominator) + 0.01f, 1.f) < 0.02f)
		{
			numerator = llround(decimal_val * test_denominator);
			denominator = llround(test_denominator);
			break;
		}
	}
}

//static
void LLPanelDisplay::onVertexShaderEnable(LLUICtrl* self, void* data)
{
	LLFloaterPreference::refreshEnabledGraphics();
}

void LLPanelDisplay::setHardwareDefaults(void* user_data)
{
	if (gFeatureManagerp)
	{
		gFeatureManagerp->applyRecommendedSettings();
		LLFloaterPreference::refreshEnabledGraphics();
	}
}

void LLPanelDisplay::updateSliderText(LLUICtrl* ctrl, void* user_data)
{
	// get our UI widgets
	LLTextBox* text_box = (LLTextBox*)user_data;
	LLSliderCtrl* slider = (LLSliderCtrl*) ctrl;
	if(text_box == NULL || slider == NULL)
	{
		return;
	}

	// get range and points when text should change
	F32 range = slider->getMaxValue() - slider->getMinValue();
	llassert(range > 0);
	F32 midPoint = slider->getMinValue() + range / 3.0f;
	F32 highPoint = slider->getMinValue() + (2.0f * range / 3.0f);

	// choose the right text
	if(slider->getValueF32() < midPoint)
	{
		text_box->setText(LLString("Low"));
	} 
	else if (slider->getValueF32() < highPoint)
	{
		text_box->setText(LLString("Mid"));
	}
	else
	{
		text_box->setText(LLString("High"));
	}
}

void LLPanelDisplay::updateMeterText(LLUICtrl* ctrl, void* user_data)
{
	// get our UI widgets
	LLPanelDisplay* panel = (LLPanelDisplay*)user_data;
	LLSliderCtrl* slider = (LLSliderCtrl*) ctrl;

	LLTextBox* m1 = LLUICtrlFactory::getTextBoxByName(panel, "DrawDistanceMeterText1");
	LLTextBox* m2 = LLUICtrlFactory::getTextBoxByName(panel, "DrawDistanceMeterText2");
	if(m1 == NULL || m2 == NULL || slider == NULL)
	{
		return;
	}

	// toggle the two text boxes based on whether we have 1 or two digits
	F32 val = slider->getValueF32();
	bool two_digits = val < 100;
	m1->setVisible(two_digits);
	m2->setVisible(!two_digits);
}



