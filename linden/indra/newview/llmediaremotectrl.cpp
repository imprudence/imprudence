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

#include "llmediaremotectrl.h"

#include "audioengine.h"
#include "lliconctrl.h"
#include "llmimetypes.h"
#include "lloverlaybar.h"
#include "llviewermedia.h"
#include "llviewerparcelmedia.h"
#include "llviewerparcelmgr.h"
#include "llvieweruictrlfactory.h"
#include "llpanelaudiovolume.h"
#include "llparcel.h"
#include "llviewercontrol.h"
#include "llbutton.h"

////////////////////////////////////////////////////////////////////////////////
//
//
LLMediaRemoteCtrl::LLMediaRemoteCtrl()
{
	setIsChrome(TRUE);
	setFocusRoot(TRUE);

	mFactoryMap["Volume Panel"]	= LLCallbackMap(createVolumePanel, NULL);
	build();
}

void LLMediaRemoteCtrl::build()
{
	//HACK: only works because we don't have any implicit children (i.e. titlebars, close button, etc)
	deleteAllChildren();
	if (gSavedSettings.getBOOL("ShowVolumeSettingsPopup"))
	{
		gUICtrlFactory->buildPanel(this, "panel_media_remote_expanded.xml", &getFactoryMap());
	}
	else
	{
		gUICtrlFactory->buildPanel(this, "panel_media_remote.xml", &getFactoryMap());
	}
}

BOOL LLMediaRemoteCtrl::postBuild()
{
	childSetAction("media_play",LLOverlayBar::toggleMediaPlay,this);
	childSetAction("music_play",LLOverlayBar::toggleMusicPlay,this);
	childSetAction("media_stop",LLOverlayBar::mediaStop,this);
	childSetAction("music_stop",LLOverlayBar::toggleMusicPlay,this);
	childSetAction("media_pause",LLOverlayBar::toggleMediaPlay,this);
	childSetAction("music_pause",LLOverlayBar::toggleMusicPlay,this);

	childSetAction("expand", onClickExpandBtn, this);	
	return TRUE;
}

void LLMediaRemoteCtrl::draw()
{
	enableMediaButtons();
	
	LLButton* expand_button = LLUICtrlFactory::getButtonByName(this, "expand");
	if (expand_button)
	{
		if (expand_button->getToggleState())
		{
			expand_button->setImageOverlay("arrow_down.tga");
		}
		else
		{
			expand_button->setImageOverlay("arrow_up.tga");
		}
	}

	LLPanel::draw();
}

LLMediaRemoteCtrl::~LLMediaRemoteCtrl ()
{
}

//static 
void LLMediaRemoteCtrl::onClickExpandBtn(void* user_data)
{
	LLMediaRemoteCtrl* remotep = (LLMediaRemoteCtrl*)user_data;

	remotep->build();
	gOverlayBar->layoutButtons();

}

//static
void* LLMediaRemoteCtrl::createVolumePanel(void* data)
{
	LLPanelAudioVolume* panel = new LLPanelAudioVolume();
	return panel;
}

// Virtual
void LLMediaRemoteCtrl::setToolTip(const LLString& msg)
{
	LLString mime_type = LLMIMETypes::translate(LLViewerMedia::getMimeType());
	LLString tool_tip = LLMIMETypes::findToolTip(LLViewerMedia::getMimeType());
	LLString play_tip = LLMIMETypes::findPlayTip(LLViewerMedia::getMimeType());
	// childSetToolTip("media_stop", getString("stop_label") + "\n" + tool_tip);
	childSetToolTip("media_icon", tool_tip);
	childSetToolTip("media_play", play_tip);
}

