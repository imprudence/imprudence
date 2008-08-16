/** 
 * @file llmediaimplquicktime.h
 * @brief implementation that supports Apple QuickTime media.
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

#if LL_QUICKTIME_ENABLED

// header guard
#ifndef llmediaimplquicktime_h
#define llmediaimplquicktime_h

#include "stdtypes.h"

#include "llmediamoviebase.h"

#if LL_QUICKTIME_ENABLED
#if LL_DARWIN
#include <QuickTime/QuickTime.h>
#else
#include "MacTypes.h"
#include "QTML.h"
#include "Movies.h"
#include "FixMath.h"
#include "QuickTimeStreaming.h"
#endif
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class LLMediaImplQuickTime:
	public LLMediaMovieBase
{
	public:
		LLMediaImplQuickTime ();
		virtual ~LLMediaImplQuickTime ();

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// implementation of the media public interface

		// housekeeping
		virtual BOOL setBuffer ( U8* bufferIn );
		virtual BOOL init ();
		virtual BOOL load ( const LLString& urlIn );
		virtual BOOL unload ();

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
		virtual S32 updateMedia ();
		virtual void setAutoScaled ( BOOL autoScaledIn );
		virtual U8* getMediaData ();
		virtual F64 getMediaDuration () const;

	private:
		// quicktime specific
		Movie theMovie;
		ComponentInstance theController;
		PixMapHandle pixmapHandle;
		GWorldPtr theGWorld;
		Rect movieRect;
		U8* mediaData;
		BOOL movieLoaded;
		BOOL ownBuffer;
		short curVolume;
		S32 loopsLeft;
		
		BOOL autoScaled;
		BOOL sizeChangeInProgress;
		BOOL initialStartDone;
		BOOL isQTLoaded ();
		BOOL isQTPlaythroughOK ();
		void setupDummyBuffer ();

		static OSErr myFrameDrawnCallback ( Movie callbackMovie, long refCon );
		static Boolean myMCActionFilterProc (MovieController theMC, short theAction, void *theParams, long theRefCon);
		
		void rewind();
		void sizeChanged();
		void updateMediaSize();
		
		enum { ModeNone, ModeIdle, ModeError, ModeBuffering, ModeLoaded, ModeStopped, ModePaused, ModePlaying, ModeLooping } currentMode;
};


#endif // llmediaimplquicktime_h

#endif
