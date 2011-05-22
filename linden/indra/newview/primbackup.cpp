
#include "llviewerprecompiledheaders.h"


// system library includes
#include <iostream>
#include <fstream>
#include <sstream>

// linden library includes
#include "llfilepicker.h"
#include "indra_constants.h"
#include "llsdserialize.h"
#include "llsdutil.h"

#include "llcallbacklist.h"

// newview includes
#include "llagent.h"
#include "llselectmgr.h"
#include "lltoolplacer.h"

#include "lltexturecache.h"

#include "llnotify.h"

#include "llapr.h"
#include "lldir.h"
#include "llimage.h"
#include "lllfsthread.h"
#include "llassetuploadresponders.h"
#include "lleconomy.h"
#include "llhttpclient.h"
#include "lluploaddialog.h"
#include "lldir.h"
#include "llinventorymodel.h"	// gInventory
#include "llviewercontrol.h"	// gSavedSettings
#include "llviewermenu.h"	// gMenuHolder
#include "llfilepicker.h"
#include "llfloateranimpreview.h"
#include "llfloaterbuycurrency.h"
#include "llfloaterimagepreview.h"
#include "llfloaternamedesc.h"
#include "llfloatersnapshot.h"
#include "llresourcedata.h"
#include "llstatusbar.h"
#include "llviewerimagelist.h"
#include "lluictrlfactory.h"
#include "llviewerregion.h"
#include "llviewerstats.h"
#include "llviewerwindow.h"
#include "lluploaddialog.h"
// Included to allow LLTextureCache::purgeTextures() to pause watchdog timeout
#include "llappviewer.h" 
#include "lltransactiontypes.h"

#include "hippogridmanager.h"

#include "primbackup.h" 

#include "llviewerobjectlist.h"

primbackup* primbackup::sInstance = 0;

class importResponder: public LLNewAgentInventoryResponder
{
	public:

	importResponder(const LLSD& post_data,
		const LLUUID& vfile_id,
		LLAssetType::EType asset_type)
	: LLNewAgentInventoryResponder(post_data, vfile_id, asset_type)
	{
	}


	//virtual 
	virtual void uploadComplete(const LLSD& content)
	{
			lldebugs << "LLNewAgentInventoryResponder::result from capabilities" << llendl;

	LLAssetType::EType asset_type = LLAssetType::lookup(mPostData["asset_type"].asString());
	LLInventoryType::EType inventory_type = LLInventoryType::lookup(mPostData["inventory_type"].asString());

	// Update L$ and ownership credit information
	// since it probably changed on the server
	if (asset_type == LLAssetType::AT_TEXTURE ||
		asset_type == LLAssetType::AT_SOUND ||
		asset_type == LLAssetType::AT_ANIMATION)
	{
		gMessageSystem->newMessageFast(_PREHASH_MoneyBalanceRequest);
		gMessageSystem->nextBlockFast(_PREHASH_AgentData);
		gMessageSystem->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
		gMessageSystem->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
		gMessageSystem->nextBlockFast(_PREHASH_MoneyData);
		gMessageSystem->addUUIDFast(_PREHASH_TransactionID, LLUUID::null );
		gAgent.sendReliableMessage();

//		LLStringUtil::format_map_t args;
//		args["[AMOUNT]"] = llformat("%d",LLGlobalEconomy::Singleton::getInstance()->getPriceUpload());
//		LLNotifyBox::showXml("UploadPayment", args);
	}

	// Actually add the upload to viewer inventory
	llinfos << "Adding " << content["new_inventory_item"].asUUID() << " "
			<< content["new_asset"].asUUID() << " to inventory." << llendl;
	if(mPostData["folder_id"].asUUID().notNull())
	{
		LLPermissions perm;
		U32 next_owner_perm;
		perm.init(gAgent.getID(), gAgent.getID(), LLUUID::null, LLUUID::null);
		if (mPostData["inventory_type"].asString() == "snapshot")
		{
			next_owner_perm = PERM_ALL;
		}
		else
		{
			next_owner_perm = PERM_MOVE | PERM_TRANSFER;
		}
		perm.initMasks(PERM_ALL, PERM_ALL, PERM_NONE, PERM_NONE, next_owner_perm);
		S32 creation_date_now = time_corrected();
		LLPointer<LLViewerInventoryItem> item
			= new LLViewerInventoryItem(content["new_inventory_item"].asUUID(),
										mPostData["folder_id"].asUUID(),
										perm,
										content["new_asset"].asUUID(),
										asset_type,
										inventory_type,
										mPostData["name"].asString(),
										mPostData["description"].asString(),
										LLSaleInfo::DEFAULT,
										LLInventoryItem::II_FLAGS_NONE,
										creation_date_now);
		gInventory.updateItem(item);
		gInventory.notifyObservers();
	}
	else
	{
		llwarns << "Can't find a folder to put it in" << llendl;
	}

	// remove the "Uploading..." message
	LLUploadDialog::modalUploadFinished();
	
	primbackup::getInstance()->update_map(content["new_asset"].asUUID());
	primbackup::getInstance()->upload_next_asset();

	}

};



