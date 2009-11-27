/** 
 * @file importtracker.cpp
 * @brief A utility for importing linksets from XML.
 * Discrete wuz here
 * HPA support added -Patrick Sapinski (Thursday, November 12, 2009)
 */

#include "llviewerprecompiledheaders.h"

#include "llagent.h"
#include "llfilepicker.h"
#include "llframetimer.h"
#include "llprimitive.h"
#include "llviewerregion.h"
#include "llvolumemessage.h"
#include "llchat.h"
#include "importtracker.h"
#include "llsdserialize.h"
#include "lltooldraganddrop.h"
#include "llassetuploadresponders.h"
#include "lleconomy.h"
#include "lluictrlfactory.h"
#include "llviewerimagelist.h"
#include "llagent.h"
#include "llsurface.h"
#include "llspinctrl.h"
#include "llfocusmgr.h"

//#include "llfloaterperms.h"


#include "llviewertexteditor.h"
#include "llsdutil.h"
#include "lluploaddialog.h"

//
// Constants
//
enum {
	MI_BOX,
	MI_CYLINDER,
	MI_PRISM,
	MI_SPHERE,
	MI_TORUS,
	MI_TUBE,
	MI_RING,
	MI_SCULPT,
	MI_NONE,
	MI_VOLUME_COUNT
};

enum {
	MI_HOLE_SAME,
	MI_HOLE_CIRCLE,
	MI_HOLE_SQUARE,
	MI_HOLE_TRIANGLE,
	MI_HOLE_COUNT
};

const F32 CONTEXT_CONE_IN_ALPHA = 0.5f;
const F32 CONTEXT_CONE_OUT_ALPHA = 1.f;
	F32 mContextConeOpacity = 1.0f;

ImportTracker gImportTracker;

extern LLAgent gAgent;

ImportTrackerFloater* ImportTrackerFloater::sInstance = 0;
int ImportTrackerFloater::total_objects = 0;
int ImportTrackerFloater::objects_imported = 0;
int ImportTrackerFloater::total_linksets = 0;
int ImportTrackerFloater::linksets_imported = 0;
int ImportTrackerFloater::textures_imported = 0;
int ImportTrackerFloater::total_assets = 0;
int ImportTrackerFloater::assets_imported = 0;
int ImportTrackerFloater::assets_uploaded = 0;

void ImportTrackerFloater::draw()
{
	//X,Y,Z label colors
	const LLColor4	white(	1.0f,	1.0f,	1.0f,	1);
	const LLColor4	red(	1.0f,	0.25f,	0.f,	1);
	const LLColor4	green(	0.f,	1.0f,	0.f,	1);
	const LLColor4	blue(	0.f,	0.5f,	1.0f,	1);
	mCtrlPosX	->setLabelColor(red);
	mCtrlPosY	->setLabelColor(green);
	mCtrlPosZ	->setLabelColor(blue);

	LLFloater::draw();
	LLRect rec  = getChild<LLPanel>("sim_icon")->getRect();
			
	S32 mapsize = rec.getWidth() / 2; //Radius of the map

	LLViewerRegion *regionp = gAgent.getRegion();
	F32 right = rec.getCenterX() + mapsize;
	F32 left = rec.getCenterX() - mapsize;
	F32 top = rec.getCenterY() + mapsize;
	F32 bottom = rec.getCenterY() - mapsize;

	gGL.pushMatrix();
	gGL.color4fv(LLColor4::white.mV);
	// Draw using texture.
	gGL.getTexUnit(0)->bind(regionp->getLand().getSTexture());
	gGL.begin(LLRender::QUADS);
		gGL.texCoord2f(0.f, 1.f);
		gGL.vertex2f(left, top);
		gGL.texCoord2f(0.f, 0.f);
		gGL.vertex2f(left, bottom);
		gGL.texCoord2f(1.f, 0.f);
		gGL.vertex2f(right, bottom);
		gGL.texCoord2f(1.f, 1.f);
		gGL.vertex2f(right, top);
	gGL.end();
	
	// Draw water
	gGL.setAlphaRejectSettings(LLRender::CF_GREATER, ABOVE_WATERLINE_ALPHA / 255.f);
	{
		if (regionp->getLand().getWaterTexture())
		{
			gGL.getTexUnit(0)->bind(regionp->getLand().getWaterTexture());
			gGL.begin(LLRender::QUADS);
				gGL.texCoord2f(0.f, 1.f);
				gGL.vertex2f(left, top);
				gGL.texCoord2f(0.f, 0.f);
				gGL.vertex2f(left, bottom);
				gGL.texCoord2f(1.f, 0.f);
				gGL.vertex2f(right, bottom);
				gGL.texCoord2f(1.f, 1.f);
				gGL.vertex2f(right, top);
			gGL.end();
		}
	}
	gGL.setAlphaRejectSettings(LLRender::CF_DEFAULT); 

	F32 x = rec.getCenterX() - mapsize + (((gImportTracker.importoffset.mV[VX] + gImportTracker.importposition.mV[VX]) / 256) * mapsize * 2);
	F32 y = rec.getCenterY() - mapsize + (((gImportTracker.importoffset.mV[VY] + gImportTracker.importposition.mV[VY]) / 256) * mapsize * 2);

	F32 scaled_x = (gImportTracker.size.mV[VX] / 256) * mapsize;
	F32 scaled_y = (gImportTracker.size.mV[VY] / 256) * mapsize;

	if (scaled_x < 3)
		scaled_x = 3;
	if (scaled_y < 3)
		scaled_y = 3;

	gl_line_2d ( left, y, right, y, red );
	gl_line_2d ( x, top, x, bottom, green );

	right = x + scaled_x;
	left = x - scaled_x;
	top = y + scaled_y;
	bottom = y - scaled_y;

	gl_drop_shadow(left, top, right, bottom, LLColor4(0,0,0,0.5), 4);
	gl_rect_2d(left,top,right,bottom, LLColor4(1,1,1,0.5) , TRUE);
	gGL.color4fv(LLColor4::black.mV);
	gl_rect_2d(left,top + 1,right + 1,bottom, FALSE);

	gGL.popMatrix();

	std::string status_text;
	if (gImportTracker.state == gImportTracker.IDLE)
		status_text = "idle";
	else if (gImportTracker.state == gImportTracker.BUILDING)
		status_text = "building";
	else if (gImportTracker.state == gImportTracker.LINKING)
		status_text = "linking";
	else if (gImportTracker.state == gImportTracker.POSITIONING)
		status_text = "positioning";

	//is this a bad place for this function? -Patrick Sapinski (Friday, November 13, 2009)
	sInstance->getChild<LLTextBox>("status label")->setValue(
		"Status: " + status_text
		+  llformat("\nObjects: %u/%u",objects_imported,total_objects)
		+  llformat(" Linksets: %u/%u",linksets_imported,total_linksets)
		+  llformat("\nTextures: %u/%u",textures_imported,gImportTracker.uploadtextures.size())
		+  llformat(" Contents: %u/%u",assets_imported,gImportTracker.asset_insertions)
		);
}

ImportTrackerFloater::ImportTrackerFloater()
:	LLFloater( std::string("Prim Import Floater") )
{
	LLUICtrlFactory::getInstance()->buildFloater( this, "floater_prim_import.xml" );

	childSetAction("reset", onClickReset, this);
	childSetAction("import", onClickImport, this);
	childSetAction("close", onClickClose, this);

	// Position
	mCtrlPosX = getChild<LLSpinCtrl>("Pos X");
	childSetCommitCallback("Pos X",onCommitPosition,this);
	mCtrlPosY = getChild<LLSpinCtrl>("Pos Y");
	childSetCommitCallback("Pos Y",onCommitPosition,this);
	mCtrlPosZ = getChild<LLSpinCtrl>("Pos Z");
	childSetCommitCallback("Pos Z",onCommitPosition,this);

	/* I *think* this is unused... -Patrick Sapinski (Wednesday, November 18, 2009)
	LLBBox bbox = LLSelectMgr::getInstance()->getBBoxOfSelection();
	LLVector3 box_center_agent = bbox.getCenterAgent();
	
	LLVector3 temp = bbox.getExtentLocal();

	std::stringstream sstr;	
	LLUICtrl * ctrl=this->getChild<LLUICtrl>("selection size");	

	sstr <<"X: "<<llformat("%.2f", temp.mV[VX]);
	sstr <<", Y: "<<llformat("%.2f", temp.mV[VY]);
	sstr <<", Z: "<<llformat("%.2f", temp.mV[VZ]);

	ctrl->setValue(LLSD("Text")=sstr.str());
	*/
}

ImportTrackerFloater* ImportTrackerFloater::getInstance()
{
    if ( ! sInstance )
        sInstance = new ImportTrackerFloater();

	return sInstance;
}

ImportTrackerFloater::~ImportTrackerFloater()
{
	// save position of floater
	gSavedSettings.setRect("FloaterPrimImport", getRect());

	//which one?? -Patrick Sapinski (Wednesday, November 11, 2009)
	ImportTrackerFloater::sInstance = NULL;
	sInstance = NULL;
}

void ImportTrackerFloater::close()
{
	if(sInstance)
	{
		delete sInstance;
		sInstance = NULL;
	}
}

void ImportTrackerFloater::show()
{
	// Open the file open dialog
	LLFilePicker& file_picker = LLFilePicker::instance();
	if( !file_picker.getOpenFile( LLFilePicker::FFLOAD_HPA ) )
	{
		// User canceled save.
		return;
	}
		 
	const std::string filename = file_picker.getFirstFile().c_str();
	 gImportTracker.loadhpa(filename);

	if(NULL==sInstance) 
	{
		sInstance = new ImportTrackerFloater();
		llwarns << "sInstance assigned. sInstance=" << sInstance << llendl;
	}
	
	sInstance->open();	/*Flawfinder: ignore*/
}

BOOL ImportTrackerFloater::handleMouseDown(S32 x, S32 y, MASK mask)
{
	LLRect rec  = getChild<LLPanel>("sim_icon")->getRect();

	if(rec.pointInRect(x, y))
	{
		gFocusMgr.setMouseCapture(this);
	}
	
	return LLFloater::handleMouseDown(x,y,mask);
}

