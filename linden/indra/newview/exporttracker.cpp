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
#include "llviewernetwork.h"
#include "llcurl.h"
#include "llviewerimagelist.h"

#include "llimagej2c.h"

#include "llviewertexteditor.h"
#include "lllogchat.h" //for timestamp
#include "lluictrlfactory.h"
#include "llcheckboxctrl.h"
#include "llcallbacklist.h"

#ifdef LL_STANDALONE
# include <zlib.h>
#else
# include "zlib/zlib.h"
#endif

JCExportTracker* JCExportTracker::sInstance;
LLSD JCExportTracker::data;
LLSD JCExportTracker::total;
U32 JCExportTracker::totalprims;
U32 JCExportTracker::propertyqueries;
U32 JCExportTracker::invqueries;
BOOL JCExportTracker::export_properties;
BOOL JCExportTracker::export_inventory;
BOOL JCExportTracker::export_textures;
U32 JCExportTracker::status;
std::string JCExportTracker::destination;
std::string JCExportTracker::asset_dir;
std::set<LLUUID> JCExportTracker::requested_textures;
LLVector3 JCExportTracker::selection_center;
LLVector3 JCExportTracker::selection_size;
void cmdline_printchat(std::string chat);
std::list<PropertiesRequest_t*> JCExportTracker::requested_properties;

ExportTrackerFloater* ExportTrackerFloater::sInstance = 0;
LLDynamicArray<LLViewerObject*> ExportTrackerFloater::objectselection;
int ExportTrackerFloater::properties_exported = 0;
int ExportTrackerFloater::total_properties = 0;
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
		+  llformat("\nProperties: %u/%u",properties_exported,total_properties)
		+  llformat("\n   Pending Queries: %u",JCExportTracker::propertyqueries)
		+  llformat("\nInventory Items: %u/%u",assets_exported,total_assets)
		+  llformat("\n   Pending Queries: %u",JCExportTracker::invqueries)
		+  llformat("\nTextures: %u/%u",textures_exported,total_textures)
		);
}


