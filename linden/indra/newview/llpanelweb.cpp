/** 
 * @file llpanelweb.cpp
 * @brief Web browser options
 *
 * Copyright (c) 2003-2007, Linden Research, Inc.
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

#include "llpanelweb.h"

#include "llbutton.h"
#include "llscrolllistctrl.h"
#include "llcheckboxctrl.h"
#include "llradiogroup.h"
#include "lllineeditor.h"
#include "llfirstuse.h"
#include "llviewercontrol.h"
#include "llmediaengine.h"
#include "llstartup.h"
#include "llurlwhitelist.h"
#include "llvieweruictrlfactory.h"

LLPanelWeb::LLPanelWeb()
:	LLPanel("Web Panel"),
	mExternalBrowserCheck(NULL),
	mBrowserHomePage(NULL),
	mWebPagesOnPrimsCheck(NULL),
	mTrustedSitesList(NULL),
	mProxyEnabled(NULL),
	mProxyAddress(NULL),
	mProxyPort(NULL),
	mProxySocks45(NULL),
	mProxyExclusions(NULL),
	mAddTrustedSite(NULL),
	mTrustedSiteEntry(NULL),
	mRemTrustedSite(NULL)
{
	mLoggedIn = ( gStartupState >= STATE_STARTED );
}

//-----------------------------------------------------------------------------
// postBuild()
//-----------------------------------------------------------------------------
BOOL LLPanelWeb::postBuild()
{
	mExternalBrowserCheck = LLViewerUICtrlFactory::getCheckBoxByName(this, "external_browser_check");
	mExternalBrowserCheck->setCallbackUserData ( this );
	mExternalBrowserCheck->setCommitCallback ( onCommitExternalBrowser );

	mWebPagesOnPrimsCheck = LLViewerUICtrlFactory::getCheckBoxByName(this, "web_pages_on_prims_check");

	mBrowserHomePage = LLViewerUICtrlFactory::getLineEditorByName(this, "home_page");

	mProxyEnabled = LLViewerUICtrlFactory::getCheckBoxByName(this, "proxy_enabled");
	mProxyEnabled->setCallbackUserData ( this );
	mProxyEnabled->setCommitCallback ( onCommitProxyEnabled );
	mProxyAddress = LLViewerUICtrlFactory::getLineEditorByName(this, "proxy_address");
	mProxyPort = LLViewerUICtrlFactory::getLineEditorByName(this, "proxy_port");
	mProxySocks45 = LLViewerUICtrlFactory::getRadioGroupByName(this, "socks_4_5");
	mProxyExclusions = LLViewerUICtrlFactory::getLineEditorByName(this, "proxy_exclusions");

	mAddTrustedSite = LLViewerUICtrlFactory::getButtonByName(this, "add_trusted");
	mAddTrustedSite->setEnabled ( FALSE );
	mAddTrustedSite->setCallbackUserData ( this );
	mAddTrustedSite->setClickedCallback ( onAddTrustedSite );

	mTrustedSitesList = LLViewerUICtrlFactory::getScrollListByName(this, "trusted_sites_list");
	mTrustedSitesList->setAllowMultipleSelection ( TRUE );
	mTrustedSitesList->setCommitOnSelectionChange ( TRUE );
	mTrustedSitesList->setCallbackUserData ( this );
	mTrustedSitesList->setCommitCallback ( onTrustedSiteListCommit );

	mTrustedSiteEntry = LLViewerUICtrlFactory::getLineEditorByName(this, "trusted_site_entry");
	mTrustedSiteEntry->setCallbackUserData ( this );
	mTrustedSiteEntry->setKeystrokeCallback ( onTrustedSiteEntryKeystroke );

	mRemTrustedSite = LLViewerUICtrlFactory::getButtonByName(this, "rem_trusted");
	mRemTrustedSite->setEnabled ( FALSE );
	mRemTrustedSite->setCallbackUserData ( this );
	mRemTrustedSite->setClickedCallback ( onRemTrustedSite );
	
	BOOL use_external_browser = gSavedSettings.getBOOL("UseExternalBrowser");
	mExternalBrowserCheck->set(use_external_browser);

	LLString strVal = gSavedSettings.getString ( "BrowserHomePage" );
	mBrowserHomePage->setText ( strVal );

	BOOL boolVal = gSavedSettings.getBOOL ( "BrowserProxyEnabled" );
	mProxyEnabled->set ( boolVal );

	strVal = gSavedSettings.getString ( "BrowserProxyAddress" );
	mProxyAddress->setText ( strVal );

	S32 port = gSavedSettings.getS32 ( "BrowserProxyPort" );
	std::stringstream codec;
	codec << port;
	mProxyPort->setText ( codec.str () );

	S32 numVal = gSavedSettings.getS32 ( "BrowserProxySocks45" );
	mProxySocks45->setSelectedIndex ( numVal - 4 );

	strVal = gSavedSettings.getString ( "BrowserProxyExclusions" );
	mProxyExclusions->setText ( strVal );

	// switch on/off UI depending on state of 'use external browser' checkbox
	configExternaBrowserEnabledUI ( mExternalBrowserCheck->get() );

	// switch on/off UI depending on which type of proxy is chosen (only if internal browseR)
	if ( ! mExternalBrowserCheck->get() )
		 configProxyEnabledUI ( mProxyEnabled->get() );

	BOOL use_web_pages_on_prims = gSavedSettings.getBOOL("UseWebPagesOnPrims");
	mWebPagesOnPrimsCheck->set(use_web_pages_on_prims);

	// load the list of trusted sites
	loadTrustedSiteList ();

	return TRUE;
}

// helper function to enable/disable proxy UI based on what type of proxy is selected
void LLPanelWeb::configProxyEnabledUI ( BOOL enabled )
{
	if ( enabled )
	{
		mProxyAddress->setEnabled ( TRUE );
		mProxyPort->setEnabled ( TRUE );
		mProxySocks45->setEnabled ( TRUE);
		mProxyExclusions->setEnabled ( TRUE );
	}
	else
	{
		mProxyAddress->setEnabled ( FALSE );
		mProxyPort->setEnabled ( FALSE );
		mProxySocks45->setEnabled ( FALSE );
		mProxyExclusions->setEnabled ( FALSE );
	};
}

// helper function to enable/disable proxy UI based on what type of proxy is selected
void LLPanelWeb::configExternaBrowserEnabledUI ( BOOL enabled )
{
	if ( enabled )
	{
		mBrowserHomePage->setEnabled ( FALSE );
		mWebPagesOnPrimsCheck->setEnabled ( FALSE );
		mTrustedSitesList->setEnabled ( FALSE );
		mProxyEnabled->setEnabled ( FALSE );
		mProxyAddress->setEnabled ( FALSE );
		mProxyPort->setEnabled ( FALSE );
		mProxySocks45->setEnabled ( FALSE );
		mProxyExclusions->setEnabled ( FALSE );
		mAddTrustedSite->setEnabled ( FALSE );
		mTrustedSiteEntry->setEnabled ( FALSE );
		mRemTrustedSite->setEnabled ( FALSE );
	}
	else
	{
		mBrowserHomePage->setEnabled ( TRUE );
		mWebPagesOnPrimsCheck->setEnabled ( TRUE );
		mTrustedSitesList->setEnabled ( TRUE );
		mProxyEnabled->setEnabled ( TRUE );
		mProxyAddress->setEnabled ( TRUE );
		mProxyPort->setEnabled ( TRUE );
		mProxySocks45->setEnabled ( TRUE);
		mProxyExclusions->setEnabled ( TRUE );
		mAddTrustedSite->setEnabled ( TRUE );
		mTrustedSiteEntry->setEnabled ( TRUE );

		// only set this to enabled if there is text in the entry widget
		if ( mTrustedSiteEntry->getLength() )
			mAddTrustedSite->setEnabled ( TRUE );
		else
			mAddTrustedSite->setEnabled ( FALSE );

		// only set this to enabled if something is selected in the list of trusted sites
		if ( mTrustedSitesList->getFirstSelected () )
			mRemTrustedSite->setEnabled ( TRUE );
		else
			mRemTrustedSite->setEnabled ( FALSE );
	};

	// final check - disable the white list options if not
	// logged in since they are on a per-user basis
	if ( ! mLoggedIn )
	{
		mTrustedSitesList->setEnabled ( FALSE );
		mAddTrustedSite->setEnabled ( FALSE );
		mTrustedSiteEntry->setEnabled ( FALSE );
		mRemTrustedSite->setEnabled ( FALSE );
	};
}

void LLPanelWeb::onCommitExternalBrowser ( LLUICtrl* ctrl, void* data )
{
	LLPanelWeb* self = (LLPanelWeb*)data;

	// update state of UI when proxy type changes
	BOOL value = self->mExternalBrowserCheck->get ();
	self->configExternaBrowserEnabledUI ( value );
}

void LLPanelWeb::onCommitProxyEnabled ( LLUICtrl* ctrl, void* data )
{
	LLPanelWeb* self = (LLPanelWeb*)data;

	// update state of UI when proxy type changes
	BOOL value = self->mProxyEnabled->get ();
	self->configProxyEnabledUI ( value );
}

void LLPanelWeb::onTrustedSiteListCommit ( LLUICtrl* ctrl, void* data )
{
	LLPanelWeb* self = (LLPanelWeb*)data;

	// something was selected so enable the REMOVE button
	self->mRemTrustedSite->setEnabled ( TRUE );
};

void LLPanelWeb::onTrustedSiteEntryKeystroke ( LLLineEditor* caller, void* data )
{
	LLPanelWeb* self = (LLPanelWeb*)data;

	if ( caller->getLength () )
		self->mAddTrustedSite->setEnabled ( TRUE );
	else
		self->mAddTrustedSite->setEnabled ( FALSE );
}

// add site to list
void LLPanelWeb::onAddTrustedSite ( void* data )
{
	LLPanelWeb* self = (LLPanelWeb*)data;

	if ( self->mTrustedSiteEntry->getLength () )
	{
		// add to list
		self->mTrustedSitesList->addSimpleItem ( self->mTrustedSiteEntry->getText () );

		// remove from entry field
		self->mTrustedSiteEntry->clear ();

		// nothing in entry field so disable add button
		self->mAddTrustedSite->setEnabled ( FALSE );
	};
}

// remove site from list
void LLPanelWeb::onRemTrustedSite ( void* data )
{
	LLPanelWeb* self = (LLPanelWeb*)data;

	self->mTrustedSitesList->deleteSelectedItems ();

	// once we delete something, nothing is selected so disable the REMOVE button
	self->mRemTrustedSite->setEnabled ( FALSE );
}

// save off the list of trusted sites
void LLPanelWeb::loadTrustedSiteList ()
{
	// can't access white list file when not logged in since
	// the filename is based on the SL username
	if ( ! mLoggedIn )
		return;

	LLUrlWhiteList* theWhiteList = LLUrlWhiteList::getInstance ();

	if ( theWhiteList->load () )
	{
		mTrustedSitesList->clear ();

		LLString each;
		if ( theWhiteList->getFirst ( each ) )
		{
			mTrustedSitesList->addSimpleItem ( each );

			while ( theWhiteList->getNext ( each ) )
			{
				mTrustedSitesList->addSimpleItem ( each );
			};
		};
	};
}

// save off the list of trusted sites
void LLPanelWeb::saveTrustedSiteList ()
{
	// can't access white list file when not logged in since
	// the filename is based on the SL username
	if ( ! mLoggedIn )
		return;

	// erase the white list before we re-add items to it
	LLUrlWhiteList::getInstance ()->clear ();

	// step through each item in the scroll list
	std::vector<LLScrollListItem*> data_list = mTrustedSitesList->getAllData();
	std::vector<LLScrollListItem*>::iterator data_itor;
	for (data_itor = data_list.begin(); data_itor != data_list.end(); ++data_itor)
	{
		LLScrollListItem* each = *data_itor;
		LLUrlWhiteList::getInstance ()->addItem ( each->getColumn ( 0 )->getText (), false );
	}

	LLUrlWhiteList::getInstance ()->save ();
}

// save off the list of trusted sites
void LLPanelWeb::apply()
{

	BOOL use_external_browser = mExternalBrowserCheck->get();
	gSavedSettings.setBOOL("UseExternalBrowser", use_external_browser);

	gSavedSettings.setString ( "BrowserHomePage", mBrowserHomePage->getText() );

	BOOL enabled = mProxyEnabled->get ();
	gSavedSettings.setBOOL ( "BrowserProxyEnabled", enabled );

	gSavedSettings.setString ( "BrowserProxyAddress", mProxyAddress->getText() );

	S32 port;
	std::stringstream codec ( mProxyPort->getText () );
	codec >> port;
	gSavedSettings.setS32 ( "BrowserProxyPort", port );

	S32 socks = mProxySocks45->getSelectedIndex () + 4;
	gSavedSettings.setS32 ( "BrowserProxySocks45", socks );

	gSavedSettings.setString ( "BrowserProxyExclusions", mProxyExclusions->getText() );

	BOOL use_web_pages_on_prims = mWebPagesOnPrimsCheck->get();
	gSavedSettings.setBOOL("UseWebPagesOnPrims", use_web_pages_on_prims);

	// save off the list of trusted sites
	saveTrustedSiteList ();

	// update the media engine with the proxy information
	if ( LLMediaEngine::getInstance() && LLMediaEngine::getInstance()->isAvailable() )
	{
		LLMediaEngine::getInstance()->setNetworkProxy ( enabled,
														mProxyAddress->getText(), port, socks,
														mProxyExclusions->getText() );
	};
}

void LLPanelWeb::cancel()
{
}