//-----------------------------------------------------------------------------
// handleMouseUp()
//-----------------------------------------------------------------------------
BOOL ImportTrackerFloater::handleMouseUp(S32 x, S32 y, MASK mask)
{
	gFocusMgr.setMouseCapture(FALSE);
	return LLFloater::handleMouseUp(x, y, mask);
}

//-----------------------------------------------------------------------------
// handleHover()
//-----------------------------------------------------------------------------
BOOL ImportTrackerFloater::handleHover(S32 x, S32 y, MASK mask)
{
	if (hasMouseCapture())
	{
		LLRect rec  = getChild<LLPanel>("sim_icon")->getRect();
		rec.clampPointToRect(x, y);
		S32 mapsize = rec.getWidth() / 2; //Radius of the map
		gImportTracker.importoffset.mV[VX] = ((F32)x - rec.getCenterX() + mapsize) / 200 * 256 - gImportTracker.importposition.mV[VX];
		gImportTracker.importoffset.mV[VY] = ((F32)y - rec.getCenterY() + mapsize) / 200 * 256 - gImportTracker.importposition.mV[VY];
		sInstance->mCtrlPosX->set(gImportTracker.importposition.mV[VX] + gImportTracker.importoffset.mV[VX]);
		sInstance->mCtrlPosY->set(gImportTracker.importposition.mV[VY] + gImportTracker.importoffset.mV[VY]);

	}

	return TRUE;
}

// static
void ImportTrackerFloater::onCommitPosition( LLUICtrl* ctrl, void* userdata )
{
	sInstance->sendPosition();
}

// static
void ImportTrackerFloater::onClickReset(void* data)
{
	gImportTracker.importoffset.clear();
	sInstance->mCtrlPosX->set(gImportTracker.importposition.mV[VX]);
	sInstance->mCtrlPosY->set(gImportTracker.importposition.mV[VY]);
	sInstance->mCtrlPosZ->set(gImportTracker.importposition.mV[VZ]);
}

// static
void ImportTrackerFloater::onClickImport(void* data)
{
	gImportTracker.currentimportoffset = gImportTracker.importoffset;


	//let's upload our textures.
	if (gSavedSettings.getBOOL("ImportTextures"))
	{
		gImportTracker.upload_next_asset();
	}
	else
		gImportTracker.importer("bean man", NULL);

}

// static
void ImportTrackerFloater::onClickClose(void* data)
{
	sInstance->close();
}

void ImportTrackerFloater::sendPosition()
{	
	LLVector3 newpos(mCtrlPosX->get(), mCtrlPosY->get(), mCtrlPosZ->get());
	gImportTracker.importoffset = newpos - gImportTracker.importposition;
}

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
	//LLUploadDialog::modalUploadFinished();
	
	gImportTracker.update_map(content["new_asset"].asUUID());
	ImportTrackerFloater::textures_imported++;
	gImportTracker.upload_next_asset();

	}

};

