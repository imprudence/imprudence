/**
 * @file llmediaimplquicktime.h
 * @brief QuickTime media impl concrete class
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

#ifndef LLMEDIAIMPLQUICKTIME_H
#define LLMEDIAIMPLQUICKTIME_H

#include "llmediaimplcommon.h"
#include "llmediaimplfactory.h"

#if LL_QUICKTIME_ENABLED

#include <string>

// QuickTime includes
#if defined(__APPLE__)
	#include <QuickTime/QuickTime.h>
#elif defined(WIN32)
	#include "MacTypes.h"
	#include "QTML.h"
	#include "Movies.h"
	#include "QDoffscreen.h"
	#include "FixMath.h"
#endif

class  LLMediaManagerData;

class LLMediaImplQuickTime :
	public LLMediaImplCommon
{
	public:
		LLMediaImplQuickTime();
		virtual ~LLMediaImplQuickTime();

		static bool startup( LLMediaManagerData* init_data );
		static bool closedown();

		/* virtual */ std::string getVersion();
		/* virtual */ bool navigateTo( const std::string url );
		/* virtual */ bool updateMedia();
		/* virtual */ unsigned char* getMediaData();
		/* virtual */ int getMediaDataWidth() const;
		/* virtual */ int getTextureFormatPrimary() const;
		/* virtual */ int getTextureFormatType() const;
		/* virtual */ bool seek( double time );
		/* virtual */ bool setVolume( float volume );

		bool sizeChanged();

	private:
		static OSErr movieDrawingCompleteCallback( Movie call_back_movie, long ref );
		static Boolean mcActionFilterCallBack( MovieController mc, short action, void *params, long ref );
		static void moviePrePrerollCompleteCallback( Movie movie, OSErr preroll_err, void *refcon );

		bool load( const std::string url );
		bool unload();
		void rewind();
		bool processState();
		bool setMovieBoxEnhanced( Rect* rect );

		Movie mMovieHandle;
		GWorldPtr mGWorldHandle;
		ComponentInstance mMovieController;
		const int mMinWidth;
		const int mMaxWidth;
		const int mMinHeight;
		const int mMaxHeight;
		int mCurVolume;
};

// The maker class
class LLMediaImplQuickTimeMaker : public LLMediaImplMaker
{
	public:
		LLMediaImplQuickTimeMaker();
		LLMediaImplQuickTime* create()
		{
			return new LLMediaImplQuickTime();
		}
};

#endif // LL_QUICKTIME_ENABLED

#endif	// LLMEDIAIMPLQUICKTIME_H
