/** 
 * @file llindraconfigfile.cpp
 *
 *
 * This class is an LLLiveFile that has config info for indra
 * Currently only whether it's blacklisted
 *
 * $LicenseInfo:firstyear=2007&license=viewergpl$
 * 
 * Copyright (c) 2007-2009, Linden Research, Inc.
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

#include "llindraconfigfile.h"

#include "llfile.h"
#include "llsd.h"
#include "llsdserialize.h"
#include "llframetimer.h"

static std::string sConfigDir = "";
static const char indraConfigFileName[] = "indra.xml";


LLIndraConfigFile::LLIndraConfigFile()
        : LLLiveFile(filename(), configFileRefreshRate),
          mConfig(LLSD())
{
}

//static
void LLIndraConfigFile::initClass(const std::string& config_dir)
{
	sConfigDir = config_dir;
	llinfos << "LLIndraConfigFile::initClass config dir "
			<< config_dir << "/" << indraConfigFileName << llendl;
}

LLSD LLIndraConfigFile::getConfig(const std::string& config_name)
{
	if (sConfigDir.empty())
	{
		llerrs << "LLIndraConfigFile::initClass() not called" << llendl;
	}

	LLFrameTimer::updateFrameTime();
	
	static LLIndraConfigFile the_file;
	the_file.checkAndReload();

    return the_file.mConfig[config_name];
}

std::string LLIndraConfigFile::filename()
{
    std::ostringstream ostr;

	ostr << sConfigDir
		 << "/" << indraConfigFileName;

	return ostr.str();
}

/* virtual */
void LLIndraConfigFile::loadFile()
{
	llinfos << "LLIndraConfigFile::loadFile: reading from "
		<< filename() << llendl;
        
    LLSD config;

    {
        llifstream file(filename());
        if (file.is_open())
        {
            LLSDSerialize::fromXML(config, file);
        }

        if (config.isUndefined())
        {
            llinfos << "LLIndraConfigFile::loadFile: file missing, ill-formed,"
                " or simply undefined; not changing the blacklist" << llendl;
            return;
        }
    }

	if (config.isMap())
	{
		mConfig = config;
	}
	else
    {
        llwarns << "LLIndraConfigFile: " << indraConfigFileName << "  expects a map; wrong format" << llendl;
        return;
    }
}