class CacheReadResponder : public LLTextureCache::ReadResponder
	{
	public:
		CacheReadResponder(const LLUUID& id, LLImageFormatted* image)
			:  mFormattedImage(image), mID(id)
		{
			setImage(image);
		}
		void setData(U8* data, S32 datasize, S32 imagesize, S32 imageformat, BOOL imagelocal)
		{
			if(imageformat==IMG_CODEC_TGA && mFormattedImage->getCodec()==IMG_CODEC_J2C)
			{
				llwarns<<"Bleh its a tga not saving"<<llendl;
				mFormattedImage=NULL;
				mImageSize=0;
				return;
			}

			if (mFormattedImage.notNull())
			{	
				llassert_always(mFormattedImage->getCodec() == imageformat);
				mFormattedImage->appendData(data, datasize);
			}
			else
			{
				mFormattedImage = LLImageFormatted::createFromType(imageformat);
				mFormattedImage->setData(data,datasize);
			}
			mImageSize = imagesize;
			mImageLocal = imagelocal;
		}

		virtual void completed(bool success)
		{
			if(success && (mFormattedImage.notNull()) && mImageSize>0)
			{
				
				llinfos << "SUCCESS getting texture "<<mID<< llendl;
				
				std::string name;
				mID.toString(name);
				llinfos << "Saving to "<<(primbackup::getInstance()->getfolder()+"//"+name)<<llendl;			
				if(!mFormattedImage->save(primbackup::getInstance()->getfolder()+"//"+name))
				{
					llinfos << "FAIL saving texture "<<mID<< llendl;
				}

			}
			else
			{
				if(!success)
					llwarns << "FAIL NOT SUCCESSFUL getting texture "<<mID<< llendl;
				if(mFormattedImage.isNull())
					llwarns << "FAIL image is NULL "<<mID<< llendl;
			}	

			primbackup::getInstance()->m_nexttextureready=true;
			//JUST SAY NO TO APR DEADLOCKING
			//primbackup::getInstance()->export_next_texture();
		}
	private:
		LLPointer<LLImageFormatted> mFormattedImage;
		LLUUID mID;
	};



primbackup::primbackup()
:	LLFloater( std::string("Prim Import Floater") )
{
	LLUICtrlFactory::getInstance()->buildFloater( this, "floater_prim_import.xml" );

	// reposition floater from saved settings
	//LLRect rect = gSavedSettings.getRect( "FloaterPrimImport" );
	//reshape( rect.getWidth(), rect.getHeight(), FALSE );
	//setRect( rect );

	running=false;
	textures.clear();
	assetmap.clear();
	current_asset=LLUUID::null;
	m_retexture=false;
	close();
}


////////////////////////////////////////////////////////////////////////////////
//
primbackup* primbackup::getInstance()
{
    if ( ! sInstance )
        sInstance = new primbackup();

	return sInstance;
}

primbackup::~primbackup()
{
	// save position of floater
	gSavedSettings.setRect( "FloaterPrimImport", getRect() );
	sInstance = 0;
}

void primbackup::draw()
{
	LLFloater::draw();
}

void primbackup::show()
{
	// set the title 
	setTitle( "stuff" );
	m_curobject=1;
	m_curprim=0;
	m_objects=0;
	m_prims=0;
	m_textures=0;
	m_curtexture=0;
	rezcount=0;

	// make floater appear
	setVisibleAndFrontmost();
}


void primbackup::onClose( bool app_quitting )
{
	setVisible( false );
	// HACK for fast XML iteration replace with:
	// destroy();
}

