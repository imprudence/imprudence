/** 
 * @file llerrorstream.h
 * @brief Declaration of c++ log straming.
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

#ifndef LL_LLERRORSTREAM_H
#define LL_LLERRORSTREAM_H

// Usage:
// LLErrorStream gErrorStream(gErrorBuffer);
//
// gErrorStream << debug << "This is a debug message" << endl;
//
// gErrorStream << fatal << "This message will cause a crash!" << endl;

#include <iostream>
#include "llerrorbuffer.h"
#include "stdtypes.h"

class LLFixedBuffer;

class LLErrorStream : public std::ostream
{
 public:
    LLErrorStream(LLErrorBuffer *eb);
	
	// Specify error levels-- Use LLErrorBuffer::ELevel instead
	enum ELevel
	{
		DEBUG = 0,
		INFO = 1,
		WARN = 2,
		FATAL = 3,
		NONE = 4 // Don't log anything
	};

    LLErrorBuffer *mErrorBuffer;

	// This is used to specify if we need to merge the DebugMask
	// or replace it
	enum EControl
	{
			REPLACE = 0,
			MERGE = 1	
	};

	void setDebugMask(U32 mask)			{ mDebugMask = mask; }
	void mergeDebugMask(U32 mask)		{ mDebugMask |= mask; }
	U32 getDebugMask()					{ return mDebugMask; }
	void setDebugFlag(U32 flag)			{ mDebugMask |= flag; }
	void clearDebugFlag(U32 flag)		{ mDebugMask &= ~flag; }
	BOOL setFile(const char *path) { return mErrorBuffer->setFile(path); }
	void closeFile()					{ mErrorBuffer->closeFile(); }
	const char *getFilename() const			{ return mErrorBuffer->getFilename(); }
	void setFixedBuffer(LLFixedBuffer *b) { mErrorBuffer->setFixedBuffer(b); }
	void setErrorLevel(const LLErrorBuffer::ELevel l) { mErrorBuffer->setLevel(l); }
	LLErrorBuffer::ELevel getErrorLevel() { return mErrorBuffer->getLevel(); }
	void mergeErrorLevel(const LLErrorBuffer::ELevel l) { mErrorBuffer->mergeLevel(l); }
	void setError() { mKill = TRUE; };
	void crashOnError(std::ostringstream &ss, LLErrorBuffer::ELevel l);

	BOOL isEnabledFor(const LLErrorBuffer::ELevel l);
	BOOL isEnabledFor(const LLErrorBuffer::ELevel l, const U32 type);


	void mergeLevel(const LLErrorBuffer::ELevel l) { mErrorBuffer->mergeLevel(l); }

	void setPrintLocation(BOOL b) { mPrintLocation = b; }
	BOOL getPrintLocation() { return mPrintLocation; }

	void setElevatedRemote(BOOL b) { mErrorBuffer->setElevatedRemote(b); }
	void setUTCTimestamp(BOOL utc) { mErrorBuffer->setUTCTimestamp(utc); }
			
	// Deprecated
	void setLevel(const ELevel l) { setErrorLevel(ELevelToBufferELevel(l)); }
	ELevel getLevel() { return BufferELevelToELevel(getErrorLevel()); }
	void mergeLevel(const ELevel l) { mergeErrorLevel(ELevelToBufferELevel(l)); }


	// Backwards compatilibity cruft.  Should be removed
	void mergeTime(BOOL b) { } // NOP
	void mergeLocation(BOOL b) { } // NOP
	void setTime(BOOL b) { } // NOP
	char *getTime() { return ""; } // NOP
	
	typedef void(*LLErrorCallback)(const std::string &error_string);
	void setErrorCallback(LLErrorCallback callback) {mErrorCallback = callback;}

private:
	// This maintains the existing ELevel interface, but new code should use
	// LLErrorBuffer::ELevel instead.
	LLErrorBuffer::ELevel ELevelToBufferELevel(const ELevel l);
	ELevel BufferELevelToELevel(const LLErrorBuffer::ELevel l);
	U32 mDebugMask; // Mask for debugst() output

	BOOL mPrintLocation;

	S32  mSafeDepth; // Counter so we can safely do recursive calls, 0 means we're OK
	BOOL mKill;
	LLErrorCallback mErrorCallback;
};


#endif // LL_LLERRORSTREAM_H
