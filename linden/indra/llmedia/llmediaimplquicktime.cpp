/**
 * @file llmediaimplquicktime.cpp
 * @brief QuickTime media impl concrete class
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

#include "llmediaimplquicktime.h"

#if LL_QUICKTIME_ENABLED

#include "llmediamanager.h"
#include "llmediaimplregister.h"

#if LL_WINDOWS
#include <windows.h>
#endif

#include <iostream>
#include <sstream>

// register this impl with media manager factory
static LLMediaImplRegister sLLMediaImplQuickTimeReg( "LLMediaImplQuickTime", new LLMediaImplQuickTimeMaker() );

///////////////////////////////////////////////////////////////////////////////
//
LLMediaImplQuickTimeMaker::LLMediaImplQuickTimeMaker()
{
	// Register to handle the scheme
	mSchema.push_back( "rtsp" );

	// Register to handle the category
	mMimeTypeCategories.push_back( "video" );
	mMimeTypeCategories.push_back( "audio" );
	mMimeTypeCategories.push_back( "image" );
}

///////////////////////////////////////////////////////////////////////////////
//
LLMediaImplQuickTime::LLMediaImplQuickTime() :
	mMovieHandle( 0 ),
	mGWorldHandle( 0 ),
	mMovieController( 0 ),
	mMinWidth( 32 ),
	mMaxWidth( 2048 ),
	mMinHeight( 32 ),
	mMaxHeight( 2048 ),
	mCurVolume( 0 )
{
}

///////////////////////////////////////////////////////////////////////////////
//
LLMediaImplQuickTime::~LLMediaImplQuickTime()
{
	unload();
}

////////////////////////////////////////////////////////////////////////////////
// (static) super-initialization - called once at application startup
bool LLMediaImplQuickTime::startup( LLMediaManagerData* init_data )
{
#ifdef WIN32
	if ( InitializeQTML( 0L ) != noErr )
	{
		return false;
	};
#endif

	EnterMovies();

	return true;
}

////////////////////////////////////////////////////////////////////////////////
// (static) super-uninitialization - called once at application closedown
bool LLMediaImplQuickTime::closedown()
{
	ExitMovies();

#ifdef WIN32
	TerminateQTML();
#endif

	return true;
}

////////////////////////////////////////////////////////////////////////////////
// private
bool LLMediaImplQuickTime::load( const std::string url )
{
	if ( url.empty() )
		return false;

	//In case std::string::c_str() makes a copy of the url data,
	//make sure there is memory to hold it before allocating memory for handle.
	//if fails, NewHandleClear(...) should return NULL.
	const char* url_string = url.c_str() ;
	Handle handle = NewHandleClear( ( Size )( url.length() + 1 ) );
    if ( NULL == handle )
		return false;
	if(noErr != MemError() || NULL == *handle)
	{
		return false ;
	}

	BlockMove( url_string, *handle, ( Size )( url.length() + 1 ) );

	//std::cout << "LLMediaImplQuickTime::load( " << url << " )" << std::endl;

	// TODO: supposed to use NewMovieFromDataParams now
	OSErr err = NewMovieFromDataRef( &mMovieHandle, newMovieActive | newMovieDontInteractWithUser | newMovieAsyncOK | newMovieIdleImportOK, nil, handle, URLDataHandlerSubType );
	DisposeHandle( handle );
	if ( noErr != err )
		return false;

	// do pre-roll actions (typically fired for streaming movies but not always)
	PrePrerollMovie( mMovieHandle, 0, GetMoviePreferredRate( mMovieHandle ), moviePrePrerollCompleteCallback, ( void * )this );

	// get movie rect (and check for min/max)
	Rect movie_rect;
	setMovieBoxEnhanced( &movie_rect );

	// make a new movie controller
	mMovieController = NewMovieController( mMovieHandle, &movie_rect, mcNotVisible | mcTopLeftMovie );

#if defined(__APPLE__) || defined(MACOSX)
	setMediaDepth( 4 );
#else
	setMediaDepth( 3 );
#endif

	// tell manager about the media size
	setMediaSize( movie_rect.right - movie_rect.left, movie_rect.bottom - movie_rect.top);

	// movie controller
	MCSetActionFilterWithRefCon( mMovieController, mcActionFilterCallBack, ( long )this );

	SetMoviePlayHints( mMovieHandle, hintsAllowDynamicResize, hintsAllowDynamicResize );

	// function that gets called when a frame is drawn
	SetMovieDrawingCompleteProc( mMovieHandle, movieDrawingCallWhenChanged, movieDrawingCompleteCallback, ( long )this );

	// emit an event to say that a media source was loaded
	LLMediaEvent event( this );
	mEventEmitter.update( &LLMediaObserver::onMediaLoaded, event );

	// set up inital state
	sizeChanged();

	return true;
}

////////////////////////////////////////////////////////////////////////////////
// virtual
std::string LLMediaImplQuickTime::getVersion()
{
	long version;
	Gestalt( gestaltQuickTimeVersion, &version );

	std::ostringstream codec( "" );
	codec << "[";
	codec << sLLMediaImplQuickTimeReg.getImplName();
	codec << "] - ";
	codec << "QuickTime: " << std::hex << version;

	return codec.str();
}

////////////////////////////////////////////////////////////////////////////////
// virtual
bool LLMediaImplQuickTime::navigateTo( const std::string url )
{
	// tell engine what we're doing
	setStatus( LLMediaBase::STATUS_NAVIGATING );

	// remove the movie we were looking at
	unload();

	// load the new one (no real 'go to this url' function in QT)
	load( url );

	return true;
}

////////////////////////////////////////////////////////////////////////////////
// virtual
bool LLMediaImplQuickTime::sizeChanged()
{
	if ( ! mMovieHandle )
		return false;

	// sanitize size of movie
	Rect movie_rect;
	setMovieBoxEnhanced( &movie_rect );

	// we need this later
	int width = ( movie_rect.right - movie_rect.left );
	int height = ( movie_rect.bottom - movie_rect.top );

	std::cout << "LLMEDIA> size changed to " << width << " x " << height << std::endl;

	setMediaSize( width, height );

	// media depth won't change
	int depth_bits = getMediaDepth() * 8;

	GWorldPtr old_gworld_handle = mGWorldHandle;

	if (old_gworld_handle)
	{
		GWorldFlags result = UpdateGWorld( &mGWorldHandle, depth_bits, &movie_rect, NULL, NULL, 0 );
		if ( gwFlagErr == result )
		{
			// TODO: unrecoverable?? throw exception?  return something?
			return false;
		}
	}
	else
	{
		OSErr result = NewGWorld( &mGWorldHandle, depth_bits, &movie_rect, NULL, NULL, keepLocal | pixelsLocked );
		if ( noErr != result )
		{
			// ATODO: unrecoverable??  throw exception?  return something?
			return false;
		}

		// clear memory in GWorld to avoid random screen visual fuzz from uninitialized texture data
		if ( mGWorldHandle )
		{
			PixMapHandle pix_map_handle = GetGWorldPixMap( mGWorldHandle );
			unsigned char* ptr = ( unsigned char* )GetPixBaseAddr( pix_map_handle );
			memset( ptr, 0x00, height * QTGetPixMapHandleRowBytes( pix_map_handle ) );
		}
	}

	// point movie at GWorld if it's new
	if ( mMovieHandle && ! old_gworld_handle )
	{
		SetMovieGWorld( mMovieHandle, mGWorldHandle, GetGWorldDevice ( mGWorldHandle ) );
	}

	// update movie controller
	if ( mMovieController )
	{
		MCSetControllerPort( mMovieController, mGWorldHandle );
		MCPositionController( mMovieController, &movie_rect, &movie_rect,
							  mcTopLeftMovie | mcPositionDontInvalidate );
		MCMovieChanged( mMovieController, mMovieHandle );
	}

	// Emit event with size change so the calling app knows about it too
	LLMediaEvent event( this );
	mEventEmitter.update( &LLMediaObserver::onMediaSizeChange, event );

	return true;
}

////////////////////////////////////////////////////////////////////////////////
// static
Boolean LLMediaImplQuickTime::mcActionFilterCallBack( MovieController mc, short action, void *params, long ref )
{
	Boolean result = false;

	LLMediaImplQuickTime* self = ( LLMediaImplQuickTime* )ref;

	switch( action )
	{
		// handle window resizing
		case mcActionControllerSizeChanged:
			self->sizeChanged();
			break;

		// Block any movie controller actions that open URLs.
		case mcActionLinkToURL:
		case mcActionGetNextURL:
		case mcActionLinkToURLExtended:
			// Prevent the movie controller from handling the message
			result = true;
			break;

		default:
			break;
	};

	return result;
}

////////////////////////////////////////////////////////////////////////////////
// private
bool LLMediaImplQuickTime::unload()
{
	if ( mMovieHandle )
	{
		StopMovie( mMovieHandle );
		if ( mMovieController )
		{
			MCMovieChanged( mMovieController, mMovieHandle );
		};
	};

	if ( mMovieController )
	{
		MCSetActionFilterWithRefCon( mMovieController, NULL, (long)this );
		DisposeMovieController( mMovieController );
		mMovieController = NULL;
	};

	if ( mMovieHandle )
	{
		SetMovieDrawingCompleteProc( mMovieHandle, movieDrawingCallWhenChanged, nil, ( long )this );
		DisposeMovie ( mMovieHandle );
		mMovieHandle = NULL;
	};

	if ( mGWorldHandle )
	{
		DisposeGWorld( mGWorldHandle );
		mGWorldHandle = NULL;
	};

	return true;
}

////////////////////////////////////////////////////////////////////////////////
// static
OSErr LLMediaImplQuickTime::movieDrawingCompleteCallback( Movie call_back_movie, long ref )
{
	LLMediaImplQuickTime* self = ( LLMediaImplQuickTime* )ref;

	// IMPORTANT: typically, a consumer who is observing this event will set a flag
	// when this event is fired then render later. Be aware that the media stream
	// can change during this period - dimensions, depth, format etc.
	LLMediaEvent event( self );
	self->mEventEmitter.update( &LLMediaObserver::onMediaContentsChange, event );

	return noErr;
}

////////////////////////////////////////////////////////////////////////////////
// static
void LLMediaImplQuickTime::moviePrePrerollCompleteCallback( Movie movie, OSErr preroll_err, void *ref )
{
	LLMediaImplQuickTime* self = ( LLMediaImplQuickTime* )ref;

	LLMediaEvent event( self );
	self->mEventEmitter.update( &LLMediaObserver::onMediaPreroll, event );
}

///////////////////////////////////////////////////////////////////////////////
// used for stop / loop
void LLMediaImplQuickTime::rewind()
{
	GoToBeginningOfMovie ( mMovieHandle );

	MCMovieChanged( mMovieController, mMovieHandle );
}

////////////////////////////////////////////////////////////////////////////////
//
bool LLMediaImplQuickTime::processState()
{
	// start stream
	if ( nextCommand() == LLMediaBase::COMMAND_START )
	{
		// valid when we are in these states
		if ( getStatus() == LLMediaBase::STATUS_NAVIGATING|| getStatus() == LLMediaBase::STATUS_STOPPED || getStatus() == LLMediaBase::STATUS_PAUSED )
		{
			// it appears that the movie must be in a loaded state before we do this command
			if ( GetMovieLoadState( mMovieHandle ) >= kMovieLoadStatePlaythroughOK )
			{
				MCDoAction( mMovieController, mcActionPrerollAndPlay, (void*)GetMoviePreferredRate( mMovieHandle ) );

				MCDoAction( mMovieController, mcActionSetVolume, (void*)mCurVolume );

				setStatus( LLMediaBase::STATUS_STARTED );

				clearCommand();
			}
		}
	}
	else
	if ( nextCommand() == LLMediaBase::COMMAND_STOP )
	{
		// valid when we are in these states
		if ( getStatus() == LLMediaBase::STATUS_NAVIGATING || getStatus() == LLMediaBase::STATUS_STARTED || getStatus() == LLMediaBase::STATUS_PAUSED )
		{
			// it appears that the movie must be in a loaded state before we do this command
			if ( GetMovieLoadState( mMovieHandle ) >= kMovieLoadStatePlaythroughOK )
			{
				// stop playing
				Fixed rate = X2Fix( 0.0 );
				MCDoAction( mMovieController, mcActionPlay, (void*)rate );

				// go back to start
				rewind();

				setStatus( LLMediaBase::STATUS_STOPPED );
				clearCommand();
			};
		};
	}
	else
	if ( nextCommand() == LLMediaBase::COMMAND_PAUSE )
	{
		// valid when we are in these states
		if ( getStatus() == LLMediaBase::STATUS_NAVIGATING || getStatus() == LLMediaBase::STATUS_STARTED || getStatus() == LLMediaBase::STATUS_STOPPED )
		{
			// it appears that the movie must be in a loaded state before we do this command
			if ( GetMovieLoadState( mMovieHandle ) >= kMovieLoadStatePlaythroughOK )
			{
				// stop playing
				Fixed rate = X2Fix( 0.0 );
				MCDoAction( mMovieController, mcActionPlay, (void*)rate );

				setStatus( LLMediaBase::STATUS_PAUSED );
				clearCommand();
			};
		};
	};

	return true;
}

////////////////////////////////////////////////////////////////////////////////
// virtual
bool LLMediaImplQuickTime::setMovieBoxEnhanced( Rect* rect )
{
	// get movie rect
	GetMovieNaturalBoundsRect( mMovieHandle, rect );

	int natural_width  = ( rect->right - rect->left );
	int natural_height = ( rect->bottom - rect->top );

	int width  = natural_width;
	int height = natural_height;
	
	// if the user has requested a specific size, use it:
	if ((mMediaRequestedWidth != 0) && (mMediaRequestedHeight != 0))
	{
		width = mMediaRequestedWidth;
		height = mMediaRequestedHeight;
	}

	// if the user has requested, resize media to exactly fit texture
	if (mAutoScaled)
	{
		width = LLMediaManager::textureWidthFromMediaWidth( width );
		height = LLMediaManager::textureHeightFromMediaHeight( height );
	}

	// make sure it falls in valid range
	if ( width < mMinWidth )
		width = mMinWidth;

	if ( width > mMaxWidth )
		width = mMaxWidth;

	if ( height < mMinHeight )
		height = mMinHeight;

	if ( height > mMaxHeight )
		height = mMaxHeight;

	
	// scale movie to fit rect and invert vertically to match opengl image format
	MatrixRecord transform;
	SetIdentityMatrix( &transform );	// transforms are additive so start from identify matrix
	double scaleX = (double) width / natural_width;
	double scaleY = -1.0 * (double) height / natural_height;
	double centerX = width / 2.0;
	double centerY = height / 2.0;
	ScaleMatrix( &transform, X2Fix ( scaleX ), X2Fix ( scaleY ), X2Fix ( centerX ), X2Fix ( centerY ) );
	SetMovieMatrix( mMovieHandle, &transform );

	// return the new rect
	rect->right = width;
	rect->bottom = height;
	rect->left = 0;
	rect->top = 0;

	return true;
}

////////////////////////////////////////////////////////////////////////////////
// virtual
bool LLMediaImplQuickTime::updateMedia()
{
	if ( ! mMovieHandle )
		return false;

	if ( ! mMovieController )
		return false;

	if ( ! mGWorldHandle )
		return false;

	// service QuickTime
	MoviesTask( mMovieHandle, 0 );
	MCIdle( mMovieController );

	// update state machine (deals with transport controls for example)
	processState();

	// special code for looping - need to rewind at the end of the movie

	if ( isLooping() )
	{
		// QT call to see if we are at the end - can't do with controller
		if ( IsMovieDone( mMovieHandle ) )
		{
			// go back to start
			rewind();

			// kick off new play
			MCDoAction( mMovieController, mcActionPrerollAndPlay, (void*)GetMoviePreferredRate( mMovieHandle ) );

			// set the volume
			MCDoAction( mMovieController, mcActionSetVolume, (void*)mCurVolume );
		}
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////
// virtual
unsigned char* LLMediaImplQuickTime::getMediaData()
{
	unsigned char* ptr = NULL;

	if ( mGWorldHandle )
	{
		PixMapHandle pix_map_handle = GetGWorldPixMap( mGWorldHandle );

		ptr = ( unsigned char* )GetPixBaseAddr( pix_map_handle );
	};

	return ptr;
}

////////////////////////////////////////////////////////////////////////////////
// virtual
int LLMediaImplQuickTime::getMediaDataWidth() const
{
	if ( mGWorldHandle )
	{
		int depth = getMediaDepth();

		if (depth < 1)
			depth = 1;

		// ALWAYS use the row bytes from the PixMap if we have a GWorld because
		// sometimes it's not the same as mMediaDepth * mMediaWidth !
		PixMapHandle pix_map_handle = GetGWorldPixMap( mGWorldHandle );
		return QTGetPixMapHandleRowBytes( pix_map_handle ) / depth;
	}
	else
	{
		return LLMediaImplCommon::getMediaDataWidth();
	}
}

////////////////////////////////////////////////////////////////////////////////
// virtual
int LLMediaImplQuickTime::getTextureFormatPrimary() const
{
#if defined(__APPLE__) || defined(MACOSX)
	return LL_MEDIA_BGRA;
#else
	return LL_MEDIA_RGB;
#endif
}

////////////////////////////////////////////////////////////////////////////////
// virtual
int LLMediaImplQuickTime::getTextureFormatType() const
{
#if defined(__APPLE__) || defined(MACOSX)
	#ifdef __BIG_ENDIAN__
		return LL_MEDIA_UNSIGNED_INT_8_8_8_8_REV;
	#else
		return LL_MEDIA_UNSIGNED_INT_8_8_8_8;
	#endif
#else
	return LL_MEDIA_UNSIGNED_BYTE;
#endif
}

////////////////////////////////////////////////////////////////////////////////
// virtual
bool LLMediaImplQuickTime::seek( double time )
{
	if ( mMovieController )
	{
		TimeRecord when;
		when.scale = GetMovieTimeScale( mMovieHandle );
		when.base = 0;

		// 'time' is in (floating point) seconds.  The timebase time will be in 'units', where
		// there are 'scale' units per second.
		SInt64 raw_time = ( SInt64 )( time * (double)( when.scale ) );

		when.value.hi = ( SInt32 )( raw_time >> 32 );
		when.value.lo = ( SInt32 )( ( raw_time & 0x00000000FFFFFFFF ) );

		MCDoAction( mMovieController, mcActionGoToTime, &when );

		return true;
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////
// virtual
bool LLMediaImplQuickTime::setVolume( float volume )
{
	mCurVolume = (short)(volume * ( double ) 0x100 );

	if ( mMovieController )
	{
		MCDoAction( mMovieController, mcActionSetVolume, (void*)mCurVolume );

		return true;
	}

	return false;
}

#endif // _3DNOW_InstructionExtensions/ LL_QUICKTIME_ENABLED

