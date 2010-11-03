/*
 *  floatergridmanager.cpp
 *  This is Meerkats grid manager.
 *  -Patrick Sapinski (Monday, August 17, 2009)
 *
 *  Modified by McCabe Maxsted for Imprudence
 */

#include "llviewerprecompiledheaders.h"

#include "floatergridmanager.h"

#include <boost/algorithm/string.hpp>
#include "llviewercontrol.h"
#include "llcombobox.h"
#include "llscrolllistctrl.h"
#include "llmd5.h"
#include "llurlsimstring.h"
#include "lluictrlfactory.h"
#include "hippogridmanager.h"
#include "llviewernetwork.h"
#include "llpanellogin.h"

#define PASSWORD_FILLER "123456789!123456"

LoginController* FloaterGridManager::sController = NULL;
bool FloaterGridManager::sIsInitialLogin;
std::string FloaterGridManager::sGrid;

FloaterGridManager::FloaterGridManager(const LLSD& key)
:	mState(NORMAL),
	mCurGrid(gHippoGridManager->getCurrentGridNick()),
	mIncomingPassword(""),
	mMungedPassword("")
{
	llinfos << "Opening grid manager" << llendl;

	LLUICtrlFactory::getInstance()->buildFloater(this, "floater_grid_manager.xml");
	center();

	LLLineEditor* edit = getChild<LLLineEditor>("avatar_password_edit");
	if (edit)
	{
		edit->setDrawAsterixes(TRUE);
	}
}


FloaterGridManager::~FloaterGridManager()
{
	FloaterGridManager::sController = NULL;
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
	// childSetAction("set_default", onClickDefault, this); // We use the last selected grid as the default
	childSetAction("btn_cancel", onClickCancel, this);
	childSetAction("btn_clear", onClickClear, this);
	childSetAction("btn_gridinfo", onClickGridInfo, this);
	// childSetAction("btn_help_render_compat", onClickHelpRenderCompat, this); // Ugly ugly ugly

	childSetCommitCallback("grid_selector", onSelectGrid, this);
	LLScrollListCtrl* combo = getChild<LLScrollListCtrl>("grid_selector");
	combo->setFocus(TRUE);

	refreshGrids();

	return TRUE;
}

void FloaterGridManager::clearInfo()
{
	//getChild<LLScrollListCtrl>("grid_selector")->clear();
	//getChild<LLLineEditor>("gridnick")->clear();
	//getChild<LLLineEditor>("gridname")->clear();
	//getChild<LLLineEditor>("loginuri")->clear();
	getChild<LLLineEditor>("loginpage")->clear();
	getChild<LLLineEditor>("helperuri")->clear();
	getChild<LLLineEditor>("website")->clear();
	getChild<LLLineEditor>("support")->clear();
	getChild<LLLineEditor>("register")->clear();
	getChild<LLLineEditor>("password")->clear();
	//getChild<LLLineEditor>("first_name")->clear();
	//getChild<LLLineEditor>("last_name")->clear();
	//getChild<LLLineEditor>("avatar_password")->clear();
	getChild<LLLineEditor>("search")->clear();
	//getChild<LLButton>("btn_delete");
	//getChild<LLButton>("btn_add")
	//getChild<LLButton>("btn_copy");
	//getChild<LLButton>("set_default")->setEnabled(FALSE);
	getChild<LLButton>("btn_gridinfo")->setEnabled(TRUE);
	//getChild<LLButton>("btn_clear");
	//getChild<LLButton>("btn_help_render_compat")->setEnabled(FALSE);
}

void FloaterGridManager::draw()
{
	refresh();

	LLFloater::draw();
}

