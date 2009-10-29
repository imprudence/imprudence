/** 
 * @file llfloatermap.cpp
 * @brief The "mini-map" or radar in the upper right part of the screen.
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

#include "llviewerprecompiledheaders.h"

#include "llfloatermap.h"

#include "llagent.h"
#include "llcolorscheme.h"
#include "llviewercontrol.h"
#include "lldraghandle.h"
#include "llnetmap.h"
#include "llregionhandle.h"
#include "llresizebar.h"
#include "lluictrlfactory.h"
#include "llfirstuse.h"
#include "panelradar.h"


LLFloaterMap::LLFloaterMap(const LLSD& key)
	:
	LLFloater(std::string("minimap")),
	mPanelMap(NULL),
	mPanelRadar(NULL)
{
	LLCallbackMap::map_t factory_map;
	factory_map["mini_mapview"] = LLCallbackMap(createPanelMiniMap, this);
	factory_map["RadarPanel"] = LLCallbackMap(createPanelRadar, this);
	LLUICtrlFactory::getInstance()->buildFloater(this, "floater_mini_map.xml", &factory_map, FALSE);
}


// static
void* LLFloaterMap::createPanelMiniMap(void* data)
{
	LLFloaterMap* self = (LLFloaterMap*)data;
	self->mPanelMap = new LLNetMap("Mapview");
	return self->mPanelMap;
}

// static
void* LLFloaterMap::createPanelRadar(void* data)
{
	LLFloaterMap* self = (LLFloaterMap*)data;
	self->mPanelRadar = new PanelRadar();
	return self->mPanelRadar;
}


BOOL LLFloaterMap::postBuild()
{
	// Send the drag handle to the back, but make sure close stays on top
	sendChildToBack(getDragHandle());
	sendChildToFront(getChild<LLButton>("llfloater_minimize_btn"));
	sendChildToFront(getChild<LLButton>("llfloater_close_btn"));
	setIsChrome(TRUE);

	childSetAction("toggle_radar", onToggleRadar, this);

	if (!gSavedSettings.getBOOL("ShowMiniMapRadar"))
	{
		// Collapse radar if it's not showing.
		adjustLayout( false );
	}
	
	return TRUE;
}


LLFloaterMap::~LLFloaterMap()
{
}


// virtual 
void LLFloaterMap::onOpen()
{
	gFloaterView->adjustToFitScreen(this, FALSE);

	gSavedSettings.setBOOL("ShowMiniMap", TRUE);

	bool showing_radar = gSavedSettings.getBOOL("ShowMiniMapRadar");
	setRadarVisible( showing_radar );
}


// virtual
void LLFloaterMap::onClose(bool app_quitting)
{
	LLFloater::setVisible(FALSE);

	if (!app_quitting)
	{
		gSavedSettings.setBOOL("ShowMiniMap", FALSE);
	}
}

BOOL LLFloaterMap::canClose()
{
	return !LLApp::isExiting();
}


// virtual
void LLFloaterMap::draw()
{
	// Note: we can't just gAgent.check cameraMouselook() because the transition states are wrong.
	if( gAgent.cameraMouselook())
	{
		setMouseOpaque(FALSE);
		getDragHandle()->setMouseOpaque(FALSE);

		drawChild(mPanelMap);
	}
	else
	{
		setMouseOpaque(TRUE);
		getDragHandle()->setMouseOpaque(TRUE);

		LLFloater::draw();
	}
}

// [RLVa:KB] - Version: 1.22.11 | Checked: 2009-07-05 (RLVa-1.0.0c)
void LLFloaterMap::open()
{
	if (!gRlvHandler.hasBehaviour(RLV_BHVR_SHOWMINIMAP))
	{
		LLFloater::open();
		LLFirstUse::useMiniMap();
	}
}
// [/RLVa:KB]


PanelRadar* LLFloaterMap::getRadar()
{
	return mPanelRadar;
}


// static
void LLFloaterMap::onToggleRadar(void *user_data)
{
	LLFloaterMap* self = (LLFloaterMap*) user_data;
	self->toggleRadarVisible();
}


void LLFloaterMap::toggleRadarVisible()
{
	bool show_radar = gSavedSettings.getBOOL("ShowMiniMapRadar");
	show_radar = !show_radar;
	setRadarVisible( show_radar );
}


void LLFloaterMap::setRadarVisible( bool show_radar )
{
	bool old_show_radar = gSavedSettings.getBOOL("ShowMiniMapRadar");

	gSavedSettings.setBOOL("ShowMiniMapRadar", show_radar);
	setRadarButtonState( show_radar );
	mPanelRadar->setVisible( show_radar );

	// Adjust the minimap window's size if visibility is changing
	if (show_radar != old_show_radar)
	{
		adjustLayout( show_radar );
	}
}


void LLFloaterMap::setRadarButtonState( bool showing_radar )
{
	LLButton* toggle = getChild<LLButton>("toggle_radar");
	if (toggle)
	{
		toggle->setToggleState(showing_radar);
		if (showing_radar)
		{
			// Expanded, so show image to offer to collapse upwards.
			toggle->setImageOverlay("arrow_up.tga");
		}
		else
		{
			// Collapsed, so show image to offer to expand downwards.
			toggle->setImageOverlay("arrow_down.tga");
		}
	}
}


void LLFloaterMap::adjustLayout( bool expand )
{
	S32 radar_height = mPanelRadar->getRect().getHeight();
	S32 height = getRect().getHeight();
	LLRect map_rect = mPanelMap->getRect();
	S32 map_bottom = map_rect.mBottom;

	S32 min_width, min_height;
	getResizeLimits( &min_width, &min_height );

	S32 adjust = radar_height;
	if (!expand)
	{
		adjust = -adjust;
	}
	
	height += adjust;
	min_height += adjust;
	map_bottom += adjust;

	map_rect.set( map_rect.mLeft,  map_rect.mTop,
	              map_rect.mRight, map_bottom );
	mPanelMap->setRect(map_rect);

	setResizeLimits( min_width, min_height );
	reshape( getRect().getWidth(), height, false );

	LLRect temp_rect = getRect();
	temp_rect.translate( 0, -adjust );
	setRect( temp_rect );

	LLButton* toggle = getChild<LLButton>("toggle_radar");
	if (toggle)
	{
		temp_rect = toggle->getRect();
		temp_rect.translate( 0, adjust );
		toggle->setRect( temp_rect );
	}
}
