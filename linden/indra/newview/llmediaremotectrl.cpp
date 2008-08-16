/** 
 * @file llmediaremotectrl.cpp
 * @brief A remote control for media (video and music)
 *
 * $LicenseInfo:firstyear=2005&license=viewergpl$
 * 
 * Copyright (c) 2005-2008, Linden Research, Inc.
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

#include "llmediaremotectrl.h"

#include "lloverlaybar.h"
#include "llvieweruictrlfactory.h"
#include "llpanelaudiovolume.h"

////////////////////////////////////////////////////////////////////////////////
//
//
LLMediaRemoteCtrl::LLMediaRemoteCtrl ( const LLString& name,
									   const LLString& label,
									   const LLRect& rect,
									   const LLString& xml_file ) :
	LLPanel ( name, rect, FALSE )
{
	setIsChrome(TRUE);

	gUICtrlFactory->buildPanel(this, xml_file);

	mIsFocusRoot = TRUE;
}

BOOL LLMediaRemoteCtrl::postBuild()
{
	childSetAction("media_play",LLOverlayBar::mediaPlay,this);
	childSetAction("media_stop",LLOverlayBar::mediaStop,this);
	childSetAction("media_pause",LLOverlayBar::mediaPause,this);

	childSetAction("music_play",LLOverlayBar::musicPlay,this);
	childSetAction("music_stop",LLOverlayBar::musicStop,this);
	childSetAction("music_pause",LLOverlayBar::musicPause,this);

 	childSetAction("volume",LLOverlayBar::toggleAudioVolumeFloater,this);
	
	return TRUE;
}

LLMediaRemoteCtrl::~LLMediaRemoteCtrl ()
{
}

////////////////////////////////////////////////////////////////////////////////
//
//
EWidgetType LLMediaRemoteCtrl::getWidgetType() const
{
	return WIDGET_TYPE_MEDIA_REMOTE;
}

LLString LLMediaRemoteCtrl::getWidgetTag() const
{
	return LL_MEDIA_REMOTE_CTRL_TAG;
}

////////////////////////////////////////////////////////////////////////////////
//
//
void LLMediaRemoteCtrl::draw()
{
	LLOverlayBar::enableMusicButtons(this);
	LLOverlayBar::enableMediaButtons(this);
	LLPanel::draw();
	// make volume button reflect of volume floater
	childSetValue("volume", LLFloaterAudioVolume::instanceVisible(LLSD()));
}