void LLMediaRemoteCtrl::enableMediaButtons()
{
	// Media
	bool play_media_enabled = false;
	bool stop_media_enabled = false;
	bool play_music_enabled = false;
	bool stop_music_enabled = false;
	bool music_show_pause = false;
	bool media_show_pause = false;
	LLColor4 music_icon_color = LLUI::sColorsGroup->getColor( "IconDisabledColor" );
	LLColor4 media_icon_color = LLUI::sColorsGroup->getColor( "IconDisabledColor" );
	LLString media_type = "none/none";

	// Put this in xui file
	LLString media_url = getString("default_tooltip_label");
	LLParcel* parcel = gParcelMgr->getAgentParcel();

	if (gSavedSettings.getBOOL("AudioStreamingVideo"))
	{
		if ( parcel && parcel->getMediaURL()[0])
		{
			// Set the tooltip
			// Put this text into xui file
			media_url = parcel->getObscureMedia() ? getString("media_hidden_label") : parcel->getMediaURL();
			media_type = parcel->getMediaType();

			play_media_enabled = true;
			media_icon_color = LLUI::sColorsGroup->getColor( "IconEnabledColor" );

			LLMediaBase::EStatus status = LLViewerParcelMedia::getStatus();
			switch(status)
			{
			case LLMediaBase::STATUS_STOPPED:
			case LLMediaBase::STATUS_UNKNOWN:
				media_show_pause = false;
				stop_media_enabled = false;
				break;
			case LLMediaBase::STATUS_STARTED:
			case LLMediaBase::STATUS_NAVIGATING:
			case LLMediaBase::STATUS_RESETTING:
				// HACK: only show the pause button for movie types
				media_show_pause = LLMIMETypes::widgetType(parcel->getMediaType()) == "movie" ? true : false;
				stop_media_enabled = true;
				play_media_enabled = false;
				break;
			case LLMediaBase::STATUS_PAUSED:
				media_show_pause = false;
				stop_media_enabled = true;
				break;
			default:
				// inherit defaults above
				break;
			}
		}
	}
	if (gSavedSettings.getBOOL("AudioStreamingMusic") && gAudiop)
	{
	
		if ( parcel && parcel->getMusicURL()[0])
		{
			play_music_enabled = true;
			music_icon_color = LLUI::sColorsGroup->getColor( "IconEnabledColor" );

			if (gOverlayBar->musicPlaying())
			{
				music_show_pause = true;
				stop_music_enabled = true;
			}
			else
			{
				music_show_pause = false;
				stop_music_enabled = false;
			}
		}
		// if no mime type has been set disable play
		if( LLViewerMedia::getMimeType().empty() 
			|| LLViewerMedia::getMimeType() == "none/none")
		{
			play_media_enabled = false;
			stop_media_enabled = false;
		}
	}
	const LLUUID media_icon_id = LLUUID(gViewerArt.findString(LLMIMETypes::findIcon(media_type)));
	LLButton* music_play_btn = LLUICtrlFactory::getButtonByName(this, "music_play");
	LLButton* music_stop_btn = LLUICtrlFactory::getButtonByName(this, "music_stop");
	LLButton* music_pause_btn = LLUICtrlFactory::getButtonByName(this, "music_pause");
	LLButton* media_play_btn = LLUICtrlFactory::getButtonByName(this, "media_play");
	LLButton* media_stop_btn = LLUICtrlFactory::getButtonByName(this, "media_stop");
	LLButton* media_pause_btn = LLUICtrlFactory::getButtonByName(this, "media_pause");
	LLIconCtrl* media_icon = LLUICtrlFactory::getIconByName(this, "media_icon");

	music_play_btn->setEnabled(play_music_enabled);
	music_stop_btn->setEnabled(stop_music_enabled);
	music_pause_btn->setEnabled(music_show_pause);
	music_pause_btn->setVisible(music_show_pause);
	music_play_btn->setVisible(! music_show_pause);
	childSetColor("music_icon", music_icon_color);
	if(! media_icon_id.isNull())
	{
		media_icon->setImage(media_icon_id);
	}

	media_play_btn->setEnabled(play_media_enabled);
	media_stop_btn->setEnabled(stop_media_enabled);
	media_pause_btn->setEnabled(media_show_pause);
	media_pause_btn->setVisible(media_show_pause);
	media_play_btn->setVisible(! media_show_pause);
	childSetColor("media_icon", media_icon_color);

	setToolTip(media_url);
}


