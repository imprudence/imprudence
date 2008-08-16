/** 
 * @file llmediaimplgstreamer.cpp
 * @brief implementation that supports various media through GStreamer.
 *
 * Copyright (c) 2007-2007, Linden Research, Inc.
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

#include "linden_common.h"

#if LL_GSTREAMER_ENABLED

extern "C" {
#include <gst/gst.h>
}

#include "llmediaimplgstreamer.h"

#include "llmediaimplgstreamervidplug.h"

#ifdef LL_GST_SOUNDSINK
#include "llmediaimplgstreamersndplug.h"
#endif // LL_GST_SOUNDSINK

#include "llmediaimplgstreamer_syms.h"

#include "llgl.h"
#include "llglheaders.h"	// For gl texture modes

///////////////////////////////////////////////////////////////////////////////
//
LLMediaImplGStreamer::
LLMediaImplGStreamer () :
	mediaData ( NULL ),
	ownBuffer ( TRUE ),
	mVolume ( 1.0f ),
	currentMode ( ModeIdle ),
	mPump ( NULL ),
	mPlaybin ( NULL ),
	mVideoSink ( NULL )
#ifdef LL_GST_SOUNDSINK
	,mAudioSink ( NULL )
#endif // LL_GST_SOUNDSINK
{
	mMediaDepthBytes = 4;
	mTextureDepth = 4;
	mTextureFormatInternal = GL_RGB8;
	mTextureFormatPrimary = GL_BGRA;
	mTextureFormatType = GL_UNSIGNED_INT_8_8_8_8_REV;
}

///////////////////////////////////////////////////////////////////////////////
//
LLMediaImplGStreamer::
~LLMediaImplGStreamer ()
{
	unload();
}

void UnloadGStreamer()
{
	ungrab_gst_syms();
}


///////////////////////////////////////////////////////////////////////////////
//
BOOL
LLMediaImplGStreamer::
setBuffer ( U8* bufferIn )
{
	// Since we've pointed GStreamer at the old media data buffer
	// directly, we need to be somewhat careful deleting it...
	U8* oldMediaData = mediaData;
	BOOL ownedMediaData = ownBuffer;

	if(bufferIn == NULL)
	{
		// Passing NULL to this function requests that the object
		// allocate its own buffer.
		mediaData = new unsigned char[ mMediaHeight * mMediaRowbytes ];
		ownBuffer = TRUE;
	}
	else
	{
		// Use the supplied buffer.
		mediaData = bufferIn;
		ownBuffer = FALSE;
	}
	
	if(mediaData == NULL)
	{
		// This is bad - probably out of memory.
		llerrs << "LLMediaImplGStreamer::setBuffer: mediaData is NULL" << llendl;
		// NOTE: This case doesn't clean up properly.  This assert is fatal, so this isn't a huge problem,
		// but if this assert is ever removed the code should be fixed to clean up correctly.
		return FALSE;
	}
	
	// [..]

	// Delete the old media data buffer iff we owned it.
	if ( ownedMediaData )
	{
		if ( oldMediaData )
		{
			delete [] oldMediaData;
		}
	}
	
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//
BOOL
LLMediaImplGStreamer::
init ()
{
	static bool done_init = false;
	if (!done_init)
	{
		// Get symbols!
		if (! grab_gst_syms("libgstreamer-0.10.so.0",
				    "libgstvideo-0.10.so.0",
				    "libgstaudio-0.10.so.0") )
		{
			llwarns << "Couldn't find suitable GStreamer 0.10 support on this system - video playback disabled." << llendl;
			return FALSE;
		}

		if (llgst_segtrap_set_enabled)
			llgst_segtrap_set_enabled(FALSE);
		else
			llwarns << "gst_segtrap_set_enabled() is not available; Second Life automated crash-reporter may cease to function until next restart." << llendl;

		if (0 == llgst_init_check(NULL, NULL, NULL))
		{
			llwarns << "GST init failed for unspecified reason." << llendl;
			return FALSE;
		}
		
		// Init our custom plugins - only really need do this once.
		gst_slvideo_init_class();
#if 0
		gst_slsound_init_class();
#endif

		done_init = true;
	}

	// Create a pumpable main-loop for this media
	mPump = g_main_loop_new (NULL, FALSE);
	if (!mPump)
	{
		return FALSE;
	}

	// instantiate a playbin element to do the hard work
	mPlaybin = llgst_element_factory_make ("playbin", "play");
	if (!mPlaybin)
	{
		// todo: cleanup pump
		return FALSE;
	}

	if (NULL == getenv("LL_GSTREAMER_EXTERNAL")) {
		// instantiate and connect a custom video sink
		mVideoSink =
			GST_SLVIDEO(llgst_element_factory_make ("private-slvideo", "slvideo"));
		if (!mVideoSink)
		{
			llwarns << "Could not instantiate private-slvideo element."
				<< llendl;
			// todo: cleanup.
			return FALSE;
		}

		g_object_set(mPlaybin, "video-sink", mVideoSink, NULL);

#ifdef LL_GST_SOUNDSINK
		// instantiate and connect a custom audio sink
		mAudioSink =
			GST_SLSOUND(llgst_element_factory_make ("private-slsound", "slsound"));
		if (!mAudioSink)
		{
			llwarns << "Could not instantiate private-slsound element."
				<< llendl;
			// todo: cleanup.
			return FALSE;
		}

		g_object_set(mPlaybin, "audio-sink", mAudioSink, NULL);
#endif
	}

	return LLMediaMovieBase::init();
}


///////////////////////////////////////////////////////////////////////////////
//
//#define LL_GST_REPORT_STATE_CHANGES
#ifdef LL_GST_REPORT_STATE_CHANGES
static char* get_gst_state_name(GstState state)
{
	switch (state) {
	case GST_STATE_VOID_PENDING: return "VOID_PENDING";
	case GST_STATE_NULL: return "NULL";
	case GST_STATE_READY: return "READY";
	case GST_STATE_PAUSED: return "PAUSED";
	case GST_STATE_PLAYING: return "PLAYING";
	}
	return "(unknown)";
}
#endif // LL_GST_REPORT_STATE_CHANGES

static gboolean
my_bus_callback (GstBus     *bus,
		 GstMessage *message,
		 gpointer    data)
{
	if (GST_MESSAGE_TYPE(message) != GST_MESSAGE_STATE_CHANGED &&
	    GST_MESSAGE_TYPE(message) != GST_MESSAGE_BUFFERING)
	{
		llinfos << "Got GST message type: "
			<< LLGST_MESSAGE_TYPE_NAME (message)
			<< llendl;
	}
	else
	{
		lldebugs << "Got GST message type: "
			 << LLGST_MESSAGE_TYPE_NAME (message)
			 << llendl;
	}

	LLMediaImplGStreamer *impl = (LLMediaImplGStreamer*)data;

	switch (GST_MESSAGE_TYPE (message)) {
	case GST_MESSAGE_BUFFERING: {
		// NEEDS GST 0.10.11+
		if (llgst_message_parse_buffering)
		{
			gint percent = 0;
			llgst_message_parse_buffering(message, &percent);
			llinfos << "GST buffering: " << percent
				<< "%" << llendl;
			// ModeBuffering seems to do nothing except make
			// the UI worse
			/*if (percent < 100) impl->setCurrentMode(LLMediaImplGStreamer::ModeBuffering);*/
		}
		break;
	}
	case GST_MESSAGE_STATE_CHANGED: {
		GstState old_state;
		GstState new_state;
		GstState pending_state;
		llgst_message_parse_state_changed(message,
						&old_state,
						&new_state,
						&pending_state);
#ifdef LL_GST_REPORT_STATE_CHANGES
		// not generally very useful, and rather spammy.
		llinfos << "state change (old,<new>,pending): "
			<< get_gst_state_name(old_state) << ", <"
			<< get_gst_state_name(new_state) << ">, "
			<< get_gst_state_name(pending_state) <<
			llendl;
#endif // LL_GST_REPORT_STATE_CHANGES

		switch (new_state) {
		case GST_STATE_VOID_PENDING:
			impl->setCurrentMode(LLMediaImplGStreamer::ModeNone);
			break;
		case GST_STATE_NULL:
			impl->setCurrentMode(LLMediaImplGStreamer::ModeNone);
			break;
		case GST_STATE_READY:
			impl->setCurrentMode(LLMediaImplGStreamer::ModeStopped);
			break;
		case GST_STATE_PAUSED:
			impl->setCurrentMode(LLMediaImplGStreamer::ModePaused);
			break;
		case GST_STATE_PLAYING:
			impl->setCurrentMode(LLMediaImplGStreamer::ModePlaying);
			break;
		}
		break;
	}
	case GST_MESSAGE_ERROR: {
		GError *err;
		gchar *debug;

		llgst_message_parse_error (message, &err, &debug);
		llinfos << "GST error: " << err->message << llendl;
		g_error_free (err);
		g_free (debug);

		impl->setCurrentMode(LLMediaImplGStreamer::ModeError);

		impl->stop();

		break;
	}
	case GST_MESSAGE_INFO: {
		if (llgst_message_parse_info)
		{
			GError *err;
			gchar *debug;
			
			llgst_message_parse_info (message, &err, &debug);
			llinfos << "GST info: " << err->message << llendl;
			g_error_free (err);
			g_free (debug);
		}
		break;
	}
	case GST_MESSAGE_WARNING: {
		GError *err;
		gchar *debug;

		llgst_message_parse_warning (message, &err, &debug);
		llinfos << "GST warning: " << err->message << llendl;
		g_error_free (err);
		g_free (debug);

		break;
	}
	case GST_MESSAGE_EOS:
		/* end-of-stream */
		llinfos << "GST EOS." << llendl;
		impl->setCurrentMode(LLMediaImplGStreamer::ModeStopped);//?
		impl->stop();
		break;
	default:
		/* unhandled message */
		break;
	}

	/* we want to be notified again the next time there is a message
	 * on the bus, so returning TRUE (FALSE means we want to stop watching
	 * for messages on the bus and our callback should not be called again)
	 */
	return TRUE;
}

