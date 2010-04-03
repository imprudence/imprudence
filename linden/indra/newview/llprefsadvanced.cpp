/**
* @file llprefsadvanced.cpp
* @brief Advanced preferences options for Imprudence
*
* $LicenseInfo:firstyear=2009&license=viewergpl$
*
* Copyright (c) 2010, McCabe Maxsted
*
* Imprudence Viewer Source Code
* The source code in this file ("Source Code") is provided to you
* under the terms of the GNU General Public License, version 2.0
* ("GPL"). Terms of the GPL can be found in doc/GPL-license.txt in
* this distribution, or online at
* http://secondlifegrid.net/programs/open_source/licensing/gplv2
*
* There are special exceptions to the terms and conditions of the GPL as
* it is applied to this Source Code. View the full text of the exception
* in the file doc/FLOSS-exception.txt in this software distribution, or
* online at http://secondlifegrid.net/programs/open_source/licensing/flossexception
*
* By copying, modifying or distributing this software, you acknowledge
* that you have read and understood your obligations described above,
* and agree to abide by those obligations.
*
* ALL SOURCE CODE IS PROVIDED "AS IS." THE AUTHOR MAKES NO
* WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
* COMPLETENESS OR PERFORMANCE.
* $/LicenseInfo$
*/

#include "llviewerprecompiledheaders.h"

#include "llprefsadvanced.h"

#include "lluictrlfactory.h"

LLPrefsAdvanced::LLPrefsAdvanced()
{
	LLUICtrlFactory::getInstance()->buildPanel(this, "panel_preferences_advanced.xml");
}

LLPrefsAdvanced::~LLPrefsAdvanced()
{
	// Children all cleaned up by default view destructor.
}

BOOL LLPrefsAdvanced::postBuild()
{
	return TRUE;
}

void LLPrefsAdvanced::apply()
{
}

void LLPrefsAdvanced::cancel()
{
}
