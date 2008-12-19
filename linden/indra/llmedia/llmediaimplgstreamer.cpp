/** 
 * @file llmediaimplgstreamer.cpp
 * @author Tofu Linden
 * @brief implementation that supports various media through GStreamer.
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

#include "llmediaimplgstreamer.h"

#if LL_GSTREAMER_ENABLED

extern "C" {
#include <gst/gst.h>
}

#include "llmediamanager.h"
#include "llmediaimplregister.h"

#include "llmediaimplgstreamervidplug.h"

#ifdef LL_GST_SOUNDSINK
#include "llmediaimplgstreamersndplug.h"
#endif // LL_GST_SOUNDSINK

#include "llmediaimplgstreamer_syms.h"

#include "llerror.h"
// register this impl with media manager factory
static LLMediaImplRegister sLLMediaImplGStreamerReg( "LLMediaImplGStreamer", new LLMediaImplGStreamerMaker() );

LLMediaImplGStreamerMaker::LLMediaImplGStreamerMaker()
{
	// Register to handle the scheme
	mSchema.push_back( "rtsp" );
	mSchema.push_back( "rtmp" );
	
	// Register to handle the category
	mMimeTypeCategories.push_back( "video" );
	mMimeTypeCategories.push_back( "audio" );
}

///////////////////////////////////////////////////////////////////////////////
//
LLMediaImplGStreamer::
LLMediaImplGStreamer () :
	mediaData ( NULL ),
	mMediaRowbytes ( 1 ),
	mTextureFormatPrimary ( LL_MEDIA_BGRA ),
	mTextureFormatType ( LL_MEDIA_UNSIGNED_INT_8_8_8_8_REV ),
	mPump ( NULL ),
	mPlaybin ( NULL ),
	mVideoSink ( NULL ),
        mState( GST_STATE_NULL )
#ifdef LL_GST_SOUNDSINK
	,mAudioSink ( NULL )
#endif // LL_GST_SOUNDSINK
{
	LL_DEBUGS("MediaManager") << "constructing media..." << LL_ENDL;
	mVolume = -1.0; // XXX Hack to make the vould change happend first time

	setMediaDepth(4);

	// Create a pumpable main-loop for this media
	mPump = g_main_loop_new (NULL, FALSE);
	if (!mPump)
	{
		return; // error
	}

	// instantiate a playbin element to do the hard work
	mPlaybin = llgst_element_factory_make ("playbin", "play");
	if (!mPlaybin)
	{
		// todo: cleanup pump
		return; // error
	}

	if (NULL == getenv("LL_GSTREAMER_EXTERNAL")) {
		// instantiate and connect a custom video sink
		LL_DEBUGS("MediaManager") << "extrenal video sink..." << LL_ENDL;
		mVideoSink =
			GST_SLVIDEO(llgst_element_factory_make ("private-slvideo", "slvideo"));
		if (!mVideoSink)
		{
			LL_WARNS("MediaImpl") << "Could not instantiate private-slvideo element." << LL_ENDL;
			// todo: cleanup.
			return; // error
		}

		g_object_set(mPlaybin, "video-sink", mVideoSink, NULL);

#ifdef LL_GST_SOUNDSINK
		LL_DEBUGS("MediaManager") << "extrenal audio sink..." << LL_ENDL;
		// instantiate and connect a custom audio sink
		mAudioSink =
			GST_SLSOUND(llgst_element_factory_make ("private-slsound", "slsound"));
		if (!mAudioSink)
		{
			LL_WARN("MediaImpl") << "Could not instantiate private-slsound element." << LL_ENDL;
			// todo: cleanup.
			return; // error
		}

		g_object_set(mPlaybin, "audio-sink", mAudioSink, NULL);
#endif
	}
}

// virtual
int LLMediaImplGStreamer::getTextureFormatPrimary() const
{
	return mTextureFormatPrimary;
}

// virtual
int LLMediaImplGStreamer::getTextureFormatType() const
{
	return mTextureFormatType;
}

// virtual
int LLMediaImplGStreamer::getTextureFormatInternal() const
{
	return LL_MEDIA_RGB8;
}

///////////////////////////////////////////////////////////////////////////////
//
LLMediaImplGStreamer::
~LLMediaImplGStreamer ()
{
	LL_DEBUGS("MediaImpl") << ("dtor of media...") << LL_ENDL;
	unload();
}

////////////////////////////////////////////////////////////////////////////////
// virtual
std::string LLMediaImplGStreamer::getVersion()
{
	std::string rtn;
	rtn = "[" + sLLMediaImplGStreamerReg.getImplName() + "] - GStreamer 0.10.x";
	return rtn;
}

///////////////////////////////////////////////////////////////////////////////
// (static) super-initialization - called once at application startup
bool
LLMediaImplGStreamer::
startup ( LLMediaManagerData* init_data )
{
	static bool done_init = false;
	if (!done_init)
	{
		// Get symbols!
		if (! grab_gst_syms("libgstreamer-0.10.so.0",
				    "libgstvideo-0.10.so.0",
				    "libgstaudio-0.10.so.0") )
		{
		    	LL_WARNS("MediaImpl") << "Couldn't find suitable GStreamer 0.10 support on this system - video playback disabled." << LL_ENDL;
			return false;
		}

		if (llgst_segtrap_set_enabled)
			llgst_segtrap_set_enabled(FALSE);
		else
		{
		    LL_WARNS("MediaImpl") << "gst_segtrap_set_enabled() is not available; Automated crash-reporter may cease to function until next restart." << LL_ENDL;
		}

		// Protect against GStreamer resetting the locale, yuck.
		static std::string saved_locale;
		saved_locale = setlocale(LC_ALL, NULL);
		if (0 == llgst_init_check(NULL, NULL, NULL))
		{
		    LL_WARNS("MediaImpl") << "GST init failed for unspecified reason." << LL_ENDL;
			setlocale(LC_ALL, saved_locale.c_str() );
			return false;
		}
		setlocale(LC_ALL, saved_locale.c_str() );
		
		// Init our custom plugins - only really need do this once.
		gst_slvideo_init_class();
#if 0
		gst_slsound_init_class();
#endif

		done_init = true;
	}

	return true;
}


bool LLMediaImplGStreamer::
closedown()
{
	ungrab_gst_syms();

	return true;
}


///////////////////////////////////////////////////////////////////////////////
//
//#define LL_GST_REPORT_STATE_CHANGES
#ifdef LL_GST_REPORT_STATE_CHANGES
static const char* get_gst_state_name(GstState state)
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

//static
gboolean
LLMediaImplGStreamer::bus_callback (GstBus     *bus,
				    GstMessage *message,
				    gpointer    data)
{
	LL_DEBUGS("MediaCallback") << "Got GST message type: " << LLGST_MESSAGE_TYPE_NAME (message) << LL_ENDL;

	LLMediaImplGStreamer *impl = (LLMediaImplGStreamer*)data;

	switch (GST_MESSAGE_TYPE (message)) {
	case GST_MESSAGE_BUFFERING: {
		// NEEDS GST 0.10.11+
		if (llgst_message_parse_buffering)
		{
			gint percent = 0;
			llgst_message_parse_buffering(message, &percent);
			LL_DEBUGS("MediaBuffering") << "GST buffering: " << percent << "%%" << LL_ENDL;
			LLMediaEvent event( impl, percent );
			impl->getEventEmitter().update( &LLMediaObserver::onUpdateProgress, event );

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
		LL_DEBUGS("MediaState") << "GST state change (old,<new>,pending): "<< get_gst_state_name(old_state) << ",<" << get_gst_state_name(new_state) << ">," << get_gst_state_name(pending_state) << LL_ENDL;
#endif // LL_GST_REPORT_STATE_CHANGES

		switch (new_state) {
		case GST_STATE_VOID_PENDING:
			break;
		case GST_STATE_NULL:
			LL_DEBUGS("MediaImpl") << "State changed to NULL" << LL_ENDL;
			if (impl->getState() == GST_STATE_PLAYING) { // We got stoped by gstremer...
			    impl->play();
			    LL_DEBUGS("MediaImpl") << "Trying to restart." << LL_ENDL;
			}
			break;
		case GST_STATE_READY:
			break;
		case GST_STATE_PAUSED:
			break;
		case GST_STATE_PLAYING:
			LLMediaEvent event( impl, 100 );
			impl->getEventEmitter().update( &LLMediaObserver::onUpdateProgress, event );
			// emit an event to say that a media source was loaded
			LLMediaEvent event2( impl );
			impl->getEventEmitter().update( &LLMediaObserver::onMediaLoaded, event2 );
			break;
		}
		break;
	}
	case GST_MESSAGE_ERROR: {
		GError *err = NULL;
		gchar *debug = NULL;

		llgst_message_parse_error (message, &err, &debug);
		LL_WARNS("MediaImpl") << "GST Error: " << err->message << LL_ENDL;
		g_error_free (err);
		g_free (debug);

		impl->addCommand(LLMediaBase::COMMAND_STOP);
		//impl->addCommand(LLMediaBase::COMMAND_START);

		break;
	}
	case GST_MESSAGE_INFO: {
		if (llgst_message_parse_info)
		{
			GError *err = NULL;
			gchar *debug = NULL;
			
			llgst_message_parse_info (message, &err, &debug);
			LL_INFOS("MediaImpl") << "GST info: " << err->message
			    << LL_ENDL;
			g_error_free (err);
			g_free (debug);
		}
		break;
	}
	case GST_MESSAGE_WARNING: {
		GError *err = NULL;
		gchar *debug = NULL;

		llgst_message_parse_warning (message, &err, &debug);
		LL_WARNS("MediaImpl") << "GST warning: " <<  err->message
		    << LL_ENDL;
		g_error_free (err);
		g_free (debug);

		break;
	}
	case GST_MESSAGE_TAG: {
#if 0
	        GstTagList *tag_list;
		gchar *title;
		gchar *artist;
		llgst_message_parse_tag(message, &tag_list);
		gboolean hazTitle = llgst_tag_list_get_string(tag_list,
			GST_TAG_TITLE, &title);
		gboolean hazArtist = llgst_tag_list_get_string(tag_list,
			GST_TAG_ARTIST, &artist);
		if(hazTitle) 
		    LL_INFOS("MediaInfo") << "Title is " << title << LL_ENDL;
		if(hazArtist) 
		    LL_INFOS("MediaInfo") << "Artist is " << artist << LL_ENDL;
#endif
		break;
	}
	case GST_MESSAGE_EOS:
		/* end-of-stream */
		LL_DEBUGS("MediaImpl") << "GST end-of-stream." << LL_ENDL;
		if (impl->isLooping())
		{
			LL_DEBUGS("MediaImpl") << "looping media..." << LL_ENDL;
			impl->stop();
			impl->play();
		}
		else
		{
			// inject a COMMAND_STOP
			impl->addCommand(LLMediaBase::COMMAND_STOP);
		}
		break;
	default:
		/* unhandled message */
		break;
	}

	/* we want to be notified again the next time there is a message
	 * on the bus, so return true (false means we want to stop watching
	 * for messages on the bus and our callback should not be called again)
	 */
	return TRUE;
}

