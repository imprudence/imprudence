/** 
 * @file llfloateravatarinfo.cpp
 * @brief LLFloaterAvatarInfo class implementation
 *
 * Copyright (c) 2002-2007, Linden Research, Inc.
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

/**
 * Avatar information as shown in a floating window from right-click
 * Profile.  Used for editing your own avatar info.  Just a wrapper
 * for LLPanelAvatar, shared with the Find directory.
 */

// system includes

// linden prefix includes
#include "llviewerprecompiledheaders.h"

// self include
#include "llfloateravatarinfo.h"

// linden library includes
#include "llcachename.h"
#include "llfontgl.h"
#include "llinventory.h"
#include "message.h"

// viewer project includes
#include "llagentdata.h"
#include "llbutton.h"
#include "llcallingcard.h"
#include "llcheckboxctrl.h"
#include "llfloaterworldmap.h"
#include "llfloatermute.h"
#include "llinventoryview.h"
#include "lllineeditor.h"
#include "llmutelist.h"
#include "llscrolllistctrl.h"
#include "lltabcontainer.h"
#include "llimview.h"
#include "lluiconstants.h"
#include "llviewerobject.h"
#include "llviewerobjectlist.h"
#include "llviewerregion.h"
#include "llviewborder.h"
#include "llinventorymodel.h"
#include "lltextbox.h"
#include "lltexturectrl.h"
#include "llviewertexteditor.h"
#include "llpanelavatar.h"

#include "llvieweruictrlfactory.h"

const char FLOATER_TITLE[] = "Profile";
const LLRect FAI_RECT(0, 530, 420, 0);

const S32 RULER0 = 90;
const S32 RULER1 = RULER0 + 5;
const S32 RULER2 = RULER1 + 75;
const S32 RULER3 = RULER2 + 90;
const S32 RULER4 = RULER3 + 10;

//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------

LLMap< const LLUUID, LLFloaterAvatarInfo* > gAvatarInfoInstances;


//-----------------------------------------------------------------------------
// Member functions
//-----------------------------------------------------------------------------

//----------------------------------------------------------------------------

void*	LLFloaterAvatarInfo::createPanelAvatar(void*	data)
{
	LLFloaterAvatarInfo* self = (LLFloaterAvatarInfo*)data;
	self->mPanelAvatarp = new LLPanelAvatar("PanelAv", LLRect(), TRUE); // allow edit self
	return self->mPanelAvatarp;
}

//----------------------------------------------------------------------------


BOOL	LLFloaterAvatarInfo::postBuild()
{
	return TRUE;
}

LLFloaterAvatarInfo::LLFloaterAvatarInfo(const std::string& name, const LLRect &rect, const LLUUID &avatar_id)
:	LLPreview(name, rect, FLOATER_TITLE, LLUUID::null, LLUUID::null),
	mAvatarID( avatar_id ),
	mSuggestedOnlineStatus(ONLINE_STATUS_NO)
{
	mAutoFocus = TRUE;

	LLCallbackMap::map_t factory_map;

	factory_map["Panel Avatar"] = LLCallbackMap(createPanelAvatar, this);
	
	gUICtrlFactory->buildFloater(this, "floater_profile.xml", &factory_map);


	if(mPanelAvatarp)
	{
		mPanelAvatarp->selectTab(0);
	}

	gAvatarInfoInstances.addData(avatar_id, this);

	
}

// virtual
LLFloaterAvatarInfo::~LLFloaterAvatarInfo()
{
	// child views automatically deleted
	gAvatarInfoInstances.removeData(mAvatarID);

}

void LLFloaterAvatarInfo::resetGroupList()
{
	// only get these updates asynchronously via the group floater, which works on the agent only
	if (mAvatarID != gAgentID)
	{
		return;
	}

	mPanelAvatarp->resetGroupList();
}


