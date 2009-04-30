 /** 
 * @file llfloaterhtml.h
 * @author James Cook
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

#ifndef LL_LLFLOATERHTML_H
#define LL_LLFLOATERHTML_H

#include "llfloater.h"
#include "llhtmlhelp.h"

class LLWebBrowserCtrl;

class LLFloaterHtml :
	public LLFloater
{
	public:
		static LLFloaterHtml* getInstance();
		virtual ~LLFloaterHtml();
		
		virtual void draw();
		virtual void onClose( bool app_quitting );
		
		// Pass string like "in-world_help" or "additional help"
		void show( std::string content_id, bool open_link_external, bool trusted_browser );

		// Pass raw URL and window title
		// Can be set to handle secondlife:///app/ URLs, but this should
		// usually be false.
		void show( std::string start_url, std::string title, bool open_link_external, bool trusted_browser );

		std::string getSupportUrl(); 
		
		static void onClickClose( void* data );
		static void onClickBack( void* data );
		static void onClickHome( void* data );
		static void onClickForward( void* data );
		static void onCommitUrlEdit(LLUICtrl* ctrl, void* user_data);
		static void onClickGo( void* data );
		
	private:
		LLFloaterHtml();

		LLWebBrowserCtrl* mWebBrowser;
		static LLFloaterHtml* sInstance;
		LLButton* mCloseButton;
};

#endif
