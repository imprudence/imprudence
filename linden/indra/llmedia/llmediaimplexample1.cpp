/**
 * @file llmediaimplexample1.cpp
 * @brief Example 1 of a media impl concrete class
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

#include "llmediaimplexample1.h"
#include "llmediaimplregister.h"

#include <cstring>

// register this impl with media manager factory
static LLMediaImplRegister sLLMediaImplExample1Reg( "LLMediaImplExample1", new LLMediaImplExample1Maker() );

#include <iostream>

#include <time.h>

///////////////////////////////////////////////////////////////////////////////
//
LLMediaImplExample1Maker::LLMediaImplExample1Maker()
{
	// Register to handle the scheme
	mSchema.push_back( "example1" );
}

///////////////////////////////////////////////////////////////////////////////
//
LLMediaImplExample1::LLMediaImplExample1() :
	mMediaPixels( 0 )
{
	setRequestedMediaSize( 400, 200 );
	setMediaDepth( 3 );

	srand( (unsigned int)(time( NULL )) );
}

////////////////////////////////////////////////////////////////////////////////
// (static) super-initialization - called once at application startup
bool LLMediaImplExample1::startup( LLMediaManagerData* init_data )
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////
// (static) super-uninitialization - called once at application closedown
bool LLMediaImplExample1::closedown()
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////
// virtual
bool LLMediaImplExample1::init()
{
	int buffer_size = getMediaBufferSize();

	mMediaPixels = new unsigned char[ buffer_size ];

	memset( mMediaPixels, 0xAA, buffer_size );

	return true;
}

////////////////////////////////////////////////////////////////////////////////
// virtual
bool LLMediaImplExample1::navigateTo( const std::string url )
{
	std::cout << "LLMediaImplExample1::navigateTo" << std::endl;

	setStatus( LLMediaBase::STATUS_NAVIGATING );

	// force a size change event for new URL
	LLMediaEvent event( this );
	mEventEmitter.update( &LLMediaObserver::onMediaSizeChange, event );

	return true;
}

////////////////////////////////////////////////////////////////////////////////
// virtual
std::string LLMediaImplExample1::getVersion()
{
	std::string version_string = "[" + sLLMediaImplExample1Reg.getImplName() + "] - " + "1.0.0.0";

	return version_string;
}

////////////////////////////////////////////////////////////////////////////////
// virtual
bool LLMediaImplExample1::updateMedia()
{
	if ( mMediaPixels && getStatus() == LLMediaBase::STATUS_STARTED )
	{
		// first time - make sure it's a few seconds back so first update happens immediately
		static time_t t = time( 0 ) - 4;

		// selected time period elapsed (1 second)
		if ( time( 0 ) - t > 1 )
		{
			// display checkerboard
			const int num_squares = rand() % 20 + 4;
			int sqr1_r = rand() % 0x80;
			int sqr1_g = rand() % 0x80;
			int sqr1_b = rand() % 0x80;
			int sqr2_r = rand() % 0x80;
			int sqr2_g = rand() % 0x80;
			int sqr2_b = rand() % 0x80;

			for ( int y1 = 0; y1 < num_squares; ++y1 )
			{
				for ( int x1 = 0; x1 < num_squares; ++x1 )
				{
					int px_start = getMediaWidth() * x1 / num_squares;
					int px_end = ( getMediaWidth() * ( x1 + 1 ) ) / num_squares;
					int py_start = getMediaHeight() * y1 / num_squares;
					int py_end = ( getMediaHeight() * ( y1 + 1 ) ) / num_squares;

					for( int y2 = py_start; y2 < py_end; ++y2 )
					{
						for( int x2 = px_start; x2 < px_end; ++x2 )
						{
							int rowspan = getMediaWidth() * getMediaDepth();

							if ( ( y1 % 2 ) ^ ( x1 % 2 ) )
							{
								mMediaPixels[ y2 * rowspan + x2 * getMediaDepth() + 0 ] = sqr1_r;
								mMediaPixels[ y2 * rowspan + x2 * getMediaDepth() + 1 ] = sqr1_g;
								mMediaPixels[ y2 * rowspan + x2 * getMediaDepth() + 2 ] = sqr1_b;
							}
							else
							{
								mMediaPixels[ y2 * rowspan + x2 * getMediaDepth() + 0 ] = sqr2_r;
								mMediaPixels[ y2 * rowspan + x2 * getMediaDepth() + 1 ] = sqr2_g;
								mMediaPixels[ y2 * rowspan + x2 * getMediaDepth() + 2 ] = sqr2_b;
							};
						};
					};
				};
			};

			// emit an event to say that something in the media stream changed
			LLMediaEvent event( this );
			mEventEmitter.update( &LLMediaObserver::onMediaContentsChange, event );

			// reset time
			t = time( 0 );

			return true;
		};
	};

	// update the command (e.g. transport controls) state
	updateCommand();

	return false;
}

////////////////////////////////////////////////////////////////////////////////
// virtual
unsigned char* LLMediaImplExample1::getMediaData()
{
	return mMediaPixels;
}

////////////////////////////////////////////////////////////////////////////////
// virtual
bool LLMediaImplExample1::reset()
{
	if ( mMediaPixels )
	{
		delete [] mMediaPixels;
	};

	return true;
}

////////////////////////////////////////////////////////////////////////////////
// virtual
bool LLMediaImplExample1::mouseMove( int x_pos, int y_pos )
{
	if ( mMediaPixels && getStatus() == LLMediaBase::STATUS_STARTED )
	{
		int base_pos = x_pos * getMediaDepth() + y_pos * getMediaDepth() * getMediaWidth();
		// example: write a bright pixel to the display when we move the mouse
		mMediaPixels[ base_pos + 0 ] = rand() % 0x80 + 0x80;
		mMediaPixels[ base_pos + 1 ] = rand() % 0x80 + 0x80;
		mMediaPixels[ base_pos + 2 ] = rand() % 0x80 + 0x80;

		// emit an event to say that something in the media stream changed
		LLMediaEvent event( this );
		mEventEmitter.update( &LLMediaObserver::onMediaContentsChange, event );
	};

	return true;
}


////////////////////////////////////////////////////////////////////////////////
// virtual
bool LLMediaImplExample1::setRequestedMediaSize( int width, int height )
{
	// we accept any size:
	return setMediaSize(width, height);
}
