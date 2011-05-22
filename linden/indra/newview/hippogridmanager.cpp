/**
* @file hippogridmanager.cpp
* @brief stores grid information
*
* $LicenseInfo:firstyear=2011&license=viewergpl$
*
* Copyright (c) 2011
* Ported to Imprudence from the Hippo OpenSim Viewer by Jacek Antonelli
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

#include "hippogridmanager.h"

#include <cctype>

#include <stdtypes.h>
#include <lldir.h>
#include <lleconomy.h>
#include <llerror.h>
#include <llfile.h>
#include <llhttpclient.h>
#include "llmd5.h"
#include <llsdserialize.h>

#include "lltrans.h"
#include "llviewercontrol.h"
#include "llviewernetwork.h" // gMacAddress
#include "llweb.h"
#include "llxorcipher.h"	// saved password, MAC address

#include "hipporestrequest.h"


// ********************************************************************
// Global Variables

HippoGridManager* gHippoGridManager = 0;

HippoGridInfo HippoGridInfo::FALLBACK_GRIDINFO("");



// ********************************************************************
// ********************************************************************
// HippoGridInfo
// ********************************************************************
// ********************************************************************


// ********************************************************************
// Initialize

HippoGridInfo::HippoGridInfo(const std::string& gridNick) :
	mPlatform(PLATFORM_OPENSIM),
	mGridNick(gridNick),
	mGridName(LLStringUtil::null),
	mLoginURI(LLStringUtil::null),
	mLoginPage(LLStringUtil::null),
	mHelperURI(LLStringUtil::null),
	mWebSite(LLStringUtil::null),
	mSupportURL(LLStringUtil::null),
	mRegisterURL(LLStringUtil::null),
	mPasswordURL(LLStringUtil::null),
	mSearchURL(LLStringUtil::null),
	mFirstName(LLStringUtil::null),
	mLastName(LLStringUtil::null),
	mPasswordAvatar(LLStringUtil::null),
	mXmlState(XML_VOID),
	mVoiceConnector("SLVoice"),
	mRenderCompat(false),
	mMaxAgentGroups(-1),
	mCurrencySymbol("OS$"),
	mRealCurrencySymbol("US$"),
	mDirectoryFee(30),
	mUsername(LLStringUtil::null),
	mUsernameCompat(false)
{
	std::string nick = gridNick;
	mGridNick = sanitizeGridNick(nick);
}

void HippoGridInfo::setPlatform(Platform platform)
{
	mPlatform = platform;
	if (mPlatform == PLATFORM_SECONDLIFE)
	{
		mCurrencySymbol = "L$";
		mUsernameCompat = true;
	}
}

void HippoGridInfo::setPlatform(const std::string& platform)
{
	std::string tmp = platform;
	for (unsigned i=0; i<platform.size(); i++)
		tmp[i] = tolower(tmp[i]);

	if (tmp == "opensim") 
	{
		setPlatform(PLATFORM_OPENSIM);
	} 
	else if (tmp == "secondlife") 
	{
		setPlatform(PLATFORM_SECONDLIFE);
	} 
	else 
	{
		setPlatform(PLATFORM_OTHER);
		llwarns << "Unknown platform '" << platform << "'." << llendl;
	}
}

void HippoGridInfo::setLoginURI(const std::string& loginURI)
{
	std::string uri = loginURI;
	mLoginURI = sanitizeURI(uri);
}

void HippoGridInfo::setHelperURI(const std::string& helperURI)
{
	std::string uri = helperURI;
	mHelperURI = sanitizeURI(uri);
}


// ********************************************************************
// Grid Info

std::string HippoGridInfo::getSearchURL(SearchType ty, bool is_web) const
{
	// Don't worry about whether or not mSearchURL is empty here anymore -- MC
	if (is_web)
	{
		if (mPlatform == PLATFORM_SECONDLIFE) 
		{
			// Second Life defaults
			if (ty == SEARCH_ALL_EMPTY) 
			{
				return gSavedSettings.getString("SearchURLDefault");
			} 
			else if (ty == SEARCH_ALL_QUERY) 
			{
				return gSavedSettings.getString("SearchURLQuery");
			} 
			else if (ty == SEARCH_ALL_TEMPLATE) 
			{
				return gSavedSettings.getString("SearchURLSuffix2");
			} 
			else 
			{
				llinfos << "Illegal search URL type " << ty << llendl;
				return "";
			}
		}
		else if (!mSearchURL.empty())
		{
			// Search url sent to us in the login response
			if (ty == SEARCH_ALL_EMPTY) 
			{
				return (mSearchURL);
			} 
			else if (ty == SEARCH_ALL_QUERY) 
			{
				return (mSearchURL + "q=[QUERY]&s=[COLLECTION]&");
			} 
			else if (ty == SEARCH_ALL_TEMPLATE) 
			{
				return "lang=[LANG]&mat=[MATURITY]&t=[TEEN]&region=[REGION]&x=[X]&y=[Y]&z=[Z]&session=[SESSION]";
			} 
			else 
			{
				llinfos << "Illegal search URL type " << ty << llendl;
				return "";
			}
		}
		else
		{
			// OpenSim and other web search defaults
			if (ty == SEARCH_ALL_EMPTY) 
			{
				return gSavedSettings.getString("SearchURLDefaultOpenSim");
			} 
			else if (ty == SEARCH_ALL_QUERY) 
			{
				return gSavedSettings.getString("SearchURLQueryOpenSim");
			} 
			else if (ty == SEARCH_ALL_TEMPLATE) 
			{
				return gSavedSettings.getString("SearchURLSuffixOpenSim");
			} 
			else 
			{
				llinfos << "Illegal search URL type " << ty << llendl;
				return "";
			}
		} 
	}
	else 
	{
        // Use the old search all
        if (ty == SEARCH_ALL_EMPTY) 
		{
            return (mSearchURL + "panel=All&");
        } 
		else if (ty == SEARCH_ALL_QUERY) 
		{
            return (mSearchURL + "q=[QUERY]&s=[COLLECTION]&");
        } 
		else if (ty == SEARCH_ALL_TEMPLATE) 
		{
            return "lang=[LANG]&m=[MATURITY]&t=[TEEN]&region=[REGION]&x=[X]&y=[Y]&z=[Z]&session=[SESSION]";
        } 
		else 
		{
            llinfos << "Illegal search URL type " << ty << llendl;
            return "";
        }
    }
}


//static
void HippoGridInfo::onXmlElementStart(void* userData, const XML_Char* name, const XML_Char** atts)
{
	HippoGridInfo* self = (HippoGridInfo*)userData;
	if (strcasecmp(name, "gridnick") == 0)
		self->mXmlState = XML_GRIDNICK;
	else if (strcasecmp(name, "gridname") == 0)
		self->mXmlState = XML_GRIDNAME;
	else if (strcasecmp(name, "platform") == 0)
		self->mXmlState = XML_PLATFORM;
	else if ((strcasecmp(name, "login") == 0) || (strcasecmp(name, "loginuri") == 0))
		self->mXmlState = XML_LOGINURI;
	else if ((strcasecmp(name, "welcome") == 0) || (strcasecmp(name, "loginpage") == 0))
		self->mXmlState = XML_LOGINPAGE;
	else if ((strcasecmp(name, "economy") == 0) || (strcasecmp(name, "helperuri") == 0))
		self->mXmlState = XML_HELPERURI;
	else if ((strcasecmp(name, "about") == 0) || (strcasecmp(name, "website") == 0))
		self->mXmlState = XML_WEBSITE;
	else if ((strcasecmp(name, "help") == 0) || (strcasecmp(name, "support") == 0))
		self->mXmlState = XML_SUPPORT;
	else if ((strcasecmp(name, "register") == 0) || (strcasecmp(name, "account") == 0))
		self->mXmlState = XML_REGISTER;
	else if (strcasecmp(name, "password") == 0)
		self->mXmlState = XML_PASSWORD;
	else if (strcasecmp(name, "search") == 0)
		self->mXmlState = XML_SEARCH;
}

//static
void HippoGridInfo::onXmlElementEnd(void* userData, const XML_Char* name)
{
	HippoGridInfo* self = (HippoGridInfo*)userData;
	self->mXmlState = XML_VOID;
}

//static
void HippoGridInfo::onXmlCharacterData(void* userData, const XML_Char* s, S32 len)
{
	HippoGridInfo* self = (HippoGridInfo*)userData;
	switch (self->mXmlState) 
	{
		case XML_GRIDNICK: 
		{
			if (self->mGridNick == "") self->mGridNick.assign(s, len);
			self->mGridNick = sanitizeGridNick(self->mGridNick);
			break;
		}

		case XML_PLATFORM: 
		{
			std::string platform(s, len);
			self->setPlatform(platform); 
			break;
		}	

		case XML_LOGINURI: 
		{
			std::string loginuri(s, len);
			self->mLoginURI = sanitizeURI( loginuri );
			break;
		}

		case XML_HELPERURI: 
		{
			std::string helperuri(s, len);
			self->mHelperURI = sanitizeURI( helperuri );
			break;
		}

		case XML_SEARCH: 
		{
			self->mSearchURL.assign(s, len);
			//sanitizeQueryURL(mSearchURL);
			break;
		}

		case XML_GRIDNAME: self->mGridName.assign(s, len); break;
		case XML_LOGINPAGE: self->mLoginPage.assign(s, len); break;
		case XML_WEBSITE: self->mWebSite.assign(s, len); break;
		case XML_SUPPORT: self->mSupportURL.assign(s, len); break;
		case XML_REGISTER: self->mRegisterURL.assign(s, len); break;
		case XML_PASSWORD: self->mPasswordURL.assign(s, len); break;

		case XML_VOID: break;
	}
}


bool HippoGridInfo::retrieveGridInfo()
{
	if (mLoginURI == "") return false;

	// If last character in uri is not "/"
	std::string uri = mLoginURI;
	if (uri.compare(uri.length()-1, 1, "/") != 0) 
	{
	 	uri += '/';
	}
	std::string reply;
	S32 result = HippoRestRequest::getBlocking(uri + "get_grid_info", &reply);
	if (result != 200) return false;

	llinfos << "Received: " << reply << llendl;

	bool success = true;
	XML_Parser parser = XML_ParserCreate(0);
	XML_SetUserData(parser, this);
	XML_SetElementHandler(parser, onXmlElementStart, onXmlElementEnd);
	XML_SetCharacterDataHandler(parser, onXmlCharacterData);
	mXmlState = XML_VOID;
	if (!XML_Parse(parser, reply.data(), reply.size(), TRUE)) 
	{
		llwarns << "XML Parse Error: " << XML_ErrorString(XML_GetErrorCode(parser)) << llendl;
		success = false;
	}
	XML_ParserFree(parser);

	return success;
}

const std::string& HippoGridInfo::getGridName() const
{
	if (mGridName.empty())
	{
		return mGridNick;
	}
	return mGridName;
}

std::string HippoGridInfo::getUploadFee() const
{
	std::string fee;
	formatFee(fee, LLGlobalEconomy::Singleton::getInstance()->getPriceUpload(), true);
	return fee;
}

std::string HippoGridInfo::getGroupCreationFee() const
{
	std::string fee;
	formatFee(fee, LLGlobalEconomy::Singleton::getInstance()->getPriceGroupCreate(), false);
	return fee;
}

std::string HippoGridInfo::getDirectoryFee() const
{
	std::string fee;
	formatFee(fee, mDirectoryFee, true);
	if (fee != LLTrans::getString("hippo_label_free")) fee += "/" + LLTrans::getString("hippo_label_week");
	return fee;
}

void HippoGridInfo::formatFee(std::string &fee, S32 cost, bool showFree) const
{
	if (showFree && (cost == 0)) 
	{
		fee = LLTrans::getString("hippo_label_free");
	} 
	else 
	{
		fee = llformat("%s%d", getCurrencySymbol().c_str(), cost);
	}
}


void HippoGridInfo::setPassword(const std::string& unhashed_password)
{
	if (unhashed_password.empty())
	{
		mPasswordAvatar = "";
		return;
	}

	if (unhashed_password == mPasswordAvatar)
	{
		return;
	}

	std::string hashed_password("");

	// Max "actual" password length is 16 characters.
	// Hex digests are always 32 characters.
	if (unhashed_password.length() == 32)
	{
		hashed_password = unhashed_password;
	}
	else
	{
		// this is a user-entered plaintext password
		LLMD5 pass((unsigned char *)unhashed_password.c_str());
		char munged_password[MD5HEX_STR_SIZE];
		pass.hex_digest(munged_password);
		hashed_password = munged_password;
	}

	// need to fix the bug in this
	/*

	// Encipher with MAC address
	const S32 HASHED_LENGTH = 32;
	U8 buffer[HASHED_LENGTH+1];

	LLStringUtil::copy((char*)buffer, hashed_password.c_str(), HASHED_LENGTH+1);

	LLXORCipher cipher(gMACAddress, 6);
	cipher.encrypt(buffer, HASHED_LENGTH);

	mPasswordAvatar.assign((char*)buffer);
	*/
	mPasswordAvatar.assign(hashed_password);
}


