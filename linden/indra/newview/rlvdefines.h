#ifndef RLV_DEFINES_H
#define RLV_DEFINES_H

// ============================================================================
// Extensions
//

// Comment out if you don't want the Advanced / RLVa menu (may prevent enabling some extensions or experimental features - see below)
#define RLV_ADVANCED_MENU
// Comment out if you provide your own way to enable/disable RLVa
#define RLV_ADVANCED_TOGGLE_RLVA

// Provides access to "advanced" feature through the RLVa debug menu
#define RLV_EXTENSION_ENABLE_WEAR			// "Enable Wear"
#define RLV_EXTENSION_FLOATER_RESTRICTIONS	// Enables the Advanced / RLVa / Restrictions... floater
#define RLV_EXTENSION_HIDELOCKED			// "Hide locked layers", "Hide locked attachments" and "Hide locked inventory"

// Extensions
#define RLV_EXTENSION_CMD_GETSETDEBUG_EX	// Extends the debug variables accessible through @getdebug_xxx/@setdebug_xxx
#define RLV_EXTENSION_CMD_FINDFOLDERS		// @findfolders:<option>=<channel> - @findfolder with multiple results
#define RLV_EXTENSION_FLAG_NOSTRIP			// Layers and attachments marked as "nostrip" are exempt from @detach/@remoutfit
#define RLV_EXTENSION_STARTLOCATION			// Reenables "Start Location" at login if not @tploc=n or @unsit=n restricted at last logoff
#define RLV_EXPERIMENTAL					// Enables/disables experimental features en masse

// Experimental features
#ifdef RLV_EXPERIMENTAL
	// Stable (will mature to RLV_EXTENSION_XXX in next release if no bugs are found)
	#define RLV_EXPERIMENTAL_FARTOUCH_FEEDBACK		// Enables "cleaner" UI responses when fartouch blocks something

	// Under testing (stable, but requires further testing - safe for public release but may be quirky)
	#define RLV_EXPERIMENTAL_FIRSTUSE				// Enables a number of "first use" popups

	// Under development (don't include in public release)
	#if LL_RELEASE_WITH_DEBUG_INFO || LL_DEBUG
	#endif // LL_RELEASE_WITH_DEBUG_INFO || LL_DEBUG
#endif // RLV_EXPERIMENTAL

// ============================================================================
// Defines
//

// Version of the specifcation we support
const S32 RLV_VERSION_MAJOR = 1;
const S32 RLV_VERSION_MINOR = 22;
const S32 RLV_VERSION_PATCH = 0;
const S32 RLV_VERSION_BUILD = 0;

// Implementation version
const S32 RLVa_VERSION_MAJOR = 1;
const S32 RLVa_VERSION_MINOR = 0;
const S32 RLVa_VERSION_PATCH = 5;
const S32 RLVa_VERSION_BUILD = 4;

// The official viewer version we're patching against
#define RLV_MAKE_TARGET(x, y, z)	((x << 16) | (y << 8) | z)
#define RLV_TARGET					RLV_MAKE_TARGET(1, 22, 11)

// Defining these makes it easier if we ever need to change our tag
#define RLV_WARNS	LL_WARNS("RLV")
#define RLV_INFOS	LL_INFOS("RLV")
#define RLV_DEBUGS	LL_DEBUGS("RLV")
#define RLV_ENDL	LL_ENDL

#if LL_RELEASE_WITH_DEBUG_INFO || LL_DEBUG
	// Turn on extended debugging information
	#define RLV_DEBUG
	// Make sure we halt execution on errors
	#define RLV_ERRS		LL_ERRS("RLV")
	// Keep our asserts separate from LL's
	#define RLV_ASSERT(f)	if (!(f)) RLV_ERRS << "ASSERT (" << #f << ")" << RLV_ENDL;
	// Uncomment to enable the Advanced / RLVa / Unit Tests menu (non-public)
	//#define RLV_DEBUG_TESTS
#else
	// Uncomment if you want extended debugging information on release builds
	//#define RLV_DEBUG
	// Don't halt execution on errors in release
	#define RLV_ERRS  LL_WARNS("RLV")
	// We don't want to check assertions in release builds
	#define RLV_ASSERT(f)
#endif // LL_RELEASE_WITH_DEBUG_INFO || LL_DEBUG

