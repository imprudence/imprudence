/** 
 * @file llmediaimplquicktime.cpp
 * @brief implementation that supports Apple QuickTime media.
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

#include "linden_common.h"

#if LL_QUICKTIME_ENABLED

#include <iostream>

#include "llmediaimplquicktime.h"

#include "llgl.h"
#include "llglheaders.h"	// For gl texture modes

///////////////////////////////////////////////////////////////////////////////
//
LLMediaImplQuickTime::
LLMediaImplQuickTime () :
	theController ( NULL ),
	currentMode ( ModeIdle ),
	theGWorld ( 0 ),
	theMovie ( 0 ),
	mediaData ( 0 ),
	loopsLeft ( 0 ),
	ownBuffer ( TRUE ),
	curVolume ( 0 ),
	sizeChangeInProgress ( FALSE ),
	initialStartDone ( FALSE ),
	autoScaled ( FALSE )
{
// These should probably be in the initializer list above, but that seemed uglier...
#if LL_DARWIN
	// Mac OS -- gworld will be xRGB (4 byte pixels, like ARGB, but QuickDraw doesn't actually do alpha...)
	mMediaDepthBytes = 4;
	mTextureDepth = 4;
	mTextureFormatInternal = GL_RGB8;
	mTextureFormatPrimary = GL_BGRA;
#ifdef LL_BIG_ENDIAN	
	mTextureFormatType = GL_UNSIGNED_INT_8_8_8_8_REV;
#else
	mTextureFormatType = GL_UNSIGNED_INT_8_8_8_8;
#endif

#else
	// Windows -- GWorld will be RGB (3 byte pixels)
	mMediaDepthBytes = 3;
	mTextureDepth = 3;
	mTextureFormatInternal = GL_RGB8;
	mTextureFormatPrimary = GL_RGB;
	mTextureFormatType = GL_UNSIGNED_BYTE;
#endif
};

///////////////////////////////////////////////////////////////////////////////
//
LLMediaImplQuickTime::
~LLMediaImplQuickTime ()
{
	unload();
}

///////////////////////////////////////////////////////////////////////////////
//
BOOL
LLMediaImplQuickTime::
setBuffer ( U8* bufferIn )
{
	OSErr err = noErr;
	
	// If we're waiting for a size change, we just got one.
	sizeChangeInProgress = FALSE;
		
	// Since we've pointed QuickTime at the old media data buffer directly, we need to be somewhat careful deleting it...
	U8* oldMediaData = mediaData;
	BOOL ownedMediaData = ownBuffer;
#if LL_DARWIN
	GWorldPtr oldGWorld = theGWorld;
#endif
	
	if(bufferIn == NULL)
	{
		// Passing NULL to this function requests that the object allocate its own buffer.
		mediaData = new unsigned char [ mMediaHeight * mMediaRowbytes ];
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
		// This is bad.
		llerrs << "LLMediaImplQuickTime::setBuffer: mediaData is NULL" << llendl;
		// NOTE: This case doesn't clean up properly.  This assert is fatal, so this isn't a huge problem,
		// but if this assert is ever removed the code should be fixed to clean up correctly.
		return FALSE;
	}
	
	err = NewGWorldFromPtr ( &theGWorld, mMediaDepthBytes * 8, &movieRect, NULL, NULL, 0, (Ptr)mediaData, mMediaRowbytes);
	if(err == noErr)
	{
		if(theMovie)
		{
			// tell the movie about it
			SetMovieGWorld ( theMovie, theGWorld, GetGWorldDevice ( theGWorld ) );
		}
		
		if(theController)
		{
			// and tell the movie controller about it.
			MCSetControllerPort(theController, theGWorld);
		}

#if LL_DARWIN
// NOTE: (CP) This call ultimately leads to a crash in NewGWorldFromPtr on Windows (only) 
//			  Not calling DisposeGWorld doesn't appear to leak anything significant and stops the crash occuring.
//			  This will eventually be fixed but for now, leaking slightly is better than crashing.
		if ( oldGWorld != NULL )
		{
			// Delete the old GWorld
			DisposeGWorld ( oldGWorld );
			oldGWorld = NULL;
		}
#endif
	}
	else
	{
		// Hmm... this may be bad.  Assert here?
		llerrs << "LLMediaImplQuickTime::setBuffer: NewGWorldFromPtr failed" << llendl;
		theGWorld = NULL;
		return FALSE;
	}
	
	// Delete the old media data buffer iff we owned it.
	if ( ownedMediaData )
	{
		if ( oldMediaData )
		{
			delete [] oldMediaData;
		}
	}

	// build event and emit it
	
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//
BOOL
LLMediaImplQuickTime::
init ()
{
	// movied to main application initialization for now because it's non-trivial and only needs to be done once
	// (even though it goes against the media framework design)
	//if ( InitializeQTML ( 0L ) != 0 )
	//{
	//	return FALSE;
	//};

	//if ( EnterMovies () != 0 )
	//{
	//	return FALSE;
	//};

	return LLMediaMovieBase::init();
}

///////////////////////////////////////////////////////////////////////////////
//
void 
LLMediaImplQuickTime::
updateMediaSize()
{
	if((theController == NULL) && (!isQTLoaded()))
	{
		// The movie's not loaded enough to get info about it yet.
		// Set up a dummy buffer.
		movieRect.left = movieRect.top = 0;
		movieRect.right = movieRect.bottom = 64;
		mMediaRowbytes = mMediaDepthBytes * 64;
		mMediaWidth = 64;
		mMediaHeight = 64;
		mTextureWidth = 64;
		mTextureHeight = 64;

		return;
	}
		
	// pick up the size of the movie
	GetMovieBox ( theMovie, &movieRect );
	
	// save the size of the media so consumer of media class can use it
	mMediaWidth = movieRect.right - movieRect.left;
	mMediaHeight = movieRect.bottom - movieRect.top;
	
	// Giant media could make us try to use textures bigger than the opengl implementation can handle.
	// Pin the maximum X or Y dimension to 1024.
	// NOTE: 1024x1024 may still hurt a lot, but it shouldn't cause opengl to flame out.
	if(mMediaWidth > 1024)
	{
		mMediaWidth = 1024;
	}
	if(mMediaHeight > 1024)
	{
		mMediaHeight = 1024;
	}
	
	// calculate the texture size required to hold media of this size (next power of 2 bigger)
	for ( mTextureWidth = 1; mTextureWidth < mMediaWidth; mTextureWidth <<= 1 )
	{
	};

	for ( mTextureHeight = 1; mTextureHeight < mMediaHeight; mTextureHeight <<= 1 )
	{
	};

//	llinfos << "Media texture size will be " << mTextureWidth << " x " << mTextureHeight << llendl;
	
	// if autoscale is on we simply make the media & texture sizes the same and quicktime does all the hard work
	if ( autoScaled )
	{
		// Stretch the movie to fill the texture.
		mMediaWidth = mTextureWidth;
		mMediaHeight = mTextureHeight;

		// scale movie using high quality but slow algorithm.
		// NOTE: this results in close to same quality as texture scaling option but with (perhaps) significant
		//       loss of performance (e.g. my machine, release build, frame rate goes from 92 -> 82 fps
		//       To revert to original behaviour, just remove the line below.
		
		// MBW -- There seems to be serious drop in performance above a particular size, on both low and high end machines.
		// 512x256 is fine, while 512x512 is unusable.  I theorize that this is due to CPU cache getting broken around that size.
		if((mTextureWidth * mTextureHeight) <= (512 * 256))
		{
//			llinfos << "Setting high-quality hint." << llendl;
			SetMoviePlayHints ( theMovie, hintsHighQuality, hintsHighQuality );
		}
	};
	
	// always flip movie using quicktime (little performance impact and no loss in quality)
	if ( TRUE )
	{
		// Invert the movie in the Y directon to match the expected orientation of GL textures.
		MatrixRecord transform;
		GetMovieMatrix ( theMovie, &transform );
		
		double centerX = mMediaWidth / 2.0;
		double centerY = mMediaHeight / 2.0;
		ScaleMatrix ( &transform, X2Fix ( 1.0 ), X2Fix ( -1.0 ), X2Fix ( centerX ), X2Fix ( centerY ) );

		SetMovieMatrix ( theMovie, &transform );
	};

	movieRect.left = 0;
	movieRect.top = 0;
	movieRect.right = mMediaWidth;
	movieRect.bottom = mMediaHeight;

	// Calculate the rowbytes of the texture
	mMediaRowbytes = mMediaWidth * mMediaDepthBytes;
	
	SetMovieBox(theMovie, &movieRect);

	if(theController == NULL)
	{
		SetGWorld(theGWorld, NULL);
		
		// Create a movie controller for the movie
		theController = NewMovieController(theMovie, &movieRect, mcNotVisible|mcTopLeftMovie);
		
		MCSetActionFilterWithRefCon(theController, myMCActionFilterProc, (long)this);
			
		// Allow the movie to dynamically resize (may be necessary for streaming movies to work right...)
		SetMoviePlayHints(theMovie, hintsAllowDynamicResize, hintsAllowDynamicResize);
	}
	else
	{
		MCPositionController(theController, &movieRect, &movieRect, mcTopLeftMovie|mcPositionDontInvalidate);
	}
}

///////////////////////////////////////////////////////////////////////////////
//
void 
LLMediaImplQuickTime::
setupDummyBuffer()
{
	// Used when the movie can't be drawn for some reason.  This sets up a buffer that keeps callers from getting annoyed.
	movieRect.left = movieRect.top = 0;
	movieRect.right = movieRect.bottom = 64;
	mMediaRowbytes = mMediaDepthBytes * 64;
	mMediaWidth = 64;
	mMediaHeight = 64;
	mTextureWidth = 64;
	mTextureHeight = 64;
	
	setBuffer ( NULL );
	
	memset(mediaData, 0, mMediaRowbytes * mMediaHeight );
}

///////////////////////////////////////////////////////////////////////////////
//
BOOL
LLMediaImplQuickTime::
load ( const LLString& urlIn )
{
	// The movie may do things to the current port when it's created.  Make sure we have a valid port set.
	setupDummyBuffer();
	SetGWorld(theGWorld, NULL);
	
	Size mySize = ( Size ) urlIn.length () + 1;
	if ( mySize == 0 )
		return FALSE;

	Handle myHandle = NewHandleClear ( mySize );
    if ( myHandle == NULL )
			return FALSE;

	BlockMove ( urlIn.c_str (), *myHandle, mySize );
	
	// Might be able to make this asynchronous with (newMovieActive|newMovieAsyncOK|newMovieIdleImportOK)? 
	OSErr err = NewMovieFromDataRef ( &theMovie, newMovieActive|newMovieDontInteractWithUser|newMovieAsyncOK|newMovieIdleImportOK, NULL, myHandle, URLDataHandlerSubType );
	
	if ( err != noErr )
		return false;

	// function that gets called when a frame is drawn
	SetMovieDrawingCompleteProc ( theMovie, movieDrawingCallWhenChanged, myFrameDrawnCallback, ( long ) this );
	
	if(isQTLoaded())
	{
		updateMediaSize();
		setBuffer(NULL);
	}
	
	// Tell the controller to play the movie.  This also deals with the movie still loading.
	//play();
		
	return LLMediaMovieBase::load(urlIn);
}

///////////////////////////////////////////////////////////////////////////////
//
OSErr
LLMediaImplQuickTime::
myFrameDrawnCallback ( Movie callbackMovie, long refCon )
{
	LLMediaImplQuickTime* myQtRenderer = ( LLMediaImplQuickTime* ) refCon;
	
	// The gworld quicktime is playing back into is now wrapped around myQtRenderer->mediaData,
	// so there's no need to copy any data here.
#if 0
	Ptr pixels = GetPixBaseAddr ( myQtRenderer->pixmapHandle );

	LockPixels ( myQtRenderer->pixmapHandle );

	memcpy ( ( U8* ) myQtRenderer->mediaData, pixels, myQtRenderer->getMediaBufferSize () );	/* Flawfinder: ignore */

	UnlockPixels ( myQtRenderer->pixmapHandle );