std::string HippoGridInfo::getPassword() const
{
	// need to fix the bug in this
	/*
	if (mPasswordAvatar.empty() || mPasswordAvatar.length() == 32)
	{
		return mPasswordAvatar;
	}

	std::string hashed_password("");

	// UUID is 16 bytes, written into ASCII is 32 characters
	// without trailing \0
	const S32 HASHED_LENGTH = 32;
	U8 buffer[HASHED_LENGTH+1];

	LLStringUtil::copy((char*)buffer, mPasswordAvatar.c_str(), HASHED_LENGTH+1);
	
	// Decipher with MAC address
	LLXORCipher cipher(gMACAddress, 6);
	cipher.decrypt(buffer, HASHED_LENGTH);

	buffer[HASHED_LENGTH] = '\0';

	// Check to see if the mac address generated a bad hashed
	// password. It should be a hex-string or else the mac adress has
	// changed. This is a security feature to make sure that if you
	// get someone's grid_info.xml file, you cannot hack their account.
	if (is_hex_string(buffer, HASHED_LENGTH))
	{
		hashed_password.assign((char*)buffer);
	}

	return hashed_password;
	*/
	return mPasswordAvatar;
}

// ********************************************************************
// Static Helpers

// static
const char* HippoGridInfo::getPlatformString(Platform platform)
{
	static const char* platformStrings[PLATFORM_LAST] = 
	{
		"Other", "OpenSim", "SecondLife"
	};

	if ((platform < PLATFORM_OTHER) || (platform >= PLATFORM_LAST))
	{
		platform = PLATFORM_OTHER;
	}
	return platformStrings[platform];
}


