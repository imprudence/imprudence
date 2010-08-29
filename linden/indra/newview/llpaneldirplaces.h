/** 
 * @file llpaneldirplaces.h
 * @brief "Places" panel in the Find directory (not popular places, just places)
 *
 * $LicenseInfo:firstyear=2001&license=viewergpl$
 * 
 * Copyright (c) 2001-2009, Linden Research, Inc.
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

#ifndef LL_LLPANELDIRPLACES_H
#define LL_LLPANELDIRPLACES_H

#include "llpaneldirbrowser.h"

// UI class forward declarations
class LLLineEditor;

class LLPanelDirPlaces : public LLPanelDirBrowser
{
public:
	LLPanelDirPlaces(const std::string& name, LLFloaterDirectory* floater);
	virtual ~LLPanelDirPlaces();

	/*virtual*/ void draw();

	virtual BOOL postBuild();

	static void onClickSearch(void *userdata);
	static void onKeystrokeName(LLLineEditor* line, void* data);

	void performQuery();

	// Initially fill in some data for the panel.
	void initialQuery();

private:
	void queryCore(const std::string& name, S32 category, U32 flags);
};

#endif