#endif

	myQtRenderer->bufferChanged();

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
Boolean 
LLMediaImplQuickTime::myMCActionFilterProc (MovieController theMC, short theAction, void *theParams, long theRefCon)
{
	Boolean result = false;
	LLMediaImplQuickTime *self = (LLMediaImplQuickTime*)theRefCon;

	switch ( theAction ) 
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

	return ( result );
}

///////////////////////////////////////////////////////////////////////////////
//
void 
LLMediaImplQuickTime::rewind()
{
	// MBW -- XXX -- I don't see an easy way to do this via the movie controller.  
	GoToBeginningOfMovie ( theMovie );
	
	// Call this afterwards so the movie controller can sync itself with the movie.
	MCMovieChanged(theController, theMovie);
	
#if 0
	// Maybe something like this?
	TimeRecord when;
	when.value.hi = 0;
	when.value.lo = 0;
	when.scale = GetMovieTimeScale(theMovie);
	
	// This seems like the obvious thing, but a tech note (http://developer.apple.com/technotes/qt/qt_510.html) says otherwise...
//	when.base = GetMovieTimeBase(theMovie);
	when.base = 0;
	
	MCDoAction(theController, mcActionGoToTime, &when);
#endif
}

///////////////////////////////////////////////////////////////////////////////
//
void 
LLMediaImplQuickTime::sizeChanged()
{
	// Set the movie to render (well, actually NOT render) to an internal buffer until the size change can be handled.
	setupDummyBuffer();
	
	// Make the next call to updateMedia request a size change.
	sizeChangeInProgress = true;

	// Recalculate the values that depend on the movie rect.
	updateMediaSize();
}

