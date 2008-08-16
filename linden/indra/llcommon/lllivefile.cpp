/** 
 * @file lllivefile.cpp
 *
 * Copyright (c) 2006-2007, Linden Research, Inc.
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

#include "lllivefile.h"


LLLiveFile::LLLiveFile(const std::string &filename, const F32 refresh_period) :
mForceCheck(true),
mRefreshPeriod(refresh_period),
mFilename(filename),
mLastModTime(0),
mLastExists(false)
{
}


LLLiveFile::~LLLiveFile()
{
}


bool LLLiveFile::checkAndReload()
{
	if (!mForceCheck && mRefreshTimer.getElapsedTimeF32() < mRefreshPeriod)
	{
		// Skip the check if not enough time has elapsed and we're not
		// forcing a check of the file
		return false;
	}
	mForceCheck = false;
	mRefreshTimer.reset();

	// Stat the file to see if it exists and when it was last modified.
	llstat stat_data;
	int res = LLFile::stat(mFilename.c_str(), &stat_data);

	if (res)
	{
		// Couldn't stat the file, that means it doesn't exist or is
		// broken somehow.  Clear flags and return.
		if (mLastExists)
		{
			loadFile(); // Load the file, even though it's missing to allow it to clear state.
			mLastExists = false;
			return true;
		}
		return false;
	}

	// The file exists, decide if we want to load it.
	if (mLastExists)
	{
		// The file existed last time, don't read it if it hasn't changed since
		// last time.
		if (stat_data.st_mtime <= mLastModTime)
		{
			return false;
		}
	}

	// We want to read the file.  Update status info for the file.
	mLastExists = true;
	mLastModTime = stat_data.st_mtime;
	
	loadFile();
	return true;
}

