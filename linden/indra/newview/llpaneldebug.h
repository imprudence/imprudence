/** 
 * @file llpaneldebug.h
 * @brief Debug preferences panel for preferences floater
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

#ifndef LL_LLPANELDEBUG_H
#define LL_LLPANELDEBUG_H

#include "llpanel.h"

class LLCheckBoxCtrl;

class LLPanelDebug : public LLPanel
{
public:
	LLPanelDebug(const std::string& name, const LLRect& rect);
	virtual ~LLPanelDebug();

	void apply();	// Apply the changed values.
	void cancel();	// Cancel the changed values.

protected:
	LLCheckBoxCtrl*	mLeftClickCheck;
};

#endif // LL_LLPANELDEBUG_H
