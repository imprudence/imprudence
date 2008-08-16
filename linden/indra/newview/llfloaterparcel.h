/** 
 * @file llfloaterparcelinfo.h
 * @brief LLFloaterParcelInfo class definition
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
 * Parcel information as shown in a floating window from a sl-url.
 * Just a wrapper for LLPanelPlace, shared with the Find directory.
 */

#ifndef LL_FLOATERPARCELINFO_H
#define LL_FLOATERPARCELINFO_H

#include "llfloater.h"

class LLPanelPlace;

class LLFloaterParcelInfo
:	public LLFloater
{
public:
	static	void*	createPanelPlace(void*	data);

	LLFloaterParcelInfo(const std::string& name, const LLUUID &parcel_id );
	/*virtual*/ ~LLFloaterParcelInfo();

	void displayParcelInfo(const LLUUID& parcel_id);

	static LLFloaterParcelInfo* show(const LLUUID& parcel_id);

private:
	LLUUID			mParcelID;			// for which parcel is this window?
	LLPanelPlace*	mPanelParcelp;
};


#endif
