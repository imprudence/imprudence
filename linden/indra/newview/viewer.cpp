/** 
 * @file viewer.cpp
 * @brief A window into the virtual world.
 *
 * Copyright (c) 2000-2007, Linden Research, Inc.
 * 
 * Second Life Viewer Source Code
 * The source code in this file ("Source Code") is provided by Linden Lab
 * to you under the terms of the GNU General Public License, version 2.0
 * ("GPL"), unless you have obtained a separate licensing agreement
 * ("Other License"), formally executed by you and Linden Lab.  Terms of
 * the GPL can be found in doc/GPL-license.txt in this distribution, or
 * online at http://secondlife.com/developers/opensource/gplv2
 * 
 * There are special exceptions to the terms and conditions of the GPL as
 * it is applied to this Source Code. View the full text of the exception
 * in the file doc/FLOSS-exception.txt in this software distribution, or
 * online at http://secondlife.com/developers/opensource/flossexception
 * 
 * By copying, modifying or distributing this software, you acknowledge
 * that you have read and understood your obligations described above,
 * and agree to abide by those obligations.
 * 
 * ALL LINDEN LAB SOURCE CODE IS PROVIDED "AS IS." LINDEN LAB MAKES NO
 * WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
 * COMPLETENESS OR PERFORMANCE.
 */

#include "llviewerprecompiledheaders.h"

#include "viewer.h"

#include "llparcel.h"
#include "llviewerparcelmgr.h"
#include "llviewerjoystick.h"

// System library headers
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <stdexcept>
#include <math.h>
#if LL_WINDOWS
#	include <share.h>
#else
#	include <sys/file.h>
#	include <signal.h>
#endif
#include <time.h>
#include <sys/stat.h>
#include <memory.h>
#include <string.h>
#include <fstream>
#include <boost/tokenizer.hpp>
#include "curl/curl.h"

#if LL_WINDOWS
#include <fcntl.h>		//_O_APPEND
#include <io.h>			//_open_osfhandle()
#include <errorrep.h>	// for AddERExcludedApplicationA()
#include <process.h>	// _spawnl()
#include <tchar.h>		// For TCHAR support

#if LL_WINDOWS && _MSC_VER < 1400
//#define LL_USE_SMARTHEAP 0
#else
#define LL_USE_SMARTHEAP 0
#endif

#if LL_WINDOWS && LL_RELEASE_FOR_DOWNLOAD && LL_USE_SMARTHEAP
#include "smrtheap/smrtheap.h"
#endif // LL_WINDOWS && LL_RELEASE_FOR_DOWNLOAD && LL_USE_SMARTHEAP

#elif LL_DARWIN || LL_LINUX

  #	include <sys/socket.h>
//  #	include <sys/stat.h>		// mkdir()
  #	include <netinet/in.h>
  #	include <arpa/inet.h>	// inet_ntoa()

  #if LL_LINUX
  #	include <dlfcn.h>		// RTLD_LAZY
  #     include <execinfo.h>            // backtrace - glibc only
  #define LL_ELFBIN 1
  #     if LL_ELFBIN
  #          include <cxxabi.h>         // for symbol demangling
  #          include "ELFIO.h"          // for better backtraces
  #     endif // LL_ELFBIN
  #endif

  #if LL_DARWIN
    #include <Carbon/Carbon.h>
    // Apple sucks!  AssertMacros.h defines these COMMONLY used names.
    #ifdef check
	    #undef check
    #endif
    #ifdef verify
	    #undef verify
    #endif // verify
    #ifdef require
	    #undef require
    #endif
  #endif
#endif // !LL_WINDOWS

// Support for sending crash reports from the viewer?
//#define LL_SEND_CRASH_REPORTS 0


//
// Linden library headers
//

#include "audioengine.h" 
#include "llcommon.h" 
#include "llapr.h" 
#include "llcachename.h"
#include "llcurl.h"
#include "llcriticaldamp.h"
#include "lldir.h"
#include "lleconomy.h"
#include "llerrorcontrol.h"
#include "llhttpnode.h"
#include "llflexibleobject.h"
#include "llfasttimer.h"
#include "llfocusmgr.h"
#include "llgroupmgr.h"
#include "llimage.h"
#include "llimageworker.h"
#include "lllfsthread.h"
#include "llmemtype.h"
#include "llmd5.h"
#include "llsecondlifeurls.h"
#include "llversion.h"
#include "llvfile.h"
#include "llvfs.h"
#include "llwindow.h"		// for shell_open
#include "llworkerthread.h"
#include "llvfsthread.h"
#include "llxfermanager.h"
#include "message.h"
#include "llvoavatar.h"
#include "llglslshader.h"

//
// Viewer headers
//

#include "llasynchostbyname.h"
#include "llagent.h"
#include "llagentpilot.h"
#include "llbutton.h" // For constants
#include "llcallbacklist.h"
#include "llchatbar.h"
#include "llcombobox.h" // For constants
#include "llconsole.h"
#include "llcontainerview.h"
#include "lldebugview.h"
#include "lldrawpoolbump.h"
#include "lldrawpoolterrain.h"
#include "lleventnotifier.h"
#include "llfasttimerview.h"
#include "llfeaturemanager.h"
#include "llfirstuse.h"
#include "llfloatertools.h"
#include "llfloaterworldmap.h"
#include "llfloaterhtmlhelp.h"
#include "llfloatersaveavatar.h"
#include "llfloatersnapshot.h"
#include "llfolderview.h"
#include "llframestats.h"
#include "llgesturemgr.h"
#include "llhoverview.h"
#include "llhudeffecttrail.h"
#include "llhudmanager.h"
#include "llhttpclient.h"
#include "llimview.h"
#include "llinventorymodel.h"
#include "llinventoryview.h"
#include "llkeyboard.h"
#include "llkeyframemotion.h"
#include "llpanellogin.h"
#include "llmutelist.h"
#include "llmenugl.h"
#include "llnamelistctrl.h"
#include "llnamebox.h"
#include "llnameeditor.h"
#include "llpumpio.h"
#include "llnotify.h"
#include "llselectmgr.h"
#include "llsky.h"
#include "llstartup.h"
#include "llstatusbar.h"
#include "llsurface.h"
#include "lltexlayer.h"
#include "lltexturecache.h"
#include "lltexturefetch.h"
#include "lltoolbar.h"
#include "lltoolmgr.h"
#include "lltracker.h"
#include "llurlwhitelist.h"
#include "llviewerbuild.h"
#include "llviewercamera.h"
#include "llviewercontrol.h"
#include "llviewerimagelist.h"
#include "llviewerkeyboard.h"
#include "llviewermenu.h"
#include "llviewermessage.h"
#include "llviewernetwork.h"
#include "llviewerobjectlist.h"
#include "llviewerparcelmgr.h"
#include "llviewerregion.h"
#include "llviewerstats.h"
#include "llviewerthrottle.h"
#include "llvieweruictrlfactory.h"
#include "llviewerwindow.h"
#include "llvlmanager.h"
#include "llvoavatar.h"
#include "llvograss.h"
#include "llvotree.h"
#include "llvovolume.h"		// To set a static member.
#include "llvowater.h"
#include "llvolume.h"
#include "llvolumemgr.h"
#include "llvolumemessage.h"
#include "llweb.h"
#include "llworld.h"
#include "llworldmap.h"
#include "llworldmapview.h"
#include "pipeline.h"
#include "llface.h"
#include "audiosettings.h"
#include "res/resource.h"

#if LL_WINDOWS
#include "llwindebug.h"
#include "lldxhardware.h"
#include "llwindowwin32.h"
#endif // LL_WINDOWS

#if LL_QUICKTIME_ENABLED
#if LL_DARWIN
#include <QuickTime/QuickTime.h>
#else
// quicktime specific includes
#include "MacTypes.h"
#include "QTML.h"
#include "Movies.h"
#include "FixMath.h"
#endif
#endif

#include "llmediaengine.h"

#if LL_LIBXUL_ENABLED
#include "llmozlib.h"
#endif // LL_LIBXUL_ENABLED

/////////////////////////////////////////////////////////////////////////////////
// Support for crash handling.
/////////////////////////////////////////////////////////////////////////////////

void errorCallback(const std::string &error_string);
S32 gCrashBehavior = CRASH_BEHAVIOR_ASK;
void (*gCrashCallback)(void) = NULL;
BOOL gReportedCrash = FALSE;
bool gVerifySSLCert = true;

BOOL gHandleKeysAsync = FALSE;

// Use DirectX 9 to probe for hardware
BOOL gProbeHardware = TRUE;
std::string gSerialNumber;

/////////////////////////////////////////////////////////////////////////////////
// Application constants
/////////////////////////////////////////////////////////////////////////////////

S32 gStartupState = STATE_FIRST;

BOOL gAgentMovementCompleted = FALSE;
BOOL gHaveSavedSnapshot = FALSE;

S32 gYieldMS = 0;
BOOL gYieldTime = FALSE;

const U32 COLLISION_LIST_DEPTH = 300;	// only looks at the first so many collisionable objects
const S32 MAX_CONSOLE_LINES = 500;
const S32 NUM_SESSIONS_BEFORE_SHOW_PROFILE = 5;
const F32 DEFAULT_AFK_TIMEOUT = 5.f * 60.f; // time with no input before user flagged as Away From Keyboard

const char *VFS_DATA_FILE_BASE			= "data.db2.x.";
const char *VFS_INDEX_FILE_BASE			= "index.db2.x.";

const F32 MAX_USER_FOG_RATIO = 4.f;
const F32 MIN_USER_FOG_RATIO = 0.5f;

F32 gSimLastTime;
F32 gSimFrames;

const S32 MAX_USER_COMPOSITE_LIMIT = 100;
const S32 MIN_USER_COMPOSITE_LIMIT = 0;

//#define RENDER_CLOUD_DENSITY		// uncomment to look at cloud density

/////////////////////////////////////////////////////////////////////////////////
// Globals
/////////////////////////////////////////////////////////////////////////////////

//
// Core Application globals
//

LLString gSecondLife;
LLString gWindowTitle;
LLString gWindowName("Second Life");
LLString gDisabledMessage;
BOOL gHideLinks = FALSE;

// This is whether or not we are connect to a production grid.
BOOL gInProductionGrid	= FALSE;

//#define APPLE_PREVIEW // Define this if you're doing a preview build on the Mac
#if LL_RELEASE_FOR_DOWNLOAD
// Default userserver for production builds is agni
#ifndef APPLE_PREVIEW
static EUserServerDomain UserServerDefaultChoice = USERSERVER_AGNI;
#else
static EUserServerDomain UserServerDefaultChoice = USERSERVER_ADITI;
#endif
#else
// Default userserver for development builds is dmz
static EUserServerDomain UserServerDefaultChoice = USERSERVER_DMZ;
#endif

#ifdef TOGGLE_HACKED_GODLIKE_VIEWER
BOOL 				gHackGodmode = FALSE;
#endif

// Only used if not empty.  Otherwise uses value from table above.
static std::string	gLoginURI;
static std::string	gHelperURI;

LLAgent				gAgent;
LLPipeline			gPipeline;

// Set true when the viewer has been disconnected from the server, for example,
// by the user being kicked.
BOOL				gDoDisconnect = FALSE;
BOOL				gDisconnected = FALSE;

// Tells us to clean up the cache directory in the case of network corruption
BOOL				gPurgeOnExit = FALSE;
BOOL				gPurgeCache = FALSE;

// Allow multiple viewers in ReleaseForDownload
#if LL_RELEASE_FOR_DOWNLOAD
BOOL				gMultipleViewersOK = FALSE;
#else
BOOL				gMultipleViewersOK = TRUE;
#endif
BOOL				gSecondInstance = FALSE;

LLString			gArgs;

// Setting this true will cause the app to exit cleanly at the end of the frame.
BOOL				gQuit = FALSE;
// Set when user has indicated desire to quit, but may have modified documents open
BOOL				gQuitRequested = FALSE;
LLString			gLaunchFileOnQuit;
BOOL				gDoneLogout = FALSE;

BOOL				gInitializationComplete = FALSE;	// used in windows handlers to determine if OK to call idle()
BOOL				gAutoLogin = FALSE;
LLString			gOldSettingsFileName;
BOOL				gPrintMessagesThisFrame = FALSE;
const char*			DEFAULT_SETTINGS_FILE = "settings.xml";
const char*			LEGACY_DEFAULT_SETTINGS_FILE = "settings.ini";
BOOL				gUseWireframe = FALSE;
BOOL				gRunLocal = FALSE;
LLUUID				gViewerDigest;	// MD5 digest of the viewer's executable file.
LLPumpIO*			gServicePump = NULL;
S32					gNumSessions = 0;

BOOL				gAllowAFK = TRUE;
F32					gAFKTimeout = DEFAULT_AFK_TIMEOUT;
F32					gMouseSensitivity = 3.f;
BOOL				gInvertMouse = FALSE;
BOOL				gLogoutRequestSent = FALSE;
LLTimer				gLogoutTimer;
BOOL				gShowObjectUpdates = FALSE;

const F32			LOGOUT_REQUEST_TIME = 6.f;  // this will be cut short by the LogoutReply msg.
F32					gLogoutMaxTime = LOGOUT_REQUEST_TIME;

// Map scale in pixels per region
F32 				gMapScale = 128.f;
F32 				gMiniMapScale = 128.f;

// Sky object, globals
LLSky				gSky;

// HUD display lines in lower right
BOOL				gDisplayWindInfo = FALSE;
BOOL				gDisplayCameraPos = FALSE;
BOOL				gDisplayNearestWater = FALSE;
BOOL				gDoNearestWaterSearch = FALSE;
BOOL				gDisplayFOV = FALSE;

// used to restore texture state after a mode switch
LLFrameTimer	gRestoreGLTimer;
BOOL			gRestoreGL = FALSE;


// VFS globals - see viewer.h
LLVFS* gStaticVFS = NULL;

// Threads
LLTextureCache* gTextureCache = NULL;
LLWorkerThread* gImageDecodeThread = NULL;
LLTextureFetch* gTextureFetch = NULL;

// Debugging
FILE *gDebugFile = NULL;	// File pointer used by the function which writes debug data.
BOOL gRandomizeFramerate = FALSE;
BOOL gPeriodicSlowFrame = FALSE;
std::map<S32,LLFrameTimer> gDebugTimers;

//LLVector3			gCameraVelocitySmoothed;
//
// Timing/Performance/statistics globals
//

// Frame timing
U64 gFrameTime = 0;
F32 gFrameTimeSeconds = 0.f;
F32 gFrameIntervalSeconds = 0.f;
U32	gFrameCount = 0;
U64	gStartTime = 0; // gStartTime is "private", used only to calculate gFrameTimeSeconds
U64	gSpaceTime = 0; // gSpaceTime is the time, according to the spaceserver.

//  Timing Globals
LLTimer gRenderStartTime;
F32 gQuitAfterSeconds = 0.f;
BOOL gRotateRight = FALSE;

// Is the Pacific time zone (aka server time zone)
// current in daylight savings time?
BOOL gPacificDaylightTime = FALSE;

//
// Simulator/SpaceServer configuration information
//
U32	gSecondsPerDay = 0;
U32	gSecondsPerYear = 0;

//
// Region/Object globals
//


LLVector3			gWindVec(3.0, 3.0, 0.0);
LLVector3			gRelativeWindVec(0.0, 0.0, 0.0);

//
// Globals for controls...
//

BOOL gVelocityInterpolate = TRUE; //  (These are written once/frame with the data from gSavedSettings)
BOOL gPingInterpolate = TRUE; 

//static
LLURLSimString LLURLSimString::sInstance;

//
// System info
//

LLMemoryInfo gSysMemory;
LLOSInfo gSysOS;

// file globals
static const char USAGE[] = "\n"
"usage:\tviewer [options]\n"
"options:\n"
" -login <first> <last> <password>     log in as a user\n"
" -autologin                           log in as last saved user\n"
" -loginuri <URI>                      login server and CGI script to use\n"
" -helperuri <URI>                     helper web CGI prefix to use\n"
" -settings <filename>                 specify the filename of a\n"
"                                        configuration file\n"
"                                        default is settings.xml\n"
" -setdefault <variable> <value>       specify the value of a particular\n"
"                                        configuration variable which can be\n"
"                                        overridden by settings.xml\n"
" -set <variable> <value>              specify the value of a particular\n"
"                                        configuration variable that\n"
"                                        overrides all other settings\n"
" -user <user_server_ip>               specify userserver in dotted quad\n"
#if !LL_RELEASE_FOR_DOWNLOAD
" -sim <simulator_ip>                  specify the simulator ip address\n"
" -local                               run without simulator\n"
#endif
" -god		                           log in as god if you have god access\n"
" -purge                               delete files in cache\n"
" -safe                                reset preferences, run in safe mode\n"
" -noutc                               logs in local time, not UTC\n"
" -nothread                            run vfs in single thread\n"
" -noinvlib                            Do not request inventory library\n"
" -multiple                            allow multiple viewers\n"
" -nomultiple                          block multiple viewers\n"
" -ignorepixeldepth                    ignore pixel depth settings\n"
" -cooperative [ms]                    yield some idle time to local host\n"
" -skin                                ui/branding skin folder to use\n"
#if LL_WINDOWS
" -noprobe                             disable hardware probe\n"
#endif
" -noquicktime                         disable QuickTime movies, speeds startup\n"
" -nopreload                           don't preload UI images or sounds, speeds startup\n"
// these seem to be unused
//" -noenv                               turn off environmental effects\n"
//" -proxy <proxy_ip>                    specify the proxy ip address\n"
"\n";


// Variables used for passing data out of main
BOOL gGodConnect = FALSE;
BOOL gUseConsole = TRUE;
BOOL gUseAudio = TRUE;
BOOL gUseFMOD = TRUE;
BOOL gConnectToSomething = TRUE;
BOOL gLogMessages = FALSE;
BOOL gRequestInventoryLibrary = TRUE;
BOOL gAcceptTOS = FALSE;
BOOL gAcceptCriticalMessage = FALSE;
// this is the channel the viewer uses to check for updates/login
std::string gChannelName = "Second Life Release";

LLUUID gInventoryLibraryOwner;
LLUUID gInventoryLibraryRoot;
bool gPreloadImages = true;
bool gPreloadSounds = true;

LLString gCmdLineFirstName;
LLString gCmdLineLastName;
LLString gCmdLinePassword;
std::map<std::string, std::string> gCommandLineSettings;
std::map<std::string, std::string> gCommandLineForcedSettings;
BOOL gLastExecFroze = FALSE;
BOOL gIgnorePixelDepth = FALSE;


LLPointer<LLImageGL> gDisconnectedImagep = NULL;


/*
class LLFirstInventoryLoadObserver : public LLInventoryObserver
{
	// Called when the inventory is first loaded.
	// Must be allocated on the heap so that the inventory model can destroy it 
	// in case the app ends before the inventory finishes loading.
public:
	virtual void changed(U32 mask);
	void observe(LLInventoryModel* model);
protected:
	LLInventoryModel* mInventory;
};
*/

///////////////////////
//
// Forward declarations
//

//
// Application initialization and cleanup
//
void init_marker_file();
void init_crash_handler();
void init_logging();
void create_console();
void write_system_info();
int parse_args(int argc, char **argv);
void saved_settings_to_globals();
BOOL init_cache();
void purge_cache();
void cleanup_app();
void disconnect_viewer(void *); // Don't use directly - use do_disconnect()

//
// Debugging, logging, and error reporting
//
void write_debug(const char *str);	// Write a string to the debug log
void write_debug(const std::string& str);
void close_debug();					// Close the debug log
void catch_signals();
void viewer_crash_callback();
void remove_marker_file();
#if !LL_WINDOWS
void release_signals();
#endif

void bad_network_handler();

#if LL_WINDOWS
void disable_win_error_reporting();
#endif
std::string get_serial_number();
BOOL another_instance_running();
void main_loop();

//
// Callbacks
//
// Callbacks and other stuff that's not directly used in main
//
void uuid_table_request_file_callback(void **user_data, S32 result);
void send_stats();

//
// Apple specific stuff
//
#if LL_DARWIN
void init_apple_menu(const char* product);
OSErr AEGURLHandler(const AppleEvent *messagein, AppleEvent *reply, long refIn);
OSErr AEQuitHandler(const AppleEvent *messagein, AppleEvent *reply, long refIn);
OSStatus simpleDialogHandler(EventHandlerCallRef handler, EventRef event, void *userdata);
OSStatus DisplayReleaseNotes(void);
#endif // LL_DARWIN

void ui_audio_callback(const LLUUID& uuid, F32 volume)
{
	if (gAudiop)
	{
		gAudiop->triggerSound(uuid, gAgent.getID(), volume);
	}
}

#if LL_WINDOWS
BOOL CALLBACK login_dialog_func(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lparam)
{
	char buffer[MAX_STRING];		/* Flawfinder: ignore */
	switch(msg)
	{
    case WM_INITDIALOG:
		{
			LLString first_name = gCmdLineFirstName;
			LLString last_name = gCmdLineLastName;
			LLString password = gCmdLinePassword;

			if (!first_name.empty())
			{
				SetDlgItemTextA( hwnd, IDC_EDIT_FIRSTNAME, first_name.c_str());
			}
			if (!last_name.empty())
			{
				SetDlgItemTextA( hwnd, IDC_EDIT_LASTNAME, last_name.c_str());
			}
			if (!password.empty())
			{
				SetDlgItemTextA( hwnd, IDC_EDIT_PASSWORD, password.c_str());
			}

			first_name = gSavedSettings.getString("FirstName");
			last_name = gSavedSettings.getString("LastName");
			password = load_password_from_disk();

			if (!first_name.empty())
			{
				SetDlgItemTextA( hwnd, IDC_EDIT_FIRSTNAME, first_name.c_str());
			}
			if (!last_name.empty())
			{
				SetDlgItemTextA( hwnd, IDC_EDIT_LASTNAME, last_name.c_str());
			}
			if (!password.empty())
			{
				SetDlgItemTextA( hwnd, IDC_EDIT_PASSWORD, password.c_str());
			}

			// Focus on password if other fields are full
			if (first_name.empty())
			{
				SetFocus(GetDlgItem(hwnd, IDC_EDIT_FIRSTNAME)); 
			}
			else if (last_name.empty())
			{
				SetFocus(GetDlgItem(hwnd, IDC_EDIT_LASTNAME)); 
			}
			else if (password.empty())
			{
				SetFocus(GetDlgItem(hwnd, IDC_EDIT_PASSWORD)); 
			}
			else
			{
				SetFocus(GetDlgItem(hwnd, IDC_EDIT_FIRSTNAME)); 
			}

			BOOL remember_password = gSavedSettings.getBOOL("RememberPassword");
			if (remember_password)
			{
				CheckDlgButton(hwnd, IDC_REMEMBER_PASSWORD, BST_CHECKED);
			}
			else
			{
				CheckDlgButton(hwnd, IDC_REMEMBER_PASSWORD, BST_UNCHECKED);
			}
			return TRUE;
		}

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			{
				// copy login name and password into buffer
				if (GetDlgItemTextA(hwnd, IDC_EDIT_FIRSTNAME, buffer, MAX_STRING))
				{
					gCmdLineFirstName = buffer;
				}
				if (GetDlgItemTextA(hwnd, IDC_EDIT_LASTNAME, buffer, MAX_STRING))
				{
					gCmdLineLastName = buffer;
				}
				if (GetDlgItemTextA(hwnd, IDC_EDIT_PASSWORD, buffer, MAX_STRING))
				{
					gCmdLinePassword = buffer;
				}
				BOOL remember_password = (IsDlgButtonChecked(hwnd, IDC_REMEMBER_PASSWORD) == BST_CHECKED);
				gSavedSettings.setBOOL("RememberPassword", remember_password);
				EndDialog(hwnd, 0);	// return success
				return TRUE;	// handled
			}
		}
		// If we get here, we didn't handle it
		return FALSE;

	case WM_CLOSE:
	case WM_DESTROY:
		EndDialog(hwnd, 666);	// assume user wants normal login screen
		return TRUE;	// handled

	default:
		return FALSE;
	}
}
#endif

#if LL_WINDOWS && LL_RELEASE_FOR_DOWNLOAD && LL_USE_SMARTHEAP

MEM_BOOL MEM_CALLBACK second_mem_error_handler(MEM_ERROR_INFO *errorInfo);

MEM_BOOL MEM_CALLBACK first_mem_error_handler(MEM_ERROR_INFO *errorInfo)
{
    MemSetErrorHandler(second_mem_error_handler);

	// Really should free up reserved memory here and warn users
	// with dialog they have precious little time left in Second
	// Life!

	llerrs << "Memory allocation failed; aborting." << llendl;
	// llerrs << "Memory allocation failed; reserve memory released." << llendl;

	// NOTREACHED better not be, but see second_mem_error_handler
	// Could do that freeing up reserved memory thing here and
	// return 1;
	return 0;
}

MEM_BOOL MEM_CALLBACK second_mem_error_handler(MEM_ERROR_INFO *errorInfo)
{
	// Just in case "llerrs" and "llendl" cause another out-of-memory.
	LLError::crashAndLoop("");
	// NOTREACHED better not be!
	return 0;
}

#endif // LL_WINDOWS && LL_RELEASE_FOR_DOWNLOAD && LL_USE_SMARTHEAP


