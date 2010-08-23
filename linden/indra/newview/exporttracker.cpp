/* Copyright (c) 2009
 *
 * Modular Systems All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following
 * conditions are met:
 *
 *   1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *   3. Neither the name Modular Systems Ltd nor the names of its contributors
 *      may be used to endorse or promote products derived from this
 *      software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY MODULAR SYSTEMS LTD AND CONTRIBUTORS “AS IS”
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL MODULAR SYSTEMS OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

//jcool410 was here ;P
//HPA export and more added -Patrick Sapinski (Wednesday, November 25, 2009)

#include "llviewerprecompiledheaders.h"

#include "exporttracker.h"
#include "llviewerobjectlist.h"
#include "llvoavatar.h"
#include "llsdutil.h"
#include "llsdserialize.h"
#include "lldirpicker.h"
#include "llfilepicker.h"
#include "llviewerregion.h"
#include "llwindow.h"
#include "lltransfersourceasset.h"
#include "lltooldraganddrop.h"
#include "llviewernetwork.h"
#include "llcurl.h"
#include "llviewerimagelist.h"
#include "llscrolllistctrl.h"

#include "llimagej2c.h"

#include "llviewertexteditor.h"
#include "lllogchat.h" //for timestamp
#include "lluictrlfactory.h"
#include "llcheckboxctrl.h"
#include "llcallbacklist.h"

#include <fstream>
using namespace std;

#ifdef LL_STANDALONE
# include <zlib.h>
#else
# include "zlib/zlib.h"
#endif

JCExportTracker* JCExportTracker::sInstance;
LLSD JCExportTracker::total;
U32 JCExportTracker::totalprims;
BOOL JCExportTracker::export_properties;
BOOL JCExportTracker::export_inventory;
BOOL JCExportTracker::export_tga;
BOOL JCExportTracker::export_j2c;
U32 JCExportTracker::status;
std::string JCExportTracker::destination;
std::string JCExportTracker::asset_dir;
std::set<LLUUID> JCExportTracker::requested_textures;
LLVector3 JCExportTracker::selection_center;
LLVector3 JCExportTracker::selection_size;
void cmdline_printchat(std::string chat);
std::list<PropertiesRequest_t*> JCExportTracker::requested_properties;
std::list<InventoryRequest_t*> JCExportTracker::requested_inventory;

ExportTrackerFloater* ExportTrackerFloater::sInstance = 0;
LLDynamicArray<LLViewerObject*> ExportTrackerFloater::objectselection;
LLDynamicArray<LLViewerObject*> ExportTrackerFloater::mObjectSelectionWaitList;

std::list<LLSD *> JCExportTracker::processed_prims;
std::map<LLUUID,LLSD *> JCExportTracker::received_inventory;
std::map<LLUUID,LLSD *> JCExportTracker::received_properties;

int ExportTrackerFloater::properties_received = 0;
int ExportTrackerFloater::inventories_received = 0;
int ExportTrackerFloater::property_queries = 0;
int ExportTrackerFloater::assets_exported = 0;
int ExportTrackerFloater::textures_exported = 0;
int ExportTrackerFloater::total_objects = 0;
int ExportTrackerFloater::total_assets = 0;
int ExportTrackerFloater::total_textures = 0;
int ExportTrackerFloater::total_linksets = 0;
int ExportTrackerFloater::linksets_exported = 0;

void ExportTrackerFloater::draw()
{
	LLFloater::draw();

	std::string status_text;

	if (JCExportTracker::status == JCExportTracker::IDLE)
		status_text = "idle";
	else
		status_text = "exporting";

	//is this a bad place for this function? -Patrick Sapinski (Friday, November 13, 2009)
	sInstance->getChild<LLTextBox>("status label")->setValue(
		"Status: " + status_text
		+  llformat("\nObjects: %u/%u",linksets_exported,total_linksets)
		+  llformat("\nPrimitives: %u/%u",JCExportTracker::totalprims,total_objects)
		+  llformat("\nProperties Received: %u/%u",properties_received,total_objects)
		+  llformat("\nInventories Received: %u/%u",inventories_received,total_objects)
		//+  llformat("\n   Pending Queries: %u",JCExportTracker::requested_properties.size())
		+  llformat("\nInventory Items: %u/%u",assets_exported,total_assets)
		//+  llformat("\n   Pending Queries: %u",JCExportTracker::requested_inventory.size())
		+  llformat("\nTextures: %u/%u",textures_exported,total_textures)
		);

	// Draw the progress bar.

	LLRect rec  = getChild<LLPanel>("progress_bar")->getRect();

	S32 bar_width = rec.getWidth();
	S32 left = rec.mLeft, right = rec.mRight;
	S32 top = rec.mTop;
	S32 bottom = rec.mBottom;

	gGL.color4f(0.f, 0.f, 0.f, 0.75f);
	gl_rect_2d(rec);

	F32 data_progress = ((F32)JCExportTracker::totalprims/total_objects);

	if (data_progress > 0.0f)
	{
		// Downloaded bytes
		right = left + llfloor(data_progress * (F32)bar_width);
		if (right > left)
		{
			gGL.color4f(0.f, 0.f, 1.f, 0.75f);
			gl_rect_2d(left, top, right, bottom);
		}
	}

	//find active tab

	S32 panel = sInstance->getChild<LLTabContainer>("exporttab")->getCurrentPanelIndex();
	/* if (panel == 0) //Error list
	{
		cmdline_printchat("error panel selected");
	}
	else */if (panel == 1) //object list
	{
		//update export queue list
		LLScrollListCtrl* mResultList;
		mResultList = getChild<LLScrollListCtrl>("object_result_list");

		LLDynamicArray<LLUUID> selected = mResultList->getSelectedIDs();
		S32 scrollpos = mResultList->getScrollPos();
		mResultList->deleteAllItems();

		LLDynamicArray<LLViewerObject*> objectlist;

		if (mObjectSelectionWaitList.empty())
		{
			objectlist = objectselection;
		}
		else {
			objectlist = mObjectSelectionWaitList;
		}
		

		LLDynamicArray<LLViewerObject*>::iterator iter=objectlist.begin();
		LLViewerObject* object = NULL;

		for(;iter!=objectlist.end();iter++)
		{
			object = (*iter);
			LLVector3 object_pos = object->getPosition();

			LLSD element;
			element["id"] = llformat("%u",object->getLocalID()); //object->getLocalID();

			LLSD * props=JCExportTracker::received_properties[object->getID()];
			std::string name = "";
			if(props!=NULL)
			{
				name = std::string((*props)["name"]);
			}

			element["columns"][0]["column"] = "Name";
			element["columns"][0]["type"] = "text";
			element["columns"][0]["value"] = name;

			element["columns"][1]["column"] = "UUID";
			element["columns"][1]["type"] = "text";
			element["columns"][1]["value"] = object->getID();

			if (object->mPropertiesRecieved)
			{
				element["columns"][2]["column"] = "icon_prop";
				element["columns"][2]["type"] = "icon";
				element["columns"][2]["value"] = "account_id_green.tga";
			}
			if (object->mInventoryRecieved)
			{
				element["columns"][3]["column"] = "icon_inv";
				element["columns"][3]["type"] = "icon";
				element["columns"][3]["value"] = "account_id_green.tga";
			}

			element["columns"][4]["column"] = "Local ID";
			element["columns"][4]["type"] = "text";
			element["columns"][4]["value"] = llformat("%u",object->getLocalID());

			std::stringstream sstr;	
			sstr <<llformat("%.1f", object_pos.mV[VX]);
			sstr <<","<<llformat("%.1f", object_pos.mV[VY]);
			sstr <<","<<llformat("%.1f", object_pos.mV[VZ]);

			element["columns"][5]["column"] = "Position";
			element["columns"][5]["type"] = "text";
			element["columns"][5]["value"] = sstr.str();

			std::list<PropertiesRequest_t *>::iterator iter=JCExportTracker::requested_properties.begin();
			for(;iter!=JCExportTracker::requested_properties.end();iter++)
			{
				PropertiesRequest_t * req=(*iter);
				if(req->localID==object->getLocalID())
				{
					element["columns"][6]["column"] = "Retries";
					element["columns"][6]["type"] = "text";
					element["columns"][6]["value"] = llformat("%u",req->num_retries);;
				}
			}

			mResultList->addElement(element, ADD_BOTTOM);
		}


		std::list<LLSD *>::iterator iter2=JCExportTracker::processed_prims.begin();
		for(;iter2!=JCExportTracker::processed_prims.end();iter2++)
		{	// for each object

			LLSD *plsd=(*iter2);
			//llwarns<<LLSD::dump(*plsd)<<llendl;
			for(LLSD::array_iterator link_itr = plsd->beginArray();
				link_itr != plsd->endArray();
				++link_itr)
			{ 
				LLSD prim = (*link_itr);

				LLSD element;
				element["id"] = prim["id"]; //object->getLocalID();

				element["columns"][0]["column"] = "Name";
				element["columns"][0]["type"] = "text";
				element["columns"][0]["value"] = "";//name;

				element["columns"][1]["column"] = "UUID";
				element["columns"][1]["type"] = "text";
				element["columns"][1]["value"] = "";

				element["columns"][2]["column"] = "icon_prop";
				element["columns"][2]["type"] = "icon";
				element["columns"][2]["value"] = "account_id_green.tga";

				element["columns"][3]["column"] = "icon_inv";
				element["columns"][3]["type"] = "icon";
				element["columns"][3]["value"] = "account_id_green.tga";

				element["columns"][4]["column"] = "Local ID";
				element["columns"][4]["type"] = "text";
				element["columns"][4]["value"] = prim["id"];

				LLVector3 position;
				position.setVec(ll_vector3d_from_sd(prim["position"]));

				std::stringstream sstr;	
				sstr <<llformat("%.1f", position.mV[VX]);
				sstr <<","<<llformat("%.1f", position.mV[VY]);
				sstr <<","<<llformat("%.1f", position.mV[VZ]);

				element["columns"][5]["column"] = "Position";
				element["columns"][5]["type"] = "text";
				element["columns"][5]["value"] = sstr.str();

				std::list<PropertiesRequest_t *>::iterator iter=JCExportTracker::requested_properties.begin();
				for(;iter!=JCExportTracker::requested_properties.end();iter++)
				{
					PropertiesRequest_t * req=(*iter);
					if(req->localID==object->getLocalID())
					{
						cmdline_printchat("match");
						element["columns"][6]["column"] = "Retries";
						element["columns"][6]["type"] = "text";
						element["columns"][6]["value"] = llformat("%u",req->num_retries);;
					}
				}

				mResultList->addElement(element, ADD_BOTTOM);
			}
		}

		//mResultList->sortItems();
		mResultList->selectMultiple(selected);
		mResultList->setScrollPos(scrollpos);
	}
	else if (panel == 2) //inventory list
	{
	}
}


ExportTrackerFloater::ExportTrackerFloater()
:	LLFloater( std::string("Prim Export Floater") )
{
	LLUICtrlFactory::getInstance()->buildFloater( this, "floater_prim_export.xml" );

	childSetAction("export", onClickExport, this);
	childSetAction("close", onClickClose, this);
	childSetEnabled("export",true);

	childSetEnabled("export_tga",true);
	childSetEnabled("export_j2c",true);
	childSetEnabled("export_properties",true);
	childSetEnabled("export_contents",true);

	//from serializeselection
	JCExportTracker::init();

	gIdleCallbacks.deleteFunction(JCExportTracker::exportworker);
	properties_received = 0;
	inventories_received = 0;
	property_queries = 0;
	assets_exported = 0;
	textures_exported = 0;
	total_assets = 0;
	linksets_exported = 0;
	total_textures = 0;

	total_linksets = LLSelectMgr::getInstance()->getSelection()->getRootObjectCount();
	total_objects = LLSelectMgr::getInstance()->getSelection()->getObjectCount();
	//total_textures = LLSelectMgr::getInstance()->getSelection()->getTECount(); is this unique textures?
}

