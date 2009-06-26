#ifndef __HIPPO_GRID_MANAGER_H__
#define __HIPPO_GRID_MANAGER_H__


#include <map>
#include <string>

#ifndef XML_STATIC
#define XML_STATIC
#endif
#include <expat/expat.h>


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

    explicit HippoGridInfo(const std::string &gridNick);

    Platform           getPlatform()     const { return mPlatform;     }
    const std::string &getGridNick()     const { return mGridNick;     }
    const std::string &getGridName()     const { return mGridName;     }
    const std::string &getLoginUri()     const { return mLoginUri;     }
    const std::string &getLoginPage()    const { return mLoginPage;    }
    const std::string &getHelperUri()    const { return mHelperUri;    }
    const std::string &getWebSite()      const { return mWebSite;      }
    const std::string &getSupportUrl()   const { return mSupportUrl;   }
    const std::string &getRegisterUrl()  const { return mRegisterUrl;  }
    const std::string &getPasswordUrl()  const { return mPasswordUrl;  }
    const std::string &getSearchUrl()    const { return mSearchUrl;    }
    const std::string &getFirstName()    const { return mFirstName;    }
    const std::string &getLastName()     const { return mLastName;     }
    const std::string &getAvatarPassword()    const { return mAvatarPassword;    }
    std::string getSearchUrl(SearchType ty) const;
    bool isRenderCompat() const { return mRenderCompat; }

    const std::string &getCurrencySymbol() const { return mCurrencySymbol; }
    const std::string &getRealCurrencySymbol() const { return mRealCurrencySymbol; }
    std::string getUploadFee() const;
    std::string getGroupCreationFee() const;
    std::string getDirectoryFee() const;

    bool isOpenSimulator() const { return (mPlatform == PLATFORM_OPENSIM   ); }
    bool isSecondLife()    const { return (mPlatform == PLATFORM_SECONDLIFE); }

    void setPlatform (const std::string &platform);
    void setPlatform (Platform platform);
    void setGridName (const std::string &gridName)  { mGridName = gridName;   }
    void setLoginUri (const std::string &loginUri)  { mLoginUri = loginUri; cleanUpUri(mLoginUri); }
    void setLoginPage(const std::string &loginPage) { mLoginPage = loginPage; }
    void setHelperUri(const std::string &helperUri) { mHelperUri = helperUri; cleanUpUri(mHelperUri); }
    void setWebSite  (const std::string &website)   { mWebSite = website;     }
    void setSupportUrl(const std::string &url)      { mSupportUrl = url;      }
    void setRegisterUrl(const std::string &url)     { mRegisterUrl = url;     }
    void setPasswordUrl(const std::string &url)     { mPasswordUrl = url;     }
    void setSearchUrl(const std::string &url)       { mSearchUrl = url;       }
    void setRenderCompat(bool compat)               { mRenderCompat = compat; }
    void setFirstName(const std::string &firstName) { mFirstName = firstName;       } //aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    void setLastName(const std::string &lastName)   { mLastName = lastName;        }
    void setAvatarPassword(const std::string &avatarPassword) { mAvatarPassword = avatarPassword;  }

    void setCurrencySymbol(const std::string &sym) { mCurrencySymbol = sym.substr(0, 3); }
    void setRealCurrencySymbol(const std::string &sym) { mRealCurrencySymbol = sym.substr(0, 3); }
    void setDirectoryFee(int fee) { mDirectoryFee = fee; }

    bool retrieveGridInfo();

    static const char *getPlatformString(Platform platform);
    static void cleanUpGridNick(std::string &gridnick);

    static HippoGridInfo FALLBACK_GRIDINFO;
    static void initFallback();

private:
    Platform mPlatform;
    std::string mGridNick;
    std::string mGridName;
    std::string mLoginUri;
    std::string mLoginPage;
    std::string mHelperUri;
    std::string mWebSite;
    std::string mSupportUrl;
    std::string mRegisterUrl;
    std::string mPasswordUrl;
    std::string mSearchUrl;
    std::string mFirstName;
    std::string mLastName;
    std::string mAvatarPassword;
    bool mRenderCompat;

    std::string mCurrencySymbol;
    std::string mRealCurrencySymbol;
    int mDirectoryFee;

    // for parsing grid info XML
    enum XmlState {
        XML_VOID, XML_GRIDNICK, XML_PLATFORM, XML_GRIDNAME,
        XML_LOGINURI, XML_LOGINPAGE, XML_HELPERURI,
        XML_WEBSITE, XML_SUPPORT, XML_REGISTER, XML_PASSWORD, XML_SEARCH
    };
    XmlState mXmlState;

    static void cleanUpUri(std::string &uri);
    void formatFee(std::string &fee, int cost, bool showFree) const;

    static void onXmlElementStart(void *userData, const XML_Char *name, const XML_Char **atts);
    static void onXmlElementEnd(void *userData, const XML_Char *name);
    static void onXmlCharacterData(void *userData, const XML_Char *s, int len);
};


class HippoGridManager
{
public:
    HippoGridManager();
    ~HippoGridManager();

    void init();
    void saveFile();
    void discardAndReload();

    HippoGridInfo *getGrid(const std::string &grid) const;
    HippoGridInfo *getConnectedGrid() const { return (mConnectedGrid)? mConnectedGrid: getCurrentGrid(); }
    HippoGridInfo *getCurrentGrid() const;
    const std::string &getDefaultGridNick() const { return mDefaultGrid; }
    const std::string &getCurrentGridNick() const { return mCurrentGrid; }

    void setDefaultGrid(const std::string &grid);
    void setCurrentGrid(const std::string &grid);
    void setCurrentGridAsConnected() { mConnectedGrid = getCurrentGrid(); }

    void addGrid(HippoGridInfo *grid);
    void deleteGrid(const std::string &grid);

    typedef std::map<std::string, HippoGridInfo*>::iterator GridIterator;
    GridIterator beginGrid() { return mGridInfo.begin(); }
    GridIterator endGrid() { return mGridInfo.end(); }

private:
    std::map<std::string, HippoGridInfo*> mGridInfo;
    std::string mDefaultGrid;
    std::string mCurrentGrid;
    HippoGridInfo *mConnectedGrid;
    int mDefaultGridsVersion;

    void cleanup();
    void loadFromFile();
    void parseFile(const std::string &fileName, bool mergeIfNewer);
    void parseUrl(const char *url, bool mergeIfNewer);
    void parseData(LLSD &gridInfo, bool mergeIfNewer);
};


extern HippoGridManager *gHippoGridManager;


#endif