#if LL_WINDOWS
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
#else 
int main( int argc, char **argv ) 
#endif
{
	LLMemType mt1(LLMemType::MTYPE_STARTUP);
	
#if 1
	// This will eventually be done in LLApp
	LLCommon::initClass();
	// This should eventually be done in LLAppViewer
# if MEM_TRACK_MEM
	static const bool enable_threads = false;
# else
	static const bool enable_threads = true;
# endif
	LLVFSThread::initClass(enable_threads && true);
	LLLFSThread::initClass(enable_threads && true);
	// Image decoding
	gImageDecodeThread = new LLWorkerThread("ImageDecode", enable_threads && true);
	gTextureCache = new LLTextureCache(enable_threads && true);
	gTextureFetch = new LLTextureFetch(gTextureCache, enable_threads && false);
	LLImageWorker::initClass(gImageDecodeThread);
	LLImageJ2C::openDSO();
#endif

#if LL_WINDOWS
	// In Win32, we need to generate argc and argv ourselves...
	// Note: GetCommandLine() returns a  potentially return a LPTSTR
	// which can resolve to a LPWSTR (unicode string).
	// (That's why it's different from lpCmdLine which is a LPSTR.)
	// We don't currently do unicode, so call the non-unicode version
	// directly.
	LPSTR cmd_line_including_exe_name = GetCommandLineA();

	gIconResource = MAKEINTRESOURCE(IDI_LL_ICON);

	const S32	MAX_ARGS = 100;
	int argc = 0;
	char* argv[MAX_ARGS];		/* Flawfinder: ignore */

	char *token = NULL;
	if( cmd_line_including_exe_name[0] == '\"' )
	{
		// Exe name is enclosed in quotes
		token = strtok( cmd_line_including_exe_name, "\"" );
		argv[argc++] = token;
		token = strtok( NULL, " \t," );
	}
	else
	{
		// Exe name is not enclosed in quotes
		token = strtok( cmd_line_including_exe_name, " \t," );
	}

	while( (token != NULL) && (argc < MAX_ARGS) )
	{
		argv[argc++] = token;
		/* Get next token: */
		if (*(token + strlen(token) + 1) == '\"')		/* Flawfinder: ignore*/
		{
			token = strtok( NULL, "\"");
		}
		else
		{
			token = strtok( NULL, " \t," );
		}
	}
#endif

	// HACK! We REALLY want to know what grid they were trying to connect to if they
	// crashed hard.
	// So we walk through the command line args ONLY looking for the
	// userserver arguments first.  And we don't do ANYTHING but set
	// the gUserServerName (which gets passed to the crash reporter).
	// We're assuming that they're trying to log into the same grid as last
	// time, which seems fairly reasonable.
	snprintf(gUserServerName, MAX_STRING, "%s", gUserServerDomainName[UserServerDefaultChoice].mName);		/* Flawfinder: ignore */
	S32 j;
	for (j = 1; j < argc; j++) 
	{
		if (!strcmp(argv[j], "--aditi"))
		{
			snprintf(gUserServerName, MAX_STRING, "%s", gUserServerDomainName[USERSERVER_ADITI].mName);		/* Flawfinder: ignore */
		}
		else if (!strcmp(argv[j], "--agni"))
		{
			snprintf(gUserServerName, MAX_STRING, "%s", gUserServerDomainName[USERSERVER_AGNI].mName);		/* Flawfinder: ignore */
		}
		else if (!strcmp(argv[j], "--dmz"))
		{
			snprintf(gUserServerName, MAX_STRING, "%s", gUserServerDomainName[USERSERVER_DMZ].mName);		/* Flawfinder: ignore */
		}
		else if (!strcmp(argv[j], "--siva"))
		{
			snprintf(gUserServerName, MAX_STRING, "%s", gUserServerDomainName[USERSERVER_SIVA].mName);		/* Flawfinder: ignore */
		}
		else if (!strcmp(argv[j], "--shakti"))
		{
			sprintf(gUserServerName,"%s", gUserServerDomainName[USERSERVER_SHAKTI].mName);
		}
		else if (!strcmp(argv[j], "--durga"))
		{
			snprintf(gUserServerName, MAX_STRING, "%s", gUserServerDomainName[USERSERVER_DURGA].mName);		/* Flawfinder: ignore */
		}
		else if (!strcmp(argv[j], "--soma"))
		{
			snprintf(gUserServerName, MAX_STRING, "%s", gUserServerDomainName[USERSERVER_SOMA].mName);		/* Flawfinder: ignore */
		}
		else if (!strcmp(argv[j], "--ganga"))
		{
			snprintf(gUserServerName, MAX_STRING, "%s", gUserServerDomainName[USERSERVER_GANGA].mName);		/* Flawfinder: ignore */
		}
		else if (!strcmp(argv[j], "--vaak"))
		{
			sprintf(gUserServerName,"%s", gUserServerDomainName[USERSERVER_VAAK].mName);
		}
		else if (!strcmp(argv[j], "--uma"))
		{
			sprintf(gUserServerName,"%s", gUserServerDomainName[USERSERVER_UMA].mName);
		}
		else if (!strcmp(argv[j], "-user") && (++j < argc))
		{
			if (!strcmp(argv[j], "-"))
			{
				snprintf(gUserServerName, MAX_STRING, "%s", LOOPBACK_ADDRESS_STRING);		/* Flawfinder: ignore */
			}
			else
			{
				snprintf(gUserServerName, MAX_STRING, "%s", argv[j]);		/* Flawfinder: ignore */
			}
		}
		else if (!strcmp(argv[j], "-multiple"))
		{
			// Hack to detect -multiple so we can disable the marker file check (which will always fail)
			gMultipleViewersOK = TRUE;
		}
		else if (!strcmp(argv[j], "-url") && (++j < argc)) 
		{
			LLURLSimString::setString(argv[j]);
			gConnectToSomething = TRUE;
		}
	}

	//
	// Start of the application
	//
	// IMPORTANT! Do NOT put anything that will write
	// into the log files during normal startup until AFTER
	// we run the "program crashed last time" error handler below.
	//

	// Need to do this initialization before we do anything else, since anything
	// that touches files should really go through the lldir API
	gDirUtilp->initAppDirs("SecondLife");

	//
	// Set up logging defaults for the viewer
	//
	LLError::initForApplication(
				gDirUtilp->getExpandedFilename(LL_PATH_APP_SETTINGS, ""));
	LLError::setFatalFunction(errorCallback);


#if LL_RELEASE_FOR_DOWNLOAD && LL_SEND_CRASH_REPORTS
	//
	// Crash log if we hard crashed.
	// Initialize crash logging
	//
	init_crash_handler();
#endif
	// Set up SecondLife.log
	init_logging();
	
	//
	// OK to write stuff to logs now, we've now crash reported if necessary
	//

	// Set up some defaults...
	gSettingsFileName = gDirUtilp->getExpandedFilename(LL_PATH_USER_SETTINGS, DEFAULT_SETTINGS_FILE);
	gOldSettingsFileName = gDirUtilp->getExpandedFilename(LL_PATH_USER_SETTINGS, LEGACY_DEFAULT_SETTINGS_FILE);
	gUserServer.setPort(DEFAULT_USER_SERVER_PORT);

	/////////////////////////////////////////
	//
	// Process command line arguments
	//
	S32 args_result = 0;

#if LL_DARWIN
	{
		// On the Mac, read in arguments.txt (if it exists) and process it for additional arguments.
		LLString args;
		if(LLString::read(args, "arguments.txt"))		/* Flawfinder: ignore*/
		{
			// The arguments file exists.  
			// It should consist of command line arguments separated by newlines.
			// Split it into individual arguments and build a fake argv[] to pass to parse_args.
			std::vector<std::string> arglist;
			
			arglist.push_back("newview");
			
			llinfos << "Reading additional command line arguments from arguments.txt..." << llendl;
			
			typedef boost::tokenizer<boost::escaped_list_separator<char> > tokenizer;
			boost::escaped_list_separator<char> sep("\\", "\r\n ", "\"'");
			tokenizer tokens(args, sep);
			tokenizer::iterator token_iter;

			for(token_iter = tokens.begin(); token_iter != tokens.end(); ++token_iter)
			{
				llinfos << "argument: '" << (token_iter->c_str()) << "'" << llendl;
				
				arglist.push_back(*token_iter);
			}

			char **fakeargv = new char*[arglist.size()];
			int i;
			for(i=0; i < arglist.size(); i++)
				fakeargv[i] = const_cast<char*>(arglist[i].c_str());
				
			args_result = parse_args(arglist.size(), fakeargv);
			delete[] fakeargv;
		}
		
		// Get the user's preferred language string based on the Mac OS localization mechanism.
		// To add a new localization:
			// go to the "Resources" section of the project
			// get info on "language.txt"
			// in the "General" tab, click the "Add Localization" button
			// create a new localization for the language you're adding
			// set the contents of the new localization of the file to the string corresponding to our localization
			//   (i.e. "en-us", "ja", etc.  Use the existing ones as a guide.)
		CFURLRef url = CFBundleCopyResourceURL(CFBundleGetMainBundle(), CFSTR("language"), CFSTR("txt"), NULL);
		char path[MAX_PATH];
		if(CFURLGetFileSystemRepresentation(url, false, (UInt8 *)path, sizeof(path)))
		{
			LLString lang;
			if(LLString::read(lang, path))		/* Flawfinder: ignore*/
			{
				gCommandLineForcedSettings["SystemLanguage"] = lang;
			}
		}
		CFRelease(url);
	}
#endif

	//
	// Parse the command line arguments
	//
	args_result |= parse_args(argc, argv);
	if (args_result)
	{
		remove_marker_file();
		return args_result;
	}

	// XUI:translate
	gSecondLife = "Second Life";
	
	// Read skin/branding settings if specified.
	if (! gDirUtilp->getSkinDir().empty() )
	{
		std::string skin_def_file = gDirUtilp->getExpandedFilename(LL_PATH_TOP_SKIN, "skin.xml");
		LLXmlTree skin_def_tree;

		if (!skin_def_tree.parseFile(skin_def_file))
		{
			llerrs << "Failed to parse skin definition." << llendl;
		}

		LLXmlTreeNode* rootp = skin_def_tree.getRoot();
		LLXmlTreeNode* disabled_message_node = rootp->getChildByName("disabled_message");	
		if (disabled_message_node)
		{
			gDisabledMessage = disabled_message_node->getContents();
		}

		static LLStdStringHandle hide_links_string = LLXmlTree::addAttributeString("hide_links");
		rootp->getFastAttributeBOOL(hide_links_string, gHideLinks);

		// Legacy string.  This flag really meant we didn't want to expose references to "Second Life".
		// Just set gHideLinks instead.
		static LLStdStringHandle silent_string = LLXmlTree::addAttributeString("silent_update");
		BOOL silent_update;
		rootp->getFastAttributeBOOL(silent_string, silent_update);
		gHideLinks = (gHideLinks || silent_update);
	}

#if LL_DARWIN
	// Initialize apple menubar and various callbacks
	init_apple_menu(gSecondLife.c_str());

	// If the CPU doesn't have Altivec (i.e. it's not at least a G4), don't go any further.
	if(!gSysCPU.hasSSE())
	{
		std::ostringstream msg;
		msg << gSecondLife << " requires a processor with AltiVec (G4 or later).";
		OSMessageBox(
			msg.str().c_str(),
			NULL,
			OSMB_OK);
		remove_marker_file();
		return 1;
	}
	
#endif // LL_DARWIN

	// Display splash screen.  Must be after above check for previous
	// crash as this dialog is always frontmost.
	std::ostringstream splash_msg;
	splash_msg << "Loading " << gSecondLife << "...";
	LLSplashScreen::show();
	LLSplashScreen::update(splash_msg.str().c_str());

	LLVolumeMgr::initClass();

	// Initialize the feature manager
	// The feature manager is responsible for determining what features
	// are turned on/off in the app.
	gFeatureManagerp = new LLFeatureManager;

	gStartTime = totalTime();


	////////////////////////////////////////
	//
	// Process ini files
	//

	// declare all possible setting variables
	declare_settings();

#if !LL_RELEASE_FOR_DOWNLOAD
//	only write the defaults for non-release builds!
	gSavedSettings.saveToFile(gDirUtilp->getExpandedFilename(LL_PATH_USER_SETTINGS,"settings_default.xml").c_str(), FALSE);
#endif

	//
	// Set the name of the window
	//
#if LL_RELEASE_FOR_DOWNLOAD
	gWindowTitle = gSecondLife;
#elif LL_DEBUG
	gWindowTitle = gSecondLife + LLString(" [DEBUG] ") + gArgs;
#else
	gWindowTitle = gSecondLife + LLString(" ") + gArgs;
#endif
	LLString::truncate(gWindowTitle, 255);

	if (!gMultipleViewersOK)
	{
	    //
	    // Check for another instance of the app running
	    //
		//RN: if we received a URL, hand it off to the existing instance
		// don't call another_instance_running() when doing URL handoff, as
		// it relies on checking a marker file which will not work when running
		// out of different directories
		if (LLURLSimString::parse() && LLURLSimString::send_to_other_instance())
		{
			// successfully handed off URL to existing instance, exit
			return 1;
		}
		
		gSecondInstance = another_instance_running();
		
		if (gSecondInstance)
		{
			std::ostringstream msg;
			msg << 
				gSecondLife << " is already running.\n"
				"\n"
				"Check your task bar for a minimized copy of the program.\n"
				"If this message persists, restart your computer.",
			OSMessageBox(
				msg.str().c_str(),
				NULL,
				OSMB_OK);
			return 1;
		}

		init_marker_file();

#if LL_SEND_CRASH_REPORTS
		if (gLastExecFroze)
		{
			llinfos << "Last execution froze, requesting to send crash report." << llendl;
			//
			// Pop up a freeze or crash warning dialog
			//
			std::ostringstream msg;
			msg << gSecondLife
				<< " appears to have frozen or crashed on the previous run.\n"
				<< "Would you like to send a crash report?";
			std::string alert;
			alert = gSecondLife;
			alert += " Alert";
			S32 choice = OSMessageBox(msg.str().c_str(),
				alert.c_str(),
				OSMB_YESNO);
			if (OSBTN_YES == choice)
			{
				llinfos << "Sending crash report." << llendl;

 				remove_marker_file();
#if LL_WINDOWS
				std::string exe_path = gDirUtilp->getAppRODataDir();
				exe_path += gDirUtilp->getDirDelimiter();
				exe_path += "win_crash_logger.exe";

				std::string arg_string = "-previous -user ";
				arg_string += gUserServerName;
				arg_string += " -name \"";
				arg_string += gSecondLife;
				arg_string += "\"";
				// Spawn crash logger.
				// NEEDS to wait until completion, otherwise log files will get smashed.
				_spawnl(_P_WAIT, exe_path.c_str(), exe_path.c_str(), arg_string.c_str(), NULL);
#elif LL_DARWIN
				std::string command_str;
				command_str = "crashreporter.app/Contents/MacOS/crashreporter ";
				command_str += "-previous -user ";
				command_str += gUserServerName;
				// XXX -- We need to exit fullscreen mode for this to work.
				// XXX -- system() also doesn't wait for completion.  Hmm...
				system(command_str.c_str());		/* Flawfinder: Ignore */
#elif LL_LINUX
				std::string cmd =gDirUtilp->getAppRODataDir();
				cmd += gDirUtilp->getDirDelimiter();
				cmd += "linux-crash-logger.bin";
				char* const cmdargv[] =
					{(char*)cmd.c_str(),
					 (char*)"-previous",
					 (char*)"-user",
					 (char*)gUserServerName,
					 (char*)"-name",
					 (char*)gSecondLife.c_str(),
					 NULL};
				pid_t pid = fork();
				if (pid == 0)
				{ // child
					execv(cmd.c_str(), cmdargv);		/* Flawfinder: Ignore */
					llwarns << "execv failure when trying to start " << cmd << llendl;
					_exit(1); // avoid atexit()
				} else {
					if (pid > 0)
					{
						// wait for child proc to die
						int childExitStatus;
						waitpid(pid, &childExitStatus, 0);
					} else {
						llwarns << "fork failure." << llendl;
					}
				}
#endif
			}
			else
			{
				llinfos << "Not sending crash report." << llendl;
			}
		}
#endif // #if LL_SEND_CRASH_REPORTS
	}
	else
	{
		gSecondInstance = another_instance_running();
		
		/* Don't start another instance if using -multiple
		if (gSecondInstance)
		{
			//RN: if we received a URL, hand it off to the existing instance
		    if (LLURLSimString::parse())
		    {
			    LLURLSimString::send_to_other_instance();
				return 1;
			}
		}
		*/

		init_marker_file();
	}

	//
	// Write system information into the debug log (CPU, OS, etc.)
	//
	write_system_info();

#if LL_WINDOWS
	//
	// Turn off Windows XP Error Reporting
	// (Don't send our data to Microsoft--at least until we are Logo approved and have a way
	// of getting the data back from them.)
	//
	llinfos << "Turning off Windows error reporting." << llendl;
	disable_win_error_reporting();
#endif  // LL_WINDOWS

	// Build a string representing the current version number.
	gCurrentVersion = llformat("%d.%d.%d", LL_VERSION_MAJOR, LL_VERSION_MINOR, LL_VERSION_PATCH );
	
	//
	// Load the feature tables
	//
	llinfos << "Loading feature tables." << llendl;
	
	gFeatureManagerp->loadFeatureTables();
	gFeatureManagerp->initCPUFeatureMasks();

	// Merge with the command line overrides
	gSavedSettings.applyOverrides(gCommandLineSettings);

	// Need to do this before calling parseAlerts
	gUICtrlFactory = new LLViewerUICtrlFactory();
	
	// Pre-load alerts.xml to define the warnings settings (always loads from skins/xui/en-us/)
	// Do this *before* loading the settings file
	LLAlertDialog::parseAlerts("alerts.xml", &gSavedSettings, TRUE);
	
	// Overwrite default settings with user settings
	llinfos << "Loading configuration file " << gSettingsFileName << llendl;
	if (0 == gSavedSettings.loadFromFile(gSettingsFileName))
	{
		llinfos << "Failed to load settings from " << gSettingsFileName << llendl;
		llinfos << "Loading legacy settings from " << gOldSettingsFileName << llendl;
		gSavedSettings.loadFromFileLegacy(gOldSettingsFileName);
	}

	// need to do this here - need to have initialized global settings first
	LLString nextLoginLocation = gSavedSettings.getString( "NextLoginLocation" );
	if ( nextLoginLocation.length() )
	{
		LLURLSimString::setString( nextLoginLocation.c_str() );
		gConnectToSomething = TRUE;
	};

	// Merge with the command line overrides
	gSavedSettings.applyOverrides(gCommandLineForcedSettings);

	gLastRunVersion = gSavedSettings.getString("LastRunVersion");

	fixup_settings();
	
	// Get the single value from the crash settings file, if it exists
	std::string crash_settings_filename = gDirUtilp->getExpandedFilename(LL_PATH_USER_SETTINGS, CRASH_SETTINGS_FILE);
	gCrashSettings.loadFromFile(crash_settings_filename.c_str());

	/////////////////////////////////////////////////
	// OS-specific login dialogs
	/////////////////////////////////////////////////
#if LL_WINDOWS
	/*
	// Display initial login screen, comes up quickly. JC
	{
		LLSplashScreen::hide();

		INT_PTR result = DialogBox(hInstance, L"CONNECTBOX", NULL, login_dialog_func);
		if (result < 0)
		{
			llwarns << "Connect dialog box failed, returned " << result << llendl;
			return 1;
		}
		// success, result contains which button user clicked
		llinfos << "Connect dialog box clicked " << result << llendl;

		LLSplashScreen::show();
	}
	*/
#endif

	// track number of times that app has run
	gNumSessions = gSavedSettings.getS32("NumSessions");
	gNumSessions++;
	gSavedSettings.setS32("NumSessions", gNumSessions);

	gSavedSettings.setString("HelpLastVisitedURL",gSavedSettings.getString("HelpHomeURL"));

	if (gSavedSettings.getBOOL("VerboseLogs"))
	{
		LLError::setPrintLocation(true);
	}

#if !LL_RELEASE_FOR_DOWNLOAD
	if (gUserServerChoice == USERSERVER_NONE)
	{
		// Development version: load last server choice by default (overridden by cmd line args)
		if (gSavedSettings.getBOOL("UseDebugLogin"))
		{
			S32 server = gSavedSettings.getS32("ServerChoice");
			if (server != 0)
				gUserServerChoice = (EUserServerDomain)llclamp(server, 0, (S32)USERSERVER_COUNT - 1);
			if (server == USERSERVER_OTHER)
			{
				LLString custom_server = gSavedSettings.getString("CustomServer");
				if (custom_server.empty())
				{
					snprintf(gUserServerName, MAX_STRING, "none");		/* Flawfinder: ignore */
				}
				else
				{
					snprintf(gUserServerName, MAX_STRING, "%s", custom_server.c_str());		/* Flawfinder: ignore */
				}
			}
		}
	}
#endif

	if (gUserServerChoice == USERSERVER_NONE)
	{
		gUserServerChoice = UserServerDefaultChoice;
	}
	
	// Load art UUID information, don't require these strings to be declared in code.
	LLString viewer_art_filename = gDirUtilp->getExpandedFilename(LL_PATH_APP_SETTINGS,"viewerart.xml");
	llinfos << "Loading art table from " << viewer_art_filename << llendl;
	gViewerArt.loadFromFile(viewer_art_filename.c_str(), FALSE);
	LLString textures_filename = gDirUtilp->getExpandedFilename(LL_PATH_SKINS, "textures", "textures.xml");
	llinfos << "Loading art table from " << textures_filename << llendl;
	gViewerArt.loadFromFile(textures_filename.c_str(), FALSE);

	LLString colors_base_filename = gDirUtilp->getExpandedFilename(LL_PATH_APP_SETTINGS, "colors_base.xml");
	llinfos << "Loading base colors from " << colors_base_filename << llendl;
	gColors.loadFromFile(colors_base_filename.c_str(), FALSE, TYPE_COL4U);

	// Load overrides from user colors file
	LLString user_colors_filename = gDirUtilp->getExpandedFilename(LL_PATH_APP_SETTINGS, "colors.xml");
	llinfos << "Loading user colors from " << user_colors_filename << llendl;
	if (gColors.loadFromFile(user_colors_filename.c_str(), FALSE, TYPE_COL4U) == 0)
	{
		llinfos << "Failed to load user colors from " << user_colors_filename << llendl;
		LLString user_legacy_colors_filename = gDirUtilp->getExpandedFilename(LL_PATH_APP_SETTINGS, "colors.ini");
		llinfos << "Loading legacy colors from " << user_legacy_colors_filename << llendl;
		gColors.loadFromFileLegacy(user_legacy_colors_filename.c_str(), FALSE, TYPE_COL4U);
	}

	// Widget construction depends on LLUI being initialized
	LLUI::initClass(&gSavedSettings, 
					&gColors, 
					&gViewerArt,
					&gImageList,
					ui_audio_callback,
					&LLUI::sGLScaleFactor);

	gUICtrlFactory->setupPaths(); // update paths with correct language set
	
	/////////////////////////////////////////////////
	//
	// Load settings files
	//
	//
	LLGroupMgr::parseRoleActions("role_actions.xml");

	LLAgent::parseTeleportMessages("teleport_strings.xml");

	// Move certain saved settings into global variables for speed
	saved_settings_to_globals();

	// 1.6.10: Default crash reporting to on, because this dialog doesn't
	// make any sense for a new user.
	/*
	if (!gSavedSettings.getBOOL("AskedAboutCrashReports"))
	{
		// NOTE: Be sure to use gSecondLife if this becomes uncommented.
		S32 retval = OSMessageBox(
			"Welcome!  You have successfully installed Second Life.\n"
			"\n"
			"Should performance issues or crashes occur, Second Life\n"
			"can automatically send technical data about your computer\n"
			"to Linden Lab to diagnose and correct the problem.\n"
			"This information is used solely to address technical issues\n"
			"and is kept strictly confidential.\n"
			"You can change this setting under Preferences / General.\n"
			"Automatically send technical data if problems occur?",
			"Second Life",
			OSMB_YESNO);
		if (OSBTN_YES == retval)
		{
			gAutoReportCrashes = TRUE;
		}
		else
		{
			gAutoReportCrashes = FALSE;
		}

		// Setting will be saved automatically at shutdown.
		gSavedSettings.setBOOL("AskedAboutCrashReports", TRUE);
	}
	*/

	// Find partition serial number (Windows) or hardware serial (Mac)
	gSerialNumber = get_serial_number();

#if LL_WINDOWS
	//
	// Do driver verification and initialization based on DirectX
	// hardware polling and driver versions
	//
	if (gProbeHardware)
	{
		BOOL vram_only = !gSavedSettings.getBOOL("ProbeHardwareOnStartup");

		LLSplashScreen::update("Detecting hardware...");

		llinfos << "Attempting to poll DirectX for hardware info" << llendl;
		gDXHardware.setWriteDebugFunc(write_debug);
		BOOL probe_ok = gDXHardware.getInfo(vram_only);

		if (!probe_ok
			&& gSavedSettings.getWarning("AboutDirectX9"))
		{
			llinfos << "DirectX probe failed, alerting user." << llendl;

			// Warn them that runnin without DirectX 9 will
			// not allow us to tell them about driver issues
			std::ostringstream msg;
			msg << 
				gSecondLife << " is unable to detect DirectX 9.0b or greater.\n"
				"\n" <<
				gSecondLife << " uses DirectX to detect hardware and/or\n"
				"outdated drivers that can cause stability problems,\n"
				"poor performance and crashes.  While you can run\n" <<
				gSecondLife << " without it, we highly recommend running\n"
				"with DirectX 9.0b\n"
				"\n"
				"Do you wish to continue?\n";
			S32 button = OSMessageBox(
				msg.str().c_str(),
				"Warning",
				OSMB_YESNO);
			if (OSBTN_NO== button)
			{
				llinfos << "User quitting after failed DirectX 9 detection" << llendl;
				LLWeb::loadURLExternal(DIRECTX_9_URL);
				return 0;
			}
			gSavedSettings.setWarning("AboutDirectX9", FALSE);
		}
		llinfos << "Done polling DirectX for hardware info" << llendl;

		// Only probe once after installation
		gSavedSettings.setBOOL("ProbeHardwareOnStartup", FALSE);

		// Disable so debugger can work
		LLSplashScreen::update(splash_msg.str().c_str());
	}

	if (!LLWinDebug::setupExceptionHandler())
	{
		llwarns << " Someone took over my exception handler (post hardware probe)!" << llendl;
	}

	gGLManager.mVRAM = gDXHardware.getVRAM();
	llinfos << "Detected VRAM: " << gGLManager.mVRAM << llendl;
#endif

	// Always fetch the Ethernet MAC address, needed both for login
	// and password load.
	LLUUID::getNodeID(gMACAddress);

	// Prepare for out-of-memory situations, during which we will crash on
	// purpose and save a dump.
#if LL_WINDOWS && LL_RELEASE_FOR_DOWNLOAD && LL_USE_SMARTHEAP
	MemSetErrorHandler(first_mem_error_handler);
#endif // LL_WINDOWS && LL_RELEASE_FOR_DOWNLOAD && LL_USE_SMARTHEAP

	gViewerStats = new LLViewerStats();

	//
	// Initialize the VFS, and gracefully handle initialization errors
	//

	if (!init_cache())
	{
		std::ostringstream msg;
		msg <<
			gSecondLife << " is unable to access a file that it needs.\n"
			"\n"
			"This can be because you somehow have multiple copies running, "
			"or your system incorrectly thinks a file is open. "
			"If this message persists, restart your computer and try again. "
			"If it continues to persist, you may need to completely uninstall " <<
			gSecondLife << " and reinstall it.";
		OSMessageBox(
			msg.str().c_str(),
			NULL,
			OSMB_OK);
		return 1;
	}
	
#if LL_DARWIN
	// Display the release notes for the current version
	if(!gHideLinks && gCurrentVersion != gLastRunVersion)
	{
		// Current version and last run version don't match exactly.  Display the release notes.
		DisplayReleaseNotes();
	}
#endif

	//
	// Initialize the window
	//

	// pop up debug console if necessary
#if LL_WINDOWS
	if (gUseConsole && gSavedSettings.getBOOL("ShowConsoleWindow"))
	{
		create_console();
	}
#endif

	llinfos << "Initializing window..." << llendl;

	// store setting in a global for easy access and modification
	gNoRender = gSavedSettings.getBOOL("DisableRendering");

	// Hide the splash screen
	LLSplashScreen::hide();

	// HACK: Need a non-const char * for stupid window name (propagated deep down)
	char window_title_str[256];		/* Flawfinder: ignore */
	strncpy(window_title_str, gWindowTitle.c_str(), sizeof(window_title_str) - 1);		/* Flawfinder: ignore */
	window_title_str[sizeof(window_title_str) - 1] = '\0';

	// always start windowed
	gViewerWindow = new LLViewerWindow(window_title_str, "Second Life",
		gSavedSettings.getS32("WindowX"), gSavedSettings.getS32("WindowY"),
		gSavedSettings.getS32("WindowWidth"), gSavedSettings.getS32("WindowHeight"),
		FALSE, gIgnorePixelDepth);
		
	if (gSavedSettings.getBOOL("FullScreen"))
	{
		gViewerWindow->toggleFullscreen(FALSE);
			// request to go full screen... which will be delayed until login
	}
	
	if (gSavedSettings.getBOOL("WindowMaximized"))
	{
		gViewerWindow->mWindow->maximize();
		gViewerWindow->getWindow()->setNativeAspectRatio(gSavedSettings.getF32("FullScreenAspectRatio"));
	}

	LLUI::sWindow = gViewerWindow->getWindow();

	LLAlertDialog::parseAlerts("alerts.xml");
	LLNotifyBox::parseNotify("notify.xml");

	LLMediaEngine::initClass();
	
	//
	// Clean up the feature manager lookup table - settings were updated
	// in the LLViewerWindow constructor
	//
	gFeatureManagerp->cleanupFeatureTables();

	// Show watch cursor
	gViewerWindow->setCursor(UI_CURSOR_WAIT);

	// Finish view initialization
	gViewerWindow->initBase();

	// show viewer window
	gViewerWindow->mWindow->show();

	write_debug(gGLManager.getGLInfoString());
	llinfos << gGLManager.getGLInfoString() << llendl;

	//load key settings
	bind_keyboard_functions();

	// Load Default bindings
	if (!gViewerKeyboard.loadBindings(gDirUtilp->getExpandedFilename(LL_PATH_APP_SETTINGS,"keys.ini").c_str()))
	{
		llerrs << "Unable to open keys.ini" << llendl;
	}
	// Load Custom bindings (override defaults)
	gViewerKeyboard.loadBindings(gDirUtilp->getExpandedFilename(LL_PATH_APP_SETTINGS,"custom_keys.ini").c_str());

	// Calculate the digest for the executable (takes < 90ms on a fast machine).
	FILE* app_file = LLFile::fopen( gDirUtilp->getExecutablePathAndName().c_str(), "rb" );		/* Flawfinder: ignore */
	if( app_file )
	{
		LLMD5 app_md5;
		app_md5.update( app_file ); // Automatically closes the file
		app_md5.finalize();
		app_md5.raw_digest( gViewerDigest.mData );
	}
	llinfos << "Viewer Digest: " << gViewerDigest << llendl;

	// If we don't have the right GL requirements, exit.
	// BUG: This should just be changed to a generic GL "Not good enough" flag
	if (!gGLManager.mHasMultitexture && !gNoRender)
	{
		std::ostringstream msg;
		msg <<
			"You do not appear to have the proper hardware requirements "
			"for " << gSecondLife << ". " << gSecondLife << " requires an OpenGL graphics "
			"card that has multitexture support. If this is the case, "
			"you may want to make sure that you have the latest drivers for "
			"your graphics card, and service packs and patches for your "
			"operating system.\n"
			"If you continue to have problems, please go to: "
			"www.secondlife.com/support ";
		OSMessageBox(
			msg.str().c_str(),
			NULL,
			OSMB_OK);
		return 0;
	}

	// Save the current version to the prefs file
	gSavedSettings.setString("LastRunVersion", gCurrentVersion);

	gSimLastTime = gRenderStartTime.getElapsedTimeF32();
	gSimFrames   = (F32)gFrameCount;

	//-------------------------------------------
	// Run main loop until time to quit
	//-------------------------------------------
	main_loop();

	cleanup_app();

	// If we're exiting to launch an URL, do that here so the screen
	// is at the right resolution before we launch IE.
	if (!gLaunchFileOnQuit.empty())
	{
#if LL_WINDOWS
		// Indicate an application is starting.
		SetCursor(LoadCursor(NULL, IDC_WAIT));
#endif

		// HACK: Attempt to wait until the screen res. switch is complete.
		ms_sleep(1000);

		LLWeb::loadURLExternal( gLaunchFileOnQuit );
	}

	llinfos << "Goodbye" << llendflush;
	return 0;
}

