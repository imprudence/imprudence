/** 
 * @file llpaneldisplay.h
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

#ifndef LL_LLPANELDISPLAY_H
#define LL_LLPANELDISPLAY_H

#include "llpanel.h"

class LLSlider;
class LLSpinCtrl;
class LLCheckBoxCtrl;
class LLRadioGroup;
class LLComboBox;
class LLLineEditor;
class LLSliderCtrl;

class LLPanelDisplay 
: public LLPanel
{
	friend class LLPreferenceCore;
	
public:
	LLPanelDisplay();
	virtual ~LLPanelDisplay();

	virtual BOOL postBuild();
	void refresh();	// Refresh enable/disable
	void apply();	// Apply the changed values.
	void applyResolution();
	void cancel();
	
protected:
	LLComboBox		*mCtrlFullScreen;               // Fullscreen resolution
	LLCheckBoxCtrl	*mCtrlWindowed;					// windowed mode
	LLCheckBoxCtrl	*mCtrlAutoDetectAspect;			// automatically detect aspect ratio
	LLComboBox		*mCtrlAspectRatio;				// user provided aspect ratio

	BOOL mFSAutoDetectAspect;
	F32 mUIScaleFactor;
	BOOL mUIAutoScale;
	BOOL mFirstPersonAvatarVisible;
	F32 mRenderFarClip;
	F32 mAspectRatio;
	static void onCommitAutoDetectAspect(LLUICtrl *ctrl, void *data);
	static void onKeystrokeAspectRatio(LLLineEditor* caller, void* user_data);
	static void onSelectAspectRatio(LLUICtrl*, void*);
	static void onCommitWindowedMode(LLUICtrl* ctrl, void *data);
	static void onApplyResolution(LLUICtrl* ctrl, void* data);

	// helper function
	static void fractionFromDecimal(F32 decimal_val, S32& numerator, S32& denominator);
};

class LLPanelDisplay2 : public LLPanel
{
	friend class LLPreferenceCore;
	
public:
	LLPanelDisplay2();
	virtual ~LLPanelDisplay2();

	virtual BOOL postBuild();
	void refresh();	// Refresh enable/disable
	void apply();	// Apply the changed values.
	void cancel();
	void refreshEnabledState();

protected:
	LLRadioGroup*	mRadioVideoCardMem;

	BOOL mUseVBO;
	BOOL mUseAniso;
	F32 mGamma;
	F32 mBrightness;
	S32 mVideoCardMem;
	F32 mFogRatio;
	S32 mParticleCount;
	S32 mCompositeLimit;
	S32 mDebugBeaconLineWidth;
	BOOL mProbeHardwareOnStartup;
};

class LLPanelDisplay3 : public LLPanel
{
	friend class LLPreferenceCore;
	
public:
	LLPanelDisplay3();
	virtual ~LLPanelDisplay3();

	virtual BOOL postBuild();
	void refresh();	// Refresh enable/disable
	void apply();	// Apply the changed values.
	void setDefaults();
	void cancel();
	void refreshEnabledState();
	static void onVertexShaderEnable(LLUICtrl*, void*);
		
protected:
	LLCheckBoxCtrl	*mCtrlBumpShiny;
	LLCheckBoxCtrl	*mCtrlRippleWater;
	LLCheckBoxCtrl	*mCtrlAvatarVP;
	LLCheckBoxCtrl	*mCtrlShaderEnable;
	LLRadioGroup	*mCtrlAvatarMode;
	LLRadioGroup	*mRadioLightingDetail2;

	LLRadioGroup	*mRadioTerrainDetail;
	
	LLSlider		*mCtrlLODFactor;				// LOD for volume objects
	LLSlider		*mCtrlFlexFactor;				// Timeslice for flexible objects
	LLSlider		*mCtrlTreeFactor;				// Control tree cutoff distance
	LLSlider		*mCtrlAvatarFactor;				// LOD for avatars

	BOOL mBumpShiny;
	BOOL mRippleWater;
	BOOL mAvatarVP;
	BOOL mShaderEnable;
	S32 mAvatarMode;
	S32 mLightingDetail;
	S32 mTerrainDetail;
	F32 mPrimLOD;
	F32 mFlexLOD;
	F32 mTreeLOD;
	F32 mAvatarLOD;
};

const S32 LL_MAX_VRAM_INDEX = 6;

#endif
