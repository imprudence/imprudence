/**
 * @file llmediaimplexample2.cpp
 * @brief Example 2 of a media impl concrete class
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

#include "llmediaimplexample2.h"
#include "llmediaimplregister.h"

#include <cstring>

// register this impl with media manager factory
static LLMediaImplRegister sLLMediaImplExample2Reg( "LLMediaImplExample2", new LLMediaImplExample2Maker() );

#include <iostream>
#include <time.h>

///////////////////////////////////////////////////////////////////////////////
//
LLMediaImplExample2Maker::LLMediaImplExample2Maker()
{
	// Register to handle the scheme
	mSchema.push_back( "example2." );
}

///////////////////////////////////////////////////////////////////////////////
//
LLMediaImplExample2::LLMediaImplExample2() :
	mMediaPixels( 0 )
{
	setRequestedMediaSize( 500, 500 );
	setMediaDepth( 3 );

	mXpos = ( getMediaWidth() / 2 ) + rand() % ( getMediaWidth() / 16 ) - ( getMediaWidth() / 32 );
	mYpos = ( getMediaHeight() / 2 ) + rand() % ( getMediaHeight() / 16 ) - ( getMediaHeight() / 32 );

	srand( (unsigned int)(time( NULL )) );
}

////////////////////////////////////////////////////////////////////////////////
// (static) super-initialization - called once at application startup
bool LLMediaImplExample2::startup( LLMediaManagerData* init_data )
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////
// (static) super-uninitialization - called once at application closedown
bool LLMediaImplExample2::closedown()
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////
// virtual
bool LLMediaImplExample2::init()
{
	int buffer_size = getMediaBufferSize();

	mMediaPixels = new unsigned char[ buffer_size ];

	memset( mMediaPixels, 0x00, buffer_size );

	return true;
}

////////////////////////////////////////////////////////////////////////////////
// virtual
bool LLMediaImplExample2::navigateTo( const std::string url )
{
	std::cout << "LLMediaImplExample2::navigateTo" << std::endl;

	setStatus( LLMediaBase::STATUS_NAVIGATING );

	// force a size change event for new URL
	LLMediaEvent event( this );
	mEventEmitter.update( &LLMediaObserver::onMediaSizeChange, event );

	return true;
}

////////////////////////////////////////////////////////////////////////////////
// virtual
std::string LLMediaImplExample2::getVersion()
{
	std::string version_string = "[" + sLLMediaImplExample2Reg.getImplName() + "] - " + "1.0.0.0";

	return version_string;
}

////////////////////////////////////////////////////////////////////////////////
// virtual
bool LLMediaImplExample2::updateMedia()
{
	if ( mMediaPixels && getStatus() == LLMediaBase::STATUS_STARTED )
	{
		static int x_inc = rand() % 5 + 2;
		static int y_inc = rand() % 5 + 2;
		int block_size = 32;

		for( int y = 0; y < block_size; ++y )
		{
			for( int x = 0; x < block_size; ++x )
			{
				int rowspan = getMediaWidth() * getMediaDepth();
				mMediaPixels[ ( mXpos + x ) * getMediaDepth() + ( mYpos + y ) * rowspan + 0 ] = 0;
				mMediaPixels[ ( mXpos + x ) * getMediaDepth() + ( mYpos + y ) * rowspan + 1 ] = 0;
				mMediaPixels[ ( mXpos + x ) * getMediaDepth() + ( mYpos + y ) * rowspan + 2 ] = 0;
			};
		};

		if ( mXpos + x_inc < 0 || mXpos + x_inc >= getMediaWidth() - block_size )
			x_inc =- x_inc;

		if ( mYpos + y_inc < 0 || mYpos + y_inc >= getMediaHeight() - block_size )
			y_inc =- y_inc;

		mXpos += x_inc;
		mYpos += y_inc;

		unsigned char col_r = rand() % 0xff;
		unsigned char col_g = rand() % 0xff;
		unsigned char col_b = rand() % 0xff;

		for( int y = 0; y < block_size; ++y )
		{
			for( int x = 0; x < block_size; ++x )
			{
				int rowspan = getMediaWidth() * getMediaDepth();
				mMediaPixels[ ( mXpos + x ) * getMediaDepth() + ( mYpos + y ) * rowspan + 0 ] = col_r;
				mMediaPixels[ ( mXpos + x ) * getMediaDepth() + ( mYpos + y ) * rowspan + 1 ] = col_g;
				mMediaPixels[ ( mXpos + x ) * getMediaDepth() + ( mYpos + y ) * rowspan + 2 ] = col_b;
			};
		};

		// emit an event to say that something in the media stream changed
		LLMediaEvent event( this );
		mEventEmitter.update( &LLMediaObserver::onMediaContentsChange, event );
	};

	// update the command (e.g. transport controls) state
	updateCommand();

	return false;
}

////////////////////////////////////////////////////////////////////////////////
// virtual
unsigned char* LLMediaImplExample2::getMediaData()
{
	return mMediaPixels;
}

////////////////////////////////////////////////////////////////////////////////
// virtual
bool LLMediaImplExample2::reset()
{
	if ( mMediaPixels )
	{
		delete [] mMediaPixels;
	};

	return true;
}

////////////////////////////////////////////////////////////////////////////////
// virtual
bool LLMediaImplExample2::setRequestedMediaSize( int width, int height )
{
	// we accept any size:
	return setMediaSize(width, height);
}
