/** 
 * @file llfloaterbuildoptions.h
 * @brief LLFloaterBuildOptions class definition
 *
 * $LicenseInfo:firstyear=2002&license=viewergpl$
 * 
 * Copyright (c) 2002-2009, Linden Research, Inc.
 * 2010, McCabe Maxsted
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
 * online at
 * http://secondlifegrid.net/programs/open_source/licensing/flossexception
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

/**
 * Panel for setting global object-editing options, specifically
 * grid size and spacing.
 */

#ifndef LL_LLFLOATERBUILDOPTIONS_H
#define LL_LLFLOATERBUILDOPTIONS_H

#include "llfloater.h"

class InvDropTarget;
class LLViewerInventoryItem;

class LLFloaterBuildOptions
:	public LLFloater
{
protected:
	LLFloaterBuildOptions();
	~LLFloaterBuildOptions();

public:
	BOOL			postBuild();

	static void		show(void*);
	static LLFloaterBuildOptions* getInstance();
	static BOOL		visible(void*);

	void apply();
	void cancel();
	void refresh();
	void reset();

	void setItem(const LLUUID& item);
	void setTexture(const LLUUID& texture);

protected:
	static LLFloaterBuildOptions*	sInstance;

private:
	static InvDropTarget*	sBuildObjectDropTarget;
	LLUUID					mBuildItemUUID;
	LLUUID					mBuildTextureUUID;

	static void onTexturePickerCommit(LLUICtrl* ctrl, void* userdata);
	static void onComboBoxCommit(LLUICtrl* ctrl, void* userdata);
	static void BuildAutoResponseItemDrop(LLViewerInventoryItem* item);

	static void onClickOK(void* userdata);
	static void onClickApply(void* userdata);
	static void onClickCancel(void* userdata);
	static void onClickClear(void* userdata);
	static void onClickReset(void* userdata);
	
	void refreshItem();
};

#endif
