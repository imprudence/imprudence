/** 
 * @file llmediaremotectrl.cpp
 * @brief A remote control for media (video and music)
 *
 * $LicenseInfo:firstyear=2005&license=viewergpl$
 * 
 * Copyright (c) 2005-2009, Linden Research, Inc.
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

#include "llmediaremotectrl.h"

#include "llaudioengine.h"
#include "lliconctrl.h"
#include "llmimetypes.h"
#include "lloverlaybar.h"
#include "llviewermedia.h"
#include "llviewerparcelmedia.h"
#include "llviewerparcelmgr.h"
#include "lluictrlfactory.h"
#include "llpanelaudiovolume.h"
#include "llparcel.h"
#include "llviewercontrol.h"
#include "llbutton.h"

////////////////////////////////////////////////////////////////////////////////
//
//

static LLRegisterWidget<LLMediaRemoteCtrl> r("media_remote");

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
		LLUICtrlFactory::getInstance()->buildPanel(this, "panel_media_remote_expanded.xml", &getFactoryMap());
	}
	else
	{
		LLUICtrlFactory::getInstance()->buildPanel(this, "panel_media_remote.xml", &getFactoryMap());
	}
}

BOOL LLMediaRemoteCtrl::postBuild()
{
	mControls = getChild<LLPanel>("media_controls");
	llassert_always(mControls);
	
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
	
	LLButton* expand_button = getChild<LLButton>("expand");
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
void LLMediaRemoteCtrl::setToolTip(const std::string& msg)
{
	// TODO: this gets removed for Media on a Prim
	
	const std::string mime_type = LLViewerParcelMedia::getMimeType();
	std::string tool_tip = LLMIMETypes::findToolTip(mime_type);
	std::string play_tip = LLMIMETypes::findPlayTip(mime_type);
	// childSetToolTip("media_stop", mControls->getString("stop_label") + "\n" + tool_tip);
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

	static LLColor4* sIconDisabledColor = rebind_llcontrol<LLColor4>("IconDisabledColor", &gColors, true);

	LLColor4 music_icon_color = (*sIconDisabledColor).getValue();
	LLColor4 media_icon_color = (*sIconDisabledColor).getValue();
	std::string media_type = "none/none";

	// Put this in xui file
	std::string media_url = mControls->getString("default_tooltip_label");
	LLParcel* parcel = LLViewerParcelMgr::getInstance()->getAgentParcel();

	if (gSavedSettings.getBOOL("AudioStreamingVideo"))
	{
		if ( parcel && !parcel->getMediaURL().empty())
		{
			// Set the tooltip
			// Put this text into xui file
			media_url = parcel->getObscureMedia() ? mControls->getString("media_hidden_label") : parcel->getMediaURL();
			media_type = parcel->getMediaType();

			play_media_enabled = true;
			media_icon_color = LLUI::sColorsGroup->getColor( "IconEnabledColor" );

			LLViewerMediaImpl::EMediaStatus status = LLViewerParcelMedia::getStatus();
			switch(status)
			{
			case LLViewerMediaImpl::MEDIA_NONE:
				media_show_pause = false;
				stop_media_enabled = false;
				break;
			case LLViewerMediaImpl::MEDIA_LOADING:
			case LLViewerMediaImpl::MEDIA_LOADED:
			case LLViewerMediaImpl::MEDIA_PLAYING:
				// HACK: only show the pause button for movie types
				media_show_pause = LLMIMETypes::widgetType(parcel->getMediaType()) == "movie" ? true : false;
				stop_media_enabled = true;
				play_media_enabled = false;
				break;
			case LLViewerMediaImpl::MEDIA_PAUSED:
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
		if ( parcel && !parcel->getMusicURL().empty())
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
		// Don't test the mime-type: this is not updated in a consistent basis. The existence of a valid gAudiop is enough guarantee.
	}
	const std::string media_icon_name = LLMIMETypes::findIcon(media_type);
	LLButton* music_play_btn = getChild<LLButton>("music_play");
	LLButton* music_stop_btn = getChild<LLButton>("music_stop");
	LLButton* music_pause_btn = getChild<LLButton>("music_pause");
	LLButton* media_play_btn = getChild<LLButton>("media_play");
	LLButton* media_stop_btn = getChild<LLButton>("media_stop");
	LLButton* media_pause_btn = getChild<LLButton>("media_pause");
	LLIconCtrl* media_icon = getChild<LLIconCtrl>("media_icon");

	music_play_btn->setEnabled(play_music_enabled);
	music_stop_btn->setEnabled(stop_music_enabled);
	music_pause_btn->setEnabled(music_show_pause);
	music_pause_btn->setVisible(music_show_pause);
	music_play_btn->setVisible(! music_show_pause);
	childSetColor("music_icon", music_icon_color);
	if(!media_icon_name.empty())
	{
		media_icon->setImage(media_icon_name);
	}

	media_play_btn->setEnabled(play_media_enabled);
	media_stop_btn->setEnabled(stop_media_enabled);
	media_pause_btn->setEnabled(media_show_pause);
	media_pause_btn->setVisible(media_show_pause);
	media_play_btn->setVisible(! media_show_pause);
	childSetColor("media_icon", media_icon_color);

	setToolTip(media_url);
}


