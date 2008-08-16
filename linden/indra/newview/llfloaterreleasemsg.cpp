/** 
 * @file llfloaterreleasemsg.cpp
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
#include "llfloaterreleasemsg.h"
#include "llagent.h"
#include "llviewerregion.h"

extern LLAgent gAgent;
extern LLString gLastVersionChannel;

LLFloaterReleaseMsg* LLFloaterReleaseMsg::sInstance = 0;


////////////////////////////////////////////////////////////////////////////////
//
LLFloaterReleaseMsg* LLFloaterReleaseMsg::getInstance()
{
    if ( ! sInstance )
        sInstance = new LLFloaterReleaseMsg;

	return sInstance;
}

////////////////////////////////////////////////////////////////////////////////
//
LLFloaterReleaseMsg::LLFloaterReleaseMsg()
:	LLFloater( "Release Message Floater" )

#if LL_LIBXUL_ENABLED
        ,
	mWebBrowser( 0 )
#endif // LL_LIBXUL_ENABLED
{
	// create floater from its XML definition
	gUICtrlFactory->buildFloater( this, "floater_sim_release_message.xml" );

	mTitleBase = getTitle();

	// reposition floater from saved settings
	LLRect rect = gSavedSettings.getRect( "HtmlReleaseMessage" );
	reshape( rect.getWidth(), rect.getHeight(), FALSE );
	setRect( rect );

#if LL_LIBXUL_ENABLED
	mWebBrowser = LLViewerUICtrlFactory::getWebBrowserByName(this,  "release_message_floater_browser" );
	if ( mWebBrowser )
	{
		// observe browser events
		mWebBrowser->addObserver( this );

		// make links open in external browser
		mWebBrowser->setOpenInExternalBrowser( true );

		// don't automatically open secondlife links since we want to catch
		// special ones that do other stuff (like open F1 Help)
		//mWebBrowser->setOpenSecondLifeLinksInMap( false );
	}
#endif // LL_LIBXUL_ENABLED
			
	childSetAction("close_btn", onClickClose, this);
	setDefaultBtn("close_btn");
}

////////////////////////////////////////////////////////////////////////////////
//
LLFloaterReleaseMsg::~LLFloaterReleaseMsg()
{
#if LL_LIBXUL_ENABLED
	// stop observing browser events
	if ( mWebBrowser )
		mWebBrowser->remObserver( this );
#endif // LL_LIBXUL_ENABLED

	// save position of floater
	gSavedSettings.setRect( "HtmlReleaseMessage", mRect );

	sInstance = 0;
}

////////////////////////////////////////////////////////////////////////////////
//
void LLFloaterReleaseMsg::show()
{

	std::string url = gAgent.getRegion()->getCapability("ServerReleaseNotes");

	if (url.empty()) return;

	llinfos << "Release message url: " << url << llendl;

	if (!sInstance)
	{
		sInstance = new LLFloaterReleaseMsg();
		sInstance->center();
	}

	sInstance->setTitle(sInstance->mTitleBase + " " + gLastVersionChannel);
	sInstance->open();

#if LL_LIBXUL_ENABLED
	// navigate to the URL
	if ( sInstance->mWebBrowser )
		sInstance->mWebBrowser->navigateTo( url );
#endif // LL_LIBXUL_ENABLED

	// make floater appear
	sInstance->setVisibleAndFrontmost();

	sInstance->draw();
}


////////////////////////////////////////////////////////////////////////////////
//
void LLFloaterReleaseMsg::onClickClose( void* data )
{
	LLFloaterReleaseMsg* self = ( LLFloaterReleaseMsg* )data;

	self->setVisible( false );
}