// static
std::string HippoGridInfo::sanitizeGridNick(std::string &gridnick)
{
	std::string tmp;
	S32 size = gridnick.size();
	for (S32 i=0; i<size; i++) 
	{
		char c = gridnick[i];
		if ((c == '_') || isalnum(c)) 
		{
			tmp += tolower(c);
		} 
		else if (isspace(c)) 
		{
			tmp += "_";
		}
	}
	return tmp;
}

// static
std::string HippoGridInfo::sanitizeURI(std::string &uri)
{
	// Why don't we do this anymore? -- MC

	// if (uri.empty()) {
	// 	return "";
	// }

	// // If last character in uri is not "/"
	// // NOTE: This wrongly assumes that all URIs should end with "/"!
	// if (uri.compare(uri.length()-1, 1, "/") != 0) {
	// 	return uri + '/';
	// }

	return uri;
}


void HippoGridInfo::initFallback()
{
	FALLBACK_GRIDINFO.mGridNick = "localhost";
	FALLBACK_GRIDINFO.setPlatform(PLATFORM_OPENSIM);
	FALLBACK_GRIDINFO.setGridName("Local Host");
	FALLBACK_GRIDINFO.setLoginURI("http://127.0.0.1:9000/");
	FALLBACK_GRIDINFO.setHelperURI("http://127.0.0.1:9000/");
}



