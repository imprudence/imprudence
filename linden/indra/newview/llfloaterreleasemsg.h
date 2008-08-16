/** 
 * @file LLFloaterReleaseMsg.h
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

#ifndef LL_LLFLOATERRELMSG_H
#define LL_LLFLOATERRELMSG_H

#include "llwebbrowserctrl.h"
#include "llfloater.h"

class LLFloaterReleaseMsg :
	public LLFloater,
	public LLWebBrowserCtrlObserver
{
	public:
		static LLFloaterReleaseMsg* getInstance();
		virtual ~LLFloaterReleaseMsg();
				
		static void show();
		static void onClickClose( void* data );
		static bool checkVersion(const LLString& version_channel);
		static void displayMessage(const LLString& version_channel);

		static bool sDisplayMessage;
		static LLFloaterReleaseMsg* sInstance;
		LLString mTitleBase;
	private:
		LLFloaterReleaseMsg();
#if LL_LIBXUL_ENABLED
		LLWebBrowserCtrl* mWebBrowser;
#endif // LL_LIBXUL_ENABLED
		LLButton* mCloseButton;
};

#endif
