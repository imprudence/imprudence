/** 
 * @file llmediamoviebase.h
 * @brief LLMedia support - intermediate base class, for media types
 * that want movie-style controls (i.e play/pause and volume)
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
#ifndef llmediamoviebase_h
#define llmediamoviebase_h

#include "llmediabase.h"

class LLMediaMovieBase : public LLMediaBase
{
	public:
		LLMediaMovieBase ();
		
		// do the right thing with dtor
		virtual ~LLMediaMovieBase ()
		{
		};

		///////////////////////////////////////////////////////////////////////////////
		//	factory method based on explicit media type
		static LLMediaMovieBase* make ( const MediaType mediaTypeIn, S32 width_pixels, S32 height_pixels );

		///////////////////////////////////////////////////////////////////////////////
		// public interface:

		// transport controls
		virtual BOOL stop ();
		virtual BOOL play ();
		virtual BOOL loop ( S32 howMany );
		virtual BOOL pause ();
		virtual BOOL seek ( F64 time );

		// audio levels
		virtual BOOL setVolume ( F32 volumeIn );
		virtual F32 getVolume ();

		// status
		virtual BOOL isIdle () const;
		virtual BOOL isBuffering () const;
		virtual BOOL isError () const;
		virtual BOOL isLoaded () const;
		virtual BOOL isStopped () const;
		virtual BOOL isPaused () const;
		virtual BOOL isPlaying () const;
		virtual BOOL isLooping () const;
		virtual F64 getTime () const;

		// media data
		virtual void setAutoScaled ( BOOL autoScaledIn );
		virtual F64 getMediaDuration () const;
};


#endif // llmediamoviebase_h
