/**
 * @file llwebbrowserctrl.cpp
 * @brief Web browser UI control
 *
 * Copyright (c) 2006-2007, Linden Research, Inc.
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
	mTextureDepthBytes( 4 ),
	mEmbeddedBrowserWindowId( 0 ),
	mBorder(NULL),
	mFrequentUpdates( true ),
	mOpenLinksInExternalBrowser( false ),
	mOpenSecondLifeLinksInMap( true ),
	mHomePageUrl( "" ),
	mIgnoreUIScale( true ),
	mAlwaysRefresh( false )
{
	S32 screen_width = mIgnoreUIScale ? llround((F32)mRect.getWidth() * LLUI::sGLScaleFactor.mV[VX]) : mRect.getWidth();
	S32 screen_height = mIgnoreUIScale ? llround((F32)mRect.getHeight() * LLUI::sGLScaleFactor.mV[VY]) : mRect.getHeight();

	// create a new browser window
	{
#if LL_LINUX
		// Yuck, Mozilla init plays with the locale - push/pop
		// the locale to protect it, as exotic/non-C locales
		// causes our code lots of general critical weirdness
		// and crashness. (SL-35450)
		std::string saved_locale = setlocale(LC_ALL, NULL);
#endif // LL_LINUX
		mEmbeddedBrowserWindowId = LLMozLib::getInstance()->createBrowserWindow( gViewerWindow->getPlatformWindow(), screen_width, screen_height );
#if LL_LINUX
		setlocale(LC_ALL, saved_locale.c_str() );
#endif // LL_LINUX
	}

	// change color to black so transisitons aren't so noticable (this should be in XML eventually)
	LLMozLib::getInstance()->setBackgroundColor( mEmbeddedBrowserWindowId, 0x00, 0x00, 0x00 );

	// observe the browser so we can trap HREF events)
	LLMozLib::getInstance()->addObserver( mEmbeddedBrowserWindowId, this );

	// create a new texture (based on LLDynamic texture) that will be used to display the output
	mWebBrowserImage = new LLWebBrowserTexture( screen_width, screen_height, this, mEmbeddedBrowserWindowId );

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
// open secondlife:// links in map automatically or not
void LLWebBrowserCtrl::setOpenSecondLifeLinksInMap( bool valIn )
{
	mOpenSecondLifeLinksInMap = valIn;
};

////////////////////////////////////////////////////////////////////////////////
//
BOOL LLWebBrowserCtrl::handleHover( S32 x, S32 y, MASK mask )
{
	convertInputCoords(x, y);
	LLMozLib::getInstance()->mouseMove( mEmbeddedBrowserWindowId, x, y );

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
	convertInputCoords(x, y);
	LLMozLib::getInstance()->mouseUp( mEmbeddedBrowserWindowId, x, y );

	gViewerWindow->setMouseCapture( NULL );

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
//
BOOL LLWebBrowserCtrl::handleMouseDown( S32 x, S32 y, MASK mask )
{
	convertInputCoords(x, y);
	LLMozLib::getInstance()->mouseDown( mEmbeddedBrowserWindowId, x, y );

	gViewerWindow->setMouseCapture( this );

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

	LLUICtrl::onFocusLost();
}

////////////////////////////////////////////////////////////////////////////////
//
BOOL LLWebBrowserCtrl::handleKey( KEY key, MASK mask, BOOL called_from_parent )
{
	unsigned long nskey;

	// First, turn SL internal keycode enum into Mozilla keycode enum

	// We don't have to deal with printable characters here - they should
	// go through handleUnicodeChar().  This table could be more complete
	// than it is, but I think this covers all of the important
	// non-printables.
	switch (key)
	{
	case KEY_BACKSPACE:
		nskey = LL_DOM_VK_BACK_SPACE; break;
	case KEY_TAB:
		nskey = LL_DOM_VK_TAB; break;
	case KEY_RETURN:
		nskey = LL_DOM_VK_RETURN; break;
	case KEY_PAD_RETURN:
		nskey = LL_DOM_VK_ENTER; break;
	case KEY_ESCAPE:
		nskey = LL_DOM_VK_ESCAPE; break;
	case KEY_PAGE_UP:
		nskey = LL_DOM_VK_PAGE_UP; break;
	case KEY_PAGE_DOWN:
		nskey = LL_DOM_VK_PAGE_DOWN; break;
	case KEY_END:
		nskey = LL_DOM_VK_END; break;
	case KEY_HOME:
		nskey = LL_DOM_VK_HOME; break;
	case KEY_LEFT:
		nskey = LL_DOM_VK_LEFT; break;
	case KEY_UP:
		nskey = LL_DOM_VK_UP; break;
	case KEY_RIGHT:
		nskey = LL_DOM_VK_RIGHT; break;
	case KEY_DOWN:
		nskey = LL_DOM_VK_DOWN; break;
	case KEY_INSERT:
		nskey = LL_DOM_VK_INSERT; break;
	case KEY_DELETE:
		nskey = LL_DOM_VK_DELETE; break;
	default:
		llinfos << "Don't know how to map LL keycode " << U32(key)
			<< " to DOM key.  Ignoring." << llendl;
		return FALSE; // don't know how to map this key.
	}

	LLMozLib::getInstance()->keyPress( mEmbeddedBrowserWindowId, nskey );

	return TRUE;
}

BOOL LLWebBrowserCtrl::handleUnicodeChar(llwchar uni_char, BOOL called_from_parent)
{
	// only accept 'printable' characters, sigh...
	if (uni_char >= 32 // discard 'control' characters
	    && uni_char != 127) // SDL thinks this is 'delete' - yuck.
	{
		LLMozLib::getInstance()->unicodeInput( mEmbeddedBrowserWindowId, uni_char );
	}

	return TRUE;
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
	S32 screen_width = mIgnoreUIScale ? llround((F32)width * LLUI::sGLScaleFactor.mV[VX]) : width;
	S32 screen_height = mIgnoreUIScale ? llround((F32)height * LLUI::sGLScaleFactor.mV[VX]) : height;
	// when floater is minimized, these sizes are negative
	if ( screen_height > 0 && screen_width > 0 )
	{
		mWebBrowserImage->resize( screen_width, screen_height );
	}

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

	glPushMatrix();
	{
		if (mIgnoreUIScale)
		{
			glLoadIdentity();
			// font system stores true screen origin, need to scale this by UI scale factor
			// to get render origin for this view (with unit scale)
			glTranslatef(floorf(LLFontGL::sCurOrigin.mX * LLUI::sGLScaleFactor.mV[VX]), 
						floorf(LLFontGL::sCurOrigin.mY * LLUI::sGLScaleFactor.mV[VY]), 
						LLFontGL::sCurOrigin.mZ);
		}

		// scale texture to fit the space using texture coords
		mWebBrowserImage->bindTexture();
		glColor4fv( LLColor4::white.mV );
		F32 max_u = ( F32 )mWebBrowserImage->getBrowserWidth() / ( F32 )mWebBrowserImage->getWidth();
		F32 max_v = ( F32 )mWebBrowserImage->getBrowserHeight() / ( F32 )mWebBrowserImage->getHeight();

		// draw the browser
		glBlendFunc( GL_ONE, GL_ZERO );
		glBegin( GL_QUADS );
		{
			// render using web browser reported width and height, instead of trying to invert GL scale
			glTexCoord2f( max_u, 0.f );
			glVertex2i( mWebBrowserImage->getBrowserWidth(), mWebBrowserImage->getBrowserHeight() );

			glTexCoord2f( 0.f, 0.f );
			glVertex2i( 0, mWebBrowserImage->getBrowserHeight() );

			glTexCoord2f( 0.f, max_v );
			glVertex2i( 0, 0 );

			glTexCoord2f( max_u, max_v );
			glVertex2i( mWebBrowserImage->getBrowserWidth(), 0 );
		}
		glEnd();
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA  );
	}
	glPopMatrix();

	// highlight if keyboard focus here. (TODO: this needs some work)
	if ( mBorder->getVisible() )
		mBorder->setKeyboardFocusHighlight( gFocusMgr.childHasKeyboardFocus( this ) );


	LLUICtrl::draw();
}

void LLWebBrowserCtrl::convertInputCoords(S32& x, S32& y)
{
	x = mIgnoreUIScale ? llround((F32)x * LLUI::sGLScaleFactor.mV[VX]) : x;
	y = mIgnoreUIScale ? llround((F32)(mRect.getHeight() - y) * LLUI::sGLScaleFactor.mV[VY]) : (mRect.getHeight() - y);
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
			if ( mOpenSecondLifeLinksInMap )
			{
				openMapAtlocation( eventIn.getStringValue() );
			};

			// chain this event on to observers of an instance of LLWebBrowserCtrl
			LLWebBrowserCtrlEvent event( eventIn.getStringValue() );
			mEventEmitter.update( &LLWebBrowserCtrlObserver::onClickLinkSecondLife, event );
		};
	};		
}

////////////////////////////////////////////////////////////////////////////////
// virtual
void LLWebBrowserCtrl::openMapAtlocation( std::string second_life_url )
{
	// parse out sim name and coordinates
	LLURLSimString::setString( second_life_url );
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
	if ( mWebBrowserCtrl->getFrequentUpdates() || mWebBrowserCtrl->getAlwaysRefresh() )
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

			// width can change after it's rendered - (Mozilla bug# 24721)
			S32 pagebuffer_width = actual_rowspan / browser_depth;

			// Browser depth hasn't changed.  
			if(mLastBrowserDepth == browser_depth)
			{
				// Just grab the pixels.
				mTexture->setSubImage( pixels,
										pagebuffer_width, mBrowserHeight,
											0, 0, pagebuffer_width, mBrowserHeight );
			}
			else
			{
				// Browser depth has changed -- need to recreate texture to match.
				resize(mBrowserWidth, mBrowserHeight);
			};
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

	releaseGLTexture();
	
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

	BOOL ignore_ui_scale = web_browser->getIgnoreUIScale();
	node->getAttributeBOOL("ignore_ui_scale", ignore_ui_scale);
	web_browser->setIgnoreUIScale((bool)ignore_ui_scale);

	web_browser->initFromXML(node, parent);

	web_browser->setHomePageUrl( start_url );

	web_browser->setBorderVisible( border_visible );

	web_browser->navigateHome();

	return web_browser;
}

#endif // LL_LIBXUL_ENABLED
