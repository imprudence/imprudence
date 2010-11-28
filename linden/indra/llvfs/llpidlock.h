/** 
 * @file llpidlock.h
 * @brief System information debugging classes.
 *
 * $LicenseInfo:firstyear=2001&license=viewergpl$
 * 
 * Copyright (c) 2001-2009, Linden Research, Inc.
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

#ifndef LL_PIDLOCK_H
#define LL_PIDLOCK_H
#include "llnametable.h"

class LLSD;
class LLFrameTimer;

#if LL_WINDOWS	//For windows platform.

#	define WIN32_LEAN_AND_MEAN
#	include <winsock2.h>
#include <windows.h>

#else	//Everyone Else

#include <signal.h>

#endif //Everyone else.

namespace LLPidLock
{
    void initClass(); // { (void) LLPidLockFile::instance(); }

	bool requestLock( LLNameTable<void *> *name_table=NULL, bool autosave=TRUE,
					  bool force_immediate=FALSE, F32 timeout=300.0);
	bool checkLock(); 
	void releaseLock(); 
	bool isClean(); 

	//getters
	LLNameTable<void *> * getNameTable(); 
	bool getAutosave(); 
	bool getClean(); 
	std::string getSaveName(); 

	//setters
	void setClean(bool clean); 
	void setSaveName(std::string savename); 
};

#endif // LL_PIDLOCK_H
