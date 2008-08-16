/** 
 * @file moviemaker.cpp
 * @brief MovieMaker implementation
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

//  ===============================================
//  MovieMaker.cpp
//  ===============================================

#include "llviewerprecompiledheaders.h"

#include "moviemaker.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#if LL_WINDOWS

#include <windowsx.h>

HANDLE  MakeDib( HBITMAP hbitmap, UINT bits );
HBITMAP LoadBMPFromFB( int w, int h );

/*
    ===============================================
        Constructors, Destructor
    ===============================================
*/


MovieMaker::MovieMaker()
{
    snprintf( fname, sizeof(fname), "movie.avi" );		/* Flawfinder: ignore */
    width  = -1;
    height = -1;

    bOK = true;
    nFrames = 0;

  	pfile = NULL;
	ps = NULL;
	psCompressed = NULL;
	psText = NULL;
	aopts[0] = &opts;

    // Check VFW version.
	WORD wVer = HIWORD( VideoForWindowsVersion() );
	if ( wVer < 0x010A )
        {
        fprintf( stderr, "VFW version is too old.\n" );
        exit( -1 );
	    }
	else
	    {
		AVIFileInit();
	    }
}


MovieMaker::~MovieMaker()
{
	if (ps)
		AVIStreamClose(ps);

	if (psCompressed)
		AVIStreamClose(psCompressed);

	if (psText)
		AVIStreamClose(psText);

	if (pfile)
        {
		AVIFileClose(pfile);
        }

	WORD wVer = HIWORD(VideoForWindowsVersion());
	if (wVer >= 0x010A)
	    {
		AVIFileExit();
	    }
}

void MovieMaker::StartCapture( char *name , int x, int y)
{
    strncpy( fname, name, sizeof(fname) -1 );		/* Flawfinder: ignore */
    fname[sizeof(fname) -1] = '\0';

    // Get the width and height.
    width = x;
    height = y;

    fprintf( stderr, "Starting %d x %d capture to file: %s\n", width, height, fname );

	bOK = TRUE;

	nFrames = 0;

}

void MovieMaker::EndCapture()
{
    fprintf( stderr, "\n" );
	if (ps)
        {
		AVIStreamClose(ps);
        ps = NULL;
        }

	if (psCompressed)
        {
		AVIStreamClose(psCompressed);
        psCompressed = NULL;
        }

	if (psText)
        {
		AVIStreamClose(psText);
        psText = NULL;
        }

	if (pfile)
        {
		AVIFileClose(pfile);
        pfile = NULL;
        }

	WORD wVer = HIWORD(VideoForWindowsVersion());
	if (wVer >= 0x010A)
	    {
		AVIFileExit();
	    }

}

