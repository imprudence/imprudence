/** 
 * @file llmediaremotectrl.cpp
 * @brief A remote control for media (video and music)
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
 */

#include "llviewerprecompiledheaders.h"

#include "llmediaremotectrl.h"

#include "llui.h"
#include "lltextbox.h"
#include "llbutton.h"
#include "llfocusmgr.h"
#include "llmediaengine.h"
#include "llviewercontrol.h"
#include "llvieweruictrlfactory.h"

////////////////////////////////////////////////////////////////////////////////
//
//
LLMediaRemoteCtrl::
LLMediaRemoteCtrl ( const std::string& name,
					const std::string& label,
					const LLRect& rect,
					const std::string& xml_file ) :
	LLPanel ( name, rect, FALSE )
{
	setFollows(FOLLOWS_LEFT | FOLLOWS_TOP);
	setBackgroundVisible(TRUE);
	setIsChrome(TRUE);

	gUICtrlFactory->buildPanel(this, xml_file);

	playButton = LLUICtrlFactory::getButtonByName(this, "play_btn");
	stopButton = LLUICtrlFactory::getButtonByName(this, "stop_btn");
	pauseButton = LLUICtrlFactory::getButtonByName(this, "pause_btn");
	
	childSetAction("play_btn",onPlayButton,this);
	childSetAction("stop_btn",onStopButton,this);
	childSetAction("pause_btn",onPauseButton,this);

	mVolumeSlider = LLViewerUICtrlFactory::getVolumeSliderByName(this, "volume_slider");	
	
	childSetCommitCallback("volume_slider", onCommitVolume, this);

	mIsFocusRoot = TRUE;
}

////////////////////////////////////////////////////////////////////////////////
//
//
LLMediaRemoteCtrl::
~LLMediaRemoteCtrl ()
{
}

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
void
LLMediaRemoteCtrl::
setTransportState ( TransportState transportStateIn, BOOL pauseEnabled )
{
	transportState = transportStateIn;

	if ( transportState == Play )
	{
		playButton->setVisible ( TRUE );
		playButton->setEnabled ( pauseEnabled );

		pauseButton->setVisible ( FALSE );
		pauseButton->setEnabled ( FALSE );

		stopButton->setVisible ( TRUE );
		stopButton->setEnabled ( TRUE );
	}
	else
	if ( transportState == Pause )
	{
		playButton->setVisible ( FALSE );
		playButton->setEnabled ( FALSE );

		pauseButton->setVisible ( TRUE );
		pauseButton->setEnabled ( TRUE );

		stopButton->setVisible ( TRUE );
		stopButton->setEnabled ( TRUE );
	}
	else
	if ( transportState == Stop )
	{
		playButton->setVisible ( TRUE );
		playButton->setEnabled ( TRUE );

		pauseButton->setVisible ( FALSE );
		pauseButton->setEnabled ( FALSE );

		stopButton->setVisible ( TRUE );
		stopButton->setEnabled ( FALSE );
	};
}

////////////////////////////////////////////////////////////////////////////////
//
//
LLMediaRemoteCtrl::TransportState
LLMediaRemoteCtrl::
getTransportState ()
{
	return transportState;
}


////////////////////////////////////////////////////////////////////////////////
//
//
void
LLMediaRemoteCtrl::
setVolume ( F32 volumeIn )
{
	mVolumeSlider->setValue ( volumeIn );
}

////////////////////////////////////////////////////////////////////////////////
//
//
void
LLMediaRemoteCtrl::
onCommitVolume ( LLUICtrl* ctrl, void* data )
{
	LLMediaRemoteCtrl* self = ( LLMediaRemoteCtrl* ) data;

	LLMediaRemoteCtrlEvent event ( self, (F32)self->mVolumeSlider->getValue().asReal() );
	self->mediaRemoteCtrlEventEmitter.update ( &LLMediaRemoteCtrlObserver::onVolumeChange, event );
}

////////////////////////////////////////////////////////////////////////////////
//
//
void
LLMediaRemoteCtrl::
onPlayButton ( void* data )
{
	LLMediaRemoteCtrl* self = ( LLMediaRemoteCtrl* ) data;

	LLMediaRemoteCtrlEvent event ( self, 0.0f );
	self->mediaRemoteCtrlEventEmitter.update ( &LLMediaRemoteCtrlObserver::onPlayButtonPressed, event );
}

////////////////////////////////////////////////////////////////////////////////
//
//
void
LLMediaRemoteCtrl::
onPauseButton ( void* data )
{
	LLMediaRemoteCtrl* self = ( LLMediaRemoteCtrl* ) data;

	LLMediaRemoteCtrlEvent event ( self, 0.0f );
	self->mediaRemoteCtrlEventEmitter.update ( &LLMediaRemoteCtrlObserver::onPauseButtonPressed, event );
}

////////////////////////////////////////////////////////////////////////////////
//
//
void
LLMediaRemoteCtrl::
onStopButton ( void* data )
{
	LLMediaRemoteCtrl* self = ( LLMediaRemoteCtrl* ) data;

	LLMediaRemoteCtrlEvent event ( self, 0.0f );
	self->mediaRemoteCtrlEventEmitter.update ( &LLMediaRemoteCtrlObserver::onStopButtonPressed, event );
}

