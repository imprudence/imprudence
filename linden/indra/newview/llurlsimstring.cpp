/** 
 * @file llurlsimstring.cpp (was llsimurlstring.cpp)
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

#include "llviewerprecompiledheaders.h"

#include "llurlsimstring.h"

#include "llpanellogin.h"
#include "llviewercontrol.h"

#include "curl/curl.h"

//static
LLURLSimString LLURLSimString::sInstance;
std::string LLURLSimString::sLocationStringHome("My Home");
std::string LLURLSimString::sLocationStringLast("My Last Location");

// "secondlife://simname/x/y/z" -> "simname/x/y/z"
// (actually .*//foo -> foo)
// static
void LLURLSimString::setString(const std::string& sim_string)
{
	sInstance.mSimString.clear();
	sInstance.mSimName.clear();
	sInstance.mParseState = NOT_PARSED;
	if (sim_string == sLocationStringHome)
	{
		gSavedSettings.setBOOL("LoginLastLocation", FALSE);
	}
	else if (sim_string == sLocationStringLast)
	{
		gSavedSettings.setBOOL("LoginLastLocation", TRUE);
	}
	else
	{
		char* curlstr = curl_unescape(sim_string.c_str(), sim_string.size());
		std::string tstring = std::string(curlstr);
		curl_free(curlstr);
		std::string::size_type idx = tstring.find("//");
		idx = (idx == std::string::npos) ? 0 : idx+2;
		sInstance.mSimString = tstring.substr(idx);
	}
}

// "/100" -> 100
// static
std::string::size_type LLURLSimString::parseGridIdx(const std::string& in_string,
												 std::string::size_type idx0,
												 std::string::size_type* res)
{
	if (idx0 == std::string::npos || in_string[idx0] != '/')
	{
		return std::string::npos; // parse error
	}
	idx0++;
	std::string::size_type idx1 = in_string.find_first_of('/', idx0);
	std::string::size_type len = (idx1 == std::string::npos) ? std::string::npos : idx1-idx0;
	std::string tstring = in_string.substr(idx0,len);
	if (!tstring.empty())
	{
		std::string::size_type val = atoi(tstring.c_str());
		*res = val;
	}
	return idx1;
}

// "simname/x/y/z" -> mSimName = simname, mX = x, mY = y, mZ = z
// static
bool LLURLSimString::parse()
{
	if (sInstance.mParseState == NOT_SET)
	{
		return false;
	}
	if (sInstance.mParseState == NOT_PARSED)
	{
		if (parse(sInstance.mSimString,
				  &sInstance.mSimName,
				  &sInstance.mX, 
				  &sInstance.mY, 
				  &sInstance.mZ))
		{
			sInstance.mParseState = PARSE_OK;
		}
		else
		{
			sInstance.mParseState = PARSE_FAIL;
		}
	}
	return (sInstance.mParseState == PARSE_OK);
}

// static
bool LLURLSimString::parse(const std::string& sim_string,
						   std::string *region_name,
						   S32 *x, S32 *y, S32 *z)
{
	// strip any bogus initial '/'
	std::string::size_type idx0 = sim_string.find_first_not_of('/');
	if (idx0 == std::string::npos) idx0 = 0;

	std::string::size_type idx1 = sim_string.find_first_of('/', idx0);
	std::string::size_type len = (idx1 == std::string::npos) ? std::string::npos : idx1-idx0;
	std::string tstring = sim_string.substr(idx0,len);
	*region_name = unescapeRegionName(tstring);
	if (!region_name->empty())
	{
		// return position data if found. otherwise leave passed-in values alone. (DEV-18380) -MG
		if (idx1 != std::string::npos)
		{
			std::string::size_type xs = *x, ys = *y, zs = *z;
			idx1 = parseGridIdx(sim_string, idx1, &xs);
			idx1 = parseGridIdx(sim_string, idx1, &ys);
			idx1 = parseGridIdx(sim_string, idx1, &zs);
			*x = xs;
			*y = ys;
			*z = zs;
		}
		
		return true;
	}
	else
	{
		return false;
	}
}

// static
std::string LLURLSimString::getURL()
{
	std::string url;
	if (sInstance.mParseState == PARSE_OK)
	{
		url = llformat("secondlife://%s/%d/%d/%d/",
					sInstance.mSimName.c_str(),
					sInstance.mX,
					sInstance.mY,
					sInstance.mZ);
	}
	return url;
}

// static
std::string LLURLSimString::unescapeRegionName(std::string region_name)
{
	std::string result;
	char* curlstr = curl_unescape(region_name.c_str(), region_name.size());
	result = std::string(curlstr);
	curl_free(curlstr);
	return result;
}
