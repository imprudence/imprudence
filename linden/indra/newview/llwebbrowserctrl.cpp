/**
 * @file llwebbrowserctrl.cpp
 * @brief Web browser UI control
 *
 * Copyright (c) 2006-2007, Linden Research, Inc.
 * 
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

#include "llviewerprecompiledheaders.h"

#if LL_LIBXUL_ENABLED

#include "llwebbrowserctrl.h"
#include "llviewborder.h"
#include "llviewerwindow.h"
#include "llfloaterworldmap.h"
#include "llfocusmgr.h"
#include "llweb.h"
#include "viewer.h"
#include "llpanellogin.h"

// Setting the mozilla buffer width to 2048 exactly doesn't work, since it pads its rowbytes a bit, pushing the texture width over 2048.
// 2000 should give enough headroom for any amount of padding it cares to add.
const S32 MAX_DIMENSION = 2000;
const S32 MAX_TEXTURE_DIMENSION = 2048;

LLWebBrowserCtrl::LLWebBrowserCtrl( const std::string& name, const LLRect& rect ) :
	LLUICtrl( name, rect, FALSE, NULL, NULL ),
	mEmbeddedBrowserWindowId( 0 ),
	mTextureDepthBytes( 4 ),
	mBorder( 0 ),
	mFrequentUpdates( true ),
	mOpenLinksInExternalBrowser( false ),
	mHomePageUrl( "" )
{
	// create a new browser window
	mEmbeddedBrowserWindowId = LLMozLib::getInstance()->createBrowserWindow( gViewerWindow->getPlatformWindow(), mRect.getWidth(), mRect.getHeight() );

	// change color to black so transisitons aren't so noticable (this should be in XML eventually)
	LLMozLib::getInstance()->setBackgroundColor( mEmbeddedBrowserWindowId, 0x00, 0x00, 0x00 );

	// observe the browser so we can trap HREF events)
	LLMozLib::getInstance()->addObserver( mEmbeddedBrowserWindowId, this );

	// create a new texture (based on LLDynamic texture) that will be used to display the output
	mWebBrowserImage = new LLWebBrowserTexture( mRect.getWidth(), mRect.getHeight(), this, mEmbeddedBrowserWindowId );

	LLRect border_rect( 0, mRect.getHeight() + 2, mRect.getWidth() + 2, 0 );
	mBorder = new LLViewBorder( "web control border", border_rect, LLViewBorder::BEVEL_IN );
	addChild( mBorder );
}

////////////////////////////////////////////////////////////////////////////////
// note: this is now a singleton and destruction happens via initClass() now
LLWebBrowserCtrl::~LLWebBrowserCtrl()
{
	LLMozLib::getInstance()->remObserver( mEmbeddedBrowserWindowId, this );

	LLMozLib::getInstance()->destroyBrowserWindow( mEmbeddedBrowserWindowId );

	if ( mWebBrowserImage )
	{
		delete mWebBrowserImage;
		mWebBrowserImage = 0;
	};
}

////////////////////////////////////////////////////////////////////////////////
//
bool LLWebBrowserCtrl::addObserver( LLWebBrowserCtrlObserver* subjectIn )
{
	return mEventEmitter.addObserver( subjectIn );
}

////////////////////////////////////////////////////////////////////////////////
//
bool LLWebBrowserCtrl::remObserver( LLWebBrowserCtrlObserver* subjectIn )
{
	return mEventEmitter.remObserver( subjectIn );
}

////////////////////////////////////////////////////////////////////////////////
//
void LLWebBrowserCtrl::setBorderVisible( BOOL border_visible )
{
	if ( mBorder )
	{
		mBorder->setVisible( border_visible );
	};
};

////////////////////////////////////////////////////////////////////////////////
// allows access to the raw web browser window by consumers of this class
void LLWebBrowserCtrl::setOpenInExternalBrowser( bool valIn )
{
	mOpenLinksInExternalBrowser = valIn;
};

////////////////////////////////////////////////////////////////////////////////
//
BOOL LLWebBrowserCtrl::handleHover( S32 x, S32 y, MASK mask )
{
	LLMozLib::getInstance()->mouseMove( mEmbeddedBrowserWindowId, x, mRect.getHeight() - y );

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
//
BOOL LLWebBrowserCtrl::handleScrollWheel( S32 x, S32 y, S32 clicks )
{
	LLMozLib::getInstance()->scrollByLines( mEmbeddedBrowserWindowId, clicks );

	// note: this isn't really necessary right now since the page is updated
	// on a timer but if that becomes too burdensome and the page is only updated
	// once after load then this will be nexessary
	LLMozLib::getInstance()->grabBrowserWindow( mEmbeddedBrowserWindowId );

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
//
BOOL LLWebBrowserCtrl::handleMouseUp( S32 x, S32 y, MASK mask )
{
	LLMozLib::getInstance()->mouseUp( mEmbeddedBrowserWindowId, x, mRect.getHeight() - y );

	gViewerWindow->setMouseCapture( 0, 0 );

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
//
BOOL LLWebBrowserCtrl::handleMouseDown( S32 x, S32 y, MASK mask )
{
	LLMozLib::getInstance()->mouseDown( mEmbeddedBrowserWindowId, x, mRect.getHeight() - y );

	gViewerWindow->setMouseCapture( this, 0 );

	setFocus( TRUE );

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
//
void LLWebBrowserCtrl::onFocusReceived()
{
	LLMozLib::getInstance()->focusBrowser( mEmbeddedBrowserWindowId, true );

	LLUICtrl::onFocusReceived();
}

////////////////////////////////////////////////////////////////////////////////
//
void LLWebBrowserCtrl::onFocusLost()
{
	LLMozLib::getInstance()->focusBrowser( mEmbeddedBrowserWindowId, false );

	gViewerWindow->focusClient();
}

////////////////////////////////////////////////////////////////////////////////
//
BOOL LLWebBrowserCtrl::handleKey( KEY key, MASK mask, BOOL called_from_parent )
{
	LLMozLib::getInstance()->keyPress( mEmbeddedBrowserWindowId, key );

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////
//
void LLWebBrowserCtrl::onVisibilityChange ( BOOL curVisibilityIn )
{
	// set state of frequent updates automatically if visibility changes
	if ( curVisibilityIn )
	{
		mFrequentUpdates = true;
	}
	else
	{
		mFrequentUpdates = false;
	};
}

////////////////////////////////////////////////////////////////////////////////
//
void LLWebBrowserCtrl::reshape( S32 width, S32 height, BOOL called_from_parent )
{
	// when floater is minimized, these sizes are negative
	if ( height > 0 && width > 0 )
		mWebBrowserImage->resize( width, height );

	LLUICtrl::reshape( width, height, called_from_parent );
}

////////////////////////////////////////////////////////////////////////////////
//
void LLWebBrowserCtrl::navigateBack()
{
	LLMozLib::getInstance()->navigateBack( mEmbeddedBrowserWindowId );
}

////////////////////////////////////////////////////////////////////////////////
//
void LLWebBrowserCtrl::navigateForward()
{
	LLMozLib::getInstance()->navigateForward( mEmbeddedBrowserWindowId );
}

////////////////////////////////////////////////////////////////////////////////
//
bool LLWebBrowserCtrl::canNavigateBack()
{
	return LLMozLib::getInstance()->canNavigateBack( mEmbeddedBrowserWindowId );
}

////////////////////////////////////////////////////////////////////////////////
//
bool LLWebBrowserCtrl::canNavigateForward()
{
	return LLMozLib::getInstance()->canNavigateForward( mEmbeddedBrowserWindowId );
}

////////////////////////////////////////////////////////////////////////////////
//
void LLWebBrowserCtrl::navigateTo( std::string urlIn )
{
	const std::string protocol( "secondlife://" );

	// don't browse to anything that starts with secondlife://
	if ( urlIn.length() >= protocol.length() )
	{
		if ( LLString::compareInsensitive( urlIn.substr( 0, protocol.length() ).c_str(), protocol.c_str() ) != 0 )
		{
			LLMozLib::getInstance()->navigateTo( mEmbeddedBrowserWindowId, urlIn );
		}
	}
	else
	{
		LLMozLib::getInstance()->navigateTo( mEmbeddedBrowserWindowId, urlIn );
	}
}

////////////////////////////////////////////////////////////////////////////////
//
void LLWebBrowserCtrl::navigateHome()
{
	if( mHomePageUrl.length() )
	{
		LLMozLib::getInstance()->navigateTo( mEmbeddedBrowserWindowId, mHomePageUrl );
	};
}

////////////////////////////////////////////////////////////////////////////////
//
void LLWebBrowserCtrl::setHomePageUrl( const std::string urlIn )
{
	mHomePageUrl = urlIn;
}

////////////////////////////////////////////////////////////////////////////////
//
std::string LLWebBrowserCtrl::getHomePageUrl()
{
	return 	mHomePageUrl;
}

////////////////////////////////////////////////////////////////////////////////
//
void LLWebBrowserCtrl::draw()
{
	if ( ! getVisible() )
		return;

	// NOTE: optimization needed here - probably only need to do this once
	// unless tearoffs change the parent which they problably do.
	LLUICtrl* ptr = (LLUICtrl*)findRootMostFocusRoot();
	if ( ptr && ptr->hasFocus() )
	{
		setFrequentUpdates( true );
	}
	else
	{
		setFrequentUpdates( false );
	};

	// alpha off for this
	LLGLSUIDefault gls_ui;
	LLGLDisable gls_alphaTest( GL_ALPHA_TEST );

	// scale texture to fit the space using texture coords
	mWebBrowserImage->bindTexture();
	glColor4fv( LLColor4::white.mV );
	F32 max_u = ( F32 )mWebBrowserImage->getBrowserWidth() / ( F32 )mWebBrowserImage->getWidth();
	F32 max_v = ( F32 )mWebBrowserImage->getBrowserHeight() / ( F32 )mWebBrowserImage->getHeight();

	// draw the browser
	glBlendFunc( GL_ONE, GL_ZERO );
	glBegin( GL_QUADS );
	{
		glTexCoord2f( max_u, 0.f );
		glVertex2i( mRect.getWidth(), mRect.getHeight() );

		glTexCoord2f( 0.f, 0.f );
		glVertex2i( 0, mRect.getHeight() );

		glTexCoord2f( 0.f, max_v );
		glVertex2i( 0, 0 );

		glTexCoord2f( max_u, max_v );
		glVertex2i( mRect.getWidth(), 0 );
	}
	glEnd();
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA  );

	// highlight if keyboard focus here. (TODO: this needs some work)
	if ( mBorder->getVisible() )
		mBorder->setKeyboardFocusHighlight( gFocusMgr.childHasKeyboardFocus( this ) );

	LLUICtrl::draw();
}

////////////////////////////////////////////////////////////////////////////////
// virtual
void LLWebBrowserCtrl::onNavigateBegin( const EventType& eventIn )
{
	LLWebBrowserCtrlEvent event;
	mEventEmitter.update( &LLWebBrowserCtrlObserver::onNavigateBegin, event );
}

////////////////////////////////////////////////////////////////////////////////
// virtual
void LLWebBrowserCtrl::onNavigateComplete( const EventType& eventIn )
{
	// chain this event on to observers of an instance of LLWebBrowserCtrl
	LLWebBrowserCtrlEvent event;
	mEventEmitter.update( &LLWebBrowserCtrlObserver::onNavigateComplete, event );
}

////////////////////////////////////////////////////////////////////////////////
// virtual
void LLWebBrowserCtrl::onUpdateProgress( const EventType& eventIn )
{
	// chain this event on to observers of an instance of LLWebBrowserCtrl
	LLWebBrowserCtrlEvent event( eventIn.getIntValue() );
	mEventEmitter.update( &LLWebBrowserCtrlObserver::onUpdateProgress, event );
}

////////////////////////////////////////////////////////////////////////////////
// virtual
void LLWebBrowserCtrl::onStatusTextChange( const EventType& eventIn )
{
	// chain this event on to observers of an instance of LLWebBrowserCtrl
	LLWebBrowserCtrlEvent event( eventIn.getStringValue() );
	mEventEmitter.update( &LLWebBrowserCtrlObserver::onStatusTextChange, event );
}

////////////////////////////////////////////////////////////////////////////////
// virtual
void LLWebBrowserCtrl::onLocationChange( const EventType& eventIn )
{
	// chain this event on to observers of an instance of LLWebBrowserCtrl
	LLWebBrowserCtrlEvent event( eventIn.getStringValue() );
	mEventEmitter.update( &LLWebBrowserCtrlObserver::onLocationChange, event );
}

////////////////////////////////////////////////////////////////////////////////
// virtual
void LLWebBrowserCtrl::onClickLinkHref( const EventType& eventIn )
{
	const std::string protocol( "http://" );

	if( mOpenLinksInExternalBrowser )
		if ( eventIn.getStringValue().length() )
			if ( LLString::compareInsensitive( eventIn.getStringValue().substr( 0, protocol.length() ).c_str(), protocol.c_str() ) == 0 )
				LLWeb::loadURL( eventIn.getStringValue() );

	// chain this event on to observers of an instance of LLWebBrowserCtrl
	LLWebBrowserCtrlEvent event( eventIn.getStringValue() );
	mEventEmitter.update( &LLWebBrowserCtrlObserver::onClickLinkHref, event );
}

////////////////////////////////////////////////////////////////////////////////
// virtual
void LLWebBrowserCtrl::onClickLinkSecondLife( const EventType& eventIn )
{
	const std::string protocol( "secondlife://" );

	if ( eventIn.getStringValue().length() )
	{
		if ( LLString::compareInsensitive( eventIn.getStringValue().substr( 0, protocol.length() ).c_str(), protocol.c_str() ) == 0 )
		{
			// parse out sim name and coordinates
			LLURLSimString::setString( eventIn.getStringValue() );
			LLURLSimString::parse();

			// if there is a world map
			if ( gFloaterWorldMap )
			{
				#if ! LL_RELEASE_FOR_DOWNLOAD
				llinfos << "MOZ> opening map to " << LLURLSimString::sInstance.mSimName.c_str() << " at " << LLURLSimString::sInstance.mX << "," << LLURLSimString::sInstance.mY << "," << LLURLSimString::sInstance.mZ << llendl;
				#endif

				// mark where the destination is
				gFloaterWorldMap->trackURL( LLURLSimString::sInstance.mSimName.c_str(),
											LLURLSimString::sInstance.mX,
											LLURLSimString::sInstance.mY,
											LLURLSimString::sInstance.mZ );

				// display map
				LLFloaterWorldMap::show( NULL, TRUE );
			}
			else
			// if there is no world map, assume we're on the login page.. (this might be bad but I don't see a way to tell if you're at login or not)
			{
				// refresh the login page and force the location combo box to be visible
				LLPanelLogin::refreshLocation( true );
			};
		};
	};		

	// chain this event on to observers of an instance of LLWebBrowserCtrl
	LLWebBrowserCtrlEvent event( eventIn.getStringValue() );
	mEventEmitter.update( &LLWebBrowserCtrlObserver::onClickLinkSecondLife, event );
}

////////////////////////////////////////////////////////////////////////////////
//
LLWebBrowserTexture::LLWebBrowserTexture( S32 width, S32 height, LLWebBrowserCtrl* browserCtrl, int browserWindowId ) :
	LLDynamicTexture( 512, 512, 4, ORDER_FIRST, TRUE ),
	mLastBrowserDepth( 0 ),
	mWebBrowserCtrl( browserCtrl ),
	mEmbeddedBrowserWindowId( browserWindowId )
{
	mElapsedTime.start();

	resize( width, height );
}

////////////////////////////////////////////////////////////////////////////////
//
LLWebBrowserTexture::~LLWebBrowserTexture()
{
	mElapsedTime.stop();
}

////////////////////////////////////////////////////////////////////////////////
//
BOOL LLWebBrowserTexture::render()
{
	// frequent updates turned on?
	if ( mWebBrowserCtrl->getFrequentUpdates() )
	{
		// only update mozilla/texture occasionally
		if ( mElapsedTime.getElapsedTimeF32() > ( 1.0f / 15.0f ) )
		{
			mElapsedTime.reset();

			const unsigned char* pixels = LLMozLib::getInstance()->grabBrowserWindow( mEmbeddedBrowserWindowId );

			S32 actual_rowspan = LLMozLib::getInstance()->getBrowserRowSpan( mEmbeddedBrowserWindowId );
			S32 browser_depth = LLMozLib::getInstance()->getBrowserDepth( mEmbeddedBrowserWindowId );

			// these are both invalid conditions and should never happen but SL-27583 indicates it does
			if ( actual_rowspan < 1 || browser_depth < 2 )
				return FALSE;

			S32 pagebuffer_width = actual_rowspan / browser_depth;
			
			if(mLastBrowserDepth == browser_depth)
			{
				// Browser depth hasn't changed.  Just grab the pixels.

				mTexture->setSubImage( pixels,
										pagebuffer_width, mBrowserHeight,
											0, 0, pagebuffer_width, mBrowserHeight );
			}
			else
			{
				// Browser depth has changed -- need to recreate texture to match.
				resize(mBrowserWidth, mBrowserHeight);
			}
		};
	};

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
//
S32 LLWebBrowserTexture::getBrowserWidth()
{
	return mBrowserWidth;
}

////////////////////////////////////////////////////////////////////////////////
//
S32 LLWebBrowserTexture::getBrowserHeight()
{
	return mBrowserHeight;
}


////////////////////////////////////////////////////////////////////////////////
//
void LLWebBrowserTexture::resize( S32 new_width, S32 new_height )
{
	F32 scale_ratio = 1.f;
	if (new_width > MAX_DIMENSION)
	{
		scale_ratio = (F32)MAX_DIMENSION / (F32)new_width;
	}
	if (new_height > MAX_DIMENSION)
	{
		scale_ratio = llmin(scale_ratio, (F32)MAX_DIMENSION / (F32)new_height);
	}

	mBrowserWidth = llround(scale_ratio * (F32)new_width);
	mBrowserHeight = llround(scale_ratio * (F32)new_height);

	LLMozLib::getInstance()->setSize( mEmbeddedBrowserWindowId, mBrowserWidth, mBrowserHeight );

	const unsigned char* pixels = LLMozLib::getInstance()->grabBrowserWindow( mEmbeddedBrowserWindowId );

	S32 actual_rowspan = LLMozLib::getInstance()->getBrowserRowSpan( mEmbeddedBrowserWindowId );
	S32 browser_depth = LLMozLib::getInstance()->getBrowserDepth( mEmbeddedBrowserWindowId );

	// these are both invalid conditions and should never happen but SL-27583 indicates it does
	if ( actual_rowspan < 1 || browser_depth < 2 )
		return;

	S32 pagebuffer_width = actual_rowspan / browser_depth;

	// calculate the next power of 2 bigger than reqquested size for width and height
	for ( mWidth = 1; mWidth < pagebuffer_width; mWidth <<= 1 )
	{
		if ( mWidth >= MAX_TEXTURE_DIMENSION )
		{
			break;
		};
	};

	for ( mHeight = 1; mHeight < mBrowserHeight; mHeight <<= 1 )
	{
		if ( mHeight >= MAX_TEXTURE_DIMENSION )
		{
			break;
		};
	};
	
	LLGLint internal_format;
	LLGLenum primary_format;
	LLGLenum type_format;
	BOOL	 swap_bytes = FALSE;

	switch(browser_depth)
	{
		default:
		case 4:
			internal_format = GL_RGBA8;
			primary_format = GL_BGRA_EXT;
		#if LL_DARWIN
			#if LL_BIG_ENDIAN
				type_format = GL_UNSIGNED_INT_8_8_8_8_REV;
			#else
				type_format = GL_UNSIGNED_INT_8_8_8_8;
			#endif
		#else	// windows or linux
			type_format = GL_UNSIGNED_BYTE;
		#endif
		break;
		
		case 2:
		#if LL_DARWIN
			internal_format = GL_RGBA8;
			primary_format = GL_BGRA_EXT;
			type_format = GL_UNSIGNED_SHORT_1_5_5_5_REV;
			#if LL_LITTLE_ENDIAN
				swap_bytes = TRUE;
			#endif
		#else	// windows or linux
			// MBW -- XXX -- This is just a guess on my part.  Someone needs to verify which GL texture format matches the 16-bit format used on windows.
			internal_format = GL_RGB8;
			primary_format = GL_RGB;
			type_format = GL_UNSIGNED_SHORT_5_6_5;
		#endif
		break;
	}
	
	// will create mWidth * mHeight sized texture, using BGR ordering
	LLDynamicTexture::generateGLTexture(internal_format, primary_format, type_format, swap_bytes);
	
	mTexture->setSubImage( pixels,
		pagebuffer_width, mBrowserHeight,
			0, 0, pagebuffer_width, mBrowserHeight );
	
	mLastBrowserDepth = browser_depth;
}

LLView* LLWebBrowserCtrl::fromXML(LLXMLNodePtr node, LLView *parent, LLUICtrlFactory *factory)
{
	LLString name("web_browser");
	node->getAttributeString("name", name);

	LLString start_url("start_url");
	node->getAttributeString("start_url", start_url );

	BOOL border_visible = true;
	node->getAttributeBOOL("border_visible", border_visible);

	LLRect rect;
	createRect(node, rect, parent, LLRect());

	LLWebBrowserCtrl* web_browser = new LLWebBrowserCtrl( name, rect );

	web_browser->initFromXML(node, parent);

	web_browser->setHomePageUrl( start_url );

	web_browser->setBorderVisible( border_visible );

	web_browser->navigateHome();

	return web_browser;
}

#endif // LL_LIBXUL_ENABLED
