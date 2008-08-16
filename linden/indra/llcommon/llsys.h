/** 
 * @file llsys.h
 * @brief System information debugging classes.
 *
 * Copyright (c) 2001-2007, Linden Research, Inc.
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

#ifndef LL_SYS_H
#define LL_SYS_H

//
// The LLOSInfo, LLCPUInfo, and LLMemoryInfo classes are essentially
// the same, but query different machine subsystems. Here's how you
// use an LLCPUInfo object:
//
//  LLCPUInfo info;
//  llinfos << info << llendl;
//

#include <iosfwd>
#include <string>

class LLOSInfo
{
public:
	LLOSInfo();
	void stream(std::ostream& s) const;

	const std::string& getOSString() const;

	S32 mMajorVer;
	S32 mMinorVer;
	S32 mBuild;

#ifndef LL_WINDOWS
	static S32 getMaxOpenFiles();
#endif

	static U32 getProcessVirtualSizeKB();
	static U32 getProcessResidentSizeKB();
private:
	std::string mOSString;
};


class LLCPUInfo
{
public:
	LLCPUInfo();	
	void stream(std::ostream& s) const;

	std::string getCPUString() const;

	BOOL  hasSSE() const	{ return mHasSSE; }
	BOOL  hasSSE2()	const	{ return mHasSSE2; }
	S32	  getMhz() const	{ return mCPUMhz; }

	// Family is "AMD Duron" or "Intel Pentium Pro"
	const std::string& getFamily() const { return mFamily; }

private:
	BOOL mHasSSE;
	BOOL mHasSSE2;
	S32 mCPUMhz;
	std::string mFamily;
};

class LLMemoryInfo
{
public:
	LLMemoryInfo();
	void stream(std::ostream& s) const;

	U32 getPhysicalMemory() const;
};


std::ostream& operator<<(std::ostream& s, const LLOSInfo& info);
std::ostream& operator<<(std::ostream& s, const LLCPUInfo& info);
std::ostream& operator<<(std::ostream& s, const LLMemoryInfo& info);

// gunzip srcfile into dstfile.  Returns FALSE on error.
BOOL gunzip_file(const char *srcfile, const char *dstfile);
// gzip srcfile into dstfile.  Returns FALSE on error.
BOOL gzip_file(const char *srcfile, const char *dstfile);

extern LLCPUInfo gSysCPU;

#endif // LL_LLSYS_H
