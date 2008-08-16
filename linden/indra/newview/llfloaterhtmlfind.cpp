/** 
 * @file llfloaterhtmlfind.cpp
 * @brief HTML Find floater - uses embedded web browser control
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


#include "llfloaterhtmlfind.h"

#include "llvieweruictrlfactory.h"
#include "llbutton.h"
#include "llwebbrowserctrl.h"
#include "llviewerwindow.h"
#include "llviewercontrol.h"
#include "viewer.h"
#include "llfloaterworldmap.h"
#include "llfloater.h"

#if LL_LIBXUL_ENABLED

class LLFloaterHtmlFind :
	public LLFloater,
	public LLWebBrowserCtrlObserver
{
public:
	LLFloaterHtmlFind();
	virtual ~LLFloaterHtmlFind();
	
	virtual BOOL postBuild();
	virtual void onClose( bool app_quitting );
	virtual void draw();
	
	static void show( void* url_string = NULL );
	static void onClickClose( void* data );
	
	static void onFocusGained( LLUICtrl* ctrl, void* data );
	
	// embedded browser observables
	virtual void onClickLinkHref( const EventType& eventIn );
	
protected:
	LLWebBrowserCtrl* mWebBrowser;
	static LLFloaterHtmlFind* sInstance;
	LLButton* mCloseButton;
};

LLFloaterHtmlFind* LLFloaterHtmlFind::sInstance = 0;

////////////////////////////////////////////////////////////////////////////////
//
LLFloaterHtmlFind::LLFloaterHtmlFind() :
	LLFloater( "HTML Find" ),
	mWebBrowser( 0 ),
	mCloseButton( 0 )
{
	sInstance = this;
}

////////////////////////////////////////////////////////////////////////////////
//
LLFloaterHtmlFind::~LLFloaterHtmlFind()
{
	// stop observing browser events
	if ( mWebBrowser )
	{
		mWebBrowser->remObserver( this );
	};

	// save position of floater
	gSavedSettings.setRect( "HtmlFindRect", mRect );

	sInstance = 0;
}

////////////////////////////////////////////////////////////////////////////////
//
BOOL LLFloaterHtmlFind::postBuild()
{
	mCloseButton = LLUICtrlFactory::getButtonByName(this, "close_btn" );
	if ( mCloseButton )
	{
		mCloseButton->setClickedCallback( onClickClose );
		mCloseButton->setCallbackUserData( this );
		setDefaultBtn( mCloseButton );
	};

	mWebBrowser = LLViewerUICtrlFactory::getWebBrowserByName(this,  "html_find_browser" );

	//	// observe browser events
	mWebBrowser->addObserver( this );

	// browser built so navigate to the right page
	LLString homePageUrl( "http://user.lindenlab.com/~callum/search.php" );
	mWebBrowser->navigateTo( homePageUrl );

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
//
void LLFloaterHtmlFind::draw()
{
	// just call the base class for now - more later
	LLFloater::draw();
}

////////////////////////////////////////////////////////////////////////////////
//
void LLFloaterHtmlFind::show( void* url_string )
{
	if ( sInstance )
	{
		sInstance->setVisibleAndFrontmost();
		return;
	};

	LLFloaterHtmlFind* self = new LLFloaterHtmlFind;

	// create floater from its XML definition
	gUICtrlFactory->buildFloater( self, "floater_html_find.xml" );

	// reposition floater from saved settings
	LLRect rect = gSavedSettings.getRect( "HtmlFindRect" );
	self->reshape( rect.getWidth(), rect.getHeight(), FALSE );
	self->setRect( rect );
}

////////////////////////////////////////////////////////////////////////////////
//
void LLFloaterHtmlFind::onClose( bool app_quitting )
{
	setVisible( false );
}

////////////////////////////////////////////////////////////////////////////////
//
void LLFloaterHtmlFind::onClickClose( void* data )
{
	LLFloaterHtmlFind* self = ( LLFloaterHtmlFind* )data;

	self->setVisible( false );
}

////////////////////////////////////////////////////////////////////////////////
// virtual (observer on enbedded browser)
void LLFloaterHtmlFind::onClickLinkHref( const EventType& eventIn )
{
	#if !LL_RELEASE_FOR_DOWNLOAD
	llinfos << "MOZ> onClickHref=" << eventIn.getStringValue() << llendl;
	#endif
	// if it was a secondlife:// address
	if ( eventIn.getStringValue().substr( 0, std::string( "secondlife://" ).length() ) == "secondlife://" )
	{
		// parse out sim name and coordinates
		LLURLSimString::setString( eventIn.getStringValue() );
		LLURLSimString::parse();

		// if there is a world map
		if ( gFloaterWorldMap )
		{
			#if !LL_RELEASE_FOR_DOWNLOAD
			llinfos << "MOZ> sim name is [" << LLURLSimString::sInstance.mSimName.c_str() << "]" << llendl;
			#endif
			// mark where the destination is
			gFloaterWorldMap->trackURL( LLURLSimString::sInstance.mSimName.c_str(),
										LLURLSimString::sInstance.mX,
										LLURLSimString::sInstance.mY,
										LLURLSimString::sInstance.mZ );

			// display map
			LLFloaterWorldMap::show( NULL, TRUE );
		};
	};
}

// static
void LLHtmlFind::show(void* url_string)
{
	LLFloaterHtmlFind::show(url_string);
}

#endif	// LL_LIBXUL_ENABLED