// ********************************************************************
// ********************************************************************
// HippoGridManager
// ********************************************************************
// ********************************************************************


// ********************************************************************
// Initialize

HippoGridManager::HippoGridManager() :
    mConnectedGrid(0),
    mDefaultGridsVersion(0),
	mCurrentGrid("osgrid"),
	mDefaultGrid("osgrid")
{
}

HippoGridManager::~HippoGridManager()
{
	cleanup();
}


void HippoGridManager::cleanup()
{
	std::map<std::string, HippoGridInfo*>::iterator it, end = mGridInfo.end();
	for (it=mGridInfo.begin(); it != end; ++it) 
	{
		delete it->second;
	}
	mGridInfo.clear();
}


void HippoGridManager::init()
{
	HippoGridInfo::initFallback();
	loadFromFile();

	// !!!### gSavedSettings.getControl("CmdLineLoginURI");
	// !!!### gSavedSettings.getString("CmdLineLoginPage");
	// !!!### gSavedSettings.getString("CmdLineHelperURI");
	// !!!### LLString::compareInsensitive(gGridInfo[grid_index].mLabel, grid_name.c_str()))
}


void HippoGridManager::discardAndReload()
{
	cleanup();
	loadFromFile();
}


// ********************************************************************
// Public Access

HippoGridInfo* HippoGridManager::getGrid(const std::string& grid) const
{
	std::map<std::string, HippoGridInfo*>::const_iterator it;
	it = mGridInfo.find(grid);
	if (it != mGridInfo.end()) 
	{
		return it->second;
	} 
	else 
	{
		return 0;
	}
}