///////////////////////////////////////////////////////////
// virtual
bool
LLMediaImplGStreamer::
navigateTo ( const std::string urlIn )
{
	LL_DEBUGS("MediaImpl") << "Setting media URI: " << urlIn.c_str()
	    << LL_ENDL;

	if (NULL == mPump
#ifdef LL_GST_SOUNDSINK
	    || NULL == mAudioSink
#endif
	    || NULL == mPlaybin)
	{
		return false;
	}

	setStatus( LLMediaBase::STATUS_NAVIGATING );

	// set URI
	g_object_set (G_OBJECT (mPlaybin), "uri", urlIn.c_str(), NULL);
	//g_object_set (G_OBJECT (mPlaybin), "uri", "file:///tmp/movie", NULL);

	// get playbin's bus - perhaps this can/should be done in ctor
	GstBus *bus = llgst_pipeline_get_bus (GST_PIPELINE (mPlaybin));
	if (!bus)
	{
		return false;
	}
	llgst_bus_add_watch (bus, bus_callback, this);
	llgst_object_unref (bus);

	// navigateTo implicitly plays, too.
	play();

	return true;
}

///////////////////////////////////////////////////////////////////////////////
//
bool
LLMediaImplGStreamer::
unload ()
{
	LL_DEBUGS("MediaImpl") << "unloading media..." << LL_ENDL;
	if (mPlaybin)
	{
		llgst_element_set_state (mPlaybin, GST_STATE_NULL);
		mState = GST_STATE_NULL;
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
		delete [] mediaData;
		mediaData = NULL;
	}

	mVideoSink = NULL;

	return true;
}