void ImportTracker::loadhpa(std::string file)
{
	filepath = file;
	linksets = 0;
	textures = 0;
	objects = 0;
	S32 total_linksets = 0;
	ImportTrackerFloater::textures_imported = 0;

	std::string xml_filename = file;
	asset_dir = gDirUtilp->getDirName(filepath);	

	ImportTrackerFloater::sInstance->getChild<LLTextBox>("file label")->setValue("File: " + gDirUtilp->getBaseFileName(xml_filename, false));

	LLXmlTree xml_tree;

	if (!xml_tree.parseFile(xml_filename))
	{
		llwarns << "Problem reading HPA file: " << xml_filename << llendl;
		return;
	}
	
	LLXmlTreeNode* root = xml_tree.getRoot();

	for (LLXmlTreeNode* child = root->getFirstChild(); child; child = root->getNextChild())
	{
		//if (child->hasName("schema"))
			//cmdline_printchat("Version: "+child->getTextContents());

		if (child->hasName("name"))
			ImportTrackerFloater::sInstance->getChild<LLTextBox>("name label")->setValue("Name: " + child->getTextContents());
		
		if (child->hasName("date"))
			ImportTrackerFloater::sInstance->getChild<LLTextBox>("date label")->setValue("Date: " + child->getTextContents());

		//if (child->hasName("software"))
			//cmdline_printchat("Software: "+child->getTextContents());

		//if (child->hasName("platform"))
			//cmdline_printchat("Platform: "+child->getTextContents());

		if (child->hasName("grid"))
			ImportTrackerFloater::sInstance->getChild<LLTextBox>("grid label")->setValue("Grid: " + child->getTextContents());
		
		if (child->hasName("group"))
		{
			for (LLXmlTreeNode* object = child->getFirstChild(); object; object = child->getNextChild())
			{
				LLSD ls_llsd;
				if (object->hasName("center"))
				{
					object->getAttributeF32("x", importposition.mV[VX]);
					object->getAttributeF32("y", importposition.mV[VY]);
					object->getAttributeF32("z", importposition.mV[VZ]);
				}
				else if (object->hasName("max"))
				{
					object->getAttributeF32("x", size.mV[VX]);
					object->getAttributeF32("y", size.mV[VY]);
					object->getAttributeF32("z", size.mV[VZ]);
				}
				else if (object->hasName("linkset"))
				{
					U32 totalprims = 0;
					S32 object_index = 0;
					LLXmlTreeNode* prim = object->getFirstChild();

					while ((object_index < object->getChildCount()))
					{
						object_index++;

						LLSD prim_llsd;
						LLVolumeParams volume_params;
						std::string name, description;
						LLSD prim_scale, prim_pos, prim_rot;
						F32 shearx = 0.f, sheary = 0.f;
						F32 taperx = 0.f, tapery = 0.f;
						S32 selected_type = MI_BOX;
						S32 selected_hole = 1;
						F32 cut_begin = 0.f;
						F32 cut_end = 1.f;
						F32 adv_cut_begin = 0.f;
						F32 adv_cut_end = 1.f;
						F32 hollow = 0.f;
						F32 twist_begin = 0.f;
						F32 twist = 0.f;
						F32 scale_x=1.f, scale_y=1.f;
						LLUUID sculpttexture;
						U8 topology = 0;
						U8 type = 0;
						LLPCode pcode = 0;

						if (prim->hasName("box"))
							selected_type = MI_BOX;
						else if (prim->hasName("cylinder"))
							selected_type = MI_CYLINDER;
						else if (prim->hasName("prism"))
							selected_type = MI_PRISM;
						else if (prim->hasName("sphere"))
							selected_type = MI_SPHERE;
						else if (prim->hasName("torus"))
							selected_type = MI_TORUS;
						else if (prim->hasName("tube"))
							selected_type = MI_TUBE;
						else if (prim->hasName("ring"))
							selected_type = MI_RING;
						else if (prim->hasName("sculpt"))
							selected_type = MI_SCULPT;
						else if (prim->hasName("tree"))
							pcode = LL_PCODE_LEGACY_TREE;
						else if (prim->hasName("grass"))
							pcode = LL_PCODE_LEGACY_GRASS;

						//COPY PASTE FROM LLPANELOBJECT
						// Figure out what type of volume to make
						U8 profile;
						U8 path;
						switch ( selected_type )
						{
						case MI_CYLINDER:
							profile = LL_PCODE_PROFILE_CIRCLE;
							path = LL_PCODE_PATH_LINE;
							break;

						case MI_BOX:
							profile = LL_PCODE_PROFILE_SQUARE;
							path = LL_PCODE_PATH_LINE;
							break;

						case MI_PRISM:
							profile = LL_PCODE_PROFILE_EQUALTRI;
							path = LL_PCODE_PATH_LINE;
							break;

						case MI_SPHERE:
							profile = LL_PCODE_PROFILE_CIRCLE_HALF;
							path = LL_PCODE_PATH_CIRCLE;
							break;

						case MI_TORUS:
							profile = LL_PCODE_PROFILE_CIRCLE;
							path = LL_PCODE_PATH_CIRCLE;
							break;

						case MI_TUBE:
							profile = LL_PCODE_PROFILE_SQUARE;
							path = LL_PCODE_PATH_CIRCLE;
							break;

						case MI_RING:
							profile = LL_PCODE_PROFILE_EQUALTRI;
							path = LL_PCODE_PATH_CIRCLE;
							break;

						case MI_SCULPT:
							profile = LL_PCODE_PROFILE_CIRCLE;
							path = LL_PCODE_PATH_CIRCLE;
							break;
							
						default:
							llwarns << "Unknown base type " << selected_type 
								<< " in getVolumeParams()" << llendl;
							// assume a box
							selected_type = MI_BOX;
							profile = LL_PCODE_PROFILE_SQUARE;
							path = LL_PCODE_PATH_LINE;
							break;
						}

						for (LLXmlTreeNode* param = prim->getFirstChild(); param; param = prim->getNextChild())
						{
							//<name><![CDATA[Object]]></name>
							if (param->hasName("name"))
								name = param->getTextContents();
							//<description><![CDATA[]]></description>
							else if (param->hasName("description"))
								description = param->getTextContents();
							//<position x="115.80774" y="30.13144" z="41.09710" />
							else if (param->hasName("position"))
							{
								LLVector3 vec;
								param->getAttributeF32("x", vec.mV[VX]);
								param->getAttributeF32("y", vec.mV[VY]);
								param->getAttributeF32("z", vec.mV[VZ]);
								prim_pos.append((F64)vec.mV[VX]);
								prim_pos.append((F64)vec.mV[VY]);
								prim_pos.append((F64)vec.mV[VZ]);
							}
							//<size x="0.50000" y="0.50000" z="0.50000" />
							else if (param->hasName("size"))
							{
								LLVector3 vec;
								param->getAttributeF32("x", vec.mV[VX]);
								param->getAttributeF32("y", vec.mV[VY]);
								param->getAttributeF32("z", vec.mV[VZ]);
								prim_scale.append((F64)vec.mV[VX]);
								prim_scale.append((F64)vec.mV[VY]);
								prim_scale.append((F64)vec.mV[VZ]);
							}
							//<rotation w="1.00000" x="0.00000" y="0.00000" z="0.00000" />
							else if (param->hasName("rotation"))
							{
								LLQuaternion quat;
								param->getAttributeF32("w", quat.mQ[VW]);
								param->getAttributeF32("x", quat.mQ[VX]);
								param->getAttributeF32("y", quat.mQ[VY]);
								param->getAttributeF32("z", quat.mQ[VZ]);
								prim_rot.append((F64)quat.mQ[VX]);
								prim_rot.append((F64)quat.mQ[VY]);
								prim_rot.append((F64)quat.mQ[VZ]);
								prim_rot.append((F64)quat.mQ[VW]);

							}
							//<top_shear x="0.00000" y="0.00000" />
							else if (param->hasName("top_shear"))
							{
								param->getAttributeF32("x", shearx);
								param->getAttributeF32("y", sheary);
							}
							//<taper x="0.00000" y="0.00000" />
							else if (param->hasName("taper"))
							{
								// Check if we need to change top size/hole size params.
								switch (selected_type)
								{
								case MI_SPHERE:
								case MI_TORUS:
								case MI_TUBE:
								case MI_RING:
									param->getAttributeF32("x", taperx);
									param->getAttributeF32("y", tapery);
									break;
								default:
									param->getAttributeF32("x", scale_x);
									param->getAttributeF32("y", scale_y);
									scale_x = 1.f - scale_x;
									scale_y = 1.f - scale_y;
									break;
								}
							}
							//<hole_size x="1.00000" y="0.05000" />
							else if (param->hasName("hole_size"))
							{
								param->getAttributeF32("x", scale_x);
								param->getAttributeF32("y", scale_y);
							}
							//<profile_cut begin="0.22495" end="0.77499" />
							else if (param->hasName("profile_cut"))
							{
								param->getAttributeF32("begin", adv_cut_begin);
								param->getAttributeF32("end", adv_cut_end);
							}
							//<path_cut begin="0.00000" end="1.00000" />
							else if (param->hasName("path_cut"))
							{
								param->getAttributeF32("begin", cut_begin);
								param->getAttributeF32("end", cut_end);
							}
							//<twist begin="0.00000" end="0.00000" />
							else if (param->hasName("twist"))
							{
								param->getAttributeF32("begin", twist_begin);
								param->getAttributeF32("end", twist);
							}
							//<hollow amount="40.99900" shape="4" />
							else if (param->hasName("hollow"))
							{
								param->getAttributeF32("amount", hollow);
								param->getAttributeS32("shape", selected_hole);
							}
							//<topology val="1" />
							else if (param->hasName("topology"))
								param->getAttributeU8("val", topology);
							//<sculptmap_uuid>be293869-d0d9-0a69-5989-ad27f1946fd4</sculptmap_uuid>
							else if (param->hasName("sculptmap_uuid"))
							{
								sculpttexture = LLUUID(param->getTextContents());

								bool alreadyseen=false;
								std::list<LLUUID>::iterator iter;
								for(iter = uploadtextures.begin(); iter != uploadtextures.end() ; iter++) 
								{
									if( (*iter)==sculpttexture)
										alreadyseen=true;
								}
								if(alreadyseen==false)
								{
									llinfos << "Found a sculpt texture, adding to list "<<sculpttexture<<llendl;
									uploadtextures.push_back(sculpttexture);
								}
							}
							
							//<type val="3" />
							else if (param->hasName("type"))
							{
								param->getAttributeU8("val", type);
							}

							//<light>
							else if (param->hasName("light"))
							{
								F32 lightradius = 0,  lightfalloff = 0;
								LLColor4 lightcolor;

								for (LLXmlTreeNode* lightparam = param->getFirstChild(); lightparam; lightparam = param->getNextChild())
								{
									//<color b="0" g="0" r="0" />
									if (lightparam->hasName("color"))
									{
										lightparam->getAttributeF32("r", lightcolor.mV[VRED]);
										lightparam->getAttributeF32("g", lightcolor.mV[VGREEN]);
										lightparam->getAttributeF32("b", lightcolor.mV[VBLUE]);
										lightcolor.mV[VRED]/=256;
										lightcolor.mV[VGREEN]/=256;
										lightcolor.mV[VBLUE]/=256;
									}
									//<intensity val="0.80392" />
									else if (lightparam->hasName("intensity"))
									{
											lightparam->getAttributeF32("val", lightcolor.mV[VALPHA]);
									}
									//<radius val="0.80392" />
									else if (lightparam->hasName("radius"))
									{
											lightparam->getAttributeF32("val", lightradius);
									}
									//<falloff val="0.80392" />
									else if (lightparam->hasName("falloff"))
									{
											lightparam->getAttributeF32("val", lightfalloff);
									}
								}

								LLLightParams light;
								light.setColor(lightcolor);
								light.setRadius(lightradius);
								light.setFalloff(lightfalloff);
								//light.setCutoff(lightintensity);

								prim_llsd["light"] = light.asLLSD();
							}

							//<texture>
							else if (param->hasName("texture"))
							{
								LLSD textures;
								S32 texture_count = 0;

								//cmdline_printchat("texture found");
								for (LLXmlTreeNode* face = param->getFirstChild(); face; face = param->getNextChild())
								{
									LLTextureEntry thisface;

									//<face id="0">
									for (LLXmlTreeNode* param = face->getFirstChild(); param; param = face->getNextChild())
									{
										//<tile u="1.00000" v="-0.90000" />
										if (param->hasName("tile"))
										{
											F32 u,v;
											param->getAttributeF32("u", u);
											param->getAttributeF32("v", v);
											thisface.setScale(u,v);
										}
										//<offset u="0.00000" v="0.00000" />
										else if (param->hasName("offset"))
										{
											F32 u,v;
											param->getAttributeF32("u", u);
											param->getAttributeF32("v", v);
											thisface.setOffset(u,v);
										}
										//<rotation w="0.00000" />
										else if (param->hasName("rotation"))
										{
											F32 temp;
											param->getAttributeF32("w", temp);
											thisface.setRotation(temp * DEG_TO_RAD);
										}
										//<image_file><![CDATA[87008270-fe87-bf2a-57ea-20dc6ecc4e6a.tga]]></image_file>
										else if (param->hasName("image_file"))
										{
										}
										//<image_uuid>87008270-fe87-bf2a-57ea-20dc6ecc4e6a</image_uuid>
										else if (param->hasName("image_uuid"))
										{
											LLUUID temp = LLUUID(param->getTextContents());
											thisface.setID(temp);

											bool alreadyseen=false;
											std::list<LLUUID>::iterator iter;
											for(iter = uploadtextures.begin(); iter != uploadtextures.end() ; iter++) 
											{
												if( (*iter)==temp)
													alreadyseen=true;
											}
											if(alreadyseen==false)
											{
												llinfos << "Found a surface texture, adding to list "<<temp<<llendl;
												uploadtextures.push_back(temp);
											}
										}
										//<color b="1.00000" g="1.00000" r="1.00000" />
										else if (param->hasName("color"))
										{
											LLColor3 color;
											param->getAttributeF32("r", color.mV[VRED]);
											param->getAttributeF32("g", color.mV[VGREEN]);
											param->getAttributeF32("b", color.mV[VBLUE]);
											thisface.setColor(LLColor3(color.mV[VRED]/255.f,color.mV[VGREEN]/255.f,color.mV[VBLUE]/255.f));
										}
										//<transparency val="1.00000" />
										else if (param->hasName("transparency"))
										{
											F32 temp;
											param->getAttributeF32("val", temp);
											thisface.setAlpha((100.f - temp) / 100.f);
										}
										//<glow val="0.00000" />
										else if (param->hasName("glow"))
										{
											F32 temp;
											param->getAttributeF32("val", temp);
											thisface.setGlow(temp);
										}
										//<fullbright val="true" />
										else if (param->hasName("fullbright"))
										{
											int temp = 0;
											std::string value;
											param->getAttributeString("val", value);
											if (value == "true")
												temp = 1;
											thisface.setFullbright(temp);
										}
										//<shiny val="true" />
										else if (param->hasName("shine"))
										{
											U8 shiny;
											param->getAttributeU8("val", shiny);
											thisface.setShiny(shiny);
										}
									}
									textures[texture_count] = thisface.asLLSD();
									texture_count++;
								}
								prim_llsd["textures"] = textures;
									
							}
							
							//<inventory>
							else if (param->hasName("inventory"))
							{
								LLSD inventory;
								S32 inventory_count = 0;

								for (LLXmlTreeNode* item = param->getFirstChild(); item; item = param->getNextChild())
								{
									LLSD sd;

									//<item>
									for (LLXmlTreeNode* param = item->getFirstChild(); param; param = item->getNextChild())
									{
										//<description>2008-01-29 05:01:19 note card</description>
										if (param->hasName("description"))
											sd["desc"] = param->getTextContents();
										//<item_id>673b00e8-990f-3078-9156-c7f7b4a5f86c</item_id>
										else if (param->hasName("item_id"))
											sd["item_id"] = param->getTextContents();
										//<name>blah blah</name>
										else if (param->hasName("name"))
											sd["name"] = param->getTextContents();
										//<type>notecard</type>
										else if (param->hasName("type"))
											sd["type"] = param->getTextContents();
									}
									inventory[inventory_count] = sd;
									inventory_count++;
								}
								prim_llsd["inventory"] = inventory;
									
							}
						}
						
						U8 hole;
						switch (selected_hole)
						{
						case 3: 
							hole = LL_PCODE_HOLE_CIRCLE;
							break;
						case 2:
							hole = LL_PCODE_HOLE_SQUARE;
							break;
						case 4:
							hole = LL_PCODE_HOLE_TRIANGLE;
							break;
						case 1:
						default:
							hole = LL_PCODE_HOLE_SAME;
							break;
						}

						volume_params.setType(profile | hole, path);
						//mSelectedType = selected_type;
						
						// Compute cut start/end

						// Make sure at least OBJECT_CUT_INC of the object survives
						if (cut_begin > cut_end - OBJECT_MIN_CUT_INC)
						{
							cut_begin = cut_end - OBJECT_MIN_CUT_INC;
						}

						// Make sure at least OBJECT_CUT_INC of the object survives
						if (adv_cut_begin > adv_cut_end - OBJECT_MIN_CUT_INC)
						{
							adv_cut_begin = adv_cut_end - OBJECT_MIN_CUT_INC;
						}

						F32 begin_s, end_s;
						F32 begin_t, end_t;

						if (selected_type == MI_SPHERE || selected_type == MI_TORUS || 
							selected_type == MI_TUBE   || selected_type == MI_RING)
						{
							begin_s = adv_cut_begin;
							end_s	= adv_cut_end;

							begin_t = cut_begin;
							end_t	= cut_end;
						}
						else
						{
							begin_s = cut_begin;
							end_s	= cut_end;

							begin_t = adv_cut_begin;
							end_t	= adv_cut_end;
						}

						volume_params.setBeginAndEndS(begin_s, end_s);
						volume_params.setBeginAndEndT(begin_t, end_t);

						// Hollowness
						hollow = hollow/100.f;
						if (  selected_hole == MI_HOLE_SQUARE && 
							( selected_type == MI_CYLINDER || selected_type == MI_TORUS ||
							  selected_type == MI_PRISM    || selected_type == MI_RING  ||
							  selected_type == MI_SPHERE ) )
						{
							if (hollow > 0.7f) hollow = 0.7f;
						}

						volume_params.setHollow( hollow );

						// Twist Begin,End
						// Check the path type for twist conversion.
						if (path == LL_PCODE_PATH_LINE || path == LL_PCODE_PATH_FLEXIBLE)
						{
							twist_begin	/= OBJECT_TWIST_LINEAR_MAX;
							twist		/= OBJECT_TWIST_LINEAR_MAX;
						}
						else
						{
							twist_begin	/= OBJECT_TWIST_MAX;
							twist		/= OBJECT_TWIST_MAX;
						}

						volume_params.setTwistBegin(twist_begin);
						volume_params.setTwist(twist);

						volume_params.setRatio( scale_x, scale_y );
//						volume_params.setSkew(skew);
						volume_params.setTaper( taperx, tapery );
//						volume_params.setRadiusOffset(radius_offset);
//						volume_params.setRevolutions(revolutions);

						// Shear X,Y
						volume_params.setShear( shearx, sheary );

						if (selected_type == MI_SCULPT)
						{
							LLSculptParams sculpt;

							sculpt.setSculptTexture(sculpttexture);

							/* maybe we want the mirror/invert/etc data at some point?
							U8 sculpt_type = 0;
							
							if (mCtrlSculptType)
								sculpt_type |= mCtrlSculptType->getCurrentIndex();

							if ((mCtrlSculptMirror) && (mCtrlSculptMirror->get()))
								sculpt_type |= LL_SCULPT_FLAG_MIRROR;

							if ((mCtrlSculptInvert) && (mCtrlSculptInvert->get()))
								sculpt_type |= LL_SCULPT_FLAG_INVERT; */
							
							sculpt.setSculptType(topology);

							prim_llsd["sculpt"] = sculpt.asLLSD();
						}

						//we should have all our params by now, pack the LLSD.
						prim_llsd["position"] = prim_pos;
						prim_llsd["rotation"] = prim_rot;

						prim_llsd["scale"] = prim_scale;
						// Flags
						//prim_llsd["shadows"] = object->flagCastShadows();
						//prim_llsd["phantom"] = object->flagPhantom();
						//prim_llsd["physical"] = (BOOL)(object->mFlags & FLAGS_USE_PHYSICS);

						if (pcode == LL_PCODE_LEGACY_GRASS || pcode == LL_PCODE_LEGACY_TREE)
						{
							prim_llsd["pcode"] = pcode;
							prim_llsd["state"] = type;
						}
						else
							// Volume params
							prim_llsd["volume"] = volume_params.asLLSD();


						
						totalprims += 1;
						prim = object->getNextChild();

						//LLSD temp;
						//temp["Object"] = prim_llsd;
						// Changed to use link numbers zero-indexed.
						ls_llsd[object_index - 1] = prim_llsd;
					}

/* fix later, flexi support!
		if ((path == LL_PCODE_PATH_LINE) || (selected_type == MI_SCULPT))
		{
			LLVOVolume *volobjp = (LLVOVolume *)(LLViewerObject*)(mObject);
			if (volobjp->isFlexible())
			{
				path = LL_PCODE_PATH_FLEXIBLE;
			}
		}
*/
					
					LLSD temp;
					temp["Object"] = ls_llsd;
					linksets[total_linksets] = temp;
					total_linksets++; //we call total_linksets++ twice in this loop for some reason
									  //and it won't work without both calls I wish I understood this! -Patrick Sapinski (Monday, November 16, 2009)

					//we should have the proper LLSD structure by now

				}
			}
			//We've looped through the HPA now let's fill in some values.
			ImportTrackerFloater::sInstance->mCtrlPosX->set(importposition.mV[VX]);
			ImportTrackerFloater::sInstance->mCtrlPosY->set(importposition.mV[VY]);
			ImportTrackerFloater::sInstance->mCtrlPosZ->set(importposition.mV[VZ]);
			size = (size - importposition) * 2;
			importoffset.clear();
			ImportTrackerFloater::sInstance->getChild<LLTextBox>("size label")->setValue("Size: " + llformat("%.3f,%.3f,%.3f", size.mV[VX], size.mV[VY], size.mV[VZ]));
			ImportTrackerFloater::total_linksets = total_linksets;
			ImportTrackerFloater::linksets_imported = 0;
		}
	}
}