HippoGridInfo* HippoGridManager::getConnectedGrid() const
{
	return (mConnectedGrid) ? mConnectedGrid : getCurrentGrid();
}


HippoGridInfo* HippoGridManager::getCurrentGrid() const
{
	HippoGridInfo* grid = getGrid(mCurrentGrid);
	if (grid) 
	{
		return grid;
	} 
	else 
	{
		return &HippoGridInfo::FALLBACK_GRIDINFO;
	}
}

const std::string& HippoGridManager::getCurrentGridNick() const
{
	if (mCurrentGrid.empty())
	{
		return mDefaultGrid;
	}
	return mCurrentGrid;
}

void HippoGridManager::addGrid(HippoGridInfo* grid)
{
	if (!grid) return;
	const std::string& nick = grid->getGridNick();
	if (nick == "") 
	{
		llwarns << "Ignoring to try adding grid with empty nick." << llendl;
		delete grid;
		return;
	}
	if (mGridInfo.find(nick) != mGridInfo.end()) 
	{
		llwarns << "Ignoring to try adding existing grid " << nick << '.' << llendl;
		delete grid;
		return;
	}
	mGridInfo[nick] = grid;
}


void HippoGridManager::deleteGrid(const std::string& grid)
{
	GridIterator it = mGridInfo.find(grid);
	if (it == mGridInfo.end()) 
	{
		llwarns << "Trying to delete non-existing grid " << grid << '.' << llendl;
		return;
	}

	mGridInfo.erase(it);

	llinfos << "Number of grids now: " << mGridInfo.size() << llendl;

	if (mGridInfo.empty()) llinfos << "Grid info map is empty." << llendl;
	if (grid == mDefaultGrid)
	{
		setDefaultGrid(LLStringUtil::null);  // sets first grid, if map not empty
	}
	if (grid == mCurrentGrid)
	{
		mCurrentGrid = mDefaultGrid;
	}
}


void HippoGridManager::setDefaultGrid(const std::string& grid)
{
	GridIterator it = mGridInfo.find(grid);
	if (it != mGridInfo.end()) 
	{
		mDefaultGrid = grid;
	} 
	else if (mGridInfo.find("secondlife") != mGridInfo.end()) 
	{
		mDefaultGrid = "secondlife";
	} 
	else if (!mGridInfo.empty()) 
	{
        mDefaultGrid = mGridInfo.begin()->first;
	} 
	else 
	{
		mDefaultGrid = "";
	}
}