///////////////////////////////////////////////////////////////////////////////
// virtual
bool
LLMediaImplGStreamer::
updateMedia ()
{
	//LL_DEBUGS("MediaImpl") << "updating media..." << LL_ENDL;
	
	// sanity check
	if (NULL == mPump
#ifdef LL_GST_SOUNDSINK
	    || NULL == mAudioSink
#endif
	    || NULL == mPlaybin)
	{
		LL_DEBUGS("MediaImpl") << "dead media..." << LL_ENDL;
		return false;
	}

	// process next outstanding command
	switch (nextCommand())
	{
	case LLMediaBase::COMMAND_START:
		LL_DEBUGS("MediaImpl") << "COMMAND_START" << LL_ENDL;
		if (getStatus() == LLMediaBase::STATUS_PAUSED ||
		    getStatus() == LLMediaBase::STATUS_NAVIGATING ||
		    getStatus() == LLMediaBase::STATUS_STOPPED)
		{
			play();
			setStatus(LLMediaBase::STATUS_STARTED);
			clearCommand();
		}
		break;
	case LLMediaBase::COMMAND_STOP:
		LL_DEBUGS("MediaImpl") << "COMMAND_STOP" << LL_ENDL;
		stop();
		setStatus(LLMediaBase::STATUS_STOPPED);
		clearCommand();
		break;
	case LLMediaBase::COMMAND_PAUSE:
		LL_DEBUGS("MediaImpl") << "COMMAND_PAUSE" << LL_ENDL;
		if (getStatus() == LLMediaBase::STATUS_STARTED)
		{
			pause();
			setStatus(LLMediaBase::STATUS_PAUSED);
			clearCommand();
		}
		break;
	default:
		LL_INFOS("MediaImpl") << "Unknown command" << LL_ENDL;
		clearCommand();
		break;
	case LLMediaBase::COMMAND_NONE:
		break;
	}

	// deal with results
	if (g_main_context_pending(g_main_loop_get_context(mPump)))
	{
	       g_main_context_iteration(g_main_loop_get_context(mPump), FALSE);
	}

	if (mVideoSink)
	{
	        GST_OBJECT_LOCK(mVideoSink);
		if (mVideoSink->retained_frame_ready)
		{
			LL_DEBUGS("MediaImpl") <<"NEW FRAME " << LL_ENDL;
			if (mVideoSink->retained_frame_width != getMediaWidth() ||
			    mVideoSink->retained_frame_height != getMediaHeight())
				// *TODO: also check for change in format
			{
				// just resize containe
				int neww = mVideoSink->retained_frame_width;
				int newh = mVideoSink->retained_frame_height;
				int newd = SLVPixelFormatBytes[mVideoSink->retained_frame_format];
				if (SLV_PF_RGBX == mVideoSink->retained_frame_format)
				{
					mTextureFormatPrimary = LL_MEDIA_RGBA;
					mTextureFormatType = LL_MEDIA_UNSIGNED_INT_8_8_8_8_REV;
				}
				else
				{
					mTextureFormatPrimary = LL_MEDIA_BGRA;
					mTextureFormatType = LL_MEDIA_UNSIGNED_INT_8_8_8_8_REV;
				}
				mMediaRowbytes = neww * newd;
				LL_DEBUGS("MediaImpl")
				    << "video container resized to " <<
				    neww <<"x"<< newh << LL_ENDL;
				
				delete[] mediaData;
				mediaData = new unsigned char[mMediaRowbytes *
							      newh];
				
				GST_OBJECT_UNLOCK(mVideoSink);

				setMediaDepth(newd);
				setMediaSize(neww, newh);
				return true;
			}

			// we're gonna totally consume this frame - reset 'ready' flag
			mVideoSink->retained_frame_ready = FALSE;
			memcpy(mediaData, mVideoSink->retained_frame_data,
			       mMediaRowbytes * getMediaHeight());
			
			GST_OBJECT_UNLOCK(mVideoSink);
			LLMediaEvent event( this );
			mEventEmitter.update( &LLMediaObserver::onMediaContentsChange, event );
			return true;
		}
		else
		{
			// nothing to do yet.
			GST_OBJECT_UNLOCK(mVideoSink);
			return true;
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
//
bool
LLMediaImplGStreamer::
stop ()
{
	LL_DEBUGS("MediaImpl") << "stopping media..." << LL_ENDL;
	// todo: error-check this?
	llgst_element_set_state(mPlaybin, GST_STATE_READY);
	mState = GST_STATE_READY;
	return true;
}

///////////////////////////////////////////////////////////////////////////////
//
bool
LLMediaImplGStreamer::
play ()
{
	LL_DEBUGS("MediaImpl") << "playing media..." << LL_ENDL;
	// todo: error-check this?
	llgst_element_set_state(mPlaybin, GST_STATE_PLAYING);
	mState = GST_STATE_PLAYING;
	return true;
}

///////////////////////////////////////////////////////////////////////////////
//
bool
LLMediaImplGStreamer::
pause ()
{
	LL_DEBUGS("MediaImpl") <<"pausing media..." << LL_ENDL;
	// todo: error-check this?
	llgst_element_set_state(mPlaybin, GST_STATE_PAUSED);
	mState = GST_STATE_PAUSED;
	return true;
};


///////////////////////////////////////////////////////////////////////////////
// virtual
unsigned char*
LLMediaImplGStreamer::
getMediaData ()
{
	return mediaData;
}


///////////////////////////////////////////////////////////////////////////////
// virtual
bool
LLMediaImplGStreamer::
seek( double time )
{
	bool success = false;
	if (mPlaybin)
	{
		success = llgst_element_seek(mPlaybin, 1.0F, GST_FORMAT_TIME,
				GstSeekFlags(GST_SEEK_FLAG_FLUSH |
					     GST_SEEK_FLAG_KEY_UNIT),
				GST_SEEK_TYPE_SET, gint64(time*1000000000.0F),
				GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE);
	}
	LL_DEBUGS("MediaImpl") << "MEDIA SEEK REQUEST to " << float(time)
	    << "sec result was " << int(success) << LL_ENDL;
	return success;
}


///////////////////////////////////////////////////////////////////////////////
// virtual
bool
LLMediaImplGStreamer::
setVolume(float volume)
{
        // XXX hack to make volume volume changes less othen
	//     bug in gstreamer 0.10.21
	if(mVolume == volume)
	    return true;

        LL_DEBUGS("MediaImpl") << "setVolume(" << volume << ") : " << getpid() << LL_ENDL;
	mVolume = volume;
	if (mPlaybin)
	{
		g_object_set(mPlaybin, "volume", mVolume, NULL);
		return true;
	}
	return false;
}

#endif // LL_GSTREAMER_ENABLED
