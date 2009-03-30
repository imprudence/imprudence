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

///#if LL_GSTREAMER_ENABLED

#if LL_WINDOWS
	// GStreamer 0.10.22 - gstutils.h - conversion from 'guint64' to 'guint8'. 
	// This was an intentional change to make GStreamer more threadsafe, and
	// is okay. Delete this bit if GStreamer ever gets more VS-friendly -- McCabe
	#pragma warning(disable : 4244)
#endif

#include "llmediaimplgstreamer.h"

extern "C" {
#include <gst/gst.h>
#include <gst/gstelement.h>
}

#if LL_WINDOWS
	#pragma warning(default : 4244)
#include <direct.h>
#include <stdlib.h>
#endif

#include "llmediamanager.h"
#include "llmediaimplregister.h"

#include "llmediaimplgstreamervidplug.h"

#include "llerror.h"
#include "linden_common.h"

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
{
	startup( NULL );  // Startup gstreamer if it hasn't been already.

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
	mPlaybin = gst_element_factory_make ("playbin", "play");
	if (!mPlaybin)
	{
		// todo: cleanup pump
		return; // error
	}

	if (NULL == getenv("LL_GSTREAMER_EXTERNAL"))
	{
		// instantiate and connect a custom video sink
		LL_DEBUGS("MediaManager") << "extrenal video sink..." << LL_ENDL;

		// Plays inworld instead of in external player
		mVideoSink =
		GST_SLVIDEO(gst_element_factory_make ("private-slvideo", "slvideo"));
		if (!mVideoSink)
		{
			LL_WARNS("MediaImpl") << "Could not instantiate private-slvideo element." << LL_ENDL;
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
	LL_DEBUGS("MediaImpl") << ("dtor of media...") << LL_ENDL;
	unload();
}

////////////////////////////////////////////////////////////////////////////////
// virtual
std::string LLMediaImplGStreamer::getVersion()
{
	guint major, minor, micro, nano;
	gst_version(&major, &minor, &micro, &nano);
	std::string version = llformat("%d.%d.%d.%d",major,minor,micro,nano);
	return version;
}

//
// STARTUP
///////////////////////////////////////////////////////////////////////////////
// (static) super-initialization - called once at application startup
bool LLMediaImplGStreamer::startup (LLMediaManagerData* init_data)
{
	static bool done_init = false;
	if (!done_init)
	{
		// Init the glib type system - we need it.
		g_type_init();

		set_gst_plugin_path();

		// Protect against GStreamer resetting the locale, yuck.
		static std::string saved_locale;
		saved_locale = setlocale(LC_ALL, NULL);
		if (0 == gst_init_check(NULL, NULL, NULL))
		{
		    LL_WARNS("MediaImpl") << "GStreamer library failed to initialize and load standard plugins." << LL_ENDL;
			setlocale(LC_ALL, saved_locale.c_str() );
			return false;
		}
		setlocale(LC_ALL, saved_locale.c_str() );
		
		// Init our custom plugins - only really need do this once.
		gst_slvideo_init_class();


		// List the plugins GStreamer can find
		LL_DEBUGS("MediaImpl") << "Found GStreamer plugins:" << LL_ENDL;
		GList *list;
		GstRegistry *registry = gst_registry_get_default();
		std::string loaded = "";
		for (list = gst_registry_get_plugin_list(registry);
		     list != NULL;
		     list = g_list_next(list))
		{	 
			GstPlugin *list_plugin = (GstPlugin *)list->data;
			(bool)gst_plugin_is_loaded(list_plugin) ? loaded = "Yes" : loaded = "No";
			LL_DEBUGS("MediaImpl") << gst_plugin_get_name(list_plugin) << ", loaded? " << loaded << LL_ENDL;
		}
		gst_plugin_list_free(list);


		done_init = true;
	}
	return true;
}


void LLMediaImplGStreamer::set_gst_plugin_path()
{
	// Only needed for Windows.
	// Linux sets in wrapper.sh, Mac sets in Info-Imprudence.plist
#ifdef LL_WINDOWS

	char* imp_cwd;

	// Get the current working directory: 
	imp_cwd = _getcwd(NULL,0);

	if(imp_cwd == NULL)
	{
		LL_DEBUGS("MediaImpl") << "_getcwd failed, not setting GST_PLUGIN_PATH."
		                         << LL_ENDL;
	}
	else
	{
		LL_DEBUGS("MediaImpl") << "Imprudence is installed at "
		                         << imp_cwd << LL_ENDL;

		// Grab the current path, if it's set.
		std::string old_plugin_path = "";
		char *old_path = getenv("GST_PLUGIN_PATH");
		if(old_path == NULL)
		{
			LL_DEBUGS("MediaImpl") << "Did not find user-set GST_PLUGIN_PATH."
			                         << LL_ENDL;
		}
		else
		{
			old_plugin_path = ";" + std::string( old_path );
		}


		// Search both Imprudence and Imprudence\lib\gstreamer-plugins.
		// If those fail, search the path the user has set, if any.
		std::string plugin_path =
		  "GST_PLUGIN_PATH=" +
		  std::string(imp_cwd) + "\\lib\\gstreamer-plugins;" +
		  std::string(imp_cwd) +
		  old_plugin_path;

		// Place GST_PLUGIN_PATH in the environment settings for imprudence.exe
		// Returns 0 on success
		if(_putenv( (char*)plugin_path.c_str() ))
		{	
			LL_WARNS("MediaImpl") << "Setting environment variable failed!" << LL_ENDL;
		}
		else
		{
			LL_DEBUGS("MediaImpl") << "GST_PLUGIN_PATH set to "
									 << getenv("GST_PLUGIN_PATH") << LL_ENDL;
		}
	}

#endif //LL_WINDOWS
}


bool LLMediaImplGStreamer::closedown()
{
	return true;
}


///////////////////////////////////////////////////////////////////////////////
//
// Uncomment the line below to enable spammy debug data.
//#define LL_GST_REPORT_STATE_CHANGES
#ifdef LL_GST_REPORT_STATE_CHANGES
static const char* get_gst_state_name(GstState state)
{
	switch (state) 
	{
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
gboolean LLMediaImplGStreamer::bus_callback(GstBus *bus, GstMessage *message, gpointer data)
{
#ifdef LL_GST_REPORT_STATE_CHANGES
	LL_DEBUGS("MediaCallback") << "Got GST message type: " << GST_MESSAGE_TYPE_NAME (message) << LL_ENDL;
#endif

	LLMediaImplGStreamer *impl = (LLMediaImplGStreamer*)data;

	switch (GST_MESSAGE_TYPE (message)) 
	{
		case GST_MESSAGE_BUFFERING: 
		{
			gint percent = 0;
			gst_message_parse_buffering(message, &percent);
#ifdef LL_GST_REPORT_STATE_CHANGES
			LL_DEBUGS("MediaBuffering") << "GST buffering: " << percent << "%%" << LL_ENDL;
#endif
			LLMediaEvent event( impl, percent );
			impl->getEventEmitter().update( &LLMediaObserver::onUpdateProgress, event );
		}
			break;
		case GST_MESSAGE_STATE_CHANGED: 
		{
			GstState old_state;
			GstState new_state;
			GstState pending_state;
			gst_message_parse_state_changed(message,
						&old_state,
						&new_state,
						&pending_state);
#ifdef LL_GST_REPORT_STATE_CHANGES
		// not generally very useful, and rather spammy.
		LL_DEBUGS("MediaState") << "GST state change (old,<new>,pending): "<< get_gst_state_name(old_state) << ",<" << get_gst_state_name(new_state) << ">," << get_gst_state_name(pending_state) << LL_ENDL;
#endif // LL_GST_REPORT_STATE_CHANGES

			switch (new_state) 
			{
			case GST_STATE_VOID_PENDING:
				break;
			case GST_STATE_NULL:
#ifdef LL_GST_REPORT_STATE_CHANGES
				LL_DEBUGS("MediaImpl") << "State changed to NULL" << LL_ENDL;
#endif
				if (impl->getState() == GST_STATE_PLAYING) 
				{ 
					// Stream was probably dropped, trying to restart
				    impl->play();
#ifdef LL_GST_REPORT_STATE_CHANGES
				    LL_DEBUGS("MediaImpl") << "Trying to restart." << LL_ENDL;
#endif
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
		case GST_MESSAGE_ERROR: 
		{
			GError *err = NULL;
			gchar *debug = NULL;

			gst_message_parse_error (message, &err, &debug);
			LL_WARNS("MediaImpl") << "GST Error: " << err->message << LL_ENDL;
			g_error_free (err);
			g_free (debug);

			impl->addCommand(LLMediaBase::COMMAND_STOP);
			//impl->addCommand(LLMediaBase::COMMAND_START);

			break;
		}
		case GST_MESSAGE_INFO: 
		{
			GError *err = NULL;
			gchar *debug = NULL;
			
			gst_message_parse_info (message, &err, &debug);
			LL_INFOS("MediaImpl") << "GST info: " << err->message
														<< LL_ENDL;
			g_error_free (err);
			g_free (debug);
			break;
		}
		case GST_MESSAGE_WARNING: 
		{
			GError *err = NULL;
			gchar *debug = NULL;

			gst_message_parse_warning (message, &err, &debug);
			LL_WARNS("MediaImpl") << "GST warning: " <<  err->message
		    << LL_ENDL;
			g_error_free (err);
			g_free (debug);

			break;
		}
		case GST_MESSAGE_TAG: 
		{
	        GstTagList *tag_list;
		gchar *title;
		gchar *artist;
		gst_message_parse_tag(message, &tag_list);
		gboolean hazTitle = gst_tag_list_get_string(tag_list,
			GST_TAG_TITLE, &title);
		gboolean hazArtist = gst_tag_list_get_string(tag_list,
			GST_TAG_ARTIST, &artist);
		if(hazTitle) 
			LL_INFOS("MediaInfo") << "Title: " << title << LL_ENDL;
		if(hazArtist) 
			LL_INFOS("MediaInfo") << "Artist: " << artist << LL_ENDL;
			break;
		}
		case GST_MESSAGE_EOS:
		{
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
	}
	/* we want to be notified again the next time there is a message
	 * on the bus, so return true (false means we want to stop watching
	 * for messages on the bus and our callback should not be called again)
	 */
	return TRUE;
}

///////////////////////////////////////////////////////////
// virtual
bool LLMediaImplGStreamer::navigateTo (const std::string urlIn)
{
	LL_DEBUGS("MediaImpl") << "Setting media URI: " << urlIn.c_str()
	    << LL_ENDL;

	if (mPump == NULL || mPlaybin == NULL)
	{
		return false;
	}

	setStatus( LLMediaBase::STATUS_NAVIGATING );

	// set URI
	g_object_set (G_OBJECT (mPlaybin), "uri", urlIn.c_str(), NULL);

	// get playbin's bus - perhaps this can/should be done in ctor
	GstBus *bus = gst_pipeline_get_bus (GST_PIPELINE (mPlaybin));
	if (!bus)
	{
		return false;
	}
	gst_bus_add_watch (bus, bus_callback, this);
	gst_object_unref (bus);

	mState = GST_STATE_READY;

	// navigateTo implicitly plays, too.
	play();

	return true;
}

///////////////////////////////////////////////////////////////////////////////
//
bool LLMediaImplGStreamer::unload()
{
	LL_DEBUGS("MediaImpl") << "unloading media..." << LL_ENDL;
	if (mPlaybin)
	{
		gst_element_set_state (mPlaybin, GST_STATE_NULL);
		mState = GST_STATE_NULL;
		gst_object_unref (GST_OBJECT (mPlaybin));
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
	mState = GST_STATE_NULL;
	setStatus(LLMediaBase::STATUS_UNKNOWN);

	return true;
}

///////////////////////////////////////////////////////////////////////////////
// virtual
bool LLMediaImplGStreamer::updateMedia()
{
	//LL_DEBUGS("MediaImpl") << "updating media..." << LL_ENDL;
	
	// sanity check
	if (mPump == NULL || mPlaybin == NULL)
	{
#ifdef LL_GST_REPORT_STATE_CHANGES
		LL_DEBUGS("MediaImpl") << "dead media..." << LL_ENDL;
#endif
		mState = GST_STATE_NULL;
		setStatus(LLMediaBase::STATUS_DEAD);
		return false;
	}

	if (mState == GST_STATE_VOID_PENDING || mState == GST_STATE_NULL)
		return false;

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
#ifdef LL_GST_REPORT_STATE_CHANGES
			LL_DEBUGS("MediaImpl") <<"NEW FRAME " << LL_ENDL;
#endif
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
bool LLMediaImplGStreamer::stop()
{
	LL_DEBUGS("MediaImpl") << "attempting to stop..." << LL_ENDL;

	if (!mPlaybin || mState == GST_STATE_NULL)
		return true;

	GstElement *pipeline = (GstElement *)gst_object_ref(GST_OBJECT(mPlaybin));
	gst_object_unref(pipeline);
	
	gst_element_set_state(pipeline, GST_STATE_READY);

	if (mState == GST_STATE_PLAYING)
		mState = GST_STATE_VOID_PENDING;
	else
		mState = GST_STATE_READY; 

	GstStateChangeReturn state_change = gst_element_get_state(mPlaybin, NULL, NULL, GST_MSECOND*5);
	LL_DEBUGS("MediaImpl") << "get_state: " << gst_element_state_change_return_get_name(state_change) << LL_ENDL;

	return true;
}

///////////////////////////////////////////////////////////////////////////////
//
bool LLMediaImplGStreamer::play()
{
	LL_DEBUGS("MediaImpl") << "attempting to play..." << LL_ENDL;

	if (!mPlaybin || mState == GST_STATE_NULL)
		return true;

	GstElement *pipeline = (GstElement *)gst_object_ref(GST_OBJECT(mPlaybin));
	gst_object_unref(pipeline);
	
	gst_element_set_state(pipeline, GST_STATE_PLAYING);
	mState = GST_STATE_PLAYING;
	/*gst_element_set_state(mPlaybin, GST_STATE_PLAYING);
	mState = GST_STATE_PLAYING;*/

	GstStateChangeReturn state_change = gst_element_get_state(mPlaybin, NULL, NULL, GST_MSECOND*5);
	LL_DEBUGS("MediaImpl") << "get_state: " << gst_element_state_change_return_get_name(state_change) << LL_ENDL;

	// Check to make sure playing was successful. If not, stop.
	if (state_change == GST_STATE_CHANGE_FAILURE)
	{
		setStatus(LLMediaBase::STATUS_STOPPED);
		stop();
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
//
bool LLMediaImplGStreamer::pause()
{
	LL_DEBUGS("MediaImpl") << "attempting to pause..." << LL_ENDL;

	if (!mPlaybin || mState == GST_STATE_NULL)
		return true;

	gst_element_set_state(mPlaybin, GST_STATE_PAUSED);
	mState = GST_STATE_PAUSED;
	
	GstStateChangeReturn state_change = gst_element_get_state(mPlaybin, NULL, NULL, GST_MSECOND*5);
	LL_DEBUGS("MediaImpl") << "get_state: " << gst_element_state_change_return_get_name(state_change) << LL_ENDL;

	return true;
};


///////////////////////////////////////////////////////////////////////////////
// virtual
unsigned char* LLMediaImplGStreamer::getMediaData()
{
	return mediaData;
}


///////////////////////////////////////////////////////////////////////////////
// virtual
bool LLMediaImplGStreamer::seek(double time)
{
	bool success = false;
	if (mPlaybin)
	{
		success = gst_element_seek(mPlaybin, 1.0F, GST_FORMAT_TIME,
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
bool LLMediaImplGStreamer::setVolume(float volume)
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



///#endif // LL_GSTREAMER_ENABLED
