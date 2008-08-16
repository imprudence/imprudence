/** 
 * @file llfloaterimport.cpp
 * @brief LLFloaterImport class implementation
 *
 * Copyright (c) 2005-2007, Linden Research, Inc.
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

#include "llviewerprecompiledheaders.h"

#include "llfloaterimport.h"

#include "llapr.h"

#include "llimagebmp.h"
#include "llimagetga.h"
#include "llimagejpeg.h"

#include "llagent.h"
#include "llbutton.h"
#include "llcombobox.h"
#include "lldrawable.h"
#include "lldrawpoolavatar.h"
#include "llface.h"
#include "llinventorymodel.h"
#include "lllineeditor.h"
#include "llresourcedata.h"
#include "lltextbox.h"
#include "lltoolmgr.h"
#include "llui.h"
#include "lluploaddialog.h"
#include "llviewercamera.h"
#include "llviewermenufile.h"	// upload_new_resource()
#include "llviewerwindow.h"
#include "llvoavatar.h"
#include "pipeline.h"
#include "viewer.h"
#include "llvieweruictrlfactory.h"

extern LLInventoryModel gInventory;

//statics
//LLUploadDialog *LLFloaterImport::sImportDialog = NULL;
LLUUID LLFloaterImport::sImportRequestID;
LLString	LLFloaterImport::sOKText = LLString();

const S32 PREVIEW_BORDER_WIDTH = 2;
const S32 PREVIEW_RESIZE_HANDLE_SIZE = S32(RESIZE_HANDLE_WIDTH * OO_SQRT2) + PREVIEW_BORDER_WIDTH;
const S32 PREVIEW_HPAD = PREVIEW_RESIZE_HANDLE_SIZE;
const S32 PREF_BUTTON_HEIGHT = 16;
const S32 PREVIEW_TEXTURE_HEIGHT = 300;

//-----------------------------------------------------------------------------
// LLFloaterImport()
//-----------------------------------------------------------------------------
LLFloaterImport::LLFloaterImport(const std::string filename)
	: LLFloater("Import")
{
	mFilenameAndPath = filename;

	char file_path[256];	/*Flawfinder: ignore*/
	strncpy(file_path, mFilenameAndPath.c_str(), sizeof(file_path) -1);		/*Flawfinder: ignore*/
	file_path[sizeof(file_path) -1] = '\0';
	char *file_name = strrchr( file_path, gDirUtilp->getDirDelimiter()[0]);
	file_name[0] = 0;

	mFilename.assign(file_name + 1);
	mFilePath.assign(file_path);

	//LLString::toLower(mFilename);
	
	LLXMLNode::parseFile(filename, mObjectFile, NULL);
}

