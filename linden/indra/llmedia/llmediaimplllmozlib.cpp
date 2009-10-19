/**
 * @file llmediaimplllmozlib.cpp
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

#include "llmediaimplllmozlib.h"

#if LL_LLMOZLIB_ENABLED

#include "llmediaimplregister.h"
#include "llmediamanager.h"

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


#include <locale.h>


#include <iostream>

// register this impl with media manager factory
static LLMediaImplRegister sLLMediaImplLLMozLibReg( "LLMediaImplLLMozLib", new LLMediaImplLLMozLibMaker() );

///////////////////////////////////////////////////////////////////////////////
//
LLMediaImplLLMozLibMaker::LLMediaImplLLMozLibMaker()
{
	// Register to handle the mime category
	mMimeTypes.push_back( "image/svg+xml" );
	mMimeTypeCategories.push_back( "text" );
#if !LL_QUICKTIME_ENABLED
	mMimeTypeCategories.push_back( "image" );
#endif
}

///////////////////////////////////////////////////////////////////////////////
//
LLMediaImplLLMozLib::LLMediaImplLLMozLib() :
	mBrowserWindowWidth( 800 ),
	mBrowserWindowHeight( 600 ),
	mMediaDataWidth( 0 ),
	mMediaDataHeight( 0 ),
	mWindowId( 0 ),
	mNeedsUpdate( false )
{
	setRequestedMediaSize( mBrowserWindowWidth, mBrowserWindowHeight );

	setMediaDepth( 4 );
}

////////////////////////////////////////////////////////////////////////////////
// (static) super-initialization - called once at application startup
bool LLMediaImplLLMozLib::startup( LLMediaManagerData* init_data )
{

	// Yuck, Mozilla's GTK callbacks play with the locale - push/pop
	// the locale to protect it, as exotic/non-C locales
	// causes our code lots of general critical weirdness
	// and crashness. (SL-35450)
	static std::string saved_locale;
	saved_locale = setlocale(LC_ALL, NULL);


	bool result = LLMozLib::getInstance()->init( init_data->getBrowserApplicationDir(),
											init_data->getBrowserComponentDir(),
											  init_data->getBrowserProfileDir(),
												init_data->getBrowserParentWindow() );


	setlocale(LC_ALL, saved_locale.c_str() );


	return result;
}

////////////////////////////////////////////////////////////////////////////////
// (static) super-uninitialization - called once at application closedown
bool LLMediaImplLLMozLib::closedown()
{
	// name discrepancy - this reset actually shuts down LLMozLib
	LLMozLib::getInstance()->reset();

	return true;
}

////////////////////////////////////////////////////////////////////////////////
// (static)
bool LLMediaImplLLMozLib::setBrowserUserAgent(std::string user_agent)
{
	// append special string to the embedded browser user agent string
	LLMozLib::getInstance()->setBrowserAgentId(user_agent);
	return true;
}

////////////////////////////////////////////////////////////////////////////////
// virtual
bool LLMediaImplLLMozLib::init()
{
	// if mWindowId is non-0, it's we already called init() and shouldn't call it again
	// (::reset() will zero this value)
	if ( mWindowId )
		return false;

#if LL_LINUX
	static std::string saved_locale;
	saved_locale = setlocale(LC_ALL, NULL);
#endif // LL_LINUX

	mWindowId = LLMozLib::getInstance()->createBrowserWindow( mBrowserWindowWidth, mBrowserWindowHeight );

	LLMozLib::getInstance()->setSize( mWindowId, mBrowserWindowWidth, mBrowserWindowHeight );

	LLMozLib::getInstance()->setBackgroundColor( mWindowId, 0x00, 0x00, 0x00 );

	LLMozLib::getInstance()->addObserver( mWindowId, this );

	// plugins only work with some client-side hackery and they cause
	// exception handling issues (DEV-10020) so we turn them off
	LLMozLib::getInstance()->enablePlugins( false );

	// second life client needs the bitmap flipped
	LLMozLib::getInstance()->flipWindow( mWindowId, true );

	// set media depth now we have created a browser window and know what it is
	setMediaDepth( LLMozLib::getInstance()->getBrowserDepth( mWindowId ) );

#if LL_LINUX
	setlocale(LC_ALL, saved_locale.c_str() );
#endif // LL_LINUX

	return true;
}

////////////////////////////////////////////////////////////////////////////////
// virtual
std::string LLMediaImplLLMozLib::getVersion()
{
	std::string version_string = "[" + sLLMediaImplLLMozLibReg.getImplName() + "] - " + LLMozLib::getInstance()->getVersion();

	return version_string;
}

////////////////////////////////////////////////////////////////////////////////
// virtual
bool LLMediaImplLLMozLib::set404RedirectUrl(  std::string redirect_url )
{
	return LLMozLib::getInstance()->set404RedirectUrl( mWindowId, redirect_url );
}

////////////////////////////////////////////////////////////////////////////////
// virtual

bool LLMediaImplLLMozLib::clr404RedirectUrl()
{
	return LLMozLib::getInstance()->clr404RedirectUrl( mWindowId );
}

////////////////////////////////////////////////////////////////////////////////
// virtual
bool LLMediaImplLLMozLib::setBackgroundColor( unsigned int red, unsigned int green, unsigned int blue ) const
{
	return LLMozLib::getInstance()->setBackgroundColor( mWindowId, red, green, blue );
}

////////////////////////////////////////////////////////////////////////////////
// virtual
bool LLMediaImplLLMozLib::setCaretColor( unsigned int red, unsigned int green, unsigned int blue ) const
{
	return LLMozLib::getInstance()->setCaretColor( mWindowId, red, green, blue );
}

////////////////////////////////////////////////////////////////////////////////
// virtual
bool LLMediaImplLLMozLib::navigateTo( const std::string url )
{
	// pass url to llmozlib
	LLMozLib::getInstance()->navigateTo( mWindowId, url );

	// emit event with size change to kick things off
	LLMediaEvent event( this );
	mEventEmitter.update( &LLMediaObserver::onMediaSizeChange, event );

	// not that useful right now but maybe later
	return true;
}

////////////////////////////////////////////////////////////////////////////////
// virtual
bool LLMediaImplLLMozLib::updateMedia()
{
	if ( getStatus() == LLMediaBase::STATUS_STARTED )
	{
		// if flag set, the page changed and we need to update
		if ( mNeedsUpdate )
		{
			// snap browser pixels
			LLMozLib::getInstance()->grabBrowserWindow( mWindowId );

			// update media width - rendering the page can change it
			mMediaDataWidth = LLMozLib::getInstance()->getBrowserRowSpan( mWindowId ) / getMediaDepth();
			mMediaDataHeight = LLMozLib::getInstance()->getBrowserHeight( mWindowId );

			// emit an event to say that something in the media stream changed
			LLMediaEvent event( this );
			mEventEmitter.update( &LLMediaObserver::onMediaContentsChange, event );

			// flag that we've done the update and one isn't needed next frame
			mNeedsUpdate = false;
		};
	};

	// update the state (e.g. transport controls) state
	updateState();

	return false;
}

////////////////////////////////////////////////////////////////////////////////
//
bool LLMediaImplLLMozLib::updateState()
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

	if ( nextCommand() == LLMediaBase::COMMAND_BACK  )
	{
		setStatus( LLMediaBase::STATUS_STARTED );
		LLMozLib::getInstance()->navigateBack( mWindowId );
		clearCommand();
	};

	if ( nextCommand() == LLMediaBase::COMMAND_FORWARD  )
	{
		setStatus( LLMediaBase::STATUS_STARTED );
		LLMozLib::getInstance()->navigateForward( mWindowId );
		clearCommand();
	};

	return true;
}

////////////////////////////////////////////////////////////////////////////////
// virtual
void LLMediaImplLLMozLib::onPageChanged( const EventType& eventIn )
{
	// force an update when the contents of the page changes
	mNeedsUpdate = true;
}

////////////////////////////////////////////////////////////////////////////////
// virtual
void LLMediaImplLLMozLib::onClickLinkHref( const EventType& eventIn )
{
	LLMediaEvent event( this, eventIn.getStringValue(), eventIn.getStringValue2() );
	mEventEmitter.update( &LLMediaObserver::onClickLinkHref, event );
}

////////////////////////////////////////////////////////////////////////////////
// virtual
void LLMediaImplLLMozLib::onClickLinkNoFollow( const EventType& eventIn )
{
	LLMediaEvent event( this, eventIn.getStringValue() );
	mEventEmitter.update( &LLMediaObserver::onClickLinkNoFollow, event );
}

////////////////////////////////////////////////////////////////////////////////
// virtual
void LLMediaImplLLMozLib::onUpdateProgress( const EventType& eventIn )
{
	LLMediaEvent event( this, eventIn.getIntValue() );
	mEventEmitter.update( &LLMediaObserver::onUpdateProgress, event );
}

////////////////////////////////////////////////////////////////////////////////
// virtual
void LLMediaImplLLMozLib::onStatusTextChange( const EventType& eventIn )
{
	LLMediaEvent event( this, eventIn.getStringValue() );
	mEventEmitter.update( &LLMediaObserver::onStatusTextChange, event );
}

////////////////////////////////////////////////////////////////////////////////
// virtual
void LLMediaImplLLMozLib::onLocationChange( const EventType& eventIn )
{
	LLMediaEvent event( this, eventIn.getEventUri() );
	mEventEmitter.update( &LLMediaObserver::onLocationChange, event );
}

////////////////////////////////////////////////////////////////////////////////
// virtual
void LLMediaImplLLMozLib::onNavigateBegin( const EventType& eventIn )
{
	LLMediaEvent event( this, eventIn.getEventUri() );
	mEventEmitter.update( &LLMediaObserver::onNavigateBegin, event );
}

////////////////////////////////////////////////////////////////////////////////
// virtual
void LLMediaImplLLMozLib::onNavigateComplete( const EventType& eventIn )
{
	// force an update when the page is finished
	mNeedsUpdate = true;

	// pass in url and HTML response code (200/404 etc.)
	LLMediaEvent event( this, eventIn.getEventUri(), eventIn.getIntValue() );
	mEventEmitter.update( &LLMediaObserver::onNavigateComplete, event );
}

////////////////////////////////////////////////////////////////////////////////
// virtual
unsigned char* LLMediaImplLLMozLib::getMediaData()
{
	return (unsigned char*)LLMozLib::getInstance()->getBrowserWindowPixels( mWindowId );
}

// helper func to compute size of media data
bool LLMediaImplLLMozLib::recomputeSizes()
{
	int new_width = mMediaRequestedWidth;
	int new_height = mMediaRequestedHeight;

	if (new_width < 0)
		new_width = 512;

	if (new_height < 0)
		new_height = 512;

	if (mAutoScaled)
	{
		new_width = LLMediaManager::textureWidthFromMediaWidth( new_width );
		new_height = LLMediaManager::textureHeightFromMediaHeight( new_height );
	}

	bool status = LLMozLib::getInstance()->setSize( mWindowId, new_width, new_height );

	if (status)
		setMediaSize(new_width, new_height);

	return status;
}


////////////////////////////////////////////////////////////////////////////////
// virtual
int LLMediaImplLLMozLib::getMediaDataWidth() const
{
	return mMediaDataWidth;
}

////////////////////////////////////////////////////////////////////////////////
// virtual
int LLMediaImplLLMozLib::getMediaDataHeight() const
{
	return mMediaDataHeight;
}

////////////////////////////////////////////////////////////////////////////////
// virtual
bool LLMediaImplLLMozLib::setRequestedMediaSize(int width, int height)
{
	LLMediaImplCommon::setRequestedMediaSize(width, height);

	return recomputeSizes();
}

////////////////////////////////////////////////////////////////////////////////
// virtual
bool LLMediaImplLLMozLib::setAutoScaled( bool auto_scaled )
{
	LLMediaImplCommon::setAutoScaled(auto_scaled);

	return recomputeSizes();
}


////////////////////////////////////////////////////////////////////////////////
// virtual
int LLMediaImplLLMozLib::getTextureFormatPrimary() const
{
#if defined(__APPLE__) || defined(MACOSX)
	return GL_BGRA_EXT;
#else
	return LLMozLib::getInstance()->getBrowserDepth( mWindowId ) == 3 ? GL_BGR_EXT : GL_BGRA_EXT;
#endif
}

////////////////////////////////////////////////////////////////////////////////
// virtual
int LLMediaImplLLMozLib::getTextureFormatType() const
{
#if defined(__APPLE__) || defined(MACOSX)
	#ifdef __BIG_ENDIAN__
		return GL_UNSIGNED_INT_8_8_8_8_REV;
	#else
		return GL_UNSIGNED_INT_8_8_8_8;
	#endif
#else
	return GL_UNSIGNED_BYTE;
#endif
}

////////////////////////////////////////////////////////////////////////////////
// virtual
bool LLMediaImplLLMozLib::mouseDown( int x_pos, int y_pos )
{
	return LLMozLib::getInstance()->mouseDown( mWindowId, x_pos, y_pos );
}

////////////////////////////////////////////////////////////////////////////////
// virtual
bool LLMediaImplLLMozLib::mouseUp( int x_pos, int y_pos )
{
	LLMozLib::getInstance()->mouseUp( mWindowId, x_pos, y_pos );

	// this seems better than sending focus on mouse down (still need to improve this)
	LLMozLib::getInstance()->focusBrowser( mWindowId, true );

	return true;
}

////////////////////////////////////////////////////////////////////////////////
// virtual
bool LLMediaImplLLMozLib::mouseMove( int x_pos, int y_pos )
{
	return LLMozLib::getInstance()->mouseMove( mWindowId, x_pos, y_pos );
}

////////////////////////////////////////////////////////////////////////////////
// virtual
bool LLMediaImplLLMozLib::keyPress( int key_code )
{
    // We don't have to deal with printable characters here - they should
	// go through handleUnicodeChar().  This table could be more complete
	// than it is, but I think this covers all of the important
	// non-printables.

	unsigned long moz_key;

	switch(key_code)
	{
	case LL_MEDIA_KEY_BACKSPACE:
		moz_key = LL_DOM_VK_BACK_SPACE; break;
	case LL_MEDIA_KEY_TAB:
		moz_key = LL_DOM_VK_TAB; break;
	case LL_MEDIA_KEY_RETURN:
		moz_key = LL_DOM_VK_RETURN; break;
	case LL_MEDIA_KEY_PAD_RETURN:
		moz_key = LL_DOM_VK_ENTER; break;
	case LL_MEDIA_KEY_ESCAPE:
		moz_key = LL_DOM_VK_ESCAPE; break;
	case LL_MEDIA_KEY_PAGE_UP:
		moz_key = LL_DOM_VK_PAGE_UP; break;
	case LL_MEDIA_KEY_PAGE_DOWN:
		moz_key = LL_DOM_VK_PAGE_DOWN; break;
	case LL_MEDIA_KEY_END:
		moz_key = LL_DOM_VK_END; break;
	case LL_MEDIA_KEY_HOME:
		moz_key = LL_DOM_VK_HOME; break;
	case LL_MEDIA_KEY_LEFT:
		moz_key = LL_DOM_VK_LEFT; break;
	case LL_MEDIA_KEY_UP:
		moz_key = LL_DOM_VK_UP; break;
	case LL_MEDIA_KEY_RIGHT:
		moz_key = LL_DOM_VK_RIGHT; break;
	case LL_MEDIA_KEY_DOWN:
		moz_key = LL_DOM_VK_DOWN; break;
	case LL_MEDIA_KEY_INSERT:
		moz_key = LL_DOM_VK_INSERT; break;
	case LL_MEDIA_KEY_DELETE:
		moz_key = LL_DOM_VK_DELETE; break;

	default:
		return false; // don't know how to map this key.
	}

	return LLMozLib::getInstance()->keyPress( mWindowId, moz_key );
}

////////////////////////////////////////////////////////////////////////////////
// virtual
bool LLMediaImplLLMozLib::scrollByLines( int lines )
{
	return LLMozLib::getInstance()->scrollByLines(mWindowId, lines);
}

////////////////////////////////////////////////////////////////////////////////
// virtual
bool LLMediaImplLLMozLib::focus( bool focus )
{
	return LLMozLib::getInstance()->focusBrowser(mWindowId, focus);
}

////////////////////////////////////////////////////////////////////////////////
// virtual
bool LLMediaImplLLMozLib::unicodeInput( unsigned long uni_char )
{
	return LLMozLib::getInstance()->unicodeInput(mWindowId, uni_char);
}

////////////////////////////////////////////////////////////////////////////////
// virtual
bool LLMediaImplLLMozLib::mouseLeftDoubleClick( int x_pos, int y_pos )
{
	return LLMozLib::getInstance()->mouseLeftDoubleClick( mWindowId, x_pos, y_pos );
}

////////////////////////////////////////////////////////////////////////////////
// virtual
bool LLMediaImplLLMozLib::navigateForward()
{
	return LLMozLib::getInstance()->navigateForward(mWindowId);
}

////////////////////////////////////////////////////////////////////////////////
// virtual
bool LLMediaImplLLMozLib::navigateBack()
{
	return LLMozLib::getInstance()->navigateBack(mWindowId);
}

////////////////////////////////////////////////////////////////////////////////
// virtual
bool LLMediaImplLLMozLib::canNavigateForward()
{
	return LLMozLib::getInstance()->canNavigateForward(mWindowId);
}

////////////////////////////////////////////////////////////////////////////////
// virtual
bool LLMediaImplLLMozLib::canNavigateBack()
{
	return LLMozLib::getInstance()->canNavigateBack(mWindowId);
}

////////////////////////////////////////////////////////////////////////////////
// virtual
bool LLMediaImplLLMozLib::enableCookies(bool enable)
{
	return LLMozLib::getInstance()->enableCookies(enable);
}

////////////////////////////////////////////////////////////////////////////////
// virtual
bool LLMediaImplLLMozLib::enableProxy(bool enable, std::string proxy_host_name, int proxy_port)
{
	return LLMozLib::getInstance()->enableProxy(enable, proxy_host_name, proxy_port);
}

////////////////////////////////////////////////////////////////////////////////
// virtual
bool LLMediaImplLLMozLib::clearCache()
{
	return LLMozLib::getInstance()->clearCache();
}

////////////////////////////////////////////////////////////////////////////////
// virtual
bool LLMediaImplLLMozLib::clearCookies()
{
	return LLMozLib::getInstance()->clearAllCookies();
}

////////////////////////////////////////////////////////////////////////////////
// virtual
bool LLMediaImplLLMozLib::reset()
{
	LLMozLib::getInstance()->remObserver( mWindowId, this );

	LLMozLib::getInstance()->destroyBrowserWindow( mWindowId );

	mWindowId = 0;

	return true;
}

#endif	// LL_LLMOZLIB_ENABLED