ExportTrackerFloater* ExportTrackerFloater::getInstance()
{
    if ( ! sInstance )
        sInstance = new ExportTrackerFloater();

	return sInstance;
}

ExportTrackerFloater::~ExportTrackerFloater()
{	
	JCExportTracker::sInstance->close();
	//which one?? -Patrick Sapinski (Wednesday, November 11, 2009)
	ExportTrackerFloater::sInstance = NULL;
	sInstance = NULL;
}

void ExportTrackerFloater::close()
{
	if(sInstance)
	{
		delete sInstance;
		sInstance = NULL;
	}
}

void ExportTrackerFloater::show()
{
	if(sInstance)
	{
		JCExportTracker::sInstance->close();
		delete sInstance;
		sInstance = NULL;
	}

	if(NULL==sInstance) 
	{
		sInstance = new ExportTrackerFloater();
		llwarns << "sInstance assigned. sInstance=" << sInstance << llendl;
	}
	
	sInstance->open();	/*Flawfinder: ignore*/

	LLDynamicArray<LLViewerObject*> catfayse;
	for (LLObjectSelection::valid_root_iterator iter = LLSelectMgr::getInstance()->getSelection()->valid_root_begin();
		iter != LLSelectMgr::getInstance()->getSelection()->valid_root_end(); iter++)
	{
		LLSelectNode* selectNode = *iter;
		LLViewerObject* object = selectNode->getObject();
		if(object)
		{
			if(object->isAvatar())
			{
				JCExportTracker::error("", object->getLocalID(), object->getPosition(), "Invalid Object (Avatar)");
				total_linksets--;			
				total_objects--;
				total_objects-=object->getChildren().size();
			}
			else if(FOLLOW_PERMS)
			{
				if(!(object->permModify() && object->permCopy() && object->permTransfer()))
				{
					JCExportTracker::error("", object->getLocalID(), object->getPosition(), "Invalid Permissions");
					total_linksets--;			
					total_objects--;
					total_objects-=object->getChildren().size();
				}
				else
				{
					catfayse.put(object);
				}
			}
			else
			{
				catfayse.put(object);
			}
		}
		else 
		{
			JCExportTracker::error("", object->getLocalID(), object->getPosition(), "Invalid Object");
			total_linksets--;			
			total_objects--;
			total_objects-=object->getChildren().size();
		}
		//cmdline_printchat(" adding " + llformat("%d",total_linksets));
	}
	//cmdline_printchat(llformat("%d",export_properties));
	//cmdline_printchat("getrootobjectcount " + llformat("%d",LLSelectMgr::getInstance()->getSelection()->getRootObjectCount()));
	//cmdline_printchat(" size " + llformat("%d",catfayse.size()));

	objectselection = catfayse;

	LLBBox bbox = LLSelectMgr::getInstance()->getBBoxOfSelection();
	LLVector3 box_center_agent = bbox.getCenterAgent();

	LLVector3 temp = bbox.getExtentLocal();

	std::stringstream sstr;	
	LLUICtrl * ctrl=this->getChild<LLUICtrl>("selection size");	

	sstr <<"X: "<<llformat("%.2f", temp.mV[VX]);
	sstr <<", Y: "<<llformat("%.2f", temp.mV[VY]);
	sstr <<", Z: "<<llformat("%.2f", temp.mV[VZ]);

	ctrl->setValue(LLSD("Text")=sstr.str());

	JCExportTracker::selection_size = bbox.getExtentLocal();
	JCExportTracker::selection_center = bbox.getCenterAgent();
}

// static
void ExportTrackerFloater::onClickExport(void* data)
{
	JCExportTracker::export_tga = sInstance->getChild<LLCheckBoxCtrl>("export_tga")->get();
	JCExportTracker::export_j2c = sInstance->getChild<LLCheckBoxCtrl>("export_j2c")->get();
	JCExportTracker::export_properties = sInstance->getChild<LLCheckBoxCtrl>("export_properties")->get();
	JCExportTracker::export_inventory = sInstance->getChild<LLCheckBoxCtrl>("export_contents")->get();
	//sInstance->mExportTrees=sInstance->getChild<LLCheckBoxCtrl>("export_trees")->get();

	JCExportTracker::serialize(objectselection);
}

// static
void ExportTrackerFloater::onClickClose(void* data)
{
	sInstance->close();
	JCExportTracker::sInstance->close();
}

JCExportTracker::JCExportTracker()
{
	llassert_always(sInstance == NULL);
	sInstance = this;

}

JCExportTracker::~JCExportTracker()
{
	JCExportTracker::cleanup();
	sInstance = NULL;
}

void JCExportTracker::close()
{
	if(sInstance)
	{
		delete sInstance;
		sInstance = NULL;
	}
}

void JCExportTracker::init()
{
	if(!sInstance)
	{
		sInstance = new JCExportTracker();
	}
	status = IDLE;
	totalprims = 0;
	total = LLSD();
	destination = "";
	asset_dir = "";
	requested_textures.clear();
}

LLVOAvatar* find_avatar_from_object( LLViewerObject* object );

LLVOAvatar* find_avatar_from_object( const LLUUID& object_id );



struct JCAssetInfo
{
	std::string path;
	std::string name;
	LLUUID assetid;
};


