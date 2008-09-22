/** 
 * @file llfloaterbulkpermissions.h
 * @brief A floater which allows task inventory item's properties to be changed on mass.
 *
 * $LicenseInfo:firstyear=2008&license=viewergpl$
 * 
 * Copyright (c) 2008, Linden Research, Inc.
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
 * online at http://secondlifegrid.net/programs/open_source/licensing/flossexception
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

/* Allow multiple task inventory properties to be set in one go, by Michelle2 Zenovka */

#ifndef LL_LLBULKPERMISSION_H
#define LL_LLBULKPERMISSION_H

#include "lldarray.h"
#include "llinventory.h"
#include "llviewerobject.h"
#include "llvoinventorylistener.h"
#include "llmap.h"
#include "lluuid.h"

#include "llfloater.h"
#include "llscrolllistctrl.h"

#include "llviewerinventory.h"

class LLFloaterBulkPermission : public LLFloater, public LLVOInventoryListener
{
public:
	// addObject() accepts an object id.
	void addObject(const LLUUID& id);

	// start() returns TRUE if the queue has started, otherwise FALSE.
	BOOL start();

	// Use this method to create a reset queue. Once created, it
	// will be responsible for it's own destruction.
	static LLFloaterBulkPermission * create();

protected:
	LLFloaterBulkPermission(const std::string& name, const LLRect& rect,
						 const char* title, const char* start_string);
	virtual ~LLFloaterBulkPermission();

	// This is the callback method for the viewer object currently
	// being worked on.
	/*virtual*/ void inventoryChanged(LLViewerObject* obj,
								 InventoryObjectList* inv,
								 S32 serial_num,
								 void* queue);
	
	// This is called by inventoryChanged
	void handleInventory(LLViewerObject* viewer_obj,
								InventoryObjectList* inv);


	void updateInventory(LLViewerObject* object,
								LLViewerInventoryItem* item,
								U8 key,
								bool is_new);


	static void onCloseBtn(void* user_data);
	static void onApplyBtn(void* user_data);
	static void onCommitPermissions(LLUICtrl* ctrl, void* data);
	static void InvSelection(LLUICtrl* ctrl, void* data);
	static void onRecurse(LLUICtrl* ctrl, void* data);	
	static void onParent(LLUICtrl* ctrl, void* data);

	// returns true if this is done
	BOOL isDone() const;

	//Read the settings and Apply the permissions
	void doApply();

	// go to the next object. If no objects left, it falls out
	// silently and waits to be killed by the deleteIfDone() callback.
	BOOL nextObject();
	BOOL popNext();

	// Get this instances ID.
	const LLUUID& getID() const { return mID; } 

	// find an instance by ID. Return NULL if it does not exist.
	static LLFloaterBulkPermission* findInstance(const LLUUID& id);

	U32 req_perm_mask;

	BOOL processObject;
	BOOL processScript;
	BOOL processTexture;
	BOOL processSound;
	BOOL processAnimation;
	BOOL processCallingcard;
	BOOL processNotecard;
	BOOL processGesture;
	BOOL processClothing;
	BOOL processBodypart;
	BOOL processLandmark;

	BOOL recurse;
	BOOL parent;

protected:
	// UI
	LLScrollListCtrl* mMessages;
	LLButton* mCloseBtn;

	// Object Queue
	LLDynamicArray<LLUUID> mObjectIDs;
	LLUUID mCurrentObjectID;
	BOOL mDone;

	LLUUID mID;
	static LLMap<LLUUID, LLFloaterBulkPermission*> sInstances;

	const char* mStartString;

};

#endif
