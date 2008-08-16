/** 
 * @file llfloatervoicewizard.h
 * @author Richard Nelson
 * @brief Voice communication set-up wizard
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

#ifndef LL_LLFLOATERVOICEWIZARD_H
#define LL_LLFLOATERVOICEWIZARD_H

#include "llfloater.h"

class LLPrefsVoiceLogic;
class LLPanelDeviceSettings;

class LLFloaterVoiceWizard 
: public LLFloater, public LLUISingleton<LLFloaterVoiceWizard>
{
public:
	LLFloaterVoiceWizard(const LLSD& seed);
	virtual ~LLFloaterVoiceWizard();

	/*virtual*/ BOOL postBuild();
	/*virtual*/ void draw();
	/*virtual*/ void onOpen();
	/*virtual*/ void onClose(bool app_quitting);

protected:
	static void onClickNext(void *user_data);
	static void onClickBack(void *user_data);
	static void onClickOK(void *user_data);
	static void onClickCancel(void *user_data);
	static void onCommitVoiceEnable(LLUICtrl* ctrl, void* user_data);
	static void* createPanelDeviceSettings(void* user_data);

protected:
	LLPrefsVoiceLogic* mLogic;
	LLPanelDeviceSettings* mDevicePanel;
};

class LLPanelDeviceSettings : public LLPanel
{
public:
	LLPanelDeviceSettings();
	~LLPanelDeviceSettings();

	/*virtual*/ void draw();
	/*virtual*/ BOOL postBuild();
	void apply();
	void cancel();
	void refresh();
	void onOpen();
	void onClose(bool app_quitting);

protected:
	static void onCommitInputDevice(LLUICtrl* ctrl, void* user_data);
	static void onCommitOutputDevice(LLUICtrl* ctrl, void* user_data);

	F32 mMicVolume;
	std::string mInputDevice;
	std::string mOutputDevice;
	LLComboBox		*mCtrlInputDevices;
	LLComboBox		*mCtrlOutputDevices;
	BOOL mDevicesUpdated;
};

class LLFloaterDeviceSettings : public LLFloater, public LLUISingleton<LLFloaterDeviceSettings>
{
public:
	LLFloaterDeviceSettings(const LLSD& seed);
	/*virtual*/ void onOpen();
	/*virtual*/ void onClose(bool app_quitting);
	/*virtual*/ void draw();
	void apply();
	void cancel();

protected:
	static void* createPanelDeviceSettings(void* user_data);

	LLPanelDeviceSettings* mDevicePanel;
};

#endif // LL_LLFLOATERVOICEWIZARD_H