//-----------------------------------------------------------------------------
// postBuild()
//-----------------------------------------------------------------------------
BOOL LLFloaterImport::postBuild()
{
	LLString asset_name = mFilename;
	LLString::replaceNonstandardASCII( asset_name, '?' );
	LLString::replaceChar(asset_name, '|', '?');
	LLString::stripNonprintable(asset_name);
	LLString::trim(asset_name);

	char* asset_name_str = (char*)asset_name.c_str();
	char* end_p = strrchr(asset_name_str, '.');		 // strip extension if exists
	if( !end_p )
	{
		end_p = asset_name_str + strlen( asset_name_str );			/*Flawfinder: ignore*/
	}
		
	S32 len = llmin( (S32) (DB_INV_ITEM_NAME_STR_LEN), (S32) (end_p - asset_name_str) );

	asset_name = asset_name.substr( 0, len );

	setTitle(mFilename);

	{
		// Center the window
		LLRect window_rect = gViewerWindow->getRootView()->getRect();

		S32 dialog_left = window_rect.mLeft + (window_rect.getWidth() - mRect.getWidth()) / 2;
		S32 dialog_bottom = window_rect.mBottom + (window_rect.getHeight() - mRect.getHeight()) / 2;

		translate( dialog_left - mRect.mLeft, dialog_bottom - mRect.mBottom );
	}

	mNameEditor = LLViewerUICtrlFactory::getLineEditorByName(this, "name_form");
	mNameEditor->setMaxTextLength(DB_INV_ITEM_NAME_STR_LEN);
	//mNameEditor->setCommitCallback(onCommit);
	mNameEditor->setPrevalidate(&LLLineEditor::prevalidatePrintableNotPipe);

	mNameEditor->setText(asset_name);

	mDescEditor = LLViewerUICtrlFactory::getLineEditorByName(this, "description_form");
	mDescEditor->setMaxTextLength(DB_INV_ITEM_DESC_STR_LEN);
	//mDescEditor->setCommitCallback(onCommit);
	mDescEditor->setPrevalidate(&LLLineEditor::prevalidatePrintableNotPipe);

	// Cancel button
	mCancelBtn = LLViewerUICtrlFactory::getButtonByName(this, "cancel_btn");
	mCancelBtn->setClickedCallback(onBtnCancel);
	mCancelBtn->setCallbackUserData(this);

	// OK button
	mOKBtn = LLViewerUICtrlFactory::getButtonByName(this, "ok_btn");
	mOKBtn->setClickedCallback(onBtnOK);
	mOKBtn->setCallbackUserData(this);
	setDefaultBtn(mOKBtn);

	mImageLabel = LLViewerUICtrlFactory::getTextBoxByName(this, "preview_label");
	mImportList = LLViewerUICtrlFactory::getScrollListByName(this, "upload_list");

	bool object_file_read = false;
	int object_count = 0, image_count = 0;
	if (mObjectFile)
	{
		LLXMLNodeList object_list;
		mObjectFile->findName("object", object_list);
		LLXMLNodeList::iterator itor;
		LLXMLNodeList::iterator itor2;
		for (itor = object_list.begin(); itor != object_list.end(); ++itor)
		{
			LLXMLNode *object = itor->second;
			LLXMLNodeList tasks_list;
			object->getChildren("task", tasks_list, FALSE);
			for (itor2 = tasks_list.begin(); itor2 != tasks_list.end(); ++itor2)
			{
				LLXMLNode *task = itor2->second;
				LLXMLNodePtr temp_node;
				if (!task->getChild("root", temp_node, FALSE))
				{
					// This task is a root
					LLString task_name = "(unnamed object)";
					LLXMLNodePtr task_name_node;
					if (task->getChild("name", task_name_node, FALSE))
					{
						task_name_node->getStringValue(1, &task_name);
					}
					{
						LLString output_line;
						char buffer[20];		/*Flawfinder: ignore*/
						snprintf(buffer, sizeof(buffer), "%d", (S32)tasks_list.size());	/* Flawfinder: ignore */
						output_line.append(buffer);
						output_line.append(" prims");

						LLSD row;
						row["columns"][0]["value"] = "OBJECT";
						row["columns"][0]["width"] = 60;
						row["columns"][1]["value"] = output_line;
						row["columns"][1]["width"] = 80;
						row["columns"][2]["value"] = task_name;
						row["enabled"] = false;
						mImportList->addElement(row);
					}
					mImportList->setCanSelect(TRUE);
					mImportList->setAllowMultipleSelection(TRUE);
					object_file_read = true;
					object_count++;
					break;
				}
			}
		}

		if (object_count > 0)
		{
			std::string::size_type pos = mFilenameAndPath.rfind(".");
			if (pos != mFilenameAndPath.npos)
			{
				mInventoryPath = mFilenameAndPath.substr(0, pos);
			}
			else
			{
				mInventoryPath = mFilenameAndPath;
			}
			mInventoryPath.append("_inventory");

			LLXMLNodeList image_list;
			mObjectFile->findName("sl:image", image_list);
			std::vector<LLString> unique_images;
			std::vector<LLUUID> unique_ids;
			std::vector<bool> image_changed;
			for (itor = image_list.begin(); itor != image_list.end(); ++itor)
			{
				LLXMLNode *image_node = itor->second;
				LLString image_id = image_node->getID();

				LLUUID image_uuid;
				image_node->getUUIDValue(1, &image_uuid);

				bool found_image = false;
				for (U32 image_num=0; image_num<unique_images.size(); ++image_num)
				{
					if (unique_images[image_num] == image_id)
					{
						found_image = true;
						continue;
					}
				}
				if (!found_image)
				{
					unique_images.push_back(image_id);
					unique_ids.push_back(image_uuid);

					LLString node_hash = "00000000000000000000000000000000";
					LLXMLNodePtr image_hash_node;
					if (image_node->getChild("checksum", image_hash_node))
					{
						image_hash_node->getStringValue(1, &node_hash);
					}

					llinfos << "Node hash: " << node_hash << llendl;

					// Try to find image and get checksum
					LLString image_path = mInventoryPath;
					image_path.append(gDirUtilp->getDirDelimiter());
					image_path.append(image_id);
					image_path.append(".tga");

					llinfos << "Getting hash for " << image_path << llendl;

					char md5_hash_string[33];	/*Flawfinder: ignore*/
					strcpy(md5_hash_string, "00000000000000000000000000000000");			/*Flawfinder: ignore*/
					FILE* fCheck = LLFile::fopen(image_path.c_str(), "rb");	/*Flawfinder: ignore*/
					if (fCheck)
					{
						LLMD5 my_md5_hash(fCheck); // this fclose()s fCheck too
						my_md5_hash.hex_digest(md5_hash_string);

						llinfos << "hash: " << md5_hash_string << llendl;
					}

					LoadPreviewImage(image_path, image_uuid);

					if (memcmp(md5_hash_string, node_hash.c_str(), 32) == 0)
					{
						// Image has not changed
						image_changed.push_back(false);
					}
					else
					{
						// Image has changed
						image_changed.push_back(true);
					}
				}
			}
			for (U32 image_num=0; image_num<unique_images.size(); ++image_num)
			{
				LLSD row;
				row["columns"][0]["value"] = "IMAGE";
				row["columns"][0]["width"] = 60;
				row["columns"][1]["value"] = image_changed[image_num]?"NEW":"NOT NEW";
				row["columns"][1]["width"] = 80;
				row["columns"][2]["value"] = unique_images[image_num];
				mImportList->addElement(row);
				mImportList->setCanSelect(TRUE);
				image_count++;
			}
		}
	}

	if (!object_file_read)
	{
		mImportList->addSimpleItem("Error: Invalid object file.", ADD_BOTTOM, FALSE);
		mImportList->setCanSelect(FALSE);
		mOKBtn->setEnabled(FALSE);
	}
	else
	{
		recalcCost();
	}

	return TRUE;
}