void primbackup::updateexportnumbers()
{

	std::stringstream sstr;	
	LLUICtrl * ctrl=this->getChild<LLUICtrl>("name_label");	

	sstr<<"Export Progress \n";

	sstr << "Remaining Textures "<<textures.size()<<"\n";
	ctrl->setValue(LLSD("Text")=sstr.str());
	
}


void primbackup::updateimportnumbers()
{
	std::stringstream sstr;	
	LLUICtrl * ctrl=this->getChild<LLUICtrl>("name_label");	

	if(m_retexture)
	{
		sstr << " Textures uploads remaining : "<<textures.size()<<"\n";
		ctrl->setValue(LLSD("Text")=sstr.str());
	}
	else
	{
		sstr << " Textures uploads N/A \n";
		ctrl->setValue(LLSD("Text")=sstr.str());
	}
	sstr << " Objects "<<this->m_curobject<<"/"<<this->m_objects<<"\n";
	ctrl->setValue(LLSD("Text")=sstr.str());
	
	sstr << " Rez "<<this->rezcount<<"/"<<this->m_prims;
	ctrl->setValue(LLSD("Text")=sstr.str());

	sstr << " Build "<<this->m_curprim<<"/"<<this->m_prims;
	ctrl->setValue(LLSD("Text")=sstr.str());


}

void primbackup::pre_export_object()
{
	textures.clear();
	llsd.clear();
	this_group.clear();

	// Open the file save dialog
	LLFilePicker& file_picker = LLFilePicker::instance();
	if( !file_picker.getSaveFile( LLFilePicker::FFSAVE_XML ) )
	{
		// User canceled save.
		return;
	}
	 
	file_name = file_picker.getCurFile();
	folder = gDirUtilp->getDirName(file_name);

	export_state=EXPORT_INIT;
	gIdleCallbacks.addFunction(exportworker, NULL);
}


// static
bool primbackup::check_perms( LLSelectNode* node )
{
	LLPermissions *perms = node->mPermissions;
	return (gAgent.getID() == perms->getOwner() &&
	        gAgent.getID() == perms->getCreator() &&
	        (PERM_ITEM_UNRESTRICTED &
	         perms->getMaskOwner()) == PERM_ITEM_UNRESTRICTED);
}


void primbackup::exportworker(void *userdata)
{	
	primbackup::getInstance()->updateexportnumbers();

	switch(primbackup::getInstance()->export_state)
	{
		case EXPORT_INIT: {
			primbackup::getInstance()->show();		
			LLSelectMgr::getInstance()->getSelection()->ref();

			struct ff : public LLSelectedNodeFunctor
			{
				virtual bool apply(LLSelectNode* node)
				{
					return primbackup::check_perms( node );
				}
			} func;

			if(LLSelectMgr::getInstance()->getSelection()->applyToNodes(&func,false))
			{
				if(gHippoGridManager->getConnectedGrid()->isSecondLife())
				{
					LLNotifications::instance().add("NoTextureExportSL");
				}
				primbackup::getInstance()->export_state=EXPORT_STRUCTURE;
			}
			else
			{
				llwarns << "Incorrect permission to export" << llendl;
				primbackup::getInstance()->export_state=EXPORT_DONE;
				primbackup::getInstance()->close();
				gIdleCallbacks.deleteFunction(exportworker);
				LLSelectMgr::getInstance()->getSelection()->unref();
			}
			break;
		}

		case EXPORT_STRUCTURE: {
			struct ff : public LLSelectedObjectFunctor
			{
				virtual bool apply(LLViewerObject* object)
				{
					object->boostTexturePriority(TRUE);
					LLViewerObject::child_list_t children = object->getChildren();
					children.push_front(object); //push root onto list
					LLSD prim_llsd=primbackup::getInstance()->prims_to_llsd(children);
					LLSD stuff;
					stuff["root_position"] = object->getPosition().getValue();
					stuff["root_rotation"] = ll_sd_from_quaternion(object->getRotation());
					stuff["group_body"] = prim_llsd;
					primbackup::getInstance()->llsd["data"].append(stuff);
					return true;
				}
			} func;

			primbackup::getInstance()->export_state=EXPORT_LLSD;
			LLSelectMgr::getInstance()->getSelection()->applyToRootObjects(&func,false);
			LLSelectMgr::getInstance()->getSelection()->unref();

			break;
		}

		case EXPORT_TEXTURES: {
			// Exporting object textures (or other content) from Second Life
			// without checking creator is a violation of the Second Life
			// Policy on Third-Party Viewers and Terms of Service.
			if(gHippoGridManager->getConnectedGrid()->isSecondLife())
			{
				primbackup::getInstance()->export_state=EXPORT_DONE;
				return;
			}

			if(primbackup::getInstance()->m_nexttextureready==false)
				return;

			//Ok we got work to do
			primbackup::getInstance()->m_nexttextureready=false;

			if(primbackup::getInstance()->textures.empty())
			{
				primbackup::getInstance()->export_state=EXPORT_DONE;
				return;
			}

			primbackup::getInstance()->export_next_texture();
			break;
		}

		case EXPORT_LLSD: {
			// Create a file stream and write to it
			llofstream export_file(primbackup::getInstance()->file_name);
			LLSDSerialize::toPrettyXML(primbackup::getInstance()->llsd, export_file);
			export_file.close();
			primbackup::getInstance()->m_nexttextureready=true;	
			primbackup::getInstance()->export_state=EXPORT_TEXTURES;
			break;
		}

		case EXPORT_DONE: {
			llinfos << "Backup complete" << llendl;
			gIdleCallbacks.deleteFunction(exportworker);
			primbackup::getInstance()->close();
			break;
		}
	}
}

