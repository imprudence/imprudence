/** 
 * @file llrand.cpp
 * @brief a few useful math functions.
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

#include "linden_common.h"

#include "llrand.h"
#include "lluuid.h"

/* Put this back if you re-enabled slamFPCW
#if LL_WINDOWS
#include <float.h>
#else
#include <stdlib.h>
#endif
*/

void slamFPCW( void )
{
/*
#if LL_WINDOWS
	// for Intel based CPUs, slam the FP control word directly
    WORD wTemp, wSave;
 
    __asm fstcw wSave
    if (  (wSave & 0x300)            // Not single mode
        ||(0x3f != (wSave & 0x3f))   // Exceptions enabled
        ||(wSave & 0xC00))           // Not round to nearest mode
    {
        __asm
        {
            mov ax, wSave
            and ax, not 300h    ;; single mode
            or  ax, 3fh         ;; disable all exceptions
            and ax, not 0xC00   ;; round to nearest mode
            mov wTemp, ax
            fldcw   wTemp
        }
    }
#endif
*/
}

LLRand	gLindenLabRandomNumber(LLUUID::getRandomSeed());

F32	frand(F32 val)
{
//	return (val * (F32)rand()/(F32)RAND_MAX);
	return gLindenLabRandomNumber.llfrand(val);
}

