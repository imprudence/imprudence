
#include "llviewerprecompiledheaders.h"
#include "llviewermenu.h" 


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
#include "llviewercontrol.h"
#include "llassetuploadresponders.h"
#include "lleconomy.h"
#include "llhttpclient.h"
#include "lluploaddialog.h"
#include "lldir.h"
#include "llinventorymodel.h"	// gInventory
#include "llviewercontrol.h"	// gSavedSettings
#include "llviewermenu.h"	// gMenuHolder
#include "llagent.h"
#include "llfilepicker.h"
#include "llfloateranimpreview.h"
#include "llfloaterbuycurrency.h"
#include "llfloaterimagepreview.h"
#include "llfloaternamedesc.h"
#include "llfloatersnapshot.h"
#include "llinventorymodel.h"	// gInventory
#include "llresourcedata.h"
#include "llstatusbar.h"
#include "llviewercontrol.h"	// gSavedSettings
#include "llviewerimagelist.h"
#include "lluictrlfactory.h"
#include "llviewermenu.h"	// gMenuHolder
#include "llviewerregion.h"
#include "llviewerstats.h"
#include "llviewerwindow.h"
#include "llappviewer.h"
#include "lluploaddialog.h"
// Included to allow LLTextureCache::purgeTextures() to pause watchdog timeout
#include "llappviewer.h" 
#include "lltransactiontypes.h"

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

/*

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


////////////////////////////////////
				// if (save targas) -Patrick Sapinski (Thursday, September 24, 2009)
				LLPointer<LLImageTGA> image_tga = new LLImageTGA;
				LLPointer<LLImageRaw> raw = new LLImageRaw;
				
				mFormattedImage->decode(raw, 0);

				if( !image_tga->encode( raw ) )
				{
					LLStringUtil::format_map_t args;
					args["[FILE]"] = name;
					gViewerWindow->alertXml("CannotEncodeFile", args);
				}
				else if( !image_tga->save( primbackup::getInstance()->getfolder()+"//textures//"+name + ".tga" ) )
				{
					LLStringUtil::format_map_t args;
					args["[FILE]"] = name;
					gViewerWindow->alertXml("CannotWriteFile", args);
				}

////////////////////////////////////////


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



*/
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
	xml = new LLXMLNode("group", FALSE);
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

//	export_state=EXPORT_INIT;
	//gIdleCallbacks.addFunction(exportworker, NULL);
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


