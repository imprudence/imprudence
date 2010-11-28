/** 
 * @file llwindowmacosx-objc.h
 * @brief Prototypes for functions shared between llwindowmacosx.cpp
 * and llwindowmacosx-objc.mm.
 *
 * $LicenseInfo:firstyear=2006&license=viewergpl$
 * 
 * Copyright (c) 2006-2010, Linden Research, Inc.
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

#include "llimagej2c.h"
#include <Carbon/Carbon.h>

#ifdef __OBJC__
#ifdef BOOL
#undef BOOL
#endif
#endif // __OBJC__

// This will actually hold an NSCursor*, but that type is only available in objective C.
typedef void *CursorRef;

/* Defined in llwindowmacosx-objc.mm: */
void setupCocoa();
CursorRef createImageCursor(const char *fullpath, int hotspotX, int hotspotY);
OSErr releaseImageCursor(CursorRef ref);
OSErr setImageCursor(CursorRef ref);
BOOL decodeImageQuartz(std::string filename, LLImageRaw *raw_image);
BOOL decodeImageQuartz(const UInt8* data, int len, LLImageRaw *raw_image, std::string ext);
