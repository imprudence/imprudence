/** 
 * @file viewer.h
 * @brief Main file for viewer.
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

#ifndef LL_VIEWER_H
#define LL_VIEWER_H

// DO NOT ADD DEPENDENCIES HERE
// Almost every file in the viewer depends on this file.
// If you need to add a shared class, put it in its own
// little file. JC
#include "lltransactiontypes.h"
#include "llviewercontrol.h"

// Enabled this definition to compile a 'hacked' viewer that
// allows a hacked godmode to be toggled on and off.
#define TOGGLE_HACKED_GODLIKE_VIEWER 
#ifdef TOGGLE_HACKED_GODLIKE_VIEWER
extern BOOL gHackGodmode;
#endif


//
// Forward Declarations
//
class LLAudioEngine;
class LLFrameTimer;
class LLMessageSystem;
class LLHost;
class LLPieMenu;
class LLWorkerThread;
class LLTextureFetch;
class LLTextureCache;
class LLPumpIO;
class LLHTTPNode;
class LLTimer;
class LLUUID;
class LLVector3;
class LLVector3d;
class LLViewerObject;
class LLViewerRegion;
class LLVFS;

//
// Global Variables
//

extern LLString gSecondLife;

extern S32 gNumSessions;
extern BOOL gNoRender;
extern LLMemoryInfo gSysMemory;
extern BOOL gLogMessages;
extern BOOL gUseAudio;
extern bool gPreloadImages;
extern BOOL gConnectToSomething;
extern BOOL gGodConnect;
extern BOOL gRunLocal;
extern LLString gCmdLineFirstName;
extern LLString gCmdLineLastName;
extern LLString gCmdLinePassword;
extern BOOL gAutoLogin;
extern U32 gFrameCount;
extern BOOL gRequestInventoryLibrary;
extern BOOL gAcceptTOS;
extern BOOL gAcceptCriticalMessage;
extern std::string gChannelName;
extern LLUUID gInventoryLibraryOwner;
extern LLUUID gInventoryLibraryRoot;
extern BOOL gLastExecFroze;
extern F32 gLogoutMaxTime;
extern LLTimer gRenderStartTime;
extern LLTimer gLogoutTimer;
extern BOOL gInProductionGrid;
extern S32 gCrashBehavior;
extern bool gVerifySSLCert;
extern const char* DEFAULT_SETTINGS_FILE;

// TODO: Eliminate most of these, move into a globals structure.
extern const U32 PATCH_SIZE;
extern const LLVector3 DEFAULT_OBJECT_SCALE;
extern BOOL		gDisconnected;
extern BOOL		gQuit;						// We're done, quit after processing this message.
extern BOOL		gQuitRequested;				// User wants to quit, may have modified documents open.
extern BOOL		gLogoutRequestSent;			// Disconnect message sent to simulator, no longer safe to send messages to the sim.
extern BOOL		gInitializationComplete;	// OK to call display() and idle()
extern BOOL		gUseWireframe;
extern BOOL		gPurgeCache;				// Cache was purged on startup
extern U32		gSecondsPerDay;
extern U32		gSecondsPerYear;
// Is the Pacific time zone (aka server time zone)
// currently in daylight savings time?
extern BOOL gPacificDaylightTime;

extern LLVector3 gWindVec;
extern LLVector3 gRelativeWindVec;

extern U64      gFrameTime;					// The timestamp of the most-recently-processed frame
extern F32		gFrameTimeSeconds;			// Loses msec precision after ~4.5 hours...
extern F32		gFrameIntervalSeconds;		// Elapsed time between current and previous gFrameTimeSeconds
extern F32		gFPSClamped;						// Frames per second, smoothed, weighted toward last frame
extern F32		gFrameDTClamped;
extern U64		gStartTime;

extern LLFrameTimer	gRestoreGLTimer;
extern BOOL			gRestoreGL;

extern BOOL			gDisplayCameraPos;
extern BOOL			gDisplayWindInfo;
extern BOOL			gDisplayNearestWater;
extern BOOL			gDisplayFOV;
extern BOOL			gForceRenderLandFence;

extern BOOL			gAllowAFK;
extern BOOL			gShowObjectUpdates;

extern BOOL			gTeleportDisplay;
extern LLFrameTimer	gTeleportDisplayTimer;

extern std::map<S32,LLFrameTimer> gDebugTimers;

// VFS globals - gVFS is for general use
// gStaticVFS is read-only and is shipped w/ the viewer
// it has pre-cache data like the UI .TGAs
extern LLVFS	*gStaticVFS;

extern LLUUID	gViewerDigest;  // MD5 digest of the viewer's executable file.

extern LLWorkerThread* gImageDecodeThread;
extern LLTextureFetch* gTextureFetch;
extern LLTextureCache* gTextureCache;

extern LLPumpIO* gServicePump;

class LLURLSimString
{
public:
	enum { NOT_SET=0, NOT_PARSED=1, PARSE_OK=2, PARSE_FAIL=-1 };
	static void setString(const LLString& url);
	static bool parse();
	static bool unpack_data(void* data);
	static bool send_to_other_instance();
	
	LLURLSimString() : mX(128), mY(128), mZ(0), mParseState(NOT_PARSED) {}

private:
	static S32 parseGridIdx(const LLString& in_string, S32 idx0, S32* res, S32 max);
	struct llurl_data
	{
		char	mSimName[DB_SIM_NAME_BUF_SIZE];		/*Flawfinder: ignore*/
		S32		mSimX;
		S32		mSimY;
		S32		mSimZ;
	};

	