// Open profile to a certian tab.
// static
void LLFloaterAvatarInfo::showFromObject(
	const LLUUID& avatar_id,
	std::string tab_name)
{
	if(avatar_id.isNull())
	{
		return;
	}

	LLFloaterAvatarInfo *floater = NULL;
	if (gAvatarInfoInstances.checkData(avatar_id))
	{
		// ...bring that window to front
		floater = gAvatarInfoInstances.getData(avatar_id);
	}
	else
	{
		floater =  new LLFloaterAvatarInfo("avatarinfo", FAI_RECT, 
			avatar_id);
		floater->center();
		floater->mPanelAvatarp->setAvatarID(avatar_id, "", ONLINE_STATUS_NO);
		
	}
	floater->mPanelAvatarp->selectTabByName(tab_name);
	floater->open(); /*Flawfinder: ignore*/
}

// static
void LLFloaterAvatarInfo::showFromDirectory(const LLUUID &avatar_id)
{
	if(avatar_id.isNull())
	{
		return;
	}

	LLFloaterAvatarInfo *floater;
	if (gAvatarInfoInstances.checkData(avatar_id))
	{
		// ...bring that window to front
		floater = gAvatarInfoInstances.getData(avatar_id);
	}
	else
	{
		floater =  new LLFloaterAvatarInfo("avatarinfo", FAI_RECT, 
			avatar_id);
		floater->center();
		floater->mPanelAvatarp->setAvatarID(avatar_id, "", ONLINE_STATUS_NO);
	}
	if(floater)
	{
		floater->open();
	}
}


// static
void LLFloaterAvatarInfo::showFromAvatar(LLViewerObject *avatar)
{
	if (gAvatarInfoInstances.checkData(avatar->getID()))
	{
		// ...bring that window to front
		LLFloaterAvatarInfo *f = gAvatarInfoInstances.getData(avatar->getID());
		f->open();	/*Flawfinder: ignore*/
	}
	else
	{
		LLFloaterAvatarInfo *floater =  new LLFloaterAvatarInfo("avatarinfo", FAI_RECT, 
			avatar->getID() );
		floater->center();
		floater->open();	/*Flawfinder: ignore*/
	}
}


// static
void LLFloaterAvatarInfo::showFromFriend(const LLUUID& agent_id, BOOL online)
{
	if(agent_id.isNull())
	{
		return;
	}
	if (gAvatarInfoInstances.checkData( agent_id ))
	{
		// ...bring that window to front
		LLFloaterAvatarInfo *f = gAvatarInfoInstances.getData( agent_id );
		f->open();	/*Flawfinder: ignore*/
	}
	else
	{
		LLFloaterAvatarInfo *floater =  new LLFloaterAvatarInfo("avatarinfo", FAI_RECT, 
			agent_id);
		floater->center();
		floater->mSuggestedOnlineStatus = online ? ONLINE_STATUS_YES : ONLINE_STATUS_NO;
	}
}


void LLFloaterAvatarInfo::showProfileCallback(S32 option, void *userdata)
{
	if (option == 0)
	{
		showFromObject(gAgentID);
	}
}


//// virtual
void LLFloaterAvatarInfo::draw()
{
	LLFloater::draw();
}

// virtual
BOOL LLFloaterAvatarInfo::canClose()
{
	return mPanelAvatarp && mPanelAvatarp->canClose();
}

LLFloaterAvatarInfo* LLFloaterAvatarInfo::getInstance(const LLUUID &id)
{
	return gAvatarInfoInstances.getIfThere(gAgentID);
}

void LLFloaterAvatarInfo::loadAsset()
{
	if (mPanelAvatarp) {
		mPanelAvatarp->setAvatarID(mAvatarID, "", mSuggestedOnlineStatus);
		mAssetStatus = PREVIEW_ASSET_LOADING;
	}
}

LLPreview::EAssetStatus LLFloaterAvatarInfo::getAssetStatus()
{
	if (mPanelAvatarp && mPanelAvatarp->haveData())
	{
		mAssetStatus = PREVIEW_ASSET_LOADED;
	}
	return mAssetStatus;
}
