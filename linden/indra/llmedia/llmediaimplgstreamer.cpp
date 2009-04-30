/** 
 * @file llmediaimplgstreamer.cpp
 * @author Tofu Linden
 * @brief implementation that supports various media through GStreamer.
 *
 * $LicenseInfo:firstyear=2007&license=viewergpl$
 * 
 * Copyright (c) 2007-2009, Linden Research, Inc.
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

#include "llmediaimplgstreamer.h"

#if LL_GSTREAMER_ENABLED

extern "C" {
#include <gst/gst.h>
}

#include "llmediamanager.h"
#include "llmediaimplregister.h"

#include "llmediaimplgstreamervidplug.h"

#include "llmediaimplgstreamer_syms.h"

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
	mBusWatchID ( 0 ),
	mediaData ( NULL ),
	mMediaRowbytes ( 1 ),
	mTextureFormatPrimary ( LL_MEDIA_BGRA ),
	mTextureFormatType ( LL_MEDIA_UNSIGNED_INT_8_8_8_8_REV ),
	mPump ( NULL ),
	mPlaybin ( NULL ),
	mVideoSink ( NULL )
{
	if (!mDoneInit)
		return; // error

	DEBUGMSG("constructing media...");

	mVolume = 0.1234567; // minor hack to force an initial volume update

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
		return; // error
	}

	// get playbin's bus
	GstBus *bus = llgst_pipeline_get_bus (GST_PIPELINE (mPlaybin));
	if (!bus)
	{
		return; // error
	}
	mBusWatchID = llgst_bus_add_watch (bus,
					   llmediaimplgstreamer_bus_callback,
					   this);
	llgst_object_unref (bus);

	if (NULL == getenv("LL_GSTREAMER_EXTERNAL")) {
		// instantiate and connect a custom video sink
		mVideoSink =
			GST_SLVIDEO(llgst_element_factory_make ("private-slvideo", "slvideo"));
		if (!mVideoSink)
		{
			WARNMSG("Could not instantiate private-slvideo element.");
			// todo: cleanup.
			return; // error
		}

		g_object_set(mPlaybin, "video-sink", mVideoSink, NULL);
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
	DEBUGMSG("dtor of media...");
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

//static
bool LLMediaImplGStreamer::mDoneInit = false;

//static
bool
LLMediaImplGStreamer::
startup ( LLMediaManagerData* init_data )
{
	// first - check if GStreamer is explicitly disabled
	if (NULL != getenv("LL_DISABLE_GSTREAMER"))
		return false;

	// only do global GStreamer initialization once.
	if (!mDoneInit)
	{
		// Init the glib type system - we need it.
		g_type_init();

		// Get symbols!
		if (! grab_gst_syms("libgstreamer-0.10.so.0",
				    "libgstvideo-0.10.so.0") )
		{
			WARNMSG("Couldn't find suitable GStreamer 0.10 support on this system - video playback disabled.");
			return false;
		}

		if (llgst_segtrap_set_enabled)
		{
			llgst_segtrap_set_enabled(FALSE);
		}
		else
		{
			WARNMSG("gst_segtrap_set_enabled() is not available; Automated crash-reporter may cease to function until next restart.");
		}

#if LL_LINUX
		// Gstreamer tries a fork during init, waitpid-ing on it,
		// which conflicts with any installed SIGCHLD handler...
		struct sigaction tmpact, oldact;
		if (llgst_registry_fork_set_enabled) {
			// if we can disable SIGCHLD-using forking behaviour,
			// do it.
			llgst_registry_fork_set_enabled(false);
		}
		else {
			// else temporarily install default SIGCHLD handler
			// while GStreamer initialises
			tmpact.sa_handler = SIG_DFL;
			sigemptyset( &tmpact.sa_mask );
			tmpact.sa_flags = SA_SIGINFO;
			sigaction(SIGCHLD, &tmpact, &oldact);
		}
#endif // LL_LINUX

		// Protect against GStreamer resetting the locale, yuck.
		static std::string saved_locale;
		saved_locale = setlocale(LC_ALL, NULL);

		// finally, try to initialize GStreamer!
		GError *err = NULL;
		gboolean init_gst_success = llgst_init_check(NULL, NULL, &err);

		// restore old locale
		setlocale(LC_ALL, saved_locale.c_str() );

#if LL_LINUX
		// restore old SIGCHLD handler
		if (!llgst_registry_fork_set_enabled)
			sigaction(SIGCHLD, &oldact, NULL);
#endif // LL_LINUX

		if (!init_gst_success) // fail
		{
			if (err)
			{
				WARNMSG("GST init failed: %s", err->message);
				g_error_free(err);
			}
			else
			{
				WARNMSG("GST init failed for unspecified reason.");
			}
			return false;
		}
		
		// Init our custom plugins - only really need do this once.
		gst_slvideo_init_class();

		mDoneInit = true;
	}

	return true;
}


bool LLMediaImplGStreamer::
closedown()
{
	if (!mDoneInit)
		return false; // error

	ungrab_gst_syms();

	return true;
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

extern "C" {
gboolean
llmediaimplgstreamer_bus_callback (GstBus     *bus,
				   GstMessage *message,
				   gpointer    data)
{
	if (!message) 
		return TRUE; // shield against GStreamer bug

	if (GST_MESSAGE_TYPE(message) != GST_MESSAGE_STATE_CHANGED &&
	    GST_MESSAGE_TYPE(message) != GST_MESSAGE_BUFFERING)
	{
		DEBUGMSG("Got GST message type: %s",
			LLGST_MESSAGE_TYPE_NAME (message));
	}
	else
	{
		DEBUGMSG("Got GST message type: %s",
			 LLGST_MESSAGE_TYPE_NAME (message));
	}

	LLMediaImplGStreamer *impl = (LLMediaImplGStreamer*)data;

	switch (GST_MESSAGE_TYPE (message)) {
	case GST_MESSAGE_BUFFERING: {
		// NEEDS GST 0.10.11+
		if (llgst_message_parse_buffering)
		{
			gint percent = 0;
			llgst_message_parse_buffering(message, &percent);
			DEBUGMSG("GST buffering: %d%%", percent);
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
		DEBUGMSG("state change (old,<new>,pending): %s,<%s>,%s",
			 get_gst_state_name(old_state),
			 get_gst_state_name(new_state),
			 get_gst_state_name(pending_state));
#endif // LL_GST_REPORT_STATE_CHANGES

		switch (new_state) {
		case GST_STATE_VOID_PENDING:
			break;
		case GST_STATE_NULL:
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
		WARNMSG("GST error: %s", err->message);
		g_error_free (err);
		g_free (debug);

		impl->addCommand(LLMediaBase::COMMAND_STOP);

		break;
	}
	case GST_MESSAGE_INFO: {
		if (llgst_message_parse_info)
		{
			GError *err = NULL;
			gchar *debug = NULL;
			
			llgst_message_parse_info (message, &err, &debug);
			INFOMSG("GST info: %s", err->message);
			g_error_free (err);
			g_free (debug);
		}
		break;
	}
	case GST_MESSAGE_WARNING: {
		GError *err = NULL;
		gchar *debug = NULL;

		llgst_message_parse_warning (message, &err, &debug);
		WARNMSG("GST warning: %s", err->message);
		g_error_free (err);
		g_free (debug);

		break;
	}
	case GST_MESSAGE_EOS:
		/* end-of-stream */
		DEBUGMSG("GST end-of-stream.");
		if (impl->isLooping())
		{
			DEBUGMSG("looping media...");
			double eos_pos_sec = 0.0F;
			bool got_eos_position = impl->getTimePos(eos_pos_sec);

			if (got_eos_position && eos_pos_sec < impl->MIN_LOOP_SEC)
			{
				// if we know that the movie is really short, don't
				// loop it else it can easily become a time-hog
				// because of GStreamer spin-up overhead
				DEBUGMSG("really short movie (%0.3fsec) - not gonna loop this, pausing instead.", eos_pos_sec);
				// inject a COMMAND_PAUSE
				impl->addCommand(LLMediaBase::COMMAND_PAUSE);
			}
			else
			{
				// first try looping by an explicit rewind
				bool seeksuccess = impl->seek(0.0);
				if (seeksuccess)
				{
					impl->play();
				}
				else // use clumsy stop-start to loop
				{
					DEBUGMSG("couldn't loop by rewinding - stopping and starting instead...");
					impl->stop();
					impl->play();
				}
			}
		}
		else // not a looping media
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
} // extern "C"