LLSD primbackup::prims_to_llsd(LLViewerObject::child_list_t child_list)
{

	LLViewerObject* object;
	LLSD llsd;

	char localid[16];

	for (LLViewerObject::child_list_t::iterator i = child_list.begin(); i != child_list.end(); ++i)
	{
		object=(*i);
		LLUUID id = object->getID();

		llinfos << "Exporting prim " << object->getID().asString() << llendl;

		// Create an LLSD object that represents this prim. It will be injected in to the overall LLSD
		// tree structure
		LLSD prim_llsd;

		if (!object->isRoot())
		{

			// Parent id
			snprintf(localid, sizeof(localid), "%u", object->getSubParent()->getLocalID());
			prim_llsd["parent"] = localid;
		}

		// Transforms
		prim_llsd["position"] = object->getPosition().getValue();
		prim_llsd["scale"] = object->getScale().getValue();
		prim_llsd["rotation"] = ll_sd_from_quaternion(object->getRotation());

		// Flags
		prim_llsd["shadows"] = object->flagCastShadows();
		prim_llsd["phantom"] = object->flagPhantom();
		prim_llsd["physical"] = (BOOL)(object->mFlags & FLAGS_USE_PHYSICS);

		// Volume params
		LLVolumeParams params = object->getVolume()->getParams();
		prim_llsd["volume"] = params.asLLSD();

		// Extra paramsb6fab961-af18-77f8-cf08-f021377a7244
		if (object->isFlexible())
		{
			// Flexible
			LLFlexibleObjectData* flex = (LLFlexibleObjectData*)object->getParameterEntry(LLNetworkData::PARAMS_FLEXIBLE);
			prim_llsd["flexible"] = flex->asLLSD();
		}
		if (object->getParameterEntryInUse(LLNetworkData::PARAMS_LIGHT))
		{
			// Light
			LLLightParams* light = (LLLightParams*)object->getParameterEntry(LLNetworkData::PARAMS_LIGHT);
			prim_llsd["light"] = light->asLLSD();
		}
		if (object->getParameterEntryInUse(LLNetworkData::PARAMS_SCULPT))
		{
			// Sculpt
			LLSculptParams* sculpt = (LLSculptParams*)object->getParameterEntry(LLNetworkData::PARAMS_SCULPT);
			prim_llsd["sculpt"] = sculpt->asLLSD();
			
			LLUUID sculpt_texture=sculpt->getSculptTexture();
			bool alreadyseen=false;
			std::list<LLUUID>::iterator iter;
			for(iter = textures.begin(); iter != textures.end() ; iter++) 
			{
				if( (*iter)==sculpt_texture)
					alreadyseen=true;
			}
			if(alreadyseen==false)
			{
				llinfos << "Found a sculpt texture, adding to list "<<sculpt_texture<<llendl;
				textures.push_back(sculpt_texture);
			}
		}

		// Textures
		LLSD te_llsd;
		U8 te_count = object->getNumTEs();
		for (U8 i = 0; i < te_count; i++)
		{
			bool alreadyseen=false;
			te_llsd.append(object->getTE(i)->asLLSD());
			std::list<LLUUID>::iterator iter;
			for(iter = textures.begin(); iter != textures.end() ; iter++) 
			{
				if( (*iter)==object->getTE(i)->getID())
					alreadyseen=true;
			}
			if(alreadyseen==false)
				textures.push_back(object->getTE(i)->getID());
		}
		prim_llsd["textures"] = te_llsd;

		// The keys in the primitive maps do not have to be localids, they can be any
		// string. We simply use localids because they are a unique identifier
		snprintf(localid, sizeof(localid), "%u", object->getLocalID());
		llsd[(const char*)localid] = prim_llsd;
	}

	updateexportnumbers();

	return llsd;
}