void FloaterGridManager::refresh()
{
	//TODO: these should just be set in a prompt
	bool can_edit = (!getChild<LLLineEditor>("gridnick")->getText().empty() &&
					 !getChild<LLLineEditor>("loginuri")->getText().empty() &&
					 getChild<LLLineEditor>("gridnick")->getText() != "<required>" &&
					 getChild<LLLineEditor>("loginuri")->getText() != "<required>");

	//getChild<LLLineEditor>("gridname");
	getChild<LLLineEditor>("loginpage")->setEnabled(can_edit);
	getChild<LLLineEditor>("helperuri")->setEnabled(can_edit);
	getChild<LLLineEditor>("website")->setEnabled(can_edit);
	getChild<LLLineEditor>("support")->setEnabled(can_edit);
	getChild<LLLineEditor>("register")->setEnabled(can_edit);
	getChild<LLLineEditor>("password")->setEnabled(can_edit);
	//getChild<LLLineEditor>("first_name")->setEnabled(can_edit);
	//getChild<LLLineEditor>("last_name")->setEnabled(can_edit);
	//getChild<LLLineEditor>("avatar_password")->setEnabled(can_edit);
	getChild<LLLineEditor>("search")->setEnabled(can_edit);
	getChild<LLButton>("btn_delete")->setEnabled(can_edit);
	//getChild<LLButton>("btn_add")
	//getChild<LLButton>("btn_copy")->setEnabled(can_edit);
	//getChild<LLButton>("set_default")->setEnabled(can_dit);
	getChild<LLButton>("btn_gridinfo")->setEnabled(can_edit);
	getChild<LLButton>("btn_clear")->setEnabled(can_edit);
	//getChild<LLButton>("btn_help_render_compat")->setEnabled(can_edit);
}