///////////////////////////////////////////////////////////
// virtual
bool
LLMediaImplGStreamer::
navigateTo ( const std::string urlIn )
{
	if (!mDoneInit)
		return false; // error

	DEBUGMSG("Setting media URI: %s", urlIn.c_str());

	if (NULL == mPump ||
	    NULL == mPlaybin)
	{
		return false; // error
	}

	setStatus( LLMediaBase::STATUS_NAVIGATING );

	// set URI
	g_object_set (G_OBJECT (mPlaybin), "uri", urlIn.c_str(), NULL);
	//g_object_set (G_OBJECT (mPlaybin), "uri", "file:///tmp/movie", NULL);

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
	if (!mDoneInit)
		return false; // error

	DEBUGMSG("unloading media...");
	
	// stop getting callbacks for this bus
	g_source_remove(mBusWatchID);

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
	if (!mDoneInit)
		return false; // error

	DEBUGMSG("updating media...");
	
	// sanity check
	if (NULL == mPump ||
	    NULL == mPlaybin)
	{
		DEBUGMSG("dead media...");
		return false;
	}

	// process next outstanding command
	switch (nextCommand())
	{
	case LLMediaBase::COMMAND_START:
		DEBUGMSG("COMMAND_START");
		if (getStatus() == LLMediaBase::STATUS_PAUSED ||
		    getStatus() == LLMediaBase::STATUS_NAVIGATING ||
		    getStatus() == LLMediaBase::STATUS_STOPPED)
		{
			DEBUGMSG("doing COMMAND_START");
			play();
			setStatus(LLMediaBase::STATUS_STARTED);
			clearCommand();
		}
		break;
	case LLMediaBase::COMMAND_STOP:
		DEBUGMSG("COMMAND_STOP");
		DEBUGMSG("doing COMMAND_STOP");
		stop();
		setStatus(LLMediaBase::STATUS_STOPPED);
		clearCommand();
		break;
	case LLMediaBase::COMMAND_PAUSE:
		DEBUGMSG("COMMAND_PAUSE");
		if (getStatus() == LLMediaBase::STATUS_STARTED)
		{
			DEBUGMSG("doing COMMAND_PAUSE");
			pause();
			setStatus(LLMediaBase::STATUS_PAUSED);
			clearCommand();
		}
		break;
	default:
		DEBUGMSG("COMMAND_?");
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
			DEBUGMSG("NEW FRAME ");
			if (mVideoSink->retained_frame_width != getMediaWidth() ||
			    mVideoSink->retained_frame_height != getMediaHeight())
				// *TODO: also check for change in format
			{
				// just resize container
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
				DEBUGMSG("video container resized to %dx%d",
					 neww, newh);
				
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
	DEBUGMSG("stopping media...");
	// todo: error-check this?
	llgst_element_set_state(mPlaybin, GST_STATE_READY);
	return true;
}

///////////////////////////////////////////////////////////////////////////////
//
bool
LLMediaImplGStreamer::
play ()
{
	DEBUGMSG("playing media...");
	// todo: error-check this?
	llgst_element_set_state(mPlaybin, GST_STATE_PLAYING);
	return true;
}

///////////////////////////////////////////////////////////////////////////////
//
bool
LLMediaImplGStreamer::
pause ()
{
	DEBUGMSG("pausing media...");
	// todo: error-check this?
	llgst_element_set_state(mPlaybin, GST_STATE_PAUSED);
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
	if (mDoneInit && mPlaybin)
	{
		success = llgst_element_seek(mPlaybin, 1.0F, GST_FORMAT_TIME,
				GstSeekFlags(GST_SEEK_FLAG_FLUSH |
					     GST_SEEK_FLAG_KEY_UNIT),
				GST_SEEK_TYPE_SET, gint64(time*GST_SECOND),
				GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE);
	}
	DEBUGMSG("MEDIA SEEK REQUEST to %fsec result was %d",
		 float(time), int(success));
	return success;
}

bool
LLMediaImplGStreamer::
getTimePos(double &sec_out)
{
	bool got_position = false;
	if (mPlaybin)
	{
		gint64 pos;
		GstFormat timefmt = GST_FORMAT_TIME;
		got_position =
			llgst_element_query_position &&
			llgst_element_query_position(mPlaybin,
						     &timefmt,
						     &pos);
		got_position = got_position
			&& (timefmt == GST_FORMAT_TIME);
		// GStreamer may have other ideas, but we consider the current position
		// undefined if not PLAYING or PAUSED
		got_position = got_position &&
			(GST_STATE(mPlaybin) == GST_STATE_PLAYING ||
			 GST_STATE(mPlaybin) == GST_STATE_PAUSED);
		if (got_position && !GST_CLOCK_TIME_IS_VALID(pos))
		{
			if (GST_STATE(mPlaybin) == GST_STATE_PLAYING)
			{
				// if we're playing then we treat an invalid clock time
				// as 0, for complicated reasons (insert reason here)
				pos = 0;
			}
			else
			{
				got_position = false;
			}
			
		}
		// If all the preconditions succeeded... we can trust the result.
		if (got_position)
		{
			sec_out = double(pos) / double(GST_SECOND); // gst to sec
		}
	}
	return got_position;
}

///////////////////////////////////////////////////////////////////////////////
// virtual
bool
LLMediaImplGStreamer::
setVolume(float volume)
{
	// we try to only update volume as conservatively as
	// possible, as many gst-plugins-base versions up to at least
	// November 2008 have critical race-conditions in setting volume - sigh
	if (mVolume == volume)
		return true; // nothing to do, everything's fine

	mVolume = volume;
	if (mDoneInit && mPlaybin)
	{
		g_object_set(mPlaybin, "volume", mVolume, NULL);
		return true;
	}

	return false;
}

#endif // LL_GSTREAMER_ENABLED
