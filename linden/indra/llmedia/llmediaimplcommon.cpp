/**
 * @file llmediaimplcommon.cpp
 * @brief Common impl functionality
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

#include "llmediaimplcommon.h"
#include "llmediaemitter.h"
#include "llmediaimplfactory.h"
#include "llmediaobserver.h"

#ifdef WIN32
	// platform specific includes needed before OpenGL header
	#include <windows.h>
	#include <GL/gl.h>
#elif defined(__APPLE__)
	// framework-style include path when building on the Mac.
	#include <OpenGL/gl.h>
#else	// Assume this is linux
	// Linux, MESA headers, but not necessarily assuming MESA runtime.
	// quotes so we get libraries/.../GL/ version
	#include "GL/gl.h"
#endif

////////////////////////////////////////////////////////////////////////////////
// virtual (derives from LLMediaBase)
LLMediaImplCommon::LLMediaImplCommon() :
	mMimeType( std::string() ),
	mInitialURL( std::string() ),
	mImplMaker( NULL ),
	mAutoScaled( false ),
	mMediaWidth( 0 ),
	mMediaPrevWidth( 0 ),
	mMediaHeight( 0 ),
	mMediaPrevHeight( 0 ),
	mMediaDepth( 0 ),
	mMediaPrevDepth( 0 ),
	mMediaRowSpan( 0 ),
	mMediaRequestedWidth( 0 ),
	mMediaRequestedHeight( 0 ),
	mCommand( LLMediaBase::COMMAND_NONE ),
	mStatus( LLMediaBase::STATUS_UNKNOWN ),
	mVolume( 0 ),
	mLooping( false )
{
}

////////////////////////////////////////////////////////////////////////////////
// virtual (derives from LLMediaBase)
LLMediaImplCommon::~LLMediaImplCommon()
{
}

////////////////////////////////////////////////////////////////////////////////
// virtual (derives from LLMediaBase)
bool LLMediaImplCommon::init()
{
	return false;
}

////////////////////////////////////////////////////////////////////////////////
// virtual (derives from LLMediaBase)
bool LLMediaImplCommon::reset()
{
	return false;
}

////////////////////////////////////////////////////////////////////////////////
// virtual (derives from LLMediaBase)
bool LLMediaImplCommon::setMimeType( const std::string mime_type )
{
	mMimeType = mime_type;

	return true;
}

////////////////////////////////////////////////////////////////////////////////
// virtual (derives from LLMediaBase)
std::string LLMediaImplCommon::getMimeType() const
{
	return mMimeType;
}

////////////////////////////////////////////////////////////////////////////////
// virtual (derives from LLMediaBase)
std::string LLMediaImplCommon::getMediaURL() const
{
	return mInitialURL;
}

////////////////////////////////////////////////////////////////////////////////
// virtual (derives from LLMediaBase)
std::string LLMediaImplCommon::getVersion()
{
	return std::string( "" );
}

////////////////////////////////////////////////////////////////////////////////
// virtual (derives from LLMediaBase)
bool LLMediaImplCommon::set404RedirectUrl( std::string redirect_url )
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////
// virtual (derives from LLMediaBase)
bool LLMediaImplCommon::clr404RedirectUrl()
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////
// virtual (derives from LLMediaBase)
bool LLMediaImplCommon::setBackgroundColor( unsigned int red, unsigned int green, unsigned int blue ) const
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////
// virtual (derives from LLMediaBase)
bool LLMediaImplCommon::setCaretColor( unsigned int red, unsigned int green, unsigned int blue ) const
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////
// virtual (derives from LLMediaBase)
bool LLMediaImplCommon::updateMedia()
{
	return false;
}

////////////////////////////////////////////////////////////////////////////////
// virtual (derives from LLMediaBase)
unsigned char* LLMediaImplCommon::getMediaData()
{
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// virtual (derives from LLMediaBase)
int LLMediaImplCommon::getMediaDataWidth() const
{
	return getMediaWidth();
}

////////////////////////////////////////////////////////////////////////////////
// virtual (derives from LLMediaBase)
int LLMediaImplCommon::getMediaDataHeight() const
{
	return getMediaHeight();
}


////////////////////////////////////////////////////////////////////////////////
// virtual (derives from LLMediaBase)
bool LLMediaImplCommon::setMediaSize( int media_width, int media_height )
{
	// if nothing changed, don't do anything
	if ( ( mMediaWidth == media_width ) &&
		 ( mMediaHeight == media_height ) )
		return false;

	// save old values so we can tell elsewhere if media size has changed
	mMediaPrevWidth = mMediaWidth;
	mMediaPrevHeight = mMediaHeight;

	mMediaWidth = media_width;
	mMediaHeight = media_height;

	// only fire an event if the width changed
	LLMediaEvent event( this );
	mEventEmitter.update( &LLMediaObserver::onMediaSizeChange, event );

	return true;
}

////////////////////////////////////////////////////////////////////////////////
// virtual (derives from LLMediaBase)
int LLMediaImplCommon::getMediaWidth() const
{
	return mMediaWidth;
}


////////////////////////////////////////////////////////////////////////////////
// virtual (derives from LLMediaBase)
int LLMediaImplCommon::getMediaHeight() const
{
	return mMediaHeight;
}

////////////////////////////////////////////////////////////////////////////////
// virtual (derives from LLMediaBase)
bool LLMediaImplCommon::setRequestedMediaSize(int width, int height)
{
	mMediaRequestedWidth = width;
	mMediaRequestedHeight = height;

	return true;
}

////////////////////////////////////////////////////////////////////////////////
// virtual (derives from LLMediaBase)
bool LLMediaImplCommon::setMediaDepth( int media_depth )
{
	// if nothing changed, don't do anything
	if ( mMediaDepth == media_depth )
		return false;

	// save old values so we can tell elsewhere if media size has changed
	mMediaPrevDepth = mMediaDepth;
	mMediaDepth = media_depth;

	// update value of rowspan too since it's based on media width & depth
	mMediaRowSpan = mMediaWidth * mMediaDepth;

	// only fire an event if the depth changed
	//LLMediaEvent event( this );
	//mEventEmitter.update( &LLMediaObserver::onMediaSizeChange, event );

	return true;
}

////////////////////////////////////////////////////////////////////////////////
// virtual (derives from LLMediaBase)
int LLMediaImplCommon::getMediaDepth() const
{
	return mMediaDepth;
}

////////////////////////////////////////////////////////////////////////////////
// virtual (derives from LLMediaBase)
int LLMediaImplCommon::getMediaBufferSize() const
{
	return mMediaRowSpan * mMediaHeight;
}

////////////////////////////////////////////////////////////////////////////////
// virtual (derives from LLMediaBase)
int LLMediaImplCommon::getTextureFormatInternal() const
{
	return LL_MEDIA_RGB;
}

////////////////////////////////////////////////////////////////////////////////
// virtual (derives from LLMediaBase)
int LLMediaImplCommon::getTextureFormatPrimary() const
{
	return LL_MEDIA_RGB;
}

////////////////////////////////////////////////////////////////////////////////
// virtual (derives from LLMediaBase)
int LLMediaImplCommon::getTextureFormatType() const
{
	return LL_MEDIA_UNSIGNED_BYTE;
}

////////////////////////////////////////////////////////////////////////////////
// virtual (derives from LLMediaBase)
bool LLMediaImplCommon::setVolume( float volume )
{
	mVolume = volume;

	return true;
}

////////////////////////////////////////////////////////////////////////////////
// virtual (derives from LLMediaBase)
float LLMediaImplCommon::getVolume() const
{
	return mVolume;
}

////////////////////////////////////////////////////////////////////////////////
// virtual (derives from LLMediaBase)
bool LLMediaImplCommon::addCommand( LLMediaBase::ECommand cmd )
{
	// eventually will be a std::queue so you can add multiple commands
	mCommand = cmd;

	return true;
}

////////////////////////////////////////////////////////////////////////////////
// virtual (derives from LLMediaBase)
bool LLMediaImplCommon::clearCommand()
{
	// eventually will be a std::queue so you can add multiple commands
	mCommand = LLMediaBase::COMMAND_NONE;

	return true;
}

////////////////////////////////////////////////////////////////////////////////
// virtual (derives from LLMediaBase)
bool LLMediaImplCommon::updateCommand()
{
	if ( nextCommand() == LLMediaBase::COMMAND_START )
	{
		setStatus( LLMediaBase::STATUS_STARTED );
		clearCommand();
	};

	if ( nextCommand() == LLMediaBase::COMMAND_STOP )
	{
		setStatus( LLMediaBase::STATUS_STOPPED );
		clearCommand();
	};

	if ( nextCommand() == LLMediaBase::COMMAND_PAUSE  )
	{
		setStatus( LLMediaBase::STATUS_PAUSED );
		clearCommand();
	};

	return true;
}

////////////////////////////////////////////////////////////////////////////////
// non-virtual (only impls use this)
LLMediaBase::ECommand LLMediaImplCommon::nextCommand()
{
	return mCommand;
}

////////////////////////////////////////////////////////////////////////////////
// virtual (derives from LLMediaBase)
LLMediaBase::EStatus LLMediaImplCommon::getStatus()
{
	return mStatus;
}

////////////////////////////////////////////////////////////////////////////////
// non-virtual (only impls set this)
bool LLMediaImplCommon::setStatus( LLMediaBase::EStatus status )
{
	mStatus = status;

	return true;
}

////////////////////////////////////////////////////////////////////////////////
// virtual (derives from LLMediaBase)
bool LLMediaImplCommon::seek( double time )
{
	return false;
}

////////////////////////////////////////////////////////////////////////////////
// virtual (derives from LLMediaBase)
bool LLMediaImplCommon::navigateTo( const std::string url )
{
	return false;
}

////////////////////////////////////////////////////////////////////////////////
// virtual (derives from LLMediaBase)
bool LLMediaImplCommon::setAutoScaled( bool auto_scaled )
{
	mAutoScaled = auto_scaled;

	return true;
}

////////////////////////////////////////////////////////////////////////////////
// virtual (derives from LLMediaBase)
bool LLMediaImplCommon::isAutoScaled() const
{
	return mAutoScaled;
}

////////////////////////////////////////////////////////////////////////////////
// virtual (derives from LLMediaBase)
bool LLMediaImplCommon::mouseDown( int x_pos, int y_pos )
{
	return false;
}

////////////////////////////////////////////////////////////////////////////////
// virtual (derives from LLMediaBase)
bool LLMediaImplCommon::mouseUp( int x_pos, int y_pos )
{
	return false;
}

////////////////////////////////////////////////////////////////////////////////
// virtual (derives from LLMediaBase)
bool LLMediaImplCommon::mouseMove( int x_pos, int y_pos )
{
	return false;
}

////////////////////////////////////////////////////////////////////////////////
// virtual (derives from LLMediaBase)
bool LLMediaImplCommon::keyPress( int key_code )
{
	return false;
}

////////////////////////////////////////////////////////////////////////////////
// virtual (derives from LLMediaBase)
bool LLMediaImplCommon::scrollByLines( int lines )
{
	return false;
}

////////////////////////////////////////////////////////////////////////////////
// virtual (derives from LLMediaBase)
bool LLMediaImplCommon::focus( bool focus )
{
	return false;
}

////////////////////////////////////////////////////////////////////////////////
// virtual (derives from LLMediaBase)
bool LLMediaImplCommon::unicodeInput( unsigned long uni_char )
{
	return false;
}

////////////////////////////////////////////////////////////////////////////////
// virtual (derives from LLMediaBase)
bool LLMediaImplCommon::mouseLeftDoubleClick( int x_pos, int y_pos )
{
	return false;
}

////////////////////////////////////////////////////////////////////////////////
// virtual (derives from LLMediaBase)
bool LLMediaImplCommon::navigateForward()
{
	return false;
}

////////////////////////////////////////////////////////////////////////////////
// virtual (derives from LLMediaBase)
bool LLMediaImplCommon::navigateBack()
{
	return false;
}

////////////////////////////////////////////////////////////////////////////////
// virtual (derives from LLMediaBase)
bool LLMediaImplCommon::canNavigateForward()
{
	return false;
}

////////////////////////////////////////////////////////////////////////////////
// virtual (derives from LLMediaBase)
bool LLMediaImplCommon::canNavigateBack()
{
	return false;
}

////////////////////////////////////////////////////////////////////////////////
// virtual (derives from LLMediaBase)
bool LLMediaImplCommon::enableCookies( bool enable )
{
	return false;
}

////////////////////////////////////////////////////////////////////////////////
// virtual (derives from LLMediaBase)
bool LLMediaImplCommon::clearCache()
{
	return false;
}

////////////////////////////////////////////////////////////////////////////////
// virtual (derives from LLMediaBase)
bool LLMediaImplCommon::clearCookies()
{
	return false;
}

////////////////////////////////////////////////////////////////////////////////
// virtual (derives from LLMediaBase)
bool LLMediaImplCommon::enableProxy(bool enable, std::string proxy_host_name, int proxy_port)
{
	return false;
}
////////////////////////////////////////////////////////////////////////////////
//
bool LLMediaImplCommon::addObserver( LLMediaObserver* subject )
{
	return mEventEmitter.addObserver( subject );
}

////////////////////////////////////////////////////////////////////////////////
//
bool LLMediaImplCommon::remObserver( LLMediaObserver* subject )
{
	return mEventEmitter.remObserver( subject );
}

////////////////////////////////////////////////////////////////////////////////
//
void LLMediaImplCommon::setImplMaker(LLMediaImplMakerBase* impl_maker)
{
	mImplMaker = impl_maker;
}
////////////////////////////////////////////////////////////////////////////////
//
bool LLMediaImplCommon::supportsMediaType(std::string scheme, std::string type)
{
	int idx1 = type.find("/");
	int len = (idx1 == std::string::npos) ? 0 : idx1;
	std::string category = type.substr(0,len);

	return mImplMaker->supportsScheme(scheme) ||
			mImplMaker->supportsMimeType(type) ||
			mImplMaker->supportsMimeTypeCategory(category);
}
