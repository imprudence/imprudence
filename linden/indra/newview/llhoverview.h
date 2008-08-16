/** 
 * @file llhoverview.h
 * @brief LLHoverView class definition
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

#ifndef LL_LLHOVERVIEW_H
#define LL_LLHOVERVIEW_H

// Library includes
#include "lluuid.h"
#include "llview.h"
#include "llframetimer.h"
#include "llstring.h"
#include "linked_lists.h"
#include "llcoord.h"
#include "v3dmath.h"

#include "lldarray.h"
#include "llhudconnector.h"

class LLTool;

//
// Classes
//
class LLHoverView : public LLView
{
public:
	LLHoverView(const std::string& name, const LLRect& rect);
	~LLHoverView();

	virtual EWidgetType getWidgetType() const;
	virtual LLString getWidgetTag() const;

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

	static void pickCallback(S32 x, S32 y, MASK mask);

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

	LLLinkedList<LLString>	mText;

	BOOL		mUseHover;

	// If not null and not dead, we're over an object.
	LLPointer<LLViewerObject>	mLastHoverObject;

	// If not LLVector3d::zero, we're over land.
	LLVector3d					mHoverLandGlobal;
	LLVector3d					mHoverOffset;

	LLDynamicArrayPtr<LLPointer<LLHUDConnector> > mConnectors;

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
