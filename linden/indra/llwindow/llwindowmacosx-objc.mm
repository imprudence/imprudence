/** 
 * @file llwindowmacosx-objc.mm
 * @brief Definition of functions shared between llwindowmacosx.cpp
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

#include <AppKit/AppKit.h>
#include <Accelerate/Accelerate.h>
#include <Quartz/Quartz.h>

/*
 * These functions are broken out into a separate file because the
 * objective-C typedef for 'BOOL' conflicts with the one in
 * llcommon/stdtypes.h.  This makes it impossible to use the standard
 * linden headers with any objective-C++ source.
 */

#include "llwindowmacosx-objc.h"
#include "lldir.h"

BOOL decodeImageQuartz(const UInt8* data, int len, LLImageRaw *raw_image, std::string ext)
{
	CFDataRef theData = CFDataCreate(kCFAllocatorDefault, data, len);

	CGImageSourceRef srcRef = CGImageSourceCreateWithData(theData, NULL);
	CGImageRef image_ref = CGImageSourceCreateImageAtIndex(srcRef, 0, NULL);
	CFRelease(srcRef);

	size_t width = CGImageGetWidth(image_ref);
	size_t height = CGImageGetHeight(image_ref);
	size_t comps = CGImageGetBitsPerPixel(image_ref) / 8;
	size_t bytes_per_row = CGImageGetBytesPerRow(image_ref);
	CFDataRef result = CGDataProviderCopyData(CGImageGetDataProvider(image_ref));
	UInt8* bitmap = (UInt8*)CFDataGetBytePtr(result);

	CGImageAlphaInfo format = CGImageGetAlphaInfo(image_ref);
	if (comps == 4)
	{
		vImage_Buffer vb;
		vb.data = bitmap;
		vb.height = height;
		vb.width = width;
		vb.rowBytes = bytes_per_row;

		if (format & kCGImageAlphaPremultipliedFirst)
		{
			// Ele: Skip unpremultiplication for PSD, PNG and TGA files
			if (ext != std::string("psd") && ext != std::string("tga") && ext != std::string("png"))
				vImageUnpremultiplyData_ARGB8888(&vb, &vb, 0);
		}
		else if (format & kCGImageAlphaPremultipliedLast)
		{
			// Ele: Photoshop Native Transparency needs unmultiplication
			vImageUnpremultiplyData_RGBA8888(&vb, &vb, 0);
		}
	}

	raw_image->resize(width, height, comps);
	memcpy(raw_image->getData(), bitmap, height * bytes_per_row);
	raw_image->verticalFlip();

	CFRelease(theData);
	CGImageRelease(image_ref);
	CFRelease(result);

	return TRUE;
}

BOOL decodeImageQuartz(std::string filename, LLImageRaw *raw_image)
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	NSURL *url = [[NSURL alloc] initFileURLWithPath:[NSString stringWithCString:filename.c_str()]];
	NSData *data = [NSData dataWithContentsOfURL:url];

	std::string ext = gDirUtilp->getExtension(filename);

	BOOL result = decodeImageQuartz((UInt8*)[data bytes], [data length], raw_image, ext);
	[pool release];
	return result;
}

void setupCocoa()
{
	static bool inited = false;
	
	if(!inited)
	{
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
		
		// This is a bit of voodoo taken from the Apple sample code "CarbonCocoa_PictureCursor":
		//   http://developer.apple.com/samplecode/CarbonCocoa_PictureCursor/index.html
		
		//	Needed for Carbon based applications which call into Cocoa
		NSApplicationLoad();

		//	Must first call [[[NSWindow alloc] init] release] to get the NSWindow machinery set up so that NSCursor can use a window to cache the cursor image
		[[[NSWindow alloc] init] release];

		[pool release];
	}
}

CursorRef createImageCursor(const char *fullpath, int hotspotX, int hotspotY)
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	// extra retain on the NSCursor since we want it to live for the lifetime of the app.
	NSCursor *cursor =
		[[[NSCursor alloc] 
				initWithImage:
					[[[NSImage alloc] initWithContentsOfFile:
						[NSString stringWithFormat:@"%s", fullpath]
					]autorelease] 
				hotSpot:NSMakePoint(hotspotX, hotspotY)
		]retain];	
		
	[pool release];
	
	return (CursorRef)cursor;
}

// This is currently unused, since we want all our cursors to persist for the life of the app, but I've included it for completeness.
OSErr releaseImageCursor(CursorRef ref)
{
	if( ref != NULL )
	{
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
		NSCursor *cursor = (NSCursor*)ref;
		[cursor release];
		[pool release];
	}
	else
	{
		return paramErr;
	}
	
	return noErr;
}

OSErr setImageCursor(CursorRef ref)
{
	if( ref != NULL )
	{
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
		NSCursor *cursor = (NSCursor*)ref;
		[cursor set];
		[pool release];
	}
	else
	{
		return paramErr;
	}
	
	return noErr;
}
