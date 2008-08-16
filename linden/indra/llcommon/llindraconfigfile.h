/** 
 * @file llindraconfigfile.h
 * @brief manages  configuration file for indra.xml
 *
 * $LicenseInfo:firstyear=2007&license=internal$
 * 
 * Copyright (c) 2007-2008, Linden Research, Inc.
 * 
 * The following source code is PROPRIETARY AND CONFIDENTIAL. Use of
 * this source code is governed by the Linden Lab Source Code Disclosure
 * Agreement ("Agreement") previously entered between you and Linden
 * Lab. By accessing, using, copying, modifying or distributing this
 * software, you acknowledge that you have been informed of your
 * obligations under the Agreement and agree to abide by those obligations.
 * 
 * ALL LINDEN LAB SOURCE CODE IS PROVIDED "AS IS." LINDEN LAB MAKES NO
 * WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
 * COMPLETENESS OR PERFORMANCE.
 * $/LicenseInfo$
 */

#ifndef LL_LLINDRACONFIGFILE_H
#define LL_LLINDRACONFIGFILE_H

#include <string>
#include "linden_common.h"

#include "lllivefile.h"
#include "llsd.h"


// To use, call LLIndraConfigFile::initClass(config_dir);
// Then whenever getConfig is called, it will check and reload automatically

class LLIndraConfigFile : public LLLiveFile
{
public:
	LLIndraConfigFile();
	static void initClass(const std::string& config_dir);
    static LLSD getConfig(const std::string& config_name);

private:
    static std::string filename();

protected:
	/* virtual */ void loadFile();
	LLSD mConfig;
};

#endif //LL_LLINDRACONFIGFILE_H