BOOL another_instance_running()
{
	// We create a marker file when the program starts and remove the file when it finishes.
	// If the file is currently locked, that means another process is already running.

	std::string marker_file = gDirUtilp->getExpandedFilename(LL_PATH_LOGS,"SecondLife.exec_marker");
	llinfos << "Checking marker file for lock..." << llendl;

	// If file doesn't exist, we create it
	// If file does exist, try to get writing privilages
	FILE* fMarker = LLFile::fopen(marker_file.c_str(), "rb");		/* Flawfinder: ignore */
	if (fMarker != NULL)
	{
		// File exists, try opening with write permissions
		fclose(fMarker);
		fMarker = LLFile::fopen(marker_file.c_str(), "wb");		/* Flawfinder: ignore */
		if (fMarker == NULL)
		{
			llinfos << "Marker file is locked." << llendl;
			return TRUE;
		}
#if LL_DARWIN
		// Try to lock it. On Mac, this is the only way to test if it's actually locked.
		if (flock(fileno(fMarker), LOCK_EX | LOCK_NB) == -1)
		{
			// Lock failed - somebody else has it.
			fclose(fMarker);
			llinfos << "Marker file is locked." << llendl;
			return TRUE;
		}
#endif
		fclose(fMarker);
	}
	llinfos << "Marker file isn't locked." << llendl;
	return FALSE;
}


void check_for_events()
{
	LLFastTimer t2(LLFastTimer::FTM_MESSAGES);
#if LL_WINDOWS
	if (!LLWinDebug::setupExceptionHandler())
	{
		llwarns << " Someone took over my exception handler (post messagehandling)!" << llendl;
	}
#endif

	gViewerWindow->mWindow->gatherInput();
}

#include "moviemaker.h"
extern BOOL gbCapturing;
extern MovieMaker gMovieMaker;

void main_loop()
{
	// Create IO Pump to use for HTTP Requests.
	gServicePump = new LLPumpIO(gAPRPoolp);
	LLHTTPClient::setPump(*gServicePump);
	LLHTTPClient::setCABundle(gDirUtilp->getCAFile());

	LLMemType mt1(LLMemType::MTYPE_MAIN);
	LLTimer frameTimer,idleTimer;
	LLTimer debugTime;
	
	// Handle messages
	while (!gQuit)
	{
		LLFastTimer::reset(); // Should be outside of any timer instances
		{
			LLFastTimer t(LLFastTimer::FTM_FRAME);

			check_for_events();

#if 1 && !RELEASE_FOR_DOWNLOAD
			// once per second debug info
			if (debugTime.getElapsedTimeF32() > 1.f)
			{
				debugTime.reset();
			}
#endif
			if (!gQuit)
			{
				// Scan keyboard for movement keys.  Command keys and typing
				// are handled by windows callbacks.  Don't do this until we're
				// done initializing.  JC
				if (gViewerWindow->mWindow->getVisible() 
					&& gViewerWindow->getActive()
					&& !gViewerWindow->mWindow->getMinimized()
					&& gStartupState == STATE_STARTED
					&& !gViewerWindow->getShowProgress()
					&& !gFocusMgr.focusLocked())
				{
					gKeyboard->scanKeyboard();
					LLViewerJoystick::scanJoystick();
				}

				// Update state based on messages, user input, object idle.
				{
					LLFastTimer t3(LLFastTimer::FTM_IDLE);
					idle();
					LLCurl::process();
					// this pump is necessary to make the login screen show up
					gServicePump->pump();
					gServicePump->callback();
				}

				if (gDoDisconnect && (gStartupState == STATE_STARTED))
				{
					save_final_snapshot(NULL);
					disconnect_viewer(NULL);
				}

				// Render scene.
				if (!gQuit)
				{
					display();

					LLFloaterSnapshot::update(); // take snapshots
					
					if (gbCapturing)
					{
						gMovieMaker.Snap();
					}
				}

			}

			// Sleep and run background threads
			{
				LLFastTimer t2(LLFastTimer::FTM_SLEEP);
				bool run_multiple_threads = gSavedSettings.getBOOL("RunMultipleThreads");

				// yield some time to the os based on command line option
				if(gYieldTime)
				{
					ms_sleep(gYieldMS);
				}

				// yield cooperatively when not running as foreground window
				if (   gNoRender
						|| !gViewerWindow->mWindow->getVisible()
						|| !gFocusMgr.getAppHasFocus())
				{
					// Sleep if we're not rendering, or the window is minimized.
					S32 milliseconds_to_sleep = llclamp(gSavedSettings.getS32("BackgroundYieldTime"), 0, 1000);
					// don't sleep when BackgroundYieldTime set to 0, since this will still yield to other threads
					// of equal priority on Windows
					if (milliseconds_to_sleep > 0)
					{
						ms_sleep(milliseconds_to_sleep);
						// also pause worker threads during this wait period
						gTextureCache->pause();
						gImageDecodeThread->pause();
					}
				}
				
				if (gRandomizeFramerate)
				{
					ms_sleep(rand() % 200);
				}

				if (gPeriodicSlowFrame
					&& (gFrameCount % 10 == 0))
				{
					llinfos << "Periodic slow frame - sleeping 500 ms" << llendl;
					ms_sleep(500);
				}


				const F64 min_frame_time = 0.0; //(.0333 - .0010); // max video frame rate = 30 fps
				const F64 min_idle_time = 0.0; //(.0010); // min idle time = 1 ms
				const F64 max_idle_time = run_multiple_threads ? min_idle_time : .005; // 5 ms
				idleTimer.reset();
				while(1)
				{
					S32 work_pending = 0;
					S32 io_pending = 0;
 					work_pending += gTextureCache->update(1); // unpauses the texture cache thread
 					work_pending += gImageDecodeThread->update(1); // unpauses the image thread
 					work_pending += gTextureFetch->update(1); // unpauses the texture fetch thread
					io_pending += LLVFSThread::updateClass(1);
					io_pending += LLLFSThread::updateClass(1);
					if (io_pending > 1000)
					{
						ms_sleep(llmin(io_pending/100,100)); // give the vfs some time to catch up
					}

					F64 frame_time = frameTimer.getElapsedTimeF64();
					F64 idle_time = idleTimer.getElapsedTimeF64();
					if (frame_time >= min_frame_time &&
						idle_time >= min_idle_time &&
						(!work_pending || idle_time >= max_idle_time))
					{
						break;
					}
				}
				frameTimer.reset();

				 // Prevent the worker threads from running while rendering.
				// if (LLThread::processorCount()==1) //pause() should only be required when on a single processor client...
				if (run_multiple_threads == FALSE)
				{
					gTextureCache->pause();
					gImageDecodeThread->pause();
					// gTextureFetch->pause(); // Don't pause the fetch (IO) thread
				}
				//LLVFSThread::sLocal->pause(); // Prevent the VFS thread from running while rendering.
				//LLLFSThread::sLocal->pause(); // Prevent the LFS thread from running while rendering.
			}
						
		}
	}

	// Save snapshot for next time, if we made it through initialization
	if (STATE_STARTED == gStartupState)
	{
		save_final_snapshot(NULL);
	}
	
	delete gServicePump;

	llinfos << "Exiting main_loop" << llendflush;
}


void process_keystrokes_async()
{
#if LL_WINDOWS
	MSG			msg;
	// look through all input messages, leaving them in the event queue
	while( PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE | PM_NOYIELD))
	{
		// on first mouse message, break out
		if (msg.message >= WM_MOUSEFIRST && 
			msg.message <= WM_MOUSELAST ||
			msg.message == WM_QUIT)
		{
			break;
		}

		// this is a message we want to handle now, so remove it from the event queue
		PeekMessage(&msg, NULL, msg.message, msg.message, PM_REMOVE | PM_NOYIELD);
		//		if (msg.message == WM_KEYDOWN)
		//		{
		//			llinfos << "Process async key down " << (U32)msg.wParam << llendl;
		//		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// Scan keyboard for movement keys.  Command keys and typing
	// are handled by windows callbacks.  Don't do this until we're
	// done initializing.  JC
	if (gViewerWindow->mWindow->getVisible() 
		&& gViewerWindow->getActive()
		&& !gViewerWindow->mWindow->getMinimized()
		&& gStartupState == STATE_STARTED
		&& !gViewerWindow->getShowProgress()
		&& !gFocusMgr.focusLocked())
	{
		gKeyboard->scanKeyboard();
	}
#endif
}

////////////////////////////////////////////////
//
// Functions
//

FILE *gMarkerFile = NULL;

void remove_marker_file()
{
	llinfos << "remove_marker_file()" << llendl;
	if (gMarkerFile != NULL)
	{
		fclose(gMarkerFile);
		gMarkerFile = NULL;
	}
	if( gDirUtilp )
	{
		LLString marker_file = gDirUtilp->getExpandedFilename(LL_PATH_LOGS,"SecondLife.exec_marker");
		ll_apr_file_remove( marker_file );
	}
}

void init_marker_file()
{
	// We create a marker file when the program starts and remove the file when it finishes.
	// If the file is currently locked, that means another process is already running.
	// If the file exists and isn't locked, we crashed on the last run.

	std::string marker_file = gDirUtilp->getExpandedFilename(LL_PATH_LOGS,"SecondLife.exec_marker");
	llinfos << "Checking marker file for lock..." << llendl;

	FILE* fMarker = LLFile::fopen(marker_file.c_str(), "rb");		/* Flawfinder: ignore */
	if (fMarker != NULL)
	{
		// File exists, try opening with write permissions
		fclose(fMarker);
		fMarker = LLFile::fopen(marker_file.c_str(), "wb");		/* Flawfinder: ignore */
		if (fMarker == NULL)
		{
			// Another instance is running. Skip the rest of these operations.
			llinfos << "Marker file is locked." << llendl;
			return;
		}
#if LL_DARWIN
		// Try to lock it. On Mac, this is the only way to test if it's actually locked.
		if (flock(fileno(fMarker), LOCK_EX | LOCK_NB) == -1)
		{
			// Lock failed - somebody else has it.
			fclose(fMarker);
			llinfos << "Marker file is locked." << llendl;
			return;
		}
#endif

		// No other instances; we'll lock this file now & delete on quit.
		fclose(fMarker);
		gLastExecFroze = TRUE;
		llinfos << "Exec marker found: program froze on previous execution" << llendl;
	}

	// Create the marker file for this execution & lock it
// 	FILE *fp_executing_marker;
#if LL_WINDOWS
	gMarkerFile = LLFile::_fsopen(marker_file.c_str(), "w", _SH_DENYWR);
#else
	gMarkerFile = LLFile::fopen(marker_file.c_str(), "w");		/* Flawfinder: ignore */
	if (gMarkerFile)
	{
		int fd = fileno(gMarkerFile);
		// Attempt to lock
		if (flock(fd, LOCK_EX | LOCK_NB) == -1)
		{
			llinfos << "Failed to lock file." << llendl;
		}
	}
#endif
	if (gMarkerFile)
	{
		llinfos << "Marker file created." << llendl;
	}
	else
	{
		llinfos << "Failed to create marker file." << llendl;
	}

#if LL_WINDOWS
	// Clean up SecondLife.dmp files, to avoid confusion
	llinfos << "Removing SecondLife.dmp" << llendl;
	std::string dmp_filename = gDirUtilp->getExpandedFilename(LL_PATH_LOGS, "SecondLife.dmp");
	LLFile::remove(dmp_filename.c_str());
#endif

	// This is to keep the crash reporter from constantly sending stale message logs
	// We wipe the message file now.
	llinfos << "Removing message.log" << llendl;
	std::string message_filename = gDirUtilp->getExpandedFilename(LL_PATH_LOGS, "message.log");
	LLFile::remove(message_filename.c_str());

	llinfos << "Exiting init_marker_file()." << llendl;
}

void init_crash_handler()
{
	//////////////////////////////////////////
	//
	// Set up error handling and logging for LL_RELEASE_FOR_DOWNLOAD
	//

	// Signal handling (or Win32 exception handling
	catch_signals();

	// Set the crash callback for the viewer
	gCrashCallback = viewer_crash_callback;
}

void init_logging()
{
	// Remove the last ".old" log file.
	std::string old_log_file = gDirUtilp->getExpandedFilename(LL_PATH_LOGS,
							     "SecondLife.old");
	LLFile::remove(old_log_file.c_str());

#if LL_LINUX
	// Remove the last stack trace, if any
	std::string old_stack_file =
		gDirUtilp->getExpandedFilename(LL_PATH_LOGS,"stack_trace.log");
	LLFile::remove(old_stack_file.c_str());
#endif // LL_LINUX

	// Rename current log file to ".old"
	std::string log_file = gDirUtilp->getExpandedFilename(LL_PATH_LOGS,
							     "SecondLife.log");
	LLFile::rename(log_file.c_str(), old_log_file.c_str());

	// Set the log file to SecondLife.log

	LLError::logToFile(log_file);
}

void write_system_info()
{
	write_debug("SL Log: ");
	write_debug(LLError::logFileName());
	write_debug("\n");

	std::string tmp_str = gSecondLife
		+ llformat(" version %d.%d.%d build %d",
				   LL_VERSION_MAJOR, LL_VERSION_MINOR, LL_VERSION_PATCH, LL_VIEWER_BUILD);
	write_debug(tmp_str.c_str());
	write_debug("\n");
	write_debug(gSysCPU.getCPUString());
	write_debug("\n");
	
	tmp_str = llformat("RAM: %u\n", gSysMemory.getPhysicalMemory());
	write_debug(tmp_str.c_str());
	write_debug("OS: ");
	write_debug(gSysOS.getOSString().c_str());
	write_debug("\n");

	// Dump some debugging info
	llinfos << gSecondLife << " version "
		<< LL_VERSION_MAJOR << "."
		<< LL_VERSION_MINOR << "."
		<< LL_VERSION_PATCH
		<< llendl;

	// Dump the local time and time zone
	time_t now;
	time(&now);
	char tbuffer[256];		/* Flawfinder: ignore */
	strftime(tbuffer, 256, "%Y-%m-%dT%H:%M:%S %Z", localtime(&now));
	llinfos << "Local time: " << tbuffer << llendl;

	// query some system information
	llinfos << "CPU info:\n" << gSysCPU << llendl;
	llinfos << "Memory info:\n" << gSysMemory << llendl;
	llinfos << "OS info: " << gSysOS << llendl;
}

#if LL_WINDOWS
void disable_win_error_reporting()
{
	const char win_xp_string[] = "Microsoft Windows XP";
	BOOL is_win_xp = ( gSysOS.getOSString().substr(0, strlen(win_xp_string) ) == win_xp_string );		/* Flawfinder: ignore*/
	if( is_win_xp )
	{
		// Note: we need to use run-time dynamic linking, because load-time dynamic linking will fail
		// on systems that don't have the library installed (all non-Windows XP systems)
		HINSTANCE fault_rep_dll_handle = LoadLibrary(L"faultrep.dll");		/* Flawfinder: ignore */
		if( fault_rep_dll_handle )
		{
			pfn_ADDEREXCLUDEDAPPLICATIONA pAddERExcludedApplicationA  = (pfn_ADDEREXCLUDEDAPPLICATIONA) GetProcAddress(fault_rep_dll_handle, "AddERExcludedApplicationA");
			if( pAddERExcludedApplicationA )
			{

				// Strip the path off the name
				const char* executable_name = gDirUtilp->getExecutableFilename().c_str();

				if( 0 == pAddERExcludedApplicationA( executable_name ) )
				{
					U32 error_code = GetLastError();
					llinfos << "AddERExcludedApplication() failed with error code " << error_code << llendl;
				}
				else
				{
					llinfos << "AddERExcludedApplication() success for " << executable_name << llendl;
				}
			}
			FreeLibrary( fault_rep_dll_handle );
		}
	}
}
#endif  // LL_WINDOWS

// On Windows, get the C:\ volume serial number.
// On Mac, return the hardware serial number.
std::string get_serial_number()
{
	char serial_md5[MD5HEX_STR_SIZE];		/* Flawfinder: ignore */
	serial_md5[0] = 0;

#if LL_WINDOWS
	DWORD serial = 0;
	DWORD flags = 0;
	BOOL success = GetVolumeInformation(
			L"C:\\",
			NULL,		// volume name buffer
			0,			// volume name buffer size
			&serial,	// volume serial
			NULL,		// max component length
			&flags,		// file system flags
			NULL,		// file system name buffer
			0);			// file system name buffer size
	if (success)
	{
		LLMD5 md5;
		md5.update( (unsigned char*)&serial, sizeof(DWORD));
		md5.finalize();
		md5.hex_digest(serial_md5);
	}
	else
	{
		llwarns << "GetVolumeInformation failed" << llendl;
	}
	return serial_md5;

#elif LL_DARWIN
	// JC: Sample code from http://developer.apple.com/technotes/tn/tn1103.html
	CFStringRef serialNumber = NULL;
	io_service_t    platformExpert = IOServiceGetMatchingService(kIOMasterPortDefault,
																 IOServiceMatching("IOPlatformExpertDevice"));
	if (platformExpert) {
		serialNumber = (CFStringRef) IORegistryEntryCreateCFProperty(platformExpert,
																	 CFSTR(kIOPlatformSerialNumberKey),
																	 kCFAllocatorDefault, 0);		
		IOObjectRelease(platformExpert);
	}
	
	if (serialNumber)
	{
		char buffer[MAX_STRING];		/* Flawfinder: ignore */
		if (CFStringGetCString(serialNumber, buffer, MAX_STRING, kCFStringEncodingASCII))
		{
			LLMD5 md5( (unsigned char*)buffer );
			md5.hex_digest(serial_md5);
		}
		CFRelease(serialNumber);
	}

	return serial_md5;

#elif LL_LINUX
	// TODO
	return serial_md5;

#endif
}

#if LL_LINUX
#define MAX_STACK_TRACE_DEPTH 40
// This uses glibc's basic built-in stack-trace functions for a not very
// amazing backtrace.
static inline BOOL do_basic_glibc_backtrace()
{
	void *array[MAX_STACK_TRACE_DEPTH];
	size_t size;
	char **strings;
	size_t i;
	BOOL success = FALSE;

	size = backtrace(array, MAX_STACK_TRACE_DEPTH);
	strings = backtrace_symbols(array, size);

	std::string strace_filename = gDirUtilp->getExpandedFilename(LL_PATH_LOGS,"stack_trace.log");
	llinfos << "Opening stack trace file " << strace_filename << llendl;
	FILE* StraceFile = LLFile::fopen(strace_filename.c_str(), "w");		/* Flawfinder: ignore */
        if (!StraceFile)
	{
		llinfos << "Opening stack trace file " << strace_filename << " failed. Using stderr." << llendl;
		StraceFile = stderr;
	}

	if (size)
	{
		for (i = 0; i < size; i++)
			fputs((std::string(strings[i])+"\n").c_str(),
			      StraceFile);

		success = TRUE;
	}
	
	if (StraceFile != stderr)
		fclose(StraceFile);

	free (strings);
	return success;
}

#if LL_ELFBIN
// This uses glibc's basic built-in stack-trace functions together with
// ELFIO's ability to parse the .symtab ELF section for better symbol
// extraction without exporting symbols (which'd cause subtle, fatal bugs).
static inline BOOL do_elfio_glibc_backtrace()
{
	void *array[MAX_STACK_TRACE_DEPTH];
	size_t btsize;
	char **strings;
	BOOL success = FALSE;

	std::string appfilename = gDirUtilp->getExecutablePathAndName();

	std::string strace_filename = gDirUtilp->getExpandedFilename(LL_PATH_LOGS,"stack_trace.log");
	llinfos << "Opening stack trace file " << strace_filename << llendl;
	FILE* StraceFile = LLFile::fopen(strace_filename.c_str(), "w");		/* Flawfinder: ignore */
        if (!StraceFile)
	{
		llinfos << "Opening stack trace file " << strace_filename << " failed. Using stderr." << llendl;
		StraceFile = stderr;
	}

	// get backtrace address list and basic symbol info
	btsize = backtrace(array, MAX_STACK_TRACE_DEPTH);
	strings = backtrace_symbols(array, btsize);

	// create ELF reader for our app binary
	IELFI* pReader;
	const IELFISection* pSec = NULL;
	IELFISymbolTable* pSymTbl = 0;
	if (ERR_ELFIO_NO_ERROR != ELFIO::GetInstance()->CreateELFI(&pReader) ||
	    ERR_ELFIO_NO_ERROR != pReader->Load(appfilename.c_str()) ||
	    // find symbol table, create reader-object
	    NULL == (pSec = pReader->GetSection( ".symtab" )) ||
	    ERR_ELFIO_NO_ERROR != pReader->CreateSectionReader(IELFI::ELFI_SYMBOL, pSec, (void**)&pSymTbl) )
	{
		// Failed to open our binary and read its symbol table somehow
		llinfos << "Could not initialize ELF symbol reading - doing basic backtrace." << llendl;
		if (StraceFile != stderr)
			fclose(StraceFile);
		// note that we may be leaking some of the above ELFIO
		// objects now, but it's expected that we'll be dead soon
		// and we want to tread delicately until we get *some* kind
		// of useful backtrace.
		return do_basic_glibc_backtrace();
	}

	// iterate over trace and symtab, looking for plausible symbols
	std::string   name;
	Elf32_Addr    value;
	Elf32_Word    ssize;
	unsigned char bind;
	unsigned char type;
	Elf32_Half    section;
	int nSymNo = pSymTbl->GetSymbolNum();
	size_t btpos;
	for (btpos = 0; btpos < btsize; ++btpos)
	{
		fprintf(StraceFile, "%d:\t", btpos);
		int symidx;
		for (symidx = 0; symidx < nSymNo; ++symidx)
		{
			if (ERR_ELFIO_NO_ERROR ==
			    pSymTbl->GetSymbol(symidx, name, value, ssize,
					       bind, type, section))
			{
				// check if trace address within symbol range
				if (uintptr_t(array[btpos]) >= value &&
				    uintptr_t(array[btpos]) < value+ssize)
				{
					char *demangled_str = NULL;
					int demangle_result = 1;
					demangled_str =
						abi::__cxa_demangle
						(name.c_str(), NULL, NULL,
						 &demangle_result);
					if (0 == demangle_result &&
					    NULL != demangled_str) {
						fprintf(StraceFile,
							"ELF(%s", demangled_str);
						free(demangled_str);
					}
					else // failed demangle; print it raw
					{
						fprintf(StraceFile,
							"ELF(%s", name.c_str());
					}
					// print offset from symbol start
					fprintf(StraceFile,
						"+0x%x) [%p]\n",
						uintptr_t(array[btpos]) -
						value,
						array[btpos]);
					goto got_sym; // early escape
				}
			}
		}
		// Fallback:
		// Didn't find a suitable symbol in the binary - it's probably
		// a symbol in a DSO; use glibc's idea of what it should be.
		fprintf(StraceFile, "%s\n", strings[btpos]);
	got_sym:;
	}
	
	if (StraceFile != stderr)
		fclose(StraceFile);

	pSymTbl->Release();
	pSec->Release();
	pReader->Release();

	free(strings);

	llinfos << "Finished generating stack trace." << llendl;

	success = TRUE;
	return success;
}
#endif // LL_ELFBIN
#endif // LL_LINUX

void viewer_crash_callback()
{
	// Returns whether a dialog was shown.
	// Only do the logic in here once
	if (gReportedCrash)
	{
		return;
	}
	gReportedCrash = TRUE;

	BOOL do_crash_report = FALSE;

	do_crash_report = TRUE;

	write_debug("Viewer exe: ");
	write_debug(gDirUtilp->getExecutablePathAndName().c_str());
	write_debug("\n");
	write_debug("Cur path: ");
	write_debug(gDirUtilp->getCurPath().c_str());
	write_debug("\n\n");

	if (gMessageSystem && gDirUtilp)
	{
		std::string filename;
		filename = gDirUtilp->getExpandedFilename(LL_PATH_LOGS, "stats.log");
		llofstream file(filename.c_str(), llofstream::binary);
		if(file.good())
		{
			gMessageSystem->summarizeLogs(file);
		}
	}

	if (gMessageSystem)
	{
		write_debug(gMessageSystem->getCircuitInfoString());
		gMessageSystem->stopLogging();
	}
	write_debug("\n");
	if (gWorldp)
	{
		write_debug(gWorldp->getInfoString());
	}

	// Close the debug file
	close_debug();
	LLError::logToFile("");

	// Close the SecondLife.log

	remove_marker_file();

#if LL_WINDOWS
	// Windows
	std::string exe_path = gDirUtilp->getAppRODataDir();
	exe_path += gDirUtilp->getDirDelimiter();
	exe_path += "win_crash_logger.exe";

	std::string arg_string = "-user ";
	arg_string += gUserServerName;

	switch(gCrashBehavior)
	{
	case CRASH_BEHAVIOR_ASK:
	default:
		arg_string += " -dialog ";
		_spawnl(_P_NOWAIT, exe_path.c_str(), exe_path.c_str(), arg_string.c_str(), NULL);
		break;

	case CRASH_BEHAVIOR_ALWAYS_SEND:
		_spawnl(_P_NOWAIT, exe_path.c_str(), exe_path.c_str(), arg_string.c_str(), NULL);
		break;

	case CRASH_BEHAVIOR_NEVER_SEND:
		break;
	}

#elif LL_DARWIN
	// Macintosh
	LLString command_str;
	command_str = "crashreporter.app/Contents/MacOS/crashreporter ";
	command_str += "-user ";
	command_str += gUserServerName;
	command_str += " &";	// This backgrounds the command so system() doesn't block until the crashreporter exits.
	system(command_str.c_str());		/* Flawfinder: ignore */
		
	// Sometimes signals don't seem to quit the viewer.  
	// Make sure we exit so as to not totally confuse the user.
	exit(1);
#elif LL_LINUX
	// Always generate the report, have the logger do the asking, and
	// don't wait for the logger before exiting (-> total cleanup).
	if (CRASH_BEHAVIOR_NEVER_SEND != gCrashBehavior)
	{	
		// This backtrace writes into stack_trace.log
#  if LL_ELFBIN
		do_elfio_glibc_backtrace(); // more useful backtrace
#  else
		do_basic_glibc_backtrace(); // only slightly useful backtrace
#  endif // LL_ELFBIN
		// launch the actual crash logger
		char* ask_dialog = "-dialog";
		if (CRASH_BEHAVIOR_ASK != gCrashBehavior)
			ask_dialog = ""; // omit '-dialog' option
		std::string cmd =gDirUtilp->getAppRODataDir();
		cmd += gDirUtilp->getDirDelimiter();
		cmd += "linux-crash-logger.bin";
		char* const cmdargv[] =
			{(char*)cmd.c_str(),
			 ask_dialog,
			 (char*)"-user",
			 (char*)gUserServerName,
			 (char*)"-name",
			 (char*)gSecondLife.c_str(),
			 NULL};
		pid_t pid = fork();
		if (pid == 0)
		{ // child
			execv(cmd.c_str(), cmdargv);		/* Flawfinder: ignore */
			llwarns << "execv failure when trying to start " << cmd << llendl;
			_exit(1); // avoid atexit()
		} else {
			if (pid > 0)
			{
				// DO NOT wait for child proc to die; we want
				// the logger to outlive us while we quit to
				// free up the screen/keyboard/etc.
				////int childExitStatus;
				////waitpid(pid, &childExitStatus, 0);
			} else {
				llwarns << "fork failure." << llendl;
			}
		}
	}
	// Sometimes signals don't seem to quit the viewer.  
	// Make sure we exit so as to not totally confuse the user.
	exit(1);
#else
    #error do something with your platform.
#endif // LL_DARWIN

	return;
}