bool MovieMaker::Snap()
{
	HRESULT hr;

	if (!bOK)
		return false;

    // Get an image and stuff it into a bitmap.
    HBITMAP bmp;
    bmp = LoadBMPFromFB( width, height );

	LPBITMAPINFOHEADER alpbi = (LPBITMAPINFOHEADER)GlobalLock(MakeDib(bmp, 32));
    DeleteObject( bmp );

	if (alpbi == NULL)
        {
        bOK = false;
		return false;
        }
	if (width>=0 && width != alpbi->biWidth)
	{
		GlobalFreePtr(alpbi);
        bOK = false;
		return false;
	}
	if (height>=0 && height != alpbi->biHeight)
	{
		GlobalFreePtr(alpbi);
        bOK = false;
		return false;
	}
	width = alpbi->biWidth;
	height = alpbi->biHeight;
	if (nFrames == 0)
	{
		hr = AVIFileOpenA(&pfile,		    // returned file pointer
			       fname,							// file name
				   OF_WRITE | OF_CREATE,		    // mode to open file with
				   NULL);							// use handler determined
													// from file extension....
		if (hr != AVIERR_OK)
		{
			GlobalFreePtr(alpbi);
			bOK = false;
			return false;
		}
		_fmemset(&strhdr, 0, sizeof(strhdr));
		strhdr.fccType                = streamtypeVIDEO;// stream type
		strhdr.fccHandler             = 0;
		strhdr.dwScale                = 1;
		strhdr.dwRate                 = 15;
		strhdr.dwSuggestedBufferSize  = alpbi->biSizeImage;
		SetRect(&strhdr.rcFrame, 0, 0,		    // rectangle for stream
			(int) alpbi->biWidth,
			(int) alpbi->biHeight);

		// And create the stream;
		hr = AVIFileCreateStream(pfile,		    // file pointer
						         &ps,		    // returned stream pointer
								 &strhdr);	    // stream header
		if (hr != AVIERR_OK)
		{
			GlobalFreePtr(alpbi);
			bOK = false;
			return false;
		}

		_fmemset(&opts, 0, sizeof(opts));

		if (!AVISaveOptions(NULL, ICMF_CHOOSE_KEYFRAME, 1, &ps, (LPAVICOMPRESSOPTIONS FAR *) &aopts))
		{
            fprintf( stderr, "AVISaveOptions failed.\n" );
			GlobalFreePtr(alpbi);
			bOK = false;
			return false;
		}

		hr = AVIMakeCompressedStream(&psCompressed, ps, &opts, NULL);
		if (hr != AVIERR_OK)
		{
            fprintf( stderr, "AVIMakeCompressedStream failed.\n" );
			GlobalFreePtr(alpbi);
			bOK = false;
			return false;
		}

		hr = AVIStreamSetFormat(psCompressed, 0,
					   alpbi,	    // stream format
				       alpbi->biSize +   // format size
				       alpbi->biClrUsed * sizeof(RGBQUAD));
		if (hr != AVIERR_OK)
		{
            fprintf( stderr, "AVIStreamSetFormat failed.\n" );
			GlobalFreePtr(alpbi);
			bOK = false;
			return false;
		}

		// Fill in the stream header for the text stream....

		// The text stream is in 60ths of a second....
/*
		_fmemset(&strhdr, 0, sizeof(strhdr));
		strhdr.fccType                = streamtypeTEXT;
		strhdr.fccHandler             = mmioFOURCC('D', 'R', 'A', 'W');
		strhdr.dwScale                = 1;
		strhdr.dwRate                 = 60;
		strhdr.dwSuggestedBufferSize  = sizeof(szText);
		SetRect(&strhdr.rcFrame, 0, (int) alpbi->biHeight,
			(int) alpbi->biWidth, (int) alpbi->biHeight + TEXT_HEIGHT);

		// ....and create the stream.
		hr = AVIFileCreateStream(pfile, &psText, &strhdr);
		if (hr != AVIERR_OK)
		{
			GlobalFreePtr(alpbi);
			bOK = false;
			return false;
		}

		dwTextFormat = sizeof(dwTextFormat);
		hr = AVIStreamSetFormat(psText, 0, &dwTextFormat, sizeof(dwTextFormat));
		if (hr != AVIERR_OK)
		{
			GlobalFreePtr(alpbi);
			bOK = false;
			return false;
		}
*/
	}

	// Now actual writing
	hr = AVIStreamWrite(psCompressed,	// stream pointer
		nFrames * 1, // 10,				// time of this frame
		1,				// number to write
		(LPBYTE) alpbi +		// pointer to data
			alpbi->biSize +
			alpbi->biClrUsed * sizeof(RGBQUAD),
			alpbi->biSizeImage,	// size of this frame
		AVIIF_KEYFRAME,			 // flags....
		NULL,
		NULL);
	if (hr != AVIERR_OK)
	{
        fprintf( stderr, "AVIStreamWrite failed.\n" );
		GlobalFreePtr(alpbi);
		bOK = false;
		return false;
	}

	// Make some text to put in the file ...
	//LoadString(hInstance, IDS_TEXTFORMAT, szMessage, BUFSIZE );
	/*
	strcpy(szMessage, "This is frame #%d");
		
	int iLen = wsprintf(szText, szMessage, (int)(nFrames + 1));

	// ... and write it as well.
	hr = AVIStreamWrite(psText,
			nFrames * 40,
			1,
			szText,
			iLen + 1,
			AVIIF_KEYFRAME,
			NULL,
			NULL);
	if (hr != AVIERR_OK)
	{
		GlobalFreePtr(alpbi);
		bOK = false;
		return false;
	}
	*/
	GlobalFreePtr(alpbi);

	nFrames++;

    fprintf( stderr, "Wrote frame %d.\r", nFrames );

	return true;
}