void ImportTracker::importer(std::string file,  void (*callback)(LLViewerObject*))
{
	mDownCallback = callback;
	asset_insertions = 0;

	llifstream importer(file);
	LLSD data;
	LLSDSerialize::fromXMLDocument(data, importer);

	LLSD file_data = data["Objects"];
	data = LLSD();

	//filepath = file;

	//HPA hackin
	//linksetgroups=file_data;
	linksetgroups=linksets;

	//DEBUG CODE! At this point our HPA should be in Emerald LLSD format.
	// Create a file stream and write to it
	llofstream export_file(filepath + ".in.llsd");
	LLSDSerialize::toPrettyXML(linksetgroups, export_file);
	// Open the file save dialog
	export_file.close();


	//llinfos << "LOADED LINKSETS, PREPARING.." << llendl;
	groupcounter=0;
	LLSD ls_llsd;
	ls_llsd=linksetgroups[groupcounter]["Object"];
	linksetoffset=linksetgroups[groupcounter]["ObjectPos"];
	initialPos=importposition + importoffset;
	//initialPos=gAgent.getCameraPositionAgent();
	import(ls_llsd);
}

void ImportTracker::import(LLSD& ls_data)
{
	if(!(linkset.size()))
		if(!(linksetgroups.size()))
			initialPos=importposition + importoffset;
			//initialPos=gAgent.getCameraPositionAgent();
	linkset = ls_data;
	ImportTrackerFloater::total_objects = linkset.size();
	updated=0;
	LLSD rot = linkset[0]["rotation"];
	rootrot.mQ[VX] = (F32)(rot[0].asReal());
	rootrot.mQ[VY] = (F32)(rot[1].asReal());
	rootrot.mQ[VZ] = (F32)(rot[2].asReal());
	rootrot.mQ[VW] = (F32)(rot[3].asReal());
	state = BUILDING;
	//llinfos << "IMPORTED, BUILDING.." << llendl;
	plywood_above_head();
}

void ImportTracker::expectRez()
{
	numberExpected++;
	state = WAND;
	//llinfos << "EXPECTING CUBE..." << llendl;
}

void ImportTracker::clear()
{
	if(linkset.isDefined())lastrootid = linkset[0]["LocalID"].asInteger();
	localids.clear();
	linkset.clear();
	state = IDLE;
	finish();
}
void cmdline_printchat(std::string message);
LLViewerObject* find(U32 local)
{
	S32 i;
	S32 total = gObjectList.getNumObjects();

	for (i = 0; i < total; i++)
	{
		LLViewerObject *objectp = gObjectList.getObject(i);
		if (objectp)
		{
			if(objectp->getLocalID() == local)return objectp;
		}
	}
	return NULL;
}
void ImportTracker::finish()
{
	if(asset_insertions == 0)
	{
		if(lastrootid != 0)
		{
			if(mDownCallback)
			{
				LLViewerObject* objectp = find(lastrootid);
				mDownCallback(objectp);
			}
			//cmdline_printchat("import completed");
			ImportTrackerFloater::linksets_imported++;
		}
	}
}

void ImportTracker::cleargroups()
{
	linksetgroups.clear();
	groupcounter=0;
	linksetoffset=LLVector3(0.0f,0.0f,0.0f);
	initialPos=LLVector3(0.0f,0.0f,0.0f);
	state = IDLE;
}

void ImportTracker::cleanUp()
{
	clear();
	if(linksetgroups.size())
	{
		if(groupcounter < (linksetgroups.size() - 1))
		{
			++groupcounter;
			LLSD ls_llsd;
			ls_llsd=linksetgroups[groupcounter]["Object"];
			linksetoffset=linksetgroups[groupcounter]["ObjectPos"];
			import(ls_llsd);
		}
	}
	else cleargroups();
}

