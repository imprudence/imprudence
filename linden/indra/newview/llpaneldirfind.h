/** 
 * @file llpaneldirfind.h
 * @brief The "Find All" panel in the Find directory.
 *
 * Copyright (c) 2001-2007, Linden Research, Inc.
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

#ifndef LL_LLPANELDIRFIND_H
#define LL_LLPANELDIRFIND_H

#include "llpaneldirbrowser.h"

// UI class forward declarations
class LLLineEditor;


class LLPanelDirFind : public LLPanelDirBrowser
{
public:
	LLPanelDirFind(const std::string& name, LLFloaterDirectory* floater);
	virtual ~LLPanelDirFind();

	virtual BOOL postBuild();

	/*virtual*/ void draw();

	static void onClickSearch(void *userdata);
	static void onCommitScope(LLUICtrl* ctrl, void* data);
	static void onKeystrokeName(LLLineEditor* line, void* data);
};

BOOL enable_never(void*);

#endif