static HANDLE  MakeDib( HBITMAP hbitmap, UINT bits )
{
	HANDLE              hdib ;
	HDC                 hdc ;
	BITMAP              bitmap ;
	UINT                wLineLen ;
	DWORD               dwSize ;
	DWORD               wColSize ;
	LPBITMAPINFOHEADER  lpbi ;
	LPBYTE              lpBits ;
	
	GetObject(hbitmap,sizeof(BITMAP),&bitmap) ;

	//
	// DWORD align the width of the DIB
	// Figure out the size of the colour table
	// Calculate the size of the DIB
	//
	wLineLen = (bitmap.bmWidth*bits+31)/32 * 4;
	wColSize = sizeof(RGBQUAD)*((bits <= 8) ? 1<<bits : 0);
	dwSize = sizeof(BITMAPINFOHEADER) + wColSize +
		(DWORD)(UINT)wLineLen*(DWORD)(UINT)bitmap.bmHeight;

	//
	// Allocate room for a DIB and set the LPBI fields
	//
	hdib = GlobalAlloc(GHND,dwSize);
	if (!hdib)
		return hdib ;

	lpbi = (LPBITMAPINFOHEADER)GlobalLock(hdib) ;

	lpbi->biSize = sizeof(BITMAPINFOHEADER) ;
	lpbi->biWidth = bitmap.bmWidth ;
	lpbi->biHeight = bitmap.bmHeight ;
	lpbi->biPlanes = 1 ;
	lpbi->biBitCount = (WORD) bits ;
	lpbi->biCompression = BI_RGB ;
	lpbi->biSizeImage = dwSize - sizeof(BITMAPINFOHEADER) - wColSize ;
	lpbi->biXPelsPerMeter = 0 ;
	lpbi->biYPelsPerMeter = 0 ;
	lpbi->biClrUsed = (bits <= 8) ? 1<<bits : 0;
	lpbi->biClrImportant = 0 ;

	//
	// Get the bits from the bitmap and stuff them after the LPBI
	//
	lpBits = (LPBYTE)(lpbi+1)+wColSize ;

	hdc = CreateCompatibleDC(NULL) ;

	GetDIBits(hdc,hbitmap,0,bitmap.bmHeight,lpBits,(LPBITMAPINFO)lpbi, DIB_RGB_COLORS);

	// Fix this if GetDIBits messed it up....
	lpbi->biClrUsed = (bits <= 8) ? 1<<bits : 0;

	DeleteDC(hdc) ;
	GlobalUnlock(hdib);

	return hdib ;
}


static HBITMAP LoadBMPFromFB( int w, int h )
{
    // Create a normal DC and a memory DC for the entire screen. The 
    // normal DC provides a "snapshot" of the screen contents. The 
    // memory DC keeps a copy of this "snapshot" in the associated 
    // bitmap. 
 
    HDC hdcScreen = wglGetCurrentDC();
    HDC hdcCompatible = CreateCompatibleDC(hdcScreen); 
 
    // Create a compatible bitmap for hdcScreen. 

    HBITMAP hbmScreen = CreateCompatibleBitmap(hdcScreen, 
                           //  GetDeviceCaps(hdcScreen, HORZRES), 
                           //  GetDeviceCaps(hdcScreen, VERTRES)); 
                             w, 
                             h ); 

    if (hbmScreen == 0)
        {
        fprintf( stderr, "hbmScreen == NULL\nExiting.\n" );
        exit( -1 );
        //errhandler("hbmScreen", hwnd); 
        }
 
    // Select the bitmaps into the compatible DC. 
 
    if (!SelectObject(hdcCompatible, hbmScreen)) 
        {
        fprintf( stderr, "Couldn't SelectObject()\nExiting.\n" );
        exit( -1 );
        //errhandler("Compatible Bitmap Selection", hwnd); 
        }
 
    // Hide the application window. 
 
     //       ShowWindow(hwnd, SW_HIDE); 
 
    //Copy color data for the entire display into a 
    //bitmap that is selected into a compatible DC. 
 
    if (!BitBlt(hdcCompatible, 
                 0,0, 
                 w, h, 
                 hdcScreen, 
                // 512,512,
                 0, 0,
                 SRCCOPY)) 
        {
        fprintf( stderr, "Screen to Compat Blt Failed\nExiting.\n" );
        exit( -1 );
        //errhandler("Screen to Compat Blt Failed", hwnd); 
        }
 
   // Redraw the application window.  
   //ShowWindow(hwnd, SW_SHOW); 

    DeleteDC( hdcCompatible );

    return( hbmScreen );
}