void ImportTracker::get_update(S32 newid, BOOL justCreated, BOOL createSelected)
{
	switch (state)
	{
		//lgg crap
		case WAND:
			if(justCreated && createSelected)
			{
				numberExpected--;
				if(numberExpected<=0)
					state=IDLE;
				LLMessageSystem* msg = gMessageSystem;
				msg->newMessageFast(_PREHASH_ObjectImage);
				msg->nextBlockFast(_PREHASH_AgentData);
				msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
				msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());	
				msg->nextBlockFast(_PREHASH_ObjectData);				
				msg->addU32Fast(_PREHASH_ObjectLocalID,  (U32)newid);
				msg->addStringFast(_PREHASH_MediaURL, NULL);
	
				LLPrimitive obj;
				obj.setNumTEs(U8(10));	
				/*S32 shinnyLevel = 0;
				if(gSavedSettings.getString("EmeraldBuildPrefs_Shiny")== "None") shinnyLevel = 0;
				if(gSavedSettings.getString("EmeraldBuildPrefs_Shiny")== "Low") shinnyLevel = 1;
				if(gSavedSettings.getString("EmeraldBuildPrefs_Shiny")== "Medium") shinnyLevel = 2;
				if(gSavedSettings.getString("EmeraldBuildPrefs_Shiny")== "High") shinnyLevel = 3;
				*/
				for (int i = 0; i < 10; i++)
				{
					LLTextureEntry tex;// =  LLTextureEntry(LLUUID(gSavedSettings.getString("EmeraldBuildPrefs_Texture")));
					//tex.setColor(gSavedSettings.getColor4("EmeraldBuildPrefs_Color"));
					//tex.setAlpha(1.0 - ((gSavedSettings.getF32("EmeraldBuildPrefs_Alpha")) / 100.0));
					tex.setGlow(0.5f);
					//if(gSavedSettings.getBOOL("EmeraldBuildPrefs_FullBright"))
					//{
					//	tex.setFullbright(TEM_FULLBRIGHT_MASK);
					//}
									
					//tex.setShiny((U8) shinnyLevel & TEM_SHINY_MASK);
					
					obj.setTE(U8(i), tex);
				} //*/
	
				obj.packTEMessage(gMessageSystem);
	
				msg->sendReliable(gAgent.getRegion()->getHost());
				
				msg->newMessage("ObjectFlagUpdate");
				msg->nextBlockFast(_PREHASH_AgentData);
				msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID() );
				msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
				msg->addU32Fast(_PREHASH_ObjectLocalID, (U32)newid );
				//msg->addBOOLFast(_PREHASH_UsePhysics, gSavedSettings.getBOOL("EmeraldBuildPrefs_Physical"));
				//msg->addBOOL("IsTemporary", gSavedSettings.getBOOL("EmeraldBuildPrefs_Temporary"));
				//msg->addBOOL("IsPhantom", gSavedSettings.getBOOL("EmeraldBuildPrefs_Phantom") );
				//msg->addBOOL("CastsShadows", true );
				msg->sendReliable(gAgent.getRegion()->getHost());				

				//llinfos << "LGG SENDING CUBE TEXTURE.." << llendl;
			}
		break;
		case BUILDING:
			
			if (justCreated && (int)localids.size() < linkset.size())
			{
				localids.push_back(newid);
				localids.sort();
				localids.unique();

				linkset[localids.size() -1]["LocalID"] = newid;
				LLSD prim = linkset[localids.size() -1];

				//MAKERIGHT
				if (!(prim).has("Updated"))
				{
					++updated;
					ImportTrackerFloater::objects_imported = updated;

					// here be trees
					LLPCode pcode = prim["pcode"].asInteger();
					if (pcode == LL_PCODE_LEGACY_GRASS || pcode == LL_PCODE_LEGACY_TREE)
					{
						/* this doesnt work, I think we need to DeRez. This code was supposed to delete the extra 
						object we end up with when importing trees.

						LLMessageSystem* msg = gMessageSystem;
						msg->newMessageFast(_PREHASH_ObjectDelete);
						msg->nextBlockFast(_PREHASH_AgentData);
						msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
						msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
						const U8 NO_FORCE = 0;
						msg->addU8Fast(_PREHASH_Force, NO_FORCE);
						msg->nextBlockFast(_PREHASH_ObjectData);
						msg->addU32Fast(_PREHASH_ObjectLocalID, prim["LocalID"].asInteger());
						msg->sendReliable(gAgent.getRegion()->getHost()); */

						U8 type = prim["state"].asInteger();
						LLVector3 scale = prim["scale"];
						LLQuaternion rotation = ll_quaternion_from_sd(prim["rotation"]);
						LLVector3 pos = prim["position"];

						//ugly hack!
						LLMessageSystem* msg = gMessageSystem;
						msg->newMessageFast(_PREHASH_ObjectAdd);
						msg->nextBlockFast(_PREHASH_AgentData);
						msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
						msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
						msg->addUUIDFast(_PREHASH_GroupID, gAgent.getGroupID());
						msg->nextBlockFast(_PREHASH_ObjectData);
						msg->addU8Fast(_PREHASH_Material, 3);
						msg->addU32Fast(_PREHASH_AddFlags, 0);
						LLVolumeParams	volume_params;
						volume_params.setType(0x01, 0x10);
						volume_params.setBeginAndEndS(0.f, 1.f);
						volume_params.setBeginAndEndT(0.f, 1.f);
						volume_params.setRatio(1, 1);
						volume_params.setShear(0, 0);
						LLVolumeMessage::packVolumeParams(&volume_params, msg);
						msg->addU8Fast(_PREHASH_PCode, pcode);
						msg->addVector3Fast(_PREHASH_Scale, scale);
						msg->addQuatFast(_PREHASH_Rotation, rotation);
						LLViewerRegion *region = gAgent.getRegion();
						
						if (!localids.size())
							root = (initialPos + linksetoffset);
						
						msg->addVector3Fast(_PREHASH_RayStart, pos + currentimportoffset);
						msg->addVector3Fast(_PREHASH_RayEnd, pos + currentimportoffset);
						msg->addU8Fast(_PREHASH_BypassRaycast, (U8)TRUE );
						msg->addU8Fast(_PREHASH_RayEndIsIntersection, (U8)FALSE );
						msg->addU8Fast(_PREHASH_State, type);
						msg->addUUIDFast(_PREHASH_RayTargetID, LLUUID::null);
						msg->sendReliable(region->getHost());


					}
					else
					{
						send_shape(prim);
						send_image(prim);
						send_extras(prim);
						send_namedesc(prim);
						send_vectors(prim,updated);
						send_properties(prim, updated);
						send_inventory(prim);
					}
					(prim)["Updated"] = true;
				}
				if ((int)localids.size() < linkset.size())
				{
					LLSelectMgr::getInstance()->deselectAll();
					plywood_above_head();
					return;
				}
				else
				{
					if (updated >= linkset.size())
					{
						updated=0;
						llinfos << "FINISHED BUILDING, LINKING.." << llendl;
						state = LINKING;
						link();
					}
				}
			}
		break;
		case LINKING:
			link();
		break;
	}
}
struct InventoryImportInfo
{
	U32 localid;
	LLAssetType::EType type;
	LLInventoryType::EType inv_type;
	EWearableType wear_type;
	LLTransactionID tid;
	LLUUID assetid;
	std::string name;
	std::string description;
	bool compiled;
	std::string filename;
	U32 perms;
};

void insert(LLViewerInventoryItem* item, LLViewerObject* objectp, InventoryImportInfo* data)
{
	if(!item)
	{
		return;
	}
	if(objectp)
	{
		LLToolDragAndDrop::dropScript(objectp,
							item,
							TRUE,
							LLToolDragAndDrop::SOURCE_AGENT,
							gAgent.getID());
		cmdline_printchat("inserted.");
	}
	delete data;
	gImportTracker.asset_insertions -= 1;
	//ImportTrackerFloater::assets_imported++;
	if(gImportTracker.asset_insertions == 0)
	{
		gImportTracker.finish();
	}
}

class JCImportTransferCallback : public LLInventoryCallback
{
public:
	JCImportTransferCallback(InventoryImportInfo* idata)
	{
		data = idata;
	}
	void fire(const LLUUID &inv_item)
	{
		cmdline_printchat("fired transfer for "+inv_item.asString()+"|"+data->assetid.asString());
		LLViewerInventoryItem* item = (LLViewerInventoryItem*)gInventory.getItem(inv_item);
		LLViewerObject* objectp = find(data->localid);
		insert(item, objectp, data);
	}
private:
	InventoryImportInfo* data;
};

class JCImportInventoryResponder : public LLAssetUploadResponder
{
public:
	JCImportInventoryResponder(const LLSD& post_data,
								const LLUUID& vfile_id,
								LLAssetType::EType asset_type, InventoryImportInfo* idata) : LLAssetUploadResponder(post_data, vfile_id, asset_type)
	{
		data = idata;
	}

	JCImportInventoryResponder(const LLSD& post_data, const std::string& file_name,
											   LLAssetType::EType asset_type) : LLAssetUploadResponder(post_data, file_name, asset_type)
	{

	}
	virtual void uploadComplete(const LLSD& content)
	{
		LLPointer<LLInventoryCallback> cb = new JCImportTransferCallback(data);
		LLPermissions perm;
		LLUUID parent_id = gInventory.findCategoryUUIDForType(LLAssetType::AT_TRASH);

		create_inventory_item(gAgent.getID(), gAgent.getSessionID(),
			gInventory.findCategoryUUIDForType(LLAssetType::AT_TRASH), data->tid, data->name,
			data->description, data->type, LLInventoryType::defaultForAssetType(data->type), data->wear_type,
			PERM_ALL,
			cb);
		
	}
private:
	InventoryImportInfo* data;
};

class JCPostInvUploadResponder : public LLAssetUploadResponder
{
public:
	JCPostInvUploadResponder(const LLSD& post_data,
								const LLUUID& vfile_id,
								LLAssetType::EType asset_type, LLUUID item, InventoryImportInfo* idata) : LLAssetUploadResponder(post_data, vfile_id, asset_type)
	{
		item_id = item;
		data = idata;
	}

	JCPostInvUploadResponder(const LLSD& post_data,
								const std::string& file_name,
											   LLAssetType::EType asset_type) : LLAssetUploadResponder(post_data, file_name, asset_type)
	{
	}
	virtual void uploadComplete(const LLSD& content)
	{
		//ImportTrackerFloater::assets_uploaded++;
		cmdline_printchat("completed upload, inserting");
		LLViewerInventoryItem* item = (LLViewerInventoryItem*)gInventory.getItem(item_id);
		LLViewerObject* objectp = find(data->localid);
		insert(item, objectp, data);
	}
private:
	LLUUID item_id;
	InventoryImportInfo* data;
};