void primbackup::export_next_texture()
{
	if(textures.empty())
	{
		llinfos << "Finished exporting textures "<<llendl;
		return;
	}

	std::list<LLUUID>::iterator iter;
	iter = textures.begin();

	LLUUID id;

	while(1)
	{
		if(iter==textures.end())
		{
			m_nexttextureready=true;
			return;
		}

		id=(*iter);

		LLViewerImage * imagep = gImageList.hasImage(id);
		if(imagep!=NULL)
		{
			S32 cur_discard = imagep->getDiscardLevel();
			if(cur_discard>0)
			{
				if(imagep->getBoostLevel()!=LLViewerImageBoostLevel::BOOST_PREVIEW)
					imagep->setBoostLevel(LLViewerImageBoostLevel::BOOST_PREVIEW); //we want to force discard 0 this one does this.
			}
			else
			{
				break;
			}
		}
		else
		{
			llwarns<<" We *DONT* have the texture "<<llendl;
		}
		iter++;
	}

	textures.remove(id);

	llinfos<<"Requesting texture "<<id<<llendl;
	LLImageJ2C * mFormattedImage = new LLImageJ2C;
	CacheReadResponder* responder = new CacheReadResponder(id, mFormattedImage);
  	LLAppViewer::getTextureCache()->readFromCache(id,LLWorkerThread::PRIORITY_HIGH,0,999999,responder);
}



void primbackup::import_object(bool upload)
{


	textures.clear();
	assetmap.clear();
	current_asset=LLUUID::null;

	this->m_retexture=upload;

	// Open the file open dialog
	LLFilePicker& file_picker = LLFilePicker::instance();
	if( !file_picker.getOpenFile( LLFilePicker::FFLOAD_XML ) )
	{
		// User canceled save.
		return;
	}
	std::string file_name = file_picker.getFirstFile().c_str();
	folder = gDirUtilp->getDirName(file_name);

	{
		LLSD import_llsd;
		llifstream import_file(file_name);
		S32 status;
		status = LLSDSerialize::fromXML(import_llsd, import_file);
		import_file.close();
	
		if (LLSDParser::PARSE_FAILURE == status)
		{
			llwarns << "invalid xml file." << llendl;
			return;
		}

		llsd = import_llsd;
	}

	show();

	//Get the texture map
	
	LLSD::map_const_iterator prim_it;
	LLSD::array_const_iterator prim_arr_it;
		
	this->m_curobject=1;
	this->m_curprim=1;
	this->m_objects=llsd["data"].size();
	this->m_prims=0;
	rezcount=0;

	updateimportnumbers();

	for( prim_arr_it = llsd["data"].beginArray(); prim_arr_it != llsd["data"].endArray(); prim_arr_it++)
	{

		LLSD llsd2;
		llsd2=(*prim_arr_it)["group_body"];

		for( prim_it = llsd2.beginMap(); prim_it != llsd2.endMap(); prim_it++)
		{
			LLSD prim_llsd;
			prim_llsd=llsd2[prim_it->first];
			LLSD::array_iterator text_it;
			std::list<LLUUID>::iterator iter;

			if(prim_llsd.has("sculpt"))
			{
				LLSculptParams* sculpt=new LLSculptParams();
				sculpt->fromLLSD(prim_llsd["sculpt"]);
				LLUUID orig=sculpt->getSculptTexture();
				bool alreadyseen=false;
				for(iter = textures.begin(); iter != textures.end() ; iter++) 
				{
					if( (*iter)==orig)
						alreadyseen=true;
				}
				if(alreadyseen==false)
				{
					llinfos << "Found a new SCULPT texture to upload "<<orig<<llendl;			
					textures.push_back(orig);
				}
			}


			LLSD te_llsd;
			te_llsd=prim_llsd["textures"];

		
			for(text_it=te_llsd.beginArray(); text_it !=te_llsd.endArray(); text_it++)
			{
				LLSD the_te;
				the_te=(*text_it);
				LLTextureEntry te;
				te.fromLLSD(the_te);

				te.getID();
				bool alreadyseen=false;

				for(iter = textures.begin(); iter != textures.end() ; iter++) 
				{
					if( (*iter)==te.getID())
						alreadyseen=true;
				}
				if(alreadyseen==false)
				{
					llinfos << "Found a new texture to upload "<<te.getID()<<llendl;			
					textures.push_back(te.getID());
				}	     
			}

		}
	}

	if(m_retexture==TRUE)
		upload_next_asset();
	else
		import_object1a();
}

