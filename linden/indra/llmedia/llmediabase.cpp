/** 
 * @file llmediabase.cpp
 * @brief LLMedia support - base class
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

#include "llmediabase.h"
#include "llmediaimplquicktime.h"

LLMediaBase::LLMediaBase() :
	mBufferChangeCount ( 1 ),
	mLastBufferChangeCount ( 0 ),
	mMediaWidth ( 0 ),
	mMediaHeight ( 0 ),
	mMediaDepthBytes ( 0 ),
	mMediaRowbytes( 0 ),
	mTextureWidth ( 0 ),
	mTextureHeight ( 0 ),
	mTextureDepth ( 0 ),
	mTextureFormatInternal ( 0 ),
	mTextureFormatPrimary ( 0 ),
	mTextureFormatType ( 0 ),
	mTextureFormatSwapBytes ( 0 )
{
	
}

///////////////////////////////////////////////////////////////////////////////
//	factory method based on explicit media type
LLMediaBase* LLMediaBase::make( const MediaType mediaTypeIn, S32 width_pixels, S32 height_pixels )
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
LLMediaBase::
init ()
{
	// build event and emit it
	LLMediaEvent event ( 0, "" );
	mMediaEventEmitter.update ( &LLMediaObserver::onInit, event );

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//
BOOL
LLMediaBase::
load ( const LLString& urlIn )
{
	// build event and emit it
	LLMediaEvent event ( 0, urlIn );
	mMediaEventEmitter.update ( &LLMediaObserver::onLoad, event );

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//
BOOL
LLMediaBase::
unload ()
{
	LLMediaEvent event ( 0, "" );
	mMediaEventEmitter.update ( &LLMediaObserver::onUnload, event );

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//
S32 LLMediaBase::getTextureWidth() const
{
	return mTextureWidth;
}

///////////////////////////////////////////////////////////////////////////////
//
S32 LLMediaBase::getTextureHeight() const
{
	return mTextureHeight;
}

///////////////////////////////////////////////////////////////////////////////
//
S32 LLMediaBase::getTextureDepth() const
{
	return mTextureDepth;
}

///////////////////////////////////////////////////////////////////////////////
//
S32 LLMediaBase::getTextureFormatInternal() const
{
	return mTextureFormatInternal;
}

///////////////////////////////////////////////////////////////////////////////
//
S32 LLMediaBase::getTextureFormatPrimary() const
{
	return mTextureFormatPrimary;
}

///////////////////////////////////////////////////////////////////////////////
//
S32 LLMediaBase::getTextureFormatType() const
{
	return mTextureFormatType;
}

///////////////////////////////////////////////////////////////////////////////
//
S32 LLMediaBase::getTextureFormatSwapBytes() const
{
	return mTextureFormatSwapBytes;
}

///////////////////////////////////////////////////////////////////////////////
//
S32 LLMediaBase::getMediaWidth() const
{
	return mMediaWidth;
}

///////////////////////////////////////////////////////////////////////////////
//
S32 LLMediaBase::getMediaHeight() const
{
	return mMediaHeight;
}

///////////////////////////////////////////////////////////////////////////////
//
S32 LLMediaBase::getMediaDepthBytes() const
{
	return mMediaDepthBytes;
}

///////////////////////////////////////////////////////////////////////////////
//
S32 LLMediaBase::getMediaBufferSize() const
{
	if(mMediaRowbytes != 0)
	{
		return mMediaHeight * mMediaRowbytes;
	}
	
	return mMediaWidth * mMediaHeight * mMediaDepthBytes;
};

///////////////////////////////////////////////////////////////////////////////
//
BOOL LLMediaBase::addMediaObserver( LLMediaObserver* observerIn )
{
	return mMediaEventEmitter.addObserver( observerIn );
}

///////////////////////////////////////////////////////////////////////////////
//
BOOL LLMediaBase::remMediaObserver( LLMediaObserver* observerIn )
{
	return mMediaEventEmitter.remObserver( observerIn );
}