class JCPostInvCallback : public LLInventoryCallback
{
public:
	JCPostInvCallback(InventoryImportInfo* idata)
	{
		data = idata;
	}
	void fire(const LLUUID &inv_item)
	{
		S32 file_size;
		apr_file_t* fp = ll_apr_file_open(data->filename, LL_APR_RB, &file_size);

		if (fp)
		{
			cmdline_printchat("got file handle @ postinv");
			LLVFile file(gVFS, data->assetid, data->type, LLVFile::WRITE);
			file.setMaxSize(file_size);
			const S32 buf_size = 65536;
			U8 copy_buf[buf_size];
			while ((file_size = ll_apr_file_read(fp, copy_buf, buf_size)))
			{
				file.write(copy_buf, file_size);
			}
			switch(data->type)
			{
			case LLAssetType::AT_NOTECARD:
				cmdline_printchat("case notecard @ postinv");
				{
					/*LLViewerTextEditor* edit = new LLViewerTextEditor("",LLRect(0,0,0,0),S32_MAX,"");
					S32 size = gVFS->getSize(data->assetid, data->type);
					U8* buffer = new U8[size];
					gVFS->getData(data->assetid, data->type, buffer, 0, size);
					edit->setText(LLStringExplicit((char*)buffer));
					std::string card;
					edit->exportBuffer(card);
					cmdline_printchat("Encoded notecard");;
					edit->die();
					delete buffer;
					//buffer = new U8[card.size()];
					//size = card.size();
					//strcpy((char*)buffer,card.c_str());
					file.remove();
					LLVFile newfile(gVFS, data->assetid, data->type, LLVFile::APPEND);
					newfile.setMaxSize(size);
					newfile.write((const U8*)card.c_str(),size);*/
					//FAIL.



					std::string agent_url = gAgent.getRegion()->getCapability("UpdateNotecardAgentInventory");
					LLSD body;
					body["item_id"] = inv_item;
					cmdline_printchat("posting content as " + data->assetid.asString());
					LLHTTPClient::post(agent_url, body,
								new JCPostInvUploadResponder(body, data->assetid, data->type,inv_item,data));
				}
				break;
			case LLAssetType::AT_LSL_TEXT:
				cmdline_printchat("case lsltext @ postinv");
				{
					std::string url = gAgent.getRegion()->getCapability("UpdateScriptAgent");
					LLSD body;
					body["item_id"] = inv_item;
					S32 size = gVFS->getSize(data->assetid, data->type);
					U8* buffer = new U8[size];
					gVFS->getData(data->assetid, data->type, buffer, 0, size);
					std::string script((char*)buffer);
					BOOL domono = TRUE;
					if(script.find("//mono\n") != -1)
					{
						domono = TRUE;
					}else if(script.find("//lsl2\n") != -1)
					{
						domono = FALSE;
					}
					delete buffer;
					body["target"] = (domono == TRUE) ? "mono" : "lsl2";
					cmdline_printchat("posting content as " + data->assetid.asString());
					LLHTTPClient::post(url, body, new JCPostInvUploadResponder(body, data->assetid, data->type,inv_item,data));
				}
				break;
			default:
				break;
			}
		}
	}
private:
	InventoryImportInfo* data;
};

void JCImportInventorycallback(const LLUUID& uuid, void* user_data, S32 result, LLExtStat ext_status) // StoreAssetData callback (fixed)
{
	if(result == LL_ERR_NOERR)
	{
		cmdline_printchat("fired importinvcall for "+uuid.asString());
		InventoryImportInfo* data = (InventoryImportInfo*)user_data;

		LLPointer<LLInventoryCallback> cb = new JCImportTransferCallback(data);
		LLPermissions perm;
		LLUUID parent_id = gInventory.findCategoryUUIDForType(LLAssetType::AT_TRASH);

		create_inventory_item(gAgent.getID(), gAgent.getSessionID(),
			gInventory.findCategoryUUIDForType(LLAssetType::AT_TRASH), data->tid, data->name,
			data->description, data->type, LLInventoryType::defaultForAssetType(data->type), data->wear_type,
			PERM_ALL,
			cb);
	}else cmdline_printchat("err: "+std::string(LLAssetStorage::getErrorString(result)));
}


void ImportTracker::send_inventory(LLSD& prim)
{
	U32 local_id = prim["LocalID"].asInteger();
	if (prim.has("inventory"))
	{
		std::string assetpre = asset_dir + gDirUtilp->getDirDelimiter() + "inventory" + gDirUtilp->getDirDelimiter();
		LLSD inventory = prim["inventory"];
		for (LLSD::array_iterator inv = inventory.beginArray(); inv != inventory.endArray(); ++inv)
		{
			LLSD item = (*inv);
			InventoryImportInfo* data = new InventoryImportInfo;
			data->localid = local_id;
			LLTransactionID tid;
			tid.generate();
			LLUUID assetid = tid.makeAssetID(gAgent.getSecureSessionID());
			data->tid = tid;
			data->assetid = assetid;
			data->type = LLAssetType::lookup(item["type"].asString());////LLAssetType::EType(U32(item["type"].asInteger()));
			data->name = item["name"].asString();
			data->description = item["desc"].asString();
			if(item.has("item_id"))
			{
				cmdline_printchat("item id found");
				std::string filename = assetpre + item["item_id"].asString() + "." + item["type"].asString();
				//S32 file_size;
				//LLAPRFile infile ;
				//infile.open(filename, LL_APR_RB, NULL, &file_size);
				//apr_file_t* fp = infile.getFileHandle();
				//if(fp)
				if(LLFile::isfile(filename))
				{
					cmdline_printchat("file "+filename+" exists");
					data->filename = filename;
					//infile.close();
				}else
				{
					cmdline_printchat("file "+filename+" does not exist");
					delete data;
					continue;
				}
			}else
			{
				cmdline_printchat("item id not found");
				delete data;
				continue;
			}

			data->wear_type = NOT_WEARABLE;

			//if(data->type == LLAssetType::AT_LSL_TEXT)
			{
				data->inv_type = LLInventoryType::defaultForAssetType(data->type);
				//printchat("is script");
				data->compiled = false;
				//
				switch(data->type)
				{
				case LLAssetType::AT_TEXTURE:
				case LLAssetType::AT_TEXTURE_TGA:
					cmdline_printchat("case textures");
					{
						std::string url = gAgent.getRegion()->getCapability("NewFileAgentInventory");
						S32 file_size;
						apr_file_t* fp = ll_apr_file_open(data->filename, LL_APR_RB, &file_size);

						if (fp)
						{
							cmdline_printchat("got file handle");
							LLVFile file(gVFS, data->assetid, data->type, LLVFile::WRITE);
							file.setMaxSize(file_size);
							const S32 buf_size = 65536;
							U8 copy_buf[buf_size];
							while ((file_size = ll_apr_file_read(fp, copy_buf, buf_size)))
							{
								file.write(copy_buf, file_size);
							}
							LLSD body;
							body["folder_id"] = gInventory.findCategoryUUIDForType(LLAssetType::AT_TRASH);
							body["asset_type"] = LLAssetType::lookup(data->type);
							body["inventory_type"] = LLInventoryType::lookup(data->inv_type);
							body["name"] = data->name;
							body["description"] = data->description;
							body["next_owner_mask"] = LLSD::Integer(U32_MAX);
							body["group_mask"] = LLSD::Integer(U32_MAX);
							body["everyone_mask"] = LLSD::Integer(U32_MAX);
							body["expected_upload_cost"] = LLSD::Integer(LLGlobalEconomy::Singleton::getInstance()->getPriceUpload());
							cmdline_printchat("posting "+ data->assetid.asString());
							LLHTTPClient::post(url, body, new JCImportInventoryResponder(body, data->assetid, data->type,data));
							//error = TRUE;
						}
					}
					break;
				case LLAssetType::AT_CLOTHING:
				case LLAssetType::AT_BODYPART:
					cmdline_printchat("case cloth/bodypart");
					{
							S32 file_size;
							apr_file_t* fp = ll_apr_file_open(data->filename, LL_APR_RB, &file_size);

							if (fp)
							{
							cmdline_printchat("got file handle @ cloth");
							LLVFile file(gVFS, data->assetid, data->type, LLVFile::WRITE);
							file.setMaxSize(file_size);
							const S32 buf_size = 65536;
							U8 copy_buf[buf_size];
							while ((file_size = ll_apr_file_read(fp, copy_buf, buf_size)))
							{
								file.write(copy_buf, file_size);
							}

							LLFILE* fp = LLFile::fopen(data->filename, "rb");
							if(fp)//HACK LOL LOL LOL
							{
								LLWearable* wearable = new LLWearable(LLUUID::null);
								wearable->importFile( fp );
								//if (!res)
								{
									data->wear_type = wearable->getType();
								}
								delete wearable;
							}
							cmdline_printchat("storing "+data->assetid.asString());
							gAssetStorage->storeAssetData(data->tid, data->type,
												JCImportInventorycallback,
												(void*)data,
												FALSE,
												TRUE,
												FALSE);
						}
					}
					break;
				case LLAssetType::AT_NOTECARD:
					cmdline_printchat("case notecard");
					{
						//std::string agent_url = gAgent.getRegion()->getCapability("UpdateNotecardAgentInventory");
						LLPointer<LLInventoryCallback> cb = new JCPostInvCallback(data);
						LLPermissions perm;
						LLUUID parent_id = gInventory.findCategoryUUIDForType(LLAssetType::AT_TRASH);
						create_inventory_item(gAgent.getID(), gAgent.getSessionID(),
							gInventory.findCategoryUUIDForType(LLAssetType::AT_TRASH), LLTransactionID::tnull, data->name,
							data->description, data->type, LLInventoryType::defaultForAssetType(data->type), data->wear_type,
							PERM_ALL,
							cb);
					}
					break;
				case LLAssetType::AT_LSL_TEXT:
					cmdline_printchat("case LSL text");
					{
						LLPointer<LLInventoryCallback> cb = new JCPostInvCallback(data);
						LLPermissions perm;
						LLUUID parent_id = gInventory.findCategoryUUIDForType(LLAssetType::AT_TRASH);
						create_inventory_item(gAgent.getID(), gAgent.getSessionID(),
							gInventory.findCategoryUUIDForType(LLAssetType::AT_TRASH), LLTransactionID::tnull, data->name,
							data->description, data->type, LLInventoryType::defaultForAssetType(data->type), data->wear_type,
							PERM_ALL,
							cb);
					}
					break;
				case LLAssetType::AT_SCRIPT://this shouldn't happen as this is legacy shit
				case LLAssetType::AT_GESTURE://we don't import you atm...
				default:
					break;
				}
				asset_insertions += 1;
			}
		}
	ImportTrackerFloater::total_assets = asset_insertions;
	}
}

