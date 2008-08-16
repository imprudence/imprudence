/** 
 * @file llpaneldisplay.cpp
 * @brief Display preferences for the preferences floater
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
#include "lldrawpoolterrain.h"
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

//RN temporary includes for resolution switching
#include "llglheaders.h"
#include "viewer.h"
#include "llsky.h"

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
	requires("resolution_format", WIDGET_TYPE_TEXT_BOX);
	requires("aspect_ratio_text", WIDGET_TYPE_TEXT_BOX);
	requires("aspect_ratio", WIDGET_TYPE_COMBO_BOX);
	requires("aspect_auto_detect", WIDGET_TYPE_CHECKBOX);
	requires("UI Scale", WIDGET_TYPE_SLIDER);
	requires("ui_auto_scale", WIDGET_TYPE_CHECKBOX);
	requires("draw_distance", WIDGET_TYPE_SPINNER);
	requires("avfp", WIDGET_TYPE_CHECKBOX);

	if (!checkRequirements())
	{
		return FALSE;
	}

	//============================================================================
	// Resolution
	
	// radio set for fullscreen size
	
	mCtrlWindowed = LLUICtrlFactory::getCheckBoxByName(this, "windowed mode");
	mCtrlWindowed->setCommitCallback(onCommitWindowedMode);
	mCtrlWindowed->setCallbackUserData(this);

	S32 num_resolutions = 0;
	LLWindow::LLWindowResolution* supported_resolutions = gViewerWindow->getWindow()->getSupportedResolutions(num_resolutions);

	S32 fullscreen_mode = num_resolutions - 1;

	mCtrlFullScreen = LLUICtrlFactory::getComboBoxByName(this, "fullscreen combo");
	
	LLUIString resolution_label = childGetText("resolution_format");

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
			mCtrlFullScreen->setEnabled(TRUE);
		}
		else
		{
			// set to windowed mode
			//fullscreen_mode = mCtrlFullScreen->getItemCount() - 1;
			mCtrlWindowed->set(TRUE);
			mCtrlFullScreen->setCurrentByIndex(0);
			mCtrlFullScreen->setEnabled(FALSE);
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

	LLUIString aspect_ratio_text = childGetText("aspect_ratio_text");
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

	mUIScaleFactor = gSavedSettings.getF32("UIScaleFactor");
	mUIAutoScale = gSavedSettings.getBOOL("UIAutoScale");
	
	// First Person Visibility
	mFirstPersonAvatarVisible = gSavedSettings.getBOOL("FirstPersonAvatarVisible");

	// Draw distance
	mRenderFarClip = gSavedSettings.getF32("RenderFarClip");
	
}

void LLPanelDisplay::cancel()
{
	gSavedSettings.setBOOL("FullScreenAutoDetectAspectRatio", mFSAutoDetectAspect);
	gSavedSettings.setF32("UIScaleFactor", mUIScaleFactor);
	gSavedSettings.setBOOL("UIAutoScale", mUIAutoScale);
	gSavedSettings.setBOOL("FirstPersonAvatarVisible", mFirstPersonAvatarVisible);
	gSavedSettings.setF32("RenderFarClip", mRenderFarClip);
}

void LLPanelDisplay::apply()
{
	applyResolution();
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

		BOOL logged_in = (gStartupState >= STATE_STARTED);
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

	BOOL windowed_mode = ((LLCheckBoxCtrl*)ctrl)->get();

	if (windowed_mode)
	{
		panel->mCtrlFullScreen->setEnabled(FALSE);
	}
	else
	{
		panel->mCtrlFullScreen->setEnabled(TRUE);
	}
}

//static
void LLPanelDisplay::onCommitAutoDetectAspect(LLUICtrl *ctrl, void *data)
{
	LLPanelDisplay *panel = (LLPanelDisplay*)data;

	BOOL auto_detect = ((LLCheckBoxCtrl*)ctrl)->get();

	if (auto_detect)
	{
		char aspect[100];		/*Flawfinder: ignore*/
		S32 numerator = 0;
		S32 denominator = 0;
		// clear any aspect ratio override
		gViewerWindow->mWindow->setNativeAspectRatio(0.f);
		fractionFromDecimal(gViewerWindow->mWindow->getNativeAspectRatio(), numerator, denominator);

		if (numerator != 0)
		{
			snprintf(aspect, sizeof(aspect), "%d:%d", numerator, denominator); 			/* Flawfinder: ignore */
		}
		else
		{
			snprintf(aspect, sizeof(aspect), "%.3f", gViewerWindow->mWindow->getNativeAspectRatio());			/* Flawfinder: ignore */
		}

		panel->mCtrlAspectRatio->setLabel(aspect);
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

