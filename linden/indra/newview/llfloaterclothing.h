/** 
 * @file llfloaterclothing.h
 * @brief Read-only list of wearables from your inventory.
 *
 * $LicenseInfo:firstyear=2004&license=viewergpl$
 * 
 * Copyright (c) 2004-2009, Linden Research, Inc.
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

#ifndef LL_LLFLOATERCLOTHING_H
#define LL_LLFLOATERCLOTHING_H

#include "llfloater.h"

class LLScrollableContainerView;
class LLView;
class LLButton;
class LLLineEditor;
class LLComboBox;
class LLScrollListCtrl;
class LLFloaterClothingObserver;

class LLFloaterClothing
:	public LLFloater
{
public:
	LLFloaterClothing();
	virtual ~LLFloaterClothing();

	virtual void onClose(bool app_quitting);

	static void show(void* unused = NULL);
	static void toggleVisibility();
	static void refreshAll();

private:
	void buildClothingList();

	static void onClickWear(void* data);
	static void onClickTakeOff(void* data);
	static void onCommitList(LLUICtrl* ctrl, void* data);

private:
	LLUUID mSelectedID;
	BOOL mAllowSelection;

	static LLFloaterClothing* sInstance;
	static LLFloaterClothingObserver* sObserver;
};


#endif
