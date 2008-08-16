/** 
 * @file llfloatersaveavatar.cpp
 * @brief write out avatar as CAL3D file
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

#include "llfloatersaveavatar.h"

#include "lldir.h"

#include "llagent.h"
#include "llvoavatar.h"
#include "llbutton.h"
#include "lllineeditor.h"
#include "llvieweruictrlfactory.h"
#include "llviewercontrol.h"

static LLFloaterSaveAvatar* mSingleton = NULL;

LLFloaterSaveAvatar::LLFloaterSaveAvatar() : LLFloater("Save Avatar")
{
	llassert( !mSingleton );
	mSingleton = this;
}

LLFloaterSaveAvatar::~LLFloaterSaveAvatar()
{
	llassert( mSingleton == this );
	mSingleton = NULL;
}

// Creates singleton or (if it already exists) brings it to the front
// static
void LLFloaterSaveAvatar::show()
{
	if ( !mSingleton)
	{
		LLFloaterSaveAvatar *instance = new LLFloaterSaveAvatar();

		gUICtrlFactory->buildFloater(instance, "floater_save_avatar.xml");
		S32 left;
		S32 top;
		gFloaterView->getNewFloaterPosition(&left, &top);
		instance->setOrigin(left, top - instance->getRect().getHeight());
	}
	else
	{
		mSingleton->open();		/*Flawfinder: ignore*/
	}
}

BOOL LLFloaterSaveAvatar::postBuild()
{
	mBaseNameEdit = LLViewerUICtrlFactory::getLineEditorByName(this, "base_name_edit");
	mBaseNameEdit->setText(gSavedSettings.getString("AvExportBaseName"));
	mPathEdit = LLViewerUICtrlFactory::getLineEditorByName(this, "path_edit");
	mPathEdit->setText(gSavedSettings.getString("AvExportPath"));
	
	mSaveBtn = LLViewerUICtrlFactory::getButtonByName(this, "save_btn");
	mSaveBtn->setClickedCallback(onSave);
	mSaveBtn->setCallbackUserData(this);

	return TRUE;
}

//static 
void LLFloaterSaveAvatar::onSave( void* user_data )
{
	LLFloaterSaveAvatar* self = (LLFloaterSaveAvatar*)user_data;

	gSavedSettings.setString("AvExportPath", self->mPathEdit->getText());
	gSavedSettings.setString("AvExportBaseName", self->mBaseNameEdit->getText());
	
	std::string path_name = self->mPathEdit->getText();
	if (path_name.size() && *(path_name.end() - 1) == '\\')
	{
		// remove trailing backslash
		path_name.erase(path_name.end() - 1);
	}

	std::string base_name = self->mBaseNameEdit->getText();
	if (base_name.size() && *(base_name.end() - 1) == '_')
	{
		// remove trailing underscore
		base_name.erase(base_name.end() - 1);
	}

	gAgent.getAvatarObject()->writeCAL3D(path_name, base_name);
	self->close();
}