LLSD * JCExportTracker::subserialize(LLViewerObject* linkset)
{
	//Chalice - Changed to support exporting linkset groups.
	LLViewerObject* object = linkset;
	//if(!linkset)return LLSD();

	// Create an LLSD object that will hold the entire tree structure that can be serialized to a file
	LLSD * pllsd= new LLSD();
	
	//if (!node)
	//	return llsd;

	//object = root_object = node->getObject();
	
	if (!object)
	{
		error("", object->getLocalID(), object->getPosition(), "Invalid Object");
		delete pllsd;	
		return NULL;
	}

	if (!(!object->isAvatar()))
	{
		error("", object->getLocalID(), object->getPosition(), "Invalid Object (Avatar)");
		delete pllsd;	
		return NULL;
	}

	// Build a list of everything that we'll actually be exporting
	LLDynamicArray<LLViewerObject*> export_objects;

	if (!object)
	{
		cmdline_printchat("subserialize: invalid obj");
	}
	if (object->isDead())
	{
		cmdline_printchat("subserialize: object is dead");
		return NULL;
	}
	/*
	LLPCode pcode = object->getPCode();
	if (pcode != LL_PCODE_VOLUME &&
		pcode != LL_PCODE_LEGACY_GRASS &&
		pcode != LL_PCODE_LEGACY_TREE)
	{
		cmdline_printchat("subserialize: object has invalid pcode");
		return NULL;
	}
	*/

//#define _KOW

#ifdef _KOW
	//begin: copy object workaround.
	if (!object->permYouOwner()) //&& check if user opted to copy non owned objects
	{ //we don't own this object
		cmdline_printchat("someone else owns object " + llformat("%d",object->getLocalID()));

		//try and take a copy
		U8 d = (U8)DRD_ACQUIRE_TO_AGENT_INVENTORY;
		const LLUUID& category_id = gInventory.findCategoryUUIDForType(LLAssetType::AT_OBJECT);
		LLUUID tid;
		tid.generate();
		LLMessageSystem* msg = gMessageSystem;

		msg->newMessageFast(_PREHASH_DeRezObject);
		msg->nextBlockFast(_PREHASH_AgentData);
		msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
		msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
		msg->nextBlockFast(_PREHASH_AgentBlock);
		msg->addUUIDFast(_PREHASH_GroupID, gAgent.getGroupID());
		msg->addU8Fast(_PREHASH_Destination, d);	
		msg->addUUIDFast(_PREHASH_DestinationID, category_id);
		msg->addUUIDFast(_PREHASH_TransactionID, tid);
		msg->addU8Fast(_PREHASH_PacketCount, 1);
		msg->addU8Fast(_PREHASH_PacketNumber, 1);
		msg->nextBlockFast(_PREHASH_ObjectData);
		msg->addU32Fast(_PREHASH_ObjectLocalID, object->getLocalID());
		msg->sendReliable(object->getRegion()->getHost());

		//add our new copy to a list
		//copied_objects.put(object);

		
		//LLDynamicArray<U32> copied_objects;
		//U32 tmp = object->getLocalID();
		//copied_objects.push_back(tmp);
		

		//TODO: add callback which will attach this object and rerun it through this function

		
		#define vCatType (LLAssetType::EType)128
		#define vBridgeOpCat "#Emerald"

		
				//cmdline_printchat("foldering");
				//LLUUID vcatid;
				//vcatid = gInventory.findCategoryByName(vBridgeOpCat);
				//if(vcatid.isNull())
				//{
				//	cmdline_printchat("creating folder");
				//	vcatid = gInventory.createNewCategory(gAgent.getInventoryRootID(), LLAssetType::AT_NONE, vBridgeOpCat);
				//}
				std::string objectName = "blah_";
				LLUUID objectid = LLUUID("e97cf410-8e61-7005-ec06-629eba4cd1fb");//findInventoryByName(objectName);
				cmdline_printchat("id="+objectid.asString());
				LLViewerInventoryItem* object = gInventory.getItem(objectid);
				if(object)
				{
					if(0)//isworn(object->getUUID()))
					{
						cmdline_printchat("wearing object");
					}
					else
					{
						cmdline_printchat("attaching object");
						LLMessageSystem* msg = gMessageSystem;
						msg->newMessageFast(_PREHASH_RezSingleAttachmentFromInv);
						msg->nextBlockFast(_PREHASH_AgentData);
						msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
						msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
						msg->nextBlockFast(_PREHASH_ObjectData);
						msg->addUUIDFast(_PREHASH_ItemID, object->getUUID());
						msg->addUUIDFast(_PREHASH_OwnerID, object->getPermissions().getOwner());
						msg->addU8Fast(_PREHASH_AttachmentPt, 128);
						pack_permissions_slam(msg, object->getFlags(), object->getPermissions());
						msg->addStringFast(_PREHASH_Name, object->getName());
						msg->addStringFast(_PREHASH_Description, object->getDescription());
						msg->sendReliable(gAgent.getRegionHost());
					}
				}
	}
	else
#endif
	{ // we own this object.

	// Add the root object to the export list
	export_objects.put(object);

	//all child objects must also be active
	llassert_always(object);
	
	// Iterate over all of this objects children
	LLViewerObject::const_child_list_t& child_list = object->getChildren(); //this crashes sometimes. is using llassert a bad hack?? -Patrick Sapinski (Monday, November 23, 2009)
	
	for (LLViewerObject::child_list_t::const_iterator i = child_list.begin(); i != child_list.end(); i++)
	{
		LLViewerObject* child = *i;
		if(!child->isAvatar())
		{
			// Put the child objects on the export list
			export_objects.put(child);
		}
	}
	//deselect everything because the render lag from a large selection really slows down the exporter.
	LLSelectMgr::getInstance()->deselectAll();

	S32 object_index = 0;
	
	while ((object_index < export_objects.count()))
	{
		object = export_objects.get(object_index++);
		LLUUID id = object->getID();
	
		//llinfos << "Exporting prim " << object->getID().asString() << llendl;
	
		// Create an LLSD object that represents this prim. It will be injected in to the overall LLSD
		// tree structure
		LLSD prim_llsd;
	
		if (object_index == 1)
		{
			LLVOAvatar* avatar = find_avatar_from_object(object);
			if (avatar)
			{
				LLViewerJointAttachment* attachment = avatar->getTargetAttachmentPoint(object);
				U8 attachment_id = 0;
				
				if (attachment)
				{
					for (LLVOAvatar::attachment_map_t::iterator iter = avatar->mAttachmentPoints.begin();
										iter != avatar->mAttachmentPoints.end(); ++iter)
					{
						if (iter->second == attachment)
						{
							attachment_id = iter->first;
							break;
						}
					}
				}
				else
				{
					// interpret 0 as "default location"
					attachment_id = 0;
				}
				
				prim_llsd["Attachment"] = attachment_id;
				prim_llsd["attachpos"] = object->getPosition().getValue();
				prim_llsd["attachrot"] = ll_sd_from_quaternion(object->getRotation());
			}
			
			prim_llsd["position"] = object->getPositionRegion().getValue(); //LLVector3(0, 0, 0).getValue();
			prim_llsd["rotation"] = ll_sd_from_quaternion(object->getRotation());
		}
		else
		{
			prim_llsd["position"] = object->getPositionRegion().getValue();
			prim_llsd["rotation"] = ll_sd_from_quaternion(object->getRotationEdit());
		}
		//prim_llsd["name"] = "";//node->mName;
		//prim_llsd["description"] = "";//node->mDescription;
		// Transforms
		prim_llsd["scale"] = object->getScale().getValue();
		// Flags
		prim_llsd["shadows"] = object->flagCastShadows();
		prim_llsd["phantom"] = object->flagPhantom();
		prim_llsd["physical"] = (BOOL)(object->mFlags & FLAGS_USE_PHYSICS);

		// For saving tree and grass positions store the pcode so we 
		// know what to restore and the state is the species
		LLPCode pcode = object->getPCode();
		prim_llsd["pcode"] = pcode;
		prim_llsd["state"] = object->getState();

		// Only volumes have all the prim parameters
		if(LL_PCODE_VOLUME == pcode) 
		{
			LLVolumeParams params = object->getVolume()->getParams();
			prim_llsd["volume"] = params.asLLSD();

			if (object->isFlexible())
			{
				LLFlexibleObjectData* flex = (LLFlexibleObjectData*)object->getParameterEntry(LLNetworkData::PARAMS_FLEXIBLE);
				prim_llsd["flexible"] = flex->asLLSD();
			}
			if (object->getParameterEntryInUse(LLNetworkData::PARAMS_LIGHT))
			{
				LLLightParams* light = (LLLightParams*)object->getParameterEntry(LLNetworkData::PARAMS_LIGHT);
				prim_llsd["light"] = light->asLLSD();
			}
			if (object->getParameterEntryInUse(LLNetworkData::PARAMS_SCULPT))
			{

				LLSculptParams* sculpt = (LLSculptParams*)object->getParameterEntry(LLNetworkData::PARAMS_SCULPT);
				prim_llsd["sculpt"] = sculpt->asLLSD();

				if(export_tga || export_j2c)
				{
					LLFile::mkdir(asset_dir+"//sculptmaps//");
					std::string path = asset_dir+"//sculptmaps//";
					LLUUID asset_id = sculpt->getSculptTexture();
					JCAssetInfo* info = new JCAssetInfo;
					info->path = path + asset_id.asString();
					info->name = "Sculpt Texture";
					if(requested_textures.count(asset_id) == 0)
					{
						ExportTrackerFloater::total_textures++;
						requested_textures.insert(asset_id);
						LLViewerImage* img = gImageList.getImage(asset_id, MIPMAP_TRUE, FALSE);
						img->setBoostLevel(LLViewerImageBoostLevel::BOOST_MAX_LEVEL);
						img->forceToSaveRawImage(0); //this is required for us to receive the full res image.
						//img->setAdditionalDecodePriority(1.0f) ;
						img->setLoadedCallback( JCExportTracker::onFileLoadedForSave, 
										0, TRUE, FALSE, info );
						//llinfos << "Requesting texture " << asset_id.asString() << llendl;
					}
				}
			}
		}

		// Textures
		LLSD te_llsd;
		U8 te_count = object->getNumTEs();
		
		for (U8 i = 0; i < te_count; i++)
		{
			te_llsd.append(object->getTE(i)->asLLSD());
		}

		if(export_tga || export_j2c)
		{
			LLFile::mkdir(asset_dir+"//textures//");
			std::string path = asset_dir+"//textures//";
			for (U8 i = 0; i < te_count; i++)
			{
				LLUUID asset_id = object->getTE(i)->getID();
				JCAssetInfo* info = new JCAssetInfo;
				info->path = path + asset_id.asString();
				info->name = "Prim Texture";
				//gAssetStorage->getAssetData(asset_id, LLAssetType::AT_TEXTURE, JCAssetExportCallback, info,1);
				if(requested_textures.count(asset_id) == 0)
				{
					ExportTrackerFloater::total_textures++;
					requested_textures.insert(asset_id);
					LLViewerImage* img = gImageList.getImage(asset_id, MIPMAP_TRUE, FALSE);
					
					//RC
					//if we already have this texture it will never fire a loaded callback
					//so kick directly and generate a raw image ourselves from a GL readback
					if(img->getDiscardLevel()==0)
					{
						//llinfos << "Already have texture " << asset_id.asString() << " in memory, attemping GL readback" << llendl;
						onFileLoadedForSave(true,img,NULL,NULL,0,true,info);
					}
					else
					{
						img->forceToSaveRawImage(0); //this is required for us to receive the full res image. (snowglobe)
						img->setBoostLevel(LLViewerImageBoostLevel::BOOST_MAX_LEVEL);	
						img->addTextureStats( (F32)MAX_IMAGE_AREA );
						img->setLoadedCallback( JCExportTracker::onFileLoadedForSave, 
										0, TRUE, FALSE, info );
						//llinfos << "Requesting texture " << asset_id.asString() << llendl;
					}
				}
			}
		}

		//JCExportTracker::mirror(asset, obj, asset_dir, asset->getUUID().asString());
		
		prim_llsd["textures"] = te_llsd;

		prim_llsd["id"] = object->getID().asString();
		
		totalprims += 1;

		// Changed to use link numbers zero-indexed.
		(*pllsd)[object_index - 1] = prim_llsd;
	}
	}
	return pllsd;
}


void JCExportTracker::error(std::string name, U32 localid, LLVector3 object_pos, std::string error_msg)
{
	std::stringstream sstr;	
	sstr <<llformat("%.1f", object_pos.mV[VX]);
	sstr <<","<<llformat("%.1f", object_pos.mV[VY]);
	sstr <<","<<llformat("%.1f", object_pos.mV[VZ]);

	if (ExportTrackerFloater::sInstance)
	{
		//add to error list
		LLSD element;
		element["id"] = llformat("%u",localid);

		element["columns"][0]["column"] = "Name";
		element["columns"][0]["type"] = "text";
		element["columns"][0]["value"] = name;

		element["columns"][1]["column"] = "Local ID";
		element["columns"][1]["type"] = "text";
		element["columns"][1]["value"] = llformat("%u",localid);

		element["columns"][2]["column"] = "Position";
		element["columns"][2]["type"] = "text";
		element["columns"][2]["value"] = sstr.str();

		element["columns"][3]["column"] = "Error Message";
		element["columns"][3]["type"] = "text";
		element["columns"][3]["value"] = error_msg;


		LLScrollListCtrl* mResultList;
		mResultList = ExportTrackerFloater::sInstance->getChild<LLScrollListCtrl>("error_result_list");
		mResultList->addElement(element, ADD_BOTTOM);
	}
	else
		cmdline_printchat("error:" + error_msg);
}

bool JCExportTracker::getAsyncData(LLViewerObject * obj)
{
	LLPCode pcode = obj->getPCode();
	if(pcode!=LL_PCODE_VOLUME)
		return false;

	if(export_properties)
	{
		bool already_requested_prop=false;
		std::list<PropertiesRequest_t *>::iterator iter=requested_properties.begin();
		for(;iter!=requested_properties.end();iter++)
		{
			PropertiesRequest_t * req=(*iter);
			if(req->localID==obj->getLocalID())
			{
				cmdline_printchat("BREAK: have properties for: " + llformat("%d",obj->getLocalID()));
				already_requested_prop=true;
				break;	
			}
		}
			
		if(already_requested_prop==false)
		{
			//save this request to the list
			PropertiesRequest_t * req = new PropertiesRequest_t();
			req->target_prim=obj->getID();
			req->request_time=0;
			req->num_retries = 0;	
			req->localID=obj->getLocalID();
			requested_properties.push_back(req);

			//cmdline_printchat("queued property request for: " + llformat("%d",obj->getLocalID()));

		}

		if(export_inventory)
		{

			bool already_requested_inv=false;
			std::list<InventoryRequest_t *>::iterator iter2=requested_inventory.begin();
			for(;iter2!=requested_inventory.end();iter2++)
			{
				InventoryRequest_t * req=(*iter2);
				if(req->object->getLocalID()==obj->getLocalID())
				{
					cmdline_printchat("BREAK already have inventory for: " + llformat("%d",obj->getLocalID()));
					already_requested_inv=true;
					break;
				}
			}

			if(already_requested_inv==false)
			{
				InventoryRequest_t * ireq = new InventoryRequest_t();
				ireq->object=obj;
				ireq->request_time = 0;	
				ireq->num_retries = 0;	
				requested_inventory.push_back(ireq);
				obj->registerInventoryListener(sInstance,(void*)ireq);
				//cmdline_printchat("registered inventory listener for: " + llformat("%d",ireq->object->getLocalID()));
			}
		}
	}
	return true;
}


void JCExportTracker::requestPrimProperties(U32 localID)
{
	gMessageSystem->newMessageFast(_PREHASH_ObjectSelect);
	gMessageSystem->nextBlockFast(_PREHASH_AgentData);
	gMessageSystem->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
	gMessageSystem->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
	gMessageSystem->nextBlockFast(_PREHASH_ObjectData);
    gMessageSystem->addU32Fast(_PREHASH_ObjectLocalID, localID);

    gMessageSystem->sendReliable(gAgent.getRegionHost());
}

void JCExportTracker::onFileLoadedForSave(BOOL success, 
											LLViewerImage *src_vi,
											LLImageRaw* src, 
											LLImageRaw* aux_src, 
											S32 discard_level,
											BOOL final,
											void* userdata)
{
	JCAssetInfo* info = (JCAssetInfo*)userdata;
	if(final)
	{
		if( success )
		{ /*
			LLPointer<LLImageJ2C> image_j2c = new LLImageJ2C();
			if(!image_j2c->encode(src,0.0))
			{
				//errorencode
				llinfos << "Failed to encode " << info->path << llendl;
			}else if(!image_j2c->save( info->path ))
			{
				llinfos << "Failed to write " << info->path << llendl;
				//errorwrite
			}else
			{
				ExportTrackerFloater::textures_exported++;
				llinfos << "Saved texture " << info->path << llendl;
				//success
			} */

			//RC
			//If we have a NULL raw image, then read one back from the GL buffer
			bool we_created_raw=false;
			if(src==NULL)
			{
				src = new LLImageRaw();
				we_created_raw=true;

				if(!src_vi->readBackRaw(0,src,false))
				{
					cmdline_printchat("Failed to readback texture");
					src->deleteData(); //check me, is this valid?
					delete info;
					return;
				}
			}
			
			if(export_tga)
			{
				LLPointer<LLImageTGA> image_tga = new LLImageTGA;

				if( !image_tga->encode( src ) )
				{
					llinfos << "Failed to encode " << info->path << llendl;
				}
				else if( !image_tga->save( info->path + ".tga" ) )
				{
					llinfos << "Failed to write " << info->path << llendl;
				}
			}

			if(export_j2c)
			{
				LLPointer<LLImageJ2C> image_j2c = new LLImageJ2C();
				if(!image_j2c->encode(src,0.0))
				{
					//errorencode
					llinfos << "Failed to encode " << info->path << llendl;
				}else if(!image_j2c->save( info->path+".j2c" ))
				{
					llinfos << "Failed to write " << info->path << llendl;
					//errorwrite
				}else
				{
					//llinfos << "Saved texture " << info->path << llendl;
					//success
				}
			}
			ExportTrackerFloater::textures_exported++;
		
			//RC
			//meh if we did a GL readback we created the raw image
			// so we better delete, but the destructor is private
			// so this needs checking for a memory leak that this is correct
			if(we_created_raw)
				src->deleteData();

		}
		delete info;
	}

}

