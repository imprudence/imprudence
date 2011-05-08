/** 
 * @file llviewernetwork.cpp
 * @author James Cook, Richard Nelson
 * @brief Networking constants and globals for viewer.
 *
 * $LicenseInfo:firstyear=2006&license=viewergpl$
 * 
 * Copyright (c) 2006-2008, Linden Research, Inc.
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

#include "llviewernetwork.h"
#include "llviewercontrol.h"
#include "llstartup.h"

 #include "hippogridmanager.h"


unsigned char gMACAddress[MAC_ADDRESS_BYTES];		/* Flawfinder: ignore */


void LLViewerLogin::getLoginURIs(std::vector<std::string>& uris) const
{
	// return the login uri set on the command line.
	LLControlVariable* c = gSavedSettings.getControl("CmdLineLoginURI");
	if(c && !LLStartUp::shouldAutoLogin())
	{
		LLSD v = c->getValue();
		if(v.isArray())
		{
			for(LLSD::array_const_iterator itr = v.beginArray();
				itr != v.endArray(); ++itr)
			{
				std::string uri = itr->asString();
				if(!uri.empty())
				{
					uris.push_back(uri);
				}
			}
		}
		else
		{
			std::string uri = v.asString();
			if(!uri.empty())
			{
				uris.push_back(uri);
			}
		}
	}
	
	// If there was no command line uri...
	if(uris.empty())
	{
		uris.push_back(gHippoGridManager->getConnectedGrid()->getLoginURI());
		/*
		// If its a known grid choice, get the uri from the table,
		// else try the grid name.
		if(mGridChoice > GRID_INFO_NONE && mGridChoice < GRID_INFO_OTHER)
		{
			uris.push_back(gGridInfo[mGridChoice].mLoginURI);
		}
		else
		{
			uris.push_back(mGridName);
		} */
	}
}

const std::string &LLViewerLogin::getGridLabel() const
{
	return gHippoGridManager->getConnectedGrid()->getGridName();
}

const std::string &LLViewerLogin::getLoginPage() const
{
	return gHippoGridManager->getConnectedGrid()->getLoginPage();
}

const std::string &LLViewerLogin::getHelperURI() const
{
	return gHippoGridManager->getConnectedGrid()->getHelperURI();
}
  
bool LLViewerLogin::isOpenSimulator()
{
	return gHippoGridManager->getConnectedGrid()->isOpenSimulator();
}

bool LLViewerLogin::isSecondLife()
{
	return gHippoGridManager->getConnectedGrid()->isSecondLife();
}

bool LLViewerLogin::isInProductionGrid()
{
	return true;
}
