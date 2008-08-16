/** 
 * @file llmediaengine.cpp
 * @brief Top level media engine - wraps more specific functionality
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

#include "llmediaengine.h"

#include "indra_constants.h"
#include "llstring.h"

// singleton pattern - initialization
LLMediaEngine* LLMediaEngine::sInstance = 0;

//////////////////////////////////////////////////////////////////////////////

//static
void LLMediaEngine::initClass()
{
	llassert(!sInstance);
	sInstance = new LLMediaEngine();
}

//static
void LLMediaEngine::updateClass(F32 volume)
{
	llassert(sInstance);
	sInstance->setVolume(volume);
}

//static
void LLMediaEngine::cleanupClass()
{
	delete sInstance;
	sInstance = NULL;
}

//////////////////////////////////////////////////////////////////////////////
// default ctor 
LLMediaEngine::LLMediaEngine() :
	mAvailable( TRUE ),
	mEnabled( TRUE ),
	mAutoScaled( FALSE ),
	mUrl( "" ),
	mMediaRenderer( 0 ),
	mImageUUID( LLUUID::null ),
	mVolume( 0.0f ),
	mProxyEnabled ( FALSE ),
	mProxyAddress ( "" ),
	mProxyPort ( 3128 ),
	mProxySocks ( 5 ),
	mProxyExlude ( "" )
{
}

//////////////////////////////////////////////////////////////////////////////
// dtor 
LLMediaEngine::~LLMediaEngine()
{
	unload();
	destroyImageRaw();
}

//////////////////////////////////////////////////////////////////////////////
// create/destroy raw image
void LLMediaEngine::createImageRaw()
{
	S32 width = getMediaRenderer()->getMediaWidth();
	S32 height = getMediaRenderer()->getMediaHeight();
	S32 depth = getMediaRenderer()->getMediaDepthBytes();
	if ((width > 0) && (height > 0) && (depth > 0))
	{
		if (mImageRaw.isNull())
		{
			mImageRaw = new LLImageRaw;
		}
		mImageRaw->resize(width, height, depth);
		mImageRaw->clear();
	}
	else
	{
		destroyImageRaw();
	}
}

void LLMediaEngine::destroyImageRaw()
{
	mImageRaw = NULL; // deletes image
}

//////////////////////////////////////////////////////////////////////////////
//	retrieves the single instance of this class - based on singleton pattern
LLMediaEngine* LLMediaEngine::getInstance()
{
	return sInstance;
}

//////////////////////////////////////////////////////////////////////////////
//
LLMediaBase* LLMediaEngine::getMediaRenderer()
{
	return mMediaRenderer;
}


//////////////////////////////////////////////////////////////////////////////
//
BOOL LLMediaEngine::init()
{
	if( ! isAvailable() )
		return FALSE;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
//
BOOL LLMediaEngine::update()
{
	BOOL res = FALSE;
	if( mMediaRenderer )
	{
		S32 result = mMediaRenderer->updateMedia();
		switch(result)
		{
		  case LLMediaBase::updateMediaNeedsSizeChange:
			// Media renderer is requesting a size change.
			handleSizeChangedRequest();
			res = TRUE; // Need to update size of texture
			break;
			
		  case LLMediaBase::updateMediaNeedsUpdate:
			res = TRUE;
			break;
			
		  case LLMediaBase::updateMediaNoChanges:
		  default:
			res = FALSE;
			break;
		}
	}
	return res;
}


//////////////////////////////////////////////////////////////////////////////
//
BOOL LLMediaEngine::load ( const LLString& urlIn, bool web_url, const LLString& path, S32 width_pixels, S32 height_pixels )
{
	if( ! isAvailable() )
		return FALSE;

	if( isLoaded() )
		return TRUE;

	this->unload();

	mMediaRenderer = LLMediaBase::make( LLMediaBase::QuickTime, width_pixels, height_pixels);

	if( ! mMediaRenderer )
		return FALSE;

	if( ! mMediaRenderer->init() )
	{
		delete mMediaRenderer;
		mMediaRenderer = 0;
		return FALSE;
	}

	// do this here since there is no media renderer when we get the update so we store and use here
	if( mMediaRenderer )
		mMediaRenderer->setAutoScaled( mAutoScaled );

	if( ! mMediaRenderer->load( urlIn ) )
	{
		delete mMediaRenderer;
		mMediaRenderer = 0;
		return FALSE;
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
//
BOOL LLMediaEngine::isLoaded()
{
	if( ! isAvailable() )
		return FALSE;

	if( mMediaRenderer )
		return mMediaRenderer->isLoaded();
	else
		return FALSE;
}

//////////////////////////////////////////////////////////////////////////////
//
BOOL LLMediaEngine::unload()
{
	if( ! isAvailable() )
		return FALSE;

	if( mMediaRenderer )
	{
		mMediaRenderer->stop();
		mMediaRenderer->unload();
		delete mMediaRenderer;
		mMediaRenderer = 0;
		// Don't do this here.  load() calls unload(), and things get lost.
//		mUrl.clear();
//		mImageUUID  = LLUUID::null;
		return TRUE;
	};

	return FALSE;
};

//////////////////////////////////////////////////////////////////////////////
//
BOOL LLMediaEngine::play()
{
	if( ! isAvailable() )
		return FALSE;

	// base movie volume on slider in prefs (currently prefs also sets volume directly but other controls
	// may eventually control volume and updat ethis variable
	this->setVolume( mVolume );

	if( mMediaRenderer )
		if( ! mMediaRenderer->play() )
			return FALSE;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
//
BOOL LLMediaEngine::loop()
{
	if( ! isAvailable() )
		return FALSE;

	// base movie volume on slider in prefs (currently prefs also sets volume directly but other controls
	// may eventually control volume and updat ethis variable
	this->setVolume( mVolume );

	if( mMediaRenderer )
		if( ! mMediaRenderer->loop( 0 ) )
			return FALSE;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
//
BOOL LLMediaEngine::pause()
{
	if( ! isAvailable() )
		return FALSE;

	if( mMediaRenderer )
		if( ! mMediaRenderer->pause() )
			return FALSE;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
//
BOOL LLMediaEngine::stop()
{
	if( ! isAvailable() )
		return FALSE;

	if( mMediaRenderer )
		if( ! mMediaRenderer->stop() )
			return FALSE;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
//
BOOL LLMediaEngine::seek (F64 time)
{
	if( ! isAvailable() )
		return FALSE;

	if( mMediaRenderer )
		if( ! mMediaRenderer->seek (time) )
			return FALSE;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
//
void  LLMediaEngine::setAvailable( BOOL availableIn )
{
	mAvailable = availableIn;
}

//////////////////////////////////////////////////////////////////////////////
//
BOOL LLMediaEngine::isAvailable()
{
	return mAvailable;
}

//////////////////////////////////////////////////////////////////////////////
//
BOOL LLMediaEngine::setVolume( F32 volumeIn )
{
	if( ! isAvailable() )
		return FALSE;

	mVolume = volumeIn;
	
	if( mMediaRenderer )
	{
		if( ! mMediaRenderer->setVolume( volumeIn ) )
		{
			return FALSE;
		};
	};

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
//
void LLMediaEngine::setEnabled( BOOL enabledIn )
{
	if( mAvailable )
		mEnabled = enabledIn;
}

//////////////////////////////////////////////////////////////////////////////
//
BOOL LLMediaEngine::isEnabled()
{
	if( mAvailable )
		return mEnabled;
	else
		return FALSE;
}

//////////////////////////////////////////////////////////////////////////////
//
void LLMediaEngine::setAutoScaled( BOOL autoScaledIn )
{
	mAutoScaled = autoScaledIn;
}

//////////////////////////////////////////////////////////////////////////////
//
BOOL LLMediaEngine::isAutoScaled()
{
	return mAutoScaled;
}

//////////////////////////////////////////////////////////////////////////////
//
void LLMediaEngine::setUrl( const LLString& urlIn )
{
	mUrl = urlIn;
};

//////////////////////////////////////////////////////////////////////////////
//
const LLString& LLMediaEngine::getUrl ()
{
	return mUrl;
};

//////////////////////////////////////////////////////////////////////////////
//
void LLMediaEngine::setImageUUID( LLUUID imageUUIDIn )
{
	mImageUUID = imageUUIDIn;
};

//////////////////////////////////////////////////////////////////////////////
//
LLUUID LLMediaEngine::getImageUUID()
{
	return mImageUUID;
};

//////////////////////////////////////////////////////////////////////////////
//
void LLMediaEngine::handleSizeChangedRequest()
{
	if( ! isAvailable() )
		return;

	// create / resize Raw image
	LLMediaEngine::getInstance()->createImageRaw();

	// tell media library to use this buffer instead of it's own
	if (mImageRaw.notNull())
	{
		LLMediaEngine::getInstance()->getMediaRenderer()->setBuffer( mImageRaw->getData() );
	}

}

//////////////////////////////////////////////////////////////////////////////////////////
// static
void LLMediaEngine::convertImageAndLoadUrl( bool enableLooping, bool web_url, const std::string& path)
{
	LLMediaEngine* engine = LLMediaEngine::getInstance();
	LLString url = engine->getUrl();
	S32 width_pixels = 512;
	S32 height_pixels = 256;
	if (web_url)
	{
		width_pixels = 512;
		height_pixels = 512;
	}

	bool success = false;
	if (engine->load( url, web_url, path, width_pixels, height_pixels ) )
	{
		// create / resize Raw image
		engine->createImageRaw();
		
		// tell media library to use this buffer instead of it's own
		if (engine->getImageRaw())
		{
			engine->getMediaRenderer()->setBuffer( engine->mImageRaw->getData() );
			engine->getMediaRenderer()->setBufferSize(width_pixels, height_pixels);
		
			// start it playing or looping
			if( enableLooping )
			{
				engine->loop();
			}
			else
			{
				engine->play();
			}
			success = true;
		}
	}

	if (!success)
	{
		llinfos << "MEDIA> unable to load " << LLMediaEngine::getInstance()->getUrl() << llendl;
		//LLMediaEngine::getInstance()->setAvailable( FALSE );
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
// static
void LLMediaEngine::process_parcel_media( LLMessageSystem *msg, void ** )
{
	// extract the agent id
	//	LLUUID agent_id;
	//	msg->getUUID( agent_id );

	U32 flags;
	U32 command;
	F32 time;
	msg->getU32( "CommandBlock", "Flags", flags );
	msg->getU32( "CommandBlock", "Command", command);
	msg->getF32( "CommandBlock", "Time", time );

	if (flags &( (1<<PARCEL_MEDIA_COMMAND_STOP) 
				| (1<<PARCEL_MEDIA_COMMAND_PAUSE) 
				| (1<<PARCEL_MEDIA_COMMAND_PLAY) 
				| (1<<PARCEL_MEDIA_COMMAND_LOOP) 
				| (1<<PARCEL_MEDIA_COMMAND_UNLOAD) ))
	{
		// stop
		if( command == PARCEL_MEDIA_COMMAND_STOP )
		{
			//llinfos << ">>> LLMediaEngine::process_parcel_media with command = " <<( '0' + command ) << llendl;

			LLMediaEngine::getInstance()->stop();
		}
		else
		// pause
		if( command == PARCEL_MEDIA_COMMAND_PAUSE )
		{
			//llinfos << ">>> LLMediaEngine::process_parcel_media with command = " <<( '0' + command ) << llendl;

			LLMediaEngine::getInstance()->pause();
		}
		else
		// play
		if( command == PARCEL_MEDIA_COMMAND_PLAY )
		{
			//llinfos << ">>> LLMediaEngine::process_parcel_media with command = " <<( '0' + command ) << llendl;

			convertImageAndLoadUrl( false, false, std::string() );
		}
		else
		// loop
		if( command == PARCEL_MEDIA_COMMAND_LOOP )
		{
			//llinfos << ">>> LLMediaEngine::process_parcel_media with command = " <<( '0' + command ) << llendl;

			// huh? what is play?
			//convertImageAndLoadUrl( play );
			convertImageAndLoadUrl( true, false, std::string() );
		}
		else
		// unload
		if( command == PARCEL_MEDIA_COMMAND_UNLOAD )
		{
			//llinfos << ">>> LLMediaEngine::process_parcel_media with command = " <<( '0' + command ) << llendl;

			if (LLMediaEngine::getInstance()->isLoaded())
			{
				LLMediaEngine::getInstance()->unload();
				LLMediaEngine::getInstance()->destroyImageRaw();
			}
		}
	}

	if (flags & (1<<PARCEL_MEDIA_COMMAND_TIME))
	{
		// do something about fast forward
		LLMediaEngine::getInstance()->seek(time);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
// static
/*
// ParcelMediaUpdate
// Sends a parcel media update to a single user
// For global updates use the parcel manager.
{
	ParcelMediaUpdate Low NotTrusted Unencoded
	{
		DataBlock Single
		{	MediaURL		Variable	1	}	// string
		{	MediaID			LLUUID			}
		{	MediaAutoScale	U8				}
	}
}
*/
void LLMediaEngine::process_parcel_media_update( LLMessageSystem *msg, void ** )
{
	LLUUID media_uuid;
	char media_url[255];	/* Flawfinder: ignore */
	U8 auto_align;
	msg->getUUID( "DataBlock", "MediaID", media_uuid );
	msg->getString( "DataBlock", "MediaURL", 255, media_url );
	msg->getU8( "DataBlock", "MediaAutoScale", auto_align );

	LLMediaEngine* media_engine = LLMediaEngine::getInstance();
	LLString  url_string ( media_url );

	auto_align = (auto_align) ? TRUE : FALSE;

	if( !( (media_engine->getUrl() == url_string) 
			&& (media_engine->getImageUUID() == media_uuid) 
			&& (media_engine->isAutoScaled() == auto_align ) ) )
	{
		if (media_engine->isLoaded())
		{
			media_engine->unload();
			media_engine->destroyImageRaw();
		}

		media_engine->setUrl(url_string);
		media_engine->setImageUUID(media_uuid);
		media_engine->setAutoScaled(auto_align);
	}
}

// sets proxy information for any of the media impls that may want to use it
void LLMediaEngine::setNetworkProxy ( BOOL enabledIn, const LLString& addressIn, 
										S32 portIn, S32 socksIn, const LLString& excludeIn )
{
	mProxyEnabled = enabledIn;
	mProxyAddress = addressIn;
	mProxyPort = portIn;
	mProxySocks = socksIn;
	mProxyExlude = excludeIn;
}

// gets proxy information for any of the media impls that may want to use it
void LLMediaEngine::getNetworkProxy ( BOOL& enabledOut, LLString& addressOut,
										S32& portOut, S32& socksOut, LLString& excludeOut )
{
	enabledOut = mProxyEnabled;
	addressOut = mProxyAddress;
	portOut = mProxyPort;
	socksOut = mProxySocks;
	excludeOut = mProxyExlude;
}

