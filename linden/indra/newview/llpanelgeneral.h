/** 
 * @file llpanelgeneral.h
 * @author James Cook
 * @brief General preferences panel in preferences floater
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

#ifndef LL_LLPANELGENERAL_H
#define LL_LLPANELGENERAL_H

#include "llpanel.h"

class LLCheckBoxCtrl;
class LLRadioGroup;
class LLColorSwatchCtrl;
class LLSpinCtrl;
class LLTextBox;
class LLComboBox;

class LLPanelGeneral : public LLPanel
{
public:
	LLPanelGeneral();
	virtual ~LLPanelGeneral();

	virtual BOOL postBuild();
	void refresh();
	void apply();
	void cancel();

	static void clickShowStartLocation(LLUICtrl*, void* user_data);
	static void set_start_location(LLUICtrl* ctrl, void* data);

protected:
	S32 mRenderName;
	BOOL mRenderNameHideSelf;
	BOOL mSmallAvatarNames;
	BOOL mRenderHideGroupTitle;
	BOOL mChatOnlineNotification;
	F32 mAFKTimeout;
	BOOL mNotifyMoney;
	BOOL mUseDefaultColor;
	LLColor4 mEffectColor;
	BOOL mMiniMapRotate;
	S32 mCrashBehavior;
	LLString mLoginLocation;
	LLString mLanguage;
};

#endif
