/** 
 * @file llerrorstream.cpp
 * @brief Implementation of c++ log straming.
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

#include "llerrorstream.h"
#include "llerrorbuffer.h"
#include "llerror.h"

using namespace std;

// Define this if we're using APR mutexes.
#include "llapr.h"
extern apr_thread_mutex_t *gLogMutexp;

// In order to compile in Visual C++ 6.0, you must use std:: in the header, and then
// use the std namespace in the cpp.  Otherwise, you'll break the (very) fragile C++ parser.
LLErrorStream::LLErrorStream(LLErrorBuffer *eb):
		ostream(eb),
		mErrorBuffer(eb),
		mKill(FALSE),
		mErrorCallback(NULL)
{
#ifdef SHOW_DEBUG
	setErrorLevel(LLErrorBuffer::DEBUG);
#else
#ifdef SHOW_INFO
	setErrorLevel(LLErrorBuffer::INFO);
#else
#ifdef SHOW_WARN
	setErrorLevel(LLErrorBuffer::WARN);
#else
	setErrorLevel(LLErrorBuffer::FATAL);
#endif // SHOW_WARN
#endif // SHOW_INFO
#endif // SHOW_DEBUG

	setDebugMask(LLERR_NONE);
	setPrintLocation(FALSE);
}

LLErrorBuffer::ELevel LLErrorStream::ELevelToBufferELevel(const ELevel l)
{
	switch (l)
	{
	case DEBUG:
		return LLErrorBuffer::DEBUG;
	case INFO:
		return LLErrorBuffer::INFO;
	case WARN:
		return LLErrorBuffer::WARN;
	case FATAL:
		return LLErrorBuffer::FATAL;
	default:
		return LLErrorBuffer::FATAL;
	}
}

LLErrorStream::ELevel LLErrorStream::BufferELevelToELevel(const LLErrorBuffer::ELevel l)
{
	switch(l)
	{
	case LLErrorBuffer::DEBUG:
		return DEBUG;
	case LLErrorBuffer::INFO:
		return INFO;
	case LLErrorBuffer::WARN:
		return WARN;
	case LLErrorBuffer::FATAL:
		return FATAL;
	default:
		return FATAL;
	}
}


BOOL LLErrorStream::isEnabledFor(const LLErrorBuffer::ELevel l)
{
	if (l == LLErrorBuffer::FATAL)
	{
		if (LLErrorBuffer::FATAL < getErrorLevel())
		{
			// Fatal error, but we're at log level NONE (used by signal handlers)
			// We want to crash this process now instead of logging
			_llcrash_and_loop();
		}
	}
	// Always returns false if not safe (recursive call)
	return (getErrorLevel() <= l);
}


BOOL LLErrorStream::isEnabledFor(const LLErrorBuffer::ELevel l, const U32 type)
{
	// Always returns false if not safe (recursive call)
	return (getErrorLevel() <= l) && (mDebugMask & type);
}



void LLErrorStream::crashOnError(std::ostringstream &oss, LLErrorBuffer::ELevel l)
{
	if (gLogMutexp)
	{
		const S32 MAX_RETRIES = 5;
		S32 attempts = 0;
		while (attempts < MAX_RETRIES)
		{
			apr_status_t s = apr_thread_mutex_trylock(gLogMutexp);
			if (!APR_STATUS_IS_EBUSY(s))
			{
				break;
			}
			else
			{
				attempts++;
				ms_sleep(1);

				//
				// Just yielding won't necessarily work, I had problems with this on Linux - doug 12/02/04
				//apr_thread_yield();
			}
		}
		if (attempts == MAX_RETRIES)
		{
			// We're hosed, we can't get the mutex.
			// I guess we just won't log, then.  Blah.
			fprintf(stderr, "LLErrorStream::crashOnError() failed to get mutex for log\n");
			return;
		}
	}

	mErrorBuffer->setPriority(l);
	if (LLErrorBuffer::FATAL == l)
	{
		setError();
	}

	*this << oss.str();

	if (mKill)
	{
		// We want to flush this stream.
		flush();
	}

	BOOL crashme = FALSE;
	if (mKill)
	{
		crashme = TRUE;
	}
	mKill = FALSE;

	if (gLogMutexp)
	{
		apr_thread_mutex_unlock(gLogMutexp);
	}

	if (crashme)
	{
		mErrorCallback(oss.str());
		_llcrash_and_loop();
	}
}