LLVector3 primbackup::offset_agent(LLVector3 offset)
{
	LLVector3 pos= gAgent.getPositionAgent();
	LLQuaternion agent_rot=LLQuaternion(gAgent.getAtAxis(),gAgent.getLeftAxis(),gAgent.getUpAxis());
	pos=(offset*agent_rot+pos);
	return pos;
}

void primbackup::rez_agent_offset(LLVector3 offset)
{
	// This will break for a sitting agent
	LLToolPlacer* mPlacer = new LLToolPlacer();
	mPlacer->setObjectType(LL_PCODE_CUBE);	
	//LLVector3 pos=offset_agent(offset);
	mPlacer->placeObject((S32)(offset.mV[0]), (S32)(offset.mV[1]), 0);
}

void primbackup::import_object1a()
{
	running=true;

	show();

	group_prim_import_iter=llsd["data"].beginArray();	
	root_root_pos=(*group_prim_import_iter)["root_position"];

	this->m_objects=llsd["data"].size();
	this->m_curobject=1;
	import_next_object();
}

void primbackup::import_next_object()
{
	toselect.clear();
	rezcount=0;

	this_group=(*group_prim_import_iter)["group_body"];
	prim_import_iter=this_group.beginMap();

	m_curprim=0;
	m_prims=this_group.size();
	updateimportnumbers();
	LLVector3 lgpos=(*group_prim_import_iter)["root_position"];

	group_offset=lgpos-root_root_pos;
	root_pos=offset_agent(LLVector3(2.0,0,0));
	root_rot=ll_quaternion_from_sd((*group_prim_import_iter)["root_rotation"]);
	
	rez_agent_offset(LLVector3(0.0,2.0,0.0));
	// Now we must wait for the callback when ViewerObjectList gets the new objects and we have the correct number selected
}

