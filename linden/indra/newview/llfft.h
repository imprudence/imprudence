/** 
 * @file llfft.h
 * @brief FFT function definitions
 *
 * Copyright (c) 2003-2007, Linden Research, Inc.
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

#ifndef LL_LLFFT_H
#define LL_LLFFT_H

/*
 *  Fast Fourier Transform
 *
 */

//#include <stdio.h>
//#include <math.h>

#include "llmath.h"

#define FORWARD	0
#define INVERSE 1

typedef struct {F32 re; F32 im;} COMPLEX;
typedef struct {F64 re; F64 im;} DPCOMPLEX;


void FFT842(S32 in, S32 n, DPCOMPLEX *b);
void R2TX(S32 nthpo, DPCOMPLEX *c0, DPCOMPLEX *c1);
void R4TX(S32 nthpo, DPCOMPLEX *c0, DPCOMPLEX *c1, DPCOMPLEX *c2, DPCOMPLEX *c3);
void R8TX(S32 nxtlt, S32 nthpo, S32 lengt, DPCOMPLEX *cc0, DPCOMPLEX *cc1, DPCOMPLEX *cc2,
	 DPCOMPLEX *cc3, DPCOMPLEX *cc4, DPCOMPLEX *cc5, DPCOMPLEX *cc6, DPCOMPLEX *cc7);
S32 power_of_2(S32 n);
S32 fastlog2(S32 n);

class LLFFTPlan
{
private:
	S32 mRows;
	S32 mCols;
	S32 mSize;
	DPCOMPLEX* mBuff;
public:
	LLFFTPlan() : mRows(0), mCols(0), mSize(0), mBuff(0) {}
	LLFFTPlan(S32 rows, S32 cols) { init(rows, cols); }
	~LLFFTPlan() { destroy(); }

	void init(S32 rows, S32 cols)
	{
		if(power_of_2(rows) && power_of_2(cols))
		{
			mRows = rows;
			mCols = cols;
			mSize = mRows > mCols ? mRows : mCols;
			mBuff = new DPCOMPLEX[mSize];
		}
	}
	void destroy()
	{
		delete [] mBuff;
		mSize = 0;
	}

	BOOL valid() const { return mBuff != NULL; }
	DPCOMPLEX* buffer() const { return mBuff; }
	S32 rows() const { return mRows; }
	S32 cols() const { return mCols; }
};

S32 fft(const LLFFTPlan& plan, COMPLEX *array, S32 rows, S32 cols, S32 direction);

/* do forward transform.  array must be COMPLEX  */
inline S32 forward_fft(const LLFFTPlan& plan, COMPLEX *array, S32 rows, S32 cols)
{
	return fft(plan, array, rows, cols, FORWARD);
}

/* do inverse transform. array must be COMPLEX  */
inline S32 inverse_fft(const LLFFTPlan& plan, COMPLEX *array, S32 rows, S32 cols)
{
	return fft(plan, array, rows, cols, INVERSE);
}

#endif
