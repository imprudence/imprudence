/** 
 * @file llerrorbuffer.cpp
 *
 * Copyright (c) 2002-2007, Linden Research, Inc.
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

#include "llerrorbuffer.h"
#include "llfixedbuffer.h"

#include <fstream>
#include <string.h>

#if LL_WINDOWS
#	define WIN32_LEAN_AND_MEAN
#	include <winsock2.h>
#	include <windows.h>		// for OutputDebugString
#else
#	include <syslog.h>
#	include <stdio.h>
#endif

#include <time.h>

// In order to compile in Visual C++ 6.0, you must use std:: in the header, and then
// use the std namespace in the cpp.  Otherwise, you'll break the (very) fragile C++ parser.
using namespace std;

LLErrorBuffer::LLErrorBuffer()
:	streambuf(),
    mFile(NULL),
	mBuf(),
	mFixedBuf(NULL),
    mErrorActive(TRUE),
	mErrorTimestamp(TRUE),
    mFileActive(FALSE),
    mSyslogActive(TRUE),
	mWinDebugActive(TRUE),
	mElevatedRemote(FALSE),
	mIsUTC(TRUE),
	mLevel(DEBUG),
	mPriority(DEBUG)
{
	mFilename[0] = '\0';
#if LL_WINDOWS
	// by default, turn off timestamps in the debug log on windows
	mErrorTimestamp = FALSE;
#endif
}

LLErrorBuffer::~LLErrorBuffer()
{
    delete mFile;
	mFile = NULL;
}

#if !LL_WINDOWS
int LLErrorBuffer::ELevelToSyslogPriority(const ELevel l)
{
		switch(l)
		{
		case DEBUG:
				return LOG_DEBUG;
		case INFO:
			switch(mElevatedRemote)
			{
			case TRUE: return LOG_NOTICE;
			default: return LOG_INFO;
			}
		case WARN:
				return LOG_WARNING;
		case FATAL:
				return LOG_CRIT;
		default:
				return LOG_CRIT;
		}
		return LOG_CRIT;
}
#endif // LL_WINDOWS

BOOL LLErrorBuffer::setFile(const char *filename)
{
    if (mFile == NULL)
	{
		mFile = new llofstream();
	}
    if (mFile->is_open())
	{
		mFile->close();
	}
	if (filename == NULL)
	{
		llwarns << "Input filename is NULL!!" << llendl;
		return FALSE;
	}
    mFile->open(filename, llofstream::out | llofstream::app);	/* Flawfinder: ignore */
    if (mFile->is_open())
	{
		mFileActive = TRUE;
	}
    else
	{
		mFileActive = FALSE;
		delete mFile;
		mFile = NULL;
    }
	snprintf(mFilename, sizeof(mFilename), filename);	/* Flawfinder: ignore */
    return mFileActive;
}

void LLErrorBuffer::closeFile()
{
    if (mFile && mFile->is_open())
	{
		mFile->close();
		mFileActive = FALSE;
		delete mFile;
		mFile = NULL;
	}
}

const char * LLErrorBuffer::getFilename() const
{
	return mFilename;
}

void LLErrorBuffer::setUTCTimestamp(BOOL utc)
{
	mIsUTC = utc;
}

void LLErrorBuffer::enableError(BOOL active)
{
    mErrorActive = active;
}

void LLErrorBuffer::enableErrorTimestamp(BOOL active)
{
	mErrorTimestamp = active;
}

void LLErrorBuffer::enableFile(BOOL active)
{
    if (mFile != NULL)
    {
	if (mFile->is_open())
	    mFileActive = active;
    }
    else
	mFileActive = FALSE;
}

#if !LL_WINDOWS
void LLErrorBuffer::enableSyslog(BOOL active)
{
    mSyslogActive = active;
}
#endif // LL_WINDOWS

#if LL_WINDOWS
void LLErrorBuffer::enableWinDebug(BOOL active)
{
	mWinDebugActive = active;
}
#endif // LL_WINDOWS

int LLErrorBuffer::overflow(int c)
{
    if (EOF != c)
    {
			if ('\n' == c)
			{
					// If we're not supposed to print anything, don't, but
					// pretend that we did so taht the iostream doesn't think
					// there's been a failure
					if (mPriority < mLevel)
					{
							// Flush our message buffer
							mBuf = "";
							return 0;
					}
#if !LL_WINDOWS
					if (mSyslogActive)
					{
							int pri = ELevelToSyslogPriority(mPriority);
							syslog(pri, "%s", mBuf.c_str());
					}
#endif // LL_WINDOWS
					const S32 BUF_SIZE = 64;
					char time_str[BUF_SIZE];	/* Flawfinder: ignore */
					if (mFileActive || mErrorActive)
					{
							time_t now;
							time(&now);
							S32 chars;
							if(mIsUTC)
							{
								chars = (S32)strftime(time_str, BUF_SIZE, 
													  "%Y-%m-%dT%H:%M:%SZ",
													  gmtime(&now));
							}
							else
							{
								chars = (S32)strftime(time_str, BUF_SIZE, 
													  "%Y-%m-%dT%H:%M:%S %Z",
													  localtime(&now));
							}
							if (0 == chars)
							{
								strcpy(time_str, "time error");	/* Flawfinder: ignore */
							}
					}
					if (mFileActive)
					{
						*mFile << time_str << " " << mBuf << std::endl;
					}
					if (mErrorActive)
					{
						if (mErrorTimestamp)
						{
							fprintf(stderr, "%s %s\n", time_str, mBuf.c_str());
						}
						else
						{
							fprintf(stderr, "%s\n", mBuf.c_str());
						}

						// std::cerr goes into the void on the viewer
						//std::cerr << time_str << ' ' << mBuf << std::endl;
					}
					if (mFixedBuf)
					{
						mFixedBuf->addLine(mBuf.c_str());
					}
#if LL_WINDOWS
					if (mWinDebugActive)
					{
						llutf16string utf16str = wstring_to_utf16str(utf8str_to_wstring(mBuf));
						utf16str += '\n';
						OutputDebugString(utf16str.c_str());
					}
#endif // LL_WINDOWS
					// Is there a better way to truncate a string?
					mBuf.erase(0, mBuf.length()); // Hack, Linux doesn't implement clear()!
			}
			else
			{
					mBuf += c;
			}
	}
	return 0;
}

LLErrorBuffer::ELevel LLErrorBuffer::mergeLevel(const LLErrorBuffer::ELevel l)
{
		mLevel = llmin(mLevel, l);
		return mLevel;
}