bool JCExportTracker::serializeSelection()
{
	//init();
	LLDynamicArray<LLViewerObject*> catfayse;
	for (LLObjectSelection::valid_root_iterator iter = LLSelectMgr::getInstance()->getSelection()->valid_root_begin();
			 iter != LLSelectMgr::getInstance()->getSelection()->valid_root_end(); iter++)
	{
		LLSelectNode* selectNode = *iter;
		LLViewerObject* object = selectNode->getObject();
		if(object)catfayse.put(object);
	}
	return serialize(catfayse);
}

bool JCExportTracker::serialize(LLDynamicArray<LLViewerObject*> objects)
{
	LLFilePicker& file_picker = LLFilePicker::instance();
		
	if (!file_picker.getSaveFile(LLFilePicker::FFSAVE_HPA))
		return false; // User canceled save.

	init();

	status = EXPORTING;

	ExportTrackerFloater::sInstance->childSetEnabled("export",false);
	destination = file_picker.getFirstFile();
	asset_dir = gDirUtilp->getDirName(destination);

	//destination = destination.substr(0,destination.find_last_of("."));
/*	if(export_inventory)
	{
		asset_dir = destination.substr(0,destination.find_last_of(".")) + "_assets";//+gDirUtilp->getDirDelimiter();
		if(!LLFile::isdir(asset_dir))
		{
			LLFile::mkdir(asset_dir);
		}else
		{
			U32 response = OSMessageBox("Directory "+asset_dir+" already exists, write on top?", "Overwrite?", OSMB_YESNO);
			if(response)
			{
				return false;
			}
		}
	} */

	total.clear();

	gIdleCallbacks.addFunction(exportworker, NULL);

	return true;

}

void JCExportTracker::exportworker(void *userdata)
{
	//CHECK IF WE'RE DONE
	if(ExportTrackerFloater::objectselection.empty() && ExportTrackerFloater::mObjectSelectionWaitList.empty()
		&& requested_inventory.empty() && requested_properties.empty())
	{
		gIdleCallbacks.deleteFunction(exportworker);
		finalize();
		return;
	}
	
	int kick_count=0;
	int total=requested_properties.size();
	// Check for requested properties that are taking too long
	while(total!=0)
	{
		std::list<PropertiesRequest_t*>::iterator iter;
		time_t tnow=time(NULL);
		
		PropertiesRequest_t * req;
		iter=requested_properties.begin();
		req=(*iter);
		requested_properties.pop_front();
		
		if( (req->request_time+PROP_REQUEST_KICK)< tnow)
		{
			req->request_time=time(NULL);
			req->num_retries++;

			requestPrimProperties(req->localID);
			kick_count++;
			//cmdline_printchat("requested property for: " + llformat("%d",req->localID));
		}
		if (req->num_retries < 40)
			requested_properties.push_back(req);
		else
		{
			//req->localID->mPropertiesRecieved = true;		
			cmdline_printchat("failed to retrieve properties for ");// + req->localID);
			ExportTrackerFloater::properties_received++;
			LLViewerObject *obj = gObjectList.findObject(req->target_prim);
			if(obj)
			{
				obj->mPropertiesRecieved=true;
				cmdline_printchat("successfully set property to received");
			}
		}
		
		if(kick_count>=20)
			break; // that will do for now				

		total--;
	}

	kick_count=0;
	total=requested_inventory.size();
	// Check for inventory properties that are taking too long
	while(total!=0)
	{
		std::list<InventoryRequest_t*>::iterator iter;
		time_t tnow=time(NULL);
		
		InventoryRequest_t * req;
		iter=requested_inventory.begin();
		req=(*iter);
		requested_inventory.pop_front();
		
		if( (req->request_time+INV_REQUEST_KICK)< tnow)
		{
			req->request_time=time(NULL);
			req->num_retries++;

			//LLViewerObject* obj = gObjectList.findObject(req->object->mID);
			LLViewerObject* obj = req->object;

			//cmdline_printchat("requesting inventory: " + obj->mID.asString());

			if(obj->mID.isNull())
				cmdline_printchat("no mID");

			if (!obj)
			{
				cmdline_printchat("export worker: " + req->object->mID.asString() + " not found in object list");
			}
			LLPCode pcode = obj->getPCode();
			if (pcode != LL_PCODE_VOLUME &&
				pcode != LL_PCODE_LEGACY_GRASS &&
				pcode != LL_PCODE_LEGACY_TREE)
			{
				cmdline_printchat("exportworker: object has invalid pcode");
				//return NULL;
			}
			obj->dirtyInventory();
			obj->requestInventory();
			
			//if (req->object->isInventoryDirty()) //yet another crashy inventory call
			//{
			//	req->object->fetchInventoryFromServer();
			//}

			//sInstance->removeVOInventoryListener();
			//sInstance->clearContents();
			//sInstance->registerVOInventoryListener(obj,NULL);
			//sInstance->requestVOInventory();


			kick_count++;
		}

		if (req->num_retries < 40)
			requested_inventory.push_back(req);
		else
		{
			req->object->mInventoryRecieved = true;

			//object->removeInventoryListener(sInstance);
			//obj->mInventoryRecieved=true;
			cmdline_printchat("failed to retrieve inventory for " + req->object->mID.asString());
			ExportTrackerFloater::inventories_received++;
		}

		
		if(kick_count>=20)
			break; // that will do for now				

		total--;
	}

	int count=0;

	//Keep the requested list topped up
	while(count < 20)
	{
		if(!ExportTrackerFloater::objectselection.empty())
		{
			LLViewerObject* obj=ExportTrackerFloater::objectselection.get(0);
			ExportTrackerFloater::objectselection.remove(0);

			if (!obj)
			{
				cmdline_printchat("export worker (getasyncdata): invalid obj");
				break;
			}

			ExportTrackerFloater::mObjectSelectionWaitList.push_back(obj);
			// We need to go off and get properties, inventorys and textures now
			getAsyncData(obj);
			//cmdline_printchat("getAsyncData for: " + llformat("%d",obj->getLocalID()));
			count++;

			//all child objects must also be active
			llassert_always(obj);

			LLViewerObject::child_list_t child_list = obj->getChildren();
			for (LLViewerObject::child_list_t::const_iterator i = child_list.begin(); i != child_list.end(); i++)
			{
				LLViewerObject* child = *i;
				getAsyncData(child);
				//cmdline_printchat("getAsyncData for: " + llformat("%d",child->getLocalID()));
			}
		}
		else
		{
			break;
		}
	}

	LLDynamicArray<LLViewerObject*>::iterator iter=ExportTrackerFloater::mObjectSelectionWaitList.begin();
	
	LLViewerObject* object = NULL;

	for(;iter!=ExportTrackerFloater::mObjectSelectionWaitList.end();iter++)
	{
		//Find an object that has completed export
		
		if(((export_properties==FALSE) || (*iter)->mPropertiesRecieved) && ((export_inventory==FALSE) || (*iter)->mInventoryRecieved))
		{
			//we have the root properties and inventory now check all children
			bool got_all_stuff=true;
			LLViewerObject::child_list_t child_list;
			llassert(child_list=(*iter)->getChildren());
			for (LLViewerObject::child_list_t::const_iterator i = child_list.begin(); i != child_list.end(); i++)
			{
				LLViewerObject* child = *i;
				if(!child->isAvatar())
				{
					if( ((export_properties==TRUE) && (!child->mPropertiesRecieved)) || ((export_inventory==TRUE) && !child->mInventoryRecieved))
					{
						got_all_stuff=false;
						getAsyncData(child);
						cmdline_printchat("Export stalled on object " + llformat("%d",child->getLocalID()));
						llwarns<<"Export stalled on object "<<child->getID()<<" local "<<child->getLocalID()<<llendl;
						break;
					}
				}
			}

			if(got_all_stuff==false)
				continue;

			object=(*iter);		
			LLVector3 object_pos = object->getPosition();
			LLSD origin;
			origin["ObjectPos"] = object_pos.getValue();
			
			//meh fix me, why copy the LLSD here, better to use just the pointer
			LLSD * plinkset = subserialize(object); 

			if(plinkset==NULL || (*plinkset).isUndefined())
			{
				cmdline_printchat("Object returned undefined linkset");				
			}
			else
			{
				processed_prims.push_back(plinkset);
			}
			
			ExportTrackerFloater::linksets_exported++;
			//Erase this entry from the wait list then BREAK, do not loop
			ExportTrackerFloater::mObjectSelectionWaitList.erase(iter);
			break;	
		}
	}
}

