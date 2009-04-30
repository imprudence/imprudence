/** 
 * @file llstatview.h
 * @brief Container for all statistics info.
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

#ifndef LL_LLSTATVIEW_H
#define LL_LLSTATVIEW_H

#include "llstatbar.h"
#include "llcontainerview.h"
#include <vector>

class LLStatBar;

class LLStatView : public LLContainerView
{
public:
	LLStatView(const std::string& name, const std::string& label, const std::string& setting, const LLRect& rect);
	~LLStatView();

/*
	virtual void draw();
	virtual void reshape(S32 width, S32 height, BOOL called_from_parent = TRUE);
	virtual LLRect getRequiredRect();	// Return the height of this object, given the set options.
*/

	LLStatBar *addStat(const std::string& name, LLStat *statp,
					   const std::string& setting = std::string(), BOOL default_bar = FALSE, BOOL default_history = FALSE);
	LLStatBar *getStatBar(const std::string& name);
	LLStatView *addStatView(const std::string& name, const std::string& label, const std::string& setting, const LLRect& rect);

protected:
	typedef std::vector<LLStatBar *> sb_vector_t;
	sb_vector_t mStatBars;
	U32 mNumStatBars;
	std::string mSetting;
};
#endif // LL_STATVIEW_H