ExportTrackerFloater::ExportTrackerFloater()
:	LLFloater( std::string("Prim Export Floater") )
{
	LLUICtrlFactory::getInstance()->buildFloater( this, "floater_prim_export.xml" );

	childSetAction("export", onClickExport, this);
	childSetAction("close", onClickClose, this);
	childSetAction("reset", onClickReset, this);
	childSetEnabled("export",true);

	//from serializeselection
	JCExportTracker::init();

	gIdleCallbacks.deleteFunction(JCExportTracker::exportworker);
	properties_exported = 0;
	property_queries = 0;
	assets_exported = 0;
	textures_exported = 0;
	total_assets = 0;
	linksets_exported = 0;
	total_textures = 0;
	total_properties = 0;

	total_linksets = LLSelectMgr::getInstance()->getSelection()->getRootObjectCount();
	total_objects = LLSelectMgr::getInstance()->getSelection()->getObjectCount();
	//total_textures = LLSelectMgr::getInstance()->getSelection()->getTECount(); is this unique textures?

	LLDynamicArray<LLViewerObject*> catfayse;
	for (LLObjectSelection::valid_root_iterator iter = LLSelectMgr::getInstance()->getSelection()->valid_root_begin();
			 iter != LLSelectMgr::getInstance()->getSelection()->valid_root_end(); iter++)
	{
		LLSelectNode* selectNode = *iter;
		LLViewerObject* object = selectNode->getObject();
		if(object)
			if(!object->isAvatar() && object->permModify() && object->permCopy() && object->permTransfer() && !gAgent.getGodLevel())
			{
				catfayse.put(object);
			}
			else
			{
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
}

// static
void ExportTrackerFloater::onClickExport(void* data)
{
	JCExportTracker::serialize(objectselection);
}

// static
void ExportTrackerFloater::onClickClose(void* data)
{
	sInstance->close();
	JCExportTracker::sInstance->close();
}

// static
void ExportTrackerFloater::onClickReset(void* data)
{
	JCExportTracker::propertyqueries = 0;
	JCExportTracker::invqueries = 0;
}

JCExportTracker::JCExportTracker()
{
	llassert_always(sInstance == NULL);
	sInstance = this;

}

JCExportTracker::~JCExportTracker()
{
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
	invqueries = 0;
	propertyqueries = 0;
	totalprims = 0;
	data = LLSD();
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
};


LLSD JCExportTracker::subserialize(LLViewerObject* linkset)
{
	//Chalice - Changed to support exporting linkset groups.
	LLViewerObject* object = linkset;
	//if(!linkset)return LLSD();

	// Create an LLSD object that will hold the entire tree structure that can be serialized to a file
	LLSD llsd;

	//if (!node)
	//	return llsd;

	//object = root_object = node->getObject();
	
	if (!object)
		return llsd;
	if(!(!object->isAvatar() && object->permModify() && object->permCopy() && object->permTransfer() && !gAgent.getGodLevel()))
		return llsd;
	// Build a list of everything that we'll actually be exporting
	LLDynamicArray<LLViewerObject*> export_objects;
	
	// Add the root object to the export list
	export_objects.put(object);

	//all child objects must also be active
	llassert_always(object);
	
	// Iterate over all of this objects children
	LLViewerObject::const_child_list_t& child_list = object->getChildren(); //this crashes sometimes. is using llassert a bad hack?? -Patrick Sapinski (Monday, November 23, 2009)
	
	llinfos << "num objects " << (S32) child_list.size() << llendl;

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

				if(export_textures)
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
						img->setBoostLevel(LLViewerImage::BOOST_MAX_LEVEL);
						//required for snowglobe texture fetcher// img->forceToSaveRawImage(0); //this is required for us to receive the full res image.
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

		if(export_textures)
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
						//img->forceToSaveRawImage(0); //this is required for us to receive the full res image. (snowglobe)
						img->setBoostLevel(LLViewerImage::BOOST_MAX_LEVEL);	
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
		if(export_properties)
		{
			//cmdline_printchat("Requesting properties for %s" + object->getID().asString());
			propertyqueries += 1;

			//save this request to the list
			PropertiesRequest_t * req = new PropertiesRequest_t();
			req->target_prim=object->getID();
			req->request_time=time(NULL);
			req->localID=object->getLocalID();
			requested_properties.push_back(req);
			
			requestPrimProperties(object->getLocalID());

			if(export_inventory)
			{
				object->registerInventoryListener(sInstance,NULL);
				object->dirtyInventory();
				object->requestInventory();
				invqueries += 1;
			}
		}//else //cmdline_printchat(llformat("no %d",export_properties));
		totalprims += 1;

		// Changed to use link numbers zero-indexed.
		llsd[object_index - 1] = prim_llsd;
	}
	return llsd;
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

			if(gSavedSettings.getBOOL("ExportTGATextures"))
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

			if(gSavedSettings.getBOOL("ExportJ2CTextures"))
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
	
	export_properties = gSavedSettings.getBOOL("EmeraldExportProperties"); 	 
	export_inventory = gSavedSettings.getBOOL("EmeraldExportInventory");
	export_textures = gSavedSettings.getBOOL("ExportJ2CTextures") | gSavedSettings.getBOOL("ExportTGATextures"); 

	if (export_properties)
		ExportTrackerFloater::total_properties = ExportTrackerFloater::total_objects;

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
	LLVector3 first_pos;

	bool success = true;

	gIdleCallbacks.addFunction(exportworker, NULL);

	return success;

}


void JCExportTracker::exportworker(void *userdata)
{
	if (ExportTrackerFloater::linksets_exported >= ExportTrackerFloater::objectselection.count())
	{
		if((requested_properties.size()==0) && (ExportTrackerFloater::properties_exported >= ExportTrackerFloater::total_properties))
			gIdleCallbacks.deleteFunction(exportworker);
		else
		{
			int kick_count=0;				
			std::list<PropertiesRequest_t*>::iterator iter;
			time_t tnow=time(NULL);
			for (iter=requested_properties.begin(); iter != requested_properties.end(); iter++)
			{
				PropertiesRequest_t * req;
				req=(*iter);
				if( (req->request_time+PROP_REQUEST_KICK)< tnow)
				{
					requestPrimProperties(req->localID);
					kick_count++;
				}
				if(kick_count>=50)
					break; // that will do for now
			}			
		}
	}
	else
	{
		if((propertyqueries + invqueries) <= 200) //this has the potential to screw up if a linkset contains enough 
		{										  //inventory in the prims that we get booted for flooding the sim
			//cmdline_printchat(llformat("backing up object %u",ExportTrackerFloater::linksets_exported));
			LLViewerObject* object = ExportTrackerFloater::objectselection.get(ExportTrackerFloater::linksets_exported);

			if(!(!object->isAvatar() && object->permModify() && object->permCopy() && object->permTransfer() && !gAgent.getGodLevel()))
			{
				LLVector3 temp = object->getPosition();
				ExportTrackerFloater::total_linksets--;
				//cmdline_printchat("failed to backup object at position " + llformat( "%f, %f, %f", temp.mV[VX], temp.mV[VY], temp.mV[VZ]));
				//success = false;
				//break;
			}
			else
			{
				LLVector3 object_pos = object->getPosition();
				LLSD origin;

				origin["ObjectPos"] = object_pos.getValue();

				if (object)//impossible condition, you check avatar above//&& !(object->isAvatar()))
				{
					LLSD linkset = subserialize(object);

					if(!linkset.isUndefined())origin["Object"] = linkset;

					total[ExportTrackerFloater::linksets_exported] = origin;
				}
			}
			ExportTrackerFloater::linksets_exported++;
			data = total;

			//cmdline_printchat("exporting " + llformat("%d",objects.size()) + " objects");
			if((ExportTrackerFloater::linksets_exported >= ExportTrackerFloater::objectselection.count()) && !total.isUndefined() && propertyqueries == 0 && invqueries == 0)
			{
				completechk();
			}
		}
	}

}

void JCExportTracker::finalize(LLSD data)
{
	//We convert our LLSD to HPA here.

	LLXMLNode *group_xml;
	group_xml = new LLXMLNode("group", FALSE);

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

	cmdline_printchat("Attempting to output " + llformat("%u", data.size()) + " Objects.");

	// for each linkset
	for(LLSD::array_iterator array_itr = data.beginArray();
		array_itr != data.endArray();
		++array_itr)
		{
			if((*array_itr).has("Object"))
			{	// for each object
				
				LLXMLNode *linkset_xml = new LLXMLNode("linkset", FALSE);

				LLSD linkset_llsd = (*array_itr)["Object"];
				for(LLSD::array_iterator link_itr = linkset_llsd.beginArray();
					link_itr != linkset_llsd.endArray();
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

					if(prim.has("name"))
						prim_xml->createChild("name", FALSE)->setValue("<![CDATA[" + std::string(prim["name"]) + "]]>");

					if(prim.has("description"))
 						prim_xml->createChild("description", FALSE)->setValue("<![CDATA[" + std::string(prim["description"]) + "]]>");

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

					// Extra params // b6fab961-af18-77f8-cf08-f021377a7244

					// Flexible
					if(prim.has("flexible"))
					{  //FIXME
			//			LLFlexibleObjectData* flex = (LLFlexibleObjectData*)object->getParameterEntry(LLNetworkData::PARAMS_FLEXIBLE);
			//			prim_xml->createChild("flexible", FALSE)->setValue(flex->asLLSD());
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
////////////////////////////////////////////////////////////

					//<inventory>
					LLXMLNodePtr inventory_xml = prim_xml->createChild("inventory", FALSE);

					LLSD inventory = prim["inventory"];
				
					//for each inventory item
					for (LLSD::array_iterator inv = inventory.beginArray(); inv != inventory.endArray(); ++inv)
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
					linkset_xml->addChild(prim_xml);
				} //end for each object
				//add this linkset to the group.
				group_xml->addChild(linkset_xml);
			} //end for each linkset.
		}

		// Create a file stream and write to it
		llofstream out(destination	);
							
		if (!out.good())
		{
			llwarns << "Unable to open for output." << llendl;
		}

		out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
							
		LLXMLNode *temp_xml = new LLXMLNode("project", FALSE);
		temp_xml->createChild("schema", FALSE)->setValue("1.0");
		temp_xml->createChild("name", FALSE)->setValue(gDirUtilp->getBaseFileName(destination, false));
		temp_xml->createChild("date", FALSE)->setValue(LLLogChat::timestamp(1));
		temp_xml->createChild("software", FALSE)->setValue(llformat("%s %d.%d.%d.%d",
		LLAppViewer::instance()->getSecondLifeTitle().c_str(), LL_VERSION_MAJOR, LL_VERSION_MINOR, LL_VERSION_PATCH, LL_VERSION_BUILD));
		temp_xml->createChild("platform", FALSE)->setValue("Second Life");
		std::vector<std::string> uris;
		LLViewerLogin* vl = LLViewerLogin::getInstance();
		std::string grid_uri = vl->getGridLabel(); //RC FIXME
		temp_xml->createChild("grid", FALSE)->setValue(grid_uri);

		temp_xml->addChild(group_xml);

		temp_xml->writeToOstream(out);
		out.close();
		cmdline_printchat("File Saved.");

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

/*	we can still output an LLSD with this but it's no longer emerald compatible. useful for testing. */
	LLSD file;
	LLSD header;
	header["Version"] = 2;
	file["Header"] = header;
	//std::vector<std::string> uris;
	//	LLViewerLogin* vl = LLViewerLogin::getInstance();
	//	std::string grid_uri = vl->getGridLabel(); //RC FIXME
	//LLStringUtil::toLower(uris[0]);
	file["Grid"] = grid_uri;
	file["Objects"] = data;

	// Create a file stream and write to it
	llofstream export_file(destination + ".llsd");
	LLSDSerialize::toPrettyXML(file, export_file);
	// Open the file save dialog
	export_file.close();
//*/
	
		ExportTrackerFloater::getInstance()->childSetEnabled("export",true);
		status = IDLE;
}

void JCExportTracker::completechk()
{
	if(propertyqueries == 0 && invqueries == 0 && ExportTrackerFloater::linksets_exported >= ExportTrackerFloater::objectselection.count())
	{
		//cmdline_printchat("Full property export completed.");
		//cmdline_printchat("(Content downloads may require more time, but the tracker is free for another export.)");
		finalize(data);
	}
}

//LLSD* chkdata(LLUUID id, LLSD* data)
//{
//	if(primitives
//}

void JCExportTracker::processObjectProperties(LLMessageSystem* msg, void** user_data)
{
	if(!export_properties)
	{
		return;
	}
	if(status == IDLE)
	{
		////cmdline_printchat("status == IDLE");
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
				if (propertyqueries > 0)
					propertyqueries -= 1;
				ExportTrackerFloater::properties_exported++;
				break;
			}
		}

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
						if((*link_itr)["id"].asString() == id.asString())
						{
							if(!((*link_itr).has("properties")))
							{
								//cmdline_printchat("Received information for prim "+id.asString());
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
								(*link_itr)["properties"] = (S32)1;
								(*link_itr)["creator_id"] = creator_id.asString();
								(*link_itr)["owner_id"] = owner_id.asString();
								(*link_itr)["group_id"] = group_id.asString();
								(*link_itr)["last_owner_id"] = last_owner_id.asString();
								(*link_itr)["creation_date"] = llformat("%d",creation_date);
								(*link_itr)["extra_id"] = extra_id.asString();
								(*link_itr)["base_mask"] = llformat("%d",base_mask);
								(*link_itr)["owner_mask"] = llformat("%d",owner_mask);
								(*link_itr)["group_mask"] = llformat("%d",group_mask);
								(*link_itr)["everyone_mask"] = llformat("%d",everyone_mask);
								(*link_itr)["next_owner_mask"] = llformat("%d",next_owner_mask);
								(*link_itr)["sale_info"] = sale_info.asLLSD();

								(*link_itr)["inv_serial"] = (S32)inv_serial;
								(*link_itr)["item_id"] = item_id.asString();
								(*link_itr)["folder_id"] = folder_id.asString();
								(*link_itr)["from_task_id"] = from_task_id.asString();
								(*link_itr)["name"] = name;
								(*link_itr)["description"] = desc;
								(*link_itr)["touch_name"] = touch_name;
								(*link_itr)["sit_name"] = sit_name;

								//cmdline_printchat(llformat("%d server queries left",propertyqueries));
							}
						}
					}
				}
				(*array_itr)["Object"] = linkset_llsd;
			}
		}
		completechk();
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

								//cmdline_printchat("downloaded inventory for "+obj->getID().asString());
								LLSD inventory;
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
											//cmdline_printchat("requesting asset for "+asset->getName());
											inv_item["desc"] = ((LLInventoryItem*)((LLInventoryObject*)(*it)))->getDescription();//god help us all
											inv_item["item_id"] = asset->getUUID().asString();
											if(!LLFile::isdir(asset_dir+gDirUtilp->getDirDelimiter()+"inventory"))
											{
												LLFile::mkdir(asset_dir+gDirUtilp->getDirDelimiter()+"inventory");
											}
											JCExportTracker::mirror(asset, obj, asset_dir+gDirUtilp->getDirDelimiter()+"inventory", asset->getUUID().asString());//loltest
											//unacceptable
											inventory[num] = inv_item;
											num += 1;
											ExportTrackerFloater::total_assets++;
										}
									}
								}
								(*link_itr)["inventory"] = inventory;
								if (invqueries > 0)
									invqueries -= 1;
								//cmdline_printchat(llformat("%d inv queries left",invqueries));
							}
						}
					}
				}
				(*array_itr)["Object"] = linkset_llsd;
			}
		}


	}
	obj->removeInventoryListener(sInstance);

	completechk();
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
			}//else //cmdline_printchat("Failed to decode notecard");
		}
		apr_file_t* fp = ll_apr_file_open(info->path.c_str(), LL_APR_WB);
		if(fp)ll_apr_file_write(fp, buffer, size);
		apr_file_close(fp);
		//delete[] buffer;
		ExportTrackerFloater::assets_exported++;
	}else //cmdline_printchat("Failed to save file "+info->path+" ("+info->name+") : "+std::string(LLAssetStorage::getErrorString(result)));

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
				//cmdline_printchat("Error: mirror root \""+root+"\" is nonexistant");
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

			return TRUE;

			//gAssetStorage->getAssetData(asset_id, item->getType(), JCAssetExportCallback, info,1);
		}
	}
	return FALSE;
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