public:
	static LLURLSimString sInstance;
	static LLString sLocationStringHome;
	static LLString sLocationStringLast;

public:
	LLString mSimString; // "name/x/y/z"
	LLString mSimName;
	S32 mX,mY,mZ;
	S32 mParseState;
};

extern LLUUID gSunTextureID;
extern LLUUID gMoonTextureID;

extern BOOL gHandleKeysAsync;

// Map scale in pixels per region
extern F32 gMapScale;
extern F32 gMiniMapScale;

extern F32 gMouseSensitivity;
extern BOOL gInvertMouse;

//
// Defines and constants
//

const S32 AGENT_UPDATES_PER_SECOND  = 10;

// Global because multiple functions need to identify the window
// by its name.
extern LLString gWindowTitle;
extern LLString gWindowName;
extern LLString gDisabledMessage;
extern BOOL gHideLinks;

#define REGIONS_PER_LANDSCAPE_EDGE  3 
#define NUMBER_OF_REGIONS	(REGIONS_PER_LANDSCAPE_EDGE 		\
							* REGIONS_PER_LANDSCAPE_EDGE)
#define CENTER_REGION		((REGIONS_PER_LANDSCAPE_EDGE / 2)	\
							* REGIONS_PER_LANDSCAPE_EDGE		\
							+ REGIONS_PER_LANDSCAPE_EDGE / 2)


// translates constant EAST, NORTH, WEST, etc. to x,y offsets
// eg, EAST = +1, 0
// eg, SOUTHWEST = -1, -1
const S32 MAP_REGION_TO_X[NUMBER_OF_REGIONS - 1] =
{
	1,	// east
	0,	// north
	-1,	// west
	0,	// south
	1,	// northeast
	-1,	// northwest
	-1,	// southwest
	1	// southeast
};
const S32 MAP_REGION_TO_Y[NUMBER_OF_REGIONS - 1] =
{
	0,	// east
	1,	// north
	0,	// west
	-1,	// south
	1,	// northeast
	1,	// northwest
	-1,	// southwest
	-1	// southeast
};

const BOOL FOR_SELECTION = TRUE;
const BOOL NOT_FOR_SELECTION = FALSE;

const U8 YES_PHYSICS = 1;
const U8 NO_PHYSICS = 0;

//
// Prototypes
//
void display(BOOL rebuild = TRUE, F32 zoom_factor = 1.f, int subfield = 0);
void draw_axes();

void fire_shot(const U8 shot_type, const U8 use_physics );
void add_object(LLPCode pcode, 
				U8 use_physics, 
				const LLVector3d &object_pos_global, 
				const LLQuaternion &rotation, 
				const LLVector3 &scale, 
				const LLVector3 &velocity,
				const LLVector3 &angular_velocity);

BOOL add_object(LLPCode pcode, S32 screen_x, S32 screen_y, U8 use_physics);

void idle();
void idle_network();
void init_audio();
void init_textures();
void init_landscape();
void keyboard(char key);
void keyboard_windows(char key);
void kill_all_trees();
void move_sun();
BOOL raycast_for_new_obj_pos( S32 x, S32 y, LLViewerObject** hit_obj, S32* hit_face, BOOL* b_hit_land, LLVector3* ray_start_region, LLVector3* ray_end_region, LLViewerRegion** region );

// Saves the final snapshot, but only once.
void save_final_snapshot(void*);

// Call when user is requesting application to quit.
void app_request_quit();

// Call when user aborts the quit, for example, canceling a required save of 
// dirty document.
void app_abort_quit();

// Call if you need to quit immediately.  You can optionally
// spawn a web page or open another application.
void app_force_quit(const char* launch_file_on_quit = NULL);

// Display an error dialog and forcibly quit.
void app_early_exit(const LLString& message);

// exit() immediately (after some cleanup)
void app_force_exit(S32 arg);

void do_disconnect(const LLString& message);
void send_logout_request();
void save_avatar(void*);

void set_view(S32 which);

void user_logout();

F32 mouse_x_from_center(S32 x);
F32 mouse_y_from_center(S32 x);

void print_agent_nvpairs(void*);
void reload_viewer(void);
void reset_statistics();
void output_statistics(void*);
void write_debug(const char *str);
void write_debug(const std::string& str);
void login_alert_done(S32 option, void* user_data);
void callback_cache_name(const LLUUID& id, const char* firstname, const char* lastname, BOOL is_group, void* data);
void load_name_cache();
void remove_marker_file();

// Avoid including llagent.h everywhere.  This returns gAgent.getID().
const LLUUID& agent_get_id();
const LLUUID& agent_get_session_id();
void agent_send_reliable_message();

// Helpers for URIs
const std::string& getLoginURI();
const std::string& getHelperURI();

#endif