//============================================================================

LLPanelDisplay2::LLPanelDisplay2()
{
	gUICtrlFactory->buildPanel(this, "panel_preferences_graphics3.xml");
}

BOOL LLPanelDisplay2::postBuild()
{
	requires("ani", WIDGET_TYPE_CHECKBOX);
	requires("gamma", WIDGET_TYPE_SPINNER);
	requires("vbo", WIDGET_TYPE_CHECKBOX);
	requires("video card memory radio", WIDGET_TYPE_RADIO_GROUP);
	requires("fog", WIDGET_TYPE_SPINNER);
	requires("particles", WIDGET_TYPE_SPINNER);
	requires("comp limit", WIDGET_TYPE_SPINNER);
	requires("debug beacon line width", WIDGET_TYPE_SPINNER);

	if (!checkRequirements())
	{
		return FALSE;
	}

	// Graphics Card Memory
	mRadioVideoCardMem = LLUICtrlFactory::getRadioGroupByName(this, "video card memory radio");

#if !LL_WINDOWS
	// The probe_hardware_checkbox setting is only used in the Windows build
	// (It apparently controls a time-consuming DX9 hardware probe.)
	// Disable the checkbox everywhere else
	gSavedSettings.setBOOL("ProbeHardwareOnStartup", FALSE );
	childSetEnabled("probe_hardware_checkbox", false);
#endif // !LL_WINDOWS

	refresh();

	return TRUE;
}


LLPanelDisplay2::~LLPanelDisplay2()
{
	// Children all cleaned up by default view destructor.
}

void LLPanelDisplay2::refresh()
{
	LLPanel::refresh();

	mUseVBO = gSavedSettings.getBOOL("RenderVBOEnable");
	mUseAniso = gSavedSettings.getBOOL("RenderAnisotropic");
	mGamma = gSavedSettings.getF32("RenderGamma");
	mBrightness = gSavedSettings.getF32("RenderNightBrightness");
	mVideoCardMem = gSavedSettings.getS32("GraphicsCardMemorySetting");
	mFogRatio = gSavedSettings.getF32("RenderFogRatio");
	mParticleCount = gSavedSettings.getS32("RenderMaxPartCount");
	mCompositeLimit = gSavedSettings.getS32("AvatarCompositeLimit");
	mDebugBeaconLineWidth = gSavedSettings.getS32("DebugBeaconLineWidth");
	mProbeHardwareOnStartup = gSavedSettings.getBOOL("ProbeHardwareOnStartup");

	refreshEnabledState();
}

void LLPanelDisplay2::refreshEnabledState()
{
	S32 max_setting = LLViewerImageList::getMaxVideoRamSetting();
	max_setting = llclamp(max_setting, 0, 5);
	for (S32 i=max_setting+1; i < mRadioVideoCardMem->getItemCount(); i++)
	{
		mRadioVideoCardMem->getRadioButton(i)->setEnabled(FALSE);
	}

	if (!gFeatureManagerp->isFeatureAvailable("RenderVBO") ||
		!gGLManager.mHasVertexBufferObject)
	{
		childSetEnabled("vbo", FALSE);
	}
}

void LLPanelDisplay2::apply()
{
	// Anisotropic rendering
	BOOL old_anisotropic = LLImageGL::sGlobalUseAnisotropic;
	LLImageGL::sGlobalUseAnisotropic = childGetValue("ani");
	if (old_anisotropic != LLImageGL::sGlobalUseAnisotropic)
	{
		BOOL logged_in = (gStartupState >= STATE_STARTED);
		gViewerWindow->restartDisplay(logged_in);
	}

	refresh();
}