void JCExportTracker::finalize()
{
	//We convert our LLSD to HPA here.

	LLXMLNode *project_xml = new LLXMLNode("project", FALSE);
														
	project_xml->createChild("schema", FALSE)->setValue("1.0");
	project_xml->createChild("name", FALSE)->setValue(gDirUtilp->getBaseFileName(destination, false));
	project_xml->createChild("date", FALSE)->setValue(LLLogChat::timestamp(1));
	project_xml->createChild("software", FALSE)->setValue(llformat("%s %d.%d.%d.%d",
	LLAppViewer::instance()->getSecondLifeTitle().c_str(), LL_VERSION_MAJOR, LL_VERSION_MINOR, LL_VERSION_PATCH, LL_VERSION_BUILD));
	project_xml->createChild("platform", FALSE)->setValue("Second Life");
	std::vector<std::string> uris;
	LLViewerLogin* vl = LLViewerLogin::getInstance();
	std::string grid_uri = vl->getGridLabel(); //RC FIXME
	project_xml->createChild("grid", FALSE)->setValue(grid_uri);

	LLXMLNode *group_xml;
	group_xml = new LLXMLNode("group",FALSE);

	LLVector3 max = selection_center + selection_size / 2;
	LLVector3 min = selection_center - selection_size / 2;

	LLXMLNodePtr max_xml = group_xml->createChild("max", FALSE);
	max_xml->createChild("x", TRUE)->setValue(llformat("%.5f", max.mV[VX]));
	max_xml->createChild("y", TRUE)->setValue(llformat("%.5f", max.mV[VY]));
	max_xml->createChild("z", TRUE)->setValue(llformat("%.5f", max.mV[VZ]));

	LLXMLNodePtr min_xml = group_xml->createChild("min", FALSE);
	min_xml->createChild("x", TRUE)->setValue(llformat("%.5f", min.mV[VX]));
	min_xml->createChild("y", TRUE)->setValue(llformat("%.5f", min.mV[VY]));
	min_xml->createChild("z", TRUE)->setValue(llformat("%.5f", min.mV[VZ]));
											
	LLXMLNodePtr center_xml = group_xml->createChild("center", FALSE);
	center_xml->createChild("x", TRUE)->setValue(llformat("%.5f", selection_center.mV[VX]));
	center_xml->createChild("y", TRUE)->setValue(llformat("%.5f", selection_center.mV[VY]));
	center_xml->createChild("z", TRUE)->setValue(llformat("%.5f", selection_center.mV[VZ]));

	//cmdline_printchat("Attempting to output " + llformat("%u", data.size()) + " Objects.");

	std::list<LLSD *>::iterator iter=processed_prims.begin();
	for(;iter!=processed_prims.end();iter++)
	{	// for each object
			
		LLXMLNode *linkset_xml = new LLXMLNode("linkset", FALSE);
		LLSD *plsd=(*iter);
		//llwarns<<LLSD::dump(*plsd)<<llendl;
		for(LLSD::array_iterator link_itr = plsd->beginArray();
			link_itr != plsd->endArray();
			++link_itr)
		{ 
				LLSD prim = (*link_itr);
				
				std::string selected_item	= "box";
				F32 scale_x=1.f, scale_y=1.f;
			
				LLVolumeParams volume_params;
				volume_params.fromLLSD(prim["volume"]);
				// Volume type
				U8 path = volume_params.getPathParams().getCurveType();
				U8 profile_and_hole = volume_params.getProfileParams().getCurveType();
				U8 profile	= profile_and_hole & LL_PCODE_PROFILE_MASK;
				U8 hole		= profile_and_hole & LL_PCODE_HOLE_MASK;
				
				// Scale goes first so we can differentiate between a sphere and a torus,
				// which have the same profile and path types.
				// Scale
				scale_x = volume_params.getRatioX();
				scale_y = volume_params.getRatioY();
				BOOL linear_path = (path == LL_PCODE_PATH_LINE) || (path == LL_PCODE_PATH_FLEXIBLE);
				if ( linear_path && profile == LL_PCODE_PROFILE_CIRCLE )
				{
					selected_item = "cylinder";
				}
				else if ( linear_path && profile == LL_PCODE_PROFILE_SQUARE )
				{
					selected_item = "box";
				}
				else if ( linear_path && profile == LL_PCODE_PROFILE_ISOTRI )
				{
					selected_item = "prism";
				}
				else if ( linear_path && profile == LL_PCODE_PROFILE_EQUALTRI )
				{
					selected_item = "prism";
				}
				else if ( linear_path && profile == LL_PCODE_PROFILE_RIGHTTRI )
				{
					selected_item = "prism";
				}
				else if (path == LL_PCODE_PATH_FLEXIBLE) // shouldn't happen
				{
					selected_item = "cylinder"; // reasonable default
				}
				else if ( path == LL_PCODE_PATH_CIRCLE && profile == LL_PCODE_PROFILE_CIRCLE && scale_y > 0.75f)
				{
					selected_item = "sphere";
				}
				else if ( path == LL_PCODE_PATH_CIRCLE && profile == LL_PCODE_PROFILE_CIRCLE && scale_y <= 0.75f)
				{
					selected_item = "torus";
				}
				else if ( path == LL_PCODE_PATH_CIRCLE && profile == LL_PCODE_PROFILE_CIRCLE_HALF)
				{
					selected_item = "sphere";
				}
				else if ( path == LL_PCODE_PATH_CIRCLE2 && profile == LL_PCODE_PROFILE_CIRCLE )
				{
					// Spirals aren't supported.  Make it into a sphere.  JC
					selected_item = "sphere";
				}
				else if ( path == LL_PCODE_PATH_CIRCLE && profile == LL_PCODE_PROFILE_EQUALTRI )
				{
					selected_item = "ring";
				}
				else if ( path == LL_PCODE_PATH_CIRCLE && profile == LL_PCODE_PROFILE_SQUARE && scale_y <= 0.75f)
				{
					selected_item = "tube";
				}
				else
				{
					llinfos << "Unknown path " << (S32) path << " profile " << (S32) profile << " in getState" << llendl;
					selected_item = "box";
				}
				// Create an LLSD object that represents this prim. It will be injected in to the overall LLSD
				// tree structure
				LLXMLNode *prim_xml;
				LLPCode pcode = prim["pcode"].asInteger();
				// Sculpt
				if (prim.has("sculpt"))
					prim_xml = new LLXMLNode("sculpt", FALSE);
				else if (pcode == LL_PCODE_LEGACY_GRASS)
				{
					prim_xml = new LLXMLNode("grass", FALSE);
					LLXMLNodePtr shadow_xml = prim_xml->createChild("type", FALSE);
					shadow_xml->createChild("val", TRUE)->setValue(prim["state"]);
				}
				else if (pcode == LL_PCODE_LEGACY_TREE)
				{
					prim_xml = new LLXMLNode("tree", FALSE);
					LLXMLNodePtr shadow_xml = prim_xml->createChild("type", FALSE);
					shadow_xml->createChild("val", TRUE)->setValue(prim["state"]);
				}
				else
					prim_xml = new LLXMLNode(selected_item.c_str(), FALSE);
				
				//Properties
				
				LLSD * props=received_properties[prim["id"]];

				if(props!=NULL)
				{
					prim_xml->createChild("name", FALSE)->setValue("<![CDATA[" + std::string((*props)["name"]) + "]]>");
					prim_xml->createChild("description", FALSE)->setValue("<![CDATA[" + std::string((*props)["description"]) + "]]>");
					
					//All done with properties?
					free(props);
					received_properties.erase(prim["id"]);
				}
				
				// Transforms		
				LLXMLNodePtr position_xml = prim_xml->createChild("position", FALSE);
				LLVector3 position;
				position.setVec(ll_vector3d_from_sd(prim["position"]));
				position_xml->createChild("x", TRUE)->setValue(llformat("%.5f", position.mV[VX]));
				position_xml->createChild("y", TRUE)->setValue(llformat("%.5f", position.mV[VY]));
				position_xml->createChild("z", TRUE)->setValue(llformat("%.5f", position.mV[VZ]));
				LLXMLNodePtr scale_xml = prim_xml->createChild("size", FALSE);
				LLVector3 scale = ll_vector3_from_sd(prim["scale"]);
				scale_xml->createChild("x", TRUE)->setValue(llformat("%.5f", scale.mV[VX]));
				scale_xml->createChild("y", TRUE)->setValue(llformat("%.5f", scale.mV[VY]));
				scale_xml->createChild("z", TRUE)->setValue(llformat("%.5f", scale.mV[VZ]));
				LLXMLNodePtr rotation_xml = prim_xml->createChild("rotation", FALSE);
				LLQuaternion rotation = ll_quaternion_from_sd(prim["rotation"]);
				rotation_xml->createChild("x", TRUE)->setValue(llformat("%.5f", rotation.mQ[VX]));
				rotation_xml->createChild("y", TRUE)->setValue(llformat("%.5f", rotation.mQ[VY]));
				rotation_xml->createChild("z", TRUE)->setValue(llformat("%.5f", rotation.mQ[VZ]));
				rotation_xml->createChild("w", TRUE)->setValue(llformat("%.5f", rotation.mQ[VW]));
				// Flags
				if(prim["phantom"].asBoolean())
				{
					LLXMLNodePtr shadow_xml = prim_xml->createChild("phantom", FALSE);
					shadow_xml->createChild("val", TRUE)->setValue("true");
				}
				if(prim["physical"].asBoolean())
				{
					LLXMLNodePtr shadow_xml = prim_xml->createChild("physical", FALSE);
					shadow_xml->createChild("val", TRUE)->setValue("true");
				}
				
				// Grab S path
				F32 begin_s	= volume_params.getBeginS();
				F32 end_s	= volume_params.getEndS();
				// Compute cut and advanced cut from S and T
				F32 begin_t = volume_params.getBeginT();
				F32 end_t	= volume_params.getEndT();
				// Hollowness
				F32 hollow = volume_params.getHollow();
				// Twist
				F32 twist		= volume_params.getTwist() * 180.0;
				F32 twist_begin = volume_params.getTwistBegin() * 180.0;
				// Cut interpretation varies based on base object type
				F32 cut_begin, cut_end, adv_cut_begin, adv_cut_end;
				if ( selected_item == "sphere" || selected_item == "torus" || 
					 selected_item == "tube"   || selected_item == "ring" )
				{
					cut_begin		= begin_t;
					cut_end			= end_t;
					adv_cut_begin	= begin_s;
					adv_cut_end		= end_s;
				}
				else
				{
					cut_begin       = begin_s;
					cut_end         = end_s;
					adv_cut_begin   = begin_t;
					adv_cut_end     = end_t;
				}
				if (selected_item != "sphere")
				{		
					// Shear
					//<top_shear x="0" y="0" />
					F32 shear_x = volume_params.getShearX();
					F32 shear_y = volume_params.getShearY();
					LLXMLNodePtr shear_xml = prim_xml->createChild("top_shear", FALSE);
					shear_xml->createChild("x", TRUE)->setValue(llformat("%.5f", shear_x));
					shear_xml->createChild("y", TRUE)->setValue(llformat("%.5f", shear_y));
				}
				else
				{	
					// Dimple
					//<dimple begin="0.0" end="1.0" />
					LLXMLNodePtr shear_xml = prim_xml->createChild("dimple", FALSE);
					shear_xml->createChild("begin", TRUE)->setValue(llformat("%.5f", adv_cut_begin));
					shear_xml->createChild("end", TRUE)->setValue(llformat("%.5f", adv_cut_end));
				}

				if (selected_item == "box" || selected_item == "cylinder" || selected_item == "prism")
				{
					// Taper
					//<taper x="0" y="0" />
					F32 taper_x = 1.f - volume_params.getRatioX();
					F32 taper_y = 1.f - volume_params.getRatioY();
					LLXMLNodePtr taper_xml = prim_xml->createChild("taper", FALSE);
					taper_xml->createChild("x", TRUE)->setValue(llformat("%.5f", taper_x));
					taper_xml->createChild("y", TRUE)->setValue(llformat("%.5f", taper_y));
				}
				else if (selected_item == "torus" || selected_item == "tube" || selected_item == "ring")
				{
					// Taper
					//<taper x="0" y="0" />
					F32 taper_x	= volume_params.getTaperX();
					F32 taper_y = volume_params.getTaperY();
					LLXMLNodePtr taper_xml = prim_xml->createChild("taper", FALSE);
					taper_xml->createChild("x", TRUE)->setValue(llformat("%.5f", taper_x));
					taper_xml->createChild("y", TRUE)->setValue(llformat("%.5f", taper_y));
					//Hole Size
					//<hole_size x="0.2" y="0.35" />
					F32 hole_size_x = volume_params.getRatioX();
					F32 hole_size_y = volume_params.getRatioY();
					LLXMLNodePtr hole_size_xml = prim_xml->createChild("hole_size", FALSE);
					hole_size_xml->createChild("x", TRUE)->setValue(llformat("%.5f", hole_size_x));
					hole_size_xml->createChild("y", TRUE)->setValue(llformat("%.5f", hole_size_y));
					//Advanced cut
					//<profile_cut begin="0" end="1" />
					LLXMLNodePtr profile_cut_xml = prim_xml->createChild("profile_cut", FALSE);
					profile_cut_xml->createChild("begin", TRUE)->setValue(llformat("%.5f", adv_cut_begin));
					profile_cut_xml->createChild("end", TRUE)->setValue(llformat("%.5f", adv_cut_end));
					//Skew
					//<skew val="0.0" />
					F32 skew = volume_params.getSkew();
					LLXMLNodePtr skew_xml = prim_xml->createChild("skew", FALSE);
					skew_xml->createChild("val", TRUE)->setValue(llformat("%.5f", skew));
					//Radius offset
					//<radius_offset val="0.0" />
					F32 radius_offset = volume_params.getRadiusOffset();
					LLXMLNodePtr radius_offset_xml = prim_xml->createChild("radius_offset", FALSE);
					radius_offset_xml->createChild("val", TRUE)->setValue(llformat("%.5f", radius_offset));
					// Revolutions
					//<revolutions val="1.0" />
					F32 revolutions = volume_params.getRevolutions();
					LLXMLNodePtr revolutions_xml = prim_xml->createChild("revolutions", FALSE);
					revolutions_xml->createChild("val", TRUE)->setValue(llformat("%.5f", revolutions));
				}
				//<path_cut begin="0" end="1" />
				LLXMLNodePtr path_cut_xml = prim_xml->createChild("path_cut", FALSE);
				path_cut_xml->createChild("begin", TRUE)->setValue(llformat("%.5f", cut_begin));
				path_cut_xml->createChild("end", TRUE)->setValue(llformat("%.5f", cut_end));
				//<twist begin="0" end="0" />
				LLXMLNodePtr twist_xml = prim_xml->createChild("twist", FALSE);
				twist_xml->createChild("begin", TRUE)->setValue(llformat("%.5f", twist_begin));
				twist_xml->createChild("end", TRUE)->setValue(llformat("%.5f", twist));
				// All hollow objects allow a shape to be selected.
				if (hollow > 0.f)
				{
					const char	*selected_hole	= "1";
					switch (hole)
					{
					case LL_PCODE_HOLE_CIRCLE:
						selected_hole = "3";
						break;
					case LL_PCODE_HOLE_SQUARE:
						selected_hole = "2";
						break;
					case LL_PCODE_HOLE_TRIANGLE:
						selected_hole = "4";
						break;
					case LL_PCODE_HOLE_SAME:
					default:
						selected_hole = "1";
						break;
					}
					//<hollow amount="0" shape="1" />
					LLXMLNodePtr hollow_xml = prim_xml->createChild("hollow", FALSE);
					hollow_xml->createChild("amount", TRUE)->setValue(llformat("%.5f", hollow * 100.0));
					hollow_xml->createChild("shape", TRUE)->setValue(llformat("%s", selected_hole));
				}
				// Extra params
				// Flexible
				if(prim.has("flexible"))
				{
					LLFlexibleObjectData attributes;
					attributes.fromLLSD(prim["flexible"]);
					//<flexible>
					LLXMLNodePtr flex_xml = prim_xml->createChild("flexible", FALSE);
					//<softness val="2.0">
					LLXMLNodePtr softness_xml = flex_xml->createChild("softness", FALSE);
					softness_xml->createChild("val", TRUE)->setValue(llformat("%.5f", (F32)attributes.getSimulateLOD()));
					//<gravity val="0.3">
					LLXMLNodePtr gravity_xml = flex_xml->createChild("gravity", FALSE);
					gravity_xml->createChild("val", TRUE)->setValue(llformat("%.5f", attributes.getGravity()));
					//<drag val="2.0">
					LLXMLNodePtr drag_xml = flex_xml->createChild("drag", FALSE);
					drag_xml->createChild("val", TRUE)->setValue(llformat("%.5f", attributes.getAirFriction()));
					//<wind val="0.0">
					LLXMLNodePtr wind_xml = flex_xml->createChild("wind", FALSE);
					wind_xml->createChild("val", TRUE)->setValue(llformat("%.5f", attributes.getWindSensitivity()));
					//<tension val="1.0">
					LLXMLNodePtr tension_xml = flex_xml->createChild("tension", FALSE);
					tension_xml->createChild("val", TRUE)->setValue(llformat("%.5f", attributes.getTension()));
					//<force x="0.0" y="0.0" z="0.0" />
					LLXMLNodePtr force_xml = flex_xml->createChild("force", FALSE);
					force_xml->createChild("x", TRUE)->setValue(llformat("%.5f", attributes.getUserForce().mV[VX]));
					force_xml->createChild("y", TRUE)->setValue(llformat("%.5f", attributes.getUserForce().mV[VY]));
					force_xml->createChild("z", TRUE)->setValue(llformat("%.5f", attributes.getUserForce().mV[VZ]));
				}
				
				// Light
				if (prim.has("light"))
				{
					LLLightParams light;
					light.fromLLSD(prim["light"]);
					//<light>
					LLXMLNodePtr light_xml = prim_xml->createChild("light", FALSE);
					//<color r="255" g="255" b="255" />
					LLXMLNodePtr color_xml = light_xml->createChild("color", FALSE);
					LLColor4 color = light.getColor();
					color_xml->createChild("r", TRUE)->setValue(llformat("%u", (U32)(color.mV[VRED] * 255)));
					color_xml->createChild("g", TRUE)->setValue(llformat("%u", (U32)(color.mV[VGREEN] * 255)));
					color_xml->createChild("b", TRUE)->setValue(llformat("%u", (U32)(color.mV[VBLUE] * 255)));
					//<intensity val="1.0" />
					LLXMLNodePtr intensity_xml = light_xml->createChild("intensity", FALSE);
					intensity_xml->createChild("val", TRUE)->setValue(llformat("%.5f", color.mV[VALPHA]));
					//<radius val="10.0" />
					LLXMLNodePtr radius_xml = light_xml->createChild("radius", FALSE);
					radius_xml->createChild("val", TRUE)->setValue(llformat("%.5f", light.getRadius()));
					//<falloff val="0.75" />
					LLXMLNodePtr falloff_xml = light_xml->createChild("falloff", FALSE);
					falloff_xml->createChild("val", TRUE)->setValue(llformat("%.5f", light.getFalloff()));
				}
				// Sculpt
				if (prim.has("sculpt"))
				{
					LLSculptParams sculpt;
					sculpt.fromLLSD(prim["sculpt"]);
					
					//<topology val="4" />
					LLXMLNodePtr topology_xml = prim_xml->createChild("topology", FALSE);
					topology_xml->createChild("val", TRUE)->setValue(llformat("%u", sculpt.getSculptType()));
					
					//<sculptmap_uuid>1e366544-c287-4fff-ba3e-5fafdba10272</sculptmap_uuid>
					//<sculptmap_file>apple_map.tga</sculptmap_file>
					//FIXME png/tga/j2c selection itt.
					std::string sculpttexture;
					sculpt.getSculptTexture().toString(sculpttexture);
					prim_xml->createChild("sculptmap_file", FALSE)->setValue(sculpttexture+".tga");
					prim_xml->createChild("sculptmap_uuid", FALSE)->setValue(sculpttexture);
				}
				//<texture>
				LLXMLNodePtr texture_xml = prim_xml->createChild("texture", FALSE);
				// Textures
				LLSD te_llsd;
				LLSD tes = prim["textures"];
				LLPrimitive object;
				object.setNumTEs(U8(tes.size()));
				
				for (int i = 0; i < tes.size(); i++)
				{
					LLTextureEntry tex;
					tex.fromLLSD(tes[i]);
					object.setTE(U8(i), tex);
				}
			
				//U8 te_count = object->getNumTEs();
				//for (U8 i = 0; i < te_count; i++)
				//{
				//for each texture
				for (int i = 0; i < tes.size(); i++)
				{
					LLTextureEntry tex;
					tex.fromLLSD(tes[i]);
					//bool alreadyseen=false;
					//te_llsd.append(object->getTE(i)->asLLSD());
					std::list<LLUUID>::iterator iter;
					
					/* this loop keeps track of seen textures, replace with
					emerald version.
					for(iter = textures.begin(); iter != textures.end() ; iter++) 
					{
						if( (*iter)==object->getTE(i)->getID())
							alreadyseen=true;
					}
		*/
					//<face id=0>
					LLXMLNodePtr face_xml = texture_xml->createChild("face", FALSE);
					//This may be a hack, but it's ok since we're not using id in this code. We set id differently because for whatever reason
					//llxmlnode filters a few parameters including ID. -Patrick Sapinski (Friday, September 25, 2009)
					face_xml->mID = llformat("%d", i);
					//<tile u="-1" v="1" />
					//object->getTE(face)->mScaleS
					//object->getTE(face)->mScaleT
					LLXMLNodePtr tile_xml = face_xml->createChild("tile", FALSE);
					tile_xml->createChild("u", TRUE)->setValue(llformat("%.5f", object.getTE(i)->mScaleS));
					tile_xml->createChild("v", TRUE)->setValue(llformat("%.5f", object.getTE(i)->mScaleT));
					//<offset u="0" v="0" />
					//object->getTE(face)->mOffsetS
					//object->getTE(face)->mOffsetT
					LLXMLNodePtr offset_xml = face_xml->createChild("offset", FALSE);
					offset_xml->createChild("u", TRUE)->setValue(llformat("%.5f", object.getTE(i)->mOffsetS));
					offset_xml->createChild("v", TRUE)->setValue(llformat("%.5f", object.getTE(i)->mOffsetT));
					//<rotation w="0" />
					//object->getTE(face)->mRotation
					LLXMLNodePtr rotation_xml = face_xml->createChild("rotation", FALSE);
					rotation_xml->createChild("w", TRUE)->setValue(llformat("%.5f", (object.getTE(i)->mRotation * RAD_TO_DEG)));
					//<image_file><![CDATA[76a0319a-e963-44b0-9f25-127815226d71.tga]]></image_file>
					//<image_uuid>76a0319a-e963-44b0-9f25-127815226d71</image_uuid>
					LLUUID texture = object.getTE(i)->getID();
					std::string uuid_string;
					object.getTE(i)->getID().toString(uuid_string);
					
					face_xml->createChild("image_file", FALSE)->setValue("<![CDATA[" + uuid_string + ".tga]]>");
					face_xml->createChild("image_uuid", FALSE)->setValue(uuid_string);
					//<color r="255" g="255" b="255" />
					LLXMLNodePtr color_xml = face_xml->createChild("color", FALSE);
					LLColor4 color = object.getTE(i)->getColor();
					color_xml->createChild("r", TRUE)->setValue(llformat("%u", (int)(color.mV[VRED] * 255.f)));
					color_xml->createChild("g", TRUE)->setValue(llformat("%u", (int)(color.mV[VGREEN] * 255.f)));
					color_xml->createChild("b", TRUE)->setValue(llformat("%u", (int)(color.mV[VBLUE] * 255.f)));
					//<transparency val="0" />
					LLXMLNodePtr transparency_xml = face_xml->createChild("transparency", FALSE);
					transparency_xml->createChild("val", TRUE)->setValue(llformat("%u", (int)((1.f - color.mV[VALPHA]) * 100.f)));
					//<glow val="0" />
					//object->getTE(face)->getGlow()
					LLXMLNodePtr glow_xml = face_xml->createChild("glow", FALSE);
					glow_xml->createChild("val", TRUE)->setValue(llformat("%.5f", object.getTE(i)->getGlow()));
					//HACK! primcomposer chokes if we have fullbright but don't specify shine+bump.
					//<fullbright val="false" />
					//<shine val="0" />
					//<bump val="0" />
					if(object.getTE(i)->getFullbright() || object.getTE(i)->getShiny() || object.getTE(i)->getBumpmap())
					{
						std::string temp = "false";
						if(object.getTE(i)->getFullbright())
							temp = "true";
						LLXMLNodePtr fullbright_xml = face_xml->createChild("fullbright", FALSE);
						fullbright_xml->createChild("val", TRUE)->setValue(temp);
						LLXMLNodePtr shine_xml = face_xml->createChild("shine", FALSE);
						shine_xml->createChild("val", TRUE)->setValue(llformat("%u",object.getTE(i)->getShiny()));
						LLXMLNodePtr bumpmap_xml = face_xml->createChild("bump", FALSE);
						bumpmap_xml->createChild("val", TRUE)->setValue(llformat("%u",object.getTE(i)->getBumpmap()));
					}
						
					//<mapping val="0" />
				} // end for each texture
				//<inventory>

				
				LLXMLNodePtr inventory_xml = prim_xml->createChild("inventory", FALSE);
				//LLSD inventory = prim["inventory"];
				LLSD * inventory = received_inventory[prim["id"]];

				if(inventory !=NULL)
				{
					//for each inventory item
					for (LLSD::array_iterator inv = (*inventory).beginArray(); inv != (*inventory).endArray(); ++inv)
					{
						LLSD item = (*inv);
						//<item>
						LLXMLNodePtr field_xml = inventory_xml->createChild("item", FALSE);
						   //<description>2008-01-29 05:01:19 note card</description>
						field_xml->createChild("description", FALSE)->setValue(item["desc"].asString());
						   //<item_id>673b00e8-990f-3078-9156-c7f7b4a5f86c</item_id>
						field_xml->createChild("item_id", FALSE)->setValue(item["item_id"].asString());
						   //<name>blah blah</name>
						field_xml->createChild("name", FALSE)->setValue(item["name"].asString());
						   //<type>notecard</type>
						field_xml->createChild("type", FALSE)->setValue(item["type"].asString());
					} // end for each inventory item
					//add this prim to the linkset.

					delete(inventory);
					received_inventory.erase(LLUUID(prim["id"].asString()));

				}
				linkset_xml->addChild(prim_xml);
			} //end for each object
			//add this linkset to the group.
			group_xml->addChild(linkset_xml);
			delete plsd;
		} //end for each linkset.

		// Create a file stream and write to it
		llofstream out(destination,ios_base::out);
		if (!out.good())
		{
			llwarns << "Unable to open for output." << llendl;
		}
		else
		{
			out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
			project_xml->addChild(group_xml);
			project_xml->writeToOstream(out);
			out.close();
		}

		/* this code gzips the archive, we want to zip it though!
		std::string gzip_filename(destination);
		gzip_filename.append(".gz");
		if(gzip_file(destination, gzip_filename))
		{
			lldebugs << "Successfully compressed " << destination << llendl;
			//LLFile::remove(inventory_filename);
		}
		else
		{
			llwarns << "Unable to compress " << destination << llendl;
		}
		*/



/*	we can still output an LLSD with this but it's no longer emerald compatible because 
	we switched to region relative positions. useful for testing. */

//	LLSD file;
//	LLSD header;
//	header["Version"] = 2;
//	file["Header"] = header;
	//std::vector<std::string> uris;
	//	LLViewerLogin* vl = LLViewerLogin::getInstance();
	//	std::string grid_uri = vl->getGridLabel(); //RC FIXME
	//LLStringUtil::toLower(uris[0]);
//	file["Grid"] = grid_uri;
//	file["Objects"] = data;

	// Create a file stream and write to it
//	llofstream export_file(destination + ".llsd",std::ios_base::app | std::ios_base::out);
//	LLSDSerialize::toPrettyXML(file, export_file);
	// Open the file save dialog
//	export_file.close();

	processed_prims.clear();
	received_inventory.clear();
	ExportTrackerFloater::getInstance()->childSetEnabled("export",true);
	status = IDLE;
}

