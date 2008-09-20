/** 
 * @file llfloaterbulkpermissions.cpp
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

/* TODO
	
	* Add in the option to select objects or task inventory
s

It would be nice to set the permissions on groups of prims as well as task inventory

*/


#include "llviewerprecompiledheaders.h"
#include "llfloaterbulkpermission.h"
#include "llagent.h"
#include "llchat.h"
#include "llviewerwindow.h"
#include "llviewerobject.h"
#include "llviewerobjectlist.h"
#include "llviewerregion.h"
#include "lscript_rt_interface.h"
#include "llviewercontrol.h"
#include "llviewerobject.h"
#include "llviewerregion.h"
#include "llresmgr.h"
#include "llbutton.h"
#include "lldir.h"
#include "llfloaterchat.h"
#include "llviewerstats.h"
#include "lluictrlfactory.h"
#include "llselectmgr.h"
#include "llinventory.h"


#include <algorithm>
#include <functional>
#include "llcachename.h"
#include "lldbstrings.h"
#include "llinventory.h"

#include "llagent.h"
#include "llbutton.h"
#include "llcheckboxctrl.h"
#include "llfloateravatarinfo.h"
#include "llfloatergroupinfo.h"
#include "llinventorymodel.h"
#include "lllineeditor.h"
#include "llradiogroup.h"
#include "llresmgr.h"
#include "roles_constants.h"
#include "llselectmgr.h"
#include "lltextbox.h"
#include "lluiconstants.h"
#include "llviewerinventory.h"
#include "llviewerobjectlist.h"
#include "llviewerregion.h"
#include "llviewercontrol.h"

#include "lluictrlfactory.h"


const char* BULKPERM_QUEUE_TITLE = "Update Progress";
const char* BULKPERM_START_STRING = "update";

namespace
{
	struct BulkQueueObjects : public LLSelectedObjectFunctor
	{
		BOOL scripted;
		BOOL modifiable;
		LLFloaterBulkPermission* mQueue;
		BulkQueueObjects(LLFloaterBulkPermission* q) : mQueue(q), scripted(FALSE), modifiable(FALSE) {}
		virtual bool apply(LLViewerObject* obj)
		{
			scripted = obj->flagScripted();
			modifiable = obj->permModify();

			mQueue->addObject(obj->getID());
			return false;
			
		}
	};
}

///----------------------------------------------------------------------------
/// Class LLFloaterBulkPermission
///----------------------------------------------------------------------------

// static
LLMap<LLUUID, LLFloaterBulkPermission*> LLFloaterBulkPermission::sInstances;


// Default constructor
LLFloaterBulkPermission::LLFloaterBulkPermission(const std::string& name,
											 const LLRect& rect,
											 const char* title,
											 const char* start_string) :
	LLFloater(name, rect, title,
			  RESIZE_YES, DEFAULT_MIN_WIDTH, DEFAULT_MIN_HEIGHT,
			  DRAG_ON_TOP, MINIMIZE_YES, CLOSE_YES)
{

	req_perm_mask=0; // This should match the default state the checkboxes are set to
	recurse=false;

	LLUICtrlFactory::getInstance()->buildFloater(this,"floater_bulk_perms.xml");

	childSetAction("Apply...",onApplyBtn,this);
	childSetEnabled("Apply...",TRUE);

	childSetCommitCallback("Modify",&onCommitPermissions, this);
	childSetCommitCallback("Trans",&onCommitPermissions, this);
	childSetCommitCallback("Copy",&onCommitPermissions, this);

	//childSetCommitCallback("Recurse",&onRecurse, this);

	childSetCommitCallback("Parent",&onParent, this);

	childSetCommitCallback("objects",&InvSelection, this);
	childSetCommitCallback("scripts",&InvSelection, this);
	childSetCommitCallback("textures",&InvSelection, this);
	childSetCommitCallback("sounds",&InvSelection, this);
	childSetCommitCallback("animations",&InvSelection, this);
	childSetCommitCallback("notecards",&InvSelection, this);
	childSetCommitCallback("landmarks",&InvSelection, this);
	childSetCommitCallback("bodyparts",&InvSelection, this);
	childSetCommitCallback("clothing",&InvSelection, this);
	childSetCommitCallback("gestures",&InvSelection, this);

	//Set variable state to XUI default state consistancy
	processObject=getChild<LLCheckBoxCtrl>("objects")->get();
	processScript=getChild<LLCheckBoxCtrl>("scripts")->get();
	processTexture=getChild<LLCheckBoxCtrl>("textures")->get();
	processSound=getChild<LLCheckBoxCtrl>("sounds")->get();
	processAnimation=getChild<LLCheckBoxCtrl>("animations")->get();
	processNotecard=getChild<LLCheckBoxCtrl>("notecards")->get();
	processGesture=getChild<LLCheckBoxCtrl>("gestures")->get();
	processClothing=getChild<LLCheckBoxCtrl>("clothing")->get();
	processBodypart=getChild<LLCheckBoxCtrl>("bodyparts")->get();
	processLandmark=getChild<LLCheckBoxCtrl>("landmarks")->get();
	parent=getChild<LLCheckBoxCtrl>("Parent")->get();


	setTitle(title);
	
	if (!getHost())
	{
		LLRect curRect = getRect();
		translate(rect.mLeft - curRect.mLeft, rect.mTop - curRect.mTop);
	}
	
	mStartString = start_string;
	mDone = FALSE;
	sInstances.addData(mID, this);

}

