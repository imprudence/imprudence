/** 
 * @file llvoiceremotectrl.cpp
 * @brief A remote control for voice chat
 *
 * $LicenseInfo:firstyear=2005&license=viewergpl$
 * 
 * Copyright (c) 2005-2007, Linden Research, Inc.
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
 * $/LicenseInfo$
 */

#include "llviewerprecompiledheaders.h"

#include "llvoiceremotectrl.h"

#include "llagent.h"
#include "llui.h"
#include "llbutton.h"
#include "llvieweruictrlfactory.h"
#include "llviewercontrol.h"
#include "llvoiceclient.h"
#include "llimpanel.h"
#include "llfloateractivespeakers.h"
#include "lliconctrl.h"

LLVoiceRemoteCtrl::LLVoiceRemoteCtrl (const LLString& name) : LLPanel(name)
{
	setIsChrome(TRUE);

	gUICtrlFactory->buildPanel(this, "panel_voice_remote.xml");

	mTalkBtn = LLUICtrlFactory::getButtonByName(this, "push_to_talk");
	mTalkBtn->setClickedCallback(onBtnTalkClicked);
	mTalkBtn->setHeldDownCallback(onBtnTalkHeld);
	mTalkBtn->setMouseUpCallback(onBtnTalkReleased);

	mTalkLockBtn = LLUICtrlFactory::getButtonByName(this, "ptt_lock");
	mTalkLockBtn->setClickedCallback(onBtnLock);

	mSpeakersBtn = LLUICtrlFactory::getButtonByName(this, "speakers_btn");
	mSpeakersBtn->setClickedCallback(onClickSpeakers);

	mIsFocusRoot = TRUE;
}

LLVoiceRemoteCtrl::~LLVoiceRemoteCtrl()
{
}

void LLVoiceRemoteCtrl::draw()
{
	BOOL voice_active = FALSE;
	LLVoiceChannel* channelp = LLVoiceChannel::getCurrentVoiceChannel();
	if (channelp)
	{
		voice_active = channelp->isActive();
	}

	mTalkBtn->setEnabled(voice_active);
	mTalkLockBtn->setEnabled(voice_active);

//	if (voice_active)
//	{
//		mTalkBtn->setToolTip("");
//		mTalkLockBtn->setToolTip("");
//	}
//	else
//	{
//		mTalkBtn->setToolTip("");
//		mTalkLockBtn->setToolTip("");
//	}

	// propagate ptt state to button display,
	if (!mTalkBtn->hasMouseCapture())
	{
		// not in push to talk mode, or push to talk is active means I'm talking
		mTalkBtn->setToggleState(!gSavedSettings.getBOOL("PTTCurrentlyEnabled") || gVoiceClient->getUserPTTState());
	}
	mSpeakersBtn->setToggleState(LLFloaterActiveSpeakers::instanceVisible(LLSD()));
	mTalkLockBtn->setToggleState(!gSavedSettings.getBOOL("PTTCurrentlyEnabled"));

	LLUUID talk_blip_image_id;
	if (gVoiceClient->getIsSpeaking(gAgent.getID()))
	{
		F32 voice_power = gVoiceClient->getCurrentPower(gAgent.getID());

		if (voice_power > LLVoiceClient::OVERDRIVEN_POWER_LEVEL)
		{
			talk_blip_image_id = LLUUID(gViewerArt.getString("icn_voice_ptt-on-lvl3.tga"));
		}
		else
		{
			F32 power = gVoiceClient->getCurrentPower(gAgent.getID());
			S32 icon_image_idx = llmin(2, llfloor((power / LLVoiceClient::OVERDRIVEN_POWER_LEVEL) * 3.f));

			switch(icon_image_idx)
			{
			case 0:
				talk_blip_image_id = LLUUID(gViewerArt.getString("icn_voice_ptt-on.tga"));
				break;
			case 1:
				talk_blip_image_id = LLUUID(gViewerArt.getString("icn_voice_ptt-on-lvl1.tga"));
				break;
			case 2:
				talk_blip_image_id = LLUUID(gViewerArt.getString("icn_voice_ptt-on-lvl2.tga"));
				break;
			}
		}
	}
	else
	{
		talk_blip_image_id = LLUUID(gViewerArt.getString("icn_voice_ptt-off.tga"));
	}

	LLIconCtrl* icon = LLUICtrlFactory::getIconByName(this, "voice_volume");
	if (icon)
	{
		icon->setImage(talk_blip_image_id);
	}

	LLPanel::draw();
}

void LLVoiceRemoteCtrl::onBtnTalkClicked(void *user_data)
{
	// when in toggle mode, clicking talk button turns mic on/off
	if (gSavedSettings.getBOOL("PushToTalkToggle"))
	{
		gVoiceClient->toggleUserPTTState();
	}
}

void LLVoiceRemoteCtrl::onBtnTalkHeld(void *user_data)
{
	// when not in toggle mode, holding down talk button turns on mic
	if (!gSavedSettings.getBOOL("PushToTalkToggle"))
	{
		gVoiceClient->setUserPTTState(true);
	}
}

void LLVoiceRemoteCtrl::onBtnTalkReleased(void* user_data)
{
	// when not in toggle mode, releasing talk button turns off mic
	if (!gSavedSettings.getBOOL("PushToTalkToggle"))
	{
		gVoiceClient->setUserPTTState(false);
	}
}

void LLVoiceRemoteCtrl::onBtnLock(void* user_data)
{
	LLVoiceRemoteCtrl* remotep = (LLVoiceRemoteCtrl*)user_data;

	remotep->mTalkLockBtn->toggleState();
	gSavedSettings.setBOOL("PTTCurrentlyEnabled", !remotep->mTalkLockBtn->getToggleState());
}

void LLVoiceRemoteCtrl::onClickSpeakers(void *user_data)
{
	LLFloaterActiveSpeakers::toggleInstance(LLSD());
}