void FloaterGridManager::refreshGrids()
{
	LLScrollListCtrl *grids = FloaterGridManager::getInstance()->getChild<LLScrollListCtrl>("grid_selector");
	std::string lastSelectedItem;
	LLSD element;

	if (grids->getFirstSelected())
	{
		lastSelectedItem = grids->getFirstSelected()->getValue().asString();
	}

	grids->deleteAllItems();

	for (HippoGridManager::GridIterator it = gHippoGridManager->beginGrid(); it != gHippoGridManager->endGrid(); ++it) 
	{
		std::string grid_nick = it->second->getGridNick();
		// There's no reason why empty grids nicks should be in this list, ugh
		if (!grid_nick.empty() && grid_nick != gHippoGridManager->getCurrentGridNick()) 
		{
			element["id"] = grid_nick;
			element["columns"][0]["column"] = "grid";
			element["columns"][0]["type"] = "text";
			element["columns"][0]["value"] = grid_nick;
			grids->addElement(element, ADD_BOTTOM);
		}
	}

	// Setting the default needs to be rethought. 
	// Right now, we just use the last-selected grid,
	// but leaving this as the current behavior for now
	if (!gHippoGridManager->getCurrentGridNick().empty()) 
	{
		element["id"] = gHippoGridManager->getCurrentGridNick();
		element["columns"][0]["column"] = "grid";
		element["columns"][0]["type"] = "text";
		element["columns"][0]["font-style"] = "BOLD";
		element["columns"][0]["value"] = gHippoGridManager->getCurrentGridNick();
		grids->addElement(element, ADD_TOP);
	}

	// Reselect the item if we had one selected
	if (lastSelectedItem.empty())
	{
		grids->selectItemByLabel(gHippoGridManager->getCurrentGridNick());
	}
	else
	{
		grids->selectItemByLabel(lastSelectedItem);
	}

	// TODO: get rid of all this state junk
	if ((FloaterGridManager::getInstance()->getState() == ADD_NEW) || (FloaterGridManager::getInstance()->getState() == ADD_COPY)) 
	{
		grids->addElement("<new>", ADD_BOTTOM);
	}

	//if (selectIndex >= 0) 
	//{
	//	grids->setCurrentByIndex(selectIndex);
	//} 
	//else 
	//{
	//	grids->setLabel(LLStringExplicit(""));  // LLComboBox::removeall() does not clear the label
	//}
			
	// FloaterGridManager::getInstance()->childSetTextArg("default_grid", "[DEFAULT]", (defaultGrid != "")? defaultGrid: " ");

	FloaterGridManager::getInstance()->childSetEnabled("btn_delete", grids->getItemCount() > 0);

	FloaterGridManager::getInstance()->childSetEnabled("btn_copy", (FloaterGridManager::getInstance()->getState() == NORMAL) && (grids->getItemCount() > 0));
	// FloaterGridManager::getInstance()->childSetEnabled("set_default", (FloaterGridManager::getInstance()->getState() == NORMAL) && (grids->getItemCount() > 0));
	FloaterGridManager::getInstance()->childSetEnabled("gridnick", (FloaterGridManager::getInstance()->getState() == ADD_NEW) || (FloaterGridManager::getInstance()->getState() == ADD_COPY));

	if (FloaterGridManager::getInstance()->getState() == NORMAL) 
	{
		HippoGridInfo *gridInfo = gHippoGridManager->getGrid(FloaterGridManager::getInstance()->getCurGrid());
		if (gridInfo) 
		{
			FloaterGridManager::getInstance()->childSetText("gridnick", gridInfo->getGridNick());
			//FloaterGridManager::getInstance()->childSetText("grid_name", gridInfo->getGridName());
			FloaterGridManager::getInstance()->childSetText("loginuri", gridInfo->getLoginUri());
			FloaterGridManager::getInstance()->childSetText("loginpage", gridInfo->getLoginPage());
			FloaterGridManager::getInstance()->childSetText("helperuri", gridInfo->getHelperUri());
			FloaterGridManager::getInstance()->childSetText("website", gridInfo->getWebSite());
			FloaterGridManager::getInstance()->childSetText("support", gridInfo->getSupportUrl());
			FloaterGridManager::getInstance()->childSetText("register", gridInfo->getRegisterUrl());
			FloaterGridManager::getInstance()->childSetText("password", gridInfo->getPasswordUrl());

			// FloaterGridManager::getInstance()->childSetText("first_name", gridInfo->getFirstName());
			// FloaterGridManager::getInstance()->childSetText("last_name", gridInfo->getLastName());
			// if(gridInfo->getAvatarPassword().length() == 32)
			// 	FloaterGridManager::getInstance()->childSetText("avatar_password", std::string(PASSWORD_FILLER));
			// else if(gridInfo->getPasswordUrl().empty())
			// 	FloaterGridManager::getInstance()->childSetText("avatar_password", std::string(""));

			 if (gridInfo->getPlatform() == HippoGridInfo::PLATFORM_SECONDLIFE) 
			 {
			 	FloaterGridManager::getInstance()->childSetEnabled("search", false);
				FloaterGridManager::getInstance()->childSetText("search", LLStringUtil::null);
			 	//childSetEnabled("render_compat", false);
			 	//childSetValue("render_compat", false);
			 } 
			 else 
			 {
			 	FloaterGridManager::getInstance()->childSetEnabled("search", true);
			 	FloaterGridManager::getInstance()->childSetText("search", gridInfo->getSearchUrl());
			 	//childSetEnabled("render_compat", true);
			 	//childSetValue("render_compat", gridInfo->isRenderCompat());
			 }

		}	
		else 
		{
			FloaterGridManager::getInstance()->childSetText("gridnick", LLStringUtil::null);
			FloaterGridManager::getInstance()->childSetText("gridname", LLStringUtil::null);
			FloaterGridManager::getInstance()->childSetText("loginuri", LLStringUtil::null);
			FloaterGridManager::getInstance()->childSetText("loginpage", LLStringUtil::null);
			FloaterGridManager::getInstance()->childSetText("helperuri", LLStringUtil::null);
			FloaterGridManager::getInstance()->childSetText("website", LLStringUtil::null);
			FloaterGridManager::getInstance()->childSetText("support", LLStringUtil::null);
			FloaterGridManager::getInstance()->childSetText("register", LLStringUtil::null);
			FloaterGridManager::getInstance()->childSetText("password", LLStringUtil::null);
			// FloaterGridManager::getInstance()->childSetText("first_name", LLStringUtil::null);
			// FloaterGridManager::getInstance()->childSetText("last_name", LLStringUtil::null);
			// FloaterGridManager::getInstance()->childSetText("avatar_password", LLStringUtil::null);
			FloaterGridManager::getInstance()->childSetText("search", LLStringUtil::null);
		}
	} 
	else if (FloaterGridManager::getInstance()->getState() == ADD_NEW) 
	{
		llinfos << "mState == ADD_NEW" << llendl;
		std::string required = "<required>";
		FloaterGridManager::getInstance()->childSetText("gridnick", required);
		FloaterGridManager::getInstance()->childSetText("gridname", LLStringUtil::null);
		FloaterGridManager::getInstance()->childSetText("loginuri", required);
		FloaterGridManager::getInstance()->childSetText("loginpage", LLStringUtil::null);
		FloaterGridManager::getInstance()->childSetText("helperuri", LLStringUtil::null);
		FloaterGridManager::getInstance()->childSetText("website", LLStringUtil::null);
		FloaterGridManager::getInstance()->childSetText("support", LLStringUtil::null);
		FloaterGridManager::getInstance()->childSetText("register", LLStringUtil::null);
		FloaterGridManager::getInstance()->childSetText("password", LLStringUtil::null);
		// FloaterGridManager::getInstance()->childSetText("first_name", LLStringUtil::null);
		// FloaterGridManager::getInstance()->childSetText("last_name", LLStringUtil::null);
		// FloaterGridManager::getInstance()->childSetText("avatar_password", LLStringUtil::null);
		//childSetEnabled("search", true);
		FloaterGridManager::getInstance()->childSetText("search", LLStringUtil::null);
	} 
	else if (FloaterGridManager::getInstance()->getState() == ADD_COPY) 
	{
		llinfos << "mState == ADD_COPY" << llendl;
		FloaterGridManager::getInstance()->childSetText("gridnick", LLStringExplicit("<required>"));
	} 
	else 
	{
		llwarns << "Illegal state " << FloaterGridManager::getInstance()->getState() << llendl; 	
	}
	return;
}