void HippoGridManager::setCurrentGrid(const std::string& grid)
{
	GridIterator it = mGridInfo.find(grid);
	if (it != mGridInfo.end()) 
	{
		mCurrentGrid = grid;
	} 
	else if (!mGridInfo.empty()) 
	{
		llwarns << "Unknown grid '" << grid << "'. Setting to default grid." << llendl;
        mCurrentGrid = mDefaultGrid;
	}
}

bool HippoGridManager::hasGridNick(const std::string& grid_nick)
{
	for (GridIterator it = beginGrid(); it != endGrid(); ++it) 
	{
		if (grid_nick == it->second->getGridNick())
		{
			return true;
		}
	}
	return false;
}


// ********************************************************************
// Persistent Store

void HippoGridManager::loadFromFile()
{
	mDefaultGridsVersion = 0;
	// load user grid info
	parseFile(gDirUtilp->getExpandedFilename(LL_PATH_USER_SETTINGS, "grid_info.xml"), false);
	// merge default grid info, if newer. Force load, if list of grids is empty.
	parseFile(gDirUtilp->getExpandedFilename(LL_PATH_APP_SETTINGS, "default_grids.xml"), !mGridInfo.empty());
	// merge grid info from web site, if newer. Force load, if list of grids is empty.
	if (gSavedSettings.getBOOL("CheckForGridUpdates"))
	{
		std::string update_list = gSavedSettings.getString("GridUpdateList");
		if (!update_list.empty())
		{
			parseURL(update_list, !mGridInfo.empty());
		}
		else
		{
			llwarns << "\"CheckForGridUpdates\" is set to true, but \"GridUpdateList\" contains no URL to fetch the grid info from. Skipping." << llendl;
		}
	}

	std::string last_grid = gSavedSettings.getString("LastSelectedGrid");
	if (last_grid.empty()) last_grid = gSavedSettings.getString("DefaultGrid");
	setDefaultGrid(last_grid);
	setCurrentGrid(last_grid);
}


void HippoGridManager::parseURL(const std::string url, bool mergeIfNewer)
{
	llinfos << "Loading grid info from '" << url << "'." << llendl;

	// query update server
	std::string escaped_url = LLWeb::escapeURL(url);
	LLSD response = LLHTTPClient::blockingGet(url);

	// check response, return on error
	S32 status = response["status"].asInteger();
	if ((status != 200) || !response["body"].isArray()) 
	{
		llinfos << "GridInfo Update failed (" << status << "): "
			<< (response["body"].isString()? response["body"].asString(): "<unknown error>")
			<< llendl;
		return;
	}

	LLSD gridInfo = response["body"];
	parseData(gridInfo, mergeIfNewer);
}

void HippoGridManager::parseFile(const std::string& fileName, bool mergeIfNewer)
{
	llifstream infile;
	infile.open(fileName.c_str());
	if (!infile.is_open()) 
	{
		llwarns << "Cannot find grid info file " << fileName << " to load." << llendl;
		return;
	}

	LLSD gridInfo;
	if (LLSDSerialize::fromXML(gridInfo, infile) <= 0) 
	{
		llwarns << "Unable to parse grid info file " << fileName << '.' << llendl;		
		return;
	}

	llinfos << "Loading grid info file " << fileName << '.' << llendl;
	parseData(gridInfo, mergeIfNewer);
}