void JCExportTracker::processObjectProperties(LLMessageSystem* msg, void** user_data)
{
	if(!export_properties)
	{
		return;
	}
	if(status == IDLE)
	{
		return;
	}
	S32 i;
	S32 count = msg->getNumberOfBlocksFast(_PREHASH_ObjectData);
	for (i = 0; i < count; i++)
	{
		LLUUID id;
		msg->getUUIDFast(_PREHASH_ObjectData, _PREHASH_ObjectID, id, i);

		//Remove this request from the list of requests
		std::list<PropertiesRequest_t*>::iterator iter;
		for (iter=requested_properties.begin(); iter != requested_properties.end(); iter++)
		{
			PropertiesRequest_t * req;
			req=(*iter);
			if(id==req->target_prim)
			{
				free(req);
				requested_properties.erase(iter);
				ExportTrackerFloater::properties_received++;
				break;
			}
		}

		if(iter==requested_properties.end())
			return;
			
		LLUUID creator_id;
		LLUUID owner_id;
		LLUUID group_id;
		LLUUID last_owner_id;
		U64 creation_date;
		LLUUID extra_id;
		U32 base_mask, owner_mask, group_mask, everyone_mask, next_owner_mask;
		LLSaleInfo sale_info;
		LLCategory category;
		LLAggregatePermissions ag_perms;
		LLAggregatePermissions ag_texture_perms;
		LLAggregatePermissions ag_texture_perms_owner;
								
		msg->getUUIDFast(_PREHASH_ObjectData, _PREHASH_CreatorID, creator_id, i);
		msg->getUUIDFast(_PREHASH_ObjectData, _PREHASH_OwnerID, owner_id, i);
		msg->getUUIDFast(_PREHASH_ObjectData, _PREHASH_GroupID, group_id, i);
		msg->getU64Fast(_PREHASH_ObjectData, _PREHASH_CreationDate, creation_date, i);
		msg->getU32Fast(_PREHASH_ObjectData, _PREHASH_BaseMask, base_mask, i);
		msg->getU32Fast(_PREHASH_ObjectData, _PREHASH_OwnerMask, owner_mask, i);
		msg->getU32Fast(_PREHASH_ObjectData, _PREHASH_GroupMask, group_mask, i);
		msg->getU32Fast(_PREHASH_ObjectData, _PREHASH_EveryoneMask, everyone_mask, i);
		msg->getU32Fast(_PREHASH_ObjectData, _PREHASH_NextOwnerMask, next_owner_mask, i);
		sale_info.unpackMultiMessage(msg, _PREHASH_ObjectData, i);

		ag_perms.unpackMessage(msg, _PREHASH_ObjectData, _PREHASH_AggregatePerms, i);
		ag_texture_perms.unpackMessage(msg, _PREHASH_ObjectData, _PREHASH_AggregatePermTextures, i);
		ag_texture_perms_owner.unpackMessage(msg, _PREHASH_ObjectData, _PREHASH_AggregatePermTexturesOwner, i);
		category.unpackMultiMessage(msg, _PREHASH_ObjectData, i);

		S16 inv_serial = 0;
		msg->getS16Fast(_PREHASH_ObjectData, _PREHASH_InventorySerial, inv_serial, i);

		LLUUID item_id;
		msg->getUUIDFast(_PREHASH_ObjectData, _PREHASH_ItemID, item_id, i);
		LLUUID folder_id;
		msg->getUUIDFast(_PREHASH_ObjectData, _PREHASH_FolderID, folder_id, i);
		LLUUID from_task_id;
		msg->getUUIDFast(_PREHASH_ObjectData, _PREHASH_FromTaskID, from_task_id, i);

		msg->getUUIDFast(_PREHASH_ObjectData, _PREHASH_LastOwnerID, last_owner_id, i);

		std::string name;
		msg->getStringFast(_PREHASH_ObjectData, _PREHASH_Name, name, i);
		std::string desc;
		msg->getStringFast(_PREHASH_ObjectData, _PREHASH_Description, desc, i);

		std::string touch_name;
		msg->getStringFast(_PREHASH_ObjectData, _PREHASH_TouchName, touch_name, i);
		std::string sit_name;
		msg->getStringFast(_PREHASH_ObjectData, _PREHASH_SitName, sit_name, i);

		//unpack TE IDs
		std::vector<LLUUID> texture_ids;
		S32 size = msg->getSizeFast(_PREHASH_ObjectData, i, _PREHASH_TextureID);
		if (size > 0)
		{
			S8 packed_buffer[SELECT_MAX_TES * UUID_BYTES];
			msg->getBinaryDataFast(_PREHASH_ObjectData, _PREHASH_TextureID, packed_buffer, 0, i, SELECT_MAX_TES * UUID_BYTES);

			for (S32 buf_offset = 0; buf_offset < size; buf_offset += UUID_BYTES)
			{
				LLUUID tid;
				memcpy(tid.mData, packed_buffer + buf_offset, UUID_BYTES);		/* Flawfinder: ignore */
				texture_ids.push_back(tid);
			}
		}

		LLSD * props=new LLSD();

		(*props)["creator_id"] = creator_id.asString();
		(*props)["owner_id"] = owner_id.asString();
		(*props)["group_id"] = group_id.asString();
		(*props)["last_owner_id"] = last_owner_id.asString();
		(*props)["creation_date"] = llformat("%d",creation_date);
		(*props)["extra_id"] = extra_id.asString();
		(*props)["base_mask"] = llformat("%d",base_mask);
		(*props)["owner_mask"] = llformat("%d",owner_mask);
		(*props)["group_mask"] = llformat("%d",group_mask);
		(*props)["everyone_mask"] = llformat("%d",everyone_mask);
		(*props)["next_owner_mask"] = llformat("%d",next_owner_mask);
		(*props)["sale_info"] = sale_info.asLLSD();

		(*props)["inv_serial"] = (S32)inv_serial;
		(*props)["item_id"] = item_id.asString();
		(*props)["folder_id"] = folder_id.asString();
		(*props)["from_task_id"] = from_task_id.asString();
		(*props)["name"] = name;
		(*props)["description"] = desc;
		(*props)["touch_name"] = touch_name;
		(*props)["sit_name"] = sit_name;

		received_properties[id]=props;

		LLViewerObject *obj = gObjectList.findObject(id);
		if(obj)
		{
			obj->mPropertiesRecieved=true;
		}
		else
		{
			llwarns << "Failed to find object "<<id<< " and mark as properties recieved, wtf?"<<llendl;
		}
	}
}

