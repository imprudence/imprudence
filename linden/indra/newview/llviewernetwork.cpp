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
 * online at http://secondlifegrid.net/programs/open_source/licensing/flossexception
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

struct LLGridData
{
	const char* mLabel;
  const char* mCodeName;
	const char* mName;
	const char* mLoginURI;
	const char* mHelperURI;
};

static LLGridData gGridInfo[GRID_INFO_COUNT] = 
{
	{ "None", "", "", "", "" },
	{ "SL Main Grid",
	  "Agni",
	  "util.agni.lindenlab.com", 
	  "https://login.agni.lindenlab.com/cgi-bin/login.cgi",
	  "https://secondlife.com/helpers/" },
	{ "SL Beta Grid",
	  "Aditi",
	  "util.aditi.lindenlab.com",
	  "https://login.aditi.lindenlab.com/cgi-bin/login.cgi",
	  "http://aditi-secondlife.webdev.lindenlab.com/helpers/" },
	{ "Local OpenSim",
	  "",
	  "localhost",
	  "http://127.0.0.1:9000",
	  "" },
	{ "Other", "", "", "", "" }
};

const EGridInfo DEFAULT_GRID_CHOICE = GRID_INFO_AGNI;


unsigned char gMACAddress[MAC_ADDRESS_BYTES];		/* Flawfinder: ignore */

LLViewerLogin::LLViewerLogin() :
	mGridChoice(DEFAULT_GRID_CHOICE)
{
}

void LLViewerLogin::setGridChoice(EGridInfo grid)
{	
	if(grid < 0 || grid >= GRID_INFO_COUNT)
	{
		llerrs << "Invalid grid index specified." << llendl;
	}

	if(mGridChoice != grid || gSavedSettings.getS32("ServerChoice") != grid)
	{
		mGridChoice = grid;
		if(GRID_INFO_LOCAL == mGridChoice)
		{
			mGridName = LOOPBACK_ADDRESS_STRING;
		}
		else if(GRID_INFO_OTHER == mGridChoice)
		{
			// *FIX:Mani - could this possibly be valid?
			mGridName = "other"; 
		}
		else
		{
			mGridName = gGridInfo[mGridChoice].mLabel;
		}

		gSavedSettings.setS32("ServerChoice", mGridChoice);
		gSavedSettings.setString("CustomServer", "");
	}
}

void LLViewerLogin::setGridChoice(const std::string& grid_name)
{
	// Set the grid choice based on a string.
	// The string can be:
	// - a grid label from the gGridInfo table 
	// - an ip address
	if(!grid_name.empty())
	{
		// find the grid choice from the user setting.
		int grid_index = GRID_INFO_NONE; 
		for(;grid_index < GRID_INFO_OTHER; ++grid_index)
		{
			if(0 == LLStringUtil::compareInsensitive(gGridInfo[grid_index].mLabel, grid_name))
			{
				// Founding a matching label in the list...
				setGridChoice((EGridInfo)grid_index);
				break;
			}
		}

		if(GRID_INFO_OTHER == grid_index)
		{
			// *FIX:MEP Can and should we validate that this is an IP address?
			mGridChoice = GRID_INFO_OTHER;
			mGridName = grid_name;
			gSavedSettings.setS32("ServerChoice", mGridChoice);
			gSavedSettings.setString("CustomServer", mGridName);
		}
	}
}

void LLViewerLogin::resetURIs()
{
	// Clear URIs when picking a new server
	gSavedSettings.setValue("CmdLineLoginURI", LLSD::emptyArray());
	gSavedSettings.setString("CmdLineHelperURI", "");
}

EGridInfo LLViewerLogin::getGridChoice() const
{
	return mGridChoice;
}

std::string LLViewerLogin::getGridLabel() const
{
	if(mGridChoice == GRID_INFO_NONE)
	{
		return "None";
	}
	else if(mGridChoice < GRID_INFO_OTHER)
	{
		return gGridInfo[mGridChoice].mLabel;
	}

	return mGridName;
}

std::string LLViewerLogin::getGridCodeName() const
{
	// Fall back to grid label if code name is empty.
	if( strcmp(gGridInfo[mGridChoice].mCodeName, "") == 0 )
	{
		return getGridLabel();
	}

	return gGridInfo[mGridChoice].mCodeName;
}

std::string LLViewerLogin::getKnownGridLabel(EGridInfo grid_index) const
{
	if(grid_index > GRID_INFO_NONE && grid_index < GRID_INFO_OTHER)
	{
		return gGridInfo[grid_index].mLabel;
	}
	return gGridInfo[GRID_INFO_NONE].mLabel;
}

void LLViewerLogin::getLoginURIs(std::vector<std::string>& uris) const
{
	// return the login uri set on the command line.
	LLControlVariable* c = gSavedSettings.getControl("CmdLineLoginURI");
	if(c)
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
		// If its a known grid choice, get the uri from the table,
		// else try the grid name.
		if(mGridChoice > GRID_INFO_NONE && mGridChoice < GRID_INFO_OTHER)
		{
			uris.push_back(gGridInfo[mGridChoice].mLoginURI);
		}
		else
		{
			uris.push_back(mGridName);
		}
	}
}

std::string LLViewerLogin::getHelperURI() const
{
	std::string helper_uri = gSavedSettings.getString("CmdLineHelperURI");
	if (helper_uri.empty())
	{
		// grab URI from selected grid
		if(mGridChoice > GRID_INFO_NONE && mGridChoice < GRID_INFO_OTHER)
		{
			helper_uri = gGridInfo[mGridChoice].mHelperURI;
		}

		if (helper_uri.empty())
		{
			// what do we do with unnamed/miscellaneous grids?
			// for now, operations that rely on the helper URI (currency/land purchasing) will fail
		}
	}
	return helper_uri;
}

bool LLViewerLogin::isInProductionGrid()
{
	// *NOTE:Mani This used to compare GRID_INFO_AGNI to gGridChoice,
	// but it seems that loginURI trumps that.
	std::vector<std::string> uris;
	getLoginURIs(uris);
	LLStringUtil::toLower(uris[0]);
	if((uris[0].find("agni") != std::string::npos))
	{
		return true;
	}

	return false;
}