/*
void primbackup::exportworker(void *userdata)
{	
	primbackup::getInstance()->updateexportnumbers();

	switch(primbackup::getInstance()->export_state)
	{
		case EXPORT_INIT:
			{
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
					primbackup::getInstance()->export_state=EXPORT_STRUCTURE;
				else
				{
					llwarns<<"Incorrect permission to export"<<llendl;
					primbackup::getInstance()->export_state=EXPORT_DONE;
					primbackup::getInstance()->close();
					gIdleCallbacks.deleteFunction(exportworker);
					LLSelectMgr::getInstance()->getSelection()->unref();

				}
				break;
			}

			break;
		case EXPORT_STRUCTURE:
		{
			struct ff : public LLSelectedObjectFunctor
			{
				virtual bool apply(LLViewerObject* object)
				{
					object->boostTexturePriority(TRUE);
					LLViewerObject::child_list_t children = object->getChildren();
					children.push_front(object); //push root onto list
#if 0
					LLSD prim_llsd=primbackup::getInstance()->prims_to_llsd(children);				
					LLSD stuff;
					stuff["root_position"] = object->getPosition().getValue();
					stuff["root_rotation"] = ll_sd_from_quaternion(object->getRotation());
					stuff["group_body"] = prim_llsd;
					primbackup::getInstance()->llsd["data"].append(stuff);
#endif

 					
					//primbackup::getInstance()->xml->createChild("root_position", FALSE)->setValue(object->getPosition().getValue());
					//primbackup::getInstance()->xml->createChild("root_rotation", FALSE)->setValue(ll_sd_from_quaternion(object->getRotation()));
					
					LLBBox bbox	= LLSelectMgr::getInstance()->getBBoxOfSelection();

					LLXMLNodePtr max_xml = primbackup::getInstance()->xml->createChild("max", FALSE);
					LLVector3 max = object->getPosition();
					max_xml->createChild("x", TRUE)->setValue(llformat("%.5f", max.mV[VX]));
					max_xml->createChild("y", TRUE)->setValue(llformat("%.5f", max.mV[VY]));
					max_xml->createChild("z", TRUE)->setValue(llformat("%.5f", max.mV[VZ]));

					LLXMLNodePtr min_xml = primbackup::getInstance()->xml->createChild("min", FALSE);
					LLVector3 min = object->getPosition();
					min_xml->createChild("x", TRUE)->setValue(llformat("%.5f", min.mV[VX]));
					min_xml->createChild("y", TRUE)->setValue(llformat("%.5f", min.mV[VY]));
					min_xml->createChild("z", TRUE)->setValue(llformat("%.5f", min.mV[VZ]));
					
					LLXMLNodePtr center_xml = primbackup::getInstance()->xml->createChild("center", FALSE);
					LLVector3 center = object->getPosition();
					center_xml->createChild("x", TRUE)->setValue(llformat("%.5f", center.mV[VX]));
					center_xml->createChild("y", TRUE)->setValue(llformat("%.5f", center.mV[VY]));
					center_xml->createChild("z", TRUE)->setValue(llformat("%.5f", center.mV[VZ]));


 					//primbackup::getInstance()->xml->createChild("max", FALSE)->setValue(object->getPosition().getValue());
 					//primbackup::getInstance()->xml->createChild("min", FALSE)->setValue(object->getPosition().getValue());
 					//primbackup::getInstance()->xml->createChild("center", FALSE)->setValue(object->getPosition().getValue());


					primbackup::getInstance()->xml->addChild(primbackup::getInstance()->prims_to_xml(children));
					return true;
				}
			} func;

			primbackup::getInstance()->export_state=EXPORT_LLSD;
			LLSelectMgr::getInstance()->getSelection()->applyToRootObjects(&func,false);
			LLSelectMgr::getInstance()->getSelection()->unref();

			break;
		}
		case EXPORT_TEXTURES:
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

		case EXPORT_LLSD:
			{
				// Create a file stream and write to it
				llofstream export_file(primbackup::getInstance()->file_name);
				LLSDSerialize::toPrettyXML(primbackup::getInstance()->llsd, export_file);
				export_file.close();

#if 1
//-Patrick Sapinski (Thursday, September 17, 2009)

		llofstream out(primbackup::getInstance()->file_name);
		
		if (!out.good())
		{
			llwarns << "Unable to open for output." << llendl;
		}

		out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";

		
		LLXMLNode *temp_xml = new LLXMLNode("project", FALSE);

		temp_xml->createChild("schema", FALSE)->setValue("1.0");
		temp_xml->createChild("name", FALSE)->setValue(primbackup::getInstance()->file_name);
		temp_xml->createChild("date", FALSE)->setValue("test");
		temp_xml->createChild("sotware", FALSE)->setValue("Meerkat 0.3.0");
		temp_xml->createChild("platform", FALSE)->setValue("Second Life");
		temp_xml->createChild("grid", FALSE)->setValue("test");
		
		temp_xml->addChild(primbackup::getInstance()->xml);

			/*
			  <schema>1.0</schema>
			  <name>TextureTest</name>
			  <date>7/8/2009 12:46 AM</date>
			  <software>Maxproxy 1.3.2</software>
			  <creator>3b520582-1d15-41c6-8849-afdd1b2528bd</creator>
			  <platform>OpenSim</platform>
			  <grid>unknown</grid>
			  *//*


		temp_xml->writeToOstream(out);
		out.close();
	///////////////////////////////////////////////
#endif

				primbackup::getInstance()->m_nexttextureready=true;	
				primbackup::getInstance()->export_state=EXPORT_TEXTURES;
			}
			break;
		case EXPORT_DONE:
					llinfos<<"Backup complete"<<llendl
					gIdleCallbacks.deleteFunction(exportworker);
					primbackup::getInstance()->close();
			break;
	}
}

LLXMLNode *primbackup::prims_to_xml(LLViewerObject::child_list_t child_list)
{

	LLViewerObject* object;
	LLXMLNode *xml = new LLXMLNode("linkset", FALSE);

	char localid[16];

	for (LLViewerObject::child_list_t::iterator i = child_list.begin(); i != child_list.end(); ++i)
	{
		object=(*i);
		LLUUID id = object->getID();

		llinfos << "Exporting prim " << object->getID().asString() << llendl;
				
		std::string selected_item	= "box";
		F32 scale_x=1.f, scale_y=1.f;
		
		const LLVolumeParams &volume_params = object->getVolume()->getParams();

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

		if (object->getParameterEntryInUse(LLNetworkData::PARAMS_SCULPT))
		{
			selected_item = "sculpt";
		}

		// Create an LLSD object that represents this prim. It will be injected in to the overall LLSD
		// tree structure
		LLXMLNode *prim_xml = new LLXMLNode(selected_item.c_str(), FALSE);

		if (!object->isRoot())
		{

			// Parent id
			snprintf(localid, sizeof(localid), "%u", object->getSubParent()->getLocalID());
 			prim_xml->createChild("uuid", FALSE)->setValue(localid);
		}


		LLSelectNode* node = LLSelectMgr::getInstance()->getSelection()->findNode(object);
		if (node)
		{
 			prim_xml->createChild("name", FALSE)->setValue("<![CDATA[" + node->mName + "]]>");
 			prim_xml->createChild("description", FALSE)->setValue("<![CDATA[" + node->mDescription + "]]>");
		}

		// Transforms		
		LLXMLNodePtr position_xml = prim_xml->createChild("position", FALSE);
		LLVector3 position;
		position.setVec(object->getPositionRegion());
		position_xml->createChild("x", TRUE)->setValue(llformat("%.5f", position.mV[VX]));
		position_xml->createChild("y", TRUE)->setValue(llformat("%.5f", position.mV[VY]));
		position_xml->createChild("z", TRUE)->setValue(llformat("%.5f", position.mV[VZ]));

		LLXMLNodePtr scale_xml = prim_xml->createChild("size", FALSE);
		LLVector3 scale = object->getScale();
		scale_xml->createChild("x", TRUE)->setValue(llformat("%.5f", scale.mV[VX]));
		scale_xml->createChild("y", TRUE)->setValue(llformat("%.5f", scale.mV[VY]));
		scale_xml->createChild("z", TRUE)->setValue(llformat("%.5f", scale.mV[VZ]));

		LLXMLNodePtr rotation_xml = prim_xml->createChild("rotation", FALSE);
		LLQuaternion rotation = object->getRotationEdit();
		rotation_xml->createChild("x", TRUE)->setValue(llformat("%.5f", rotation.mQ[VX]));
		rotation_xml->createChild("y", TRUE)->setValue(llformat("%.5f", rotation.mQ[VY]));
		rotation_xml->createChild("z", TRUE)->setValue(llformat("%.5f", rotation.mQ[VZ]));
		rotation_xml->createChild("w", TRUE)->setValue(llformat("%.5f", rotation.mQ[VW]));

		// Flags
		if(object->flagPhantom())
		{
			LLXMLNodePtr shadow_xml = prim_xml->createChild("phantom", FALSE);
			shadow_xml->createChild("val", TRUE)->setValue("true");
		}

		if(object->mFlags & FLAGS_USE_PHYSICS)
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
		if (object->isFlexible())
		{  //FIXME
			LLFlexibleObjectData* flex = (LLFlexibleObjectData*)object->getParameterEntry(LLNetworkData::PARAMS_FLEXIBLE);
			prim_xml->createChild("flexible", FALSE)->setValue(flex->asLLSD());
		}
		
		// Light
		if (object->getParameterEntryInUse(LLNetworkData::PARAMS_LIGHT))
		{
			LLLightParams* light = (LLLightParams*)object->getParameterEntry(LLNetworkData::PARAMS_LIGHT);

			//<light>
			LLXMLNodePtr light_xml = prim_xml->createChild("light", FALSE);

			//<color r="255" g="255" b="255" />
			LLXMLNodePtr color_xml = light_xml->createChild("color", FALSE);
			LLColor4 color = light->getColor();
			color_xml->createChild("r", TRUE)->setValue(llformat("%u", color.mV[VRED]));
			color_xml->createChild("g", TRUE)->setValue(llformat("%u", color.mV[VGREEN]));
			color_xml->createChild("b", TRUE)->setValue(llformat("%u", color.mV[VBLUE]));

			//<intensity val="1.0" />
			LLXMLNodePtr intensity_xml = light_xml->createChild("intensity", FALSE);
			intensity_xml->createChild("val", TRUE)->setValue(llformat("%.5f", color.mV[VALPHA]));

			//<radius val="10.0" />
			LLXMLNodePtr radius_xml = light_xml->createChild("radius", FALSE);
			radius_xml->createChild("val", TRUE)->setValue(llformat("%.5f", light->getRadius()));

			//<falloff val="0.75" />
			LLXMLNodePtr falloff_xml = light_xml->createChild("falloff", FALSE);
			falloff_xml->createChild("val", TRUE)->setValue(llformat("%.5f", light->getFalloff()));

			//return light->getCutoff(); wtf is this?
		}

		// Sculpt
		if (object->getParameterEntryInUse(LLNetworkData::PARAMS_SCULPT))
		{
			LLSculptParams* sculpt = (LLSculptParams*)object->getParameterEntry(LLNetworkData::PARAMS_SCULPT);

			//<topology val="4" />
			LLXMLNodePtr topology_xml = prim_xml->createChild("topology", FALSE);
			topology_xml->createChild("val", TRUE)->setValue(llformat("%u", sculpt->getSculptType()));
			
			//<sculptmap_uuid>1e366544-c287-4fff-ba3e-5fafdba10272</sculptmap_uuid>
			//<sculptmap_file>apple_map.tga</sculptmap_file>
			//FIXME png/tga/j2c selection itt.
			prim_xml->createChild("sculptmap_file", FALSE)->setValue(llformat("%s", "testing"));
			prim_xml->createChild("sculptmap_uuid", FALSE)->setValue(llformat("%s", "testing"));

			//prim_xml->createChild("sculptmap_file", FALSE)->setValue(llformat("%s", sculpt->getSculptTexture()));
			//prim_xml->createChild("sculptmap_uuid", FALSE)->setValue(llformat("%s", sculpt->getSculptTexture()));

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

		//<texture>
		LLXMLNodePtr texture_xml = prim_xml->createChild("texture", FALSE);

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

			//<face id=0>
			LLXMLNodePtr face_xml = texture_xml->createChild("face", FALSE);
			//This may be a hack, but it's ok since we're not using id in this code. We set id differently because for whatever reason
			//llxmlnode filters a few parameters including ID. -Patrick Sapinski (Friday, September 25, 2009)
			face_xml->mID = llformat("%d", i);

			//<tile u="-1" v="1" />
			//object->getTE(face)->mScaleS
			//object->getTE(face)->mScaleT
			LLXMLNodePtr tile_xml = face_xml->createChild("tile", FALSE);
			tile_xml->createChild("u", TRUE)->setValue(llformat("%.5f", object->getTE(i)->mScaleS));
			tile_xml->createChild("v", TRUE)->setValue(llformat("%.5f", object->getTE(i)->mScaleT));

			//<offset u="0" v="0" />
			//object->getTE(face)->mOffsetS
			//object->getTE(face)->mOffsetT
			LLXMLNodePtr offset_xml = face_xml->createChild("offset", FALSE);
			offset_xml->createChild("u", TRUE)->setValue(llformat("%.5f", object->getTE(i)->mOffsetS));
			offset_xml->createChild("v", TRUE)->setValue(llformat("%.5f", object->getTE(i)->mOffsetT));


			//<rotation w="0" />
			//object->getTE(face)->mRotation
			LLXMLNodePtr rotation_xml = face_xml->createChild("rotation", FALSE);
			rotation_xml->createChild("w", TRUE)->setValue(llformat("%.5f", object->getTE(i)->mRotation));


			//<image_file><![CDATA[76a0319a-e963-44b0-9f25-127815226d71.tga]]></image_file>
			//<image_uuid>76a0319a-e963-44b0-9f25-127815226d71</image_uuid>
			LLUUID texture = object->getTE(i)->getID();
			std::string uuid_string;
			object->getTE(i)->getID().toString(uuid_string);
			
			face_xml->createChild("image_file", FALSE)->setValue("<![CDATA[" + uuid_string + ".tga]]>");
			face_xml->createChild("image_uuid", FALSE)->setValue(uuid_string);


			//<color r="255" g="255" b="255" />
			LLXMLNodePtr color_xml = face_xml->createChild("color", FALSE);
			LLColor4 color = object->getTE(i)->getColor();
			color_xml->createChild("r", TRUE)->setValue(llformat("%.5f", color.mV[VRED]));
			color_xml->createChild("g", TRUE)->setValue(llformat("%.5f", color.mV[VGREEN]));
			color_xml->createChild("b", TRUE)->setValue(llformat("%.5f", color.mV[VBLUE]));

			//<transparency val="0" />
			LLXMLNodePtr transparency_xml = face_xml->createChild("transparency", FALSE);
			transparency_xml->createChild("val", TRUE)->setValue(llformat("%.5f", color.mV[VALPHA]));

			//<glow val="0" />
			//object->getTE(face)->getGlow()
			LLXMLNodePtr glow_xml = face_xml->createChild("glow", FALSE);
			glow_xml->createChild("val", TRUE)->setValue(llformat("%.5f", object->getTE(i)->getGlow()));

			//<fullbright val="false" />
			//object->getTE(face)->getFullbright()
			if(object->getTE(i)->getFullbright())
			{
				LLXMLNodePtr fullbright_xml = face_xml->createChild("fullbright", FALSE);
				fullbright_xml->createChild("val", TRUE)->setValue("true");
			}
				
			//<shine val="0" />
			//object->getTE(face)->getShiny()
			if (object->getTE(i)->getShiny())
			{
				LLXMLNodePtr shine_xml = face_xml->createChild("shine", FALSE);
				shine_xml->createChild("val", TRUE)->setValue("1");
			}
				
			//<bump val="0" />
			//object->getTE(face)->getBumpmap()
			if (object->getTE(i)->getBumpmap())
			{
				LLXMLNodePtr bumpmap_xml = face_xml->createChild("bumpmap", FALSE);
				bumpmap_xml->createChild("val", TRUE)->setValue("1");
			}
				
			//<mapping val="0" />

			if(alreadyseen==false)
				textures.push_back(object->getTE(i)->getID());
		}

		// The keys in the primitive maps do not have to be localids, they can be any
		// string. We simply use localids because they are a unique identifier
		snprintf(localid, sizeof(localid), "%u", object->getLocalID());
		//put prim_xml inside of xml? -Patrick Sapinski (Thursday, September 17, 2009)
		//llsd[(const char*)localid] = prim_llsd;
		xml->addChild(prim_xml);

	}

	updateexportnumbers();

	return xml;
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

/*
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
				if(imagep->getBoostLevel()!=LLViewerImage::BOOST_PREVIEW)
					imagep->setBoostLevel(LLViewerImage::BOOST_PREVIEW); //we want to force discard 0 this one does this.
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

*/

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

	llifstream import_file(file_name);
	LLSDSerialize::fromXML(llsd, import_file);
	import_file.close();

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
	mPlacer->placeObject((S32)offset.mV[0], (S32)offset.mV[1],(S32)offset.mV[2]);
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
/*
	S32 file_size;
	apr_file_t* fp = ll_apr_file_open(filename, LL_APR_RB, &file_size);
	if (fp)
	{
		const S32 buf_size = 65536;	
		U8 copy_buf[buf_size];
		LLVFile file(gVFS, uuid,  LLAssetType::AT_TEXTURE, LLVFile::WRITE);
		file.setMaxSize(file_size);
		
		while ((file_size = ll_apr_file_read(fp, copy_buf, buf_size)))
		{
			file.write(copy_buf, file_size);
		}
		apr_file_close(fp);
	}
	else
	{
		llwarns<<"Unable to access output file "<<filename<<llendl;
		upload_next_asset();
		return;
	}*/ 

	 myupload_new_resource(
	 tid, LLAssetType::AT_TEXTURE, struid,
		struid, 0,
		LLAssetType::AT_TEXTURE, 
		 LLInventoryType::defaultForAssetType(LLAssetType::AT_TEXTURE),
		 0x0,
		 "Uploaded texture",
		 NULL,
		 NULL);
return;

}

