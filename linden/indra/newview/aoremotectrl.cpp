/**
* @file aoremotectrl.cpp
* @brief toolbar remote for toggling the viewer AO
*
* $LicenseInfo:firstyear=2009&license=viewergpl$
*
* Copyright (c) 2010, McCabe Maxsted
*
* Imprudence Viewer Source Code
* The source code in this file ("Source Code") is provided to you
* under the terms of the GNU General Public License, version 2.0
* ("GPL"). Terms of the GPL can be found in doc/GPL-license.txt in
* this distribution, or online at
* http://secondlifegrid.net/programs/open_source/licensing/gplv2
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
* ALL SOURCE CODE IS PROVIDED "AS IS." THE AUTHOR MAKES NO
* WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
* COMPLETENESS OR PERFORMANCE.
* $/LicenseInfo$
*/

#include "llviewerprecompiledheaders.h"

#include "aoremotectrl.h"

#include "floaterao.h"
#include "llbutton.h"
#include "lloverlaybar.h"
#include "lluictrlfactory.h"
#include "llviewercontrol.h"


AORemoteCtrl::AORemoteCtrl() 	
{
	setIsChrome(TRUE);
	build();
	setFocusRoot(TRUE);
}

AORemoteCtrl::~AORemoteCtrl()
{
}

void AORemoteCtrl::draw()
{
	LLButton* expand_button = getChild<LLButton>("popup_btn");
	if (expand_button)
	{
		if (expand_button->getToggleState())
		{
			expand_button->setImageOverlay("arrow_down.tga");
		}
		else
		{
			expand_button->setImageOverlay("arrow_up.tga");
		}
	}

	LLPanel::draw();
}

void AORemoteCtrl::build()
{
	if (gSavedSettings.getBOOL("ShowAOSitPopup"))
	{
		LLUICtrlFactory::getInstance()->buildPanel(this, "panel_ao_remote_expanded.xml");
	}
	else
	{
		LLUICtrlFactory::getInstance()->buildPanel(this, "panel_ao_remote.xml");
	}
}

BOOL AORemoteCtrl::postBuild()
{
	
	childSetAction("ao_btn", onClickToggleAO, this);
	childSetAction("ao_sit_btn", onClickToggleAOSit, this);
	childSetAction("ao_show_btn", onClickShowAO, this);
	childSetAction("popup_btn", onClickPopupBtn, this);

	return TRUE;
}

// static
void AORemoteCtrl::onClickToggleAO(void* data)
{
	BOOL ao_enable = gSavedSettings.getBOOL("AOEnabled");
	gSavedSettings.setBOOL("AOEnabled", !ao_enable);
	LLFloaterAO::run();
}

//static 
void AORemoteCtrl::onClickToggleAOSit(void* data)
{
	BOOL sit_enable = gSavedSettings.getBOOL("AOSitsEnabled");
	gSavedSettings.setBOOL("AOSitsEnabled", !sit_enable);
	LLFloaterAO::run();
}

//static 
void AORemoteCtrl::onClickShowAO(void* data)
{
	LLFloaterAO::show(NULL);
}

//static
void AORemoteCtrl::onClickPopupBtn(void* data)
{
	AORemoteCtrl* remotep = (AORemoteCtrl*)data;

	remotep->deleteAllChildren();
	remotep->build();
	gOverlayBar->layoutButtons();
}
