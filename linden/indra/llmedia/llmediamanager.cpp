/**
 * @file llmediamanager.cpp
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

#include "llmediamanager.h"
#include "llmediaimplfactory.h"

#include "llmediaimplexample1.h"
#include "llmediaimplexample2.h"
#include "llmediaimplquicktime.h"
#include "llmediaimplgstreamer.h"
#if LL_LLMOZLIB_ENABLED
# include "llmediaimplllmozlib.h"
#endif

LLMediaManager* LLMediaManager::sInstance = 0;


////////////////////////////////////////////////////////////////////////////////
// (private)
LLMediaManager::LLMediaManager()
{
}

////////////////////////////////////////////////////////////////////////////////
LLMediaManager::~LLMediaManager()
{
}

////////////////////////////////////////////////////////////////////////////////
// Early initialization for web browser for the viewer, so we can show
// the login screen and defer initialization of QuickTime, etc. JC
// (static)
void LLMediaManager::initBrowser( LLMediaManagerData* init_data )
{
	if ( ! sInstance )
		sInstance = new LLMediaManager();

#if LL_LLMOZLIB_ENABLED
	LLMediaImplLLMozLib::startup( init_data );
#endif // LL_LLMOZLIB_ENABLED
}

////////////////////////////////////////////////////////////////////////////////
// (static)
void LLMediaManager::initClass( LLMediaManagerData* init_data )
{
	if ( ! sInstance )
		sInstance = new LLMediaManager();

	// Initialize impl classes here - this breaks the encapsulation model
	// but some of the initialization takes a long time and we only want to
	// do it once at app startup before any of the impls have been created
	// Each impl provides a static startup method that does any initialization
	// which takes a significant amount of time.
	LLMediaImplExample1::startup( init_data );
	LLMediaImplExample2::startup( init_data );

#if LL_QUICKTIME_ENABLED
	LLMediaImplQuickTime::startup( init_data );
#endif // LL_QUICKTIME_ENABLED

#if LL_GSTREAMER_ENABLED
	LLMediaImplGStreamer::startup( init_data );
#endif // LL_GSTREAMER_ENABLED
}

////////////////////////////////////////////////////////////////////////////////
// (static)
void LLMediaManager::updateClass()
{
	if (!sInstance) return;

	media_impl_container_t::iterator it
		= sInstance->mMediaImplContainer.begin();
	media_impl_container_t::iterator end
		= sInstance->mMediaImplContainer.end();
	for ( ; it != end; ++it )
	{
		LLMediaBase* impl = *it;
		impl->updateMedia();
	}
}

////////////////////////////////////////////////////////////////////////////////
// (static)
void LLMediaManager::cleanupClass()
{
	// Uninitialize impl classes here - this breaks the encapsulation model
	// but some of the uninitialization takes a long time and we only want to
	// do it once at app startup before any of the impls have been created.
	// Each impl provides a static closedown method that does any uninitialization
	// which takes a significant amount of time.
	LLMediaImplExample1::closedown();
	LLMediaImplExample2::closedown();

#if LL_LLMOZLIB_ENABLED
	LLMediaImplLLMozLib::closedown();
#endif // LL_LLMOZLIB_ENABLED

#if LL_QUICKTIME_ENABLED
	LLMediaImplQuickTime::closedown();
#endif // LL_QUICKTIME_ENABLED

#if LL_GSTREAMER_ENABLED
	LLMediaImplGStreamer::closedown();
#endif // LL_QUICKTIME_ENABLED

	if ( sInstance )
		delete sInstance;

	sInstance = 0;
}

////////////////////////////////////////////////////////////////////////////////
// (static)
LLMediaManager* LLMediaManager::getInstance()
{
	return sInstance;
}

////////////////////////////////////////////////////////////////////////////////
// (static)
void LLMediaManager::setBrowserUserAgent(std::string user_agent)
{
#if LL_LLMOZLIB_ENABLED
	// *HACK: Breaks encapsulation model, as initClass does above. JC
	LLMediaImplLLMozLib::setBrowserUserAgent(user_agent);
#endif // LL_LLMOZLIB_ENABLED
}

////////////////////////////////////////////////////////////////////////////////
//
LLMediaBase* LLMediaManager::createSourceFromMimeType( std::string scheme, std::string mime_type )
{

	LLMediaImplMakerBase* impl_maker = LLMediaImplFactory::getInstance()->getImplMaker( scheme, mime_type );

	// If an impl maker is return it means this media type is supported
	if ( impl_maker )
	{
		LLMediaBase* media_impl = impl_maker->create();
		if( media_impl )
		{
			media_impl->setImplMaker( impl_maker );
			std::pair< media_impl_container_t::iterator, bool > result =
				mMediaImplContainer.insert( media_impl );

			if ( result.second )
			{
				media_impl->setMimeType( mime_type );

				media_impl->init();

				return media_impl;
			};
		};
	};

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
//
bool LLMediaManager::destroySource( LLMediaBase* media_impl )
{
	media_impl_container_t::iterator iter =
		mMediaImplContainer.find( media_impl );

	if ( iter != mMediaImplContainer.end() )
	{
		if ( *iter )
		{
			( *iter)->reset();

			delete ( *iter );

			mMediaImplContainer.erase( iter );

			return true;
		};
	};

	return false;
}

////////////////////////////////////////////////////////////////////////////////
//
bool LLMediaManager::addMimeTypeImplNameMap( std::string mime_type, std::string impl_name )
{
	std::pair< mime_type_impl_name_container_t::iterator, bool > result =
		mMimeTypeImplNameContainer.insert( std::make_pair( mime_type, impl_name ) );

	return result.second;
}

////////////////////////////////////////////////////////////////////////////////
//
std::string LLMediaManager::getImplNameFromMimeType( std::string mime_type )
{
	mime_type_impl_name_container_t::iterator iter =
		mMimeTypeImplNameContainer.find( mime_type );

	if ( iter != mMimeTypeImplNameContainer.end() )
	{
		return ( *iter ).second;
	}
	else
	{
		return std::string( "" );
	};
}
////////////////////////////////////////////////////////////////////////////////
//
bool LLMediaManager::supportsMediaType( const std::string& impl_name, const std::string& scheme, const std::string& mime_type )
{
	LLMediaImplMakerBase* impl_maker = LLMediaImplFactory::getInstance()->getImplMaker( impl_name );
	if( impl_maker )
	{
		int idx1 = mime_type.find("/");
		int len = (idx1 == std::string::npos) ? 0 : idx1;
		std::string category = mime_type.substr(0,len);

		return impl_maker->supportsScheme(scheme) ||
				impl_maker->supportsMimeType(mime_type) ||
				impl_maker->supportsMimeTypeCategory(category);
	}
	return false;
}

// static
int LLMediaManager::textureWidthFromMediaWidth( int media_width )
{
	int texture_width = 1;
	while ( texture_width < media_width )
	{
		texture_width <<= 1;
	};
	return texture_width;
}

// static
int LLMediaManager::textureHeightFromMediaHeight( int media_height )
{
	int texture_height = 1;
	while ( texture_height < media_height )
	{
		texture_height <<= 1;
	};
	return texture_height;
}