// This function takes a pointer to a viewerobject and applys the prim definition that prim_llsd has
void primbackup::xmltoprim(LLSD prim_llsd,LLViewerObject * object)
{
	LLUUID id = object->getID();
	expecting_update = object->getID();
	LLSelectMgr::getInstance()->selectObjectAndFamily(object);

	if(prim_llsd.has("parent"))
	{
		//we are not the root node.
		LLVector3 pos=prim_llsd["position"];
		LLQuaternion rot=ll_quaternion_from_sd(prim_llsd["rotation"]);
		object->setPositionRegion((pos*root_rot)+(root_pos+group_offset));
		object->setRotation(rot*root_rot);
	}
	else
	{
		object->setPositionRegion(root_pos+group_offset);
		LLQuaternion rot=ll_quaternion_from_sd(prim_llsd["rotation"]);
		object->setRotation(rot);
	}

	object->setScale(prim_llsd["scale"]);

	if(prim_llsd.has("shadows"))
		if(prim_llsd["shadows"].asInteger()==1)
			object->setFlags(FLAGS_CAST_SHADOWS,true);

	if(prim_llsd.has("phantom"))
		if(prim_llsd["phantom"].asInteger()==1)
			object->setFlags(FLAGS_PHANTOM,true);

	if(prim_llsd.has("physical"))
		if(prim_llsd["physical"].asInteger()==1)
			object->setFlags(FLAGS_USE_PHYSICS,true);

	// Volume params
	LLVolumeParams volume_params = object->getVolume()->getParams();
	volume_params.fromLLSD(prim_llsd["volume"]) ;
	object->updateVolume(volume_params);
	
	if(prim_llsd.has("sculpt"))
	{
		LLSculptParams* sculpt=new LLSculptParams();
		sculpt->fromLLSD(prim_llsd["sculpt"]);
		
		//TODO check if map is valid and only set texture is map is valid and changes

		if(assetmap[sculpt->getSculptTexture()].notNull())
		{
			LLUUID replacment=assetmap[sculpt->getSculptTexture()];
			sculpt->setSculptTexture(replacment);
		}

		object->setParameterEntry(LLNetworkData::PARAMS_SCULPT,(LLNetworkData&)(*sculpt),true);
	}
		
	if(prim_llsd.has("light"))
	{
		LLLightParams * light=new LLLightParams();
		light->fromLLSD(prim_llsd["light"]);
		object->setParameterEntry(LLNetworkData::PARAMS_LIGHT,(LLNetworkData&)(*light),true);
	}

	if(prim_llsd.has("flexible"))
	{
		LLFlexibleObjectData* flex=new LLFlexibleObjectData();
		flex->fromLLSD(prim_llsd["flexible"]);
		object->setParameterEntry(LLNetworkData::PARAMS_FLEXIBLE,(LLNetworkData&)(*flex),true);
	}


	// Textures
	LLSD te_llsd;
	llinfos << "Processing textures for prim" << llendl;

	te_llsd=prim_llsd["textures"];
		
	LLSD::array_iterator text_it;
	U8 i=0;
	i=0;

	for(text_it=te_llsd.beginArray(); text_it !=te_llsd.endArray(); text_it++)
	{
	    LLSD the_te;
	    the_te=(*text_it);
	    LLTextureEntry te;
	    te.fromLLSD(the_te);

		if(assetmap[te.getID()].notNull())
		{
			LLUUID replacment=assetmap[te.getID()];
			te.setID(replacment);
		}

	    object->setTE(i,te); //	
	    i++;
	}

	llinfos << "Textures done!" << llendl;

	//bump the iterator now so the callbacks hook together nicely
	//if(prim_import_iter!=this_group.endMap())
	//	prim_import_iter++;

    object->sendRotationUpdate();
	object->sendTEUpdate();	
	object->sendShapeUpdate();
	LLSelectMgr::getInstance()->sendMultipleUpdate(UPD_SCALE |UPD_POSITION);

	LLSelectMgr::getInstance()->deselectAll();	
}

//This is fired when the update packet is processed so we know the prim settings have stuck
void primbackup::prim_update(LLViewerObject* object)
{
	if(!running)
		return;

	if(object!=NULL)
		if(object->mID!=expecting_update)
			return;

	m_curprim++;
	updateimportnumbers();

	prim_import_iter++;

	LLUUID x;
	expecting_update=x.null;

	if(prim_import_iter==this_group.endMap())
	{
		llinfos<<"Trying to link"<<llendl;

		if(toselect.size()>1)
		{
			std::reverse(toselect.begin(),toselect.end());
			//Now link
			LLSelectMgr::getInstance()->deselectAll();	
			LLSelectMgr::getInstance()->selectObjectAndFamily(toselect,true);
			LLSelectMgr::getInstance()->sendLink();
			LLViewerObject * root=toselect.back();
			root->setRotation(root_rot);
		}
	
		this->m_curobject++;
		group_prim_import_iter++;
		if(group_prim_import_iter!=llsd["data"].endArray())
		{
			import_next_object();
			return;
		}
		
		running=false;
		this->close();
		return;
	}

	LLSD prim_llsd;
	prim_llsd=this_group[prim_import_iter->first];

	if(toselect.empty())
	{
		llwarns << "error: ran out of objects to mod" << llendl;
		return;
	}

	if(prim_import_iter!=this_group.endMap())
	{
		//rez_agent_offset(LLVector3(1.0,0,0));
		LLSD prim_llsd=this_group[prim_import_iter->first];
		process_iter++;
		xmltoprim(prim_llsd,(*process_iter));	
	}
}

// Callback when we rez a new object when the importer is running.
bool primbackup::newprim(LLViewerObject * pobject)
{
	if(running)
	{
		rezcount++;
		toselect.push_back(pobject);
		updateimportnumbers();
		prim_import_iter++;

		if(prim_import_iter!=this_group.endMap())
		{

			pobject->setPosition(this->offset_agent(LLVector3(0,1.0,0)));
			LLSelectMgr::getInstance()->sendMultipleUpdate(UPD_POSITION);

			rez_agent_offset(LLVector3(1.0,0,0));
		}
		else
		{
			llinfos << "All prims rezed, moving to build stage" <<llendl;
			prim_import_iter=this_group.beginMap();
			LLSD prim_llsd=this_group[prim_import_iter->first];
			process_iter=toselect.begin();
			xmltoprim(prim_llsd,(*process_iter));	
		}
	}

	return true;
}