#elif LL_DARWIN

 #include <AGL/agl.h>
 #include <AGL/gl.h>
 #include <AGL/glu.h>

// BAD Apple.  BAD!
#ifdef verify
	#undef verify
#endif

#include "llviewerwindow.h"
#include "llworld.h"

MovieMaker::MovieMaker()
{
	movie = NULL;
	movieResRef = 0;
	track = NULL;
	media = NULL;
	width = 0;
	height = 0;
	bufferSize = 0;
	rowBytes = 0;
	buffer = NULL;
	invertedBuffer = NULL;
	ci = NULL;
	gworld = NULL;
	idh = NULL;
}

MovieMaker::~MovieMaker()
{
	EndCapture();
}

void MovieMaker::StartCapture( char *name , int x, int y)
{
    strncpy( fname, name, sizeof(fname));		/* Flawfinder: ignore */
	width = x;
	height = y;
	
	setupMovie();
}

OSStatus MovieMaker::setupMovie()
{
	OSStatus	error = noErr;
	FSRef		fileRef;
	FSSpec		fileSpec;
	
	rowBytes = width * 4;
	bufferSize = height * rowBytes;
	buffer = (char*)malloc(bufferSize);
	invertedBuffer = (char*)malloc(bufferSize);

	rect.left = 0;
	rect.top = 0;
	rect.right = width;
	rect.bottom = height;
	
	error = NewGWorldFromPtr(&gworld, k32ARGBPixelFormat, &rect, 0, 0, 0, buffer, rowBytes);

	if (error == noErr)
	{
		LockPixels(GetGWorldPixMap(gworld));
	}

// MBW -- I think this needs to happen after all the dialogs, etc.	
//	if (error == noErr)
//	{
//		Microseconds(&lastFrameTime);
//		error = grabFrame();
//	}

	if (error == noErr)
	{
		error = EnterMovies();
	}
	
	if (error == noErr)
	{
		ci = OpenDefaultComponent(StandardCompressionType,StandardCompressionSubType);
		if(ci == NULL)
			error = paramErr;
	}
		
	if (error == noErr)
	{
		long flags;
		
		SCGetInfo(ci,scPreferenceFlagsType,&flags);
		flags &= ~scShowBestDepth;
		flags |= scAllowZeroFrameRate;
		SCSetInfo(ci,scPreferenceFlagsType,&flags);
	}

	if (error == noErr)
	{
		send_agent_pause();
		gViewerWindow->mWindow->beforeDialog();

		error = SCRequestSequenceSettings(ci);

		gViewerWindow->mWindow->afterDialog();
		send_agent_resume();

		if (error == scUserCancelled) 
		{
			// deal with user cancelling.
			EndCapture();
		}
	}
	
	if (error == noErr)
	{
		// This is stoopid. I have to take the passed full path, create the file so I can get an FSRef, and Get Info to get the FSSpec for QuickTime. Could Apple make this any more difficult...
		FILE* file = LLFile::fopen(fname, "w");		/* Flawfinder: ignore */
		if (file)
		{
			fclose(file);
			
			error = FSPathMakeRef((UInt8*)fname, &fileRef, NULL);
			if (error == noErr)
				error = FSGetCatalogInfo(&fileRef, 0, NULL, NULL, &fileSpec, NULL);
		}
		else
		{
			error = paramErr;
		}
	}
	
	if (error == noErr)
	{
		error = CreateMovieFile(&fileSpec, 'TVOD', smCurrentScript, createMovieFileDeleteCurFile | createMovieFileDontCreateResFile, &movieResRef, &movie);
	}
	
	if (error == noErr)
	{
		track = NewMovieTrack(movie, FixRatio(width, 1), FixRatio(height, 1), kNoVolume);
		error = GetMoviesError();
	}
	
	if (error == noErr)
	{
		media = NewTrackMedia(track, VideoMediaType, 600, NULL, 0);
		error = GetMoviesError();
	}
	
	if (error == noErr)
	{
		Microseconds(&lastFrameTime);
		error = grabFrame();
	}

	if (error == noErr)
	{
		error = SCCompressSequenceBegin(ci,GetPortPixMap(gworld),nil,&idh);
	}

	if (error == noErr)
	{
		error = BeginMediaEdits(media);
	}
	
	if (error != noErr)
	{
		media = NULL;
	}
	
	return error;
}

