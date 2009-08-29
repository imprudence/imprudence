/**
 * @file llmediaimplllmozlib.cpp
 * @brief Example 2 of a media impl concrete class
 *
 * $LicenseInfo:firstyear=2007&license=viewergpl$
 * 
 * Copyright (c) 2007-2009, Linden Research, Inc.
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

#ifndef LLMEDIAIMPLLLMOZLIB_H
#define LLMEDIAIMPLLLMOZLIB_H

#include "llmediaimplcommon.h"
#include "llmediaimplfactory.h"

#if LL_LLMOZLIB_ENABLED

#include "llmozlib2.h"

class LLMediaManagerData;

class LLMediaImplLLMozLib :
	public LLMediaImplCommon,
		public LLEmbeddedBrowserWindowObserver
{
	public:
		LLMediaImplLLMozLib();

		static bool startup( LLMediaManagerData* init_data );
		static bool closedown();

		// Update the user-agent string reported when the browser requests
		// web page, because we need to include the Second Life version 
		// and skin name (which can change without restarts).
		// Must be called after startup().
		static bool setBrowserUserAgent(std::string user_agent);

		/* virtual */ bool init();
		/* virtual */ std::string getVersion();
		/* virtual */ bool set404RedirectUrl( std::string redirect_url );
		/* virtual */ bool clr404RedirectUrl();
		/* virtual */ bool setBackgroundColor( unsigned int red, unsigned int green, unsigned int blue ) const;
		/* virtual */ bool setCaretColor( unsigned int red, unsigned int green, unsigned int blue ) const;
		/* virtual */ bool navigateTo( const std::string url );
		/* virtual */ bool updateMedia();
		/* virtual */ unsigned char* getMediaData();
		/* virtual */ int getMediaDataWidth() const;
		/* virtual */ int getMediaDataHeight() const;
		/* virtual */ bool setRequestedMediaSize(int width, int height);
        /* virtual */ bool setAutoScaled( bool auto_scaled );
		/* virtual */ int getTextureFormatPrimary() const;
		/* virtual */ int getTextureFormatType() const;
		/* virtual */ bool mouseDown( int x_pos, int y_pos );
		/* virtual */ bool mouseUp( int x_pos, int y_pos );
		/* virtual */ bool mouseMove( int x_pos, int y_pos );
		/* virtual */ bool keyPress( int key_code );
		/* virtual */ bool scrollByLines( int lines );
		/* virtual */ bool focus( bool focus );
		/* virtual */ bool unicodeInput( unsigned long uni_char );
		/* virtual */ bool mouseLeftDoubleClick( int x_pos, int y_pos );
		/* virtual */ bool navigateForward();
		/* virtual */ bool navigateBack();
		/* virtual */ bool canNavigateForward();
		/* virtual */ bool canNavigateBack();
		/* virtual */ bool enableCookies(bool enable);
		/* virtual */ bool enableProxy(bool enable, std::string proxy_host_name, int proxy_port);
		/* virtual */ bool clearCache();
		/* virtual */ bool clearCookies();
		/* virtual */ bool reset();

		// LLMozLib observerables
		virtual void onNavigateBegin( const EventType& eventIn );
		virtual void onNavigateComplete( const EventType& eventIn );
		virtual void onUpdateProgress( const EventType& eventIn );
		virtual void onPageChanged( const EventType& eventIn );
		virtual void onStatusTextChange( const EventType& eventIn );
		virtual void onLocationChange( const EventType& eventIn );
		virtual void onClickLinkHref( const EventType& eventIn );
		virtual void onClickLinkNoFollow( const EventType& eventIn );

	private:
		bool recomputeSizes();
		int mWindowId;
		int mBrowserWindowWidth;
		int mBrowserWindowHeight;
		int mMediaDataWidth;
		int mMediaDataHeight;
		bool mNeedsUpdate;
		bool updateState();
};

// The maker class
class LLMediaImplLLMozLibMaker : public LLMediaImplMaker
{
	public:
		LLMediaImplLLMozLibMaker();
		LLMediaImplLLMozLib* create()
		{
			return new LLMediaImplLLMozLib();
		}
};
#endif	// LL_LLMOZLIB_ENABLED

#endif	// LLMEDIAIMPLLLMOZLIB_H