void LLFloaterBulkPermission::doApply()
{
	// Its alive now do the nasty work that the ScriptQueue and friends try to do in the menu code
	// but first grab the user options

	LLScrollListCtrl* list = getChild<LLScrollListCtrl>("queue output");
	list->deleteAllItems();

	//Apply to selected objects if requested first

	if(parent)
	{
		llinfos<< "Setting permission on parent items" << llendl;
		LLSelectMgr::getInstance()->selectionSetObjectPermissions(PERM_NEXT_OWNER,true, req_perm_mask);
		LLSelectMgr::getInstance()->selectionSetObjectPermissions(PERM_NEXT_OWNER,false, ~req_perm_mask); //How annoying need to set and unset
	}


	LLFloaterBulkPermission* q;
	q=(LLFloaterBulkPermission*)this;

	BulkQueueObjects func(q);
	const bool firstonly = false;
	bool fail = LLSelectMgr::getInstance()->getSelection()->applyToObjects(&func, firstonly);
	if(fail)
	{
		if ( !func.modifiable )
		{
			gViewerWindow->alertXml("NO MODIFY");
		}
		else
		{
			llwarns << "Bad logic. Are there actualy any items in that prim?" << llendl;
		}
	}
	else
	{
		if (!q->start())
		{
			llwarns << "Unexpected failure attepmting to set permissions." << llendl;
		}
	}
}

// Destroys the object
LLFloaterBulkPermission::~LLFloaterBulkPermission()
{
	sInstances.removeData(mID);
}

// find an instance by ID. Return NULL if it does not exist.
// static
LLFloaterBulkPermission* LLFloaterBulkPermission::findInstance(const LLUUID& id)
{
	if(sInstances.checkData(id))
	{
		return sInstances.getData(id);
	}
	return NULL;
}


// This is the callback method for the viewer object currently being
// worked on.
// NOT static, virtual!
void LLFloaterBulkPermission::inventoryChanged(LLViewerObject* viewer_object,
											 InventoryObjectList* inv,
											 S32,
											 void* q_id)
{
	llinfos << "LLFloaterBulkPermission::inventoryChanged() for  object "
			<< viewer_object->getID() << llendl;

	//Remove this listener from the object since its
	//listener callback is now being executed.
	
	//We remove the listener here because the function
	//removeVOInventoryListener removes the listener from a ViewerObject
	//which it internally stores.
	
	//If we call this further down in the function, calls to handleInventory
	//and nextObject may update the interally stored viewer object causing
	//the removal of the incorrect listener from an incorrect object.
	
	//Fixes SL-6119:Recompile scripts fails to complete
	removeVOInventoryListener();

	if (viewer_object && inv && (viewer_object->getID() == mCurrentObjectID) )
	{
		handleInventory(viewer_object, inv);
	}
	else
	{
		// something went wrong...
		// note that we're not working on this one, and move onto the
		// next object in the list.
		llwarns << "No inventory for " << mCurrentObjectID
				<< llendl;
		nextObject();
	}
}

