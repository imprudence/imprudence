/** 
 * @file llpanelweb.h
 * @brief Web browser preferences panel
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

#ifndef LL_LLPANELWEB_H
#define LL_LLPANELWEB_H

#include "llpanel.h"

class LLScrollListCtrl;
class LLCheckBoxCtrl;
class LLLineEditor;
class LLRadioGroup;
//class LLButton;

class LLPanelWeb : public LLPanel
{
public:
	LLPanelWeb();
	virtual ~LLPanelWeb(){};

	virtual BOOL postBuild();

	void apply();
	void cancel();

	void loadTrustedSiteList ();
	void saveTrustedSiteList ();

	void configProxyEnabledUI ( BOOL enabled );
	static void onCommitProxyEnabled ( LLUICtrl* ctrl, void* data );

	void configExternaBrowserEnabledUI ( BOOL enabled );
	static void onCommitExternalBrowser ( LLUICtrl* ctrl, void* data );

	static void onTrustedSiteListCommit ( LLUICtrl* ctrl, void* data );
	static void onTrustedSiteEntryFocusChange ( LLUICtrl* ctrl, void* data );
	static void onTrustedSiteEntryKeystroke ( LLLineEditor* caller, void* data );
	static void onAddTrustedSite ( void* data );
	static void onRemTrustedSite ( void* data );

protected:
	BOOL mLoggedIn;
	LLCheckBoxCtrl* mExternalBrowserCheck;
	LLLineEditor* mBrowserHomePage;
	LLCheckBoxCtrl* mWebPagesOnPrimsCheck;
	LLScrollListCtrl* mTrustedSitesList;
	LLCheckBoxCtrl* mProxyEnabled;
	LLLineEditor* mProxyAddress;
	LLLineEditor* mProxyPort;
	LLRadioGroup* mProxySocks45;
	LLLineEditor* mProxyExclusions;
	LLButton* mAddTrustedSite;
	LLLineEditor* mTrustedSiteEntry;
	LLButton* mRemTrustedSite;
};

#endif
