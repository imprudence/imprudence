/** 
 * @file llmediamoviebase.cpp
 * @brief LLMedia support - base class.
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

#include "llmediamoviebase.h"
#include "llmediaimplquicktime.h"

LLMediaMovieBase::LLMediaMovieBase()
{
	
}

///////////////////////////////////////////////////////////////////////////////
//	factory method based on explicit media type
LLMediaMovieBase* LLMediaMovieBase::make( const MediaType mediaTypeIn, S32 width_pixels, S32 height_pixels )
{
#if LL_QUICKTIME_ENABLED
	if ( mediaTypeIn == QuickTime )
	{
		return new LLMediaImplQuickTime ();
	}
	else
#endif

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
//
BOOL
LLMediaMovieBase::
play ()
{
	// build event and emit it
	LLMediaEvent event ( 0, "" );
	mMediaEventEmitter.update ( &LLMediaObserver::onPlay, event );

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//
BOOL
LLMediaMovieBase::
pause ()
{
	LLMediaEvent event ( 0, "" );
	mMediaEventEmitter.update ( &LLMediaObserver::onPause, event );

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//
BOOL
LLMediaMovieBase::
stop ()
{
	LLMediaEvent event ( 0, "" );
	mMediaEventEmitter.update ( &LLMediaObserver::onStop, event );

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//
BOOL
LLMediaMovieBase::
loop ( S32 howMany )
{
	return TRUE;
};

///////////////////////////////////////////////////////////////////////////////
//
void LLMediaMovieBase::setAutoScaled ( BOOL autoScaledIn )
{
}

///////////////////////////////////////////////////////////////////////////////
//
BOOL
LLMediaMovieBase::
setVolume ( F32 volumeIn )
{
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//
F32
LLMediaMovieBase::
getVolume ()
{
	return ( F32 ) 0.0f;
}

///////////////////////////////////////////////////////////////////////////////
//
BOOL
LLMediaMovieBase::
isIdle () const
{
	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
//
BOOL
LLMediaMovieBase::
isBuffering () const
{
	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
//
BOOL
LLMediaMovieBase::
isError () const
{
	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
//
BOOL
LLMediaMovieBase::
isLoaded () const
{
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//
BOOL
LLMediaMovieBase::
isStopped () const
{
	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
//
BOOL
LLMediaMovieBase::
isPaused () const
{
	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
//
BOOL
LLMediaMovieBase::
isPlaying () const
{
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//
BOOL
LLMediaMovieBase::
isLooping () const
{
	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
//
BOOL 
LLMediaMovieBase::
seek ( F64 time )
{
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//
F64 
LLMediaMovieBase::
getTime () const
{
	F64 result = 0;
	return result;
}

///////////////////////////////////////////////////////////////////////////////
//
F64 
LLMediaMovieBase::
getMediaDuration () const
{
	F64 result = 0;
	return result;
}
