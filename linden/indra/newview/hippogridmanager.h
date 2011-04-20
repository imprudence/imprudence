/**
* @file hippogridmanager.h
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

#ifndef __HIPPO_GRID_MANAGER_H__
#define __HIPPO_GRID_MANAGER_H__


#include <map>
#include <string>

#ifndef XML_STATIC
#define XML_STATIC
#endif

#ifdef LL_STANDALONE
#include <expat.h>
#else
#include "expat/expat.h"
#endif

class LLSD;


class HippoGridInfo
{
public:
	enum Platform {
		PLATFORM_OTHER = 0,
		PLATFORM_OPENSIM,
		PLATFORM_SECONDLIFE,
		PLATFORM_LAST
	};
	enum SearchType {
		SEARCH_ALL_EMPTY,
		SEARCH_ALL_QUERY,
		SEARCH_ALL_TEMPLATE
	};

	explicit HippoGridInfo(const std::string& gridNick);

	Platform getPlatform()							  { return mPlatform; }
	bool isOpenSimulator()						const { return (mPlatform == PLATFORM_OPENSIM); }
	bool isSecondLife()							const { return (mPlatform == PLATFORM_SECONDLIFE); }
	bool isRenderCompat()						const { return mRenderCompat; }
	// currently only SecondLife grids support username-style logins but Aurora is working on implementing it
	bool isUsernameCompat()						const { return mUsernameCompat; }

	const std::string& getGridNick()			const { return mGridNick; }
	// returns grid nick if no grid name is available
	const std::string& getGridName()			const;
	const std::string& getLoginURI()			const { return mLoginURI; }
	const std::string& getLoginPage()			const { return mLoginPage; }
	const std::string& getHelperURI()			const { return mHelperURI; }
	const std::string& getWebSite()				const { return mWebSite; }
	const std::string& getSupportURL()			const { return mSupportURL; }
	const std::string& getRegisterURL()			const { return mRegisterURL; }
	const std::string& getPasswordURL()			const { return mPasswordURL; }
	// Returns the url base used for the Web Search tab
	const std::string& getSearchURL()			const { return mSearchURL; }
	const std::string& getFirstName()			const { return mFirstName; }
	const std::string& getLastName()			const { return mLastName; }
	const std::string& getUsername()			const { return mUsername; }
	std::string getPassword()				const;
	const std::string& getVoiceConnector()		const { return mVoiceConnector; }
	S32 getMaxAgentGroups()						const { return mMaxAgentGroups; }
	const std::string& getCurrencySymbol()		const { return mCurrencySymbol; }
	const std::string& getRealCurrencySymbol()	const { return mRealCurrencySymbol; }
	std::string getUploadFee()					const;
	std::string getGroupCreationFee()			const;
	std::string getDirectoryFee()				const;
	std::string getSearchURL(SearchType ty, bool is_web) const;

	void setPlatform (const std::string& platform);
	void setPlatform (Platform platform);
	void setGridName (const std::string& gridName)				{ mGridName = gridName; }
	void setLoginURI (const std::string& loginURI);
	void setLoginPage(const std::string& loginPage)				{ mLoginPage = loginPage; }
	void setHelperURI(const std::string& helperURI);
	void setWebSite  (const std::string& website)				{ mWebSite = website; }
	void setSupportURL(const std::string& url)					{ mSupportURL = url; }
	void setRegisterURL(const std::string& url)					{ mRegisterURL = url; }
	void setPasswordURL(const std::string& url)					{ mPasswordURL = url; }
	// sets the url base used for the Web Search tab
	void setSearchURL(const std::string& url)					{ mSearchURL = url; }
	void setRenderCompat(bool compat)							{ mRenderCompat = compat; }
	void setMaxAgentGroups(S32 max)								{ mMaxAgentGroups = max;   }
	void setFirstName(const std::string& firstName)				{ mFirstName = firstName; }
	void setLastName(const std::string& lastName)				{ mLastName = lastName; }
	void setPassword(const std::string& unhashed_password);
	void setVoiceConnector(const std::string& vc)				{ mVoiceConnector = vc; }
	void setCurrencySymbol(const std::string& sym)				{ mCurrencySymbol = sym.substr(0, 3); }
	void setRealCurrencySymbol(const std::string& sym)			{ mRealCurrencySymbol = sym.substr(0, 3); }
	void setDirectoryFee(U32 fee)								{ mDirectoryFee = fee; }
	void setUsername(const std::string& username)				{ mUsername = username; }
	void setUsernameCompat(bool compat)							{ mUsernameCompat = compat; }

	bool retrieveGridInfo();

	static const char* getPlatformString(Platform platform);
	static std::string sanitizeGridNick(std::string &gridnick);

	static HippoGridInfo FALLBACK_GRIDINFO;
	static void initFallback();

private:
	Platform	mPlatform;
	std::string mGridNick;
	std::string mGridName;
	std::string mLoginURI;
	std::string mLoginPage;
	std::string mHelperURI;
	std::string mWebSite;
	std::string mSupportURL;
	std::string mRegisterURL;
	std::string mPasswordURL;
	std::string mSearchURL;
	std::string mVoiceConnector;
	std::string mFirstName;
	std::string mLastName;
	std::string mPasswordAvatar;
	bool		mRenderCompat;
	S32			mMaxAgentGroups;

	std::string mUsername;
	bool		mUsernameCompat;

	std::string mCurrencySymbol;
	std::string mRealCurrencySymbol;
	U32 mDirectoryFee;

	// for parsing grid info XML
	enum XmlState 
	{
		XML_VOID, XML_GRIDNICK, XML_PLATFORM, XML_GRIDNAME,
		XML_LOGINURI, XML_LOGINPAGE, XML_HELPERURI,
		XML_WEBSITE, XML_SUPPORT, XML_REGISTER, XML_PASSWORD, XML_SEARCH
	};
	XmlState mXmlState;

	static std::string sanitizeURI(std::string &uri);
	void formatFee(std::string &fee, S32 cost, bool showFree) const;

	static void onXmlElementStart(void* userData, const XML_Char* name, const XML_Char** atts);
	static void onXmlElementEnd(void* userData, const XML_Char* name);
	static void onXmlCharacterData(void* userData, const XML_Char* s, S32 len);
};


class HippoGridManager
{
public:
	HippoGridManager();
	~HippoGridManager();

	void init();
	void saveFile();
	void discardAndReload();

	HippoGridInfo* getGrid(const std::string& grid) const;
	HippoGridInfo* getConnectedGrid()               const;
	HippoGridInfo* getCurrentGrid()                 const;
	const std::string& getDefaultGridNick()         const { return mDefaultGrid; }
	const std::string& getCurrentGridNick()         const;

	void setDefaultGrid(const std::string& grid);
	void setCurrentGrid(const std::string& grid);
	void setCurrentGridAsConnected()				      { mConnectedGrid = getCurrentGrid(); }

	void addGrid(HippoGridInfo* grid);
	void deleteGrid(const std::string& grid);

	typedef std::map<std::string, HippoGridInfo*>::iterator GridIterator;
	GridIterator beginGrid()							  { return mGridInfo.begin(); }
	GridIterator endGrid()								  { return mGridInfo.end(); }

	// returns true if the grid manager knows of a nickname
	bool hasGridNick(const std::string& grid_nick);

private:
	std::map<std::string, HippoGridInfo*> mGridInfo;
	std::string mDefaultGrid;
	std::string mCurrentGrid;
	HippoGridInfo* mConnectedGrid;
	S32 mDefaultGridsVersion;

	void cleanup();
	void loadFromFile();
	void parseFile(const std::string& fileName, bool mergeIfNewer);
	void parseURL(const std::string url, bool mergeIfNewer);
	void parseData(LLSD &gridInfo, bool mergeIfNewer);
};


extern HippoGridManager* gHippoGridManager;


#endif
