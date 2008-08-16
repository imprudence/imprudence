/** 
 * @file llmediaremotectrl.h
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

#ifndef LL_LLMEDIAREMOTECTRL_H
#define LL_LLMEDIAREMOTECTRL_H

#include "llpanel.h"
#include "llvolumesliderctrl.h"
#include "lleventemitter.h"

////////////////////////////////////////////////////////////////////////////////
//
class LLMediaRemoteCtrlEvent
{
	public:
		LLMediaRemoteCtrlEvent ( LLUICtrl* controlIn, F32 valueIn ):
			control ( controlIn ),
				value ( valueIn )
		{
		};

		virtual ~LLMediaRemoteCtrlEvent () { }

		LLUICtrl* getControl () const { return control; };
		F32 getValue () const { return value; };

	private:
		LLUICtrl* control;
		F32 value;
};

////////////////////////////////////////////////////////////////////////////////
//
class LLMediaRemoteCtrlObserver
{
public:
	typedef LLMediaRemoteCtrlEvent EventType;
	virtual ~LLMediaRemoteCtrlObserver() {}
	virtual void onVolumeChange ( const EventType& eventIn ) { };
	virtual void onStopButtonPressed ( const EventType& eventIn ) { };
	virtual void onPlayButtonPressed ( const EventType& eventIn ) { };
	virtual void onPauseButtonPressed ( const EventType& eventIn ) { };
};

////////////////////////////////////////////////////////////////////////////////
//
class LLMediaRemoteCtrl :
	public LLPanel
{
	public:
		LLMediaRemoteCtrl ( const std::string& name,
							const std::string& label,
							const LLRect& rect,
							const std::string& xml_file );
	
		virtual ~LLMediaRemoteCtrl ();

		virtual EWidgetType getWidgetType() const;
		virtual LLString getWidgetTag() const;

		// set current transport state of remote control
		enum TransportState { Stop, Play, Pause };
		void setTransportState ( TransportState transportStateIn, BOOL pauseEnabled );
		TransportState getTransportState ();
		void setVolume ( F32 volumeIn );
		
		// allow consumers to observe remote control events
		virtual BOOL addObserver ( LLMediaRemoteCtrlObserver* observerIn )
		{
			return mediaRemoteCtrlEventEmitter.addObserver ( observerIn );
		};
		virtual BOOL remObserver ( LLMediaRemoteCtrlObserver* observerIn )
		{
			return mediaRemoteCtrlEventEmitter.remObserver ( observerIn );
		};

	private:
		LLVolumeSliderCtrl* mVolumeSlider;
		LLRect volumeSliderRect;

		TransportState transportState;

		LLTextBox* titleLabel;
		LLButton* playButton;
		LLButton* pauseButton;
		LLButton* stopButton;

		// event emitter
		eventEmitter < LLMediaRemoteCtrlObserver > mediaRemoteCtrlEventEmitter;

		// callbacks
		static void onCommitVolume ( LLUICtrl* ctrl, void* data );
		static void onPlayButton ( void* data );
		static void onPauseButton ( void* data );
		static void onStopButton ( void* data );
};

#endif