void primbackup::update_map(LLUUID uploaded_asset)
{
	if(current_asset.isNull())
		return;

	assetmap.insert(std::pair<LLUUID,LLUUID>(current_asset,uploaded_asset));
	llinfos << "Mapping "<<current_asset<<" to "<<uploaded_asset<<llendl;

}


void myupload_new_resource(const LLTransactionID &tid, LLAssetType::EType asset_type,
						 std::string name,
						 std::string desc, S32 compression_info,
						 LLAssetType::EType destination_folder_type,
						 LLInventoryType::EType inv_type,
						 U32 next_owner_perm,
						 const std::string& display_name,
						 LLAssetStorage::LLStoreAssetCallback callback,
						 void *userdata)
{
	if(gDisconnected)
	{
		return ;
	}

	LLAssetID uuid = tid.makeAssetID(gAgent.getSecureSessionID());	
	
	// At this point, we're ready for the upload.
	std::string upload_message = "Uploading...\n\n";
	upload_message.append(display_name);
	LLUploadDialog::modalUploadDialog(upload_message);

	std::string url = gAgent.getRegion()->getCapability("NewFileAgentInventory");
	if (!url.empty())
	{
		LLSD body;
		body["folder_id"] = gInventory.findCategoryUUIDForType((destination_folder_type == LLAssetType::AT_NONE) ? asset_type : destination_folder_type);
		body["asset_type"] = LLAssetType::lookup(asset_type);
		body["inventory_type"] = LLInventoryType::lookup(inv_type);
		body["name"] = name;
		body["description"] = desc;
		
		std::ostringstream llsdxml;
		LLSDSerialize::toXML(body, llsdxml);
		lldebugs << "posting body to capability: " << llsdxml.str() << llendl;
		//LLHTTPClient::post(url, body, new LLNewAgentInventoryResponder(body, uuid, asset_type));
		LLHTTPClient::post(url, body, new importResponder(body, uuid, asset_type));

	}
	else
	{
		llinfos << "NewAgentInventory capability not found, FUCK!" << llendl;	
	}
}



void primbackup::upload_next_asset()
{
	if(textures.empty())
	{
		llinfos<<" Texture list is empty, moving to rez statge"<< llendl;
		current_asset=LLUUID::null;
		import_object1a();
		return;
	}

	this->updateimportnumbers();

	std::list<LLUUID>::iterator iter;
	iter=textures.begin();
	LLUUID id=(*iter);
	textures.pop_front();

	llinfos<<"Got texture ID "<<id<< "trying to upload"<<llendl;

	current_asset=id;
	std::string struid;
	id.toString(struid);
	std::string filename=folder+"//"+struid;
	

	LLAssetID uuid;
	LLTransactionID tid;

	// gen a new transaction ID for this asset
	tid.generate();
	uuid = tid.makeAssetID(gAgent.getSecureSessionID());

	S32 file_size;
	apr_file_t* fp;
    LLAPRFile aFile;
    aFile.open(filename, LL_APR_RB, LLAPRFile::global, &file_size);
    fp = aFile.getFileHandle();
	if (fp)
	{
		const S32 buf_size = 65536;	
		U8 copy_buf[buf_size];
		LLVFile file(gVFS, uuid,  LLAssetType::AT_TEXTURE, LLVFile::WRITE);
		file.setMaxSize(file_size);
		
		while ((file_size =aFile.read(copy_buf, buf_size)))
		{
			file.write(copy_buf, file_size);
		}
		aFile.close();
	}
	else
	{
		llwarns<<"Unable to access output file "<<filename<<llendl;
		upload_next_asset();
		return;
	}

	 myupload_new_resource(
	 tid, LLAssetType::AT_TEXTURE, struid,
		struid, 0,
		LLAssetType::AT_TEXTURE, 
		 LLInventoryType::defaultForAssetType(LLAssetType::AT_TEXTURE),
		 0x0,
		 "Uploaded texture",
		 NULL,
		 NULL);


}

