/** 
 * @file linden_common.h
 * @brief Includes common headers that are always safe to include
 *
 * Copyright (c) 2001-2007, Linden Research, Inc.
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

#ifndef LL_LINDEN_COMMON_H
#define LL_LINDEN_COMMON_H

#include "llpreprocessor.h"

#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// Work around stupid Microsoft STL warning
#ifdef LL_WINDOWS
#pragma warning (disable : 4702) // warning C4702: unreachable code
#endif	//	LL_WINDOWS

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "llfile.h"

#include "stdtypes.h"
#include "lldefs.h"
#include "llerror.h"
#include "llformat.h"
#include "llstring.h"
#include "lltimer.h"
#include "llfasttimer.h"
#include "llsys.h"

#ifdef LL_WINDOWS
#pragma warning (3 : 4702) // we like level 3, not 4
#endif	//	LL_WINDOWS

#endif
