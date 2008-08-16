/** 
 * @file llfloaterhtml.cpp
 * @brief In-world HTML dialog
 *
 * $LicenseInfo:firstyear=2005&license=viewergpl$
 * 
 * Copyright (c) 2005-2008, Linden Research, Inc.
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

#include "llviewerprecompiledheaders.h"

#include "llfloaterhtml.h"

// viewer includes
#include "llvieweruictrlfactory.h"
#include "llviewercontrol.h"
#include "lllineeditor.h"
#include "llviewerwindow.h"
#include "llweb.h"

#include "llwebbrowserctrl.h"

LLFloaterHtml* LLFloaterHtml::sInstance = 0;
LLViewerHtmlHelp gViewerHtmlHelp;

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

	childSetAction("back_btn", onClickBack, this);
	childSetAction("home_btn", onClickHome, this);
	childSetAction("forward_btn", onClickForward, this);
	childSetAction("close_btn", onClickClose, this);
	childSetCommitCallback("url_edit", onCommitUrlEdit, this );
	childSetAction("go_btn", onClickGo, this );

	// reposition floater from saved settings
	LLRect rect = gSavedSettings.getRect( "HtmlFloaterRect" );
	reshape( rect.getWidth(), rect.getHeight(), FALSE );
	setRect( rect );

#if LL_LIBXUL_ENABLED
	mWebBrowser = LLViewerUICtrlFactory::getWebBrowserByName(this,  "html_floater_browser" );
	if ( mWebBrowser )
	{
		// open links in internal browser
		mWebBrowser->setOpenInExternalBrowser( false );
	}
#endif // LL_LIBXUL_ENABLED
}

////////////////////////////////////////////////////////////////////////////////
//
LLFloaterHtml::~LLFloaterHtml()
{
	// save position of floater
	gSavedSettings.setRect( "HtmlFloaterRect", mRect );

	sInstance = 0;
}

////////////////////////////////////////////////////////////////////////////////
// virtual 
void LLFloaterHtml::draw()
{
#if LL_LIBXUL_ENABLED
	// enable/disable buttons depending on state
	if ( mWebBrowser )
	{
		bool enable_back = mWebBrowser->canNavigateBack();	
		childSetEnabled( "back_btn", enable_back );

		bool enable_forward = mWebBrowser->canNavigateForward();	
		childSetEnabled( "forward_btn", enable_forward );
	};
#endif // LL_LIBXUL_ENABLED

	LLFloater::draw();
}

////////////////////////////////////////////////////////////////////////////////
//
void LLFloaterHtml::show( LLString content_id, bool open_app_slurls )
{
	// calculate the XML labels we'll need (if only XML folders worked)
	LLString title_str = content_id + "_title";
	LLString url_str = content_id + "_url";

	std::string title = childGetValue( title_str ).asString();
	std::string url = childGetValue( url_str ).asString();
	show( url, title, open_app_slurls );
}

////////////////////////////////////////////////////////////////////////////////
//
void LLFloaterHtml::show( std::string start_url, std::string title, bool open_app_slurls )
{
	// set the title 
	setTitle( title );

#if LL_LIBXUL_ENABLED
	// navigate to the URL
	if ( mWebBrowser )
	{
		mWebBrowser->setOpenAppSLURLs( open_app_slurls );
		mWebBrowser->navigateTo( start_url );
	}
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
// static
void LLFloaterHtml::onClickBack( void* data )
{
#if LL_LIBXUL_ENABLED
	LLFloaterHtml* self = ( LLFloaterHtml* )data;
	if ( self )
	{
		if ( self->mWebBrowser )
		{
			self->mWebBrowser->navigateBack();
		};
	};
#endif // LL_LIBXUL_ENABLED
}

////////////////////////////////////////////////////////////////////////////////
//
void LLFloaterHtml::onClickHome( void* data )
{
	LLFloaterHtml* self = ( LLFloaterHtml* )data;
	if ( self )
	{
#if LL_LIBXUL_ENABLED
		if ( self->mWebBrowser )
		{
			std::string home_url = self->childGetText("home_page_url");
			if ( home_url.length() > 4 )
			{
				self->mWebBrowser->navigateTo( home_url );
			}
			else
			{
				llwarns << "Invalid home page specified for HTML floater - navigating to default" << llendl;
				self->mWebBrowser->navigateTo( "http://secondlife.com" );
			}
		};
#endif // LL_LIBXUL_ENABLED
	};
}

////////////////////////////////////////////////////////////////////////////////
// static
void LLFloaterHtml::onClickForward( void* data )
{
	LLFloaterHtml* self = ( LLFloaterHtml* )data;
	if ( self )
	{
#if LL_LIBXUL_ENABLED
		if ( self->mWebBrowser )
		{
			self->mWebBrowser->navigateForward();
		};
#endif // LL_LIBXUL_ENABLED
	};
}

////////////////////////////////////////////////////////////////////////////////
// static
void LLFloaterHtml::onCommitUrlEdit(LLUICtrl* ctrl, void* user_data)
{
#if LL_LIBXUL_ENABLED
	LLFloaterHtml* self = (LLFloaterHtml*)user_data;

	LLLineEditor* editor = (LLLineEditor*)ctrl;
	std::string url = editor->getText();

	if ( self->mWebBrowser )
	{
		self->mWebBrowser->navigateTo( url );
	};
#endif // LL_LIBXUL_ENABLED
}

////////////////////////////////////////////////////////////////////////////////
//  static
void LLFloaterHtml::onClickGo( void* data )
{
	LLFloaterHtml* self = ( LLFloaterHtml* )data;
	if ( self )
	{
		std::string url = self->childGetValue( "url_edit" ).asString();
		if ( url.length() )
		{
#if LL_LIBXUL_ENABLED
			if ( self->mWebBrowser )
			{
				self->mWebBrowser->navigateTo( url );
			};
#endif // LL_LIBXUL_ENABLED
		};
	};
}

////////////////////////////////////////////////////////////////////////////////
//
static void onClickF1HelpLoadURL(S32 option, void* userdata)
{
	if (option == 0)
	{
		// choose HELP url based on selected language - default to english language support page
		LLString lang = LLUI::sConfigGroup->getString("Language");

		// *TODO:Translate
		// this sucks but there isn't a way to grab an arbitrary string from an XML file
		// (using llcontroldef strings causes problems if string don't exist)
		LLString help_url( "http://secondlife.com/support" );
		if ( lang == "ja" )
			help_url = "http://help.secondlife.com/jp";
		else
		if ( lang == "ko" )
			help_url = "http://help.secondlife.com/kr";
		else
		if ( lang == "pt" )
			help_url = "http://help.secondlife.com/pt";
		else
		if ( lang == "de" )
			help_url = "http://de.secondlife.com/support";

		LLWeb::loadURL( help_url );
	}
}

LLViewerHtmlHelp::LLViewerHtmlHelp()
{
	LLUI::setHtmlHelp(this);
}

LLViewerHtmlHelp::~LLViewerHtmlHelp()
{
	LLUI::setHtmlHelp(NULL);
}

void LLViewerHtmlHelp::show(std::string url, std::string title)
{
    gViewerWindow->alertXml("ClickOpenF1Help", onClickF1HelpLoadURL, (void*) NULL);
}