void LLPanelDisplay2::cancel()
{
	gSavedSettings.setBOOL("RenderVBOEnable", mUseVBO);
	gSavedSettings.setBOOL("RenderAnisotropic", mUseAniso);
	gSavedSettings.setF32("RenderGamma", mGamma);
	gSavedSettings.setF32("RenderNightBrightness", mBrightness);
	gSavedSettings.setS32("GraphicsCardMemorySetting", mVideoCardMem);
	gSavedSettings.setF32("RenderFogRatio", mFogRatio);
	gSavedSettings.setS32("RenderMaxPartCount", mParticleCount);
	gSavedSettings.setS32("AvatarCompositeLimit", mCompositeLimit);
	gSavedSettings.setS32("DebugBeaconLineWidth", mDebugBeaconLineWidth);
	gSavedSettings.setBOOL("ProbeHardwareOnStartup", mProbeHardwareOnStartup );
}

//============================================================================

LLPanelDisplay3::LLPanelDisplay3()
{
	gUICtrlFactory->buildPanel(this, "panel_preferences_graphics2.xml");
}

BOOL LLPanelDisplay3::postBuild()
{
	requires("bumpshiny", WIDGET_TYPE_CHECKBOX);
	requires("ripple", WIDGET_TYPE_CHECKBOX);
	requires("avatarvp", WIDGET_TYPE_CHECKBOX);
	requires("shaders", WIDGET_TYPE_CHECKBOX);
	
	requires("Avatar Appearance", WIDGET_TYPE_RADIO_GROUP);
	requires("lighting detail radio", WIDGET_TYPE_RADIO_GROUP);
	requires("terrain detail radio", WIDGET_TYPE_RADIO_GROUP);

	requires("Prim LOD Factor", WIDGET_TYPE_SLIDER_BAR);
	requires("Flex Factor", WIDGET_TYPE_SLIDER_BAR);
	requires("Tree LOD Factor", WIDGET_TYPE_SLIDER_BAR);
	requires("Avatar LOD Factor", WIDGET_TYPE_SLIDER_BAR);
	
	if (!checkRequirements())
	{
		return FALSE;
	}

	//----------------------------------------------------------------------------
	// Enable Bump/Shiny
	mCtrlBumpShiny = LLUICtrlFactory::getCheckBoxByName(this, "bumpshiny");
	
	//----------------------------------------------------------------------------
	// Enable Ripple Water
	mCtrlRippleWater = LLUICtrlFactory::getCheckBoxByName(this, "ripple");
	
	//----------------------------------------------------------------------------
	// Enable Avatar Shaders
	mCtrlAvatarVP = LLUICtrlFactory::getCheckBoxByName(this, "avatarvp");
	mCtrlAvatarVP->setCommitCallback(&LLPanelDisplay3::onVertexShaderEnable);
	mCtrlAvatarVP->setCallbackUserData(this);

	//----------------------------------------------------------------------------
	// Avatar Render Mode
	mCtrlAvatarMode = LLUICtrlFactory::getRadioGroupByName(this, "Avatar Appearance");
	
	//----------------------------------------------------------------------------
	// radio set for lighting detail
	mRadioLightingDetail2 = LLUICtrlFactory::getRadioGroupByName(this, "lighting detail radio");

	//----------------------------------------------------------------------------
	// radio set for terrain detail mode
	mRadioTerrainDetail = LLUICtrlFactory::getRadioGroupByName(this, "terrain detail radio");

	//----------------------------------------------------------------------------
	// Global Shader Enable
	mCtrlShaderEnable = LLUICtrlFactory::getCheckBoxByName(this, "shaders");
	mCtrlShaderEnable->setCommitCallback(&LLPanelDisplay3::onVertexShaderEnable);
	mCtrlShaderEnable->setCallbackUserData(this);

	
	//============================================================================

	// Object detail slider
	mCtrlLODFactor = LLUICtrlFactory::getSliderBarByName(this, "Prim LOD Factor");
	
	// Flex object detail slider
	mCtrlFlexFactor = LLUICtrlFactory::getSliderBarByName(this, "Flex Factor");
	
	// Tree detail slider
	mCtrlTreeFactor = LLUICtrlFactory::getSliderBarByName(this, "Tree LOD Factor");
	
	// Avatar detail slider
	mCtrlAvatarFactor = LLUICtrlFactory::getSliderBarByName(this, "Avatar LOD Factor");
	
	refresh();
	
	return TRUE;
}


