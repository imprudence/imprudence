/** 
 * @file llmediaengine.h
 * @brief Top level media engine - wraps more specific functionality.
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

#ifndef LLMEDIAENGINE_H
#define LLMEDIAENGINE_H

#include "linden_common.h"
#include "lluuid.h"
#include "llimage.h"

#include "llmediabase.h"
#include "llmediaobservers.h"


#include "message.h"

// older: after a 7.3.1 update version is 120684544 (dec) and 7318000 (hex)
// older: current version on my Windows system after a 7.4 update version is 121667584 (dec) and 7408000 (hex)
// current version on my Windows system after a 7.4.1 update version is 121733120 (dec) and 7418000 (hex)

#define LL_MIN_QUICKTIME_VERSION   ( 0x7418000 )	// QT 7.4.1

//////////////////////////////////////////////////////////////////////////////
// media engine singleton
class LLMediaEngine
{
public:
	static void initClass();
	static void updateClass(F32 volume);
	static void cleanupClass();

protected:
	// don't let anyone else make one of these
	LLMediaEngine ();

public:
	virtual ~LLMediaEngine ();

	// used to get access to single instance of the class (singleton pattern)
	static LLMediaEngine* getInstance ();

	// public methods
	BOOL init ();
	BOOL update ();
	
	// Pass web_url true if it's a web page, false if it's a movie.
	// path is to mozilla directory for mozilla
	BOOL load( const LLString& urlIn, bool web_url, const LLString& path, S32 width_pixels, S32 height_pixels);
	
	BOOL isLoaded ();
	BOOL unload ();
	BOOL play ();
	BOOL loop ();
	BOOL pause ();
	BOOL stop ();
	BOOL seek (F64 time);
	void setAvailable ( BOOL availableIn );
	BOOL isAvailable ();
	void setEnabled ( BOOL enabledIn );
	BOOL isEnabled ();
	void setAutoScaled ( BOOL autoScaledIn );
	BOOL isAutoScaled ();
	BOOL setVolume ( F32 volumeIn );
	S32 getQuickTimeVersion();

	void setUrl ( const LLString& urlIn );
	const LLString& getUrl ();
	void setImageUUID ( LLUUID textureIdIn );
	LLUUID getImageUUID ();

	// MBW -- XXX -- This should return a LLMediaMovieBase, but the web and movie media classes still haven't been
	// fully disentangled.
	LLMediaBase* getMediaRenderer();
	LLImageRaw* getImageRaw() { return mImageRaw; }
	void handleSizeChangedRequest();
	
	//////////////////////////////////////////////////////////////////////////////////////////
	//
	static void convertImageAndLoadUrl ( bool enableLooping, bool web_url, const std::string& path);

	//////////////////////////////////////////////////////////////////////////////////////////
	//
	static void process_parcel_media ( LLMessageSystem *msg, void ** );
	static void process_parcel_media_update ( LLMessageSystem *msg, void ** );

	// proxy configuration
	void setNetworkProxy ( BOOL enabledIn, const LLString& addressIn, 
										S32 portIn, S32 socksIn, const LLString& excludeIn );

	void getNetworkProxy ( BOOL& enabledOut, LLString& addressOut,
											S32& portOut, S32& socksOut, LLString& excludeOuy );

private:
	void createImageRaw();
	void destroyImageRaw();

private:
	BOOL mAvailable;
	BOOL mEnabled;
	BOOL mAutoScaled;
	LLString mUrl;
	// MBW -- XXX -- This should be a LLMediaMovieBase, but the web and movie media classes still haven't been
	// fully disentangled.
	LLMediaBase* mMediaRenderer;	
	LLPointer<LLImageRaw> mImageRaw;

	LLUUID mImageUUID;
	F32 mVolume;

	// proxy information
	BOOL mProxyEnabled;
	LLString mProxyAddress;
	S32 mProxyPort;
	S32 mProxySocks;
	LLString mProxyExlude;

private:
	static LLMediaEngine* sInstance;
};

#endif // LLMEDIAENGINE_H