BOOL couldDL(LLAssetType::EType type)
{
	switch(type)
	{//things we could plausibly DL anyway atm
	case LLAssetType::AT_TEXTURE:
	case LLAssetType::AT_SCRIPT:
	case LLAssetType::AT_CLOTHING:
	case LLAssetType::AT_NOTECARD:
	case LLAssetType::AT_LSL_TEXT:
	case LLAssetType::AT_TEXTURE_TGA:
	case LLAssetType::AT_BODYPART:
	case LLAssetType::AT_GESTURE:
		return TRUE;
		break;
	default:
		return FALSE;
		break;
	}
}

void JCExportTracker::inventoryChanged(LLViewerObject* obj,
								 InventoryObjectList* inv,
								 S32 serial_num,
								 void* user_data)
{
	if(!export_inventory)
	{
		obj->removeInventoryListener(sInstance);
		return;
	}
	if(status == IDLE)
	{
		obj->removeInventoryListener(sInstance);
		return;
	}

	if(requested_inventory.empty())
		return;

	std::list<InventoryRequest_t*>::iterator iter=requested_inventory.begin();
	for(;iter!=requested_inventory.end();iter++)
	{
		if((*iter)->object->getID()==obj->getID())
		{
			//cmdline_printchat("downloaded inventory for "+obj->getID().asString());
			LLSD * inventory = new LLSD();
			//lol lol lol lol lol
			InventoryObjectList::const_iterator it = inv->begin();
			InventoryObjectList::const_iterator end = inv->end();
			U32 num = 0;
			for( ;	it != end;	++it)
			{
				LLInventoryObject* asset = (*it);
				if(asset)
				{
					LLInventoryItem* item = (LLInventoryItem*)((LLInventoryObject*)(*it));
					LLViewerInventoryItem* new_item = (LLViewerInventoryItem*)item;
					new_item; //ugh
					LLPermissions perm;
					llassert(perm = new_item->getPermissions());
					if(couldDL(asset->getType())
						&& perm.allowCopyBy(gAgent.getID())
						&& perm.allowModifyBy(gAgent.getID())
						&& perm.allowTransferTo(LLUUID::null))// && is_asset_id_knowable(asset->getType()))
					{
						LLSD inv_item;
						inv_item["name"] = asset->getName();
						inv_item["type"] = LLAssetType::lookup(asset->getType());
						//cmdline_printchat("requesting asset "+asset->getName());
						inv_item["desc"] = ((LLInventoryItem*)((LLInventoryObject*)(*it)))->getDescription();//god help us all
						inv_item["item_id"] = asset->getUUID().asString();
						if(!LLFile::isdir(asset_dir+gDirUtilp->getDirDelimiter()+"inventory"))
						{
							LLFile::mkdir(asset_dir+gDirUtilp->getDirDelimiter()+"inventory");
						}

						JCExportTracker::mirror(asset, obj, asset_dir+gDirUtilp->getDirDelimiter()+"inventory", asset->getUUID().asString());//loltest
						//unacceptable
						(*inventory)[num] = inv_item;
						num += 1;
						ExportTrackerFloater::total_assets++;
					}
				}
			}

			//MEH
			//(*link_itr)["inventory"] = inventory;
			received_inventory[obj->getID()]=inventory;

			//cmdline_printchat(llformat("%d inv queries left",requested_inventory.size()));

			obj->removeInventoryListener(sInstance);
			obj->mInventoryRecieved=true;

			requested_inventory.erase(iter);
			ExportTrackerFloater::inventories_received++;
			break;
		}
	}
		// we should not get here and hopefully checking requested_inventory contained this
		// object was unnecessary.

/*

	{
		for(LLSD::array_iterator array_itr = data.beginArray();
			array_itr != data.endArray();
			++array_itr)
		{
			if((*array_itr).has("Object"))
			{
				////cmdline_printchat("has object entry?");
				LLSD linkset_llsd = (*array_itr)["Object"];
				for(LLSD::array_iterator link_itr = linkset_llsd.beginArray();
					link_itr != linkset_llsd.endArray();
					++link_itr)
				{
					if((*link_itr) && (*link_itr).has("id"))
					{
						////cmdline_printchat("lol, has ID");
						if((*link_itr)["id"].asString() == obj->getID().asString())
						{
							if(!((*link_itr).has("inventory")))
							{
							}
						}
					}
				}
				(*array_itr)["Object"] = linkset_llsd;
			}
		}


	}
	*/

}