BOOL init_cache()
{
	gPurgeCache = FALSE;
	// Purge cache if user requested it
	if (gSavedSettings.getBOOL("PurgeCacheOnStartup") ||
		gSavedSettings.getBOOL("PurgeCacheOnNextStartup"))
	{
		gSavedSettings.setBOOL("PurgeCacheOnNextStartup", FALSE);
		gPurgeCache = TRUE;
	}
	// Purge cache if it belongs to an old version
	else
	{
		static const S32 cache_version = 5;
		if (gSavedSettings.getS32("LocalCacheVersion") != cache_version)
		{
			gPurgeCache = TRUE;
			gSavedSettings.setS32("LocalCacheVersion", cache_version);
		}
	}
	
	// Setup and verify the cache location
	LLString cache_location = gSavedSettings.getString("CacheLocation");
	LLString new_cache_location = gSavedSettings.getString("NewCacheLocation");
	if (new_cache_location != cache_location)
	{
		gDirUtilp->setCacheDir(gSavedSettings.getString("CacheLocation"));
		purge_cache(); // purge old cache
		gSavedSettings.setString("CacheLocation", new_cache_location);
	}
	
	if (!gDirUtilp->setCacheDir(gSavedSettings.getString("CacheLocation")))
	{
		llwarns << "Unable to set cache location" << llendl;
		gSavedSettings.setString("CacheLocation", "");
	}
	
	if (gPurgeCache)
	{
		LLSplashScreen::update("Clearing cache...");
		purge_cache();
	}

	LLSplashScreen::update("Initializing Texture Cache...");
	
	// Init the texture cache
	// Allocate 80% of the cache size for textures
	BOOL read_only = gSecondInstance ? TRUE : FALSE;
	const S32 MB = 1024*1024;
	S64 cache_size = (S64)(gSavedSettings.getU32("CacheSize")) * MB;
	const S64 MAX_CACHE_SIZE = 1024*MB;
	cache_size = llmin(cache_size, MAX_CACHE_SIZE);
	S64 texture_cache_size = ((cache_size * 8)/10);
	S64 extra = gTextureCache->initCache(LL_PATH_CACHE, texture_cache_size, read_only);
	texture_cache_size -= extra;

	LLSplashScreen::update("Initializing VFS...");
	
	// Init the VFS
	S64 vfs_size = cache_size - texture_cache_size;
	const S64 MAX_VFS_SIZE = 1024 * MB; // 1 GB
	vfs_size = llmin(vfs_size, MAX_VFS_SIZE);
	vfs_size = (vfs_size / MB) * MB; // make sure it is MB aligned
	U32 vfs_size_u32 = (U32)vfs_size;
	U32 old_vfs_size = gSavedSettings.getU32("VFSOldSize") * MB;
	bool resize_vfs = (vfs_size_u32 != old_vfs_size);
	if (resize_vfs)
	{
		gSavedSettings.setU32("VFSOldSize", vfs_size_u32/MB);
	}
	llinfos << "VFS CACHE SIZE: " << vfs_size/(1024*1024) << " MB" << llendl;
	
	// This has to happen BEFORE starting the vfs
	//time_t	ltime;
	srand(time(NULL));		/* Flawfinder: ignore */
	U32 old_salt = gSavedSettings.getU32("VFSSalt");
	U32 new_salt;
	char old_vfs_data_file[LL_MAX_PATH];		/* Flawfinder: ignore */
	char old_vfs_index_file[LL_MAX_PATH];	/* Flawfinder: ignore */		
	char new_vfs_data_file[LL_MAX_PATH];		/* Flawfinder: ignore */
	char new_vfs_index_file[LL_MAX_PATH];	/* Flawfinder: ignore */
	char static_vfs_index_file[LL_MAX_PATH];	/* Flawfinder: ignore */
	char static_vfs_data_file[LL_MAX_PATH];	/* Flawfinder: ignore */

	if (gMultipleViewersOK)
	{
		// don't mess with renaming the VFS in this case
		new_salt = old_salt;
	}
	else
	{
		do
		{
			new_salt = rand();
		} while( new_salt == old_salt );
	}

	snprintf(old_vfs_data_file,  LL_MAX_PATH, "%s%u",		/* Flawfinder: ignore */
		gDirUtilp->getExpandedFilename(LL_PATH_CACHE,VFS_DATA_FILE_BASE).c_str(),
		old_salt);

	// make sure this file exists
	llstat s;
	S32 stat_result = LLFile::stat(old_vfs_data_file, &s);
	if (stat_result)
	{
		// doesn't exist, look for a data file
		std::string mask;
		mask = gDirUtilp->getDirDelimiter();
		mask += VFS_DATA_FILE_BASE;
		mask += "*";

		std::string dir;
		dir = gDirUtilp->getExpandedFilename(LL_PATH_CACHE,"");

		std::string found_file;
		if (gDirUtilp->getNextFileInDir(dir, mask, found_file, FALSE))
		{
			snprintf(old_vfs_data_file, LL_MAX_PATH, "%s%s%s", dir.c_str(), gDirUtilp->getDirDelimiter().c_str(), found_file.c_str());		/* Flawfinder: ignore */

			S32 start_pos;
			S32 length = strlen(found_file.c_str());		/* Flawfinder: ignore*/
			for (start_pos = length - 1; start_pos >= 0; start_pos--)
			{
				if (found_file[start_pos] == '.')
				{
					start_pos++;
					break;
				}
			}
			if (start_pos > 0)
			{
				sscanf(found_file.c_str() + start_pos, "%d", &old_salt);
			}
			llinfos << "Default vfs data file not present, found " << old_vfs_data_file << llendl;
			llinfos << "Old salt: " << old_salt << llendl;
		}
	}

	snprintf(old_vfs_index_file, LL_MAX_PATH, "%s%u",		/* Flawfinder: ignore */
			gDirUtilp->getExpandedFilename(LL_PATH_CACHE,VFS_INDEX_FILE_BASE).c_str(),
			old_salt);

	stat_result = LLFile::stat(old_vfs_index_file, &s);
	if (stat_result)
	{
		// We've got a bad/missing index file, nukem!
		llwarns << "Bad or missing vfx index file " << old_vfs_index_file << llendl;
		llwarns << "Removing old vfs data file " << old_vfs_data_file << llendl;
		LLFile::remove(old_vfs_data_file);
		LLFile::remove(old_vfs_index_file);
		
		// Just in case, nuke any other old cache files in the directory.
		std::string dir;
		dir = gDirUtilp->getExpandedFilename(LL_PATH_CACHE,"");

		std::string mask;
		mask = gDirUtilp->getDirDelimiter();
		mask += VFS_DATA_FILE_BASE;
		mask += "*";

		gDirUtilp->deleteFilesInDir(dir, mask);

		mask = gDirUtilp->getDirDelimiter();
		mask += VFS_INDEX_FILE_BASE;
		mask += "*";

		gDirUtilp->deleteFilesInDir(dir, mask);
	}

	snprintf(new_vfs_data_file, LL_MAX_PATH, "%s%u",		/* Flawfinder: ignore */
		gDirUtilp->getExpandedFilename(LL_PATH_CACHE,VFS_DATA_FILE_BASE).c_str(),
		new_salt);

	snprintf(new_vfs_index_file, LL_MAX_PATH, "%s%u", gDirUtilp->getExpandedFilename(LL_PATH_CACHE, VFS_INDEX_FILE_BASE).c_str(),		/* Flawfinder: ignore */
		new_salt);


	strncpy(static_vfs_data_file, gDirUtilp->getExpandedFilename(LL_PATH_APP_SETTINGS,"static_data.db2").c_str(), LL_MAX_PATH -1);		/* Flawfinder: ignore */
	static_vfs_data_file[LL_MAX_PATH -1] = '\0';
	strncpy(static_vfs_index_file, gDirUtilp->getExpandedFilename(LL_PATH_APP_SETTINGS,"static_index.db2").c_str(), LL_MAX_PATH -1);		/* Flawfinder: ignore */
	static_vfs_index_file[LL_MAX_PATH -1] = '\0';

	if (resize_vfs)
	{
		llinfos << "Removing old vfs and re-sizing" << llendl;
		
		LLFile::remove(old_vfs_data_file);
		LLFile::remove(old_vfs_index_file);
	}
	else if (old_salt != new_salt)
	{
		// move the vfs files to a new name before opening
		llinfos << "Renaming " << old_vfs_data_file << " to " << new_vfs_data_file << llendl;
		llinfos << "Renaming " << old_vfs_index_file << " to " << new_vfs_index_file << llendl;
		LLFile::rename(old_vfs_data_file, new_vfs_data_file);
		LLFile::rename(old_vfs_index_file, new_vfs_index_file);
	}

	// Startup the VFS...
	gSavedSettings.setU32("VFSSalt", new_salt);

	// Don't remove VFS after viewer crashes.  If user has corrupt data, they can reinstall. JC
	gVFS = new LLVFS(new_vfs_index_file, new_vfs_data_file, FALSE, vfs_size_u32, FALSE);
	if( VFSVALID_BAD_CORRUPT == gVFS->getValidState() )
	{
		// Try again with fresh files 
		// (The constructor deletes corrupt files when it finds them.)
		llwarns << "VFS corrupt, deleted.  Making new VFS." << llendl;
		delete gVFS;
		gVFS = new LLVFS(new_vfs_index_file, new_vfs_data_file, FALSE, vfs_size_u32, FALSE);
	}

	gStaticVFS = new LLVFS(static_vfs_index_file, static_vfs_data_file, TRUE, 0, FALSE);

	BOOL success = gVFS->isValid() && gStaticVFS->isValid();
	if( !success )
	{
		return FALSE;
	}
	else
	{
		LLVFile::initClass();
		return TRUE;
	}
}

#if LL_DARWIN

OSErr AEGURLHandler(const AppleEvent *messagein, AppleEvent *reply, long refIn)
{
	OSErr result = noErr;
	DescType actualType;
	char buffer[1024];		/* Flawfinder: ignore */
	Size size;
	
	result = AEGetParamPtr (
		messagein,
		keyDirectObject,
		typeCString,
		&actualType,
		(Ptr)buffer,
		sizeof(buffer),
		&size);	
	
	if(result == noErr)
	{
		// Got the URL out of the event.
		// secondlife://

		// Parse it and stash in globals.
		LLURLSimString::setString(buffer);
		gConnectToSomething = TRUE;
		
		if(gFloaterWorldMap != NULL)
		{
			// If the viewer's already logged in, pass it along directly.
			if (LLURLSimString::parse())
			{
				gFloaterWorldMap->trackURL(LLURLSimString::sInstance.mSimName,
										   LLURLSimString::sInstance.mX,
										   LLURLSimString::sInstance.mY,
										   LLURLSimString::sInstance.mZ);
			}
		}
	}
	
	return(result);
}

OSErr AEQuitHandler(const AppleEvent *messagein, AppleEvent *reply, long refIn)
{
	OSErr result = noErr;
	
	app_request_quit();
	
	return(result);
}

OSStatus simpleDialogHandler(EventHandlerCallRef handler, EventRef event, void *userdata)
{
	OSStatus result = eventNotHandledErr;
	OSStatus err;
	UInt32 evtClass = GetEventClass(event);
	UInt32 evtKind = GetEventKind(event);
	WindowRef window = (WindowRef)userdata;
	
	if((evtClass == kEventClassCommand) && (evtKind == kEventCommandProcess))
	{
		HICommand cmd;
		err = GetEventParameter(event, kEventParamDirectObject, typeHICommand, NULL, sizeof(cmd), NULL, &cmd);
		
		if(err == noErr)
		{
			switch(cmd.commandID)
			{
				case kHICommandOK:
					QuitAppModalLoopForWindow(window);
					result = noErr;
				break;
				
				case kHICommandCancel:
					QuitAppModalLoopForWindow(window);
					result = userCanceledErr;
				break;
			}
		}
	}
	
	return(result);
}

OSStatus DisplayReleaseNotes(void)
{
	OSStatus err;
	IBNibRef nib = NULL;
	WindowRef window = NULL;
	
	err = CreateNibReference(CFSTR("SecondLife"), &nib);
	
	if(err == noErr)
	{
		CreateWindowFromNib(nib, CFSTR("Release Notes"), &window);
	}
		
	if(err == noErr)
	{
		// Get the text view control
		HIViewRef textView;
		ControlID id;

		id.signature = 'text';
		id.id = 0;

		LLString releaseNotesText;
		
		LLString::read(releaseNotesText, "releasenotes.txt");		/* Flawfinder: ignore*/

		err = HIViewFindByID(HIViewGetRoot(window), id, &textView);
		
		if(err == noErr)
		{
			// Convert from the encoding used in the release notes.
			CFStringRef str = CFStringCreateWithBytes(
				NULL, 
				(const UInt8*)releaseNotesText.c_str(), 
				releaseNotesText.size(), 
				kCFStringEncodingWindowsLatin1, 		// This matches the way the Windows version displays the release notes.
				FALSE);
			
			if(str != NULL)
			{
				int size = CFStringGetLength(str);

				if(size > 0)
				{
					UniChar *chars = new UniChar[size + 1];
					CFStringGetCharacters(str, CFRangeMake(0, size), chars);
				
					err = TXNSetData(HITextViewGetTXNObject(textView), kTXNUnicodeTextData, chars, size * sizeof(UniChar), kTXNStartOffset, kTXNStartOffset);
					
					delete[] chars;
				}
				
				CFRelease(str);
			}
			else
			{
				// Creating the string failed.  Probably an encoding problem.  Display SOMETHING...
				err = TXNSetData(HITextViewGetTXNObject(textView), kTXNTextData, releaseNotesText.c_str(), releaseNotesText.size(), kTXNStartOffset, kTXNStartOffset);
			}
		}
		
		// Set the selection to the beginning of the text and scroll it into view.
		if(err == noErr)
		{
			err = TXNSetSelection(HITextViewGetTXNObject(textView), kTXNStartOffset, kTXNStartOffset);
		}
		
		if(err == noErr)
		{
			// This function returns void.
			TXNShowSelection(HITextViewGetTXNObject(textView), false);
		}
	}

	if(err == noErr)
	{
		ShowWindow(window);
	}

	if(err == noErr)
	{
		// Set up an event handler for the window.
		EventHandlerRef handler = NULL;
		EventTypeSpec handlerEvents[] = 
		{
			{ kEventClassCommand, kEventCommandProcess }
		};

		InstallWindowEventHandler(
				window, 
				NewEventHandlerUPP(simpleDialogHandler), 
				GetEventTypeCount (handlerEvents), 
				handlerEvents, 
				(void*)window, 
				&handler);
	}
			
	if(err == noErr)
	{
		RunAppModalLoopForWindow(window);
	}
			
	if(window != NULL)
	{
		DisposeWindow(window);
	}
	
	if(nib != NULL)
	{
		DisposeNibReference(nib);
	}

	return(err);
}

void init_apple_menu(const char* product)
{
	// Load up a proper menu bar.
	{
		OSStatus err;
		IBNibRef nib = NULL;
		// NOTE: DO NOT translate or brand this string.  It's an internal name in the .nib file, and MUST match exactly.
		err = CreateNibReference(CFSTR("SecondLife"), &nib);
		
		if(err == noErr)
		{
			// NOTE: DO NOT translate or brand this string.  It's an internal name in the .nib file, and MUST match exactly.
			SetMenuBarFromNib(nib, CFSTR("MenuBar"));
		}

		if(nib != NULL)
		{
			DisposeNibReference(nib);
		}
	}
	
	// Install a handler for 'gurl' AppleEvents.  This is how secondlife:// URLs get passed to the viewer.
	
	if(AEInstallEventHandler('GURL', 'GURL', NewAEEventHandlerUPP(AEGURLHandler),0, false) != noErr)
	{
		// Couldn't install AppleEvent handler.  This error shouldn't be fatal.
		llinfos << "Couldn't install 'GURL' AppleEvent handler.  Continuing..." << llendl;
	}

	// Install a handler for 'quit' AppleEvents.  This makes quitting the application from the dock work.
	if(AEInstallEventHandler(kCoreEventClass, kAEQuitApplication, NewAEEventHandlerUPP(AEQuitHandler),0, false) != noErr)
	{
		// Couldn't install AppleEvent handler.  This error shouldn't be fatal.
		llinfos << "Couldn't install Quit AppleEvent handler.  Continuing..." << llendl;
	}
}
#endif

void user_logout()
{
	if (!gDoneLogout)
	{
		LLMessageSystem* msg = gMessageSystem;
		if (msg)
		{
/*			if (gSavedSettings.getBOOL("LoggedIn") )
			{
				// bleah - we can't use llinfos inside a signal handler
				//llinfos << "Sending logout message" << llendl;

				msg->newMessage("LogoutDemand");
				msg->nextBlockFast(_PREHASH_LogoutBlock);
				msg->addUUIDFast(_PREHASH_SessionID, gAgentSessionID );
				msg->sendMessage(gUserServer);
*/
				gSavedSettings.setBOOL("LoggedIn", FALSE);
//			}
			/*  This message has been removed and functionality moved into logoutrequest

			if (gAgent.getRegion())
			{
				msg->newMessageFast(_PREHASH_AgentQuit);
				msg->nextBlockFast(_PREHASH_AgentData);
				msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
				msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
				gAgent.sendMessage();
			}
			*/

			// Go through all of the regions we know about...
/*			if (gWorldp)
			{
				gWorldp->disconnectRegions();
			}*/

		}
		gDoneLogout = TRUE;
	}
}


// This routine may get called more than once during the shutdown process.
// This can happen because we need to get the screenshot before the window
// is destroyed.
void save_final_snapshot(void*)
{
	if (!gHaveSavedSnapshot && !gNoRender)
	{
		gSavedSettings.setVector3d("FocusPosOnLogout", gAgent.calcFocusPositionTargetGlobal());
		gSavedSettings.setVector3d("CameraPosOnLogout", gAgent.calcCameraPositionTargetGlobal());
		gViewerWindow->setCursor(UI_CURSOR_WAIT);
		gAgent.changeCameraToThirdPerson( FALSE );	// don't animate, need immediate switch
		gSavedSettings.setBOOL("ShowParcelOwners", FALSE);
		idle();
		char temp_str[MAX_PATH];		/* Flawfinder: ignore */
		strncpy (temp_str,gDirUtilp->getLindenUserDir().c_str(), MAX_PATH -1);		/* Flawfinder: ignore */
		temp_str[MAX_PATH -1] = '\0';
		strcat (temp_str,"/");		/* Flawfinder: ignore */
		strcat (temp_str,SCREEN_LAST_FILENAME);		/* Flawfinder: ignore */
		gViewerWindow->saveSnapshot(temp_str, gViewerWindow->getWindowWidth(), gViewerWindow->getWindowHeight(), FALSE, TRUE);
		gHaveSavedSnapshot = TRUE;
	}
}

// If you pass a path+file or URL in as a 
// Note: This may get called multiple times during shutdown.
void app_force_quit(const char* launch_file_on_quit)
{
	// Don't actually DO anything complicated here, because
	// it's called from signal handlers for the Mac on SIGTERM.
	// It calls this with NULL.
	if (launch_file_on_quit)
	{
		gLaunchFileOnQuit.assign( (const char*)launch_file_on_quit );
	}

	gQuit = TRUE;
}

// Don't quit instantly.  Instead, request to be logged off.
// Called from control-Q handler, Windows(tm) close-window message (WM_CLOSE), and Mac Quit AppleEvent handler.
void app_request_quit()
{
	llinfos << "app_request_quit" << llendl;

	LLViewerRegion* region = gAgent.getRegion();
	
	if( (gStartupState < STATE_STARTED) || !region )
	{
		// Quit immediately
		app_force_quit(NULL);
		return;
	}

	if (gHUDManager)
	{
		LLHUDEffectSpiral *effectp = (LLHUDEffectSpiral*)gHUDManager->createViewerEffect(LLHUDObject::LL_HUD_EFFECT_POINT, TRUE);
		effectp->setPositionGlobal(gAgent.getPositionGlobal());
		effectp->setColor(LLColor4U(gAgent.getEffectColor()));
		gHUDManager->sendEffects();
	}

	// Attempt to close all floaters that might be
	// editing things.
	if (gFloaterView)
	{
		// application is quitting
		gFloaterView->closeAllChildren(true);
	}

	send_stats();

	gLogoutTimer.reset();
	gQuitRequested = TRUE;
}


// User didn't really want to quit, for example, clicked "Cancel"
// in a floater save dialog.
void app_abort_quit()
{
	llinfos << "app_abort_quit()" << llendl;
	gQuitRequested = FALSE;
}

void idle_shutdown()
{
	// Wait for all modal alerts to get resolved
	if (LLModalDialog::activeCount() > 0)
	{
		return;
	}

	// close IM interface
	if(gIMView)
	{
		gIMView->disconnectAllSessions();
	}
	
	// Wait for all floaters to get resolved
	if (gFloaterView
		&& !gFloaterView->allChildrenClosed())
	{
		return;
	}

	static bool saved_snapshot = false;
	if (!saved_snapshot)
	{
		saved_snapshot = true;
		save_final_snapshot(NULL);
		return;
	}

	const F32 SHUTDOWN_UPLOAD_SAVE_TIME = 5.f;

	S32 pending_uploads = gAssetStorage->getNumPendingUploads();
	if (pending_uploads > 0
		&& gLogoutTimer.getElapsedTimeF32() < SHUTDOWN_UPLOAD_SAVE_TIME
		&& !gLogoutRequestSent)
	{
		static S32 total_uploads = 0;
		// Sometimes total upload count can change during logout.
		total_uploads = llmax(total_uploads, pending_uploads);
		gViewerWindow->setShowProgress(TRUE);
		S32 finished_uploads = total_uploads - pending_uploads;
		F32 percent = 100.f * finished_uploads / total_uploads;
		gViewerWindow->setProgressPercent(percent);
		char buffer[MAX_STRING];		/* Flawfinder: ignore */
		snprintf(buffer, MAX_STRING, "Saving final data...");		/* Flawfinder: ignore */
		gViewerWindow->setProgressString(buffer);
		return;
	}

	// All floaters are closed.  Tell server we want to quit.
	if( !gLogoutRequestSent )
	{
		send_logout_request();

		// Wait for a LogoutReply message
		gViewerWindow->setShowProgress(TRUE);
		gViewerWindow->setProgressPercent(100.f);
		gViewerWindow->setProgressString("Logging out...");
		return;
	}

	// Make sure that we quit if we haven't received a reply from the server.
	if( gLogoutRequestSent 
		&& gLogoutTimer.getElapsedTimeF32() > gLogoutMaxTime )
	{
		app_force_quit(NULL);
		return;
	}
}


U32		gTotalLandIn = 0, gTotalLandOut = 0;
U32		gTotalWaterIn = 0, gTotalWaterOut = 0;

F32		gAveLandCompression = 0.f, gAveWaterCompression = 0.f;
F32		gBestLandCompression = 1.f, gBestWaterCompression = 1.f;
F32		gWorstLandCompression = 0.f, gWorstWaterCompression = 0.f;

F32		gFPSClamped = 10.f;						// Pretend we start at target rate.
F32		gFrameDTClamped = 0.f;					// Time between adjacent checks to network for packets
F32		gFrameDT = 0.f;

//S32		gDecodedBits = 0;
U32		gPacketsIn = 0;

U32		gTotalWorldBytes = 0, gTotalObjectBytes = 0, gTotalTextureBytes = 0, gSimPingCount = 0;
U32		gObjectBits = 0;
F32		gAvgSimPing = 0.f;


extern U32  gVisCompared;
extern U32  gVisTested;


void update_statistics(U32 frame_count)
{
	gTotalWorldBytes += gVLManager.getTotalBytes();
	gTotalObjectBytes += gObjectBits / 8;
	gTotalTextureBytes += LLViewerImageList::sTextureBits / 8;

	// make sure we have a valid time delta for this frame
	if (gFrameIntervalSeconds > 0.f)
	{
		if (gAgent.getCameraMode() == CAMERA_MODE_MOUSELOOK)
		{
			gViewerStats->incStat(LLViewerStats::ST_MOUSELOOK_SECONDS, gFrameIntervalSeconds);
		}
		else if (gAgent.getCameraMode() == CAMERA_MODE_CUSTOMIZE_AVATAR)
		{
			gViewerStats->incStat(LLViewerStats::ST_AVATAR_EDIT_SECONDS, gFrameIntervalSeconds);
		}
		else if (gFloaterTools && gFloaterTools->getVisible())
		{
			gViewerStats->incStat(LLViewerStats::ST_TOOLBOX_SECONDS, gFrameIntervalSeconds);
		}
	}
	gViewerStats->setStat(LLViewerStats::ST_ENABLE_VBO, (F64)gSavedSettings.getBOOL("RenderVBOEnable"));
	gViewerStats->setStat(LLViewerStats::ST_LIGHTING_DETAIL, (F64)gSavedSettings.getS32("RenderLightingDetail"));
	gViewerStats->setStat(LLViewerStats::ST_DRAW_DIST, (F64)gSavedSettings.getF32("RenderFarClip"));
	gViewerStats->setStat(LLViewerStats::ST_CHAT_BUBBLES, (F64)gSavedSettings.getBOOL("UseChatBubbles"));
#if 0 // 1.9.2
	gViewerStats->setStat(LLViewerStats::ST_SHADER_OBJECTS, (F64)gSavedSettings.getS32("VertexShaderLevelObject"));
	gViewerStats->setStat(LLViewerStats::ST_SHADER_AVATAR, (F64)gSavedSettings.getBOOL("VertexShaderLevelAvatar"));
	gViewerStats->setStat(LLViewerStats::ST_SHADER_ENVIRONMENT, (F64)gSavedSettings.getBOOL("VertexShaderLevelEnvironment"));
#endif
	gViewerStats->setStat(LLViewerStats::ST_FRAME_SECS, gDebugView->mFastTimerView->getTime(LLFastTimer::FTM_FRAME));
	F64 idle_secs = gDebugView->mFastTimerView->getTime(LLFastTimer::FTM_IDLE);
	F64 network_secs = gDebugView->mFastTimerView->getTime(LLFastTimer::FTM_NETWORK);
	gViewerStats->setStat(LLViewerStats::ST_UPDATE_SECS, idle_secs - network_secs);
	gViewerStats->setStat(LLViewerStats::ST_NETWORK_SECS, network_secs);
	gViewerStats->setStat(LLViewerStats::ST_IMAGE_SECS, gDebugView->mFastTimerView->getTime(LLFastTimer::FTM_IMAGE_UPDATE));
	gViewerStats->setStat(LLViewerStats::ST_REBUILD_SECS, gDebugView->mFastTimerView->getTime(LLFastTimer::FTM_REBUILD));
	gViewerStats->setStat(LLViewerStats::ST_RENDER_SECS, gDebugView->mFastTimerView->getTime(LLFastTimer::FTM_RENDER_GEOMETRY));
		
	LLCircuitData *cdp = gMessageSystem->mCircuitInfo.findCircuit(gAgent.getRegion()->getHost());
	if (cdp)
	{
		gViewerStats->mSimPingStat.addValue(cdp->getPingDelay());
		gAvgSimPing = ((gAvgSimPing * (F32)gSimPingCount) + (F32)(cdp->getPingDelay())) / ((F32)gSimPingCount + 1);
		gSimPingCount++;
	}
	else
	{
		gViewerStats->mSimPingStat.addValue(10000);
	}

	gViewerStats->mFPSStat.addValue(1);
	F32 layer_bits = (F32)(gVLManager.getLandBits() + gVLManager.getWindBits() + gVLManager.getCloudBits());
	gViewerStats->mLayersKBitStat.addValue(layer_bits/1024.f);
	gViewerStats->mObjectKBitStat.addValue(gObjectBits/1024.f);
	gViewerStats->mTextureKBitStat.addValue(LLViewerImageList::sTextureBits/1024.f);
	gViewerStats->mVFSPendingOperations.addValue(LLVFile::getVFSThread()->getPending());
	gViewerStats->mAssetKBitStat.addValue(gTransferManager.getTransferBitsIn(LLTCT_ASSET)/1024.f);
	gTransferManager.resetTransferBitsIn(LLTCT_ASSET);

	static S32 tex_bits_idle_count = 0;
	if (LLViewerImageList::sTextureBits == 0)
	{
		if (++tex_bits_idle_count >= 30)
			gDebugTimers[0].pause();
	}
	else
	{
		tex_bits_idle_count = 0;
		gDebugTimers[0].unpause();
	}
	
	gViewerStats->mTexturePacketsStat.addValue(LLViewerImageList::sTexturePackets);

	cdp = gMessageSystem->mCircuitInfo.findCircuit(gUserServer);
	if (cdp)
	{
		gViewerStats->mUserserverPingStat.addValue(cdp->getPingDelay());
	}
	else
	{
		gViewerStats->mUserserverPingStat.addValue(10000);
	}

	// log when the LibXUL (aka Mozilla) widget is used and opened so we can monitor framerate changes
	#if LL_LIBXUL_ENABLED
	{
		BOOL result = gViewerHtmlHelp.getFloaterOpened();
		gViewerStats->setStat(LLViewerStats::ST_LIBXUL_WIDGET_USED, (F64)result);
	}
	#endif

	{
		static F32 visible_avatar_frames = 0.f;
		static F32 avg_visible_avatars = 0;
		F32 visible_avatars = (F32)LLVOAvatar::sNumVisibleAvatars;
		if (visible_avatars > 0.f)
		{
			visible_avatar_frames = 1.f;
			avg_visible_avatars = (avg_visible_avatars * (F32)(visible_avatar_frames - 1.f) + visible_avatars) / visible_avatar_frames;
		}
		gViewerStats->setStat(LLViewerStats::ST_VISIBLE_AVATARS, (F64)avg_visible_avatars);
	}
	gWorldp->updateNetStats();
	gWorldp->requestCacheMisses();
	
	// Reset all of these values.
	gVLManager.resetBitCounts();
	gObjectBits = 0;
//	gDecodedBits = 0;

	LLViewerImageList::sTextureBits = 0;
	LLViewerImageList::sTexturePackets = 0;
}