void HippoGridManager::parseData(LLSD &gridInfo, bool mergeIfNewer)
{
	if (mergeIfNewer) 
	{
		LLSD::array_const_iterator it, end = gridInfo.endArray();
		for (it = gridInfo.beginArray(); it != end; ++it) 
		{
			LLSD gridMap = *it;
			if (gridMap.has("default_grids_version")) 
			{
				S32 version = gridMap["default_grids_version"];
				if (version <= mDefaultGridsVersion) return;
				else break;
			}
		}
		if (it == end) 
		{
			llwarns << "Grid data has no version number." << llendl;
			return;
		}
	}

	llinfos << "Loading grid data." << llendl;

	LLSD::array_const_iterator it, end = gridInfo.endArray();
	for (it = gridInfo.beginArray(); it != end; ++it) 
	{
		LLSD gridMap = *it;
		if (gridMap.has("default_grids_version")) 
		{
			mDefaultGridsVersion = gridMap["default_grids_version"];
		} 
		else if (gridMap.has("gridnick") && gridMap.has("loginuri")) 
		{
			std::string gridnick = gridMap["gridnick"];
			HippoGridInfo* grid;
			GridIterator it = mGridInfo.find(gridnick);
			bool newGrid = (it == mGridInfo.end());
			if (newGrid) 
			{
				// create new grid info
				grid = new HippoGridInfo(gridnick);
			} 
			else 
			{
				// update existing grid info
				grid = it->second;
			}
			grid->setLoginURI(gridMap["loginuri"]);
			if (gridMap.has("platform")) grid->setPlatform(gridMap["platform"]);
			if (gridMap.has("gridname")) grid->setGridName(gridMap["gridname"]);
			if (gridMap.has("loginpage")) grid->setLoginPage(gridMap["loginpage"]);
			if (gridMap.has("helperuri")) grid->setHelperURI(gridMap["helperuri"]);
			if (gridMap.has("website")) grid->setWebSite(gridMap["website"]);
			if (gridMap.has("support")) grid->setSupportURL(gridMap["support"]);
			if (gridMap.has("register")) grid->setRegisterURL(gridMap["register"]);
			if (gridMap.has("password")) grid->setPasswordURL(gridMap["password"]);
			if (gridMap.has("search")) grid->setSearchURL(gridMap["search"]);
			if (gridMap.has("render_compat")) grid->setRenderCompat(gridMap["render_compat"]);
			if (gridMap.has("firstname")) grid->setFirstName(gridMap["firstname"]);
			if (gridMap.has("lastname")) grid->setLastName(gridMap["lastname"]);
			if (gridMap.has("avatarpassword")) grid->setPassword(gridMap["avatarpassword"]);
			if (gridMap.has("username")) grid->setUsername(gridMap["username"]);
			if (gridMap.has("username_compat")) grid->setUsernameCompat(gridMap["username_compat"]);
			if (newGrid) addGrid(grid);
		}
	}
}


void HippoGridManager::saveFile()
{
	// save default grid to client settings
	gSavedSettings.setString("DefaultGrid", mDefaultGrid);

	// build LLSD
	LLSD gridInfo;
	gridInfo[0]["default_grids_version"] = mDefaultGridsVersion;

	// add grids
	S32 i = 1;
	GridIterator it, end = mGridInfo.end();
	for (it = mGridInfo.begin(); it != end; ++it, i++) 
	{
		HippoGridInfo* grid = it->second;
		gridInfo[i]["gridnick"] = grid->getGridNick();
		gridInfo[i]["platform"] = HippoGridInfo::getPlatformString(grid->getPlatform());
		gridInfo[i]["gridname"] = grid->getGridName();
		gridInfo[i]["loginuri"] = grid->getLoginURI();
		gridInfo[i]["loginpage"] = grid->getLoginPage();
		gridInfo[i]["helperuri"] = grid->getHelperURI();
		gridInfo[i]["website"] = grid->getWebSite();
		gridInfo[i]["support"] = grid->getSupportURL();
		gridInfo[i]["register"] = grid->getRegisterURL();
		gridInfo[i]["password"] = grid->getPasswordURL();
		gridInfo[i]["firstname"] = grid->getFirstName();
		gridInfo[i]["lastname"] = grid->getLastName();
		gridInfo[i]["avatarpassword"] = grid->getPassword();
		
		gridInfo[i]["search"] = grid->getSearchURL();
		gridInfo[i]["render_compat"] = grid->isRenderCompat();

		gridInfo[i]["username"] = grid->getUsername();
		gridInfo[i]["username_compat"] = grid->isUsernameCompat();
	}

	// write client grid info file
	std::string fileName = gDirUtilp->getExpandedFilename(LL_PATH_USER_SETTINGS, "grid_info.xml");
	llofstream file;
	file.open(fileName.c_str());
	if (file.is_open()) 
	{
		LLSDSerialize::toPrettyXML(gridInfo, file);
		file.close();
		llinfos << "Saved grids to " << fileName << llendl;
	} 
	else 
	{
		llerrs << "Unable to open grid info file: " << fileName << llendl;
	}
}
