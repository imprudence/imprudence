/** 
 * @file llviewerjointmesh.cpp
 * @brief LLV4* class header file - vector processor enabled math
 *
 * Copyright (c) 2007-2007, Linden Research, Inc.
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

#ifndef	LL_LLV4MATH_H
#define	LL_LLV4MATH_H

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// LLV4MATH - GNUC
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#if LL_GNUC && __GNUC__ >= 4 && __SSE__

#define			LL_VECTORIZE					1

#if LL_DARWIN

#include <Accelerate/Accelerate.h>
#include <xmmintrin.h>
typedef vFloat	V4F32;

#else

#include <xmmintrin.h>
typedef float	V4F32							__attribute__((vector_size(16)));

#endif

#endif
#if LL_GNUC

#define			LL_LLV4MATH_ALIGN_PREFIX
#define			LL_LLV4MATH_ALIGN_POSTFIX		__attribute__((aligned(16)))

#endif

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// LLV4MATH - MSVC
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#if LL_MSVC && _M_IX86_FP

#define			LL_VECTORIZE					1

#include <xmmintrin.h>

typedef __m128	V4F32;

#endif
#if LL_MSVC

#define			LL_LLV4MATH_ALIGN_PREFIX		__declspec(align(16))
#define			LL_LLV4MATH_ALIGN_POSTFIX

#endif

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// LLV4MATH - default - no vectorization
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#if !LL_VECTORIZE

#define			LL_VECTORIZE					0

struct			V4F32							{ F32 __pad__[4]; };

inline F32 llv4lerp(F32 a, F32 b, F32 w)		{ return ( b - a ) * w + a; }

#endif

#ifndef			LL_LLV4MATH_ALIGN_PREFIX
#	define			LL_LLV4MATH_ALIGN_PREFIX
#endif
#ifndef			LL_LLV4MATH_ALIGN_POSTFIX
#	define			LL_LLV4MATH_ALIGN_POSTFIX
#endif

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// LLV4MATH
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------


#define			LLV4_NUM_AXIS					4

class LLV4Vector3;
class LLV4Matrix3;
class LLV4Matrix4;

#endif