void FloaterGridManager::update()
{
	setState(NORMAL);
	setCurGrid(gHippoGridManager->getCurrentGridNick());
	refreshGrids();
	//KOW gHippoLimits->setLimits();
}

void FloaterGridManager::applyChanges()
{ 
	HippoGridInfo* gridInfo = gHippoGridManager->getGrid(mCurGrid);
	if (gridInfo) 
	{
		if (gridInfo->getGridNick() == childGetValue("gridnick").asString()) 
		{
			gridInfo->setGridName(childGetValue("gridname"));
			gridInfo->setLoginUri(childGetValue("loginuri"));
			gridInfo->setLoginPage(childGetValue("loginpage"));
			gridInfo->setHelperUri(childGetValue("helperuri"));
			gridInfo->setWebSite(childGetValue("website"));
			gridInfo->setSupportUrl(childGetValue("support"));
			gridInfo->setRegisterUrl(childGetValue("register"));
			gridInfo->setPasswordUrl(childGetValue("password"));
			gridInfo->setSearchUrl(childGetValue("search"));
			gridInfo->setRenderCompat(childGetValue("render_compat"));
			
			// gridInfo->setFirstName(childGetValue("first_name"));
			// gridInfo->setLastName(childGetValue("last_name"));
			// if(childGetValue("avatar_password").asString().empty())
			// 	gridInfo->setAvatarPassword(std::string(""));
			// else if(childGetValue("avatar_password").asString() != std::string(PASSWORD_FILLER))
			// {
			// 	// store account authentication data
			// 	std::string auth_password = childGetValue("avatar_password");
			// 	std::string hashed_password;
			// 	hashPassword(auth_password, hashed_password);
			// 	gridInfo->setAvatarPassword(hashed_password);
			// }

			//this bug was a feature -Patrick Sapinski (Friday, August 21, 2009)
			//LLPanelLogin::setFields(gridInfo->getFirstName(), gridInfo->getLastName(),
			//						gridInfo->getAvatarPassword(), true);
		} 
		else 
		{
			llwarns << "Grid nickname mismatch, ignoring changes." << llendl;
		}
	}
}