//
// Handle messages, and all message related stuff
//

#define TIME_THROTTLE_MESSAGES

#ifdef TIME_THROTTLE_MESSAGES
#define CHECK_MESSAGES_DEFAULT_MAX_TIME .020f // 50 ms = 50 fps (just for messages!)
static F32 CheckMessagesMaxTime = CHECK_MESSAGES_DEFAULT_MAX_TIME;
#endif

void idle_network()
{
	gObjectList.mNumNewObjects = 0;
	S32 total_decoded = 0;

	if (!gSavedSettings.getBOOL("SpeedTest") && !gRunLocal)
	{
		LLFastTimer t(LLFastTimer::FTM_IDLE_NETWORK); // decode
		
		// deal with any queued name requests and replies.
		gCacheName->processPending();

		LLTimer check_message_timer;
		//  Read all available packets from network 
		stop_glerror();
		const S64 frame_count = gFrameCount;  // U32->S64
		F32 total_time = 0.0f;
   		while (gMessageSystem->checkAllMessages(frame_count, gServicePump)) 
		{
			if (gDoDisconnect)
			{
				// We're disconnecting, don't process any more messages from the server
				// We're usually disconnecting due to either network corruption or a
				// server going down, so this is OK.
				break;
			}
			stop_glerror();

			total_decoded++;
			gPacketsIn++;

			if (total_decoded > MESSAGE_MAX_PER_FRAME)
			{
				break;
			}

#ifdef TIME_THROTTLE_MESSAGES
			// Prevent slow packets from completely destroying the frame rate.
			// This usually happens due to clumps of avatars taking huge amount
			// of network processing time (which needs to be fixed, but this is
			// a good limit anyway).
			total_time = check_message_timer.getElapsedTimeF32();
			if (total_time >= CheckMessagesMaxTime)
				break;
#endif
		}
		// Handle per-frame message system processing.
		gMessageSystem->processAcks();

#ifdef TIME_THROTTLE_MESSAGES
		if (total_time >= CheckMessagesMaxTime)
		{
			// Increase CheckMessagesMaxTime so that we will eventually catch up
			CheckMessagesMaxTime *= 1.035f; // 3.5% ~= x2 in 20 frames, ~8x in 60 frames
		}
		else
		{
			// Reset CheckMessagesMaxTime to default value
			CheckMessagesMaxTime = CHECK_MESSAGES_DEFAULT_MAX_TIME;
		}
#endif
		


		// we want to clear the control after sending out all necessary agent updates
		gAgent.resetControlFlags();
		stop_glerror();

		
		// Decode enqueued messages...
		S32 remaining_possible_decodes = MESSAGE_MAX_PER_FRAME - total_decoded;

		if( remaining_possible_decodes <= 0 )
		{
			llinfos << "Maxed out number of messages per frame at " << MESSAGE_MAX_PER_FRAME << llendl;
		}

		if (gPrintMessagesThisFrame)
		{
			llinfos << "Decoded " << total_decoded << " msgs this frame!" << llendl;
			gPrintMessagesThisFrame = FALSE;
		}
	}

	gObjectList.mNumNewObjectsStat.addValue(gObjectList.mNumNewObjects);

	// Retransmit unacknowledged packets.
	gXferManager->retransmitUnackedPackets();
	gAssetStorage->checkForTimeouts();

	gViewerThrottle.updateDynamicThrottle();
}

void idle_afk_check()
{
	// check idle timers
	if (gAwayTriggerTimer.getElapsedTimeF32() > gAFKTimeout)
	{
		gAgent.setAFK();
	}
}
void request_initial_instant_messages()
{
	static BOOL requested = FALSE;
	if (!requested
		&& gMuteListp
		&& gMuteListp->isLoaded()
		&& gAgent.getAvatarObject())
	{
		// Auto-accepted inventory items may require the avatar object
		// to build a correct name.  Likewise, inventory offers from
		// muted avatars require the mute list to properly mute.
		LLMessageSystem* msg = gMessageSystem;
		msg->newMessageFast(_PREHASH_RetrieveInstantMessages);
		msg->nextBlockFast(_PREHASH_AgentData);
		msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
		msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
		gAgent.sendReliableMessage();
		requested = TRUE;
	}
}

///////////////////////////////////////////////////////
// idle()
//
// Called every time the window is not doing anything.
// Receive packets, update statistics, and schedule a redisplay.
///////////////////////////////////////////////////////

void idle()
{
	// Update frame timers
	static LLTimer idle_timer;

	LLControlBase::updateAllListeners();

	LLFrameTimer::updateFrameTime();
	LLEventTimer::updateClass();
	LLCriticalDamp::updateInterpolants();
	LLMortician::updateClass();
	F32 dt_raw = idle_timer.getElapsedTimeAndResetF32();

	// Cap out-of-control frame times
	// Too low because in menus, swapping, debugger, etc.
	// Too high because idle called with no objects in view, etc.
	const F32 MIN_FRAME_RATE = 1.f;
	const F32 MAX_FRAME_RATE = 200.f;

	F32 frame_rate_clamped = 1.f / dt_raw;
	frame_rate_clamped = llclamp(frame_rate_clamped, MIN_FRAME_RATE, MAX_FRAME_RATE);
	gFrameDTClamped = 1.f / frame_rate_clamped;

	// Global frame timer
	// Smoothly weight toward current frame
	gFPSClamped = (frame_rate_clamped + (4.f * gFPSClamped)) / 5.f;

	if (gQuitAfterSeconds > 0.f)
	{
		if (gRenderStartTime.getElapsedTimeF32() > gQuitAfterSeconds)
		{
			app_force_quit(NULL);
		}
	}

	// Must wait until both have avatar object and mute list, so poll
	// here.
	request_initial_instant_messages();

	///////////////////////////////////
	//
	// Special case idle if still starting up
	//

	if (gStartupState < STATE_STARTED)
	{
		// Skip rest if idle startup returns false (essentially, no world yet)
		if (!idle_startup())
		{
			return;
		}
	}

	
    F32 yaw = 0.f;				// radians

	if (!gDisconnected)
	{
		LLFastTimer t(LLFastTimer::FTM_NETWORK);
		
	    // Update spaceserver timeinfo
	    gWorldp->setSpaceTimeUSec(gWorldp->getSpaceTimeUSec() + (U32)(dt_raw * SEC_TO_MICROSEC));
    
    
	    //////////////////////////////////////
	    //
	    // Update simulator agent state
	    //

		if (gRotateRight)
		{
			gAgent.moveYaw(-1.f);
		}

	    // Handle automatic walking towards points
	    gAgentPilot.updateTarget();
	    gAgent.autoPilot(&yaw);
    
	    static LLFrameTimer agent_update_timer;
	    static U32 				last_control_flags;
    
	    //	When appropriate, update agent location to the simulator.
	    F32 agent_update_time = agent_update_timer.getElapsedTimeF32();
	    BOOL flags_changed = gAgent.controlFlagsDirty() || (last_control_flags != gAgent.getControlFlags());
    
	    if (flags_changed || (agent_update_time > (1.0f / (F32) AGENT_UPDATES_PER_SECOND)))
	    {
		    // Send avatar and camera info
		    last_control_flags = gAgent.getControlFlags();
		    send_agent_update(TRUE);
		    agent_update_timer.reset();
	    }
	}

	//////////////////////////////////////
	//
	// Manage statistics
	//
	//

	{
		static LLFrameTimer	viewer_stats_timer;
		reset_statistics();

		// Update session stats every large chunk of time
		// *FIX: (???) SAMANTHA
		if (viewer_stats_timer.getElapsedTimeF32() >= 300.f && !gDisconnected)
		{
			llinfos << "Transmitting sessions stats" << llendl;
			send_stats();
			viewer_stats_timer.reset();
		}

		// Print the object debugging stats
		static LLFrameTimer object_debug_timer;
		if (object_debug_timer.getElapsedTimeF32() > 5.f)
		{
			object_debug_timer.reset();
			if (gObjectList.mNumDeadObjectUpdates)
			{
				llinfos << "Dead object updates: " << gObjectList.mNumDeadObjectUpdates << llendl;
				gObjectList.mNumDeadObjectUpdates = 0;
			}
			if (gObjectList.mNumUnknownKills)
			{
				llinfos << "Kills on unknown objects: " << gObjectList.mNumUnknownKills << llendl;
				gObjectList.mNumUnknownKills = 0;
			}
			if (gObjectList.mNumUnknownUpdates)
			{
				llinfos << "Unknown object updates: " << gObjectList.mNumUnknownUpdates << llendl;
				gObjectList.mNumUnknownUpdates = 0;
			}
		}
		gFrameStats.addFrameData();
	}
	
	if (!gDisconnected)
	{
		LLFastTimer t(LLFastTimer::FTM_NETWORK);
	
	    ////////////////////////////////////////////////
	    //
	    // Network processing
	    //
	    // NOTE: Starting at this point, we may still have pointers to "dead" objects
	    // floating throughout the various object lists.
	    //
    
	    gFrameStats.start(LLFrameStats::IDLE_NETWORK);
		idle_network();
	    stop_glerror();
	        
	    gFrameStats.start(LLFrameStats::AGENT_MISC);

		// Check for away from keyboard, kick idle agents.
		idle_afk_check();

		//  Update statistics for this frame
		update_statistics(gFrameCount);

		gViewerWindow->updateDebugText();
	}

	////////////////////////////////////////
	//
	// Handle the regular UI idle callbacks as well as
	// hover callbacks
	//

	{
// 		LLFastTimer t(LLFastTimer::FTM_IDLE_CB);

		// Do event notifications if necessary.  Yes, we may want to move this elsewhere.
		gEventNotifier.update();
		
		gIdleCallbacks.callFunctions();
	}
	
	if (gDisconnected)
    {
		return;
    }

	gViewerWindow->handlePerFrameHover();

	///////////////////////////////////////
	// Agent and camera movement
	//
		LLCoordGL current_mouse = gViewerWindow->getCurrentMouse();

// 		BOOL was_in_prelude = gAgent.inPrelude();

	{
		//LLFastTimer t(LLFastTimer::FTM_TEMP1);
		
		// After agent and camera moved, figure out if we need to
		// deselect objects.
		gSelectMgr->deselectAllIfTooFar();

	}

	{
		LLFastTimer t(LLFastTimer::FTM_RESET_DRAWORDER);
			
		//////////////////////////////////////////////
		//
		// Clear draw orders
		//
		// Should actually be done after render, but handlePerFrameHover actually does a "render"
		// to do its selection.
		//

		gPipeline.resetDrawOrders();
	}
	{
		// Handle pending gesture processing
		gGestureManager.update();

		gAgent.updateAgentPosition(gFrameDTClamped, yaw, current_mouse.mX, current_mouse.mY);
	}

	{
		LLFastTimer t(LLFastTimer::FTM_OBJECTLIST_UPDATE); // Actually "object update"
		gFrameStats.start(LLFrameStats::OBJECT_UPDATE);
		
		if (!(gLogoutRequestSent && gHaveSavedSnapshot))
		{
			gObjectList.update(gAgent, *gWorldp);
		}
	}
	
	{
		LLFastTimer t(LLFastTimer::FTM_UPDATE_SKY);	
		gSky.updateSky();
	}

	//////////////////////////////////////
	//
	// Deletes objects...
	// Has to be done after doing idleUpdates (which can kill objects)
	//

	{
		LLFastTimer t(LLFastTimer::FTM_CLEANUP);
		gFrameStats.start(LLFrameStats::CLEAN_DEAD);
		gObjectList.cleanDeadObjects();
		LLDrawable::cleanupDeadDrawables();
	}
	
	//
	// After this point, in theory we should never see a dead object
	// in the various object/drawable lists.
	//

	//////////////////////////////////////
	//
	// Update/send HUD effects
	//
	// At this point, HUD effects may clean up some references to
	// dead objects.
	//

	{
		//LLFastTimer t(LLFastTimer::FTM_TEMP3);
		
		gFrameStats.start(LLFrameStats::UPDATE_EFFECTS);
		gSelectMgr->updateEffects();
		gHUDManager->cleanupEffects();
		gHUDManager->sendEffects();
	}

	stop_glerror();

	////////////////////////////////////////
	//
	// Unpack layer data that we've received
	//

	{
		LLFastTimer t(LLFastTimer::FTM_NETWORK);
		gVLManager.unpackData();
	}
	
	/////////////////////////
	//
	// Update surfaces, and surface textures as well.
	//

	gWorldp->updateVisibilities();
	{
		const F32 max_region_update_time = .001f; // 1ms
		LLFastTimer t(LLFastTimer::FTM_REGION_UPDATE);
		gWorldp->updateRegions(max_region_update_time);
	}
	
	/////////////////////////
	//
	// Update weather effects
	//

	if (!gNoRender)
	{
		gWorldp->updateClouds(gFrameDTClamped);
		gSky.propagateHeavenlyBodies(gFrameDTClamped);				// moves sun, moon, and planets

		// Update wind vector 
		LLVector3 wind_position_region;
		static LLVector3 average_wind;

		LLViewerRegion *regionp;
		regionp = gWorldp->resolveRegionGlobal(wind_position_region, gAgent.getPositionGlobal());	// puts agent's local coords into wind_position	
		if (regionp)
		{
			gWindVec = regionp->mWind.getVelocity(wind_position_region);

			// Compute average wind and use to drive motion of water
			
			average_wind = regionp->mWind.getAverage();
			F32 cloud_density = regionp->mCloudLayer.getDensityRegion(wind_position_region);
			
			gSky.setCloudDensityAtAgent(cloud_density);
			gSky.setWind(average_wind);
			//LLVOWater::setWind(average_wind);
		}
		else
		{
			gWindVec.setVec(0.0f, 0.0f, 0.0f);
		}
	}
	stop_glerror();
	
	//////////////////////////////////////
	//
	// Update images, using the image stats generated during object update/culling
	//
	// Can put objects onto the retextured list.
	//
	gFrameStats.start(LLFrameStats::IMAGE_UPDATE);

	LLFastTimer t(LLFastTimer::FTM_IMAGE_UPDATE);
	
	LLViewerImage::updateClass(gCamera->getVelocityStat()->getMean(),
								gCamera->getAngularVelocityStat()->getMean());

	gBumpImageList.updateImages();  // must be called before gImageList version so that it's textures are thrown out first.

	const F32 max_image_decode_time = 0.005f; // 5 ms decode time
	gImageList.updateImages(max_image_decode_time);
	stop_glerror();

	//////////////////////////////////////
	//
	// Sort and cull in the new renderer are moved to pipeline.cpp
	// Here, particles are updated and drawables are moved.
	//
	
	if (!gNoRender)
	{
		gFrameStats.start(LLFrameStats::UPDATE_MOVE);
		gPipeline.updateMove();

		gFrameStats.start(LLFrameStats::UPDATE_PARTICLES);
		gWorldp->updateParticles();
	}
	stop_glerror();

	if (!LLViewerJoystick::sOverrideCamera)
	{
		gAgent.updateCamera();
	}
	else
	{
		LLViewerJoystick::updateCamera();
	}

	// objects and camera should be in sync, do LOD calculations now
	{
		LLFastTimer t(LLFastTimer::FTM_LOD_UPDATE);
		gObjectList.updateApparentAngles(gAgent);
	}

	//////////////////////////////////////
	//
	// Audio stuff
	//
	//

	if (gSavedSettings.getBOOL("MuteAudio"))
	{
		LLMediaEngine::updateClass( 0.0f );
	}
	else
	{
		// only restore the volume if we're not minimized
		if ( ! gViewerWindow->mWindow->getMinimized() )
			LLMediaEngine::updateClass( gSavedSettings.getF32( "MediaAudioVolume" ) );
	};
	
	if (gAudiop)
	{
		LLFastTimer t(LLFastTimer::FTM_AUDIO_UPDATE);
		
		gFrameStats.start(LLFrameStats::AUDIO);
		// update listener position because agent has moved

		LLVector3d lpos_global = gAgent.getCameraPositionGlobal();		
		LLVector3 lpos_global_f;
		lpos_global_f.setVec(lpos_global);
	
		gAudiop->setListener(lpos_global_f,
					  // gCameraVelocitySmoothed, 
					  // LLVector3::zero,	
					  gAgent.getVelocity(),    // !!! BUG need to replace this with smoothed velocity!
					  gCamera->getUpAxis(),
					  gCamera->getAtAxis());

		// this line rotates the wind vector to be listener (agent) relative
		// unfortunately we have to pre-translate to undo the translation that
		// occurs in the transform call
		gRelativeWindVec = gAgent.getFrameAgent().rotateToLocal(gWindVec - gAgent.getVelocity());
		
#ifdef kAUDIO_ENABLE_WIND
		//
		//  Extract height above water to modulate filter by whether above/below water 
		// 
		static F32 last_camera_water_height = -1000.f;
		LLVector3 camera_pos = gAgent.getCameraPositionAgent();
		F32 camera_water_height = camera_pos.mV[VZ] - gAgent.getRegion()->getWaterHeight();
		
		//
		//  Don't update rolloff factor unless water surface has been crossed
		//
		if ((last_camera_water_height * camera_water_height) < 0.f)
		{
			if (camera_water_height < 0.f)
			{
				gAudiop->setRolloffFactor(gSavedSettings.getF32("AudioLevelRolloff") * LL_ROLLOFF_MULTIPLIER_UNDER_WATER);
			}
			else 
			{
				gAudiop->setRolloffFactor(gSavedSettings.getF32("AudioLevelRolloff"));
			}
		}
		last_camera_water_height = camera_water_height;
		gAudiop->updateWind(gRelativeWindVec, camera_water_height);
#endif

		
		// this line actually commits the changes we've made to source positions, etc.
		const F32 max_audio_decode_time = 0.002f; // 2 ms decode time
		gAudiop->idle(max_audio_decode_time);
	}

	// Handle shutdown process, for example, 
	// wait for floaters to close, send quit message,
	// forcibly quit if it has taken too long
	if (gQuitRequested)
	{
		idle_shutdown();
	}

	stop_glerror();
}


F32 mouse_x_from_center(S32 x)
{
	return ((F32) x / (F32) gViewerWindow->getWindowWidth() ) - 0.5f;
}


F32 mouse_y_from_center(S32 y)
{
	return ((F32) y / (F32) gViewerWindow->getWindowHeight() ) - 0.5f;
}


/////////////////////////////////////////////////////////

class AudioSettingsListener: public LLSimpleListener
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		// Note: Ignore the specific event value, look up the ones we want
		if (!gAudiop) return true;
		gAudiop->setDopplerFactor(gSavedSettings.getF32("AudioLevelDoppler"));
		gAudiop->setDistanceFactor(gSavedSettings.getF32("AudioLevelDistance")); 
#ifdef kAUDIO_ENABLE_WIND
		// Wind Gain
		gAudiop->mMaxWindGain = gSavedSettings.getF32("AudioLevelWind");
		// Rolloff
		LLVector3 camera_pos = gAgent.getCameraPositionAgent();
		LLViewerRegion* region = gAgent.getRegion();
		F32 camera_water_height = region ? camera_pos.mV[VZ] - region->getWaterHeight() : 0.f;
		if (camera_water_height < 0.f)
		{
			gAudiop->setRolloffFactor(gSavedSettings.getF32("AudioLevelRolloff") * LL_ROLLOFF_MULTIPLIER_UNDER_WATER);
		}
		else 
		{
			gAudiop->setRolloffFactor(gSavedSettings.getF32("AudioLevelRolloff"));
		}
#endif
		return true;
	}
};
static AudioSettingsListener audio_settings_listener;

void init_audio() 
{
	if (!gAudiop) 
	{
		llwarns << "Failed to create an appropriate Audio Engine" << llendl;
		return;
	}
	LLVector3d lpos_global = gAgent.getCameraPositionGlobal();
	LLVector3 lpos_global_f;

	lpos_global_f.setVec(lpos_global);
					
	gAudiop->setListener(lpos_global_f,
						  LLVector3::zero,	// gCamera->getVelocity(),    // !!! BUG need to replace this with smoothed velocity!
						  gCamera->getUpAxis(),
						  gCamera->getAtAxis());

// load up our initial set of sounds we'll want so they're in memory and ready to be played

	BOOL mute_audio = gSavedSettings.getBOOL("MuteAudio");

	if (!mute_audio && gPreloadSounds)
	{
		gAudiop->preloadSound(LLUUID(gSavedSettings.getString("UISndAlert")));
		gAudiop->preloadSound(LLUUID(gSavedSettings.getString("UISndBadKeystroke")));
		//gAudiop->preloadSound(LLUUID(gSavedSettings.getString("UISndChatFromObject")));
		gAudiop->preloadSound(LLUUID(gSavedSettings.getString("UISndClick")));
		gAudiop->preloadSound(LLUUID(gSavedSettings.getString("UISndClickRelease")));
		gAudiop->preloadSound(LLUUID(gSavedSettings.getString("UISndHealthReductionF")));
		gAudiop->preloadSound(LLUUID(gSavedSettings.getString("UISndHealthReductionM")));
		//gAudiop->preloadSound(LLUUID(gSavedSettings.getString("UISndIncomingChat")));
		//gAudiop->preloadSound(LLUUID(gSavedSettings.getString("UISndIncomingIM")));
		//gAudiop->preloadSound(LLUUID(gSavedSettings.getString("UISndInvApplyToObject")));
		gAudiop->preloadSound(LLUUID(gSavedSettings.getString("UISndInvalidOp")));
		//gAudiop->preloadSound(LLUUID(gSavedSettings.getString("UISndInventoryCopyToInv")));
		gAudiop->preloadSound(LLUUID(gSavedSettings.getString("UISndMoneyChangeDown")));
		gAudiop->preloadSound(LLUUID(gSavedSettings.getString("UISndMoneyChangeUp")));
		//gAudiop->preloadSound(LLUUID(gSavedSettings.getString("UISndObjectCopyToInv")));
		gAudiop->preloadSound(LLUUID(gSavedSettings.getString("UISndObjectCreate")));
		gAudiop->preloadSound(LLUUID(gSavedSettings.getString("UISndObjectDelete")));
		gAudiop->preloadSound(LLUUID(gSavedSettings.getString("UISndObjectRezIn")));
		gAudiop->preloadSound(LLUUID(gSavedSettings.getString("UISndObjectRezOut")));
		gAudiop->preloadSound(LLUUID(gSavedSettings.getString("UISndPieMenuAppear")));
		gAudiop->preloadSound(LLUUID(gSavedSettings.getString("UISndPieMenuHide")));
		gAudiop->preloadSound(LLUUID(gSavedSettings.getString("UISndPieMenuSliceHighlight0")));
		gAudiop->preloadSound(LLUUID(gSavedSettings.getString("UISndPieMenuSliceHighlight1")));
		gAudiop->preloadSound(LLUUID(gSavedSettings.getString("UISndPieMenuSliceHighlight2")));
		gAudiop->preloadSound(LLUUID(gSavedSettings.getString("UISndPieMenuSliceHighlight3")));
		gAudiop->preloadSound(LLUUID(gSavedSettings.getString("UISndPieMenuSliceHighlight4")));
		gAudiop->preloadSound(LLUUID(gSavedSettings.getString("UISndPieMenuSliceHighlight5")));
		gAudiop->preloadSound(LLUUID(gSavedSettings.getString("UISndPieMenuSliceHighlight6")));
		gAudiop->preloadSound(LLUUID(gSavedSettings.getString("UISndPieMenuSliceHighlight7")));
		gAudiop->preloadSound(LLUUID(gSavedSettings.getString("UISndSnapshot")));
		//gAudiop->preloadSound(LLUUID(gSavedSettings.getString("UISndStartAutopilot")));
		//gAudiop->preloadSound(LLUUID(gSavedSettings.getString("UISndStartFollowpilot")));
		gAudiop->preloadSound(LLUUID(gSavedSettings.getString("UISndStartIM")));
		//gAudiop->preloadSound(LLUUID(gSavedSettings.getString("UISndStopAutopilot")));
		gAudiop->preloadSound(LLUUID(gSavedSettings.getString("UISndTeleportOut")));
		//gAudiop->preloadSound(LLUUID(gSavedSettings.getString("UISndTextureApplyToObject")));
		//gAudiop->preloadSound(LLUUID(gSavedSettings.getString("UISndTextureCopyToInv")));
		gAudiop->preloadSound(LLUUID(gSavedSettings.getString("UISndTyping")));
		gAudiop->preloadSound(LLUUID(gSavedSettings.getString("UISndWindowClose")));
		gAudiop->preloadSound(LLUUID(gSavedSettings.getString("UISndWindowOpen")));
	}

#ifdef kAUDIO_ENABLE_WIND
	gAudiop->enableWind(!mute_audio);
	gAudiop->mMaxWindGain = gSavedSettings.getF32("AudioLevelWind");
	gSavedSettings.getControl("AudioLevelWind")->addListener(&audio_settings_listener);
	gSavedSettings.getControl("AudioLevelRolloff")->addListener(&audio_settings_listener);
	// don't use the setter setMaxWindGain() because we don't
	// want to screw up the fade-in on startup by setting actual source gain
	// outside the fade-in.
#endif

	gAudiop->setMasterGain ( gSavedSettings.getF32 ( "AudioLevelMaster" ) );

	gAudiop->setDopplerFactor(gSavedSettings.getF32("AudioLevelDoppler"));
	gSavedSettings.getControl("AudioLevelDoppler")->addListener(&audio_settings_listener);
 	gAudiop->setDistanceFactor(gSavedSettings.getF32("AudioLevelDistance")); 
	gSavedSettings.getControl("AudioLevelDistance")->addListener(&audio_settings_listener);
	gAudiop->setRolloffFactor(gSavedSettings.getF32("AudioLevelRolloff"));
	gAudiop->setMuted(mute_audio);
}


/////////////////////////////////////////////////////////