BOOL
LLMediaImplGStreamer::
load ( const LLString& urlIn )
{
	llinfos << "Setting media URI: " << urlIn << llendl;

	// set URI
	g_object_set (G_OBJECT (mPlaybin), "uri", urlIn.c_str(), NULL);
	//g_object_set (G_OBJECT (mPlaybin), "uri", "file:///tmp/movie", NULL);

	// get playbin's bus - perhaps this can/should be done at init()
	GstBus *bus = llgst_pipeline_get_bus (GST_PIPELINE (mPlaybin));
	if (!bus)
	{
		return FALSE;
	}
	llgst_bus_add_watch (bus, my_bus_callback, this);
	llgst_object_unref (bus);

	if (true) // dummy values
	{
		const int fixedsize = 2;
		mMediaRowbytes = mMediaDepthBytes * fixedsize;
		mMediaWidth = fixedsize;
		mMediaHeight = fixedsize;
		mTextureWidth = fixedsize;
		mTextureHeight = fixedsize;
	}

	BOOL rtn = LLMediaMovieBase::load(urlIn);
	llinfos << "load returns " << int(rtn) << llendl;
	return rtn;
}

///////////////////////////////////////////////////////////////////////////////
//
BOOL
LLMediaImplGStreamer::
unload ()
{
	if (mPlaybin)
	{
		llgst_element_set_state (mPlaybin, GST_STATE_NULL);
		llgst_object_unref (GST_OBJECT (mPlaybin));
		mPlaybin = NULL;
	}

	if (mPump)
	{
		g_main_loop_quit(mPump);
		mPump = NULL;
	}

	if (mediaData)
	{
		if (ownBuffer)
		{
			delete mediaData;
			mediaData = NULL;
		}
	}

	mVideoSink = NULL;

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//
S32
LLMediaImplGStreamer::
updateMedia ()
{
	//llinfos << "updating media..." << llendl;
	if (g_main_context_pending(g_main_loop_get_context(mPump)))
	{
	       g_main_context_iteration(g_main_loop_get_context(mPump), FALSE);
	}

	if (mVideoSink)
	{
	        GST_OBJECT_LOCK(mVideoSink);
		if (mVideoSink->retained_frame_ready)
		{
			//llinfos << "NEW FRAME " << llendl;
			if (mVideoSink->retained_frame_width != mMediaWidth ||
			    mVideoSink->retained_frame_height != mMediaHeight)
				// *TODO: also check for change in format
			{
				// just resize container
				mMediaWidth = mVideoSink->retained_frame_width;
				mMediaHeight = mVideoSink->retained_frame_height;
				mTextureWidth = mMediaWidth;
				mTextureHeight = mMediaHeight;
				mMediaDepthBytes = mTextureDepth =
					SLVPixelFormatBytes[mVideoSink->retained_frame_format];
				if (SLV_PF_RGBX == mVideoSink->retained_frame_format)
				{
					mTextureFormatPrimary = GL_RGBA;
					mTextureFormatType=GL_UNSIGNED_INT_8_8_8_8_REV;
				}
				else
				{
					mTextureFormatPrimary = GL_BGRA;
					mTextureFormatType=GL_UNSIGNED_INT_8_8_8_8_REV;
				}
				mMediaRowbytes = mMediaWidth * mMediaDepthBytes;
				llinfos << "video container resized to " <<
					mMediaWidth << "x" << mMediaHeight << llendl;
				
				if (ownBuffer)
				{
					// we manage the buffer, so we need to realloc
					delete[] mediaData;
					mediaData = new U8[mMediaRowbytes *
							   mMediaHeight];
				}
				
				GST_OBJECT_UNLOCK(mVideoSink);
				return updateMediaNeedsSizeChange;
			}

			// we're gonna totally consume this frame - reset 'ready' flag
			mVideoSink->retained_frame_ready = FALSE;
			memcpy(mediaData, mVideoSink->retained_frame_data,
			       mMediaRowbytes * mMediaHeight);
			
			GST_OBJECT_UNLOCK(mVideoSink);
			return updateMediaNeedsUpdate;
		}
		else
		{
			// nothing to do yet.
			GST_OBJECT_UNLOCK(mVideoSink);
			return updateMediaNoChanges;
		}
	}

	return updateMediaNoChanges;
}

///////////////////////////////////////////////////////////////////////////////
//
void
LLMediaImplGStreamer::
setAutoScaled ( BOOL autoScaledIn )
{
	autoScaled = autoScaledIn;
}

///////////////////////////////////////////////////////////////////////////////
//
BOOL
LLMediaImplGStreamer::
stop ()
{
	llinfos << "stopping media..." << llendl;
	// todo: error-check this?
	llgst_element_set_state(mPlaybin, GST_STATE_READY);

	BOOL rtn = LLMediaMovieBase::stop();
	setCurrentMode(LLMediaImplGStreamer::ModeStopped);//?
	return rtn;
}

///////////////////////////////////////////////////////////////////////////////
//
BOOL
LLMediaImplGStreamer::
play ()
{
	llinfos << "playing media..." << llendl;
	// todo: error-check this?
	llgst_element_set_state(mPlaybin, GST_STATE_PLAYING);

	return LLMediaMovieBase::play();
}

///////////////////////////////////////////////////////////////////////////////
//
BOOL
LLMediaImplGStreamer::
loop ( S32 howMany )
{
	llinfos << "looping media... " << howMany << llendl;
	// todo: implement this
	if (!play())
		return FALSE;

	return LLMediaMovieBase::loop(howMany);
};

///////////////////////////////////////////////////////////////////////////////
//
BOOL
LLMediaImplGStreamer::
pause ()
{
	llinfos << "pausing media..." << llendl;
	// todo: error-check this?
	llgst_element_set_state(mPlaybin, GST_STATE_PAUSED);

	return LLMediaMovieBase::pause();
};

///////////////////////////////////////////////////////////////////////////////
//
BOOL
LLMediaImplGStreamer::
setVolume ( F32 volumeIn )
{
	mVolume = volumeIn;
	g_object_set(mPlaybin, "volume", mVolume, NULL);
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//
F32
LLMediaImplGStreamer::
getVolume ()
{
	return mVolume;
}

///////////////////////////////////////////////////////////////////////////////
//
BOOL
LLMediaImplGStreamer::
isIdle () const
{
	// todo: probably semantically decouple from currentMode
	return currentMode == ModeIdle;
}

///////////////////////////////////////////////////////////////////////////////
//
BOOL
LLMediaImplGStreamer::
isError () const
{
	// todo: probably semantically decouple from currentMode
	return currentMode == ModeError;
}

///////////////////////////////////////////////////////////////////////////////
//
BOOL
LLMediaImplGStreamer::
isBuffering () const
{
	// todo: probably semantically decouple from currentMode
	return currentMode == ModeBuffering;
}

///////////////////////////////////////////////////////////////////////////////
//
BOOL
LLMediaImplGStreamer::
isLoaded () const
{
	// todo: probably semantically decouple from currentMode
	//return currentMode == ModeLoaded;
	return (mPump != NULL);
}

///////////////////////////////////////////////////////////////////////////////
//
BOOL
LLMediaImplGStreamer::
isPlaying () const
{
	// todo: probably semantically decouple from currentMode
	return currentMode == ModePlaying;
}

///////////////////////////////////////////////////////////////////////////////
//
BOOL
LLMediaImplGStreamer::
isLooping () const
{
	// todo: probably semantically decouple from currentMode
	return currentMode == ModeLooping;
}

///////////////////////////////////////////////////////////////////////////////
//
BOOL
LLMediaImplGStreamer::
isPaused () const
{
	// todo: probably semantically decouple from currentMode
	return currentMode == ModePaused;
}

///////////////////////////////////////////////////////////////////////////////
//
BOOL
LLMediaImplGStreamer::
isStopped () const
{
	// todo: probably semantically decouple from currentMode
	return currentMode == ModeStopped;
}

///////////////////////////////////////////////////////////////////////////////
//
U8*
LLMediaImplGStreamer::
getMediaData ()
{
	return mediaData;
}

///////////////////////////////////////////////////////////////////////////////
//
BOOL 
LLMediaImplGStreamer::
seek ( F64 time )
{
	// todo: implement this
	llinfos << "Tried to seek to time " << time
		<< " - faking it" << llendl;
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//
F64 
LLMediaImplGStreamer::
getTime () const
{
	// todo: implement this
	F64 result = 0;	
	return result;
}

///////////////////////////////////////////////////////////////////////////////
//
F64 
LLMediaImplGStreamer::
getMediaDuration () const
{
	// todo: implement this
	F64 result = 0;
	return result;
}

#endif // LL_GSTREAMER_ENABLED