bool FloaterGridManager::createNewGrid()
{
	// check nickname
	std::string gridnick = childGetValue("gridnick");
	if (gridnick == "<required>") 
	{
		gridnick = "";
	}

	if (gridnick.empty()) 
	{
		LLNotifications::instance().add("GridsNoNick");
		return false;
	}

	if (gHippoGridManager->getGrid(gridnick)) 
	{
		LLSD args;
		args["[NAME]"] = gridnick;
		LLNotifications::instance().add("GridExists", args);
		return false;
	}

	// check login URI
	std::string loginuri = childGetValue("loginuri");
	if ((loginuri.empty()) || (loginuri == "<required>")) 
	{
		LLSD args;
		args["[NAME]"] = gridnick;
		LLNotifications::instance().add("GridsNoLoginUri", args);
		return false;
	}

	// create new grid
	HippoGridInfo* grid = new HippoGridInfo(gridnick);
	grid->setGridName(childGetValue("gridname"));
	grid->setLoginUri(loginuri);
	grid->setLoginPage(childGetValue("loginpage"));
	grid->setHelperUri(childGetValue("helperuri"));
	grid->setWebSite(childGetValue("website"));
	grid->setSupportUrl(childGetValue("support"));
	grid->setRegisterUrl(childGetValue("register"));
	grid->setPasswordUrl(childGetValue("password"));
	grid->setSearchUrl(childGetValue("search"));
	grid->setRenderCompat(childGetValue("render_compat"));
	gHippoGridManager->addGrid(grid);
	
	// grid->setFirstName(childGetValue("first_name"));
	// grid->setLastName(childGetValue("last_name"));
	// if(childGetValue("avatar_password").asString().empty())
	// 	grid->setAvatarPassword(std::string(""));
	// else
	// {
	// 	std::string hashed_password;
	// 	hashPassword(childGetValue("avatar_password"), hashed_password);
	// 	grid->setAvatarPassword(hashed_password);
	// }
	
	setCurGrid(gridnick);
	return true;
}

void FloaterGridManager::retrieveGridInfo()
{
	std::string loginuri = childGetValue("loginuri");
	if ((loginuri == "") || (loginuri == "<required>")) 
	{
		LLNotifications::instance().add("GridInfoNoLoginUri");
		return;
	}

	HippoGridInfo* grid = 0;
	bool cleanupGrid = false;

	if (mState == NORMAL) 
	{
		grid = gHippoGridManager->getGrid(mCurGrid);
	} 
	else if ((mState == ADD_NEW) || (mState == ADD_COPY)) 
	{
		grid = new HippoGridInfo("");
		cleanupGrid = true;
	} 
	else 
	{
		llerrs << "Illegal state " << mState << '.' << llendl;
		return;
	}
	if (!grid) 
	{
		llerrs << "Internal error retrieving grid info." << llendl;
		return;
	}

	grid->setLoginUri(loginuri);
	if (grid->retrieveGridInfo()) 
	{
		if (grid->getGridNick() != "") childSetText("gridnick", grid->getGridNick());
		if (grid->getGridName() != "") childSetText("gridname", grid->getGridName());
		if (grid->getLoginUri() != "") childSetText("loginuri", grid->getLoginUri());
		if (grid->getLoginPage() != "") childSetText("loginpage", grid->getLoginPage());
		if (grid->getHelperUri() != "") childSetText("helperuri", grid->getHelperUri());
		if (grid->getWebSite() != "") childSetText("website", grid->getWebSite());
		if (grid->getSupportUrl() != "") childSetText("support", grid->getSupportUrl());
		if (grid->getRegisterUrl() != "") childSetText("register", grid->getRegisterUrl());
		if (grid->getPasswordUrl() != "") childSetText("password", grid->getPasswordUrl());
		if (grid->getSearchUrl() != "") childSetText("search", grid->getSearchUrl());
	} 
	else 
	{
		LLNotifications::instance().add("GridInfoError");
	}

	if (cleanupGrid) delete grid;
}