BOOL raycast_for_new_obj_pos( S32 x, S32 y, LLViewerObject** hit_obj, S32* hit_face, 
							 BOOL* b_hit_land, LLVector3* ray_start_region, LLVector3* ray_end_region, LLViewerRegion** region )
{
	F32 max_dist_from_camera = gSavedSettings.getF32( "MaxSelectDistance" ) - 1.f;

	// Viewer-side pick to find the right sim to create the object on.  
	// First find the surface the object will be created on.
	gViewerWindow->hitObjectOrLandGlobalImmediate(x, y, NULL, FALSE);
	
	// Note: use the frontmost non-flora version because (a) plants usually have lots of alpha and (b) pants' Havok
	// representations (if any) are NOT the same as their viewer representation.
	*hit_obj = gObjectList.findObject( gLastHitNonFloraObjectID );
	*hit_face = gLastHitNonFloraObjectFace;
	*b_hit_land = !(*hit_obj) && !gLastHitNonFloraPosGlobal.isExactlyZero();
	LLVector3d land_pos_global = gLastHitNonFloraPosGlobal;

	// Make sure there's a surface to place the new object on.
	BOOL bypass_sim_raycast = FALSE;
	LLVector3d	surface_pos_global;
	if (*b_hit_land)
	{
		surface_pos_global = land_pos_global; 
		bypass_sim_raycast = TRUE;
	}
	else 
	if (*hit_obj)
	{
		surface_pos_global = (*hit_obj)->getPositionGlobal();
	}
	else
	{
		return FALSE;
	}

	// Make sure the surface isn't too far away.
	LLVector3d ray_start_global = gAgent.getCameraPositionGlobal();
	F32 dist_to_surface_sq = (F32)((surface_pos_global - ray_start_global).magVecSquared());
	if( dist_to_surface_sq > (max_dist_from_camera * max_dist_from_camera) )
	{
		return FALSE;
	}

	// Find the sim where the surface lives.
	LLViewerRegion *regionp = gWorldp->getRegionFromPosGlobal(surface_pos_global);
	if (!regionp)
	{
		llwarns << "Trying to add object outside of all known regions!" << llendl;
		return FALSE;
	}

	// Find the simulator-side ray that will be used to place the object accurately
	LLVector3d		mouse_direction;
	mouse_direction.setVec( gViewerWindow->mouseDirectionGlobal( x, y ) );

	*region = regionp;
	*ray_start_region =	regionp->getPosRegionFromGlobal( ray_start_global );
	F32 near_clip = gCamera->getNear() + 0.01f;  // Include an epsilon to avoid rounding issues.
	*ray_start_region += gCamera->getAtAxis() * near_clip;

	if( bypass_sim_raycast )
	{
		// Hack to work around Havok's inability to ray cast onto height fields
		*ray_end_region = regionp->getPosRegionFromGlobal( surface_pos_global );  // ray end is the viewer's intersection point
	}
	else
	{
		LLVector3d		ray_end_global = ray_start_global + (1.f + max_dist_from_camera) * mouse_direction;  // add an epsilon to the sim version of the ray to avoid rounding problems.
		*ray_end_region = regionp->getPosRegionFromGlobal( ray_end_global );
	}

	return TRUE;
}

const LLVector3 DEFAULT_OBJECT_SCALE(0.5f, 0.5f, 0.5f);

BOOL add_object( LLPCode pcode, S32 x, S32 y, U8 use_physics )
{
	LLVector3 ray_start_region;
	LLVector3 ray_end_region;
	LLViewerRegion* regionp = NULL;
	BOOL b_hit_land = FALSE;
	S32 hit_face = -1;
	LLViewerObject* hit_obj = NULL;
	U8 state = 0;
	BOOL success = raycast_for_new_obj_pos( x, y, &hit_obj, &hit_face, &b_hit_land, &ray_start_region, &ray_end_region, &regionp );
	if( !success )
	{
		return FALSE;
	}

	if( hit_obj && (hit_obj->isAvatar() || hit_obj->isAttachment()) )
	{
		// Can't create objects on avatars or attachments
		return FALSE;
	}

	if (NULL == regionp)
	{
		llwarns << "regionp was NULL; aborting function." << llendl;
		return FALSE;
	}

	if (regionp->getRegionFlags() & REGION_FLAGS_SANDBOX)
	{
		LLFirstUse::useSandbox();
	}

	// Set params for new object based on its PCode.
	LLQuaternion	rotation;
	LLVector3		scale = DEFAULT_OBJECT_SCALE;
	U8				material = LL_MCODE_WOOD;
	BOOL			create_selected = FALSE;
	LLVolumeParams	volume_params;
	
	switch (pcode) 
	{
	case LL_PCODE_LEGACY_GRASS:
		//  Randomize size of grass patch 
		scale.setVec(10.f + ll_frand(20.f), 10.f + ll_frand(20.f),  1.f + ll_frand(2.f));
		state = rand() % LLVOGrass::sMaxGrassSpecies;
		break;


	case LL_PCODE_LEGACY_TREE:
	case LL_PCODE_TREE_NEW:
		state = rand() % LLVOTree::sMaxTreeSpecies;
		break;

	case LL_PCODE_SPHERE:
	case LL_PCODE_CONE:
	case LL_PCODE_CUBE:
	case LL_PCODE_CYLINDER:
	case LL_PCODE_TORUS:
	case LLViewerObject::LL_VO_SQUARE_TORUS:
	case LLViewerObject::LL_VO_TRIANGLE_TORUS:
	default:
		create_selected = TRUE;
		break;
	}

	// Play creation sound
	if (gAudiop)
	{
		gAudiop->triggerSound( LLUUID(gSavedSettings.getString("UISndObjectCreate")), gAgent.getID(), 1.f);
	}

	gMessageSystem->newMessageFast(_PREHASH_ObjectAdd);
	gMessageSystem->nextBlockFast(_PREHASH_AgentData);
	gMessageSystem->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
	gMessageSystem->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
	gMessageSystem->addUUIDFast(_PREHASH_GroupID, gAgent.getGroupID());
	gMessageSystem->nextBlockFast(_PREHASH_ObjectData);
	gMessageSystem->addU8Fast(_PREHASH_Material,	material);

	U32 flags = 0;		// not selected
	if (use_physics)
	{
		flags |= FLAGS_USE_PHYSICS;
	}
	if (create_selected)
	{
		flags |= FLAGS_CREATE_SELECTED;
	}
	gMessageSystem->addU32Fast(_PREHASH_AddFlags, flags );

	LLPCode volume_pcode;	// ...PCODE_VOLUME, or the original on error
	switch (pcode)
	{
	case LL_PCODE_SPHERE:
		rotation.setQuat(90.f * DEG_TO_RAD, LLVector3::y_axis);

		volume_params.setType( LL_PCODE_PROFILE_CIRCLE_HALF, LL_PCODE_PATH_CIRCLE );
		volume_params.setBeginAndEndS( 0.f, 1.f );
		volume_params.setBeginAndEndT( 0.f, 1.f );
		volume_params.setRatio	( 1, 1 );
		volume_params.setShear	( 0, 0 );
		LLVolumeMessage::packVolumeParams(&volume_params, gMessageSystem);
		volume_pcode = LL_PCODE_VOLUME;
		break;

	case LL_PCODE_TORUS:
		rotation.setQuat(90.f * DEG_TO_RAD, LLVector3::y_axis);

		volume_params.setType( LL_PCODE_PROFILE_CIRCLE, LL_PCODE_PATH_CIRCLE );
		volume_params.setBeginAndEndS( 0.f, 1.f );
		volume_params.setBeginAndEndT( 0.f, 1.f );
		volume_params.setRatio	( 1.f, 0.25f );	// "top size"
		volume_params.setShear	( 0, 0 );
		LLVolumeMessage::packVolumeParams(&volume_params, gMessageSystem);
		volume_pcode = LL_PCODE_VOLUME;
		break;

	case LLViewerObject::LL_VO_SQUARE_TORUS:
		rotation.setQuat(90.f * DEG_TO_RAD, LLVector3::y_axis);

		volume_params.setType( LL_PCODE_PROFILE_SQUARE, LL_PCODE_PATH_CIRCLE );
		volume_params.setBeginAndEndS( 0.f, 1.f );
		volume_params.setBeginAndEndT( 0.f, 1.f );
		volume_params.setRatio	( 1.f, 0.25f );	// "top size"
		volume_params.setShear	( 0, 0 );
		LLVolumeMessage::packVolumeParams(&volume_params, gMessageSystem);
		volume_pcode = LL_PCODE_VOLUME;
		break;

	case LLViewerObject::LL_VO_TRIANGLE_TORUS:
		rotation.setQuat(90.f * DEG_TO_RAD, LLVector3::y_axis);

		volume_params.setType( LL_PCODE_PROFILE_EQUALTRI, LL_PCODE_PATH_CIRCLE );
		volume_params.setBeginAndEndS( 0.f, 1.f );
		volume_params.setBeginAndEndT( 0.f, 1.f );
		volume_params.setRatio	( 1.f, 0.25f );	// "top size"
		volume_params.setShear	( 0, 0 );
		LLVolumeMessage::packVolumeParams(&volume_params, gMessageSystem);
		volume_pcode = LL_PCODE_VOLUME;
		break;

	case LL_PCODE_SPHERE_HEMI:
		volume_params.setType( LL_PCODE_PROFILE_CIRCLE_HALF, LL_PCODE_PATH_CIRCLE );
		//volume_params.setBeginAndEndS( 0.5f, 1.f );
		volume_params.setBeginAndEndT( 0.f, 0.5f );
		volume_params.setRatio	( 1, 1 );
		volume_params.setShear	( 0, 0 );
		LLVolumeMessage::packVolumeParams(&volume_params, gMessageSystem);
		volume_pcode = LL_PCODE_VOLUME;
		break;

	case LL_PCODE_CUBE:
		volume_params.setType( LL_PCODE_PROFILE_SQUARE, LL_PCODE_PATH_LINE );
		volume_params.setBeginAndEndS( 0.f, 1.f );
		volume_params.setBeginAndEndT( 0.f, 1.f );
		volume_params.setRatio	( 1, 1 );
		volume_params.setShear	( 0, 0 );
		LLVolumeMessage::packVolumeParams(&volume_params, gMessageSystem);
		volume_pcode = LL_PCODE_VOLUME;
		break;

	case LL_PCODE_PRISM:
		volume_params.setType( LL_PCODE_PROFILE_SQUARE, LL_PCODE_PATH_LINE );
		volume_params.setBeginAndEndS( 0.f, 1.f );
		volume_params.setBeginAndEndT( 0.f, 1.f );
		volume_params.setRatio	( 0, 1 );
		volume_params.setShear	( -0.5f, 0 );
		LLVolumeMessage::packVolumeParams(&volume_params, gMessageSystem);
		volume_pcode = LL_PCODE_VOLUME;
		break;

	case LL_PCODE_PYRAMID:
		volume_params.setType( LL_PCODE_PROFILE_SQUARE, LL_PCODE_PATH_LINE );
		volume_params.setBeginAndEndS( 0.f, 1.f );
		volume_params.setBeginAndEndT( 0.f, 1.f );
		volume_params.setRatio	( 0, 0 );
		volume_params.setShear	( 0, 0 );
		LLVolumeMessage::packVolumeParams(&volume_params, gMessageSystem);
		volume_pcode = LL_PCODE_VOLUME;
		break;

	case LL_PCODE_TETRAHEDRON:
		volume_params.setType( LL_PCODE_PROFILE_EQUALTRI, LL_PCODE_PATH_LINE );
		volume_params.setBeginAndEndS( 0.f, 1.f );
		volume_params.setBeginAndEndT( 0.f, 1.f );
		volume_params.setRatio	( 0, 0 );
		volume_params.setShear	( 0, 0 );
		LLVolumeMessage::packVolumeParams(&volume_params, gMessageSystem);
		volume_pcode = LL_PCODE_VOLUME;
		break;

	case LL_PCODE_CYLINDER:
		volume_params.setType( LL_PCODE_PROFILE_CIRCLE, LL_PCODE_PATH_LINE );
		volume_params.setBeginAndEndS( 0.f, 1.f );
		volume_params.setBeginAndEndT( 0.f, 1.f );
		volume_params.setRatio	( 1, 1 );
		volume_params.setShear	( 0, 0 );
		LLVolumeMessage::packVolumeParams(&volume_params, gMessageSystem);
		volume_pcode = LL_PCODE_VOLUME;
		break;

	case LL_PCODE_CYLINDER_HEMI:
		volume_params.setType( LL_PCODE_PROFILE_CIRCLE, LL_PCODE_PATH_LINE );
		volume_params.setBeginAndEndS( 0.25f, 0.75f );
		volume_params.setBeginAndEndT( 0.f, 1.f );
		volume_params.setRatio	( 1, 1 );
		volume_params.setShear	( 0, 0 );
		LLVolumeMessage::packVolumeParams(&volume_params, gMessageSystem);
		volume_pcode = LL_PCODE_VOLUME;
		break;

	case LL_PCODE_CONE:
		volume_params.setType( LL_PCODE_PROFILE_CIRCLE, LL_PCODE_PATH_LINE );
		volume_params.setBeginAndEndS( 0.f, 1.f );
		volume_params.setBeginAndEndT( 0.f, 1.f );
		volume_params.setRatio	( 0, 0 );
		volume_params.setShear	( 0, 0 );
		LLVolumeMessage::packVolumeParams(&volume_params, gMessageSystem);
		volume_pcode = LL_PCODE_VOLUME;
		break;

	case LL_PCODE_CONE_HEMI:
		volume_params.setType( LL_PCODE_PROFILE_CIRCLE, LL_PCODE_PATH_LINE );
		volume_params.setBeginAndEndS( 0.25f, 0.75f );
		volume_params.setBeginAndEndT( 0.f, 1.f );
		volume_params.setRatio	( 0, 0 );
		volume_params.setShear	( 0, 0 );
		LLVolumeMessage::packVolumeParams(&volume_params, gMessageSystem);
		volume_pcode = LL_PCODE_VOLUME;
		break;

	default:
		LLVolumeMessage::packVolumeParams(0, gMessageSystem);
		volume_pcode = pcode;
		break;
	}
	gMessageSystem->addU8Fast(_PREHASH_PCode, volume_pcode);

	gMessageSystem->addVector3Fast(_PREHASH_Scale,			scale );
	gMessageSystem->addQuatFast(_PREHASH_Rotation,			rotation );
	gMessageSystem->addVector3Fast(_PREHASH_RayStart,		ray_start_region );
	gMessageSystem->addVector3Fast(_PREHASH_RayEnd,			ray_end_region );
	gMessageSystem->addU8Fast(_PREHASH_BypassRaycast,		(U8)b_hit_land );
	gMessageSystem->addU8Fast(_PREHASH_RayEndIsIntersection, (U8)FALSE );
	gMessageSystem->addU8Fast(_PREHASH_State, state);

	// Limit raycast to a single object.  
	// Speeds up server raycast + avoid problems with server ray hitting objects
	// that were clipped by the near plane or culled on the viewer.
	LLUUID ray_target_id;
	if( hit_obj )
	{
		ray_target_id = hit_obj->getID();
	}
	else
	{
		ray_target_id.setNull();
	}
	gMessageSystem->addUUIDFast(_PREHASH_RayTargetID,			ray_target_id );
	
	// Pack in name value pairs
	gMessageSystem->sendReliable(regionp->getHost());

	// Spawns a message, so must be after above send
	if (create_selected)
	{
		gSelectMgr->deselectAll();
		gViewerWindow->getWindow()->incBusyCount();
	}

	// VEFFECT: AddObject
	LLHUDEffectSpiral *effectp = (LLHUDEffectSpiral *)gHUDManager->createViewerEffect(LLHUDObject::LL_HUD_EFFECT_BEAM, TRUE);
	effectp->setSourceObject(gAgent.getAvatarObject());
	effectp->setPositionGlobal(regionp->getPosGlobalFromRegion(ray_end_region));
	effectp->setDuration(LL_HUD_DUR_SHORT);
	effectp->setColor(LLColor4U(gAgent.getEffectColor()));

	gViewerStats->incStat(LLViewerStats::ST_CREATE_COUNT);

	return TRUE;
}



#if LL_WINDOWS
void create_console()
{
	int h_con_handle;
	long l_std_handle;

	CONSOLE_SCREEN_BUFFER_INFO coninfo;
	FILE *fp;

	// allocate a console for this app
	AllocConsole();

	// set the screen buffer to be big enough to let us scroll text
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);
	coninfo.dwSize.Y = MAX_CONSOLE_LINES;
	SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);

	// redirect unbuffered STDOUT to the console
	l_std_handle = (long)GetStdHandle(STD_OUTPUT_HANDLE);
	h_con_handle = _open_osfhandle(l_std_handle, _O_TEXT);
	fp = _fdopen( h_con_handle, "w" );
	*stdout = *fp;
	setvbuf( stdout, NULL, _IONBF, 0 );

	// redirect unbuffered STDIN to the console
	l_std_handle = (long)GetStdHandle(STD_INPUT_HANDLE);
	h_con_handle = _open_osfhandle(l_std_handle, _O_TEXT);
	fp = _fdopen( h_con_handle, "r" );
	*stdin = *fp;
	setvbuf( stdin, NULL, _IONBF, 0 );

	// redirect unbuffered STDERR to the console
	l_std_handle = (long)GetStdHandle(STD_ERROR_HANDLE);
	h_con_handle = _open_osfhandle(l_std_handle, _O_TEXT);
	fp = _fdopen( h_con_handle, "w" );
	*stderr = *fp;
	setvbuf( stderr, NULL, _IONBF, 0 );
}
#endif

class LLAFKTimeoutListener: public LLSimpleListener
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		gAFKTimeout = (F32) event->getValue().asReal();
		return true;
	}
};
static LLAFKTimeoutListener afk_timeout_listener;

class LLMouseSensitivityListener: public LLSimpleListener
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		gMouseSensitivity = (F32) event->getValue().asReal();
		return true;
	}
};
static LLMouseSensitivityListener mouse_sensitivity_listener;


class LLInvertMouseListener: public LLSimpleListener
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		gInvertMouse = event->getValue().asBoolean();
		return true;
	}
};
static LLInvertMouseListener invert_mouse_listener;

class LLRenderAvatarMouselookListener: public LLSimpleListener
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLVOAvatar::sVisibleInFirstPerson = event->getValue().asBoolean();
		return true;
	}
};
static LLRenderAvatarMouselookListener render_avatar_mouselook_listener;

class LLRenderFarClipListener: public LLSimpleListener
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		F32 draw_distance = (F32) event->getValue().asReal();
		gAgent.mDrawDistance = draw_distance;
		if (gWorldPointer)
		{
			gWorldPointer->setLandFarClip(draw_distance);
		}
		return true;
	}
};
static LLRenderFarClipListener render_far_clip_listener;

class LLTerrainDetailListener: public LLSimpleListener
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLDrawPoolTerrain::sDetailMode = event->getValue().asInteger();
		return true;
	}
};
static LLTerrainDetailListener terrain_detail_listener;


class LLSetShaderListener: public LLSimpleListener
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLShaderMgr::setShaders();
		return true;
	}
};
static LLSetShaderListener set_shader_listener;

class LLReleaseGLBufferListener: public LLSimpleListener
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		gPipeline.releaseGLBuffers();
		LLShaderMgr::setShaders();
		return true;
	}
};
static LLReleaseGLBufferListener release_gl_buffer_listener;

class LLVolumeLODListener: public LLSimpleListener
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLVOVolume::sLODFactor = (F32) event->getValue().asReal();
		LLVOVolume::sDistanceFactor = 1.f-LLVOVolume::sLODFactor * 0.1f;
		return true;
	}
};
static LLVolumeLODListener volume_lod_listener;

class LLAvatarLODListener: public LLSimpleListener
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLVOAvatar::sLODFactor = (F32) event->getValue().asReal();
		return true;
	}
};
static LLAvatarLODListener avatar_lod_listener;

class LLTreeLODListener: public LLSimpleListener
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLVOTree::sTreeFactor = (F32) event->getValue().asReal();
		return true;
	}
};
static LLTreeLODListener tree_lod_listener;

class LLFlexLODListener: public LLSimpleListener
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLVolumeImplFlexible::sUpdateFactor = (F32) event->getValue().asReal();
		return true;
	}
};
static LLFlexLODListener flex_lod_listener;

class LLGammaListener: public LLSimpleListener
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		F32 gamma = (F32) event->getValue().asReal();
		if (gamma == 0.0f)
		{
			gamma = 1.0f; // restore normal gamma
		}
		if (gamma != gViewerWindow->getWindow()->getGamma())
		{
			// Only save it if it's changed
			if (!gViewerWindow->getWindow()->setGamma(gamma))
			{
				llwarns << "setGamma failed!" << llendl;
			}
		}

		return true;
	}
};
static LLGammaListener gamma_listener;

class LLNightBrightnessListener: public LLSimpleListener
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLVOSky::sNighttimeBrightness = (F32) event->getValue().asReal();
		return true;
	}
};
static LLNightBrightnessListener night_brightness_listener;

class LLFogRatioListener: public LLSimpleListener
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		F32 fog_ratio = llmax(MIN_USER_FOG_RATIO, 
							llmin((F32) event->getValue().asReal(), 
							MAX_USER_FOG_RATIO));
		gSky.setFogRatio(fog_ratio);
		return true;
	}
};
static LLFogRatioListener fog_ratio_listener;

class LLMaxPartCountListener: public LLSimpleListener
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLViewerPartSim::setMaxPartCount(event->getValue().asInteger());
		return true;
	}
};
static LLMaxPartCountListener max_partCount_listener;

class LLCompositeLimitListener: public LLSimpleListener
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		S32 composite_limit = llmax(MIN_USER_COMPOSITE_LIMIT, 
							llmin((S32)event->getValue().asInteger(), 
							MAX_USER_COMPOSITE_LIMIT));
		LLVOAvatar::sMaxOtherAvatarsToComposite = composite_limit;
		return true;
	}
};
static LLCompositeLimitListener composite_limit_listener;

class LLVideoMemoryListener: public LLSimpleListener
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		gImageList.updateMaxResidentTexMem(event->getValue().asInteger());
		return true;
	}
};
static LLVideoMemoryListener video_memory_listener;

class LLBandwidthListener: public LLSimpleListener
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		gViewerThrottle.setMaxBandwidth((F32) event->getValue().asReal());
		return true;
	}
};
static LLBandwidthListener bandwidth_listener;

class LLChatFontSizeListener: public LLSimpleListener
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		gConsole->setFontSize(event->getValue().asInteger());
		return true;
	}
};
static LLChatFontSizeListener chat_font_size_listener;

class LLChatPersistTimeListener: public LLSimpleListener
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		gConsole->setLinePersistTime((F32) event->getValue().asReal());
		return true;
	}
};
static LLChatPersistTimeListener chat_persist_time_listener;

class LLConsoleMaxLinesListener: public LLSimpleListener
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		gConsole->setMaxLines(event->getValue().asInteger());
		return true;
	}
};
static LLConsoleMaxLinesListener console_max_lines_listener;


class LLMasterAudioListener: public LLSimpleListener
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		if(gAudiop)
		{
			gAudiop->setMasterGain ((F32) event->getValue().asReal() );
		}

		if (LLMediaEngine::getInstance ()->isAvailable())
		{
			LLMediaEngine::getInstance ()->setVolume ((F32) event->getValue().asReal() );
		}

		return true;
	}
};

class LLJoystickListener : public LLSimpleListener
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLViewerJoystick::updateCamera(TRUE);
		return true;
	}
};
static LLJoystickListener joystick_listener;

void stop_video();
void prepare_video(const LLParcel *parcel);

static LLMasterAudioListener master_audio_listener;


class LLAudioStreamMusicListener: public LLSimpleListener
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		if (gAudiop)
		{
			if ( event->getValue().asBoolean() )
			{
				if (gParcelMgr
					&& gParcelMgr->getAgentParcel()
					&& gParcelMgr->getAgentParcel()->getMusicURL())
				{
					// if stream is already playing, don't call this
					// otherwise music will briefly stop
					if ( ! gAudiop->isInternetStreamPlaying () )
					{
						gAudiop->startInternetStream(
							gParcelMgr->getAgentParcel()->getMusicURL());
					}
				}
			}
			else
			{
				gAudiop->stopInternetStream();
			}
		}
		return true;
	}
};

static LLAudioStreamMusicListener audio_stream_music_listener;



class LLAudioStreamMediaListener: public LLSimpleListener
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		if (LLMediaEngine::getInstance() && LLMediaEngine::getInstance()->isAvailable())
		{
			if (event->getValue().asBoolean())
			{
				gMessageSystem->setHandlerFunc ( "ParcelMediaCommandMessage", LLMediaEngine::process_parcel_media );
				gMessageSystem->setHandlerFunc ( "ParcelMediaUpdate", LLMediaEngine::process_parcel_media_update );
				if ( ( gParcelMgr ) &&
					( gParcelMgr->getAgentParcel () ) && 
						( gParcelMgr->getAgentParcel()->getMediaURL () ) )
				{
					prepare_video ( gParcelMgr->getAgentParcel () );
				}
			}
			else
			{
				gMessageSystem->setHandlerFunc("ParcelMediaCommandMessage", null_message_callback);
				gMessageSystem->setHandlerFunc ( "ParcelMediaUpdate", null_message_callback );
				stop_video();
			}
		}
		else
		{
			if (gSavedSettings.getWarning("QuickTimeInstalled"))
			{
				gSavedSettings.setWarning("QuickTimeInstalled", FALSE);

				LLNotifyBox::showXml("NoQuickTime" );
			}
		}

		return true;
	}
};

static LLAudioStreamMediaListener audio_stream_media_listener;


class LLAudioMuteListener: public LLSimpleListener
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		if (gAudiop)
		{
			gAudiop->setMuted(event->getValue().asBoolean());
		}
		return true;
	}
};

static LLAudioMuteListener audio_mute_listener;

class LLUseOcclusionListener: public LLSimpleListener
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLPipeline::sUseOcclusion = (event->getValue().asBoolean() && gGLManager.mHasOcclusionQuery &&
			!gUseWireframe);
		return true;
	}
};
static LLUseOcclusionListener use_occlusion_listener;

class LLNumpadControlListener: public LLSimpleListener
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		if (gKeyboard)
		{
			gKeyboard->setNumpadDistinct(static_cast<LLKeyboard::e_numpad_distinct>(event->getValue().asInteger()));
		}
		return true;
	}
};

static LLNumpadControlListener numpad_control_listener;

class LLRenderUseVBOListener: public LLSimpleListener
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		gPipeline.setUseVBO(event->getValue().asBoolean());
		return true;
	}
};
static LLRenderUseVBOListener render_use_vbo_listener;

class LLRenderLightingDetailListener: public LLSimpleListener
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		gPipeline.setLightingDetail(event->getValue().asInteger());
		return true;
	}
};
static LLRenderLightingDetailListener render_lighting_detail_listener;

