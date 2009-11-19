/**
 * @file llsimurlstring.h
 * @brief Handles "SLURL fragments" like Ahern/123/45 for
 * startup processing, login screen, prefs, etc.
 *
 * $LicenseInfo:firstyear=2006&license=viewergpl$
 * 
 * Copyright (c) 2006-2009, Linden Research, Inc.
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
#ifndef LLSIMURLSTRING_H
#define LLSIMURLSTRING_H

#include "llstring.h"

class LLURLSimString
{
public:
	enum { NOT_SET=0, NOT_PARSED=1, PARSE_OK=2, PARSE_FAIL=-1 };

	static void setString(const std::string& url);
		// Accepts all sorts of fragments:
		//   secondlife://RegionName/1/2/
		//   sl://RegionName/1/2/3/
		//   //Ahern/123/45/
		//   Ahern

	static bool parse();
		// Returns true if we have an URL fragment in the static instance
		// (and it parsed correctly, which is basically always because
		// any bare region string is a valid fragment).

	static bool parse(const std::string& sim_string, std::string *region_name, S32 *x, S32 *y, S32 *z);
		// Parse a sim string "Ahern/1/2" and return location data,
		// doesn't affect static instance.

	static std::string getURL();
		// Get the canonical URL secondlife://RegionName/123/45/6/
	
	static std::string unescapeRegionName(std::string region_name);
		// Does URL unescaping, in particular %20 -> space

	LLURLSimString() : mX(128), mY(128), mZ(0), mParseState(NOT_PARSED) {}

private:
	static std::string::size_type parseGridIdx(const std::string& in_string,
											   std::string::size_type idx0,
											   std::string::size_type* res);

public:
	static LLURLSimString sInstance;
	static std::string sLocationStringHome;
	static std::string sLocationStringLast;

public:
	std::string mSimString; // "name/x/y/z"
	std::string mSimName;
	S32 mX,mY,mZ;
	S32 mParseState;
};

#endif