void JCAssetExportCallback(LLVFS *vfs, const LLUUID& uuid, LLAssetType::EType type, void *userdata, S32 result, LLExtStat extstat)
{
	JCAssetInfo* info = (JCAssetInfo*)userdata;
	if(result == LL_ERR_NOERR)
	{
		//cmdline_printchat("Saved file "+info->path+" ("+info->name+")");
		S32 size = vfs->getSize(uuid, type);
		U8* buffer = new U8[size];
		vfs->getData(uuid, type, buffer, 0, size);

		if(type == LLAssetType::AT_NOTECARD)
		{
			LLViewerTextEditor* edit = new LLViewerTextEditor("",LLRect(0,0,0,0),S32_MAX,"");
			if(edit->importBuffer((char*)buffer, (S32)size))
			{
				//cmdline_printchat("Decoded notecard");
				std::string card = edit->getText();
				//delete edit;
				edit->die();
				delete buffer;
				buffer = new U8[card.size()];
				size = card.size();
				strcpy((char*)buffer,card.c_str());
			}else cmdline_printchat("Failed to decode notecard");
		}
		LLAPRFile infile;
		infile.open(info->path.c_str(), LL_APR_WB,LLAPRFile::global);
		apr_file_t *fp = infile.getFileHandle();
		if(fp)infile.write(buffer, size);
		infile.close();

		ExportTrackerFloater::assets_exported++;

		//mark as downloaded in inventory list
		LLScrollListCtrl* mResultList;
		mResultList = ExportTrackerFloater::sInstance->getChild<LLScrollListCtrl>("inventory_result_list");

		if (mResultList->getItemIndex(info->assetid) >= 0)
		{
			LLScrollListItem* itemp = mResultList->getItem(info->assetid);
			itemp->getColumn(4)->setValue("account_id_green.tga");

			//LLSD temp = mResultList->getColumn(mResultList->getItemIndex(info->assetid))->getValue();
			//mResultList->deleteSingleItem(mResultList->getItemIndex(info->assetid));
			//mResultList->addElement(element, ADD_BOTTOM);

		}
		else cmdline_printchat("received unrequested asset");
		

		//delete buffer;
	} else cmdline_printchat("Failed to save file "+info->path+" ("+info->name+") : "+std::string(LLAssetStorage::getErrorString(result)));

	delete info;
}

BOOL JCExportTracker::mirror(LLInventoryObject* item, LLViewerObject* container, std::string root, std::string iname)
{
	if(item)
	{
		////cmdline_printchat("item");
		//LLUUID asset_id = item->getAssetUUID();
		//if(asset_id.notNull())
		LLPermissions perm(((LLInventoryItem*)item)->getPermissions());
		if(perm.allowCopyBy(gAgent.getID())
		&& perm.allowModifyBy(gAgent.getID())
		&& perm.allowTransferTo(LLUUID::null))
		{
			////cmdline_printchat("asset_id.notNull()");
			LLDynamicArray<std::string> tree;
			LLViewerInventoryCategory* cat = gInventory.getCategory(item->getParentUUID());
			while(cat)
			{
				std::string folder = cat->getName();
				folder = curl_escape(folder.c_str(), folder.size());
				tree.insert(tree.begin(),folder);
				cat = gInventory.getCategory(cat->getParentUUID());
			}
			if(container)
			{
				//tree.insert(tree.begin(),objectname i guess fuck);
				//wat
			}
			if(root == "")root = gSavedSettings.getString("EmeraldInvMirrorLocation");
			if(!LLFile::isdir(root))
			{
				cmdline_printchat("Error: mirror root \""+root+"\" is nonexistant");
				return FALSE;
			}
			std::string path = gDirUtilp->getDirDelimiter();
			root = root + path;
			for (LLDynamicArray<std::string>::iterator it = tree.begin();
			it != tree.end();
			++it)
			{
				std::string folder = *it;
				root = root + folder;
				if(!LLFile::isdir(root))
				{
					LLFile::mkdir(root);
				}
				root = root + path;
				//cmdline_printchat(root);
			}
			if(iname == "")
			{
				iname = item->getName();
				iname = curl_escape(iname.c_str(), iname.size());
			}
			root = root + iname + "." + LLAssetType::lookup(item->getType());
			//cmdline_printchat(root);

			JCAssetInfo* info = new JCAssetInfo;
			info->path = root;
			info->name = item->getName();
			info->assetid = item->getUUID();
			
			//LLHost host = container != NULL ? container->getRegion()->getHost() : LLHost::invalid;

			gAssetStorage->getInvItemAsset(container != NULL ? container->getRegion()->getHost() : LLHost::invalid,
			gAgent.getID(),
			gAgent.getSessionID(),
			perm.getOwner(),
			container != NULL ? container->getID() : LLUUID::null,
			item->getUUID(),
			LLUUID::null,
			item->getType(),
			JCAssetExportCallback,
			info,
			TRUE);

			//add to floater list
			LLSD element;
			element["id"] = info->assetid; //object->getLocalID();

			element["columns"][0]["column"] = "Name";
			element["columns"][0]["type"] = "text";
			element["columns"][0]["value"] = info->name;

			element["columns"][1]["column"] = "UUID";
			element["columns"][1]["type"] = "text";
			element["columns"][1]["value"] = info->assetid;

			element["columns"][2]["column"] = "Object ID";
			element["columns"][2]["type"] = "text";
			element["columns"][2]["value"] = llformat("%u",container->getLocalID());

			LLVector3 object_pos = container->getPositionRegion();
			std::stringstream sstr;	
			sstr <<llformat("%.1f", object_pos.mV[VX]);
			sstr <<","<<llformat("%.1f", object_pos.mV[VY]);
			sstr <<","<<llformat("%.1f", object_pos.mV[VZ]);

			element["columns"][3]["column"] = "Position";
			element["columns"][3]["type"] = "text";
			element["columns"][3]["value"] = sstr.str();

			//element["columns"][4]["column"] = "Retries";
			//element["columns"][4]["type"] = "text";
			//element["columns"][4]["value"] = "0";

			element["columns"][4]["column"] = "icon_rec";
			element["columns"][4]["type"] = "icon";

			LLScrollListCtrl* mResultList;
			mResultList = ExportTrackerFloater::sInstance->getChild<LLScrollListCtrl>("inventory_result_list");
			mResultList->addElement(element, ADD_BOTTOM);

			return TRUE;

			//gAssetStorage->getAssetData(asset_id, item->getType(), JCAssetExportCallback, info,1);
		}
	}
	return FALSE;
}


void JCExportTracker::cleanup()
{
	gIdleCallbacks.deleteFunction(exportworker);
	
	status=IDLE;

	requested_textures.clear();
	requested_properties.clear();

	std::list<InventoryRequest_t*>::iterator iter3=requested_inventory.begin();
	for(;iter3!=requested_inventory.end();iter3++)
	{
		(*iter3)->object->removeInventoryListener(sInstance);
	}

	requested_inventory.clear();

	
	std::list<LLSD *>::iterator iter=processed_prims.begin();
	for(;iter!=processed_prims.end();iter++)
	{
		free((*iter));
	}

	processed_prims.clear();

	std::map<LLUUID,LLSD *>::iterator iter4=received_properties.begin();
	for(;iter4!=received_properties.end();iter4++)
	{
		free((*iter4).second);
	}
	received_properties.clear();

	std::map<LLUUID,LLSD *>::iterator iter2=received_inventory.begin();
	for(;iter2!=received_inventory.begin();iter2++)
	{
		free((*iter2).second);
	}
	received_inventory.clear();
}

BOOL zip_folder(const std::string& srcfile, const std::string& dstfile)
{
	const S32 COMPRESS_BUFFER_SIZE = 32768;
	std::string tmpfile;
	BOOL retval = FALSE;
	U8 buffer[COMPRESS_BUFFER_SIZE];
	gzFile dst = NULL;
	LLFILE *src = NULL;
	S32 bytes = 0;
	tmpfile = dstfile + ".t";
	dst = gzopen(tmpfile.c_str(), "wb");		/* Flawfinder: ignore */
	if (! dst) goto err;
	src = LLFile::fopen(srcfile, "rb");		/* Flawfinder: ignore */
	if (! src) goto err;

	do
	{
		bytes = (S32)fread(buffer, sizeof(U8), COMPRESS_BUFFER_SIZE,src);
		gzwrite(dst, buffer, bytes);
	} while(feof(src) == 0);
	gzclose(dst);
	dst = NULL;
#if LL_WINDOWS
	// Rename in windows needs the dstfile to not exist.
	LLFile::remove(dstfile);
#endif
	if (LLFile::rename(tmpfile, dstfile) == -1) goto err;		/* Flawfinder: ignore */
	retval = TRUE;
 err:
	if (src != NULL) fclose(src);
	if (dst != NULL) gzclose(dst);
	return retval;
}