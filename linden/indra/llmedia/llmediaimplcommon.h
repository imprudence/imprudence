/**
 * @file llmediaimplcommon.h
 * @brief Common impl functionality
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

#ifndef LLMEDIAIMPLCOMMON_H
#define LLMEDIAIMPLCOMMON_H

#include "llmediabase.h"
#include "llmediaemitter.h"
#include "llmediaobserver.h"

#include <string>

class LLMediaImplMakerBase;

class LLMediaImplCommon :
	public LLMediaBase
{
	public:
		LLMediaImplCommon();
		virtual ~LLMediaImplCommon();

		////////////////////////////////////////////////////////////////////////////////
		// begin: default implementation of the abstract interface
		// see llmediabase.h for documentation

		// housekeeping
		virtual bool init();
		virtual bool reset();
		virtual bool setMimeType( const std::string url );
		virtual std::string getMimeType() const;
		virtual std::string getMediaURL() const;
		virtual std::string getVersion();
		virtual bool set404RedirectUrl( std::string redirect_url );
		virtual bool clr404RedirectUrl();
		virtual bool setBackgroundColor( unsigned int red, unsigned int green, unsigned int blue ) const;
		virtual bool setCaretColor( unsigned int red, unsigned int green, unsigned int blue ) const;

		// media management
		virtual bool updateMedia();
		virtual bool setRequestedMediaSize( int width, int height );
		virtual int getMediaWidth() const;
		virtual int getMediaHeight() const;
		virtual int getMediaDepth() const;
		virtual int getMediaBufferSize() const;
		virtual unsigned char* getMediaData();
		virtual int getMediaDataWidth() const;
		virtual int getMediaDataHeight() const;

		// texture management
		virtual int getTextureFormatInternal() const;
		virtual int getTextureFormatPrimary() const;
		virtual int getTextureFormatType() const;

		// audio
		virtual bool setVolume( float volume );
		virtual float getVolume() const;

		// transport control
		virtual bool addCommand( ECommand cmd );
		virtual bool clearCommand();
		virtual bool updateCommand();
		LLMediaBase::ECommand nextCommand();
		virtual LLMediaBase::EStatus getStatus();
		bool setStatus( LLMediaBase::EStatus status );

		virtual bool seek( double time );
		virtual bool setLooping(bool enable) { mLooping = enable; return true; }
		virtual bool isLooping() { return mLooping; }
		virtual bool navigateTo( const std::string url );

		// scaling
		virtual bool setAutoScaled( bool auto_scaled );
		virtual bool isAutoScaled() const;

		// mouse and keyboard interaction
		virtual bool mouseDown( int x_pos, int y_pos );
		virtual bool mouseUp( int x_pos, int y_pos );
		virtual bool mouseMove( int x_pos, int y_pos );
		virtual bool keyPress( int key_code );
        virtual bool scrollByLines( int lines );
		virtual bool focus( bool focus );
		virtual bool unicodeInput( unsigned long uni_char );
		virtual bool mouseLeftDoubleClick( int x_pos, int y_pos );

		// navigation
		virtual bool navigateForward();
		virtual bool navigateBack();
		virtual bool canNavigateForward();
		virtual bool canNavigateBack();

		// caching/cookies
		virtual bool enableCookies( bool enable );
		virtual bool clearCache();
		virtual bool clearCookies();

		virtual bool enableProxy(bool enable, std::string proxy_host_name, int proxy_port);

		// observer interface
		bool addObserver( LLMediaObserver* subject );
		bool remObserver( LLMediaObserver* subject );

		// type registry interface
		void setImplMaker(LLMediaImplMakerBase* impl_maker);
		bool supportsMediaType(std::string scheme, std::string type);

	protected:
		virtual bool setMediaSize( int width, int height );
		virtual bool setMediaDepth( int media_depth );

		LLMediaEmitter< LLMediaObserver > mEventEmitter;

		// Back pointer to the construction object, which is used to discover types handled
		// by the Impl, and meta data associated with the Impl.
		LLMediaImplMakerBase* mImplMaker;
		std::string mMimeType;
		std::string mInitialURL;
		bool mAutoScaled;
		int mMediaWidth;
		int mMediaPrevWidth;
		int mMediaHeight;
		int mMediaPrevHeight;
		int mMediaDepth;
		int mMediaPrevDepth;
		int mMediaRowSpan;
		int mMediaRequestedWidth;
		int mMediaRequestedHeight;
		float mVolume;
		LLMediaBase::ECommand mCommand;
		LLMediaBase::EStatus mStatus;
		bool mLooping;
};

#endif	// LLMEDIAIMPLCOMMON_H
