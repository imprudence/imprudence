/**
* @file floatergriddefault.cpp
* @brief prompts user to set the default grid on first use
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

#include "floatergriddefault.h"

#include "hippogridmanager.h"
#include "llpanellogin.h"
#include "llscrolllistctrl.h"
#include "lluictrlfactory.h"

FloaterGridDefault::FloaterGridDefault(const LLSD& key)
{
	LLUICtrlFactory::getInstance()->buildFloater(this, "floater_grid_default_selector.xml");
}

FloaterGridDefault::~FloaterGridDefault()
{
}

BOOL FloaterGridDefault::postBuild()
{
	// populate the grid chooser
	LLScrollListCtrl* grid_list = getChild<LLScrollListCtrl>("grid_list");
	grid_list->deleteAllItems();

	LLSD element;

	for (HippoGridManager::GridIterator it = gHippoGridManager->beginGrid(); it != gHippoGridManager->endGrid(); ++it) 
	{
		std::string grid_nick = it->second->getGridNick();
		// There's no reason why empty grids nicks should be in this list, ugh
		if (!grid_nick.empty()) 
		{
			element["id"] = grid_nick;
			element["columns"][0]["column"] = "grid";
			element["columns"][0]["type"] = "text";
			element["columns"][0]["value"] = grid_nick;
			grid_list->addElement(element, ADD_BOTTOM);
		}
	}

	grid_list->setFocus(TRUE);

	childSetAction("btn_ok", onClickOK, this);
	childSetAction("btn_cancel", onClickCancel, this);

	return TRUE;
}

// static
void FloaterGridDefault::onClickOK(void* userdata)
{
	FloaterGridDefault* self = (FloaterGridDefault*)userdata;

	LLScrollListCtrl* grid_list = self->getChild<LLScrollListCtrl>("grid_list");
	std::string selected = grid_list->getFirstSelected()->getValue().asString();

	if (!selected.empty() || selected != gHippoGridManager->getCurrentGridNick())
	{
		gHippoGridManager->setDefaultGrid(selected);
		gHippoGridManager->saveFile();
		LLPanelLogin::updateGridCombo(selected);
	}
	self->close();
}

// static
void FloaterGridDefault::onClickCancel(void* userdata)
{
	FloaterGridDefault* self = (FloaterGridDefault*)userdata;
	self->close();
}
