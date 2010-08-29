/**
 * @file llmediamanager.h
 * @brief Manages instances of media impls
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

#ifndef LLMEDIAMANAGER_H
#define LLMEDIAMANAGER_H

#include <map>
#include <set>

#include "llmediaimplcommon.h"

////////////////////////////////////////////////////////////////////////////////
//
class LLMediaManagerData
{
	public:
		LLMediaManagerData() :
			mBrowserParentWindow( 0 ),
			mBrowserProfileDir( "" ),
			mBrowserProfileName ( "" )
		{ };

		void setBrowserApplicationDir( const std::string& browser_application_dir ) { mBrowserApplicationDir = browser_application_dir; };
		std::string& getBrowserApplicationDir() { return mBrowserApplicationDir; };

		void setBrowserComponentDir( const std::string& browser_component_dir ) { mBrowserComponentDir = browser_component_dir; };
		std::string& getBrowserComponentDir() { return mBrowserComponentDir; };

		void setBrowserParentWindow( void* browser_parent_window ) { mBrowserParentWindow = browser_parent_window; };
		void* getBrowserParentWindow() { return mBrowserParentWindow; };

		void setBrowserProfileDir( const std::string& browser_profile_dir ) { mBrowserProfileDir = browser_profile_dir; };
		std::string& getBrowserProfileDir() { return mBrowserProfileDir; };

		void setBrowserProfileName( const std::string& browser_profile_name ) { mBrowserProfileName = browser_profile_name; };
		std::string& getBrowserProfileName() { return mBrowserProfileName; };

	private:
		void* mBrowserParentWindow;
		std::string mBrowserProfileDir;
		std::string mBrowserProfileName;
		std::string mBrowserApplicationDir;
		std::string mBrowserComponentDir;
};

////////////////////////////////////////////////////////////////////////////////
//
class LLMediaManager
{
	public:
		virtual ~LLMediaManager();

		// Special case early init for just web browser component
		// so we can show login screen.  See .cpp file for details. JC
		static void initBrowser( LLMediaManagerData* init_data );

		static void initClass( LLMediaManagerData* init_data );
		static void cleanupClass();
		static LLMediaManager* getInstance();

		// We append the skin name to the browser user agent string, so
		// we need to change it while the app is running, not just at
		// init time.
		// Must be called after initClass() above.
		// *HACK: Breaks encapsulation model. JC
		static void setBrowserUserAgent(std::string user_agent);

		// Calls update on all media sources
		static void updateClass();

		// Given an URL and mime_type, construct/destroy a playback engine for
		// it (a "media impl").
		LLMediaBase* createSourceFromMimeType( std::string scheme, std::string mime_type );
		bool destroySource( LLMediaBase* media_impl );

		// mime type to impl mapping functions
		bool addMimeTypeImplNameMap( std::string mime_type, std::string impl_name );
		std::string getImplNameFromMimeType( std::string mime_type );

		// Name accessor for querying type support
		bool supportsMediaType( const std::string& impl_name, const std::string& scheme, const std::string& mime_type );

		// convenience functions for getting suggested texture sizes to hold various size media
		static int textureWidthFromMediaWidth( int media_width );
		static int textureHeightFromMediaHeight( int media_height );

	private:
		LLMediaManager();
		static LLMediaManager* sInstance;

		typedef std::set< LLMediaBase* > media_impl_container_t;
		media_impl_container_t mMediaImplContainer;

		typedef std::map< std::string, std::string > mime_type_impl_name_container_t;
		mime_type_impl_name_container_t mMimeTypeImplNameContainer;
};

#endif // LLMEDIAMANAGER_H