LLPanelDisplay3::~LLPanelDisplay3()
{
}

void LLPanelDisplay3::refresh()
{
	LLPanel::refresh();
	
	mBumpShiny = gSavedSettings.getBOOL("RenderObjectBump");
	mRippleWater = gSavedSettings.getBOOL("RenderRippleWater");
	mAvatarVP = gSavedSettings.getBOOL("RenderAvatarVP");
	mShaderEnable = gSavedSettings.getBOOL("VertexShaderEnable");
	mAvatarMode = gSavedSettings.getS32("RenderAvatarMode");
	mLightingDetail = gSavedSettings.getS32("RenderLightingDetail");
	mTerrainDetail =  gSavedSettings.getS32("RenderTerrainDetail");
	mPrimLOD = gSavedSettings.getF32("RenderVolumeLODFactor");
	mFlexLOD = gSavedSettings.getF32("RenderFlexTimeFactor");
	mTreeLOD = gSavedSettings.getF32("RenderTreeLODFactor");
	mAvatarLOD = gSavedSettings.getF32("RenderAvatarLODFactor");

	refreshEnabledState();
}

void LLPanelDisplay3::refreshEnabledState()
{
	// Ripple Water
	bool ripple = (LLShaderMgr::getMaxVertexShaderLevel(LLShaderMgr::SHADER_ENVIRONMENT) >= 2);
	mCtrlRippleWater->setEnabled(ripple ? TRUE : FALSE);

	// Avatar Mode
	S32 max_avatar_shader = LLShaderMgr::getMaxVertexShaderLevel(LLShaderMgr::SHADER_AVATAR);
	mCtrlAvatarVP->setEnabled((max_avatar_shader > 0) ? TRUE : FALSE);
	
	if (gSavedSettings.getBOOL("RenderAvatarVP") == FALSE)
	{
		max_avatar_shader = 1;
	}
	max_avatar_shader = llmax(max_avatar_shader, 1);
	for (S32 i = 0; i < mCtrlAvatarMode->getItemCount(); i++)
	{
		mCtrlAvatarMode->setIndexEnabled(i, i < max_avatar_shader);
	}
	if (mCtrlAvatarMode->getSelectedIndex() >= max_avatar_shader)
	{
		mCtrlAvatarMode->setSelectedIndex(llmax(max_avatar_shader-1,0));
	}
	// Vertex Shaders
	mCtrlShaderEnable->setEnabled(gPipeline.canUseVertexShaders());
}

void LLPanelDisplay3::apply()
{
	
}

void LLPanelDisplay3::setDefaults()
{

}

void LLPanelDisplay3::cancel()
{
	gSavedSettings.setBOOL("RenderObjectBump", mBumpShiny);
	gSavedSettings.setBOOL("RenderRippleWater", mRippleWater);
	gSavedSettings.setBOOL("RenderAvatarVP", mAvatarVP);
	gSavedSettings.setS32("RenderAvatarMode", mAvatarMode);
	gSavedSettings.setS32("RenderLightingDetail", mLightingDetail);
	gSavedSettings.setS32("RenderTerrainDetail", mTerrainDetail);
	gSavedSettings.setF32("RenderVolumeLODFactor", mPrimLOD);
	gSavedSettings.setF32("RenderFlexTimeFactor", mFlexLOD);
	gSavedSettings.setF32("RenderTreeLODFactor", mTreeLOD);
	gSavedSettings.setF32("RenderAvatarLODFactor", mAvatarLOD);
}

//static
void LLPanelDisplay3::onVertexShaderEnable(LLUICtrl* self, void* data)
{
	((LLPanelDisplay3*) data)->refreshEnabledState();
}