// Use these strictly for things that are constructed at startup,
// or for things that are performance critical.  JC
void saved_settings_to_globals()
{
	LLBUTTON_H_PAD		= gSavedSettings.getS32("ButtonHPad");
	LLBUTTON_V_PAD		= gSavedSettings.getS32("ButtonVPad");
	BTN_HEIGHT_SMALL	= gSavedSettings.getS32("ButtonHeightSmall");
	BTN_HEIGHT			= gSavedSettings.getS32("ButtonHeight");

	MENU_BAR_HEIGHT		= gSavedSettings.getS32("MenuBarHeight");
	MENU_BAR_WIDTH		= gSavedSettings.getS32("MenuBarWidth");
	STATUS_BAR_HEIGHT	= gSavedSettings.getS32("StatusBarHeight");

	LLCOMBOBOX_HEIGHT	= BTN_HEIGHT - 2;
	LLCOMBOBOX_WIDTH	= 128;

	LLSurface::setTextureSize(gSavedSettings.getU32("RegionTextureSize"));

	LLVOSky::sNighttimeBrightness		= gSavedSettings.getF32("RenderNightBrightness");
	
	LLImageGL::sGlobalUseAnisotropic	= gSavedSettings.getBOOL("RenderAnisotropic");
	LLVOVolume::sLODFactor				= gSavedSettings.getF32("RenderVolumeLODFactor");
	LLVOVolume::sDistanceFactor			= 1.f-LLVOVolume::sLODFactor * 0.1f;
	LLVolumeImplFlexible::sUpdateFactor = gSavedSettings.getF32("RenderFlexTimeFactor");
	LLVOTree::sTreeFactor				= gSavedSettings.getF32("RenderTreeLODFactor");
	LLVOAvatar::sLODFactor				= gSavedSettings.getF32("RenderAvatarLODFactor");
	LLVOAvatar::sMaxVisible				= gSavedSettings.getS32("RenderAvatarMaxVisible");
	LLVOAvatar::sVisibleInFirstPerson	= gSavedSettings.getBOOL("FirstPersonAvatarVisible");
	// clamp auto-open time to some minimum usable value
	LLFolderView::sAutoOpenTime			= llmax(0.25f, gSavedSettings.getF32("FolderAutoOpenDelay"));
	LLToolBar::sInventoryAutoOpenTime	= gSavedSettings.getF32("InventoryAutoOpenDelay");
	LLSelectMgr::sRectSelectInclusive	= gSavedSettings.getBOOL("RectangleSelectInclusive");
	LLSelectMgr::sRenderHiddenSelections = gSavedSettings.getBOOL("RenderHiddenSelections");
	LLSelectMgr::sRenderLightRadius = gSavedSettings.getBOOL("RenderLightRadius");

	gFrameStats.setTrackStats(gSavedSettings.getBOOL("StatsSessionTrackFrameStats"));
	gAgentPilot.mNumRuns		= gSavedSettings.getS32("StatsNumRuns");
	gAgentPilot.mQuitAfterRuns	= gSavedSettings.getBOOL("StatsQuitAfterRuns");
	gAgent.mHideGroupTitle		= gSavedSettings.getBOOL("RenderHideGroupTitle");

	gDebugWindowProc = gSavedSettings.getBOOL("DebugWindowProc");
	gAllowAFK = gSavedSettings.getBOOL("AllowAFK");
	gAFKTimeout = gSavedSettings.getF32("AFKTimeout");
	gMouseSensitivity = gSavedSettings.getF32("MouseSensitivity");
	gInvertMouse = gSavedSettings.getBOOL("InvertMouse");
	gShowObjectUpdates = gSavedSettings.getBOOL("ShowObjectUpdates");
	gMapScale = gSavedSettings.getF32("MapScale");
	gMiniMapScale = gSavedSettings.getF32("MiniMapScale");
	gHandleKeysAsync = gSavedSettings.getBOOL("AsyncKeyboard");
	LLHoverView::sShowHoverTips = gSavedSettings.getBOOL("ShowHoverTips");

	// Into a global in case we corrupt the list on crash.
	gCrashBehavior = gCrashSettings.getS32(CRASH_BEHAVIOR_SETTING);

	//various listeners
	gSavedSettings.getControl("FirstPersonAvatarVisible")->addListener(&render_avatar_mouselook_listener);
	gSavedSettings.getControl("MouseSensitivity")->addListener(&mouse_sensitivity_listener);
	gSavedSettings.getControl("InvertMouse")->addListener(&invert_mouse_listener);
	gSavedSettings.getControl("AFKTimeout")->addListener(&afk_timeout_listener);
	gSavedSettings.getControl("RenderFarClip")->addListener(&render_far_clip_listener);
	gSavedSettings.getControl("RenderTerrainDetail")->addListener(&terrain_detail_listener);
	gSavedSettings.getControl("RenderRippleWater")->addListener(&set_shader_listener);
	gSavedSettings.getControl("RenderAvatarVP")->addListener(&set_shader_listener);
	gSavedSettings.getControl("VertexShaderEnable")->addListener(&set_shader_listener);
	gSavedSettings.getControl("RenderDynamicReflections")->addListener(&set_shader_listener);
	gSavedSettings.getControl("RenderGlow")->addListener(&release_gl_buffer_listener);
	gSavedSettings.getControl("RenderGlowResolution")->addListener(&release_gl_buffer_listener);
	gSavedSettings.getControl("RenderAvatarMode")->addListener(&set_shader_listener);
	gSavedSettings.getControl("RenderVolumeLODFactor")->addListener(&volume_lod_listener);
	gSavedSettings.getControl("RenderAvatarLODFactor")->addListener(&avatar_lod_listener);
	gSavedSettings.getControl("RenderTreeLODFactor")->addListener(&tree_lod_listener);
	gSavedSettings.getControl("RenderFlexTimeFactor")->addListener(&flex_lod_listener);
	gSavedSettings.getControl("ThrottleBandwidthKBPS")->addListener(&bandwidth_listener);
	gSavedSettings.getControl("RenderGamma")->addListener(&gamma_listener);
	gSavedSettings.getControl("RenderNightBrightness")->addListener(&night_brightness_listener);
	gSavedSettings.getControl("RenderFogRatio")->addListener(&fog_ratio_listener);
	gSavedSettings.getControl("RenderMaxPartCount")->addListener(&max_partCount_listener);
	gSavedSettings.getControl("AvatarCompositeLimit")->addListener(&composite_limit_listener);
	gSavedSettings.getControl("GraphicsCardMemorySetting")->addListener(&video_memory_listener);
	gSavedSettings.getControl("ChatFontSize")->addListener(&chat_font_size_listener);
	gSavedSettings.getControl("ChatPersistTime")->addListener(&chat_persist_time_listener);
	gSavedSettings.getControl("ConsoleMaxLines")->addListener(&console_max_lines_listener);
	gSavedSettings.getControl("UseOcclusion")->addListener(&use_occlusion_listener);
	gSavedSettings.getControl("AudioLevelMaster")->addListener(&master_audio_listener);
	gSavedSettings.getControl("AudioStreamingMusic")->addListener(&audio_stream_music_listener);
	gSavedSettings.getControl("AudioStreamingVideo")->addListener(&audio_stream_media_listener);
	gSavedSettings.getControl("MuteAudio")->addListener(&audio_mute_listener);
	gSavedSettings.getControl("RenderVBOEnable")->addListener(&render_use_vbo_listener);
	gSavedSettings.getControl("RenderLightingDetail")->addListener(&render_lighting_detail_listener);
	gSavedSettings.getControl("NumpadControl")->addListener(&numpad_control_listener);
	gSavedSettings.getControl("FlycamAxis0")->addListener(&joystick_listener);
	gSavedSettings.getControl("FlycamAxis1")->addListener(&joystick_listener);
	gSavedSettings.getControl("FlycamAxis2")->addListener(&joystick_listener);
	gSavedSettings.getControl("FlycamAxis3")->addListener(&joystick_listener);
	gSavedSettings.getControl("FlycamAxis4")->addListener(&joystick_listener);
	gSavedSettings.getControl("FlycamAxis5")->addListener(&joystick_listener);
	gSavedSettings.getControl("FlycamAxis6")->addListener(&joystick_listener);
	
	// gAgent.init() also loads from saved settings.
}

void cleanup_saved_settings()
{
	gSavedSettings.setBOOL("MouseSun", FALSE);

	gSavedSettings.setBOOL("FlyBtnState", FALSE);

	gSavedSettings.setBOOL("FirstPersonBtnState", FALSE);
	gSavedSettings.setBOOL("ThirdPersonBtnState", TRUE);
	gSavedSettings.setBOOL("BuildBtnState", FALSE);

	gSavedSettings.setBOOL("UseEnergy", TRUE);				// force toggle to turn off, since sends message to simulator

	gSavedSettings.setBOOL("DebugWindowProc", gDebugWindowProc);
		
	gSavedSettings.setBOOL("AllowAFK", gAllowAFK);
	gSavedSettings.setBOOL("ShowObjectUpdates", gShowObjectUpdates);
	
	if (!gNoRender)
	{
		if (gDebugView)
		{
			gSavedSettings.setBOOL("ShowDebugConsole", gDebugView->mDebugConsolep->getVisible());
			gSavedSettings.setBOOL("ShowDebugStats", gDebugView->mStatViewp->getVisible());
		}
	}

	// save window position if not fullscreen
	// as we don't track it in callbacks
	BOOL fullscreen = gViewerWindow->mWindow->getFullscreen();
	BOOL maximized = gViewerWindow->mWindow->getMaximized();
	if (!fullscreen && !maximized)
	{
		LLCoordScreen window_pos;

		if (gViewerWindow->mWindow->getPosition(&window_pos))
		{
			gSavedSettings.setS32("WindowX", window_pos.mX);
			gSavedSettings.setS32("WindowY", window_pos.mY);
		}
	}

	gSavedSettings.setF32("MapScale", gMapScale );
	gSavedSettings.setF32("MiniMapScale", gMiniMapScale );
	gSavedSettings.setBOOL("AsyncKeyboard", gHandleKeysAsync);
	gSavedSettings.setBOOL("ShowHoverTips", LLHoverView::sShowHoverTips);

	// Some things are cached in LLAgent.
	if (gAgent.mInitialized)
	{
		gSavedSettings.setF32("RenderFarClip", gAgent.mDrawDistance);
	}

	gCrashSettings.setS32(CRASH_BEHAVIOR_SETTING, gCrashBehavior);
}


void callback_cache_name(const LLUUID& id, const char* firstname, const char* lastname, BOOL is_group, void* data)
{
	LLNameListCtrl::refreshAll(id, firstname, lastname, is_group);
	LLNameBox::refreshAll(id, firstname, lastname, is_group);
	LLNameEditor::refreshAll(id, firstname, lastname, is_group);
	
	// TODO: Actually be intelligent about the refresh.
	// For now, just brute force refresh the dialogs.
	dialog_refresh_all();
}


void write_debug(const std::string& str)
{
	write_debug(str.c_str());
}

void write_debug(const char *str)
{
	if (!gDebugFile)
	{
		std::string debug_filename = gDirUtilp->getExpandedFilename(LL_PATH_LOGS,"debug_info.log");
		llinfos << "Opening debug file " << debug_filename << llendl;
		gDebugFile = LLFile::fopen(debug_filename.c_str(), "w");		/* Flawfinder: ignore */
        if (!gDebugFile)
        {
		    llinfos << "Opening debug file " << debug_filename << " failed. Using stderr." << llendl;
            gDebugFile = stderr;
        }
	}
	fputs(str, gDebugFile);
	fflush(gDebugFile);
}

void close_debug()
{
	if (gDebugFile)
	{
		fclose(gDebugFile);
	}
	gDebugFile = NULL;
}

void print_agent_nvpairs(void*)
{
	LLViewerObject *objectp;

	llinfos << "Agent Name Value Pairs" << llendl;

	objectp = gObjectList.findObject(gAgentID);
	if (objectp)
	{
		objectp->printNameValuePairs();
	}
	else
	{
		llinfos << "Can't find agent object" << llendl;
	}

	llinfos << "Camera at " << gAgent.getCameraPositionGlobal() << llendl;
}


void reset_statistics()
{
	gPipeline.resetFrameStats();	// Reset per-frame statistics.
	if (LLSurface::sTextureUpdateTime)
	{
		LLSurface::sTexelsUpdatedPerSecStat.addValue(0.001f*(LLSurface::sTexelsUpdated / LLSurface::sTextureUpdateTime));
		LLSurface::sTexelsUpdated = 0;
		LLSurface::sTextureUpdateTime = 0.f;
	}
}


void output_statistics(void*)
{
	llinfos << "Number of orphans: " << gObjectList.getOrphanCount() << llendl;
	llinfos << "Number of dead objects: " << gObjectList.mNumDeadObjects << llendl;
	llinfos << "Num images: " << gImageList.getNumImages() << llendl;
	llinfos << "Texture usage: " << LLImageGL::sGlobalTextureMemory << llendl;
	llinfos << "Texture working set: " << LLImageGL::sBoundTextureMemory << llendl;
	llinfos << "Raw usage: " << LLImageRaw::sGlobalRawMemory << llendl;
	llinfos << "Formatted usage: " << LLImageFormatted::sGlobalFormattedMemory << llendl;
	llinfos << "Zombie Viewer Objects: " << LLViewerObject::getNumZombieObjects() << llendl;
	llinfos << "Number of lights: " << gPipeline.getLightCount() << llendl;

	llinfos << "Memory Usage:" << llendl;
	llinfos << "--------------------------------" << llendl;
	llinfos << "Pipeline:" << llendl;
	llinfos << llendl;

#if LL_SMARTHEAP
	llinfos << "--------------------------------" << llendl;
	{
		llinfos << "sizeof(LLVOVolume) = " << sizeof(LLVOVolume) << llendl;

		U32 total_pool_size = 0;
		U32 total_used_size = 0;
		MEM_POOL_INFO pool_info;
		MEM_POOL_STATUS pool_status;
		U32 pool_num = 0;
		for(pool_status = MemPoolFirst( &pool_info, 1 ); 
			pool_status != MEM_POOL_END; 
			pool_status = MemPoolNext( &pool_info, 1 ) )
		{
			llinfos << "Pool #" << pool_num << llendl;
			if( MEM_POOL_OK != pool_status )
			{
				llwarns << "Pool not ok" << llendl;
				continue;
			}

			llinfos << "Pool blockSizeFS " << pool_info.blockSizeFS
				<< " pageSize " << pool_info.pageSize
				<< llendl;

			U32 pool_count = MemPoolCount(pool_info.pool);
			llinfos << "Blocks " << pool_count << llendl;

			U32 pool_size = MemPoolSize( pool_info.pool );
			if( pool_size == MEM_ERROR_RET )
			{
				llinfos << "MemPoolSize() failed (" << pool_num << ")" << llendl;
			}
			else
			{
				llinfos << "MemPool Size " << pool_size / 1024 << "K" << llendl;
			}

			total_pool_size += pool_size;

			if( !MemPoolLock( pool_info.pool ) )
			{
				llinfos << "MemPoolLock failed (" << pool_num << ") " << llendl;
				continue;
			}

			U32 used_size = 0; 
			MEM_POOL_ENTRY entry;
			entry.entry = NULL;
			while( MemPoolWalk( pool_info.pool, &entry ) == MEM_POOL_OK )
			{
				if( entry.isInUse )
				{
					used_size += entry.size;
				}
			}

			MemPoolUnlock( pool_info.pool );

			llinfos << "MemPool Used " << used_size/1024 << "K" << llendl;
			total_used_size += used_size;
			pool_num++;
		}
		
		llinfos << "Total Pool Size " << total_pool_size/1024 << "K" << llendl;
		llinfos << "Total Used Size " << total_used_size/1024 << "K" << llendl;

	}
#endif

	llinfos << "--------------------------------" << llendl;
	llinfos << "Avatar Memory (partly overlaps with above stats):" << llendl;
	gTexStaticImageList.dumpByteCount();
	LLVOAvatar::dumpScratchTextureByteCount();
	LLTexLayerSetBuffer::dumpTotalByteCount();
	LLVOAvatar::dumpTotalLocalTextureByteCount();
	LLTexLayerParamAlpha::dumpCacheByteCount();
	LLVOAvatar::dumpBakedStatus();

	llinfos << llendl;

	llinfos << "Object counts:" << llendl;
	S32 i;
	S32 obj_counts[256];
//	S32 app_angles[256];
	for (i = 0; i < 256; i++)
	{
		obj_counts[i] = 0;
	}
	for (i = 0; i < gObjectList.getNumObjects(); i++)
	{
		LLViewerObject *objectp = gObjectList.getObject(i);
		if (objectp)
		{
			obj_counts[objectp->getPCode()]++;
		}
	}
	for (i = 0; i < 256; i++)
	{
		if (obj_counts[i])
		{
			llinfos << LLPrimitive::pCodeToString(i) << ":" << obj_counts[i] << llendl;
		}
	}

	/*
	llinfos << "Object size distribution" << llendl;
	llinfos << "------------------------" << llendl;
	for (i = 0; i < 256; i++)
	{
		obj_counts[i] = 0;
		//app_angles[i] = 0;
	}
	for (i = 0; i < gObjectList.getNumObjects(); i++)
	{
		LLViewerObject *objectp = gObjectList.getObject(i);
		S32 size = llmin(255, (S32)objectp->getRadius()/2);
		obj_counts[size]++;
	}
	for (i = 0; i < 256; i++)
	{
		if (obj_counts[i])
		{
			llinfos << i*2 << ":" << (i+1)*2 << " - " << obj_counts[i] << llendl;
		}
	}
	*/

	/*

	llinfos << llendl;
	llinfos << "Lighting statistics" << llendl;
	llinfos << "-------------------" << llendl;
	S32 light_bins[256];
	S32 light_counts[256];
	for (i = 0; i < 256; i++)
	{
		light_bins[i] = 0;
		light_counts[i] = 0;
	}
	for (i = 0; i < gPipeline.mLightSet.count(); i++)
	{
		LLDrawable *drawablep = gPipeline.mLightSet[i];
		F32 light_range = drawablep->getMetric() * LIGHT_RANGE_FACTOR;
		S32 light_bin = light_range / 4;
		S32 count_bin = llmin(drawablep->mLightSet.getLength() / 25, 255);

		if (count_bin > 20)
		{
			llinfos << drawablep->getVObj()->getPCodeString() << ":" << count_bin*25 << llendl;
			drawablep->getVObj()->dump();
		}
		//llinfos << i << ": " << light_range << llendl;
		light_bins[light_bin]++;
		light_counts[count_bin]++;
	}
	for (i = 0; i < 256; i++)
	{
		if (light_bins[i])
		{
			llinfos << i*4 << ":" << (i+1)*4 << " - " << light_bins[i] << llendl;
		}
	}
	llinfos << llendl;
	llinfos << "LightSet counts" << llendl;
	for (i = 0; i < 256; i++)
	{
		if (light_counts[i])
		{
			llinfos << i*25 << ":" << (i+1)*25 << " - " << light_counts[i] << llendl;
		}
	}
	*/
}

void send_stats()
{
	// IW 9/23/02 I elected not to move this into LLViewerStats
	// because it depends on too many viewer.cpp globals.
	// Someday we may want to merge all our stats into a central place
	// but that day is not today.

	// Only send stats if the agent is connected to a region.
	if (!gAgent.getRegion() || gNoRender)
	{
		return;
	}

	gMessageSystem->newMessageFast(_PREHASH_ViewerStats);
	
	gMessageSystem->nextBlockFast(_PREHASH_AgentData);
	gMessageSystem->addUUIDFast(_PREHASH_AgentID, gAgentID);
	gMessageSystem->addUUIDFast(_PREHASH_SessionID, gAgentSessionID);
	
	// IP will get filled in by the receiver
	gMessageSystem->addIPAddrFast(_PREHASH_IP, 0);

	time_t	ltime;
	time(&ltime);
	F32 run_time = F32(LLFrameTimer::getElapsedSeconds());

	gMessageSystem->addU32Fast(_PREHASH_StartTime, ((U32)ltime - (U32)run_time));
	gMessageSystem->addF32Fast(_PREHASH_RunTime, run_time);
	gMessageSystem->addF32Fast(_PREHASH_FPS, ((F32)gFrameCount / gRenderStartTime.getElapsedTimeF32()));
	
	gMessageSystem->addF32Fast(_PREHASH_SimFPS, ((F32)gFrameCount - gSimFrames) / (F32)(gRenderStartTime.getElapsedTimeF32() - gSimLastTime) );
	gSimLastTime = gRenderStartTime.getElapsedTimeF32();
	gSimFrames   = (F32)gFrameCount;

	gMessageSystem->addU8Fast (_PREHASH_AgentsInView, (U8)LLVOAvatar::sNumVisibleAvatars );
	gMessageSystem->addF32Fast(_PREHASH_Ping, gAvgSimPing);
	gMessageSystem->addF64Fast(_PREHASH_MetersTraveled, gAgent.getDistanceTraveled());
	gMessageSystem->addS32Fast(_PREHASH_RegionsVisited, gAgent.getRegionsVisited());
	gMessageSystem->addU32Fast(_PREHASH_SysRAM, gSysMemory.getPhysicalMemory());
	gMessageSystem->addStringFast(_PREHASH_SysOS, gSysOS.getOSString());
	gMessageSystem->addStringFast(_PREHASH_SysCPU, gSysCPU.getCPUString());

	
	std::string gpu_desc = llformat("%-6s Class %d ",
									gGLManager.mGLVendorShort.substr(0,6).c_str(),
									gFeatureManagerp->getGPUClass())
		+ gFeatureManagerp->getGPUString();

	gMessageSystem->addStringFast(_PREHASH_SysGPU, gpu_desc);

	gMessageSystem->nextBlockFast(_PREHASH_DownloadTotals);
	gMessageSystem->addU32Fast(_PREHASH_World, gTotalWorldBytes);
	gMessageSystem->addU32Fast(_PREHASH_Objects, gTotalObjectBytes);
	gMessageSystem->addU32Fast(_PREHASH_Textures, gTotalTextureBytes);

	gMessageSystem->nextBlockFast(_PREHASH_NetStats);
	gMessageSystem->addU32Fast(_PREHASH_Bytes, (U32)gMessageSystem->mTotalBytesIn);
	gMessageSystem->addU32Fast(_PREHASH_Packets, (U32)gMessageSystem->mPacketsIn);
	gMessageSystem->addU32Fast(_PREHASH_Compressed, (U32)gMessageSystem->mCompressedPacketsIn);
	gMessageSystem->addU32Fast(_PREHASH_Savings, (U32)(gMessageSystem->mUncompressedBytesIn - gMessageSystem->mCompressedBytesIn));
	
	gMessageSystem->nextBlockFast(_PREHASH_NetStats);
	gMessageSystem->addU32Fast(_PREHASH_Bytes, (U32)gMessageSystem->mTotalBytesOut);
	gMessageSystem->addU32Fast(_PREHASH_Packets, (U32)gMessageSystem->mPacketsOut);
	gMessageSystem->addU32Fast(_PREHASH_Compressed, (U32)gMessageSystem->mCompressedPacketsOut);
	gMessageSystem->addU32Fast(_PREHASH_Savings, (U32)(gMessageSystem->mUncompressedBytesOut - gMessageSystem->mCompressedBytesOut));

	gMessageSystem->nextBlockFast(_PREHASH_FailStats);
	gMessageSystem->addU32Fast(_PREHASH_SendPacket, (U32)gMessageSystem->mSendPacketFailureCount);
	gMessageSystem->addU32Fast(_PREHASH_Dropped, (U32)gMessageSystem->mDroppedPackets);
	gMessageSystem->addU32Fast(_PREHASH_Resent, (U32)gMessageSystem->mResentPackets);
	gMessageSystem->addU32Fast(_PREHASH_FailedResends, (U32)gMessageSystem->mFailedResendPackets);
	gMessageSystem->addU32Fast(_PREHASH_OffCircuit, (U32)gMessageSystem->mOffCircuitPackets);
	gMessageSystem->addU32Fast(_PREHASH_Invalid, (U32)gMessageSystem->mInvalidOnCircuitPackets);

	// 1.00.00.000000
	F64 version =
		LL_VERSION_MAJOR * 10000000000.0 +
		LL_VERSION_MINOR *   100000000.0 +
		LL_VERSION_PATCH *     1000000.0 +
		LL_VERSION_BUILD;
	gViewerStats->setStat(LLViewerStats::ST_VERSION, version);
	
	gViewerStats->addToMessage();

	gAgent.sendReliableMessage();
}


#if !LL_WINDOWS
// catch the first signal and send logout messages logout
void signal_handlers(S32 s)
{
	release_signals();

	// Check for graceful exit conditions
	if(SIGTERM == s)
	{
		// Raise the flag telling the system to shut down
		app_force_quit(NULL);
		return;
	}

# if LL_LINUX
	// Really useful to know what KIND of crash we got.
	// Might want this on OSX too!
	llwarns << "*** Caught signal " << s << llendl;
# endif

	if (gCrashCallback)
	{
		gCrashCallback();
	}
}
#endif

// Assign signal handler to be called in case we crash.
void catch_signals()
{
#if LL_WINDOWS
	// Win32 signal handling uses a Microsoft API instead of signal handlers
	LLWinDebug::setupExceptionHandler();
	return;
#else
	// This happens before log file output is properly set up, so this would go to the system log.
//	llinfos << "Assigning signal handlers!" << llendl;

	// Handle the signals that default to causing a core image to be created, as per the man page on signal(2).
	signal(SIGILL, signal_handlers);
	signal(SIGTRAP, signal_handlers);
	signal(SIGABRT, signal_handlers);
	signal(SIGFPE, signal_handlers);
	signal(SIGBUS, signal_handlers);
	signal(SIGSEGV, signal_handlers);
	signal(SIGSYS, signal_handlers);

	// SIGEMT is an 'emulator trap' which is not defined on linux.
#if !LL_LINUX
	signal(SIGEMT, signal_handlers);
#endif

	// This one is treated specially in the signal handler.
	signal(SIGTERM, signal_handlers);

#endif // LL_WINDOWS
}

#if !LL_WINDOWS
void release_signals()
{
	signal(SIGTERM, SIG_DFL);
	signal(SIGABRT, SIG_DFL);
	signal(SIGILL, SIG_DFL);
	signal(SIGFPE, SIG_DFL);
	signal(SIGSEGV, SIG_DFL);

#ifdef SIGBUS
	signal(SIGBUS, SIG_DFL);
#endif

#ifdef SIGSYS
	signal(SIGSYS, SIG_DFL);
#endif
}
#endif // !LL_WINDOWS

void purge_cache()
{
	llinfos << "Purging Texture Cache..." << llendl;
	gTextureCache->purgeCache(LL_PATH_CACHE);
	llinfos << "Purging Cache..." << llendl;
	std::string mask = gDirUtilp->getDirDelimiter() + "*.*";
	gDirUtilp->deleteFilesInDir(gDirUtilp->getExpandedFilename(LL_PATH_CACHE,"").c_str(),mask);
}

