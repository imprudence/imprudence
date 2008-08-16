/** 
 * @file llmediabase.h
 * @brief LLMedia support - base class
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

// header guard
#ifndef llmediabase_h
#define llmediabase_h

#include "llstring.h"
#include "llmediaemitter.h"
#include "llmediaobservers.h"
#include "llmediaemitterevents.h"

class LLMediaBase
{
	public:
		LLMediaBase ();
		
		// do the right thing with dtor
		virtual ~LLMediaBase ()
		{
		};

		///////////////////////////////////////////////////////////////////////////////
		// public interface:

		///////////////////////////////////////////////////////////////////////////////
		//	different types of supported media
		enum MediaType { Unknown, QuickTime };

		///////////////////////////////////////////////////////////////////////////////
		//	factory method based on explicit media type
		static LLMediaBase* make ( const MediaType mediaTypeIn, S32 width_pixels, S32 height_pixels );
		
		// Result codes for updateMedia
		enum
		{
			updateMediaNoChanges,
			updateMediaNeedsUpdate,
			updateMediaNeedsSizeChange

		} updateMediaResult;

		// housekeeping
		virtual BOOL setBuffer ( U8* bufferIn ) = 0;
		virtual bool setBufferSize(S32 width_pixels, S32 height_pixels) { return false; }
		virtual BOOL init ();
		virtual BOOL load ( const LLString& urlIn );
		virtual BOOL unload ();

		// media data
		virtual S32 updateMedia () = 0;
		virtual U8* getMediaData () = 0;
		virtual S32 getTextureWidth () const;
		virtual S32 getTextureHeight () const;
		virtual S32 getTextureDepth () const;
		virtual S32 getTextureFormatInternal () const;
		virtual S32 getTextureFormatPrimary () const;
		virtual S32 getTextureFormatType () const;
		virtual S32 getTextureFormatSwapBytes () const;
		virtual S32 getMediaWidth () const;
		virtual S32 getMediaHeight () const;
		virtual S32 getMediaDepthBytes () const;
		virtual S32 getMediaBufferSize () const;

		// allow consumers to observe media events
		virtual BOOL addMediaObserver( LLMediaObserver* observerIn );
		virtual BOOL remMediaObserver( LLMediaObserver* observerIn );

		// MBW -- XXX -- These don't belong here!
		// LLMediaEngine should really only deal with LLMediaMovieBase subclasses
		virtual BOOL stop () { return TRUE; }
		virtual BOOL play () { return TRUE; }
		virtual BOOL loop ( S32 howMany )  { return TRUE; }
		virtual BOOL pause ()  { return TRUE; }
		virtual BOOL seek ( F64 time ) { return TRUE; }
		virtual BOOL setVolume ( F32 volumeIn ) { return TRUE; }
		virtual BOOL isLoaded () const { return TRUE; }
		virtual BOOL isPaused () const { return FALSE; }
		virtual BOOL isPlaying () const { return TRUE; }
		virtual BOOL isLooping () const { return FALSE; }
		virtual void setAutoScaled ( BOOL autoScaledIn ) {}
		
	protected:
		// event emitter
		LLMediaEmitter<LLMediaObserver> mMediaEventEmitter;
		
		U32 mBufferChangeCount;  		// Incremented when the buffer changes
		U32 mLastBufferChangeCount;		// Set to mBufferChangeCount when the buffer is reported as changed
		
		S32 mMediaWidth;
		S32 mMediaHeight;
		S32 mMediaDepthBytes;
		S32 mMediaRowbytes;
		S32 mTextureWidth;
		S32 mTextureHeight;
		S32 mTextureDepth;
		S32 mTextureFormatInternal;
		S32 mTextureFormatPrimary;
		S32 mTextureFormatType;
		S32 mTextureFormatSwapBytes;

	public:
	
		// Has memory buffer been updated?  (page content change, scroll, movie frame drawn, etc)
		void bufferChanged() { mBufferChangeCount++; }
		bool getBufferChanged() const { return mBufferChangeCount != mLastBufferChangeCount; }
		void resetBufferChanged() { mLastBufferChangeCount = mBufferChangeCount; }

};


#endif // llmediabase_h