void ImportTracker::send_properties(LLSD& prim, int counter)
{
	if(prim.has("properties"))
	{
		if(counter == 1)//root only shit
		{
			//prim["LocalID"]
			LLMessageSystem* msg = gMessageSystem;
			msg->newMessageFast(_PREHASH_ObjectPermissions);
			msg->nextBlockFast(_PREHASH_AgentData);
			msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
			msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
			msg->nextBlockFast(_PREHASH_HeaderData);
			msg->addBOOLFast(_PREHASH_Override, FALSE);
			msg->nextBlockFast(_PREHASH_ObjectData);
			msg->addU32Fast(_PREHASH_ObjectLocalID, prim["LocalID"].asInteger());
			msg->addU8Fast(_PREHASH_Field,	PERM_NEXT_OWNER);
			msg->addBOOLFast(_PREHASH_Set,		PERM_ITEM_UNRESTRICTED);
			msg->addU32Fast(_PREHASH_Mask,		U32(atoi(prim["next_owner_mask"].asString().c_str())));
			/*msg->sendReliable(gAgent.getRegion()->getHost());

			//LLMessageSystem* msg = gMessageSystem;
			msg->newMessageFast(_PREHASH_ObjectPermissions);
			msg->nextBlockFast(_PREHASH_AgentData);
			msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
			msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
			msg->nextBlockFast(_PREHASH_HeaderData);
			msg->addBOOLFast(_PREHASH_Override, data->mOverride);*/
			msg->nextBlockFast(_PREHASH_ObjectData);
			msg->addU32Fast(_PREHASH_ObjectLocalID, prim["LocalID"].asInteger());
			msg->addU8Fast(_PREHASH_Field,	PERM_GROUP);
			msg->addBOOLFast(_PREHASH_Set,		PERM_ITEM_UNRESTRICTED);
			msg->addU32Fast(_PREHASH_Mask,		U32(atoi(prim["group_mask"].asString().c_str())));
			/*msg->sendReliable(gAgent.getRegion()->getHost());

			//LLMessageSystem* msg = gMessageSystem;
			msg->newMessageFast(_PREHASH_ObjectPermissions);
			msg->nextBlockFast(_PREHASH_AgentData);
			msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
			msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
			msg->nextBlockFast(_PREHASH_HeaderData);
			msg->addBOOLFast(_PREHASH_Override, data->mOverride);*/
			msg->nextBlockFast(_PREHASH_ObjectData);
			msg->addU32Fast(_PREHASH_ObjectLocalID, prim["LocalID"].asInteger());
			msg->addU8Fast(_PREHASH_Field,	PERM_EVERYONE);
			msg->addBOOLFast(_PREHASH_Set,		PERM_ITEM_UNRESTRICTED);
			msg->addU32Fast(_PREHASH_Mask,		U32(atoi(prim["everyone_mask"].asString().c_str())));
			msg->sendReliable(gAgent.getRegion()->getHost());

			msg->newMessageFast(_PREHASH_ObjectSaleInfo);
			
			msg->nextBlockFast(_PREHASH_AgentData);
			msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
			msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());

			msg->nextBlockFast(_PREHASH_ObjectData);
			msg->addU32Fast(_PREHASH_LocalID, prim["LocalID"].asInteger());
			LLSaleInfo sale_info;
			BOOL a;
			U32 b;
			sale_info.fromLLSD(prim["sale_info"],a,b);
			sale_info.packMessage(msg);
			msg->sendReliable(gAgent.getRegion()->getHost());

			//no facilities exist to send any other information at this time.
		}
	}
}

void ImportTracker::send_vectors(LLSD& prim,int counter)
{
	LLVector3 position = (LLVector3)prim["position"] + currentimportoffset;
	LLSD rot = prim["rotation"];
	LLQuaternion rotq;
	rotq.mQ[VX] = (F32)(rot[0].asReal());
	rotq.mQ[VY] = (F32)(rot[1].asReal());
	rotq.mQ[VZ] = (F32)(rot[2].asReal());
	rotq.mQ[VW] = (F32)(rot[3].asReal());
	LLVector3 rotation;
	//if(counter == 1)
		rotation = rotq.packToVector3();
	//else
		//rotation = (rotq * rootrot).packToVector3();
	LLVector3 scale = prim["scale"];
	U8 data[256];
	
	LLMessageSystem* msg = gMessageSystem;
	msg->newMessageFast(_PREHASH_MultipleObjectUpdate);
	msg->nextBlockFast(_PREHASH_AgentData);
	msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
	msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
	
	msg->nextBlockFast(_PREHASH_ObjectData);
	msg->addU32Fast(_PREHASH_ObjectLocalID, prim["LocalID"].asInteger());
	msg->addU8Fast(_PREHASH_Type, U8(0x01));
	htonmemcpy(&data[0], &(position.mV), MVT_LLVector3, 12);
	msg->addBinaryDataFast(_PREHASH_Data, data, 12);
	
	msg->nextBlockFast(_PREHASH_ObjectData);
	msg->addU32Fast(_PREHASH_ObjectLocalID, prim["LocalID"].asInteger());
	msg->addU8Fast(_PREHASH_Type, U8(0x02));
	htonmemcpy(&data[0], &(rotation.mV), MVT_LLQuaternion, 12); 
	msg->addBinaryDataFast(_PREHASH_Data, data, 12);
	
	msg->nextBlockFast(_PREHASH_ObjectData);
	msg->addU32Fast(_PREHASH_ObjectLocalID, prim["LocalID"].asInteger());
	msg->addU8Fast(_PREHASH_Type, U8(0x04));
	htonmemcpy(&data[0], &(scale.mV), MVT_LLVector3, 12); 
	msg->addBinaryDataFast(_PREHASH_Data, data, 12);
	
	msg->sendReliable(gAgent.getRegion()->getHost());
}

void ImportTracker::send_shape(LLSD& prim)
{
	LLMessageSystem* msg = gMessageSystem;
	msg->newMessageFast(_PREHASH_ObjectShape);
	msg->nextBlockFast(_PREHASH_AgentData);
	msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
	msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
	
	msg->nextBlockFast(_PREHASH_ObjectData);
	msg->addU32Fast(_PREHASH_ObjectLocalID, prim["LocalID"].asInteger());
	
	LLVolumeParams params;
	params.fromLLSD(prim["volume"]);
	LLVolumeMessage::packVolumeParams(&params, msg);
	
	msg->sendReliable(gAgent.getRegion()->getHost());
}

void ImportTracker::send_image(LLSD& prim)
{
	LLMessageSystem* msg = gMessageSystem;
	msg->newMessageFast(_PREHASH_ObjectImage);
	msg->nextBlockFast(_PREHASH_AgentData);
	msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
	msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
	
	msg->nextBlockFast(_PREHASH_ObjectData);
	msg->addU32Fast(_PREHASH_ObjectLocalID, prim["LocalID"].asInteger());
	msg->addStringFast(_PREHASH_MediaURL, NULL);
	
	LLPrimitive obj;
	LLSD tes = prim["textures"];
	obj.setNumTEs(U8(tes.size()));
	
	for (int i = 0; i < tes.size(); i++)
	{
		LLTextureEntry tex;
		tex.fromLLSD(tes[i]);

		if(assetmap[tex.getID()].notNull())
		{
			LLUUID replacment=assetmap[tex.getID()];
			tex.setID(replacment);
		}
		else
			cmdline_printchat("Failed to remap texture UUID " + tex.getID().asString());


		obj.setTE(U8(i), tex);
	}
	
	obj.packTEMessage(gMessageSystem);
	
	msg->sendReliable(gAgent.getRegion()->getHost());
}
void send_chat_from_viewer(std::string utf8_out_text, EChatType type, S32 channel);
void ImportTracker::send_extras(LLSD& prim)
{	
	LLMessageSystem* msg = gMessageSystem;
	msg->newMessageFast(_PREHASH_ObjectExtraParams);
	msg->nextBlockFast(_PREHASH_AgentData);
	msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID() );
	msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
	
	LLPrimitive obj;
	
	if (prim.has("flexible"))
	{
		LLFlexibleObjectData flexi;
		flexi.fromLLSD(prim["flexible"]);
		U8 tmp[MAX_OBJECT_PARAMS_SIZE];
		LLDataPackerBinaryBuffer dpb(tmp, MAX_OBJECT_PARAMS_SIZE);
		
		if (flexi.pack(dpb))
		{
			U32 datasize = (U32)dpb.getCurrentSize();
			
			msg->nextBlockFast(_PREHASH_ObjectData);
			msg->addU32Fast(_PREHASH_ObjectLocalID, prim["LocalID"].asInteger());

			msg->addU16Fast(_PREHASH_ParamType, 0x10);
			msg->addBOOLFast(_PREHASH_ParamInUse, true);

			msg->addU32Fast(_PREHASH_ParamSize, datasize);
			msg->addBinaryDataFast(_PREHASH_ParamData, tmp, datasize);
		}
	}
	
	if (prim.has("light"))
	{
		LLLightParams light;
		light.fromLLSD(prim["light"]);
		
		U8 tmp[MAX_OBJECT_PARAMS_SIZE];
		LLDataPackerBinaryBuffer dpb(tmp, MAX_OBJECT_PARAMS_SIZE);
		
		if (light.pack(dpb))
		{
			U32 datasize = (U32)dpb.getCurrentSize();
			
			msg->nextBlockFast(_PREHASH_ObjectData);
			msg->addU32Fast(_PREHASH_ObjectLocalID, prim["LocalID"].asInteger());

			msg->addU16Fast(_PREHASH_ParamType, 0x20);
			msg->addBOOLFast(_PREHASH_ParamInUse, true);

			msg->addU32Fast(_PREHASH_ParamSize, datasize);
			msg->addBinaryDataFast(_PREHASH_ParamData, tmp, datasize);
		}
	}

	if (prim.has("chat"))
	{
		//what is this? -Patrick Sapinski (Thursday, October 22, 2009)
		//send_chat_from_viewer(prim["chat"].asString(), CHAT_TYPE_SHOUT, 0);
	}
	
	if (prim.has("sculpt"))
	{
		LLSculptParams sculpt;
		sculpt.fromLLSD(prim["sculpt"]);

		if(assetmap[sculpt.getSculptTexture()].notNull())
		{
			LLUUID replacment=assetmap[sculpt.getSculptTexture()];
			sculpt.setSculptTexture(replacment);
		}
		else
			cmdline_printchat("Failed to remap sculpted texture UUID " + sculpt.getSculptTexture().asString());

		
		U8 tmp[MAX_OBJECT_PARAMS_SIZE];
		LLDataPackerBinaryBuffer dpb(tmp, MAX_OBJECT_PARAMS_SIZE);
		
		if (sculpt.pack(dpb))
		{
			U32 datasize = (U32)dpb.getCurrentSize();
			
			msg->nextBlockFast(_PREHASH_ObjectData);
			msg->addU32Fast(_PREHASH_ObjectLocalID, prim["LocalID"].asInteger());

			msg->addU16Fast(_PREHASH_ParamType, 0x30);
			msg->addBOOLFast(_PREHASH_ParamInUse, true);

			msg->addU32Fast(_PREHASH_ParamSize, datasize);
			msg->addBinaryDataFast(_PREHASH_ParamData, tmp, datasize);
		}
	}
	
	msg->sendReliable(gAgent.getRegion()->getHost());
}