#define RLV_ROOT_FOLDER					"#RLV"
#define RLV_CMD_PREFIX					'@'
#define RLV_PUTINV_PREFIX				"#RLV/~"
#define RLV_SETROT_OFFSET				F_PI_BY_TWO		// @setrot is off by 90° with the rest of SL

#define RLV_FOLDER_FLAG_NOSTRIP			"nostrip"
#define RLV_FOLDER_PREFIX_HIDDEN		'.'
#define RLV_FOLDER_PREFIX_PUTINV    	'~'

// ============================================================================
// Enumeration declarations
//

// NOTE: any changes to this enumeration should be reflected in initLookupTable()
enum ERlvBehaviour {
	RLV_BHVR_VERSION = 0,			// "version"
	RLV_BHVR_DETACH,				// "detach"
	RLV_BHVR_SENDCHAT,				// "sendchat"
	RLV_BHVR_EMOTE,					// "emote"
	RLV_BHVR_CHATSHOUT,				// "chatshout"
	RLV_BHVR_CHATNORMAL,			// "chatnormal"
	RLV_BHVR_CHATWHISPER,			// "chatwhisper"
	RLV_BHVR_REDIRCHAT,				// "redirchat"
	RLV_BHVR_REDIREMOTE,			// "rediremote"
	RLV_BHVR_SENDIM,				// "sendim"
	RLV_BHVR_RECVCHAT,				// "recvchat"
	RLV_BHVR_RECVEMOTE,				// "recvemote"
	RLV_BHVR_RECVIM,				// "recvim"
	RLV_BHVR_TPLM,					// "tplm"
	RLV_BHVR_TPLOC,					// "tploc"
	RLV_BHVR_TPLURE,				// "tplure"
	RLV_BHVR_SITTP,					// "sittp"
	RLV_BHVR_EDIT,					// "edit"
	RLV_BHVR_REZ,					// "rez"
	RLV_BHVR_ADDOUTFIT,				// "addoutfit"
	RLV_BHVR_REMOUTFIT,				// "remoutfit"
	RLV_BHVR_GETOUTFIT,				// "getoutfit"
	RLV_BHVR_ADDATTACH,				// "addattach"
	RLV_BHVR_REMATTACH,				// "remattach"
	RLV_BHVR_GETATTACH,				// "getattach"
	RLV_BHVR_SHOWINV,				// "showinv"
	RLV_BHVR_VIEWNOTE,				// "viewnote"
	RLV_BHVR_UNSIT,					// "unsit"
	RLV_BHVR_SIT,					// "sit"
	RLV_BHVR_SENDCHANNEL,			// "sendchannel"
	RLV_BHVR_GETSTATUS,				// "getstatus"
	RLV_BHVR_GETSTATUSALL,			// "getstatusall"
	RLV_BHVR_GETINV,				// "getinv"
	RLV_BHVR_GETINVWORN,			// "getinvworn"
	RLV_BHVR_FINDFOLDER,			// "findfolder"
	RLV_BHVR_FINDFOLDERS,			// "findfolders"
	RLV_BHVR_ATTACH,				// "attach"
	RLV_BHVR_ATTACHALL,				// "attachall"
	RLV_BHVR_DETACHALL,				// "detachall"
	RLV_BHVR_GETPATH,				// "getpath"
	RLV_BHVR_ATTACHTHIS,			// "attachthis"
	RLV_BHVR_ATTACHALLTHIS,			// "attachallthis"
	RLV_BHVR_DETACHTHIS,			// "detachthis"
	RLV_BHVR_DETACHALLTHIS,			// "detachallthis"
	RLV_BHVR_FARTOUCH,				// "fartouch"
	RLV_BHVR_SHOWWORLDMAP,			// "showworldmap"
	RLV_BHVR_SHOWMINIMAP,			// "showminimap"
	RLV_BHVR_SHOWLOC,				// "showloc"
	RLV_BHVR_TPTO,					// "tpto"
	RLV_BHVR_ACCEPTTP,				// "accepttp"
	RLV_BHVR_ACCEPTPERMISSION,		// "acceptpermission"
	RLV_BHVR_SHOWNAMES,				// "shownames"
	RLV_BHVR_FLY,					// "fly"
	RLV_BHVR_GETSITID,				// "getsitid"
	RLV_BHVR_SETDEBUG,				// "setdebug"
	RLV_BHVR_SETENV,				// "setenv"
	RLV_BHVR_DETACHME,				// "detachme"
	RLV_BHVR_SHOWHOVERTEXTALL,		// "showhovertextall"
	RLV_BHVR_SHOWHOVERTEXTWORLD,	// "showhovertextworld"
	RLV_BHVR_SHOWHOVERTEXTHUD,		// "showhovertexthud"
	RLV_BHVR_SHOWHOVERTEXT,			// "showhovertext"
	RLV_BHVR_NOTIFY,				// "notify"
	RLV_BHVR_DEFAULTWEAR,			// "defaultwear"
	RLV_BHVR_VERSIONNUM,			// "versionnum"
	RLV_BHVR_PERMISSIVE,			// "permissive"
	RLV_BHVR_VIEWSCRIPT,			// "viewscript"
	RLV_BHVR_VIEWTEXTURE,			// "viewtexture"