void LLFloaterImport::LoadPreviewImage(LLString image_path, LLUUID image_uuid)
{
	LLPointer<LLImageRaw> raw_image = new LLImageRaw;
	LLPointer<LLImageTGA> tga_image = new LLImageTGA;

	if (!tga_image->load(image_path))
	{
		return;
	}
	
	if (!tga_image->decode(raw_image))
	{
		return;
	}

	if(	(tga_image->getComponents() != 3) &&
		(tga_image->getComponents() != 4) )
	{
		tga_image->setLastError( "Image files with less than 3 or more than 4 components are not supported." );
		return;
	}

	raw_image->biasedScaleToPowerOfTwo(256);

	mPreviewImages[image_uuid] = raw_image;
}

//-----------------------------------------------------------------------------
// LLFloaterImport()
//-----------------------------------------------------------------------------
LLFloaterImport::~LLFloaterImport()
{
	if (mGLName)
	{
		glDeleteTextures(1, &mGLName );
	}
}

//-----------------------------------------------------------------------------
// recalcCost()
//-----------------------------------------------------------------------------
void LLFloaterImport::recalcCost()
{
	/*S32 cost = 0;
	LLScrollListItem *item = mImportList->getFirstData();
	while (item)
	{
		LLString item_type = item->getColumn(0)->getText();

		if (item_type == "IMAGE" && item->getSelected())
		{
			cost += 10; // hypothetical image cost
		}
		if (item_type == "OBJECT")
		{
			cost += 10; // hypothetical object cost
		}
		item = mImportList->getNextData();
	}

	char buffer[20];
	sOKText = "Upload (L$";
	sprintf(buffer, "%d", cost);
	sOKText.append(buffer);
	sOKText.append(")");
	mOKBtn->setLabelSelected(sOKText.c_str());
	mOKBtn->setLabelUnselected(sOKText.c_str());
	mOKBtn->setEnabled(TRUE);*/
}

