/** 
 * @file llrand.h
 * @brief Some useful math functions.
 *
 * Copyright (c) 2000-2007, Linden Research, Inc.
 * 
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

#ifndef LL_LLRAND_H
#define LL_LLRAND_H

// As long as you #include "llviewerprecompiledheaders.h", 
// you can use "gLindenLabRandomNumber.llfrand( range );" which returns a
// random number F32 ranging from 0.0f to range. 
// -Ventrella - Sept 30, 2005

// Slams Intel processors into Single Precision FP mode
// (which is not any faster on modern hardware)
void slamFPCW( void );

class LLRand
{
public:
	LLRand(U32 seed) : mSeed(seed) {}
	~LLRand() {}

	void seed(U32 seed) { mSeed = seed; }

	U32  llrand()	
	{ 
		mSeed = U64L(1664525) * mSeed + U64L(1013904223); 
		return (U32)mSeed; 
	}

	U32  llrand(U32 val)	
	{ 
		mSeed = U64L(1664525) * mSeed + U64L(1013904223); 
		return (U32)(mSeed) % val; 
	}

	// val is the maximum
	F32  llfrand(F32 val)
	{
		const U32 FP_ONE  = 0x3f800000;
		const U32 FP_MASK = 0x007fffff;
		U32 ir = llrand();

		ir = FP_ONE | (FP_MASK & ir);
		
		// generate random float
		F32	fr = (*(F32 *)&ir);

		// correct to [0..1)
		fr -= 1.f;

		fr *= val;

		return fr;
	}
	
public:
	U64 mSeed;
};

F32	frand(F32 val);

extern LLRand	gLindenLabRandomNumber;

#endif
