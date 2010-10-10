/** 
 * @file llhoverview.h
 * @brief LLHoverView class definition
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

#ifndef LL_LLHOVERVIEW_H
#define LL_LLHOVERVIEW_H

// Library includes
#include "lluuid.h"
#include "llview.h"
#include "llframetimer.h"
#include "llstring.h"
#include "llcoord.h"
#include "v3dmath.h"

#include "lldarray.h"
#include "llviewerwindow.h"
#include "llviewerobject.h"

class LLTool;
class LLPickInfo;


//
// Classes
//
class LLHoverView : public LLView
{
public:
	LLHoverView(const std::string& name, const LLRect& rect);
	~LLHoverView();

	/*virtual*/ void draw();

	void updateHover(LLTool* current_tool);
	void cancelHover();

	// The last hovered object is retained even after the hover is
	// cancelled, so allow it to be specifically reset. JC
	void resetLastHoverObject();

	void setHoverActive(const BOOL active);

	// We don't do hover picks while the user is typing.  In fact, we stop
	// until the mouse is moved.
	void setTyping(BOOL b);

	BOOL isHovering() const			
			{ return isHoveringLand() || isHoveringObject(); }

	BOOL isHoveringObject() const;
	BOOL isHoveringLand() const;

	LLViewerObject* getLastHoverObject() const;
	LLPickInfo getPickInfo() { return mLastPickInfo; }

	static void pickCallback(const LLPickInfo& info);

protected:
	void	updateText();

protected:
	LLFrameTimer	mStartHoverTimer;
	BOOL			mStartHoverPickTimer;
	BOOL			mDoneHoverPick;
	
	BOOL			mHoverActive;
	LLCoordGL		mHoverPos;

	// How long has the hover popup been visible?
	LLFrameTimer	mHoverTimer;

	typedef std::list<std::string> text_list_t;
	text_list_t mText;

	BOOL		mUseHover;

	// If not null and not dead, we're over an object.
	LLPointer<LLViewerObject>	mLastHoverObject;
	LLPickInfo					mLastPickInfo;

	// If not LLVector3d::ZERO, we're over land.
	LLVector3d					mHoverLandGlobal;
	LLVector3					mHoverOffset;

	BOOL		mTyping;

public:
	// Show in-world hover tips.  Allow to turn off for movie making, game playing.
	// Public so menu can directly toggle.
	static BOOL	sShowHoverTips;
};


//
// Globals
//

extern LLHoverView *gHoverView;


#endif