//-----------------------------------------------------------------------------
// draw()
//-----------------------------------------------------------------------------
void LLFloaterImport::draw()
{
	LLFloater::draw();

	LLRect PreviewImageRect;
	BOOL has_rect = childGetRect("dummy_preview", PreviewImageRect);
	
	if (mCurrentPreviewImage.notNull() && has_rect)
	{
		gl_rect_2d_checkerboard(PreviewImageRect);

		GLenum format_options[4] = { GL_LUMINANCE, GL_LUMINANCE_ALPHA, GL_RGB, GL_RGBA };
		GLenum format = format_options[mCurrentPreviewImage->getComponents()-1];

		GLenum internal_format_options[4] = { GL_LUMINANCE8, GL_LUMINANCE8_ALPHA8, GL_RGB8, GL_RGBA8 };
		GLenum internal_format = internal_format_options[mCurrentPreviewImage->getComponents()-1];
	
		if (mGLName)
		{
			LLImageGL::bindExternalTexture( mGLName, 0, GL_TEXTURE_2D ); 
		}
		else
		{
			glGenTextures(1, &mGLName );
			stop_glerror();

			LLImageGL::bindExternalTexture( mGLName, 0, GL_TEXTURE_2D ); 
			stop_glerror();

			glTexImage2D(
				GL_TEXTURE_2D, 0, internal_format, 
				mCurrentPreviewImage->getWidth(), mCurrentPreviewImage->getHeight(),
				0, format, GL_UNSIGNED_BYTE, mCurrentPreviewImage->getData());
			stop_glerror();

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}

		glColor3f(1.f, 1.f, 1.f);
		glBegin( GL_QUADS );
		{
			glTexCoord2f(0, 1);
			glVertex2i(PreviewImageRect.mLeft, PreviewImageRect.mTop);
			glTexCoord2f(0, 0);
			glVertex2i(PreviewImageRect.mLeft, PreviewImageRect.mBottom);
			glTexCoord2f(1, 0);
			glVertex2i(PreviewImageRect.mRight, PreviewImageRect.mBottom);
			glTexCoord2f(1, 1);
			glVertex2i(PreviewImageRect.mRight, PreviewImageRect.mTop);
		}
		glEnd();

		LLImageGL::unbindTexture(0, GL_TEXTURE_2D);

		stop_glerror();
	}
}

// static
void LLFloaterImport::finishImport(ImportAssetInfo *info)
{
	llinfos << "Uploading object " << info->FilenameAndPath << "..." << llendl;

	LLUUID new_file_id;
	new_file_id.generate();
	bool generated_file = false;

	{
		// Copy file into a local directory
		LLString new_file = "TEMP";
		new_file.append(new_file_id.asString());
		new_file.append(".slobject");

		S32 length;
		apr_file_t* fIn = ll_apr_file_open(info->FilenameAndPath.c_str(), LL_APR_RB, &length);
		if (fIn)
		{
			apr_file_t* fOut = ll_apr_file_open(new_file, LL_APR_WB);
			if (fOut)
			{
				char  *buffer = new char[length];
				ll_apr_file_read(fIn, buffer, length);
				ll_apr_file_write(fOut, buffer, length);
				delete[] buffer;
				generated_file = true;
				apr_file_close(fOut);
			}
			apr_file_close(fIn);
		}
	}

	if (generated_file)
	{
		LLMessageSystem *msg = gMessageSystem;
		msg->newMessageFast(_PREHASH_ObjectImport);
		msg->nextBlockFast(_PREHASH_AgentData);
		msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
		msg->addUUIDFast(_PREHASH_FolderID, gInventory.findCategoryUUIDForType(LLAssetType::AT_OBJECT));
		msg->nextBlockFast(_PREHASH_AssetData);
		msg->addUUIDFast(_PREHASH_FileID, new_file_id);
		msg->addStringFast(_PREHASH_ObjectName, info->Name);
		msg->addStringFast(_PREHASH_Description, info->Desc);
		msg->sendReliable(gAgent.getRegionHost());

		LLUploadDialog::modalUploadDialog("Importing geometry...");

		sImportRequestID = new_file_id;
	}
	else
	{
		llinfos << "Failed to copy file." << llendl;
	}
}

// static
void LLFloaterImport::asset_uploaded_callback(const LLUUID& uuid, void* user_data, S32 result)
{
	llinfos << "LLFloaterImport: Finished uploading image." << llendl;
	LLResourceData *resource_data = (LLResourceData*)user_data;
	ImportAssetInfo *info = (ImportAssetInfo*)resource_data->mUserData;

	info->NewImageIDList.push_back(resource_data->mAssetInfo.mUuid.asString());

	LLUploadDialog::modalUploadFinished();
	if (info->ImageFileQueue.size() == 0)
	{
		finishImport(info);
	}
	else
	{
		// Start the next download
		LLString current_image = info->ImageFileQueue[0].c_str();
		info->ImageFileQueue.erase(info->ImageFileQueue.begin());

		upload_new_resource(current_image, info->Name, // file
			info->Desc, 0, LLAssetType::AT_NONE, LLInventoryType::IT_NONE, 
			PERM_NONE, LLString::null, asset_uploaded_callback, info);
	}
}