///////////////////////////////////////////////////////////////////////////////
//
BOOL 
LLMediaImplQuickTime::
isQTLoaded()
{
	BOOL result = false;
		
	if(theMovie)
	{
		if(GetMovieLoadState(theMovie) >= kMovieLoadStateLoaded)
		{
			result = true;
		}
	}
	
	return result;
}

///////////////////////////////////////////////////////////////////////////////
//
BOOL 
LLMediaImplQuickTime::
isQTPlaythroughOK()
{
	BOOL result = false;
		
	if(theMovie)
	{
		if(GetMovieLoadState(theMovie) >= kMovieLoadStatePlaythroughOK)
		{
			result = true;
		}
	}
	
	return result;
}

///////////////////////////////////////////////////////////////////////////////
//
BOOL
LLMediaImplQuickTime::
unload ()
{

	if( theController )
	{
		// Slight paranoia...
		MCSetActionFilterWithRefCon(theController, NULL, (long)this);

		DisposeMovieController( theController );
		theController = NULL;
	};
	
	if ( theMovie )
	{
		// Slight paranoia...
		SetMovieDrawingCompleteProc ( theMovie, movieDrawingCallWhenChanged, nil, ( long ) this );

		DisposeMovie ( theMovie );
		theMovie = NULL;
	};

	if ( theGWorld )
	{
		DisposeGWorld ( theGWorld );
		theGWorld = NULL;
	};

	if ( mediaData )
	{
		if ( ownBuffer )
		{
			delete mediaData;
			mediaData = NULL;
		};
	};

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//
S32
LLMediaImplQuickTime::
updateMedia ()
{	
	if(!theController)
	{
		if(isQTLoaded())
		{
			// Movie has finished loading.  Request a size change to update buffers, etc.
			// We MUST update the media size here, so it will be correct before the size change request.
			updateMediaSize();
			return updateMediaNeedsSizeChange;
		}
		else
		{
			// Movie is still loading.
			MoviesTask ( theMovie, 0 );
		}
	}

	if(theController)
	{
		switch(currentMode)
		{
			case ModePlaying:
			case ModeLooping:
			if(!initialStartDone)
			{
				if(isQTPlaythroughOK())
				{
					// The movie is loaded enough to start playing.  Start it now.
					MCDoAction(theController, mcActionPrerollAndPlay, (void*)GetMoviePreferredRate(theMovie));

					MCDoAction(theController, mcActionSetVolume, (void*)curVolume );

					initialStartDone = TRUE;
				}
			}
			break;
		}

//		// This function may copy decompressed movie frames into our media data pointer. JC
//		if (!mediaData)
//		{
//			llwarns << "LLMediaImplQuickTime::updateMedia() about to update with null media pointer" << llendl;
//		}
//		else
//		{
//			// try writing to the pointer to see if it's valid
//			*mediaData = 0;
//		}

		MCIdle(theController);
	}

	// If we need a size change, that takes precedence.
	if(sizeChangeInProgress)
	{
		return updateMediaNeedsSizeChange;
	}

	BOOL updated = getBufferChanged();

	resetBufferChanged();
	
	if(updated)
		return updateMediaNeedsUpdate;

	// don't use movie controller for looping - appears to be broken on PCs (volume issue)
	if ( currentMode == ModeLooping )
		if ( IsMovieDone ( theMovie ) )
			loop ( 0 );

	return updateMediaNoChanges;
}

///////////////////////////////////////////////////////////////////////////////
//
void
LLMediaImplQuickTime::
setAutoScaled ( BOOL autoScaledIn )
{
	autoScaled = autoScaledIn;
}

///////////////////////////////////////////////////////////////////////////////
//
BOOL
LLMediaImplQuickTime::
stop ()
{
	currentMode = ModeStopped;

	if(theController)
	{
		Fixed rate = X2Fix(0.0);
		MCDoAction(theController, mcActionPlay, (void*)rate);
		
		rewind();
	}
	
	return LLMediaMovieBase::stop();
};

///////////////////////////////////////////////////////////////////////////////
//
BOOL
LLMediaImplQuickTime::
play ()
{
	currentMode = ModePlaying;
	
	if(theController)
	{
		if ( IsMovieDone ( theMovie ) )
		{
			rewind();
		};

		MCDoAction(theController, mcActionPrerollAndPlay, (void*)GetMoviePreferredRate(theMovie));

		MCDoAction(theController, mcActionSetVolume, (void*)curVolume );
	}
	
	return LLMediaMovieBase::play();
};

///////////////////////////////////////////////////////////////////////////////
//
BOOL
LLMediaImplQuickTime::
loop ( S32 howMany )
{
	currentMode = ModeLooping;
	
	// MBW -- XXX -- This may be harder to do with a movie controller...
//	loopsLeft = howMany;

	if ( theController )
	{
		// Movie is loaded and set up.
		if ( IsMovieDone ( theMovie ) )
		{
			rewind();
		};

		MCDoAction(theController, mcActionPrerollAndPlay, (void*)GetMoviePreferredRate(theMovie));

		MCDoAction(theController, mcActionSetVolume, (void*)curVolume );
	}
	
	return LLMediaMovieBase::loop(howMany);
};

///////////////////////////////////////////////////////////////////////////////
//
BOOL
LLMediaImplQuickTime::
pause ()
{
	currentMode = ModePaused;

	if(theController)
	{
		// Movie is loaded and set up.
		Fixed rate = X2Fix(0.0);
		MCDoAction(theController, mcActionPlay, (void*)rate);
	}

	return LLMediaMovieBase::pause();
};

///////////////////////////////////////////////////////////////////////////////
//
BOOL
LLMediaImplQuickTime::
setVolume ( F32 volumeIn )
{
	// Fixed point signed short, 8.8
	curVolume = (short)(volumeIn * ( F32 ) 0x100);
	
	if(theController != NULL)
	{
		MCDoAction(theController, mcActionSetVolume, (void*)curVolume);
	}
	
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//
F32
LLMediaImplQuickTime::
getVolume ()
{
	return ( ( F32 ) curVolume ) / ( F32 ) 0x100;
}

///////////////////////////////////////////////////////////////////////////////
//
BOOL
LLMediaImplQuickTime::
isIdle () const
{
	return ( currentMode == ModeIdle );
};

///////////////////////////////////////////////////////////////////////////////
//
BOOL
LLMediaImplQuickTime::
isError () const
{
	return ( currentMode == ModeError );
};

///////////////////////////////////////////////////////////////////////////////
//
BOOL
LLMediaImplQuickTime::
isBuffering () const
{
	return ( currentMode == ModeBuffering );
};

///////////////////////////////////////////////////////////////////////////////
//
BOOL
LLMediaImplQuickTime::
isLoaded () const
{
	// Only tell the caller the movie is loaded if we've had a chance to set up the movie controller.
	
	return (theController != NULL);
};

///////////////////////////////////////////////////////////////////////////////
//
BOOL
LLMediaImplQuickTime::
isPlaying () const
{
	return ( currentMode == ModePlaying );
};

///////////////////////////////////////////////////////////////////////////////
//
BOOL
LLMediaImplQuickTime::
isLooping () const
{
	return ( currentMode == ModeLooping );
};

///////////////////////////////////////////////////////////////////////////////
//
BOOL
LLMediaImplQuickTime::
isPaused () const
{
	return ( currentMode == ModePaused );
};

///////////////////////////////////////////////////////////////////////////////
//
BOOL
LLMediaImplQuickTime::
isStopped () const
{
	return ( currentMode == ModeStopped );
};

///////////////////////////////////////////////////////////////////////////////
//
U8*
LLMediaImplQuickTime::
getMediaData ()
{
	return mediaData;
}

///////////////////////////////////////////////////////////////////////////////
//
BOOL 
LLMediaImplQuickTime::
seek ( F64 time )
{
	// MBW -- XXX -- This should stash the time if theController is NULL, and seek to there when the movie's loaded.
	// Do this later.
	if(theController != NULL)
	{
		TimeRecord when;
		when.scale = GetMovieTimeScale(theMovie);
		when.base = 0;

		// 'time' is in (floating point) seconds.  The timebase time will be in 'units', where
		// there are 'scale' units per second.
		S64 rawTime = (S64)(time * (F64)(when.scale));
		
		when.value.hi = ( SInt32 ) ( rawTime >> 32 );
		when.value.lo = ( SInt32 ) ( ( rawTime & 0x00000000FFFFFFFF ) );
		
		MCDoAction(theController, mcActionGoToTime, &when);
	}
		
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//
F64 
LLMediaImplQuickTime::
getTime () const
{
	F64 result = 0;
	
	if(theController != NULL)
	{
		TimeValue time;
		TimeScale scale = 0;

		time = MCGetCurrentTime(theController, &scale);
		if(scale != 0)
		{		
			result = ((F64)time) / ((F64)scale);
		}
	}
		
	return result;
}

///////////////////////////////////////////////////////////////////////////////
//
F64 
LLMediaImplQuickTime::
getMediaDuration () const
{
	F64 result = 0;
	TimeScale scale = GetMovieTimeScale(theMovie);
	TimeValue duration = GetMovieDuration(theMovie);
	
	if(duration == kQTSUnknownDuration)
	{
		// Hmph.
		// Return 0 in this case.
	}
	else if(duration == kQTSInfiniteDuration)
	{
		// This is the magic number for "indefinite duration", i.e. a live stream.
		// Note that the docs claim this value is 0x7FFFFFF, while the symbolic constant is 0x7FFFFFFF.  Go figure.
		// Return 0 in this case.
	}
	else if(scale != 0)
	{
		// Timescale is a useful number.  Convert to seconds.
		result = (F64)duration;
		result /= (F64)scale;
	}
	
	return result;
}

#endif
