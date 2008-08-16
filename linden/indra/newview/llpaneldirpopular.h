/** 
 * @file llpaneldirpopular.h
 * @brief Popular places in the world, as measured by dwell.
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

#ifndef LL_LLPANELDIRPOPULAR_H
#define LL_LLPANELDIRPOPULAR_H

#include "llpaneldirbrowser.h"

class LLUICtrl;

class LLPanelDirPopular : public LLPanelDirBrowser
{
public:
	LLPanelDirPopular(const std::string& name, LLFloaterDirectory* floater);
	virtual ~LLPanelDirPopular();

	virtual void draw();

	virtual BOOL postBuild();

	void requestPopular();

protected:
	static void onClickSearch(void* data);
	static void onCommitAny(LLUICtrl* ctrl, void* data);
	
private:
	bool mRequested;
};


#endif