void FloaterGridManager::apply()
{
	if (mState == NORMAL) 
	{
		applyChanges();
	} 
	else if ((mState == ADD_NEW) || (mState == ADD_COPY)) 
	{
		if (!createNewGrid()) return;
	} 
	else 
	{
		llwarns << "Illegal state " << mState << '.' << llendl;
		return;
	}
	//gHippoGridManager->setCurrentGrid(mCurGrid);
	//gHippoGridManager->setDefaultGrid(mCurGrid);
	gHippoGridManager->saveFile();
	LLPanelLogin::addServer(LLViewerLogin::getInstance()->getGridLabel());
}

//void FloaterGridManager::setDefault()
//{
//	if (mState == NORMAL) 
//	{
//		applyChanges();
//	} 
//	else if ((mState == ADD_NEW) || (mState == ADD_COPY)) 
//	{
//		if (!createNewGrid()) return;
//	} 
//	else 
//	{
//		llwarns << "Illegal state " << mState << '.' << llendl;
//		return;
//	}
//	gHippoGridManager->setCurrentGrid(mCurGrid);
//	gHippoGridManager->setDefaultGrid(mCurGrid);
//	gHippoGridManager->saveFile();
//	LLPanelLogin::addServer(LLViewerLogin::getInstance()->getGridLabel());
//}

void FloaterGridManager::onSelectGrid(LLUICtrl* ctrl, void* data)
{
	FloaterGridManager* self = (FloaterGridManager*)data;
	if (self->getState() == NORMAL) 
	{
		self->applyChanges();
	} 
	else if ((self->getState() == ADD_NEW) || (self->getState() == ADD_COPY)) 
	{
		if (self->createNewGrid()) 
		{
			self->setState(NORMAL);
		} 
		else 
		{
			//LLScrollListCtrl *grids = self->getChild<LLScrollListCtrl>("grid_selector");
			//grids->setCurrentByIndex(grids->getItemCount() - 1);
			return;
		}
	} 
	else 
	{
		llwarns << "Illegal state " << self->getState() << llendl;
		return;
	}
	self->setCurGrid(ctrl->getValue().asString());
	self->refreshGrids();
}

//static
void FloaterGridManager::onClickDelete(void* data)
{
	//llinfos << "onClickDelete" << llendl;
	FloaterGridManager* self = (FloaterGridManager*)data;
	if (self->getState() == NORMAL)
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
	self->setState(ADD_NEW);
	self->refreshGrids();
}


//static
void FloaterGridManager::onClickCopy(void* data)
{
	//llinfos << "onClickCopy" << llendl;
	FloaterGridManager* self = (FloaterGridManager*)data;
	self->setState(ADD_COPY);
	self->refreshGrids();
}

// static
void FloaterGridManager::onClickOk(void* data)
{
	FloaterGridManager::getInstance()->apply();
	FloaterGridManager::getInstance()->close();
}

//static
void FloaterGridManager::onClickApply(void* data)
{
	FloaterGridManager::getInstance()->apply();
	refreshGrids();
}

// static
void FloaterGridManager::onClickClear(void* data)
{
	FloaterGridManager::getInstance()->clearInfo();
}

//static
//void FloaterGridManager::onClickDefault(void* data)
//{
//	FloaterGridManager::getInstance()->setDefault();
//	FloaterGridManager::getInstance()->refreshGrids();
//}