void MovieMaker::EndCapture()
{
	OSStatus	error = noErr;

	if (movie && movieResRef)
	{
		if (media && track)
		{
			// Errors adding the frame aren't too important here.
			(void)addFrame();
			
			error = EndMediaEdits(media);
			if (error == noErr)
			{
				error = SCCompressSequenceEnd(ci);
			}

			if (error == noErr)
			{
				error = InsertMediaIntoTrack(track, 0, 0, GetMediaDuration(media), fixed1);
			}
			media = NULL;
			track = NULL;
		}
		
		short resId = movieInDataForkResID;
		error = AddMovieResource(movie, movieResRef, &resId, "\pSecond Life");
		CloseMovieFile(movieResRef);
		movieResRef = 0;
		movie = NULL;
	}
	
	// NOTE:  idh is disposed by SCCompressSequenceEnd.
	idh = NULL;
	
	if(ci)
	{
		CloseComponent(ci);
		ci = NULL;
	}
	
	if(gworld)
	{
		DisposeGWorld(gworld);
		gworld = NULL;
	}

	if(buffer)
	{
		free(buffer);
		buffer = NULL;
	}

	if(invertedBuffer)
	{
		free(invertedBuffer);
		invertedBuffer = NULL;
	}
}

OSStatus MovieMaker::grabFrame()
{
	OSStatus	error = noErr;
	GLenum		glerr;
	
	// Grab a frome from GL
	glReadBuffer(GL_BACK);
	glReadPixels(0 ,0, width, height,  
#ifdef LL_BIG_ENDIAN	
	// PowerPC Mac
	GL_BGRA,
	GL_UNSIGNED_INT_8_8_8_8_REV,
#else				
	// Intel Mac
	GL_BGRA,
	GL_UNSIGNED_INT_8_8_8_8,
#endif	
	invertedBuffer);
	glerr = glGetError();
	
	// Invert the lines top to bottom
	if (glerr == GL_NO_ERROR)
	{
		long i, j;
		
		i = j = 0;
			
		// Copy rows into tmp buffer one at a time, reversing their order
		for (i = 0, j = bufferSize - rowBytes; i < bufferSize; i += rowBytes, j -= rowBytes)
			BlockMoveData(&invertedBuffer[i], &buffer[j], rowBytes);
	}
	else
	{
		error = paramErr;
	}
	
	return error;
}

OSStatus MovieMaker::addFrame()
{
	OSStatus	error = noErr;
	Handle	compressedData;
	short   syncFlag;
	long	dataSize;
	UnsignedWide now;

	CGrafPtr oldPort;
	GDHandle oldGDeviceH;

	GetGWorld(&oldPort, &oldGDeviceH);
	SetGWorld(gworld, nil);
	
	// Compress the frame and add it to the movie

	error = SCCompressSequenceFrame(ci,GetPortPixMap(gworld),&rect,&compressedData,&dataSize,&syncFlag);
	
	Microseconds(&now);

	if (error == noErr)
	{
		double duration = (now.lo - lastFrameTime.lo);	// duration in microseconds
		duration *= GetMovieTimeScale(movie);	
		duration *= 1.0 / 1000000.0;				

		error = AddMediaSample(
				media,
				compressedData,
				0,
				dataSize,
				(TimeValue)duration,
				(SampleDescriptionHandle)idh,
				1,
				syncFlag,
				nil);
		
	}

	lastFrameTime = now;

	SetGWorld(oldPort, oldGDeviceH);
	
	return error;
}

bool MovieMaker::Snap()
{
	bool	result = false;
	
	if (movie && movieResRef && media && track)
	{
		OSStatus error = noErr;
		
		error = addFrame();
		
		if (error == noErr)
		{
			error = grabFrame();
		}

		if (error == noErr)
		{
			result = true;
		}
	}
	
	return result;
}

#endif