	RLV_BHVR_COUNT,
	RLV_BHVR_UNKNOWN
};

enum ERlvParamType {
	RLV_TYPE_UNKNOWN,
	RLV_TYPE_ADD,					// <param> == "n"|"add"
	RLV_TYPE_REMOVE,				// <param> == "y"|"rem"
	RLV_TYPE_FORCE,					// <param> == "force"
	RLV_TYPE_REPLY,					// <param> == <number>
	RLV_TYPE_CLEAR
};

enum ERlvCmdRet {
	RLV_RET_NOERROR,				// Command executed succesfully
	RLV_RET_RETAINED,				// Command was retained
	RLV_RET_DISABLED,				// Command is disabled (by user)
	RLV_RET_FAILED,					// Command failed (general failure)
	RLV_RET_FAILED_SYNTAX,			// Command failed (syntax error)
	RLV_RET_FAILED_UNSET,			// Command failed (unset restriction)
	RLV_RET_FAILED_DUPLICATE,		// Command failed (duplicate)
	RLV_RET_FAILED_OPTION,			// Command failed (invalid option)
	RLV_RET_FAILED_PARAM,			// Command failed (invalid param)
	RLV_RET_UNKNOWN					// Command unkown
};

enum ERlvExceptionCheck {
	RLV_CHECK_PERMISSIVE,			// Exception can be set by any object
	RLV_CHECK_STRICT,				// Exception must be set by all objects holding the restriction
	RLV_CHECK_DEFAULT				// Permissive or strict will be determined by currently enforced restrictions
};

enum ERlvLockMask {
	RLV_LOCK_ADD    = 0x01,
	RLV_LOCK_REMOVE = 0x02,
	RLV_LOCK_ANY    = RLV_LOCK_ADD | RLV_LOCK_REMOVE
};

// ============================================================================
// Settings

#define RLV_SETTING_MAIN				"RestrainedLife"
#define RLV_SETTING_DEBUG				"RestrainedLifeDebug"
#define RLV_SETTING_NOSETENV			"RestrainedLifeNoSetEnv"
#define RLV_SETTING_FORBIDGIVETORLV		"RestrainedLifeForbidGiveToRLV"

#define RLV_SETTING_ENABLEWEAR			"RLVaEnableWear"
#define RLV_SETTING_ENABLELEGACYNAMING  "RLVaEnableLegacyNaming"
#define RLV_SETTING_HIDELOCKEDLAYER		"RLVaHideLockedLayers"
#define RLV_SETTING_HIDELOCKEDATTACH	"RLVaHideLockedAttachments"
#define RLV_SETTING_HIDELOCKEDINVENTORY	"RLVaHideLockedInventory"
#define RLV_SETTING_LOGINLASTLOCATION	"RLVaLoginLastLocation"
#define RLV_SETTING_SHOWNAMETAGS		"RLVaShowNameTags"

#define RLV_SETTING_FIRSTUSE_PREFIX		"FirstRLV"
#define RLV_SETTING_FIRSTUSE_DETACH		RLV_SETTING_FIRSTUSE_PREFIX"Detach"
#define RLV_SETTING_FIRSTUSE_ENABLEWEAR	RLV_SETTING_FIRSTUSE_PREFIX"EnableWear"
#define RLV_SETTING_FIRSTUSE_FARTOUCH	RLV_SETTING_FIRSTUSE_PREFIX"Fartouch"
#define RLV_SETTING_FIRSTUSE_GIVETORLV	RLV_SETTING_FIRSTUSE_PREFIX"GiveToRLV"

// ============================================================================

#endif // RLV_DEFINES_H
