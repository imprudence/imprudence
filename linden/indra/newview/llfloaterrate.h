/** 
 * @file llfloaterrate.h
 * @brief Dialog for rating avatars.  He's a ten!
 *
 * Copyright (c) 2002-2007, Linden Research, Inc.
 * 
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

#ifndef LL_LLFLOATERRATE_H
#define LL_LLFLOATERRATE_H

#ifndef LL_LLFLOATER_H
#include "llfloater.h"
#endif

#include "lluuid.h"
#include <map>

class LLTextBox;
class LLRadioGroup;
class LLButton;
class LLMessageSystem;
class LLLineEditor;
class LLObjectSelection;

class LLFloaterRate
: public LLFloater
{
public:
	virtual void draw();

	// Call this if you have the avatar id
	static void show(const LLUUID &avatar_id);

	// Types for show() below
	enum ERateSelection {
		RS_CREATOR,
		RS_OWNER
	};

	// Call this to rate the currently selected object, either
	// the creator or the owner.
	static void show(ERateSelection which);

	static void processReputationIndividualReply(LLMessageSystem *msg, void**);

protected:
	LLFloaterRate(const std::string& name, const LLUUID &id);
	virtual ~LLFloaterRate();

	void sendReputationIndividualRequest(const LLUUID &avatar_id);
	
	static void onClickOK(void *data);
	static void onClickCancel(void *data);

protected:
	typedef std::map<LLUUID, LLFloaterRate *> instance_map_t;
	static instance_map_t sInstanceMap;

protected:
	LLUUID			mAvatarID;
	F32				mLastBehavior;
	F32				mLastAppearance;
	F32				mLastBuilding;
	BOOL			mReputationRequested;
	LLHandle<LLObjectSelection> mObjectSelection;
};

#endif
