/** 
 * @file llerrorbuffer.h
 * @brief Buffer implementation for logging.
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

#ifndef LL_LLERRORBUFFER_H
#define LL_LLERRORBUFFER_H

#include <fstream>
#include <iostream>
#include <string>

#include "lldefs.h"
#include "stdtypes.h"
#include "llfile.h"

// A streambuf that sends it's output into a file, stderr, or syslog.
//
// Each output can be enabled/disabled, and a priority can be set.

class LLFixedBuffer;

class LLErrorBuffer : public std::streambuf
{
 public:

	// Specify error levels
	enum ELevel
	{
		DEBUG = 0,
		INFO = 1,
		WARN = 2,
		FATAL = 3,
		NONE = 4 // Do NO error logging (for use in signal handlers)
	};

    LLErrorBuffer();
    ~LLErrorBuffer();
    BOOL setFile(const char *filename);
	void closeFile();
	const char *getFilename() const;
	void setFixedBuffer(LLFixedBuffer *b) { mFixedBuf = b; };

	// Sets the priority of the current message
    void setPriority(const ELevel l) { mPriority = l; }

	// Only display messages >= to this level
	void setLevel(const ELevel l) { mLevel = l; }
	ELevel getLevel()			{ return mLevel; }
	// Display messages >= to level l, if l < current level
	ELevel mergeLevel(const ELevel l);

	// on linux, this sets syslog info to be a LOG_NOTICE which will
	// be centrally logged. *NOTE: This is very
	// linux/syslog/configuration dependent.
	void setElevatedRemote(BOOL b) { mElevatedRemote = b; }

	// logs are in utc rather than local
	void setUTCTimestamp(BOOL utc);

	// Turn on or off logging outputs
    void enableError(BOOL active);
	void enableErrorTimestamp(BOOL active);
    void enableFile(BOOL active);
    void enableSyslog(BOOL active);
#if LL_WINDOWS
	void enableWinDebug(BOOL active);
#endif // LL_WINDOWS

 protected:
    int overflow(int c = EOF);

 private:
	char mFilename[LL_MAX_PATH];	/* Flawfinder: ignore */
	int ELevelToSyslogPriority(const ELevel l);
	
	llofstream *mFile;
	std::string mBuf;
	LLFixedBuffer *mFixedBuf;
	
    BOOL mErrorActive;
	BOOL mErrorTimestamp;
    BOOL mFileActive;
    BOOL mSyslogActive;
	BOOL mWinDebugActive;
	BOOL mElevatedRemote;
	BOOL mIsUTC;

	// If priority < level, output is thrown away
	ELevel mLevel;
	// Current message priority
	ELevel mPriority;
};
 
#endif // LL_LLERRORBUFFER_H