//static
void FloaterGridManager::onClickGridInfo(void* data)
{
	//HippoPanelGrids* self = (HippoPanelGrids*)data;
	FloaterGridManager::getInstance()->retrieveGridInfo();
}

//static
void FloaterGridManager::onClickCancel(void* data)
{
	FloaterGridManager::getInstance()->close();
}

//void FloaterGridManager::setAlwaysRefresh(bool refresh)
//{
//	// wargames 2: dead code, LLPanelLogin compatibility
//	return;
//}

//void FloaterGridManager::refreshLocation( bool force_visible )
//{
//	llinfos << "refreshLocation called" << llendl;
//	
//	if (!FloaterGridManager::getInstance()) 
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

//void FloaterGridManager::setFocus(BOOL b)
//{
//	if(b != hasFocus())
//	{
//		if(b)
//		{
//			FloaterGridManager::giveFocus();
//		}
//		else
//		{
//			LLPanel::setFocus(b);
//		}
//	}
//}
//
//void FloaterGridManager::giveFocus()
//{
//	LLScrollListCtrl *combo = NULL;
//	
//	if (!FloaterGridManager::getInstance())
//	{
//		llinfos << "giveFocus has no FloaterGridManager instance. FloaterGridManager::getInstance()=" << FloaterGridManager::getInstance() << llendl;
//		return;
//	}
//	
//	// for our combo box approach, selecting the combo box is almost always
//	// the right thing to do on the floater receiving focus
//	combo = FloaterGridManager::getInstance()->getChild<LLScrollListCtrl>("grid_selector");
//	combo->setFocus(TRUE);
//}

BOOL FloaterGridManager::isGridComboDirty()
{
	BOOL user_picked = FALSE;
	if (!FloaterGridManager::getInstance())
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

void FloaterGridManager::getLocation(std::string &location)
{
	if (!FloaterGridManager::getInstance())
	{
		llwarns << "Attempted getLocation with no login view shown" << llendl;
		return;
	}
	
	LLComboBox* combo = FloaterGridManager::getInstance()->getChild<LLComboBox>("start_location_combo");
	location = combo->getValue().asString();
}

std::string& FloaterGridManager::getPassword()
{
	return mMungedPassword;
}

void FloaterGridManager::setPassword(std::string &password)
{
	mMungedPassword = password;
}

bool FloaterGridManager::isSamePassword(std::string &password)
{
	return mMungedPassword == password;
}

//void FloaterGridManager::addServer(const std::string& server, S32 domain_name)
//{
//	if (!FloaterGridManager::getInstance())
//	{
//		llwarns << "Attempted addServer with no login view shown" << llendl;
//		return;
//	}
//	
//	/*LLComboBox* combo = FloaterGridManager::getInstance()->getChild<LLComboBox>("server_combo");
//	combo->add(server, LLSD(domain_name) );
//	combo->setCurrentByIndex(0);*/
//}

//void FloaterGridManager::cancel_old()
//{
//	if (!FloaterGridManager::getInstance())
//	{
//		return;
//	}
//
//	if (FloaterGridManager::getInstance()->sIsInitialLogin)
//	{
//		// send a callback that indicates we're quitting or closing
//		if (FloaterGridManager::getInstance()->mCallback)
//			FloaterGridManager::getInstance()->mCallback(LOGIN_OPTION_QUIT, FloaterGridManager::getInstance()->mCallbackData);
//		return;
//	}
//	
//	FloaterGridManager::getInstance()->close();
//}

void FloaterGridManager::hashPassword(const std::string& password, std::string& hashedPassword)
{
	// Max "actual" password length is 16 characters.
	// Hex digests are always 32 characters.
	if (password.length() == 32)
	{
		hashedPassword = password;
	}
	else
	{
		// this is a normal text password
		LLMD5 pass((unsigned char *)password.c_str());
		char munged_password[MD5HEX_STR_SIZE];
		pass.hex_digest(munged_password);
		hashedPassword = munged_password;
	}
}
