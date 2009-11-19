/** 
 * @file llfloaterpermissionsmgr.h
 * @brief for user control of script permissions
 *
 * $LicenseInfo:firstyear=2003&license=viewergpl$
 * 
 * Copyright (c) 2003-2009, Linden Research, Inc.
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

#ifndef LL_LLFLOATERPERMISSIONSMGR_H
#define LL_LLFLOATERPERMISSIONSMGR_H

#include "llfloater.h"
#include <vector>

class LLScrollableContainerView;
class LLPermissionsView;

class LLFloaterPermissionsMgr
:	public LLFloater
{
public:
	static LLFloaterPermissionsMgr* show();

	// Message system callbacks
	static void processPermissionsList(LLMessageSystem* msg, void**);

	virtual void	onClose(bool app_quitting)		{ setVisible(FALSE); }

private:
	// Must construct by calling show().
	LLFloaterPermissionsMgr();
	virtual ~LLFloaterPermissionsMgr();

public:
	LLPermissionsView*				mPermissions;

protected:
	LLScrollableContainerView*		mScroller;

	static LLFloaterPermissionsMgr* sInstance;
};

class LLPermissionsData
{
public:
	LLPermissionsData(const LLUUID& object_id, U32 permission_flags) : mObjectID(object_id), mPermFlags(permission_flags) {};
	
	LLUUID	mObjectID;
	U32		mPermFlags;
};

class LLPermissionsView : public LLView
{
public:
	LLPermissionsView(const LLRect& rect);
	virtual ~LLPermissionsView() {};

public:
	void clearPermissionsData();
	void addPermissionsData(const std::string& object_name, const LLUUID& object_id, U32 permissions_flags);

	static void revokePermissions(void *userdata);
	static void findObject(void *userdata);

protected:
	std::map<LLUUID, LLPermissionsData*> mPermData;
};


#endif