void LLFloaterBulkPermission::onApplyBtn(void* user_data)
{
	LLFloaterBulkPermission* self = (LLFloaterBulkPermission*)user_data;
	self->doApply();
}


// static
void LLFloaterBulkPermission::InvSelection(LLUICtrl* ctrl, void* data)
{
	LLFloaterBulkPermission* self = (LLFloaterBulkPermission*)data;

	self->processObject=self->getChild<LLCheckBoxCtrl>("objects")->get();
	self->processScript=self->getChild<LLCheckBoxCtrl>("scripts")->get();
	self->processTexture=self->getChild<LLCheckBoxCtrl>("textures")->get();
	self->processSound=self->getChild<LLCheckBoxCtrl>("sounds")->get();
	self->processAnimation=self->getChild<LLCheckBoxCtrl>("animations")->get();
	self->processNotecard=self->getChild<LLCheckBoxCtrl>("notecards")->get();
	self->processGesture=self->getChild<LLCheckBoxCtrl>("gestures")->get();
	self->processClothing=self->getChild<LLCheckBoxCtrl>("clothing")->get();
	self->processBodypart=self->getChild<LLCheckBoxCtrl>("bodyparts")->get();
	self->processLandmark=self->getChild<LLCheckBoxCtrl>("landmarks")->get();


}

// static
void LLFloaterBulkPermission::onParent(LLUICtrl* ctrl, void* data)
{
	LLFloaterBulkPermission* self = (LLFloaterBulkPermission*)data;
	self->parent=self->getChild<LLCheckBoxCtrl>("Parent")->get();
}

// static
void LLFloaterBulkPermission::onRecurse(LLUICtrl* ctrl, void* data)
{
	LLFloaterBulkPermission* self = (LLFloaterBulkPermission*)data;
	self->recurse=self->getChild<LLCheckBoxCtrl>("Recurse")->get();
}

// static
void LLFloaterBulkPermission::onCommitPermissions(LLUICtrl* ctrl, void* data)
{
	LLFloaterBulkPermission* self = (LLFloaterBulkPermission*)data;
	LLCheckBoxCtrl* CheckModify = self->getChild<LLCheckBoxCtrl>("Modify");
	LLCheckBoxCtrl* CheckCopy = self->getChild<LLCheckBoxCtrl>("Copy");
	LLCheckBoxCtrl* CheckTrans = self->getChild<LLCheckBoxCtrl>("Trans");

	self->req_perm_mask=0;

	if(CheckModify->get())
	{
		self->req_perm_mask|=PERM_MODIFY;
	}
	else
	{
		self->req_perm_mask&=~PERM_MODIFY;
	}

	if(CheckCopy->get())
	{
		self->req_perm_mask|=PERM_COPY;
	}
	else
	{
		self->req_perm_mask&=~PERM_COPY;
	}

	if(CheckTrans->get())
	{
		self->req_perm_mask|=PERM_TRANSFER;
	}
	else
	{
		self->req_perm_mask&=~PERM_TRANSFER;
	}


}

void LLFloaterBulkPermission::addObject(const LLUUID& id)
{
	mObjectIDs.put(id);
}

BOOL LLFloaterBulkPermission::start()
{
	llinfos << "LLFloaterBulkPermission::start()" << llendl;
	char buffer[MAX_STRING]; 				/*Flawfinder: ignore*/
	snprintf(buffer, sizeof(buffer), "Starting %s of %d items.", mStartString, mObjectIDs.count()); 		/* Flawfinder: ignore */

	LLScrollListCtrl* list = getChild<LLScrollListCtrl>("queue output");
	list->addCommentText(buffer);

	return nextObject();
}

BOOL LLFloaterBulkPermission::isDone() const
{
	return (mCurrentObjectID.isNull() || (mObjectIDs.count() == 0));
}

