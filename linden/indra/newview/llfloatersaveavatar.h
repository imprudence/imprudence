/** 
 * @file llfloatersaveavatar.h
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

#ifndef LL_LLFLOATERSAVEAVATAR_H
#define LL_LLFLOATERSAVEAVATAR_H

#include "llfloater.h"

class LLButton;
class LLLineEditor;

class LLFloaterSaveAvatar : public LLFloater
{
public:
	static void show( void );  // Creates singleton or (if it already exists) brings it to the front
	static void onSave( void* user_data );

	virtual ~LLFloaterSaveAvatar();
	/*virtual*/ BOOL postBuild();

protected:
	LLFloaterSaveAvatar();

public:
	LLLineEditor* mBaseNameEdit;
	LLLineEditor* mPathEdit;
    LLButton* mSaveBtn;
};

#endif  // LL_LLFLOATERSAVEAVATAR_H