int parse_args(int argc, char **argv)
{
	// Sometimes IP addresses passed in on the command line have leading
	// or trailing white space.  Use LLString to clean that up.
	LLString ip_string;

	S32 j;
	// agent_sim_host holds the settings for connecting to the first simulator.

	for (j = 1; j < argc; j++)
	{
		gArgs += argv[j];
		gArgs += " ";
	}

	for (j = 1; j < argc; j++) 
	{
		LLString argument = argv[j];
		if ((!strcmp(argv[j], "-port")) && (++j < argc)) 
		{
			sscanf(argv[j], "%u", &(gAgent.mViewerPort));
		}
		else if ((!strcmp(argv[j], "-drop")) && (++j < argc)) 
		{
			sscanf(argv[j], "%f", &gPacketDropPercentage);
		}
		else if ((!strcmp(argv[j], "-inbw")) && (++j < argc))
		{
			sscanf(argv[j], "%f", &gInBandwidth);
		}
		else if ((!strcmp(argv[j], "-outbw")) && (++j < argc))
		{
			sscanf(argv[j], "%f", &gOutBandwidth);
		}
		else if (!strcmp(argv[j], "--aditi"))
		{
			gUserServerChoice = USERSERVER_ADITI;
			snprintf(gUserServerName, MAX_STRING, "%s", gUserServerDomainName[gUserServerChoice].mName);		/* Flawfinder: ignore */
			gConnectToSomething = TRUE;
		}
		else if (!strcmp(argv[j], "--agni"))
		{
			gUserServerChoice = USERSERVER_AGNI;
			snprintf(gUserServerName, MAX_STRING, "%s", gUserServerDomainName[gUserServerChoice].mName);		/* Flawfinder: ignore */
			gConnectToSomething = TRUE;
		}
		else if (!strcmp(argv[j], "--dmz"))
		{
			gUserServerChoice = USERSERVER_DMZ;
			snprintf(gUserServerName, MAX_STRING, "%s", gUserServerDomainName[gUserServerChoice].mName);		/* Flawfinder: ignore */
			gConnectToSomething = TRUE;
		}
		else if (!strcmp(argv[j], "--siva"))
		{
			gUserServerChoice = USERSERVER_SIVA;
			snprintf(gUserServerName, MAX_STRING, "%s", gUserServerDomainName[gUserServerChoice].mName);		/* Flawfinder: ignore */
			gConnectToSomething = TRUE;
		}
		else if (!strcmp(argv[j], "--shakti"))
		{
			gUserServerChoice = USERSERVER_SHAKTI;
			snprintf(gUserServerName, MAX_STRING, "%s", gUserServerDomainName[gUserServerChoice].mName);		/* Flawfinder: ignore */
			gConnectToSomething = TRUE;
		}
		else if (!strcmp(argv[j], "--durga"))
		{
			gUserServerChoice = USERSERVER_DURGA;
			snprintf(gUserServerName, MAX_STRING, "%s", gUserServerDomainName[gUserServerChoice].mName);		/* Flawfinder: ignore */
			gConnectToSomething = TRUE;
		}
		else if (!strcmp(argv[j], "--soma"))
		{
			gUserServerChoice = USERSERVER_SOMA;
			snprintf(gUserServerName, MAX_STRING, "%s", gUserServerDomainName[gUserServerChoice].mName);		/* Flawfinder: ignore */
			gConnectToSomething = TRUE;
		}
		else if (!strcmp(argv[j], "--ganga"))
		{
			gUserServerChoice = USERSERVER_GANGA;
			sprintf(gUserServerName,"%s", gUserServerDomainName[gUserServerChoice].mName);
			gConnectToSomething = TRUE;
		}
		else if (!strcmp(argv[j], "--vaak"))
		{
			gUserServerChoice = USERSERVER_VAAK;
			sprintf(gUserServerName,"%s", gUserServerDomainName[gUserServerChoice].mName);
			gConnectToSomething = TRUE;
		}
		else if (!strcmp(argv[j], "--uma"))
		{
			gUserServerChoice = USERSERVER_UMA;
			sprintf(gUserServerName,"%s", gUserServerDomainName[gUserServerChoice].mName);
			gConnectToSomething = TRUE;
		}
		else if (!strcmp(argv[j], "-user") && (++j < argc)) 
		{
			if (!strcmp(argv[j], "-"))
			{
				gUserServerChoice = USERSERVER_LOCAL;
				snprintf(gUserServerName, MAX_STRING, "%s", LOOPBACK_ADDRESS_STRING);		/* Flawfinder: ignore */
			}
			else
			{
				gUserServerChoice = USERSERVER_OTHER;
				ip_string.assign( argv[j] );
				LLString::trim(ip_string);
				gUserServer.setHostByName( ip_string.c_str() );
				snprintf(gUserServerName, MAX_STRING, "%s", ip_string.c_str());		/* Flawfinder: ignore */
			}
			gConnectToSomething = TRUE;
		}
		else if (!strcmp(argv[j], "-space") && (++j < argc)) 
		{
			if (!strcmp(argv[j], "-"))
			{
				gUserServerChoice = USERSERVER_LOCAL;
			}
			else
			{
				gUserServerChoice = USERSERVER_OTHER;
				ip_string.assign( argv[j] );
				LLString::trim(ip_string);
				gUserServer.setAddress( ip_string.c_str() );
			}
			gConnectToSomething = TRUE;

			llinfos << "Argument -space is deprecated, use -user instead" << llendl;
		}
		else if (!strcmp(argv[j], "-loginuri") && (++j < argc))
		{
			gLoginURI = argv[j];
			gLoginURI = utf8str_trim(gLoginURI);
		}
		else if (!strcmp(argv[j], "-helperuri") && (++j < argc))
		{
			gHelperURI = argv[j];
			gHelperURI = utf8str_trim(gHelperURI);
		}
		else if (!strcmp(argv[j], "-debugviews"))
		{
			LLView::sDebugRects = TRUE;
		}
		else if (!strcmp(argv[j], "-skin") && (++j < argc))
		{
			std::string folder(argv[j]);
			gDirUtilp->setSkinFolder(folder);
		}
		else if (!strcmp(argv[j], "-autologin") || !strcmp(argv[j], "--autologin")) // keep --autologin for compatibility
		{
			gAutoLogin = TRUE;
		}
		else if (!strcmp(argv[j], "-quitafter") && (++j < argc))
		{
			gQuitAfterSeconds = (F32)atof(argv[j]);
		}
		else if (!strcmp(argv[j], "-rotate"))
		{
			gRotateRight = TRUE;
		}
//		else if (!strcmp(argv[j], "-noenv")) 
//		{
			//turn OFF environmental effects for slow machines/video cards
//			gRequestParaboloidMap = FALSE;
//		}
		else if (!strcmp(argv[j], "-noaudio"))
		{
			gUseAudio = FALSE;
		}
		else if (!strcmp(argv[j], "-nosound"))  // tends to be popular cmdline on Linux.
		{
			gUseAudio = FALSE;
		}
		else if (!strcmp(argv[j], "-nofmod"))
		{
			gUseFMOD = FALSE;
		}
		else if (!strcmp(argv[j], "-noprobe"))
		{
			gProbeHardware = FALSE;
		}
		else if (!strcmp(argv[j], "-noquicktime"))
		{
			// Developers can log in faster if they don't load all the
			// quicktime dlls.
			gUseQuickTime = false;
		}
		else if (!strcmp(argv[j], "-nopreload"))
		{
			// Developers can log in faster if they don't decode sounds
			// or images on startup, ~5 seconds faster.
			gPreloadSounds = false;
			gPreloadImages = false;
		}
		else if (!strcmp(argv[j], "-purge"))
		{
			purge_cache();
		}
		else if (!strcmp(argv[j], "-local"))
		{
			gConnectToSomething = FALSE;
			gUserServer.invalidate();
			gRunLocal = TRUE;
		}
		else if(!strcmp(argv[j], "-noinvlib"))
		{
			gRequestInventoryLibrary = FALSE;
		}
		else if (!strcmp(argv[j], "-log"))
		{
			gLogMessages = TRUE;
			continue;
		}
		else if (!strcmp(argv[j], "-logfile") && (++j < argc)) 
		{
			// *NOTE: This buffer size is hard coded into scanf() below.
			char logfile[256];	/* Flawfinder: ignore */
			sscanf(argv[j], "%255s", logfile);	/* Flawfinder: ignore */
			llinfos << "Setting log file to " << logfile << llendl;
			LLFile::remove(logfile);
			LLError::logToFile(logfile);
		}
		else if (!strcmp(argv[j], "-settings") && (++j < argc)) 
		{
			gSettingsFileName = gDirUtilp->getExpandedFilename(LL_PATH_USER_SETTINGS, argv[j]);
		}
		else if (!strcmp(argv[j], "-setdefault") && (j + 2 < argc)) 
		{
			std::string control_name;
			std::string control_value;
			
			j++;
			if (argv[j]) control_name = std::string(argv[j]);

			j++;
			if (argv[j]) control_value = std::string(argv[j]);
			
			// grab control name and value
			if (!control_name.empty())
			{
				gCommandLineSettings[control_name] = control_value;
			}
		}
		else if (!strcmp(argv[j], "-set") && (j + 2 < argc)) 
		{
			std::string control_name;
			std::string control_value;
			
			j++;
			if (argv[j]) control_name = std::string(argv[j]);

			j++;
			if (argv[j]) control_value = std::string(argv[j]);
			
			// grab control name and value
			if (!control_name.empty())
			{
				gCommandLineForcedSettings[control_name] = control_value;
			}
		}
		else if (!strcmp(argv[j], "-login"))
		{
			if (j + 3 < argc)
			{
				j++;
				gCmdLineFirstName = argv[j];
				j++;
				gCmdLineLastName = argv[j];
				j++;
				gCmdLinePassword = argv[j];
			}
			else
			{
				// only works if -login is last parameter on command line
				llerrs << "Not enough parameters to -login. Did you mean -loginuri?" << llendl;
			}
		}
		else if (!strcmp(argv[j], "-god"))
		{
			gGodConnect = TRUE;
		}
		else if (!strcmp(argv[j], "-noconsole"))
		{
			gUseConsole = FALSE;
		}
		else if (!strcmp(argv[j], "-safe"))
		{
			llinfos << "Setting viewer feature table to run in safe mode, resetting prefs" << llendl;
			gFeatureManagerp->setSafe(TRUE);
		}
		else if (!strcmp(argv[j], "-multiple"))
		{
			gMultipleViewersOK = TRUE;
		}
		else if (!strcmp(argv[j], "-nomultiple"))
		{
			gMultipleViewersOK = FALSE;
		}
		else if (!strcmp(argv[j], "-nothread"))
		{
			LLVFile::ALLOW_ASYNC = FALSE;
			llinfos << "Running VFS in nothread mode" << llendl;
		}
		// some programs don't respect the command line options in protocol handlers (I'm looking at you, Opera)
		// so this allows us to parse the URL straight off the command line without a "-url" paramater
		else if (!argument.compare(0, std::string( "secondlife://" ).length(), std::string("secondlife://")))
		{
			LLURLSimString::setString(argv[j]);
			gConnectToSomething = TRUE;
		}
		else if (!strcmp(argv[j], "-url") && (++j < argc)) 
		{
			LLURLSimString::setString(argv[j]);
			gConnectToSomething = TRUE;
		}
		else if (!strcmp(argv[j], "-ignorepixeldepth"))
		{
			gIgnorePixelDepth = TRUE;
		}
		else if (!strcmp(argv[j], "-cooperative"))
		{
			S32 ms_to_yield = 0;
			if(++j < argc)
			{
				S32 rv = sscanf(argv[j], "%d", &ms_to_yield);
				if(0 == rv)
				{
					--j;
				}
			}
			else
			{
				--j;
			}
			gYieldMS = ms_to_yield;
			gYieldTime = TRUE;
		}
		else if (!strncmp(argv[j], "-lang", 5))
		{
			j++;
			if (argv[j]) gLanguage = std::string(argv[j]);
		}
		else if (!strcmp(argv[j], "-no-verify-ssl-cert"))
		{
			gVerifySSLCert = false;
		}
		else if ( (!strcmp(argv[j], "--channel") || !strcmp(argv[j], "-channel"))  && (++j < argc)) 
		{
			gChannelName = argv[j];
		}
#if LL_DARWIN
		else if (!strncmp(argv[j], "-psn_", 5))
		{
			// this is the Finder passing the process session number
			// we ignore this
		}
#endif
		else
		{

			// DBC - Mac OS X passes some stuff by default on the command line (e.g. psn).
			// Second Life URLs are passed this way as well?
			llwarns << "Possible unknown keyword " << argv[j] << llendl;

			// print usage information
			llinfos << USAGE << llendl;
			// return 1;
		}
	}
	return 0;
}

//============================================================================

LLString LLURLSimString::sLocationStringHome("My Home");
LLString LLURLSimString::sLocationStringLast("My Last Location");

// "secondlife://simname/x/y/z" -> "simname/x/y/z"
// (actually .*//foo -> foo)
// static
void LLURLSimString::setString(const LLString& url)
{
	sInstance.mSimString.clear();
	sInstance.mSimName.clear();
	sInstance.mParseState = NOT_PARSED;
	if (url == sLocationStringHome)
	{
		gSavedSettings.setBOOL("LoginLastLocation", FALSE);
	}
	else if (url == sLocationStringLast)
	{
		gSavedSettings.setBOOL("LoginLastLocation", TRUE);
	}
	else
	{
		LLString tstring(url);
		std::string::size_type idx = tstring.find("//");
		idx = (idx == LLString::npos) ? 0 : idx+2;
		sInstance.mSimString = tstring.substr(idx);
	}
	LLPanelLogin::refreshLocation( false ); // in case LLPanelLogin is visible
}

// "/100" -> 100
// static
S32 LLURLSimString::parseGridIdx(const LLString& in_string, S32 idx0, S32* res, S32 max)
{
	if ((std::string::size_type)idx0 == LLString::npos || in_string[idx0] != '/')
	{
		return LLString::npos; // parse error
	}
	idx0++;
	std::string::size_type idx1 = in_string.find_first_of('/', idx0);
	S32 len = (idx1 == LLString::npos) ? LLString::npos : idx1-idx0;
	LLString tstring = in_string.substr(idx0,len);
	S32 val = atoi(tstring.c_str());
	*res = llclamp(val,0,max);
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
		std::string::size_type idx0=0,idx1=LLString::npos;
		sInstance.mSimName.clear();
		if (!sInstance.mSimString.empty())
		{
			idx0 = sInstance.mSimString.find_first_not_of('/'); // strip any bogus initial '/'
			if (idx0 == LLString::npos) idx0 = 0;
			idx1 = sInstance.mSimString.find_first_of('/', idx0);
			S32 len = (idx1 == LLString::npos) ? LLString::npos : idx1-idx0;
			LLString tstring = sInstance.mSimString.substr(idx0,len);
			char* curlstr = curl_unescape(tstring.c_str(), tstring.size());
			sInstance.mSimName = LLString(curlstr);
			curl_free(curlstr);
		}
		if (!sInstance.mSimName.empty())
		{
			if (idx1 != LLString::npos)
			{
				idx1 = parseGridIdx(sInstance.mSimString, idx1, &sInstance.mX, 255);
				idx1 = parseGridIdx(sInstance.mSimString, idx1, &sInstance.mY, 255);
				idx1 = parseGridIdx(sInstance.mSimString, idx1, &sInstance.mZ, 1000);
			}
			sInstance.mParseState = PARSE_OK;
		}
		else
		{
			sInstance.mParseState = PARSE_FAIL;
		}
	}
	return (sInstance.mParseState == PARSE_OK);
}

//static
bool LLURLSimString::unpack_data(void* data)
{
	llurl_data* url_data = (llurl_data*)data;
	
	sInstance.mSimName = url_data->mSimName;
	if (!sInstance.mSimName.empty())
	{
		sInstance.mX = llclamp(url_data->mSimX, 0, 255);
		sInstance.mY = llclamp(url_data->mSimY, 0, 255);
		sInstance.mZ = llclamp(url_data->mSimZ, 0, 1000);
		sInstance.mSimString = llformat("%s/%d/%d/%d",
										sInstance.mSimName.c_str(),
										sInstance.mX,sInstance.mY,sInstance.mZ);
		sInstance.mParseState = PARSE_OK;
	}
	else
	{
		sInstance.mSimString.clear();
		sInstance.mParseState = PARSE_FAIL;
	}
	return true;
}

//static
bool LLURLSimString::send_to_other_instance()
{
	if (!parse())
	{
		return false;
	}
#if LL_WINDOWS
	wchar_t window_class[256]; /* Flawfinder: ignore */   // Assume max length < 255 chars.
	mbstowcs(window_class, gWindowName.c_str(), 255);
	window_class[255] = 0;
	// Use the class instead of the window name.
	HWND other_window = FindWindow(window_class, NULL);
	if (other_window != NULL)
	{
		lldebugs << "Found other window with the name '" << gWindowTitle << "'" << llendl;
		llurl_data url_data;
		strncpy(url_data.mSimName, sInstance.mSimName.c_str(), DB_SIM_NAME_BUF_SIZE);		/* Flawfinder: ignore*/
		url_data.mSimName[DB_SIM_NAME_BUF_SIZE - 1] = '\0';
		url_data.mSimX = sInstance.mX;
		url_data.mSimY = sInstance.mY;
		url_data.mSimZ = sInstance.mZ;
		COPYDATASTRUCT url_CDS;
		url_CDS.dwData = 0;
		url_CDS.cbData = sizeof(llurl_data);
		url_CDS.lpData = &url_data;

		LRESULT msg_result = SendMessage(other_window, WM_COPYDATA, NULL, (LPARAM)&url_CDS);
		lldebugs << "SendMessage(WM_COPYDATA) to other window '" 
				 << gWindowTitle << "' returned " << msg_result << llendl;
		return true;
	}
#endif
	return false;
}


//============================================================================

void load_name_cache()
{
	if (!gCacheName) return;

	std::string name_cache;
	name_cache = gDirUtilp->getExpandedFilename(LL_PATH_CACHE, "name.cache");
	FILE* name_cache_fp = LLFile::fopen(name_cache.c_str(), "r");		/* Flawfinder: ignore*/
	if (name_cache_fp)
	{
		gCacheName->importFile(name_cache_fp);
		fclose(name_cache_fp);
	}
}

void save_name_cache()
{
	if (!gCacheName) return;

	std::string name_cache;
	name_cache = gDirUtilp->getExpandedFilename(LL_PATH_CACHE, "name.cache");
	FILE* name_cache_fp = LLFile::fopen(name_cache.c_str(), "w");		/* Flawfinder: ignore*/
	if (name_cache_fp)
	{
		gCacheName->exportFile(name_cache_fp);
		fclose(name_cache_fp);
	}
}


void login_alert_done(S32 option, void* user_data)
{
	LLPanelLogin::giveFocus();
}

void disconnect_viewer(void *)
{
	if (gDisconnected)
	{
		return;
	}
	//
	// Cleanup after quitting.
	//	
	// Save snapshot for next time, if we made it through initialization

	llinfos << "Disconnecting viewer!" << llendl;

	// Dump our frame statistics
	gFrameStats.dump();


	// Signal handlers may need resources that we're destroying here, so don't use them.
#if !LL_WINDOWS
	release_signals();
#endif

	// Remember if we were flying
	gSavedSettings.setBOOL("FlyingAtExit", gAgent.getFlying() );

	// Un-minimize all windows so they don't get saved minimized
	if (!gNoRender)
	{
		if (gFloaterView)
		{
			gFloaterView->restoreAll();
		}
	}

	if (gSelectMgr)
	{
		gSelectMgr->deselectAll();
	}

	if (!gNoRender)
	{
		// save inventory if appropriate
		gInventory.cache(gAgent.getInventoryRootID(), gAgent.getID());
		if(gInventoryLibraryRoot.notNull() && gInventoryLibraryOwner.notNull())
		{
			gInventory.cache(gInventoryLibraryRoot, gInventoryLibraryOwner);
		}
	}

	save_name_cache();

	// close inventory interface, close all windows
	LLInventoryView::cleanup();

	// Log out of userserver
	user_logout();

	// Delete stuff that would result in net connections

	// Also writes cached agent settings to gSavedSettings
	gAgent.cleanup();

	gObjectList.destroy();
	delete gWorldp;
	gWorldp = NULL;

	cleanup_xfer_manager();
	gUserServer.invalidate();
	gDisconnected = TRUE;
}

// helper function for cleanup_app
void remove_cache_files(const char* file_mask)
{
	char mask[LL_MAX_PATH];		/* Flawfinder: ignore */
	snprintf(mask, LL_MAX_PATH, "%s%s", gDirUtilp->getDirDelimiter().c_str(), file_mask);		/* Flawfinder: ignore */
	gDirUtilp->deleteFilesInDir(gDirUtilp->getExpandedFilename(LL_PATH_CACHE, "").c_str(), mask);
}




void finish_early_exit(S32 option, void* userdata)
{
	app_force_quit(NULL);
}

void app_early_exit(const LLString& mesg)
{
	llwarns << "app_early_exit: " << mesg << llendl;
	gDoDisconnect = TRUE;
// 	LLStringBase<char>::format_map_t args;
// 	args["[MESSAGE]"] = mesg;
// 	gViewerWindow->alertXml("AppEarlyExit", args, finish_early_exit);
	LLAlertDialog::showCritical(mesg, finish_early_exit, NULL);
}

// Used for drones, etc
void app_force_exit(S32 arg)
{
	remove_marker_file();
	exit(arg);
}

void finish_disconnect(S32 option, void* userdata)
{
	if (1 == option)
	{
		app_force_quit(NULL);
	}
}

void send_logout_request()
{
	if(!gLogoutRequestSent)
	{
		LLMessageSystem* msg = gMessageSystem;
		msg->newMessageFast(_PREHASH_LogoutRequest);
		msg->nextBlockFast(_PREHASH_AgentData);
		msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID() );
		msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
		gAgent.sendReliableMessage();

		gLogoutTimer.reset();
		gLogoutMaxTime = LOGOUT_REQUEST_TIME;
		gLogoutRequestSent = TRUE;
	}
}

void do_disconnect(const LLString& mesg)
{
	if (gDoDisconnect)
    {
		// Already popped up one of these dialogs, don't
		// do this again.
		return;
    }
	
	//RN: just quit if we haven't logged in
	if (gStartupState < STATE_STARTED)
	{
		finish_disconnect(1, NULL);
		return;
	}

	gDoDisconnect = TRUE;
	LLStringBase<char>::format_map_t args;
	args["[MESSAGE]"] = mesg;
	gViewerWindow->alertXml("YouHaveBeenLoggedOut", args, finish_disconnect);
}

const LLUUID& agent_get_id()
{
	return gAgent.getID();
}

const LLUUID& agent_get_session_id()
{
	return gAgent.getSessionID();
}

void agent_send_reliable_message()
{
	gAgent.sendReliableMessage();
}

void bad_network_handler()
{
	// Dump the packet
	gMessageSystem->dumpPacketToLog();

	// Flush all of our caches on exit in the case of disconnect due to
	// invalid packets.

	gPurgeOnExit = TRUE;

#if LL_WINDOWS
	LLWinDebug::handleException(NULL);
#else
	// Call the crash callback
	if (gCrashCallback)
	{
		gCrashCallback();
	}
#endif

	std::ostringstream message;
	message <<
		"The viewer has detected mangled network data indicative\n"
		"of a bad upstream network connection or an incomplete\n"
		"local installation of " << gSecondLife << ". \n"
		" \n"
		"Try uninstalling and reinstalling to see if this resolves \n"
		"the issue. \n"
		" \n"
		"If the problem continues, see the Tech Support FAQ at: \n"
		"www.secondlife.com/support";
	do_disconnect(message.str());
}


// dump current avatar to .cal file
void save_avatar(void*)
{
	LLFloaterSaveAvatar::show();
}

void cleanup_app()
{
	//flag all elements as needing to be destroyed immediately
	// to ensure shutdown order
	LLMortician::setZealous(TRUE);

	disconnect_viewer(NULL);

	llinfos << "Viewer disconnected" << llendflush;

	gDisconnectedImagep = NULL;
	release_start_screen(); // just in case

	LLError::logToFixedBuffer(NULL);

	llinfos << "Cleaning Up" << llendflush;

	LLKeyframeDataCache::clear();
	
	// Must clean up texture references before viewer window is destroyed.
	LLHUDObject::cleanupHUDObjects();
	llinfos << "HUD Objects cleaned up" << llendflush;

 	// End TransferManager before deleting systems it depends on (Audio, VFS, AssetStorage)
#if 0 // this seems to get us stuck in an infinite loop...
	gTransferManager.cleanup();
#endif
	
	// Clean up map data storage
	delete gWorldMap;
	gWorldMap = NULL;

	delete gHUDManager;
	gHUDManager = NULL;

	delete gToolMgr;
	gToolMgr = NULL;

	delete gAssetStorage;
	gAssetStorage = NULL;

	LLPolyMesh::freeAllMeshes();

	delete gCacheName;
	gCacheName = NULL;

	delete gGlobalEconomy;
	gGlobalEconomy = NULL;

	LLNotifyBox::cleanup();

	llinfos << "Global stuff deleted" << llendflush;

#if !LL_RELEASE_FOR_DOWNLOAD
	if (gAudiop)
	{
		gAudiop->shutdown();
	}
#else
	// This hack exists because fmod likes to occasionally hang forever
	// when shutting down for no apparent reason.
	llwarns << "Hack, skipping audio engine cleanup" << llendflush;
#endif


	// moved to main application shutdown for now because it's non-trivial and only needs to be done once
	// (even though it goes against the media framework design)

	LLMediaEngine::cleanupClass();
	
	#if LL_QUICKTIME_ENABLED
	if (gQuickTimeInitialized)
	{
		// clean up media stuff
		llinfos << "Cleaning up QuickTime" << llendl;
		ExitMovies ();
		#if LL_WINDOWS
			// Only necessary/available on Windows.
			TerminateQTML ();
		#endif
	}
	#endif

	llinfos << "Quicktime cleaned up" << llendflush;

	llinfos << "Cleaning up feature manager" << llendflush;
	delete gFeatureManagerp;
	gFeatureManagerp = NULL;

	// Patch up settings for next time
	// Must do this before we delete the viewer window,
	// such that we can suck rectangle information out of
	// it.
	cleanup_saved_settings();
	llinfos << "Settings patched up" << llendflush;

	delete gAudiop;
	gAudiop = NULL;

	// delete some of the files left around in the cache.
	remove_cache_files("*.wav");
	remove_cache_files("*.tmp");
	remove_cache_files("*.lso");
	remove_cache_files("*.out");
	remove_cache_files("*.dsf");
	remove_cache_files("*.bodypart");
	remove_cache_files("*.clothing");

	llinfos << "Cache files removed" << llendflush;


	cleanup_menus();

	// Wait for any pending VFS IO
	while (1)
	{
		S32 pending = LLVFSThread::updateClass(0);
		pending += LLLFSThread::updateClass(0);
		if (!pending)
		{
			break;
		}
		llinfos << "Waiting for pending IO to finish: " << pending << llendflush;
		ms_sleep(100);
	}
	llinfos << "Shutting down." << llendflush;
	
	// Destroy Windows(R) window, and make sure we're not fullscreen
	// This may generate window reshape and activation events.
	// Therefore must do this before destroying the message system.
	delete gViewerWindow;
	gViewerWindow = NULL;
	llinfos << "ViewerWindow deleted" << llendflush;

	// viewer UI relies on keyboard so keep it aound until viewer UI isa gone
	delete gKeyboard;
	gKeyboard = NULL;

	// Clean up selection managers after UI is destroyed, as UI
	// may be observing them.
	LLSelectMgr::cleanupGlobals();

	LLViewerObject::cleanupVOClasses();
		
	LLTracker::cleanupInstance();
	
#if LL_LIBXUL_ENABLED
	// this must be done after floater cleanup (delete gViewerWindow) since 
	// floaters  potentially need the manager to destroy their contents.
	LLMozLib::getInstance()->reset();
#endif

#if LL_WINDOWS
	gDXHardware.cleanup();
#endif // LL_WINDOWS

	if (!gVolumeMgr->cleanup())
	{
		llwarns << "Remaining references in the volume manager!" << llendflush;
	}

	LLViewerParcelMgr::cleanupGlobals();

	delete gViewerStats;
	gViewerStats = NULL;

 	end_messaging_system();

	LLFollowCamMgr::cleanupClass();
	LLVolumeMgr::cleanupClass();
	LLWorldMapView::cleanupClass();
	LLUI::cleanupClass();
	
	//
	// Shut down the VFS's AFTER the decode manager cleans up (since it cleans up vfiles).
	// Also after viewerwindow is deleted, since it may have image pointers (which have vfiles)
	// Also after shutting down the messaging system since it has VFS dependencies
	//
	LLVFile::cleanupClass();
	llinfos << "VFS cleaned up" << llendflush;

	// Store the time of our current logoff
    gSavedPerAccountSettings.setU32("LastLogoff", time_corrected());

	// Must do this after all panels have been deleted because panels that have persistent rects
	// save their rects on delete.
	gSavedSettings.saveToFile(gSettingsFileName, TRUE);
	if (!gPerAccountSettingsFileName.empty())
	{
		gSavedPerAccountSettings.saveToFile(gPerAccountSettingsFileName, TRUE);
	}
	llinfos << "Saved settings" << llendflush;

	std::string crash_settings_filename = gDirUtilp->getExpandedFilename(LL_PATH_USER_SETTINGS, CRASH_SETTINGS_FILE);
	// save all settings, even if equals defaults
	gCrashSettings.saveToFile(crash_settings_filename.c_str(), FALSE);

	delete gUICtrlFactory;
	gUICtrlFactory = NULL;

	gSavedSettings.cleanup();
	gViewerArt.cleanup();
	gColors.cleanup();
	gCrashSettings.cleanup();

	if (gMuteListp)
	{
		// save mute list
		gMuteListp->cache(gAgent.getID());

		delete gMuteListp;
		gMuteListp = NULL;
	}

	if (gPurgeOnExit)
	{
		llinfos << "Purging all cache files on exit" << llendflush;
		char mask[LL_MAX_PATH];		/* Flawfinder: ignore */
		snprintf(mask, LL_MAX_PATH, "%s*.*", gDirUtilp->getDirDelimiter().c_str());		/* Flawfinder: ignore */
		gDirUtilp->deleteFilesInDir(gDirUtilp->getExpandedFilename(LL_PATH_CACHE,"").c_str(),mask);
	}

	remove_marker_file(); // Any crashes from here on we'll just have to ignore
	
	close_debug();

	// Let threads finish
	LLTimer idleTimer;
	idleTimer.reset();
	const F64 max_idle_time = 5.f; // 5 seconds
	while(1)
	{
		S32 pending = 0;
		pending += gTextureCache->update(1); // unpauses the worker thread
		pending += gImageDecodeThread->update(1); // unpauses the image thread
		pending += gTextureFetch->update(1); // unpauses the texture fetch thread
		pending += LLVFSThread::updateClass(0);
		pending += LLLFSThread::updateClass(0);
		F64 idle_time = idleTimer.getElapsedTimeF64();
		if (!pending || idle_time >= max_idle_time)
		{
			llwarns << "Quitting with pending background tasks." << llendl;
			break;
		}
	}
	
	// Delete workers first
	// shotdown all worker threads before deleting them in case of co-dependencies
	gTextureCache->shutdown();
	gTextureFetch->shutdown();
	gImageDecodeThread->shutdown();
	delete gTextureCache;
	delete gTextureFetch;
	delete gImageDecodeThread;

	gImageList.shutdown(); // shutdown again in case a callback added something
	
	// This should eventually be done in LLAppViewer
	LLImageJ2C::closeDSO();
	LLImageFormatted::cleanupClass();
	LLVFSThread::cleanupClass();
	LLLFSThread::cleanupClass();

	llinfos << "VFS Thread finished" << llendflush;

#ifndef LL_RELEASE_FOR_DOWNLOAD
	llinfos << "Auditing VFS" << llendl;
	gVFS->audit();
#endif

	// For safety, the LLVFS has to be deleted *after* LLVFSThread. This should be cleaned up.
	// (LLVFS doesn't know about LLVFSThread so can't kill pending requests) -Steve
	delete gStaticVFS;
	gStaticVFS = NULL;
	delete gVFS;
	gVFS = NULL;
	
	LLCurl::cleanup();

	// This will eventually be done in LLApp
	LLCommon::cleanupClass();

	end_messaging_system();
}

const std::string& getLoginURI()
{
	if (gLoginURI.empty())
	{
		// not specified on the command line, use value from table
		gLoginURI = gUserServerDomainName[gUserServerChoice].mLoginURI;
	}
	return gLoginURI;
}

const std::string& getHelperURI()
{
	if (gHelperURI.empty())
	{
		// not specified on the command line, use value from table
		gHelperURI = gUserServerDomainName[gUserServerChoice].mHelperURI;
	}
	return gHelperURI;
}

void errorCallback(const std::string &error_string)
{
#ifndef LL_RELEASE_FOR_DOWNLOAD
	OSMessageBox(error_string.c_str(), "Fatal Error", OSMB_OK);
#endif
	LLError::crashAndLoop(error_string);
}
// JC - Please don't put code here.  Find the right file, perhaps
// llviewermessage.cpp, and put it there.  Thanks!