//-----------------------------------------------------------------------------
// static onBtnOK()
//-----------------------------------------------------------------------------
void LLFloaterImport::onBtnOK(void*userdata)
{
	LLFloaterImport *fp =(LLFloaterImport*)userdata;

	ImportAssetInfo *asset_info = new ImportAssetInfo();
	asset_info->Name = fp->mNameEditor->getText();
	asset_info->Desc = fp->mDescEditor->getText();
	asset_info->FilenameAndPath = fp->mFilenameAndPath;
	asset_info->SourcePath = fp->mInventoryPath;

	std::vector<LLScrollListItem*> items = fp->mImportList->getAllSelected();
	std::vector<LLScrollListItem*>::iterator itor;
	if (items.size() > 0)
	{
		for (itor = items.begin(); itor != items.end(); ++itor)
		{
			LLUUID id = (*itor)->getUUID();

			//if (!id->isNull())
			LLString *image_id = (LLString *)(*itor)->getUserdata();
			if (image_id)
			{
				asset_info->OldImageIDList.push_back(id.asString());
				LLString image_file = fp->mInventoryPath;
				image_file.append(gDirUtilp->getDirDelimiter());
				image_file.append(*image_id);
				image_file.append(".tga");
				llinfos << "Uploading image " << image_file << "..." << llendl;
				asset_info->ImageFileQueue.push_back(image_file);
			}
		}
		if (asset_info->ImageFileQueue.size() == 0)
		{
			finishImport(asset_info);
		}
		else
		{
			// Start the first download
			LLString current_image = asset_info->ImageFileQueue[0].c_str();
			asset_info->ImageFileQueue.erase(asset_info->ImageFileQueue.begin());

			upload_new_resource(current_image, asset_info->Name, // file
				asset_info->Desc, 0, LLAssetType::AT_NONE, LLInventoryType::IT_NONE, 
				PERM_NONE, LLString::null, asset_uploaded_callback, asset_info);
		}
	}
	else
	{
		finishImport(asset_info);
	}

	fp->onClose(false);
}

//-----------------------------------------------------------------------------
// static onBtnCancel()
//-----------------------------------------------------------------------------
void LLFloaterImport::onBtnCancel(void*userdata)
{
	LLFloaterImport *fp =(LLFloaterImport*)userdata;
	fp->onClose(false);
}


//-----------------------------------------------------------------------------
// virtual handleMouseDown()
//-----------------------------------------------------------------------------
BOOL LLFloaterImport::handleMouseDown(S32 x, S32 y, MASK mask)
{
	BOOL ret = LLFloater::handleMouseDown(x, y, mask);
	recalcCost();

	LLUUID current_preview_uuid;
	LLString current_preview_name = "none";

	LLScrollListItem *item;
	
	// Did we just select something?
	item = mImportList->getLastSelectedItem();
	if (item)
	{
		current_preview_uuid = item->getUUID();
		current_preview_name = item->getColumn(2)->getText();
	}
	else
	{
		// If not, see if exactly one object is selected
		std::vector<LLScrollListItem*> items = mImportList->getAllSelected();
		std::vector<LLScrollListItem*>::iterator itor;
		for (itor = items.begin(); itor != items.end(); ++itor)
		{
			if (current_preview_uuid.isNull())
			{
				current_preview_uuid = (*itor)->getUUID();
				current_preview_name = (*itor)->getColumn(2)->getText();
			}
			else
			{
				// More than one item is selected. Turn off preview
				current_preview_uuid.setNull();
				current_preview_name = "none";
				break;
			}
		}
	}

	mCurrentPreviewImage = NULL;
	if (current_preview_uuid.notNull())
	{
		image_map_t::iterator itor;
		itor = mPreviewImages.find(current_preview_uuid);
		if (itor != mPreviewImages.end())
		{
			mCurrentPreviewImage = itor->second;
			if (mGLName)
			{
				glDeleteTextures(1, &mGLName );
			}
			mGLName = 0;
		}
	}
	LLString label_text = "Image Preview: ";
	label_text.append(current_preview_name);
	mImageLabel->setText(label_text);

	return ret;
}
