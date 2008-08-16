/** 
 * @file moviemaker.h
 * @brief MovieMaker class definition
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

#ifndef _MOVIEMAKER_H
#define _MOVIEMAKER_H

#if LL_DARWIN
#import <QuickTime/QuickTime.h>
#endif

//
// *TODO: Refactor this implementation such that we provide an abstact
// interface in this file, and provide platform specific
// implementation in the cpp file. This is just crazy to have nearly
// the same class re-defined for different platforms, because then
// usage of the class is platform dependent.
//

#if LL_WINDOWS

#include <vfw.h>

#define TEXT_HEIGHT	20
#define AVIIF_KEYFRAME	0x00000010L // this frame is a key frame.
#define BUFSIZE 260

class MovieMaker {
private:
    //CString FName;
    char fname[64];
    int width;
    int height;

  	AVISTREAMINFO strhdr;
	PAVIFILE pfile;
	PAVISTREAM ps;
	PAVISTREAM psCompressed;
	PAVISTREAM psText;
	AVICOMPRESSOPTIONS opts;
	AVICOMPRESSOPTIONS FAR * aopts[1];
	DWORD dwTextFormat;
	COMPVARS cvars;
	char szText[BUFSIZE];
	int nFrames;
	bool bOK;


public:
    MovieMaker();
    ~MovieMaker();

    inline bool IsOK() const { return bOK; };
    void StartCapture( char *name , int x, int y);
    void EndCapture();
    bool Snap();
};

#elif LL_DARWIN

class MovieMaker {
private:
    char fname[64];		/*Flawfinder: ignore*/

	Movie	movie;
	short	movieResRef;
	Track	track;
	Media	media;
	int		width;
	int		height;
	long	bufferSize;
	long	rowBytes;
	char	*buffer;
	char	*invertedBuffer;

	GWorldPtr	gworld;
	Rect		rect;

	ComponentInstance ci;
	
	ImageDescriptionHandle idh;
	SCTemporalSettings ts;
	SCDataRateSettings ds;

	UnsignedWide	lastFrameTime;
	
	OSStatus setupMovie();
	OSStatus grabFrame();
	OSStatus addFrame();

public:
    MovieMaker();
    ~MovieMaker();

    inline bool IsOK() const { return true; };
    void StartCapture( char *name , int x, int y);
    void EndCapture();
    bool Snap();
};
	
#elif LL_LINUX
// *FIX: we need a working implementation on linux.
class MovieMaker {
public:
    MovieMaker() {}
    ~MovieMaker() {}

    inline bool IsOK() const { return false; };
    void StartCapture( char *name , int x, int y) {}
    void EndCapture() {}
    bool Snap() { return false; }
};

#endif

#endif
