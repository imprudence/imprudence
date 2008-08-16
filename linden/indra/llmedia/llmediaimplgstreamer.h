/** 
 * @file llmediaimplgstreamer.h
 * @brief implementation that supports media playback via GStreamer.
 *
 * $LicenseInfo:firstyear=2007&license=viewergpl$
 * 
 * Copyright (c) 2007-2008, Linden Research, Inc.
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

// header guard
#ifndef llmediaimplgstreamer_h
#define llmediaimplgstreamer_h

#if LL_GSTREAMER_ENABLED

extern "C" {
#include <gst/gst.h>

#include <apr-1/apr_pools.h>
#include <apr-1/apr_dso.h>
}

#include "stdtypes.h"

#include "llmediamoviebase.h"

#include "llmediaimplgstreamervidplug.h"
#ifdef LL_GST_SOUNDSINK
#include "llmediaimplgstreamersndplug.h"
#endif // LL_GST_SOUNDSINK

///////////////////////////////////////////////////////////////////////////
class LLMediaImplGStreamer:
	public LLMediaMovieBase
{
	public:
		LLMediaImplGStreamer ();
		virtual ~LLMediaImplGStreamer ();

		////////////////////////////////////////////////////////
		// implementation of the media public interface

		// housekeeping
		virtual BOOL setBuffer ( U8* bufferIn );
		virtual BOOL init ();
		virtual BOOL load ( const LLString& urlIn );
		virtual BOOL unload ();

		// transport controls
		virtual BOOL stop ();
		virtual BOOL play ();
		virtual BOOL loop ( S32 howMany );
		virtual BOOL pause ();
		virtual BOOL seek ( F64 time );

		// audio levels
		virtual BOOL setVolume ( F32 volumeIn );
		virtual F32 getVolume ();

		// status
		virtual BOOL isIdle () const;
		virtual BOOL isBuffering () const;
		virtual BOOL isError () const;
		virtual BOOL isLoaded () const;
		virtual BOOL isStopped () const;
		virtual BOOL isPaused () const;
		virtual BOOL isPlaying () const;
		virtual BOOL isLooping () const;
		virtual F64 getTime () const;

		// media data
		virtual S32 updateMedia ();
		virtual void setAutoScaled ( BOOL autoScaledIn );
		virtual U8* getMediaData ();
		virtual F64 getMediaDuration () const;

	        // class-specific
	        GMainLoop *getPump() {return mPump;};
	        typedef enum { ModeNone, ModeIdle, ModeError, ModeBuffering, ModeStopped, ModePaused, ModePlaying, ModeLooping } llGstMode;
           	llGstMode getCurrentMode() {return currentMode;};
           	void setCurrentMode(llGstMode mode) {currentMode = mode;};

	private:
		// misc
		U8* mediaData;
		BOOL ownBuffer;
		BOOL autoScaled;
        	F32 mVolume;

	        llGstMode currentMode;

	        // GStreamer-specific
        	GMainLoop *mPump; // event pump for this media
	        GstElement *mPlaybin;
		GstSLVideo *mVideoSink;
#ifdef LL_GST_SOUNDSINK
		GstSLSound *mAudioSink;
#endif // LL_GST_SOUNDSINK
};

// called during shutdown when no instances may exist
void UnloadGStreamer();


#endif // LL_GSTREAMER_ENABLED

#endif // llmediaimplgstreamer_h
