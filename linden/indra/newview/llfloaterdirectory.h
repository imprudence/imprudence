/** 
 * @file llfloaterdirectory.h
 * @brief The "Find" floater.  Should be llfloaterfind.
 *
 * $LicenseInfo:firstyear=2002&license=viewergpl$
 * 
 * Copyright (c) 2002-2009, Linden Research, Inc.
 * 
 * Second Life Viewer Source Code
 * The source code in this file ("Source Code") is provided by Linden Lab
 * to you under the terms of the GNU General Public License, version 2.0
 * ("GPL"), unless you have obtained a separate licensing agreement
 * ("Other License"), formally executed by you and Linden Lab.  Terms of
 * the GPL can be found in doc/GPL-license.txt in this distribution, or
 * online at http://secondlifegrid.net/programs/open_source/licensing/gplv2
 * 
 * There are special exceptions to the terms and conditions of the GPL as
 * it is applied to this Source Code. View the full text of the exception
 * in the file doc/FLOSS-exception.txt in this software distribution, or
 * online at
 * http://secondlifegrid.net/programs/open_source/licensing/flossexception
 * 
 * By copying, modifying or distributing this software, you acknowledge
 * that you have read and understood your obligations described above,
 * and agree to abide by those obligations.
 * 
 * ALL LINDEN LAB SOURCE CODE IS PROVIDED "AS IS." LINDEN LAB MAKES NO
 * WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
 * COMPLETENESS OR PERFORMANCE.
 * $/LicenseInfo$
 */

#ifndef LL_LLFLOATERDIRECTORY_H
#define LL_LLFLOATERDIRECTORY_H

#include "llfloater.h"
#include "lltabcontainer.h"

class LLDirectoryCore;
class LLPanelDirBrowser;

class LLPanelDirAdvanced;
class LLPanelDirClassified;
class LLPanelDirEvents;
class LLPanelDirFindAll;
class LLPanelDirFindAllOld;
class LLPanelDirGroups;
class LLPanelDirLand;
class LLPanelDirPeople;
class LLPanelDirPlaces;
class LLPanelDirPopular;

class LLPanelAvatar;
class LLPanelEvent;
class LLPanelGroup;
class LLPanelPlace;
class LLPanelClassified;

// Floater to find people, places, things
class LLFloaterDirectory : public LLFloater
{
public: 
	LLFloaterDirectory(const std::string& name);
	/*virtual*/ ~LLFloaterDirectory();

	/*virtual*/ void setVisible(BOOL visible);

	// Used for toggling God mode, which changes to visibility of 
	// some picks.
	static void requestClassifieds();

	// Outside UI widgets can spawn this floater with various tabs
	// selected.
	static void showFindAll(const std::string& search_text);
	static void showClassified(const LLUUID& classified_id);
	static void showEvents(S32 event_id);
	static void showLandForSale(const LLUUID& parcel_id);
	static void showGroups();
	static void refreshGroup(const LLUUID& group_id);
	static void hide(void*);

	static void toggleEvents(void*);
	static void toggleFind(void*);
	static void onTabChanged(void*, bool);

	void hideAllDetailPanels();

private:
	static void showPanel(const std::string& tabname);
	/*virtual*/ void onClose(bool app_quitting);
	void focusCurrentPanel();

private:
	// Some special "showByID" functions use these cached pointers.
	// They could be replaced by getPanelByName(), perhaps. JC
	LLPanelDirFindAll* mFindAllPanel;
	LLPanelDirClassified* mClassifiedPanel;
	LLPanelDirEvents* mEventsPanel;
	LLPanelDirLand* mLandPanel;

	static void* createFindAll(void* userdata);
	static void* createClassified(void* userdata);
	static void* createEvents(void* userdata);
	static void* createShowcase(void* userdata);
	static void* createPlaces(void* userdata);
	static void* createLand(void* userdata);
	static void* createPeople(void* userdata);
	static void* createGroups(void* userdata);
	static void* createFindAllOld(void* userdata);
 
	static void* createClassifiedDetail(void* userdata);
	static void* createAvatarDetail(void* userdata);
	static void* createEventDetail(void* userdata);
	static void* createGroupDetail(void* userdata);
	static void* createGroupDetailHolder(void* userdata);
	static void* createPlaceDetail(void* userdata);
	static void* createPlaceDetailSmall(void* userdata);

	static	void*	createPanelAvatar(void*	data);
	
public:
	LLPanelAvatar*		mPanelAvatarp;
	LLPanelEvent*		mPanelEventp;
	LLPanelGroup*		mPanelGroupp;
	LLPanel*			mPanelGroupHolderp;
	LLPanelPlace*		mPanelPlacep;
	LLPanelPlace*		mPanelPlaceSmallp;
	LLPanelClassified*	mPanelClassifiedp;

	static S32 sOldSearchCount; // debug
	static S32 sNewSearchCount; // debug

private:
	bool mMinimizing;	// HACK: see reshape() for details
	static LLFloaterDirectory	*sInstance;
};

extern BOOL gDisplayEventHack;

#endif  // LL_LLDIRECTORYFLOATER_H
