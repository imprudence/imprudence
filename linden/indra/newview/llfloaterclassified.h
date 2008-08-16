/** 
 * @file llfloaterclassified.h
 * @brief LLFloaterClassifiedInfo class declaration
 *
 * $LicenseInfo:firstyear=2002&license=internal$
 * 
 * Copyright (c) 2002-2007, Linden Research, Inc.
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
* Classified information as shown in a floating window from a secondlife:// url.
* Just a wrapper for LLPanelClassified.
*/


#ifndef LL_FLOATERCLASSIFIED_H
#define LL_FLOATERCLASSIFIED_H

#include "llfloater.h"

class LLPanelClassified;

class LLFloaterClassifiedInfo : LLFloater
{
public:
	LLFloaterClassifiedInfo(const std::string& name, const LLUUID &id );
	virtual ~LLFloaterClassifiedInfo();

	void displayClassifiedInfo(const LLUUID& classified_id);

	static LLFloaterClassifiedInfo* show(const LLUUID& classified_id);

	static void* createClassifiedDetail(void* userdata);

private:

	LLPanelClassified*	mClassifiedPanel;
	LLUUID				mClassifiedID;

};

#endif // LL_FLOATERCLASSIFIED_H
