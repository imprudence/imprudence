/** 
 * @file llpaneldebug.cpp
 * @brief Debug preferences panel for preferences floater
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

#include "llviewerprecompiledheaders.h"

// file include
#include "llpaneldebug.h"

// linden library includes
#include "llerror.h"
#include "llrect.h"
#include "llstring.h"
#include "llfontgl.h"

// project includes
#include "llaudioengine.h"
#include "llbutton.h"
#include "llcheckboxctrl.h"
#include "llcolorswatch.h"
#include "llcombobox.h"
#include "llresmgr.h"
#include "llsky.h"
#include "llslider.h"
#include "llsliderctrl.h"
#include "llspinctrl.h"
#include "lltextbox.h"
#include "llui.h"
#include "llvosky.h"
#include "message.h"
#include "llagent.h"
#include "llviewercontrol.h"

//
// Imported globals
//


//
// Globals
//

//
// Static functions
//

LLPanelDebug::LLPanelDebug(const std::string& name, const LLRect& rect)
:	LLPanel(name, rect)
{
	LLCheckboxCtrl* check = NULL;
	LLSliderCtrl *sliderctrl = NULL;

	const LLFontGL* font = LLResMgr::getInstance()->getRes( LLFONT_SANSSERIF_SMALL );

	const S32 HPAD = 10;
	const S32 VPAD = 4;

	const S32 TOP_PAD = 10;

	// alignment "rulers" for buttons
	const S32 SEGMENT_WIDTH = 128;
	const S32 LEFT = HPAD;
	const S32 RULER1 = LEFT + SEGMENT_WIDTH + 30;
	const S32 RULER2 = RULER1 + HPAD;
	const S32 RIGHT = RULER2 + SEGMENT_WIDTH;
	const S32 LABEL_OFFSET = 60;

	S32 cur_y = rect.getHeight() - TOP_PAD;

	sliderctrl = new LLSliderCtrl(std::string("Drop Shadow Floater"),
								  LLRect( LEFT, cur_y, RIGHT, cur_y - SLIDERCTRL_HEIGHT ),
								  std::string("Drop Shadow Floater"),
								  font,
								  LABEL_OFFSET,
								  RULER2 + SPINCTRL_DEFAULT_LABEL_WIDTH + SPINCTRL_BTN_WIDTH,
								  TRUE,
								  TRUE,
								  FALSE,
								  NULL, NULL,
								  (F32)gSavedSettings.getS32("DropShadowFloater"), 
								  0.f, 10.f, 1.0f, 
								  std::string("DropShadowFloater"));
	sliderctrl->setFollowsTop();
	sliderctrl->setFollowsLeft();
	addChild(sliderctrl);
	cur_y -= VPAD + SLIDERCTRL_HEIGHT;

	sliderctrl = new LLSliderCtrl(std::string("Drop Shadow Button"),
								  LLRect( LEFT, cur_y, RIGHT, cur_y - SLIDERCTRL_HEIGHT ),
								  std::string("Drop Shadow Button"),
								  font,
								  LABEL_OFFSET,
								  RULER2 + SPINCTRL_DEFAULT_LABEL_WIDTH + SPINCTRL_BTN_WIDTH,
								  TRUE,
								  TRUE,
								  FALSE,
								  NULL, NULL,
								  (F32)gSavedSettings.getS32("DropShadowButton"), 
								  0.f, 10.f, 1.0f, 
								  std::string("DropShadowButton"));
	sliderctrl->setFollowsTop();
	sliderctrl->setFollowsLeft();
	addChild(sliderctrl);
	cur_y -= VPAD + SLIDERCTRL_HEIGHT;

	check = new LLCheckboxCtrl(std::string("left click"),
							   LLRect(LEFT, cur_y, RIGHT, cur_y - 20),
							   std::string("Left Click Shows Menu Unless Interactive"),
							   font,
							   NULL, NULL,
							   gSavedSettings.getBOOL("LeftClickShowMenu"));
	check->setFollows(FOLLOWS_LEFT|FOLLOWS_TOP);
	addChild(check);
	mLeftClickCheck = check;
	cur_y -= VPAD+20;
}


LLPanelDebug::~LLPanelDebug()
{
	// Children all cleaned up by default view destructor.
}


void LLPanelDebug::apply()
{
	gSavedSettings.setBOOL("LeftClickShowMenu", mLeftClickCheck->get() );
}

void LLPanelDebug::cancel()
{
}
