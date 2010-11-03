/** 
 * @file llfloaterhtml.cpp
 * @brief In-world HTML dialog
 *
 * $LicenseInfo:firstyear=2005&license=viewergpl$
 * 
 * Copyright (c) 2005-2009, Linden Research, Inc.
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

#include "llfloaterhtml.h"

// viewer includes
#include "lluictrlfactory.h"
#include "llviewercontrol.h"
#include "lllineeditor.h"
#include "llviewerwindow.h"
#include "llweb.h"


LLFloaterHtml* LLFloaterHtml::sInstance = 0;

////////////////////////////////////////////////////////////////////////////////
//
LLFloaterHtml* LLFloaterHtml::getInstance()
{
    if ( ! sInstance )
        sInstance = new LLFloaterHtml();

	return sInstance;
}

////////////////////////////////////////////////////////////////////////////////
//
LLFloaterHtml::LLFloaterHtml()
:	LLFloater( std::string("HTML Floater") )

        ,
	mWebBrowser( 0 )
{
	LLUICtrlFactory::getInstance()->buildFloater( this, "floater_html.xml" );

	childSetAction("back_btn", onClickBack, this);
	childSetAction("home_btn", onClickHome, this);
	childSetAction("forward_btn", onClickForward, this);
	childSetAction("close_btn", onClickClose, this);
	childSetCommitCallback("url_edit", onCommitUrlEdit, this );
	childSetAction("go_btn", onClickGo, this );

	// reposition floater from saved settings
	LLRect rect = gSavedSettings.getRect( "FloaterHtmlRect" );
	reshape( rect.getWidth(), rect.getHeight(), FALSE );
	setRect( rect );

	mWebBrowser = getChild<LLWebBrowserCtrl>("html_floater_browser" );
	if ( mWebBrowser )
	{
		// open links in internal browser
		mWebBrowser->setOpenInExternalBrowser( false );
	}
}

////////////////////////////////////////////////////////////////////////////////
//
LLFloaterHtml::~LLFloaterHtml()
{
	// save position of floater
	gSavedSettings.setRect( "FloaterHtmlRect", getRect() );

	sInstance = 0;
}

////////////////////////////////////////////////////////////////////////////////
// virtual 
void LLFloaterHtml::draw()
{
	// enable/disable buttons depending on state
	if ( mWebBrowser )
	{
		bool enable_back = mWebBrowser->canNavigateBack();	
		childSetEnabled( "back_btn", enable_back );

		bool enable_forward = mWebBrowser->canNavigateForward();	
		childSetEnabled( "forward_btn", enable_forward );
	};

	LLFloater::draw();
}

////////////////////////////////////////////////////////////////////////////////
//
void LLFloaterHtml::show( std::string content_id, bool open_link_external, bool open_app_slurls )
{
	// calculate the XML labels we'll need (if only XML folders worked)
	std::string title_str = content_id + "_title";
	std::string url_str = content_id + "_url";

	std::string title = getString( title_str );
	std::string url = getString( url_str );

	show( url, title, open_link_external, open_app_slurls );
}

////////////////////////////////////////////////////////////////////////////////
//
void LLFloaterHtml::show( std::string start_url, std::string title, bool open_link_external, bool trusted_browser )
{
	// set the title 
	setTitle( title );

	// navigate to the URL
	if ( mWebBrowser )
	{
		mWebBrowser->setTrusted( trusted_browser );
		mWebBrowser->setOpenInExternalBrowser( open_link_external );
		mWebBrowser->navigateTo( start_url );
	}

	// make floater appear
	setVisibleAndFrontmost();
}

////////////////////////////////////////////////////////////////////////////////
//
std::string LLFloaterHtml::getSupportUrl()
{
	return getString("support_page_url");
}

////////////////////////////////////////////////////////////////////////////////
//
void LLFloaterHtml::onClose( bool app_quitting )
{
	setVisible( false );
	// HACK for fast XML iteration replace with:
	// destroy();
}

////////////////////////////////////////////////////////////////////////////////
//
void LLFloaterHtml::onClickClose( void* data )
{
	LLFloaterHtml* self = ( LLFloaterHtml* )data;
	self->close();
}

////////////////////////////////////////////////////////////////////////////////
// static
void LLFloaterHtml::onClickBack( void* data )
{
	LLFloaterHtml* self = ( LLFloaterHtml* )data;
	if ( self )
	{
		if ( self->mWebBrowser )
		{
			self->mWebBrowser->navigateBack();
		};
	};
}

////////////////////////////////////////////////////////////////////////////////
//
void LLFloaterHtml::onClickHome( void* data )
{
	LLFloaterHtml* self = ( LLFloaterHtml* )data;
	if ( self )
	{
		if ( self->mWebBrowser )
		{
			std::string home_url = self->getString("home_page_url");
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
	};
}

////////////////////////////////////////////////////////////////////////////////
// static
void LLFloaterHtml::onClickForward( void* data )
{
	LLFloaterHtml* self = ( LLFloaterHtml* )data;
	if ( self )
	{
		if ( self->mWebBrowser )
		{
			self->mWebBrowser->navigateForward();
		};
	};
}

////////////////////////////////////////////////////////////////////////////////
// static
void LLFloaterHtml::onCommitUrlEdit(LLUICtrl* ctrl, void* user_data)
{
	LLFloaterHtml* self = (LLFloaterHtml*)user_data;

	LLLineEditor* editor = (LLLineEditor*)ctrl;
	std::string url = editor->getText();

	if ( self->mWebBrowser )
	{
		self->mWebBrowser->navigateTo( url );
	};
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
			if ( self->mWebBrowser )
			{
				self->mWebBrowser->navigateTo( url );
			}
		}
	}
}