void ImportTracker::send_namedesc(LLSD& prim)
{
	if(prim.has("name"))
	{
		LLMessageSystem* msg = gMessageSystem;
		msg->newMessageFast(_PREHASH_ObjectName);
		msg->nextBlockFast(_PREHASH_AgentData);
		msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID() );
		msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
		
		msg->nextBlockFast(_PREHASH_ObjectData);
		msg->addU32Fast(_PREHASH_LocalID, prim["LocalID"].asInteger());
		msg->addStringFast(_PREHASH_Name, prim["name"]);
		
		msg->sendReliable(gAgent.getRegion()->getHost());
	}
	
	if(prim.has("description"))
	{
		LLMessageSystem* msg = gMessageSystem;
		msg->newMessageFast(_PREHASH_ObjectDescription);
		msg->nextBlockFast(_PREHASH_AgentData);
		msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID() );
		msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
		
		msg->nextBlockFast(_PREHASH_ObjectData);
		msg->addU32Fast(_PREHASH_LocalID, prim["LocalID"].asInteger());
		msg->addStringFast(_PREHASH_Description, prim["description"]);
		
		msg->sendReliable(gAgent.getRegion()->getHost());
	}
}

void ImportTracker::link()
{	
	if(linkset.size() == 256)
	{
		LLMessageSystem* msg = gMessageSystem;
		msg->newMessageFast(_PREHASH_ObjectLink);
		msg->nextBlockFast(_PREHASH_AgentData);
		msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID() );
		msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
		
		LLSD::array_iterator prim = linkset.beginArray();
		++prim;
		for (; prim != linkset.endArray(); ++prim)
		{
			msg->nextBlockFast(_PREHASH_ObjectData);
			msg->addU32Fast(_PREHASH_ObjectLocalID, (*prim)["LocalID"].asInteger());		
		}
		
		msg->sendReliable(gAgent.getRegion()->getHost());

		LLMessageSystem* msg2 = gMessageSystem;
		msg2->newMessageFast(_PREHASH_ObjectLink);
		msg2->nextBlockFast(_PREHASH_AgentData);
		msg2->addUUIDFast(_PREHASH_AgentID, gAgent.getID() );
		msg2->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
		
		LLSD prim2 = linkset[0];
		msg2->nextBlockFast(_PREHASH_ObjectData);
		msg2->addU32Fast(_PREHASH_ObjectLocalID, (prim2)["LocalID"].asInteger());		
		prim2 = linkset[1];
		msg2->nextBlockFast(_PREHASH_ObjectData);
		msg2->addU32Fast(_PREHASH_ObjectLocalID, (prim2)["LocalID"].asInteger());		

		msg2->sendReliable(gAgent.getRegion()->getHost());
	}
	else
	{
		LLMessageSystem* msg = gMessageSystem;
		msg->newMessageFast(_PREHASH_ObjectLink);
		msg->nextBlockFast(_PREHASH_AgentData);
		msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID() );
		msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
		
		for (LLSD::array_iterator prim = linkset.beginArray(); prim != linkset.endArray(); ++prim)
		{
			msg->nextBlockFast(_PREHASH_ObjectData);
			msg->addU32Fast(_PREHASH_ObjectLocalID, (*prim)["LocalID"].asInteger());		
		}
		msg->sendReliable(gAgent.getRegion()->getHost());
	}

	llinfos << "FINISHED IMPORT" << llendl;
	
	if (linkset[0].has("Attachment"))
	{
		llinfos << "OBJECT IS ATTACHMENT, WAITING FOR POSITION PACKETS.." << llendl;
		state = POSITIONING;
		wear(linkset[0]);
	}
	else
		cleanUp();
}

void ImportTracker::wear(LLSD &prim)
{
	LLMessageSystem* msg = gMessageSystem;
	msg->newMessageFast(_PREHASH_ObjectAttach);
	msg->nextBlockFast(_PREHASH_AgentData);
	msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID() );
	msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
	msg->addU8Fast(_PREHASH_AttachmentPoint, U8(prim["Attachment"].asInteger()));
	
	msg->nextBlockFast(_PREHASH_ObjectData);
	msg->addU32Fast(_PREHASH_ObjectLocalID, prim["LocalID"].asInteger());
	msg->addQuatFast(_PREHASH_Rotation, LLQuaternion(0.0f, 0.0f, 0.0f, 1.0f));
	
	msg->sendReliable(gAgent.getRegion()->getHost());

	LLVector3 position = prim["attachpos"];
	
	LLSD rot = prim["attachrot"];
	LLQuaternion rotq;
	rotq.mQ[VX] = (F32)(rot[0].asReal());
	rotq.mQ[VY] = (F32)(rot[1].asReal());
	rotq.mQ[VZ] = (F32)(rot[2].asReal());
	rotq.mQ[VW] = (F32)(rot[3].asReal());
	LLVector3 rotation = rotq.packToVector3();
	U8 data[256];
	
	LLMessageSystem* msg2 = gMessageSystem;
	msg2->newMessageFast(_PREHASH_MultipleObjectUpdate);
	msg2->nextBlockFast(_PREHASH_AgentData);
	msg2->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
	msg2->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
	
	msg2->nextBlockFast(_PREHASH_ObjectData);
	msg2->addU32Fast(_PREHASH_ObjectLocalID, prim["LocalID"].asInteger());
	msg2->addU8Fast(_PREHASH_Type, U8(0x01 | 0x08));
	htonmemcpy(&data[0], &(position.mV), MVT_LLVector3, 12);
	msg2->addBinaryDataFast(_PREHASH_Data, data, 12);
	
	msg2->nextBlockFast(_PREHASH_ObjectData);
	msg2->addU32Fast(_PREHASH_ObjectLocalID, prim["LocalID"].asInteger());
	msg2->addU8Fast(_PREHASH_Type, U8(0x02 | 0x08));
	htonmemcpy(&data[0], &(rotation.mV), MVT_LLQuaternion, 12); 
	msg2->addBinaryDataFast(_PREHASH_Data, data, 12);
	
	msg2->sendReliable(gAgent.getRegion()->getHost());
	llinfos << "POSITIONED, IMPORT COMPLETED" << llendl;
	cleanUp();
}

void ImportTracker::plywood_above_head()
{
		LLMessageSystem* msg = gMessageSystem;
		msg->newMessageFast(_PREHASH_ObjectAdd);
		msg->nextBlockFast(_PREHASH_AgentData);
		msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
		msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
		msg->addUUIDFast(_PREHASH_GroupID, gAgent.getGroupID());
		msg->nextBlockFast(_PREHASH_ObjectData);
		msg->addU8Fast(_PREHASH_Material, 3);
		msg->addU32Fast(_PREHASH_AddFlags, 0);
		LLVolumeParams	volume_params;
		volume_params.setType(0x01, 0x10);
		volume_params.setBeginAndEndS(0.f, 1.f);
		volume_params.setBeginAndEndT(0.f, 1.f);
		volume_params.setRatio(1, 1);
		volume_params.setShear(0, 0);
		LLVolumeMessage::packVolumeParams(&volume_params, msg);
		msg->addU8Fast(_PREHASH_PCode, 9);
		msg->addVector3Fast(_PREHASH_Scale, LLVector3(0.52345f, 0.52346f, 0.52347f));
		LLQuaternion rot;
		msg->addQuatFast(_PREHASH_Rotation, rot);
		LLViewerRegion *region = gAgent.getRegion();
		
		if (!localids.size())
			root = (initialPos + linksetoffset);
		
		msg->addVector3Fast(_PREHASH_RayStart, root);
		msg->addVector3Fast(_PREHASH_RayEnd, root);
		msg->addU8Fast(_PREHASH_BypassRaycast, (U8)TRUE );
		msg->addU8Fast(_PREHASH_RayEndIsIntersection, (U8)FALSE );
		msg->addU8Fast(_PREHASH_State, (U8)0);
		msg->addUUIDFast(_PREHASH_RayTargetID, LLUUID::null);
		msg->sendReliable(region->getHost());
}

void ImportTracker::update_map(LLUUID uploaded_asset)
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
	//std::string upload_message = "Uploading...\n\n";
	//upload_message.append(display_name);
	//LLUploadDialog::modalUploadDialog(upload_message);

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

void ImportTracker::upload_next_asset()
{
	if(uploadtextures.empty())
	{
		llinfos<<" Texture list is empty, moving to rez statge"<< llendl;
		current_asset=LLUUID::null;
		gImportTracker.importer("bean man", NULL);
		return;
	}

	//this->updateimportnumbers();

	std::list<LLUUID>::iterator iter;
	iter=uploadtextures.begin();
	LLUUID id=(*iter);
	uploadtextures.pop_front();

	llinfos<<"Got texture ID "<<id<< "trying to upload"<<llendl;

	current_asset=id;
	std::string struid;
	id.toString(struid);

	LLAssetID uuid;
	LLTransactionID tid;

	// gen a new transaction ID for this asset
	tid.generate();
	uuid = tid.makeAssetID(gAgent.getSecureSessionID());

	std::string filename=asset_dir+"//textures//"+struid + ".j2c";
	S32 file_size;
	apr_file_t* fp = ll_apr_file_open(filename, LL_APR_RB, &file_size);

	//look for j2c first, then tga in the /textures/ folder.
	if (!fp)
	{
		filename=asset_dir+"//textures//"+struid + ".tga";
		fp = ll_apr_file_open(filename, LL_APR_RB, &file_size);

		//next look for j2c first, then tga in the /sculptmaps/ folder.
		if (!fp)
		{
			filename=asset_dir+"//sculptmaps//"+struid + ".j2c";
			fp = ll_apr_file_open(filename, LL_APR_RB, &file_size);
			if (!fp)
			{
				filename=asset_dir+"//sculptmaps//"+struid + ".tga";
				fp = ll_apr_file_open(filename, LL_APR_RB, &file_size);
				if (!fp)
					cmdline_printchat("Could not locate texture with UUID " + struid);
			}

		}
	}

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
return;

}
