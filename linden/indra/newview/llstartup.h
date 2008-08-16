/** 
 * @file llstartup.h
 * @brief startup routines and logic declaration
 *
 * Copyright (c) 2004-2007, Linden Research, Inc.
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

#ifndef LL_LLSTARTUP_H
#define LL_LLSTARTUP_H

#include "llimagegl.h"

// functions
BOOL idle_startup();
void cleanup_app();
LLString load_password_from_disk();
void release_start_screen();

// constants, variables,  & enumerations
extern const char* SCREEN_HOME_FILENAME;
extern const char* SCREEN_LAST_FILENAME;

enum EStartupState{
	STATE_FIRST,
	STATE_LOGIN_SHOW,
	STATE_LOGIN_WAIT,
	STATE_LOGIN_CLEANUP,
	STATE_RESOLVING_USERSERVER,
	STATE_USERSERVER_RESOLVED,
	STATE_MESSAGE_TEMPLATE_SEND,
	STATE_MESSAGE_TEMPLATE_WAIT,
	STATE_UPDATE_CHECK,
	STATE_LOGIN_AUTH_INIT,
	STATE_LOGIN_AUTHENTICATE,
	STATE_LOGIN_NO_DATA_YET,
	STATE_LOGIN_DOWNLOADING,
	STATE_LOGIN_PROCESS_RESPONSE,
	//STATE_USERSERVER_SEND,
	//STATE_USERSERVER_WAIT,
	//STATE_LOCATION_SEND,
	//STATE_LOCATION_WAIT,
	STATE_WORLD_INIT,
	STATE_QUICKTIME_INIT,
	STATE_WORLD_WAIT,
	STATE_AGENT_SEND,
	STATE_AGENT_WAIT,
	STATE_INVENTORY_SEND,
	STATE_CONNECT_USERSERVER,
	STATE_MISC,
	STATE_PRECACHE,
	STATE_WEARABLES_WAIT,
	//STATE_INVENTORY_WAIT,
	//STATE_TOS_AGREEMENT_START,
	//STATE_TOS_AGREEMENT_WAIT,
	//STATE_CRITICAL_MESSAGE_START,
	//STATE_CRITICAL_MESSAGE_WAIT,
	STATE_CLEANUP,
	STATE_STARTED
};

// exorted symbol
extern S32 gStartupState;
extern BOOL gAgentMovementCompleted;
extern bool gUseQuickTime;
extern bool gQuickTimeInitialized;
extern LLPointer<LLImageGL> gStartImageGL;

class LLStartUp
{
public:
	static bool canGoFullscreen();
		// returns true if we are far enough along in startup to allow
		// going full screen
};


#endif // LL_LLSTARTUP_H
