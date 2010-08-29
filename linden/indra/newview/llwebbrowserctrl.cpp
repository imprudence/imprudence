/**
 * @file llwebbrowserctrl.cpp
 * @brief Web browser UI control
 *
 * $LicenseInfo:firstyear=2006&license=viewergpl$
 * 
 * Copyright (c) 2006-2009, Linden Research, Inc.
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

#include "llviewerprecompiledheaders.h"


#include "llwebbrowserctrl.h"

// viewer includes
#include "llfloaterhtml.h"
#include "llfloaterworldmap.h"
#include "lluictrlfactory.h"
#include "llurldispatcher.h"
#include "llurlsimstring.h"
#include "llviewborder.h"
#include "llviewercontrol.h"
#include "llviewerwindow.h"
#include "llnotifications.h"
#include "llweb.h"
#include "llrender.h"

// linden library includes
#include "llfocusmgr.h"

extern BOOL gRestoreGL;

// Setting the mozilla buffer width to 2048 exactly doesn't work, since it pads its rowbytes a bit, pushing the texture width over 2048.
// 2000 should give enough headroom for any amount of padding it cares to add.
const S32 MAX_DIMENSION = 2000;
const S32 MAX_TEXTURE_DIMENSION = 2048;

static LLRegisterWidget<LLWebBrowserCtrl> r("web_browser");

LLWebBrowserCtrl::LLWebBrowserCtrl( const std::string& name, const LLRect& rect ) :
	LLUICtrl( name, rect, FALSE, NULL, NULL ),
	mTextureDepthBytes( 4 ),
	mWebBrowserImage( 0 ),
	mEmbeddedBrowserWindowId( 0 ),
	mBorder(NULL),
	mFrequentUpdates( true ),
	mForceUpdate( false ),
	mOpenLinksInExternalBrowser( false ),
	mOpenLinksInInternalBrowser( false ),
	mTrusted( false ),
	mHomePageUrl( "" ),
	mIgnoreUIScale( true ),
	mAlwaysRefresh( false ),
	mExternalUrl( "" ),
	mMediaSource( 0 ),
	mTakeFocusOnClick( true ),
	mCurrentNavUrl( "about:blank" )
{
	S32 screen_width = mIgnoreUIScale ? 
		llround((F32)getRect().getWidth() * LLUI::sGLScaleFactor.mV[VX]) : getRect().getWidth();
	S32 screen_height = mIgnoreUIScale ? 
		llround((F32)getRect().getHeight() * LLUI::sGLScaleFactor.mV[VY]) : getRect().getHeight();


	LLMediaManager *mgr = LLMediaManager::getInstance();

	if (!mgr)
	{
		llwarns << "cannot get media manager" << llendl;
		return;
	}
		
	mMediaSource = mgr->createSourceFromMimeType("http", "text/html" );
	if ( !mMediaSource )
	{
		llwarns << "media source create failed " << llendl;
		// return;
	}
	else
	{

		// mMediaSource->init();
		mMediaSource->addCommand( LLMediaBase::COMMAND_START );

		// observe the browser so we can trap HREF events)
		mMediaSource->addObserver(this);

		// create a new texture (based on LLDynamic texture) that will be used to display the output
		mWebBrowserImage = new LLWebBrowserTexture( screen_width, screen_height, this, mMediaSource );
	}

	LLRect border_rect( 0, getRect().getHeight() + 2, getRect().getWidth() + 2, 0 );
	mBorder = new LLViewBorder( std::string("web control border"), border_rect, LLViewBorder::BEVEL_IN );
	addChild( mBorder );
}

////////////////////////////////////////////////////////////////////////////////
// note: this is now a singleton and destruction happens via initClass() now
LLWebBrowserCtrl::~LLWebBrowserCtrl()
{
	LLMediaManager *mgr = LLMediaManager::getInstance();

	if (!mgr)
	{
		llwarns << "cannot get media manager" << llendl;
		return;
	}

	if (mMediaSource)
	{
		mgr->destroySource(mMediaSource);
		mMediaSource = NULL;
	}

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
//
void LLWebBrowserCtrl::setTakeFocusOnClick( bool take_focus )
{
	mTakeFocusOnClick = take_focus;
}

////////////////////////////////////////////////////////////////////////////////
// set flag that forces the embedded browser to open links in the external system browser
void LLWebBrowserCtrl::setOpenInExternalBrowser( bool valIn )
{
	mOpenLinksInExternalBrowser = valIn;
};

////////////////////////////////////////////////////////////////////////////////
// set flag that forces the embedded browser to open links in the internal browser floater
void LLWebBrowserCtrl::setOpenInInternalBrowser( bool valIn )
{
	mOpenLinksInInternalBrowser = valIn;
};

////////////////////////////////////////////////////////////////////////////////
void LLWebBrowserCtrl::setTrusted( bool valIn )
{
	mTrusted = valIn;
}

////////////////////////////////////////////////////////////////////////////////
//
BOOL LLWebBrowserCtrl::handleHover( S32 x, S32 y, MASK mask )
{
	convertInputCoords(x, y);

	if (mMediaSource)
		mMediaSource->mouseMove(x, y);

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
//
BOOL LLWebBrowserCtrl::handleScrollWheel( S32 x, S32 y, S32 clicks )
{
	if (mMediaSource)
		mMediaSource->scrollByLines(clicks);

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
//
BOOL LLWebBrowserCtrl::handleMouseUp( S32 x, S32 y, MASK mask )
{
	convertInputCoords(x, y);

	if (mMediaSource)
	{
		mMediaSource->mouseUp(x, y);

		// *HACK: LLMediaImplLLMozLib automatically takes focus on mouseup,
		// in addition to the onFocusReceived() call below.  Undo this. JC
		if (!mTakeFocusOnClick)
		{
			mMediaSource->focus(false);
			gViewerWindow->focusClient();
		}
	}
	
	gFocusMgr.setMouseCapture( NULL );

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
//
BOOL LLWebBrowserCtrl::handleMouseDown( S32 x, S32 y, MASK mask )
{
	convertInputCoords(x, y);

	if (mMediaSource)
		mMediaSource->mouseDown(x, y);
	
	gFocusMgr.setMouseCapture( this );

	if (mTakeFocusOnClick)
	{
		setFocus( TRUE );
	}

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
//
BOOL LLWebBrowserCtrl::handleDoubleClick( S32 x, S32 y, MASK mask )
{
	convertInputCoords(x, y);

	if (mMediaSource)
		mMediaSource->mouseLeftDoubleClick( x, y );

	gFocusMgr.setMouseCapture( this );

	if (mTakeFocusOnClick)
	{
		setFocus( TRUE );
	}

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
//
void LLWebBrowserCtrl::onFocusReceived()
{
	if (mMediaSource)
		mMediaSource->focus(true);
	

	LLUICtrl::onFocusReceived();
}

////////////////////////////////////////////////////////////////////////////////
//
void LLWebBrowserCtrl::onFocusLost()
{
	if (mMediaSource)
		mMediaSource->focus(false);

	gViewerWindow->focusClient();

	LLUICtrl::onFocusLost();
}

////////////////////////////////////////////////////////////////////////////////
//
BOOL LLWebBrowserCtrl::handleKeyHere( KEY key, MASK mask )
{
	unsigned long media_key;

	// First, turn SL internal keycode enum into Mozilla keycode enum

	// We don't have to deal with printable characters here - they should
	// go through handleUnicodeChar().  This table could be more complete
	// than it is, but I think this covers all of the important
	// non-printables.

	switch (key)
	{
	case KEY_BACKSPACE:
		media_key = LL_MEDIA_KEY_BACKSPACE; break;
	case KEY_TAB:
		media_key = LL_MEDIA_KEY_TAB; break;
	case KEY_RETURN:
		media_key = LL_MEDIA_KEY_RETURN; break;
	case KEY_PAD_RETURN:
		media_key = LL_MEDIA_KEY_PAD_RETURN; break;
	case KEY_ESCAPE:
		media_key = LL_MEDIA_KEY_ESCAPE; break;
	case KEY_PAGE_UP:
		media_key = LL_MEDIA_KEY_PAGE_UP; break;
	case KEY_PAGE_DOWN:
		media_key = LL_MEDIA_KEY_PAGE_DOWN; break;
	case KEY_END:
		media_key = LL_MEDIA_KEY_END; break;
	case KEY_HOME:
		media_key = LL_MEDIA_KEY_HOME; break;
	case KEY_LEFT:
		media_key = LL_MEDIA_KEY_LEFT; break;
	case KEY_UP:
		media_key = LL_MEDIA_KEY_UP; break;
	case KEY_RIGHT:
		media_key = LL_MEDIA_KEY_RIGHT; break;
	case KEY_DOWN:
		media_key = LL_MEDIA_KEY_DOWN; break;
	case KEY_INSERT:
		media_key = LL_MEDIA_KEY_INSERT; break;
	case KEY_DELETE:
		media_key = LL_MEDIA_KEY_DELETE; break;
	default:
		llinfos << "Don't know how to map LL keycode " << U32(key)
			<< " to DOM key.  Ignoring." << llendl;
		return FALSE; // don't know how to map this key.
	}

	if (mMediaSource)
		mMediaSource->keyPress(media_key);
	
	return TRUE;
}

BOOL LLWebBrowserCtrl::handleUnicodeCharHere(llwchar uni_char)
{
	// only accept 'printable' characters, sigh...
	if (uni_char >= 32 // discard 'control' characters
	    && uni_char != 127) // SDL thinks this is 'delete' - yuck.
	{
		if (mMediaSource)
			mMediaSource->unicodeInput(uni_char);
	}

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
//
void LLWebBrowserCtrl::onVisibilityChange ( BOOL new_visibility )
{
	// set state of frequent updates automatically if visibility changes
	if ( new_visibility )
	{
		mFrequentUpdates = true;
	}
	else
	{
		mFrequentUpdates = false;
	}
	LLUICtrl::onVisibilityChange(new_visibility);
}

////////////////////////////////////////////////////////////////////////////////
//
void LLWebBrowserCtrl::reshape( S32 width, S32 height, BOOL called_from_parent )
{
	S32 screen_width = mIgnoreUIScale ? llround((F32)width * LLUI::sGLScaleFactor.mV[VX]) : width;
	S32 screen_height = mIgnoreUIScale ? llround((F32)height * LLUI::sGLScaleFactor.mV[VY]) : height;

	// when floater is minimized, these sizes are negative
	if ( mWebBrowserImage && screen_height > 0 && screen_width > 0 )
	{
		mWebBrowserImage->resize( screen_width, screen_height );
		mForceUpdate = true;
	}

	LLUICtrl::reshape( width, height, called_from_parent );
}

////////////////////////////////////////////////////////////////////////////////
//
void LLWebBrowserCtrl::navigateBack()
{
	if (mMediaSource)
		mMediaSource->navigateBack();
}

////////////////////////////////////////////////////////////////////////////////
//
void LLWebBrowserCtrl::navigateForward()
{
	if (mMediaSource)
		mMediaSource->navigateForward();
}

////////////////////////////////////////////////////////////////////////////////
//
bool LLWebBrowserCtrl::canNavigateBack()
{
	if (mMediaSource)
		return mMediaSource->canNavigateBack();
	else
		return false;
}

////////////////////////////////////////////////////////////////////////////////
//
bool LLWebBrowserCtrl::canNavigateForward()
{
	if (mMediaSource)
		return mMediaSource->canNavigateForward();
	else
		return false;
}

////////////////////////////////////////////////////////////////////////////////
//
bool LLWebBrowserCtrl::set404RedirectUrl( std::string redirect_url )
{
	if(mMediaSource)
		return mMediaSource->set404RedirectUrl( redirect_url );
	else
		return false;
}

////////////////////////////////////////////////////////////////////////////////
//
bool LLWebBrowserCtrl::clr404RedirectUrl()
{
	if(mMediaSource)
		return mMediaSource->clr404RedirectUrl();
	else
		return false;
}

////////////////////////////////////////////////////////////////////////////////
//
void LLWebBrowserCtrl::navigateTo( std::string urlIn )
{
	// don't browse to anything that starts with secondlife:// or sl://
	const std::string protocol1 = "secondlife://";
	const std::string protocol2 = "sl://";
	if ((LLStringUtil::compareInsensitive(urlIn.substr(0, protocol1.length()), protocol1) == 0) ||
	    (LLStringUtil::compareInsensitive(urlIn.substr(0, protocol2.length()), protocol2) == 0))
	{
		// TODO: Print out/log this attempt?
		// llinfos << "Rejecting attempt to load restricted website :" << urlIn << llendl;
		return;
	}
	
	if (mMediaSource)
	{
		mCurrentNavUrl = urlIn;
		mMediaSource->navigateTo(urlIn);
        }
}


void LLWebBrowserCtrl::navigateToLocalPage( const std::string& subdir, const std::string& filename_in )
{
	std::string language = LLUI::getLanguage();
	std::string delim = gDirUtilp->getDirDelimiter();
	std::string filename;

	filename += subdir;
	filename += delim;
	filename += filename_in;

	std::string expanded_filename = gDirUtilp->findSkinnedFilename("html", language, filename);

	if (gDirUtilp->fileExists(expanded_filename))
	{
		navigateTo(expanded_filename);
		return;
	}
	if (language != "en-us")
	{
		expanded_filename = gDirUtilp->findSkinnedFilename("html", "en-us", filename);
		if (gDirUtilp->fileExists(expanded_filename))
		{
			navigateTo(expanded_filename);
			return;
		}
	}

	llwarns << "File " << subdir << delim << filename_in << "not found" << llendl;
}


////////////////////////////////////////////////////////////////////////////////
//
void LLWebBrowserCtrl::navigateHome()
{
	if( mHomePageUrl.length() )
	{
		if (mMediaSource)
			mMediaSource->navigateTo(mHomePageUrl);
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
bool LLWebBrowserCtrl::setCaretColor(unsigned int red, unsigned int green, unsigned int blue)
{
	if (mMediaSource)
		return mMediaSource->setCaretColor(red, green, blue);
	else
		return false;
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
	if ( ! mWebBrowserImage )
		return;

	if ( gRestoreGL == 1 )
	{
		LLRect r = getRect();
		mMediaSource->updateMedia();
		reshape( r.getWidth(), r.getHeight(), FALSE );
		return;
	};

	// NOTE: optimization needed here - probably only need to do this once
	// unless tearoffs change the parent which they probably do.
	const LLUICtrl* ptr = findRootMostFocusRoot();
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

	gGL.pushMatrix();
	{
		if (mIgnoreUIScale)
		{
			glLoadIdentity();
			// font system stores true screen origin, need to scale this by UI scale factor
			// to get render origin for this view (with unit scale)
			gGL.translatef(floorf(LLFontGL::sCurOrigin.mX * LLUI::sGLScaleFactor.mV[VX]), 
						floorf(LLFontGL::sCurOrigin.mY * LLUI::sGLScaleFactor.mV[VY]), 
						LLFontGL::sCurOrigin.mZ);
		}

		// scale texture to fit the space using texture coords
		gGL.getTexUnit(0)->bind(mWebBrowserImage->getTexture());
		gGL.color4fv( LLColor4::white.mV );
		F32 max_u = ( F32 )mWebBrowserImage->getBrowserWidth() / ( F32 )mWebBrowserImage->getWidth();
		F32 max_v = ( F32 )mWebBrowserImage->getBrowserHeight() / ( F32 )mWebBrowserImage->getHeight();

		// draw the browser
		gGL.setSceneBlendType(LLRender::BT_REPLACE);
		gGL.begin( LLRender::QUADS );
		{
			// render using web browser reported width and height, instead of trying to invert GL scale
			gGL.texCoord2f( max_u, max_v );
			gGL.vertex2i( mWebBrowserImage->getBrowserWidth(), mWebBrowserImage->getBrowserHeight() );

			gGL.texCoord2f( 0.f, max_v );
			gGL.vertex2i( 0, mWebBrowserImage->getBrowserHeight() );

			gGL.texCoord2f( 0.f, 0.f );
			gGL.vertex2i( 0, 0 );

			gGL.texCoord2f( max_u, 0.f );
			gGL.vertex2i( mWebBrowserImage->getBrowserWidth(), 0 );
		}
		gGL.end();
		gGL.setSceneBlendType(LLRender::BT_ALPHA);
	}
	gGL.popMatrix();

	// highlight if keyboard focus here. (TODO: this needs some work)
	if ( mBorder->getVisible() )
		mBorder->setKeyboardFocusHighlight( gFocusMgr.childHasKeyboardFocus( this ) );

	
	LLUICtrl::draw();
}

void LLWebBrowserCtrl::convertInputCoords(S32& x, S32& y)
{
	x = mIgnoreUIScale ? llround((F32)x * LLUI::sGLScaleFactor.mV[VX]) : x;
	y = mIgnoreUIScale ? llround((F32)(getRect().getHeight() - y) * LLUI::sGLScaleFactor.mV[VY]) : getRect().getHeight() - y;
}

////////////////////////////////////////////////////////////////////////////////
// virtual
void LLWebBrowserCtrl::onNavigateBegin( const EventType& eventIn )
{
	LLWebBrowserCtrlEvent event( eventIn.getStringValue() );
	mEventEmitter.update( &LLWebBrowserCtrlObserver::onNavigateBegin, event );
}

////////////////////////////////////////////////////////////////////////////////
// virtual
void LLWebBrowserCtrl::onNavigateComplete( const EventType& eventIn )
{
	// chain this event on to observers of an instance of LLWebBrowserCtrl
	LLWebBrowserCtrlEvent event( eventIn.getStringValue() );
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
void LLWebBrowserCtrl::onMediaContentsChange( const EventType& event_in )
{
	if ( mWebBrowserImage )
	{
		mWebBrowserImage->setNeedsUpdate();
		mForceUpdate = true;
	}
}

////////////////////////////////////////////////////////////////////////////////
// static 
bool LLWebBrowserCtrl::onClickLinkExternalTarget(const LLSD& notification, const LLSD& response )
{
	S32 option = LLNotification::getSelectedOption(notification, response);
	if ( 0 == option )
	{
		// open in external browser because we don't support 
		// creation of our own secondary browser windows
		LLWeb::loadURLExternal( notification["payload"]["external_url"].asString() );
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////////
// virtual
void LLWebBrowserCtrl::onClickLinkHref( const EventType& eventIn )
{
	// if there is a value for the target (passed in stringValueEx)
	if ( eventIn.getStringValueEx().length() )
	{
		// if the target = "_new"
		if ( eventIn.getStringValueEx() == "_external" )		
		{
			mExternalUrl = eventIn.getStringValue();
			LLSD payload;
			payload["external_url"] = mExternalUrl;
			LLNotifications::instance().add( "WebLaunchExternalTarget", LLSD(), payload, onClickLinkExternalTarget);
			return;
		}
	}

	const std::string protocol1( "http://" );
	const std::string protocol2( "https://" );
	if( mOpenLinksInExternalBrowser )
	{
		if ( eventIn.getStringValue().length() )
		{
			if ( LLStringUtil::compareInsensitive( eventIn.getStringValue().substr( 0, protocol1.length() ), protocol1 ) == 0 ||
				 LLStringUtil::compareInsensitive( eventIn.getStringValue().substr( 0, protocol2.length() ), protocol2 ) == 0 )
			{
				LLWeb::loadURLExternal( eventIn.getStringValue() );
			}
		}
	}
	else
	if( mOpenLinksInInternalBrowser )
	{
		if ( eventIn.getStringValue().length() )
		{
			if ( LLStringUtil::compareInsensitive( eventIn.getStringValue().substr( 0, protocol1.length() ), protocol1 ) == 0 ||
				 LLStringUtil::compareInsensitive( eventIn.getStringValue().substr( 0, protocol2.length() ), protocol2 ) == 0 )
			{
				// If we spawn a new LLFloaterHTML, assume we want it to
				// follow this LLWebBrowserCtrl's trust for whether or
				// not to open secondlife:///app/ links. JC.
				const bool open_links_externally = false;
				LLFloaterHtml::getInstance()->show( 
					eventIn.getStringValue(), 
						"Second Life Browser",
							open_links_externally,
								mTrusted);
			}
		}
	}

	// chain this event on to observers of an instance of LLWebBrowserCtrl
	LLWebBrowserCtrlEvent event( eventIn.getStringValue(), eventIn.getStringValueEx() );
	mEventEmitter.update( &LLWebBrowserCtrlObserver::onClickLinkHref, event );
}

////////////////////////////////////////////////////////////////////////////////
// virtual
void LLWebBrowserCtrl::onClickLinkNoFollow( const EventType& eventIn )
{
	std::string url = eventIn.getStringValue();
	if (LLURLDispatcher::isSLURLCommand(url)
		&& !mTrusted)
	{
		// block handling of this secondlife:///app/ URL
		LLNotifications::instance().add("UnableToOpenCommandURL");
		return;
	}

	LLURLDispatcher::dispatch(url, this, mTrusted);

	// chain this event on to observers of an instance of LLWebBrowserCtrl
	LLWebBrowserCtrlEvent event( eventIn.getStringValue() );
	mEventEmitter.update( &LLWebBrowserCtrlObserver::onClickLinkNoFollow, event );
}

////////////////////////////////////////////////////////////////////////////////
//
LLWebBrowserTexture::LLWebBrowserTexture( S32 width, S32 height, LLWebBrowserCtrl* browserCtrl, LLMediaBase *media_source ) :
	LLDynamicTexture( 512, 512, 4, ORDER_FIRST, TRUE ),
	mLastBrowserDepth( 0 ),
	mNeedsUpdate( true ),
	mWebBrowserCtrl( browserCtrl ),
	mMediaSource(media_source)
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
BOOL LLWebBrowserTexture::needsRender()
{
	if ( mWebBrowserCtrl->getFrequentUpdates() || 
		mWebBrowserCtrl->getAlwaysRefresh() ||
		mWebBrowserCtrl->getForceUpdate() )
	{
		// only update mozilla/texture occasionally
		if ( mElapsedTime.getElapsedTimeF32() > ( 1.0f / 15.0f ) )
		{
			return TRUE;
		}
	}

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////
//
BOOL LLWebBrowserTexture::render()
{
	if (!mMediaSource)
		return FALSE;
	
	// frequent updates turned on?
	if ( mWebBrowserCtrl->getFrequentUpdates() || 
		mWebBrowserCtrl->getAlwaysRefresh() ||
		mWebBrowserCtrl->getForceUpdate() )
	{

		if ( mNeedsUpdate )
		{

			const unsigned char* pixels = mMediaSource->getMediaData();
			if ( ! pixels )
				return FALSE;
			
			mNeedsUpdate = false;
			mWebBrowserCtrl->setForceUpdate(false);

			S32 media_depth  = mMediaSource->getMediaDepth();
			S32 media_width  = mMediaSource->getMediaWidth();
			S32 media_height = mMediaSource->getMediaHeight();
			
			// these are both invalid conditions and should never happen but SL-27583 indicates it does
			if ((media_width < 1) || (media_depth < 2))
				return FALSE;

			// Browser depth hasn't changed.  
			if(mLastBrowserDepth == media_depth)
			{
				S32 width  = llmin(media_width, mBrowserWidth);
				S32 height = llmin(media_height, mBrowserHeight);

				S32 media_data_width  = mMediaSource->getMediaDataWidth();
				S32 media_data_height = mMediaSource->getMediaDataHeight();

				// Just grab the pixels.
				if ( media_data_width > 0 && media_data_height > 0 &&
						media_data_width < MAX_DIMENSION && media_data_height < MAX_DIMENSION )
				{
					mTexture->setSubImage( pixels, 
											media_data_width, media_data_height,
												0, 0, 
													llmin(width, media_data_width), llmin(media_data_height, height) );
				};
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
void LLWebBrowserTexture::setNeedsUpdate()
{
	mNeedsUpdate = true;
}

////////////////////////////////////////////////////////////////////////////////
//
void LLWebBrowserTexture::resize( S32 new_width, S32 new_height )
{
	if (!mMediaSource)
		return;
	
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

	mMediaSource->setRequestedMediaSize(mBrowserWidth, mBrowserHeight);

	// HACK - this code is executing a render - resize should call render() instead
	// (and render() should be refactored so it doesn't call resize())
	
	mMediaSource->updateMedia();
	const unsigned char* pixels = mMediaSource->getMediaData();

	S32 media_width  = mMediaSource->getMediaWidth();
	S32 media_height = mMediaSource->getMediaHeight();
	S32 media_depth  = mMediaSource->getMediaDepth();

	// these are both invalid conditions and should never happen but SL-27583 indicates it does
	if ( media_width < 1 || media_depth < 2 )
		return;
	
	releaseGLTexture();

	// calculate the next power of 2 bigger than reqquested size for width and height
	for ( mWidth = 1; mWidth < mBrowserWidth; mWidth <<= 1 )
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

	switch(media_depth)
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


	S32 width  = llmin(media_width, mBrowserWidth);
	S32 height = llmin(media_height, mBrowserHeight);

	S32 media_data_width  = mMediaSource->getMediaDataWidth();
	S32 media_data_height = mMediaSource->getMediaDataHeight();

	if (pixels)
	{
		mTexture->setSubImage( pixels, media_data_width, media_data_height,
							   0, 0, width, height );
	}
	
	mLastBrowserDepth = media_depth;
}

// virtual
LLXMLNodePtr LLWebBrowserCtrl::getXML(bool save_children) const
{
	LLXMLNodePtr node = LLUICtrl::getXML();

	node->setName(LL_WEB_BROWSER_CTRL_TAG);

	return node;
}

LLView* LLWebBrowserCtrl::fromXML(LLXMLNodePtr node, LLView *parent, LLUICtrlFactory *factory)
{
	std::string name("web_browser");
	node->getAttributeString("name", name);

	std::string start_url("start_url");
	node->getAttributeString("start_url", start_url );

	BOOL border_visible = true;
	node->getAttributeBOOL("border_visible", border_visible);

	LLRect rect;
	createRect(node, rect, parent, LLRect());

	LLWebBrowserCtrl* web_browser = new LLWebBrowserCtrl( name, rect );

	if(node->hasAttribute("caret_color"))
	{
		LLColor4 color;
		LLUICtrlFactory::getAttributeColor(node, "caret_color", color);
		LLColor4U colorU = LLColor4U(color);
		web_browser->setCaretColor( colorU.mV[0], colorU.mV[1], colorU.mV[2] );
	}

	BOOL ignore_ui_scale = web_browser->getIgnoreUIScale();
	node->getAttributeBOOL("ignore_ui_scale", ignore_ui_scale);
	web_browser->setIgnoreUIScale((bool)ignore_ui_scale);

	web_browser->initFromXML(node, parent);

	web_browser->setHomePageUrl( start_url );

	web_browser->setBorderVisible( border_visible );

	web_browser->navigateHome();

	return web_browser;
}

std::string LLWebBrowserCtrl::getCurrentNavUrl()
{
	return mCurrentNavUrl;
}