// go to the next object. If no objects left, it falls out silently
// and waits to be killed by the window being closed.
BOOL LLFloaterBulkPermission::nextObject()
{
	S32 count;
	BOOL successful_start = FALSE;
	do
	{
		count = mObjectIDs.count();
		llinfos << "LLFloaterBulkPermission::nextObject() - " << count
				<< " objects left to process." << llendl;
		mCurrentObjectID.setNull();
		if(count > 0)
		{
			successful_start = popNext();
		}
		llinfos << "LLFloaterBulkPermission::nextObject() "
				<< (successful_start ? "successful" : "unsuccessful")
				<< llendl; 
	} while((mObjectIDs.count() > 0) && !successful_start);

	if(isDone() && !mDone)
	{
		
		LLScrollListCtrl* list = getChild<LLScrollListCtrl>("queue output");
		mDone = TRUE;
		char buffer[MAX_STRING];		/*Flawfinder: ignore*/
		snprintf(buffer, sizeof(buffer), "Done.");				/* Flawfinder: ignore */
		list->addCommentText(buffer);

	}
	return successful_start;
}

// returns true if the queue has started, otherwise false.  This
// method pops the top object off of the queue.
BOOL LLFloaterBulkPermission::popNext()
{
	// get the first element off of the container, and attempt to get
	// the inventory.
	BOOL rv = FALSE;
	S32 count = mObjectIDs.count();
	if(mCurrentObjectID.isNull() && (count > 0))
	{
		mCurrentObjectID = mObjectIDs.get(0);
		llinfos << "LLFloaterBulkPermission::popNext() - mCurrentID: "
				<< mCurrentObjectID << llendl;
		mObjectIDs.remove(0);
		LLViewerObject* obj = gObjectList.findObject(mCurrentObjectID);
		if(obj)
		{
			llinfos << "LLFloaterBulkPermission::popNext() requesting inv for "
					<< mCurrentObjectID << llendl;
			LLUUID* id = new LLUUID(mID);

			registerVOInventoryListener(obj,id);
			requestVOInventory();
			rv = TRUE;
		}
		else
		{
			llinfos<<"LLFloaterBulkPermission::popNext() returned a NULL LLViewerObject" <<llendl;
			//Arrrg what do we do here?
		}
	}

	return rv;
}


// static
LLFloaterBulkPermission* LLFloaterBulkPermission::create()
{
	S32 left, top;
	gFloaterView->getNewFloaterPosition(&left, &top);
	LLRect rect = gSavedSettings.getRect("CompileOutputRect");
	rect.translate(left - rect.mLeft, top - rect.mTop);
	LLFloaterBulkPermission* new_queue = new LLFloaterBulkPermission("queue",rect,"Setting Bulk permissions","Results");
	new_queue->open();	 /*Flawfinder: ignore*/
	return new_queue;
}


