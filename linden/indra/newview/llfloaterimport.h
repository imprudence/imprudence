/** 
 * @file llfloaterimport.h
 * @brief LLFloaterImport class definition
 *
 * Copyright (c) 2006-2007, Linden Research, Inc.
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

// llfloaterimport.h
//              
// copyright 2005, linden research inc

#ifndef LL_LLFLOATERIMPORT_H
#define LL_LLFLOATERIMPORT_H

#include "lldynamictexture.h"
#include "llfloater.h"
#include "llresizehandle.h"
#include "llscrolllistctrl.h"
#include "lluploaddialog.h"
#include "llxmlnode.h"
#include "llquaternion.h"

class LLComboBox;
class LLJoint;
class LLViewerJointMesh;
class LLVOAvatar;
class LLTextBox;

struct ImportAssetInfo
{
	LLString Name;
	LLString Desc;
	LLString FilenameAndPath;
	LLString SourcePath;
	std::vector<LLString> ImageFileQueue;
	std::vector<LLString> OldImageIDList;
	std::vector<LLString> NewImageIDList;
};

class LLFloaterImport : public LLFloater
{
public:
	LLFloaterImport(const std::string filename);
	virtual ~LLFloaterImport();

	virtual BOOL postBuild();
	
	static void setOKText(const char* text) { sOKText = text; }

	void recalcCost();

	static LLUUID sImportRequestID;

protected:
	LLXMLNodePtr		mObjectFile;

	LLScrollListCtrl*	mImportList;
	LLLineEditor*		mNameEditor;
	LLLineEditor*		mDescEditor;
	LLButton*			mOKBtn;
	LLButton*			mCancelBtn;
	LLTextBox*			mImageLabel;

	LLString			mFilenameAndPath;
	LLString			mFilename;
	LLString			mFilePath;
	LLString			mInventoryPath;

	typedef std::map<LLUUID, LLPointer<LLImageRaw> > image_map_t;
	image_map_t			mPreviewImages;
	LLPointer<LLImageRaw> mCurrentPreviewImage;
	GLuint				mGLName;

	LLUploadDialog*		mImportDialog;

protected:
	static LLString	sOKText;

	static void		onBtnOK(void*);
	static void		onBtnCancel(void*);

	static void asset_uploaded_callback(const LLUUID& uuid, void* user_data, S32 result);
	static void finishImport(ImportAssetInfo *fp);

	void draw();
	void LoadPreviewImage(LLString image_path, LLUUID image_uuid);
	BOOL handleMouseDown(S32 x, S32 y, MASK mask);
};

#endif  // LL_LLFLOATERIMPORT_H
