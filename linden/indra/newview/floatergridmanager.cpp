/**
* @file floatergridmanager.cpp
* @brief UI for managing grid information
*
* $LicenseInfo:firstyear=2011&license=viewergpl$
*
* Copyright (c) 2011, McCabe Maxsted
* based on Meerkat's grid manager by Patrick Sapinski
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

#include "floatergridmanager.h"

#include <boost/algorithm/string.hpp>
#include "floatergriddefault.h"
#include "llviewercontrol.h"
#include "llcombobox.h"
#include "llscrolllistctrl.h"
#include "llmd5.h"
#include "llnotifications.h"
#include "llurlsimstring.h"
#include "lluictrlfactory.h"
#include "hippogridmanager.h"
#include "llviewernetwork.h"
#include "llpanellogin.h"

const std::string PASSWORD_FILLER = "123456789!123456";

FloaterGridManager::FloaterGridManager(const LLSD& key)
	:	
	mState(GRID_STATE_NORMAL),
	mCurGrid("")
{
	llinfos << "Opening grid manager" << llendl;

	LLUICtrlFactory::getInstance()->buildFloater(this, "floater_grid_manager.xml");
	center();
}

FloaterGridManager::~FloaterGridManager()
{
}

BOOL FloaterGridManager::postBuild()
{
	LLLineEditor* password_edit = getChild<LLLineEditor>("avatar_password");
	if (password_edit)
	{
		password_edit->setDrawAsterixes(TRUE);
	}

	childSetAction("btn_delete", onClickDelete, this);
	childSetAction("btn_add", onClickAdd, this);
	childSetAction("btn_copy", onClickCopy, this);
	childSetAction("btn_ok", onClickOk, this);
	childSetAction("btn_apply", onClickApply, this);
	childSetAction("btn_cancel", onClickCancel, this);
	childSetAction("btn_clear", onClickClear, this);
	childSetAction("btn_gridinfo", onClickGridInfo, this);

	childSetCommitCallback("grid_selector", onSelectGrid, this);
	LLScrollListCtrl* combo = getChild<LLScrollListCtrl>("grid_selector");
	combo->setFocus(TRUE);

	clearGridInfo(true);

	refreshGrids();

	// We use the last selected grid as the default
	HippoGridInfo* first_selected = gHippoGridManager->getGrid(mCurGrid);
	if (first_selected)
	{
		getChild<LLLineEditor>("first_name")->setVisible(!first_selected->isUsernameCompat());
		getChild<LLLineEditor>("last_name")->setVisible(!first_selected->isUsernameCompat());
		getChild<LLLineEditor>("username")->setVisible(first_selected->isUsernameCompat());
	}
	getChild<LLLineEditor>("gridnick")->setEnabled(false);

	return TRUE;
}

void FloaterGridManager::draw()
{
	refresh();
	LLFloater::draw();
}

void FloaterGridManager::refresh()
{
	//TODO: new grid creation should just be set in a popup
	bool can_edit = (!(getChild<LLLineEditor>("gridnick")->getText().empty()) &&
					 !(getChild<LLLineEditor>("gridname")->getText().empty()) &&
					 !(getChild<LLLineEditor>("loginuri")->getText().empty()));

	getChild<LLLineEditor>("loginpage")->setEnabled(can_edit);
	getChild<LLLineEditor>("helperuri")->setEnabled(can_edit);
	getChild<LLLineEditor>("website")->setEnabled(can_edit);
	getChild<LLLineEditor>("support")->setEnabled(can_edit);
	getChild<LLLineEditor>("register")->setEnabled(can_edit);
	getChild<LLLineEditor>("password")->setEnabled(can_edit);
	getChild<LLLineEditor>("first_name")->setEnabled(can_edit);
	getChild<LLLineEditor>("last_name")->setEnabled(can_edit);
	getChild<LLLineEditor>("username")->setEnabled(can_edit);
	getChild<LLLineEditor>("avatar_password")->setEnabled(can_edit);
	getChild<LLLineEditor>("search")->setEnabled(can_edit);
	getChild<LLButton>("btn_delete")->setEnabled(can_edit);
	//getChild<LLButton>("btn_copy")->setEnabled(can_edit);
	getChild<LLButton>("btn_gridinfo")->setEnabled(can_edit);
	getChild<LLButton>("btn_clear")->setEnabled(can_edit);

	getChild<LLTextBox>("loginpage_text")->setEnabled(can_edit);
	getChild<LLTextBox>("helperuri_text")->setEnabled(can_edit);
	getChild<LLTextBox>("website_text")->setEnabled(can_edit);
	getChild<LLTextBox>("support_text")->setEnabled(can_edit);
	getChild<LLTextBox>("register_text")->setEnabled(can_edit);
	getChild<LLTextBox>("password_text")->setEnabled(can_edit);
	getChild<LLTextBox>("first_name_text")->setEnabled(can_edit);
	getChild<LLTextBox>("last_name_text")->setEnabled(can_edit);
	getChild<LLTextBox>("username_text")->setEnabled(can_edit);
	getChild<LLTextBox>("avatar_password_text")->setEnabled(can_edit);
	getChild<LLTextBox>("search_text")->setEnabled(can_edit);
}

void FloaterGridManager::refreshGrids()
{
	// no need to update the list while editing
	if (FloaterGridManager::getInstance()->getGridState() >= GRID_STATE_NEW)
	{
		return;
	}

	LLScrollListCtrl* grid_list = FloaterGridManager::getInstance()->getChild<LLScrollListCtrl>("grid_selector");
	std::string lastSelectedItem;
	LLSD element;

	if (grid_list->getFirstSelected())
	{
		// grid NICK
		lastSelectedItem = grid_list->getFirstSelected()->getColumn(0)->getValue().asString();
	}

	grid_list->deleteAllItems();

	// Grids should really have UUIDs, but we'll use their nicks to keep track of 'em instead
	for (HippoGridManager::GridIterator it = gHippoGridManager->beginGrid(); 
		 it != gHippoGridManager->endGrid(); ++it) 
	{
		std::string grid_nick = it->second->getGridNick();
		
		if (grid_nick.empty())
		{
			// shouldn't ever happen
			continue;
		}
		
		// Note: default sorted by grid NAME in the XUI
		if (grid_nick != gHippoGridManager->getCurrentGridNick()) 
		{
			element["id"] = grid_nick;
			element["columns"][0]["column"] = "grid_nick";
			element["columns"][0]["type"] = "text";
			element["columns"][0]["value"] = grid_nick;
			element["columns"][1]["column"] = "grid_name";
			element["columns"][1]["type"] = "text";
			element["columns"][1]["value"] = it->second->getGridName();

			grid_list->addElement(element, ADD_BOTTOM);
		}
	}

	// Setting the default needs to be rethought. 
	// Right now, we just use the last-selected grid,
	// but leaving this as the current behavior for now
	if (!gHippoGridManager->getCurrentGridNick().empty()) 
	{
		element["id"] = gHippoGridManager->getCurrentGridNick();
		element["columns"][0]["column"] = "grid_nick";
		element["columns"][0]["type"] = "text";
		element["columns"][0]["font-style"] = "BOLD";
		element["columns"][0]["value"] = gHippoGridManager->getCurrentGridNick();
		element["columns"][1]["column"] = "grid_name";
		element["columns"][1]["type"] = "text";
		element["columns"][1]["value"] = gHippoGridManager->getCurrentGrid()->getGridName();

		grid_list->addElement(element, ADD_TOP);
	}

	// Reselect the item if we had one selected
	if (lastSelectedItem.empty())
	{
		grid_list->selectItemByLabel(gHippoGridManager->getCurrentGridNick());
		FloaterGridManager::getInstance()->setCurGrid(gHippoGridManager->getCurrentGridNick());
	}
	else
	{
		grid_list->selectItemByLabel(lastSelectedItem);
		FloaterGridManager::getInstance()->setCurGrid(lastSelectedItem);
	}

	//if (selectIndex >= 0) 
	//{
	//	grid_list->setCurrentByIndex(selectIndex);
	//} 
	//else 
	//{
	//	grid_list->setLabel(LLStringExplicit(""));  // LLComboBox::removeall() does not clear the label
	//}

	FloaterGridManager::getInstance()->childSetEnabled("btn_delete", grid_list->getItemCount() > 0);
	FloaterGridManager::getInstance()->childSetEnabled("btn_copy", grid_list->getItemCount() > 0);

	HippoGridInfo *gridInfo = gHippoGridManager->getGrid(FloaterGridManager::getInstance()->mCurGrid);
	if (gridInfo) 
	{
		// Remember: NEVER let users change an existing grid's nick. Bad juju awaits if you do
		FloaterGridManager::getInstance()->getChild<LLLineEditor>("gridnick")->setText(gridInfo->getGridNick());
		FloaterGridManager::getInstance()->getChild<LLLineEditor>("gridname")->setText(gridInfo->getGridName());

		FloaterGridManager::getInstance()->getChild<LLLineEditor>("loginuri")->setText(gridInfo->getLoginURI());
		FloaterGridManager::getInstance()->getChild<LLLineEditor>("loginpage")->setText(gridInfo->getLoginPage());
		FloaterGridManager::getInstance()->getChild<LLLineEditor>("helperuri")->setText(gridInfo->getHelperURI());
		FloaterGridManager::getInstance()->getChild<LLLineEditor>("website")->setText(gridInfo->getWebSite());
		FloaterGridManager::getInstance()->getChild<LLLineEditor>("support")->setText(gridInfo->getSupportURL());
		FloaterGridManager::getInstance()->getChild<LLLineEditor>("register")->setText(gridInfo->getRegisterURL());
		FloaterGridManager::getInstance()->getChild<LLLineEditor>("password")->setText(gridInfo->getPasswordURL());
		FloaterGridManager::getInstance()->getChild<LLLineEditor>("first_name")->setText(gridInfo->getFirstName());
		FloaterGridManager::getInstance()->getChild<LLLineEditor>("last_name")->setText(gridInfo->getLastName());
		FloaterGridManager::getInstance()->getChild<LLLineEditor>("username")->setText(gridInfo->getUsername());

		if (gridInfo->getPassword().empty())
		{
			FloaterGridManager::getInstance()->getChild<LLLineEditor>("avatar_password")->setText(LLStringExplicit(""));
		}
		else
		{
			FloaterGridManager::getInstance()->getChild<LLLineEditor>("avatar_password")->setText(PASSWORD_FILLER);
		}
		
		FloaterGridManager::getInstance()->getChild<LLLineEditor>("first_name")->setVisible(!gridInfo->isUsernameCompat());
		FloaterGridManager::getInstance()->getChild<LLTextBox>("first_name_text")->setVisible(!gridInfo->isUsernameCompat());
		FloaterGridManager::getInstance()->getChild<LLLineEditor>("last_name")->setVisible(!gridInfo->isUsernameCompat());
		FloaterGridManager::getInstance()->getChild<LLTextBox>("last_name_text")->setVisible(!gridInfo->isUsernameCompat());
		FloaterGridManager::getInstance()->getChild<LLLineEditor>("username")->setVisible(gridInfo->isUsernameCompat());
		FloaterGridManager::getInstance()->getChild<LLTextBox>("username_text")->setVisible(gridInfo->isUsernameCompat());

		if (gridInfo->getPlatform() == HippoGridInfo::PLATFORM_SECONDLIFE) 
		{
			FloaterGridManager::getInstance()->childSetEnabled("search", false);
			FloaterGridManager::getInstance()->getChild<LLLineEditor>("search")->setText(LLStringExplicit(""));
		} 
		else 
		{
			 FloaterGridManager::getInstance()->childSetEnabled("search", true);
			 FloaterGridManager::getInstance()->getChild<LLLineEditor>("search")->setText(gridInfo->getSearchURL());
		}		
	}	
	else 
	{
		// should never happen, but if so default to creating a new entry
		FloaterGridManager::getInstance()->setupNewGridEntry();
	}
} 

void FloaterGridManager::update()
{
	// no need to update the list until we need to
	if (getGridState() >= GRID_STATE_NEW)
	{
		return;
	}
	refreshGrids();
}

void FloaterGridManager::applyChanges()
{
	// When we apply changes, we update based on the state
	// of the UI. This is ugly and could really be improved

	// Note: nick and loginuri are required
	std::string grid_nick = childGetValue("gridnick").asString();
	if (grid_nick.empty()) 
	{
		LLNotifications::instance().add("GridsNoNick");
		return;
	}

	if (childGetValue("loginuri").asString().empty()) 
	{
		LLSD args;
		args["[NAME]"] = grid_nick;
		LLNotifications::instance().add("GridsNoLoginURI", args);
		return ;
	}

	HippoGridInfo* grid = NULL;

	// Note: we disable the scroll list during grid creation to prevent conflicts
	if (getGridState() == GRID_STATE_NEW || getGridState() == GRID_STATE_COPY)
	{
		if (gHippoGridManager->hasGridNick(grid_nick)) 
		{
			LLSD args;
			args["[NAME]"] = grid_nick;
			LLNotifications::instance().add("GridExists", args);
			return;
		}
		else
		{
			grid = new HippoGridInfo(grid_nick);
		}
	}
	else // updating grid
	{
		if (!gHippoGridManager->hasGridNick(grid_nick))
		{
			llwarns << "Can't update info for a grid we don't know, ignoring changes." << llendl;
			return;
		}
		else
		{
			grid = gHippoGridManager->getGrid(grid_nick);
		}
	}

	grid->setGridName(childGetValue("gridname"));
	grid->setLoginURI(childGetValue("loginuri"));
	grid->setLoginPage(childGetValue("loginpage"));
	grid->setHelperURI(childGetValue("helperuri"));
	grid->setWebSite(childGetValue("website"));
	grid->setSupportURL(childGetValue("support"));
	grid->setRegisterURL(childGetValue("register"));
	grid->setPasswordURL(childGetValue("password"));
	grid->setSearchURL(childGetValue("search"));
	grid->setFirstName(childGetValue("first_name"));
	grid->setLastName(childGetValue("last_name"));
	grid->setUsername(childGetValue("username"));

	// don't allow users to set their password as PASSWORD_FILLER
	// would be nice to get grid-specific rules on password formatting, too
	// passwords are remembered by default when entered in the grid manager (good default?)
	std::string password_new = childGetValue("avatar_password").asString();
	std::string password_old = grid->getPassword(); // initialized to ""
	if (password_new != PASSWORD_FILLER && password_new != password_old)
	{
		// store account authentication data
		grid->setPassword(password_new);
	}

	FloaterGridManager::getInstance()->getChild<LLScrollListCtrl>("grid_selector")->setEnabled(true);
	FloaterGridManager::getInstance()->getChild<LLLineEditor>("gridnick")->setEnabled(false);

	if (getGridState() == GRID_STATE_NEW || getGridState() == GRID_STATE_COPY)
	{
		gHippoGridManager->addGrid(grid);
		refreshGrids();
		FloaterGridManager::getInstance()->getChild<LLScrollListCtrl>("grid_selector")->selectByValue(LLSD(grid_nick));
	}

	// just in case
	gHippoGridManager->setCurrentGrid(grid_nick);

	// should this be settable?
	if (grid->isUsernameCompat())
	{
		LLPanelLogin::setFields(grid->getUsername(), grid->getPassword());
	}
	else
	{
		LLPanelLogin::setFields(grid->getFirstName(), grid->getLastName(), grid->getPassword());
	}

	if (FloaterGridDefault::instanceVisible())
	{
		FloaterGridDefault::getInstance()->refreshGridList();
	}

	setGridState(GRID_STATE_NORMAL);
}

void FloaterGridManager::clearGridInfo(bool clear_all)
{
	if (clear_all)
	{
		getChild<LLLineEditor>("gridnick")->clear();
		getChild<LLLineEditor>("gridname")->clear();
		getChild<LLLineEditor>("loginuri")->clear();
		getChild<LLLineEditor>("password")->clear();
		getChild<LLLineEditor>("first_name")->clear();
		getChild<LLLineEditor>("last_name")->clear();
		getChild<LLLineEditor>("username")->clear();
		getChild<LLLineEditor>("avatar_password")->clear();
	}

	getChild<LLLineEditor>("loginpage")->clear();
	getChild<LLLineEditor>("helperuri")->clear();
	getChild<LLLineEditor>("website")->clear();
	getChild<LLLineEditor>("support")->clear();
	getChild<LLLineEditor>("register")->clear();
	getChild<LLLineEditor>("search")->clear();

	getChild<LLButton>("btn_gridinfo")->setEnabled(TRUE);
}

void FloaterGridManager::createNewGrid()
{
	// warn us if anything's dirty
	if (getChild<LLLineEditor>("gridnick")->isDirty() ||
		getChild<LLLineEditor>("gridname")->isDirty() ||
		getChild<LLLineEditor>("loginuri")->isDirty() ||
		getChild<LLLineEditor>("loginpage")->isDirty() ||
		getChild<LLLineEditor>("helperuri")->isDirty() ||
		getChild<LLLineEditor>("website")->isDirty() ||
		getChild<LLLineEditor>("support")->isDirty() ||
		getChild<LLLineEditor>("register")->isDirty() ||
		getChild<LLLineEditor>("password")->isDirty() ||
		getChild<LLLineEditor>("first_name")->isDirty() ||
		getChild<LLLineEditor>("last_name")->isDirty() ||
		getChild<LLLineEditor>("username")->isDirty() ||
		getChild<LLLineEditor>("avatar_password")->isDirty() ||
		getChild<LLLineEditor>("search")->isDirty())
	{
		LLNotifications::instance().add("GridInfoDirty", LLSD(), LLSD(), newGridCreationCallback);
	}
	else
	{
		// This clears the form. Until an entry is entered, we keep the UI open and blank
		// This is clearly undesirable if you want to cancel, but redoing this window is
		// a WIP -- MC
		setupNewGridEntry();
	}
}

void FloaterGridManager::setupNewGridEntry()
{
		// disable scroll list until we can use it again
		getChild<LLScrollListCtrl>("grid_selector")->setEnabled(false);

		getChild<LLLineEditor>("gridnick")->setEnabled(true);
		clearGridInfo(true);

		setGridState(GRID_STATE_NEW);
}

bool FloaterGridManager::newGridCreationCallback(const LLSD& notification, const LLSD& response)
{
	S32 option = LLNotification::getSelectedOption(notification, response);
	if (option == 0)
	{
		FloaterGridManager::getInstance()->setupNewGridEntry();
	}
	return false;
}

void FloaterGridManager::setupCopyGridEntry()
{
	// disable scroll list until we can use it again
	getChild<LLScrollListCtrl>("grid_selector")->setEnabled(false);

	getChild<LLLineEditor>("gridnick")->setEnabled(true);
	getChild<LLLineEditor>("gridnick")->clear();
	getChild<LLLineEditor>("gridname")->clear();
	getChild<LLLineEditor>("loginuri")->clear();

	setGridState(GRID_STATE_COPY);
}

void FloaterGridManager::retrieveGridInfo()
{
	std::string loginuri = childGetValue("loginuri");
	if (loginuri.empty())
	{
		LLNotifications::instance().add("GridInfoNoLoginURI");
		return;
	}

	// this can be clicked even for grids we haven't saved yet
	HippoGridInfo* grid = gHippoGridManager->getGrid(childGetValue("gridnick"));
	bool del_temp_grid = false;
	if (!grid) 
	{
		// easier than fixing the hippo grid manager
		HippoGridInfo* temp = new HippoGridInfo("");
		if (temp)
		{
			del_temp_grid = true;
			grid = temp;
		}
	}

	grid->setLoginURI(loginuri);
	if (grid->retrieveGridInfo()) 
	{
		// just in case
		if (!(grid->getGridNick().empty())) 
			getChild<LLLineEditor>("gridnick")->setText(grid->getGridNick());
		if (!(grid->getLoginURI().empty()))
			getChild<LLLineEditor>("loginuri")->setText(grid->getLoginURI());
		getChild<LLLineEditor>("gridname")->setText(grid->getGridName());
		getChild<LLLineEditor>("loginpage")->setText(grid->getLoginPage());
		getChild<LLLineEditor>("helperuri")->setText(grid->getHelperURI());
		getChild<LLLineEditor>("website")->setText(grid->getWebSite());
		getChild<LLLineEditor>("support")->setText(grid->getSupportURL());
		getChild<LLLineEditor>("register")->setText(grid->getRegisterURL());
		getChild<LLLineEditor>("password")->setText(grid->getPasswordURL());
		getChild<LLLineEditor>("search")->setText(grid->getSearchURL());
	}
	else
	{
		LLNotifications::instance().add("GridInfoError");
	}
	
	if (del_temp_grid)
	{
		delete grid;
	}
}

void FloaterGridManager::apply()
{
	// where all the magic happens! 
	applyChanges();

	gHippoGridManager->saveFile();
	LLPanelLogin::addServer(LLViewerLogin::getInstance()->getGridLabel());
	LLPanelLogin::loadLoginForm();
	LLPanelLogin::loadLoginPage();
}

// static
void FloaterGridManager::onSelectGrid(LLUICtrl* ctrl, void* data)
{
	FloaterGridManager* self = (FloaterGridManager*)data;
	if (self)
	{
		self->setCurGrid(ctrl->getValue().asString());
		self->refreshGrids();
	}
}

//static
void FloaterGridManager::onClickDelete(void* data)
{
	//llinfos << "onClickDelete" << llendl;
	FloaterGridManager* self = (FloaterGridManager*)data;
	if (self)
	{
		gHippoGridManager->deleteGrid(self->getCurGrid());
	}
	self->update();
}

//static
void FloaterGridManager::onClickAdd(void* data)
{
	//llinfos << "onClickAdd" << llendl;
	FloaterGridManager* self = (FloaterGridManager*)data;
	if (self)
	{
		self->createNewGrid();
	}
}


//static
void FloaterGridManager::onClickCopy(void* data)
{
	FloaterGridManager* self = (FloaterGridManager*)data;
	if (self)
	{
		self->setupCopyGridEntry();
	}
}

// static
void FloaterGridManager::onClickOk(void* data)
{
	FloaterGridManager* self = (FloaterGridManager*)data;
	if (self)
	{
		self->apply();
		self->close();
	}
}

//static
void FloaterGridManager::onClickApply(void* data)
{
	FloaterGridManager* self = (FloaterGridManager*)data;
	if (self)
	{
		self->apply();
	}
	refreshGrids();
}

// static
void FloaterGridManager::onClickClear(void* data)
{
	FloaterGridManager* self = (FloaterGridManager*)data;
	if (self)
	{
		self->clearGridInfo(false);
	}
}

//static
void FloaterGridManager::onClickGridInfo(void* data)
{
	FloaterGridManager* self = (FloaterGridManager*)data;
	if (self)
	{
		self->retrieveGridInfo();
	}
}

//static
void FloaterGridManager::onClickCancel(void* data)
{
	FloaterGridManager* self = (FloaterGridManager*)data;
	if (self)
	{
		self->close();
	}
}

//void FloaterGridManager::refreshLocation(bool force_visible)
//{
//	llinfos << "refreshLocation called" << llendl;
//	
//	if (!FloaterGridManager::instanceVisible()) 
//	{
//		return;
//	}

	//LLComboBox* combo = FloaterGridManager::getInstance()->getChild<LLComboBox>("start_location_combo");
	//
	//if (LLURLSimString::parse())
	//{
	//	combo->setCurrentByIndex( 3 );		// BUG?  Maybe 2?
	//	combo->setTextEntry(LLURLSimString::FloaterGridManager::getInstance().mSimString);
	//}
	//else
	//{
	//	BOOL login_last = gSavedSettings.getBOOL("LoginLastLocation");
	//	combo->setCurrentByIndex( login_last ? 1 : 0 );
	//}
	//
	//BOOL show_start = TRUE;
	//
	//if (!force_visible)
	//	show_start = gSavedSettings.getBOOL("ShowStartLocation");
	//
	//FloaterGridManager::getInstance()->childSetVisible("start_location_combo", show_start);
	//FloaterGridManager::getInstance()->childSetVisible("start_location_text", show_start);
	//FloaterGridManager::getInstance()->childSetVisible("server_combo", TRUE);
//}

BOOL FloaterGridManager::isGridComboDirty()
{
	BOOL user_picked = FALSE;
	if (!FloaterGridManager::instanceVisible())
	{
		llwarns << "Attempted getServer with no login view shown" << llendl;
	}
	else
	{
		LLComboBox* combo = FloaterGridManager::getInstance()->getChild<LLComboBox>("server_combo");
		user_picked = combo->isDirty();
	}
	return user_picked;
}

//void FloaterGridManager::getLocation(std::string& location)
//{
//	if (!FloaterGridManager::instanceVisible())
//	{
//		llwarns << "Attempted getLocation with no login view shown" << llendl;
//		return;
//	}
//	
//	LLComboBox* combo = FloaterGridManager::getInstance()->getChild<LLComboBox>("start_location_combo");
//	location = combo->getValue().asString();
//}