void LLFloaterBulkPermission::handleInventory(LLViewerObject* viewer_obj, InventoryObjectList* inv)
{
	// find all of the lsl, leaving off duplicates. We'll remove
	// all matching asset uuids on compilation success.

	llinfos<<"handleInventory"<<llendl;

	char buffer[MAX_STRING];		 /*Flawfinder: ignore*/
	LLScrollListCtrl* list = getChild<LLScrollListCtrl>("queue output");

	InventoryObjectList::const_iterator it = inv->begin();
	InventoryObjectList::const_iterator end = inv->end();
	for ( ; it != end; ++it)
	{
		llinfos<<"Doing iterator of inventory"<<llendl;

		if(  ( (*it)->getType() == LLAssetType::AT_LSL_TEXT && processScript) ||
  		     ( (*it)->getType() == LLAssetType::AT_TEXTURE && processTexture) ||
	             ( (*it)->getType() == LLAssetType::AT_SOUND && processSound) ||
	             ( (*it)->getType() == LLAssetType::AT_LANDMARK && processLandmark) ||
    		     ( (*it)->getType() == LLAssetType::AT_CLOTHING && processClothing) ||
    		     ( (*it)->getType() == LLAssetType::AT_OBJECT && processObject) ||
   		     ( (*it)->getType() == LLAssetType::AT_NOTECARD && processNotecard) ||
   		     ( (*it)->getType() == LLAssetType::AT_BODYPART && processBodypart) ||
   		     ( (*it)->getType() == LLAssetType::AT_ANIMATION && processAnimation) ||
   		     ( (*it)->getType() == LLAssetType::AT_GESTURE && processGesture))
		{

			LLViewerObject* object = gObjectList.findObject(viewer_obj->getID());

			if (object)
			{
				LLInventoryItem* item = (LLInventoryItem*)((LLInventoryObject*)(*it));
				LLViewerInventoryItem* new_item = (LLViewerInventoryItem*)item;
				LLPermissions perm(new_item->getPermissions());

				// chomp the inventory name so it fits in the scroll window nicely
				// and the user can see the [OK]
				std::string invname;
				invname=item->getName().substr(0,item->getName().size() < 30 ? item->getName().size() : 30 );
				
				// My attempt at checking valid permissions, CHECK ME
				// note its not actually bad to try to set permissions that are not allowed as the
				// server will protect against this, but it will piss the user off if its wrong
				if(
				(perm.getCreator()==gAgentID) ||
				(perm.getMaskOwner() & PERM_TRANSFER) && (perm.getMaskOwner() & PERM_MODIFY) || 
				(gAgent.getGroupID()==perm.getGroup() && (perm.getMaskGroup() & PERM_TRANSFER) && (perm.getMaskGroup() & PERM_MODIFY))
				){	
					llinfos<<"Setting perms"<<llendl;
					perm.setMaskNext(req_perm_mask);
					new_item->setPermissions(perm);
					updateInventory(object,new_item,TASK_INVENTORY_ITEM_KEY,FALSE);				
					snprintf(buffer, sizeof(buffer), "Setting perms on '%s' [OK]", invname.c_str());		 	/* Flawfinder: ignore */
				}
				else
				{
					llinfos<<"NOT setting perms"<<llendl;
					snprintf(buffer, sizeof(buffer), "Setting perms on '%s' [FAILED]", invname.c_str());		 	/* Flawfinder: ignore */

				}
				
				list->addCommentText(buffer);

				if(recurse &&  ( (*it)->getType() == LLAssetType::AT_OBJECT && processObject))
				{
					//Add this object back to the queue to be processed as it has inventory
					snprintf(buffer, sizeof(buffer), "Queueing object '%s' for open", invname.c_str());
					llwarns << "Queueing object "<<	invname.c_str() << " ID "<< (*it)->getUUID()<<llendl;
					mObjectIDs.put((*it)->getUUID());
					// This will not YET work. as this is not a viewer object the unpack will fail			
				}

			}
		}
	}

	nextObject();	
}


// Avoid inventory callbacks etc by just fire and forgetting the message with the permissions update
// we could do this via LLViewerObject::updateInventory but that uses inventory call backs and buggers
// us up and we would have a dodgy item iterator

void LLFloaterBulkPermission::updateInventory(
	LLViewerObject* object,
	LLViewerInventoryItem* item,
	U8 key,
	bool is_new)
{
	LLMemType mt(LLMemType::MTYPE_OBJECT);
	

	// This slices the object into what we're concerned about on the
	// viewer. The simulator will take the permissions and transfer
	// ownership.
	LLPointer<LLViewerInventoryItem> task_item =
		new LLViewerInventoryItem(item->getUUID(), mID, item->getPermissions(),
								  item->getAssetUUID(), item->getType(),
								  item->getInventoryType(),
								  item->getName(), item->getDescription(),
								  item->getSaleInfo(),
								  item->getFlags(),
								  item->getCreationDate());
	task_item->setTransactionID(item->getTransactionID());
	LLMessageSystem* msg = gMessageSystem;
	msg->newMessageFast(_PREHASH_UpdateTaskInventory);
	msg->nextBlockFast(_PREHASH_AgentData);
	msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
	msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
	msg->nextBlockFast(_PREHASH_UpdateData);
	msg->addU32Fast(_PREHASH_LocalID, object->mLocalID);
	msg->addU8Fast(_PREHASH_Key, key);
	msg->nextBlockFast(_PREHASH_InventoryData);
	task_item->packMessage(msg);
	msg->sendReliable(object->getRegion()->getHost());

}

