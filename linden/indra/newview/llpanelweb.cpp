/** 
 * @file LLPanelWeb.cpp
 * @brief Network preferences panel
 *
 * $LicenseInfo:firstyear=2001&license=viewergpl$
 * 
 * Copyright (c) 2001-2008, Linden Research, Inc.
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

// file include
#include "llpanelweb.h"

// linden library includes
#include "llerror.h"
#include "llrect.h"
#include "llstring.h"

// project includes
#include "llbutton.h"
#include "llcheckboxctrl.h"
#include "lllineeditor.h"
#include "llmozlib.h"
#include "llui.h"
#include "lluictrlfactory.h"
#include "llviewercontrol.h"
#include "llvieweruictrlfactory.h"
#include "llviewerwindow.h"

#if LL_LIBXUL_ENABLED
#include "llmozlib.h"
#endif // LL_LIBXUL_ENABLED

LLPanelWeb::LLPanelWeb()
{
	gUICtrlFactory->buildPanel(this, "panel_preferences_web.xml");
}

BOOL LLPanelWeb::postBuild()
{
	childSetAction( "clear_cache", onClickClearCache, this );
	childSetAction( "clear_cookies", onClickClearCookies, this );
	childSetCommitCallback( "cookies_enabled", onCommitCookies, this );
	childSetCommitCallback( "web_proxy_editor", onCommitWebProxyAddress, this);
	childSetCommitCallback( "web_proxy_port", onCommitWebProxyPort, this);
	
	childSetEnabled("web_proxy_editor", 
			gSavedSettings.getBOOL("BrowserProxyEnabled"));
	childSetEnabled("web_proxy_port", 
			gSavedSettings.getBOOL("BrowserProxyEnabled"));
	childSetEnabled("proxy_text_label", 
			gSavedSettings.getBOOL("BrowserProxyEnabled"));
	childSetCommitCallback("web_proxy_enabled", onCommitWebProxyEnabled, this);

	refresh();

	return TRUE;
}



LLPanelWeb::~LLPanelWeb()
{
	// Children all cleaned up by default view destructor.
}

void LLPanelWeb::apply()
{
}

void LLPanelWeb::refresh()
{
	LLPanel::refresh();

	mCookiesEnabled = gSavedSettings.getBOOL("CookiesEnabled");
	mWebProxyEnabled = gSavedSettings.getBOOL("BrowserProxyEnabled");
	mWebProxyAddress = gSavedSettings.getString("BrowserProxyAddress");
	mWebProxyPort =  gSavedSettings.getS32("BrowserProxyPort");
	
	LLLineEditor* web_proxy_editor = gUICtrlFactory->getLineEditorByName(this, "web_proxy_editor");
	if (web_proxy_editor)
	{
		web_proxy_editor->setText( gSavedSettings.getString("BrowserProxyAddress") );
	}

#if LL_LIBXUL_ENABLED
	llinfos << "setting cookies enabled to " << mCookiesEnabled << llendl;
	LLMozLib::getInstance()->enableCookies( mCookiesEnabled );
#endif // LL_LIBXUL_ENABLED

}

void LLPanelWeb::cancel()
{
#if LL_LIBXUL_ENABLED
	llinfos << "setting cookies enabled to " << mCookiesEnabled << llendl;
	LLMozLib::getInstance()->enableCookies( mCookiesEnabled );
#endif // LL_LIBXUL_ENABLED

	gSavedSettings.setBOOL( "CookiesEnabled", mCookiesEnabled );
	gSavedSettings.setBOOL( "BrowserProxyEnabled", mWebProxyEnabled );
	gSavedSettings.setString( "BrowserProxyAddress", mWebProxyAddress );
	gSavedSettings.setS32( "BrowserProxyPort", mWebProxyPort );

	LLMozLib::getInstance()->enableProxy( mWebProxyEnabled, mWebProxyAddress, mWebProxyPort ); 
}

// static
void LLPanelWeb::onClickClearCache(void*)
{
#if LL_LIBXUL_ENABLED
	gViewerWindow->alertXml("ConfirmClearBrowserCache", callback_clear_browser_cache, 0);
#endif // LL_LIBXUL_ENABLED
}

//static
void LLPanelWeb::callback_clear_browser_cache(S32 option, void* userdata)
{
#if LL_LIBXUL_ENABLED
	if ( option == 0 ) // YES
	{
		llinfos << "clearing browser cache" << llendl;
		LLMozLib::getInstance()->clearCache();
	}
#endif // LL_LIBXUL_ENABLED
}

// static
void LLPanelWeb::onClickClearCookies(void*)
{
#if LL_LIBXUL_ENABLED
	gViewerWindow->alertXml("ConfirmClearCookies", callback_clear_cookies, 0);
#endif // LL_LIBXUL_ENABLED
}

//static
void LLPanelWeb::callback_clear_cookies(S32 option, void* userdata)
{
#if LL_LIBXUL_ENABLED
	if ( option == 0 ) // YES
	{
		llinfos << "clearing browser cookies" << llendl;
		LLMozLib::getInstance()->clearAllCookies();
	}
#endif // LL_LIBXUL_ENABLED
}

// static
void LLPanelWeb::onCommitCookies(LLUICtrl* ctrl, void* data)
{
  LLPanelWeb* self = (LLPanelWeb*)data;
  LLCheckBoxCtrl* check = (LLCheckBoxCtrl*)ctrl;

  if (!self || !check) return;

#if LL_LIBXUL_ENABLED
	llinfos << "setting cookies enabled to " << check->get() << llendl;
	LLMozLib::getInstance()->enableCookies( check->get() );
#endif // LL_LIBXUL_ENABLED
  
}
// static
void LLPanelWeb::onCommitWebProxyEnabled(LLUICtrl* ctrl, void* data)
{
  LLPanelWeb* self = (LLPanelWeb*)data;
  LLCheckBoxCtrl* check = (LLCheckBoxCtrl*)ctrl;

  if (!self || !check) return;
  self->childSetEnabled("web_proxy_editor", 
			check->get());
  self->childSetEnabled("web_proxy_port", 
			check->get());
  self->childSetEnabled("proxy_text_label", 
			check->get());
	
  LLMozLib::getInstance()->enableProxy( gSavedSettings.getBOOL("BrowserProxyEnabled"), 
										  gSavedSettings.getString("BrowserProxyAddress"), 
										  gSavedSettings.getS32("BrowserProxyPort") ); 
}

void LLPanelWeb::onCommitWebProxyAddress(LLUICtrl *ctrl, void *userdata)
{
  LLLineEditor* web_proxy = (LLLineEditor*)ctrl;

  if (web_proxy)
  {
	  gSavedSettings.setString("BrowserProxyAddress", web_proxy->getText());
  }	
  
  LLMozLib::getInstance()->enableProxy( gSavedSettings.getBOOL("BrowserProxyEnabled"), 
										  gSavedSettings.getString("BrowserProxyAddress"), 
										  gSavedSettings.getS32("BrowserProxyPort") ); 
}

void LLPanelWeb::onCommitWebProxyPort(LLUICtrl *ctrl, void *userdata)
{
	LLMozLib::getInstance()->enableProxy( gSavedSettings.getBOOL("BrowserProxyEnabled"), 
										  gSavedSettings.getString("BrowserProxyAddress"), 
										  gSavedSettings.getS32("BrowserProxyPort") ); 

}
