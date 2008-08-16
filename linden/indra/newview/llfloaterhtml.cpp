/** 
 * @file llfloaterhtml.cpp
 * @brief In-world HTML dialog
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

#include "llviewerprecompiledheaders.h"

#include "llvieweruictrlfactory.h"
#include "llviewerwindow.h"
#include "llviewercontrol.h"
#include "llfloaterhtml.h"
#include "llfloaterhtmlhelp.h"

LLFloaterHtml* LLFloaterHtml::sInstance = 0;

////////////////////////////////////////////////////////////////////////////////
//
LLFloaterHtml* LLFloaterHtml::getInstance()
{
    if ( ! sInstance )
        sInstance = new LLFloaterHtml;

	return sInstance;
}

////////////////////////////////////////////////////////////////////////////////
//
LLFloaterHtml::LLFloaterHtml()
:	LLFloater( "HTML Floater" )

#if LL_LIBXUL_ENABLED
        ,
	mWebBrowser( 0 )
#endif // LL_LIBXUL_ENABLED
{
	// create floater from its XML definition
	gUICtrlFactory->buildFloater( this, "floater_html.xml" );

	// reposition floater from saved settings
	LLRect rect = gSavedSettings.getRect( "HtmlFloaterRect" );
	reshape( rect.getWidth(), rect.getHeight(), FALSE );
	setRect( rect );

#if LL_LIBXUL_ENABLED
	mWebBrowser = LLViewerUICtrlFactory::getWebBrowserByName(this,  "html_floater_browser" );
	if ( mWebBrowser )
	{
		// observe browser events
		mWebBrowser->addObserver( this );

		// make links open in external browser
		mWebBrowser->setOpenInExternalBrowser( true );

		// don't automatically open secondlife links since we want to catch
		// special ones that do other stuff (like open F1 Help)
		mWebBrowser->setOpenSecondLifeLinksInMap( false );
	}
#endif // LL_LIBXUL_ENABLED
			
	childSetAction("close_btn", onClickClose, this);
	setDefaultBtn("close_btn");
}

////////////////////////////////////////////////////////////////////////////////
//
LLFloaterHtml::~LLFloaterHtml()
{
#if LL_LIBXUL_ENABLED
	// stop observing browser events
	if ( mWebBrowser )
		mWebBrowser->remObserver( this );
#endif // LL_LIBXUL_ENABLED

	// save position of floater
	gSavedSettings.setRect( "HtmlFloaterRect", mRect );

	sInstance = 0;
}

////////////////////////////////////////////////////////////////////////////////
//
void LLFloaterHtml::show( LLString content_id )
{
	// calculate the XML labels we'll need (if only XML folders worked)
	LLString title_str = content_id + "_title";
	LLString url_str = content_id + "_url";

	// set the title 
	setTitle( childGetValue( title_str ).asString() );

#if LL_LIBXUL_ENABLED
	// navigate to the URL
	if ( mWebBrowser )
		mWebBrowser->navigateTo( childGetValue( url_str ).asString() );
#endif // LL_LIBXUL_ENABLED

	// make floater appear
	setVisibleAndFrontmost();
}

////////////////////////////////////////////////////////////////////////////////
//
void LLFloaterHtml::onClose( bool app_quitting )
{
	setVisible( false );
}

////////////////////////////////////////////////////////////////////////////////
//
void LLFloaterHtml::onClickClose( void* data )
{
	LLFloaterHtml* self = ( LLFloaterHtml* )data;

	self->setVisible( false );
}

////////////////////////////////////////////////////////////////////////////////
//
void LLFloaterHtml::onClickLinkSecondLife( const EventType& eventIn )
{
#if LL_LIBXUL_ENABLED
	const std::string protocol( "secondlife://app." );

	// special 'app' secondlife link (using a different protocol - one that isn't registered in the browser) causes bad
	// things to happen and Mozilla stops responding because it can't display the "invalid protocol dialog)
	if ( LLString::compareInsensitive( eventIn.getStringValue().substr( 0, protocol.length() ).c_str(), protocol.c_str() ) == 0 )
	{
		// extract the command string
		LLString cmd = eventIn.getStringValue().substr( protocol.length() );

		// command is open the F1 Help floater
		if ( LLString::compareInsensitive( cmd.c_str() , "floater.html.help" ) == 0 )
		{
			gViewerHtmlHelp.show();
		}
	}
	else
	// regular secondlife link - just open the map as normal
	{
		mWebBrowser->openMapAtlocation( eventIn.getStringValue() );
	}
#endif // LL_LIBXUL_ENABLED
};
