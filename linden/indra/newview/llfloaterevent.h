/** 
 * @file llfloaterevent.h
 * @brief LLFloaterEvent class implementation
 *
 * $LicenseInfo:firstyear=2002&license=internal$
 * 
 * Copyright (c) 2002-2008, Linden Research, Inc.
 * 
 * The following source code is PROPRIETARY AND CONFIDENTIAL. Use of
 * this source code is governed by the Linden Lab Source Code Disclosure
 * Agreement ("Agreement") previously entered between you and Linden
 * Lab. By accessing, using, copying, modifying or distributing this
 * software, you acknowledge that you have been informed of your
 * obligations under the Agreement and agree to abide by those obligations.
 * 
 * ALL LINDEN LAB SOURCE CODE IS PROVIDED "AS IS." LINDEN LAB MAKES NO
 * WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
 * COMPLETENESS OR PERFORMANCE.
 * $/LicenseInfo$
 */

/**
* Event information as shown in a floating window from a secondlife:// url.
* Just a wrapper for LLPanelEvent.
*/

#ifndef LL_LLFLOATEREVENT_H
#define LL_LLFLOATEREVENT_H

#include "llfloater.h"

class LLPanelEvent;

class LLFloaterEventInfo : public LLFloater
{
public:
	LLFloaterEventInfo(const std::string& name, const U32 event_id );
	/*virtual*/ ~LLFloaterEventInfo();

	void displayEventInfo(const U32 event_id);

	static LLFloaterEventInfo* show(const U32 event_id);
	
	static void* createEventDetail(void* userdata);

private:
	U32				mEventID;			// for which event is this window?
	LLPanelEvent*	mPanelEventp;

};

#endif // LL_LLFLOATEREVENT_H
