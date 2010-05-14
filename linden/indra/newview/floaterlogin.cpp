/*
 *  floaterlogin.cpp (floatergridmanager.cpp pls)
 *  This is Meerkats grid manager, and I accidentally finished it with the wrong name :)
 *  -Patrick Sapinski (Monday, August 17, 2009)
 */

#include "llviewerprecompiledheaders.h"

#include <boost/algorithm/string.hpp>
#include "llviewercontrol.h"
#include "llcombobox.h"
#include "llscrolllistctrl.h"
#include "llmd5.h"
#include "llurlsimstring.h"
#include "lluictrlfactory.h"
#include "floaterlogin.h"
#include "hippoGridManager.h"
#include "llviewernetwork.h"
#include "llpanellogin.h"

#define PASSWORD_FILLER "123456789!123456"

LoginFloater* LoginFloater::sInstance = NULL;
LoginController* LoginFloater::sController = NULL;
bool LoginFloater::sIsInitialLogin;
std::string LoginFloater::sGrid;

LoginFloater::LoginFloater()
:	LLFloater("floater_login")
{
	
	mState = NORMAL;
	LLUICtrlFactory::getInstance()->buildFloater(this, "floater_login.xml");
	
	
	llwarns << "LoginFloater called" << llendl;
	

	// configure the floater interface for non-initial login
	setCanMinimize(!sIsInitialLogin);
	setCanClose(!sIsInitialLogin);
	setCanDrag(!sIsInitialLogin);
	childSetVisible("server_combo", sIsInitialLogin);
	
	if(!sIsInitialLogin)
	{
		LLButton* quit_btn = getChild<LLButton>("quit_btn");
		quit_btn->setLabel(std::string("Cancel"));
		setTitle(std::string("Grid Manager"));
	}
	
	center();
	LLLineEditor* edit = getChild<LLLineEditor>("avatar_password_edit");
	if (edit) edit->setDrawAsterixes(TRUE);
	LLComboBox* combo = getChild<LLComboBox>("start_location_combo");
	combo->setAllowTextEntry(TRUE, 128, FALSE);
	
	BOOL login_last = gSavedSettings.getBOOL("LoginLastLocation");
	std::string sim_string = LLURLSimString::sInstance.mSimString;
	if (!sim_string.empty())
	{
		// Replace "<Type region name>" with this region name
		combo->remove(2);
		combo->add( sim_string );
		combo->setTextEntry(sim_string);
		combo->setCurrentByIndex( 2 );
	}
	else if (login_last)
	{
		combo->setCurrentByIndex( 1 );
	}
	else
	{
		combo->setCurrentByIndex( 0 );
	}
}


LoginFloater::~LoginFloater()
{
	LoginFloater::sController = NULL;
	LoginFloater::sInstance = NULL;
}

void LoginFloater::close()
{
	if(sInstance)
	{
		delete sInstance;
		sInstance = NULL;
	}
}

BOOL LoginFloater::postBuild()
{
	requires<LLScrollListCtrl>("grid_selector");
	requires<LLLineEditor>("gridnick");
	requires<LLLineEditor>("gridname");
	requires<LLLineEditor>("loginuri");
	requires<LLLineEditor>("loginpage");
	requires<LLLineEditor>("helperuri");
	requires<LLLineEditor>("website");
	requires<LLLineEditor>("support");
	requires<LLLineEditor>("register");
	requires<LLLineEditor>("password");
	requires<LLLineEditor>("first_name");
	requires<LLLineEditor>("last_name");
	requires<LLLineEditor>("avatar_password");
	//requires<LLLineEditor>("search");
	requires<LLButton>("btn_delete");
	requires<LLButton>("btn_add");
	requires<LLButton>("btn_copy");
	requires<LLButton>("set_default");
	requires<LLButton>("btn_gridinfo");
	requires<LLButton>("btn_help_render_compat");
	if (!checkRequirements()) return false;
	LLLineEditor* password_edit = getChild<LLLineEditor>("avatar_password");
	if (password_edit) password_edit->setDrawAsterixes(TRUE);

	childSetAction("btn_delete", onClickDelete, this);
	childSetAction("btn_add", onClickAdd, this);
	childSetAction("btn_copy", onClickCopy, this);
	childSetAction("btn_ok", onClickOk, this);
	childSetAction("btn_apply", onClickApply, this);
	// childSetAction("set_default", onClickDefault, this);
	childSetAction("btn_cancel", onClickCancel, this);
	childSetAction("btn_gridinfo", onClickGridInfo, this);
	// childSetAction("btn_help_render_compat", onClickHelpRenderCompat, this);

	childSetCommitCallback("grid_selector", onSelectGrid, this);
//KOW	childSetCommitCallback("platform", onSelectPlatform, this);

	// !!!### 	server_choice_combo->setFocusLostCallback(onServerComboLostFocus);

//KOW	update();
	return TRUE;
}

void LoginFloater::refresh_grids()
{
	const std::string &defaultGrid = gHippoGridManager->getDefaultGridNick();
	LLScrollListCtrl *grids = sInstance->getChild<LLScrollListCtrl>("grid_selector");
	S32 selectIndex = -1, i = 0;
	grids->deleteAllItems();
	if (defaultGrid != "") {
		LLSD value;
		value["id"] = defaultGrid;
		value["columns"][0]["column"] = "grid";
		value["columns"][0]["value"] = defaultGrid;
		grids->addElement(value);
		selectIndex = i++;
	}
	
	HippoGridManager::GridIterator it, end = gHippoGridManager->endGrid();
	for (it = gHippoGridManager->beginGrid(); it != end; ++it) {
		const std::string &grid = it->second->getGridNick();
		if (grid != defaultGrid) {
			LLSD value;
			value["id"] = grid;
			value["columns"][0]["column"] = "grid";
			value["columns"][0]["value"] = grid;
			grids->addElement(value);

			if (grid == sInstance->mCurGrid) selectIndex = i;
			i++;
		}
	}
	if ((sInstance->mState == ADD_NEW) || (sInstance->mState == ADD_COPY)) {
		grids->addElement("<new>");
		selectIndex = i++;
	}
	if (selectIndex >= 0) {
		//grids->setCurrentByIndex(selectIndex);
	} else {
		//grids->setLabel(LLStringExplicit(""));  // LLComboBox::removeall() does not clear the label
	}
			
	// sInstance->childSetTextArg("default_grid", "[DEFAULT]", (defaultGrid != "")? defaultGrid: " ");

	sInstance->childSetEnabled("btn_delete", (selectIndex >= 0));
	sInstance->childSetEnabled("btn_copy", (sInstance->mState == NORMAL) && (selectIndex >= 0));
	// sInstance->childSetEnabled("set_default", (sInstance->mState == NORMAL) && (selectIndex > 0));
	sInstance->childSetEnabled("gridnick", (sInstance->mState == ADD_NEW) || (sInstance->mState == ADD_COPY));

	if (sInstance->mState == NORMAL) {
		HippoGridInfo *gridInfo = gHippoGridManager->getGrid(sInstance->mCurGrid);
		if (gridInfo) {
			sInstance->childSetText("gridnick", gridInfo->getGridNick());
			//sInstance->childSetText("grid_name", gridInfo->getGridName());
			sInstance->childSetText("loginuri", gridInfo->getLoginUri());
			sInstance->childSetText("loginpage", gridInfo->getLoginPage());
			sInstance->childSetText("helperuri", gridInfo->getHelperUri());
			sInstance->childSetText("website", gridInfo->getWebSite());
			sInstance->childSetText("support", gridInfo->getSupportUrl());
			sInstance->childSetText("register", gridInfo->getRegisterUrl());
			sInstance->childSetText("password", gridInfo->getPasswordUrl());

			// sInstance->childSetText("first_name", gridInfo->getFirstName());
			// sInstance->childSetText("last_name", gridInfo->getLastName());
			// if(gridInfo->getAvatarPassword().length() == 32)
			// 	sInstance->childSetText("avatar_password", std::string(PASSWORD_FILLER));
			// else if(gridInfo->getPasswordUrl().empty())
			// 	sInstance->childSetText("avatar_password", std::string(""));

			// if (gridInfo->getPlatform() == HippoGridInfo::PLATFORM_SECONDLIFE) {
			// 	//childSetEnabled("search", false);
			// 	//childSetText("search", LLStringExplicit(""));
			// 	childSetEnabled("render_compat", false);
			// 	childSetValue("render_compat", false);
			// } else {
			// 	//childSetEnabled("search", true);
			// 	//childSetText("search", gridInfo->getSearchUrl());
			// 	childSetEnabled("render_compat", true);
			// 	childSetValue("render_compat", gridInfo->isRenderCompat());
			// }

		}	else {
			std::string empty = "";
			sInstance->childSetText("gridnick", empty);
			sInstance->childSetText("gridname", empty);
			sInstance->childSetText("loginuri", empty);
			sInstance->childSetText("loginpage", empty);
			sInstance->childSetText("helperuri", empty);
			sInstance->childSetText("website", empty);
			// sInstance->childSetText("first_name", empty);
			// sInstance->childSetText("last_name", empty);
			// sInstance->childSetText("avatar_password", empty);
		}
	} else if (sInstance->mState == ADD_NEW) {
			llwarns << "ADD_NEW" << llendl;
		std::string required = "<required>";
		std::string empty = "";
		sInstance->childSetText("gridnick", required);
		sInstance->childSetText("gridname", empty);
		sInstance->childSetText("loginuri", required);
		sInstance->childSetText("loginpage", empty);
		sInstance->childSetText("helperuri", empty);
		sInstance->childSetText("website", empty);
		sInstance->childSetText("support", empty);
		sInstance->childSetText("register", empty);
		sInstance->childSetText("password", empty);
		// sInstance->childSetText("first_name", empty);
		// sInstance->childSetText("last_name", empty);
		// sInstance->childSetText("avatar_password", empty);
		//childSetEnabled("search", true);
		//childSetText("search", empty);
	} else if (sInstance->mState == ADD_COPY) {
			llwarns << "ADD_COPY" << llendl;
		sInstance->childSetText("gridnick", LLStringExplicit("<required>"));
	} else {
		llwarns << "Illegal state " << sInstance->mState << '.' << llendl; 	
	}
	return;
}

void LoginFloater::update()
{
	mState = NORMAL;
	mCurGrid = gHippoGridManager->getCurrentGridNick();
	refresh_grids();
	//KOW gHippoLimits->setLimits();
}

void LoginFloater::applyChanges()
{ 
	HippoGridInfo *gridInfo = gHippoGridManager->getGrid(mCurGrid);
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
			//gridInfo->setSearchUrl(childGetValue("search"));
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


bool LoginFloater::createNewGrid()
{
	// check nickname
	std::string gridnick = childGetValue("gridnick");
	if (gridnick == "<required>") gridnick = "";
	if (gridnick == "") {
		//KOW gViewerWindow->alertXml("GridsNoNick");
		return false;
	}
	if (gHippoGridManager->getGrid(gridnick)) {
		LLStringUtil::format_map_t args;
		args["[NAME]"] = gridnick;
		//KOW gViewerWindow->alertXml("GridExists", args);
		return false;
	}

	// check login URI
	std::string loginuri = childGetValue("loginuri");
	if ((loginuri == "") || (loginuri == "<required>")) {
		LLStringUtil::format_map_t args;
		args["[NAME]"] = gridnick;
		//KOW gViewerWindow->alertXml("GridsNoLoginUri", args);
		return false;
	}

	// create new grid
	HippoGridInfo *grid = new HippoGridInfo(gridnick);
	grid->setGridName(childGetValue("gridname"));
	grid->setLoginUri(loginuri);
	grid->setLoginPage(childGetValue("loginpage"));
	grid->setHelperUri(childGetValue("helperuri"));
	grid->setWebSite(childGetValue("website"));
	grid->setSupportUrl(childGetValue("support"));
	grid->setRegisterUrl(childGetValue("register"));
	grid->setPasswordUrl(childGetValue("password"));
	//grid->setSearchUrl(childGetValue("search"));
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
	
	mCurGrid = gridnick;
	return true;
}

void LoginFloater::retrieveGridInfo()
{
	std::string loginuri = childGetValue("loginuri");
	if ((loginuri == "") || (loginuri == "<required>")) {
		//KOW gViewerWindow->alertXml("GridInfoNoLoginUri");
		return;
	}

	HippoGridInfo *grid = 0;
	bool cleanupGrid = false;
	if (mState == NORMAL) {
		grid = gHippoGridManager->getGrid(mCurGrid);
	} else if ((mState == ADD_NEW) || (mState == ADD_COPY)) {
		grid = new HippoGridInfo("");
		cleanupGrid = true;
	} else {
		llerrs << "Illegal state " << mState << '.' << llendl;
		return;
	}
	if (!grid) {
		llerrs << "Internal error retrieving grid info." << llendl;
		return;
	}

	grid->setLoginUri(loginuri);
	if (grid->retrieveGridInfo()) {
		if (grid->getGridNick() != "") childSetText("gridnick", grid->getGridNick());
		if (grid->getGridName() != "") childSetText("gridname", grid->getGridName());
		if (grid->getLoginUri() != "") childSetText("loginuri", grid->getLoginUri());
		if (grid->getLoginPage() != "") childSetText("loginpage", grid->getLoginPage());
		if (grid->getHelperUri() != "") childSetText("helperuri", grid->getHelperUri());
		if (grid->getWebSite() != "") childSetText("website", grid->getWebSite());
		if (grid->getSupportUrl() != "") childSetText("support", grid->getSupportUrl());
		if (grid->getRegisterUrl() != "") childSetText("register", grid->getRegisterUrl());
		if (grid->getPasswordUrl() != "") childSetText("password", grid->getPasswordUrl());
		//if (grid->getSearchUrl() != "") childSetText("search", grid->getSearchUrl());
	} else {
		//KOW gViewerWindow->alertXml("GridInfoError");
	}

	if (cleanupGrid) delete grid;
}

void LoginFloater::apply()
{
	if (mState == NORMAL) {
		applyChanges();
	} else if ((mState == ADD_NEW) || (mState == ADD_COPY)) {
		if (!createNewGrid()) return;
	} else {
		llwarns << "Illegal state " << mState << '.' << llendl;
		return;
	}
	//gHippoGridManager->setCurrentGrid(mCurGrid);
	//gHippoGridManager->setDefaultGrid(mCurGrid);
	//LLPanelLogin::refreshLoginPage();
	gHippoGridManager->saveFile();
	LLPanelLogin::addServer(LLViewerLogin::getInstance()->getGridLabel());
}

void LoginFloater::setDefault()
{
	if (mState == NORMAL) {
		applyChanges();
	} else if ((mState == ADD_NEW) || (mState == ADD_COPY)) {
		if (!createNewGrid()) return;
	} else {
		llwarns << "Illegal state " << mState << '.' << llendl;
		return;
	}
	gHippoGridManager->setCurrentGrid(mCurGrid);
	gHippoGridManager->setDefaultGrid(mCurGrid);
	gHippoGridManager->saveFile();
	LLPanelLogin::addServer(LLViewerLogin::getInstance()->getGridLabel());
}

void LoginFloater::cancel()
{
	gHippoGridManager->discardAndReload();
	update();
}

void LoginFloater::onSelectGrid(LLUICtrl* ctrl, void *data)
{
	LoginFloater* self = (LoginFloater*)data;
	if (self->mState == NORMAL) {
		self->applyChanges();
	} else if ((self->mState == ADD_NEW) || (self->mState == ADD_COPY)) {
		if (self->createNewGrid()) {
			self->mState = NORMAL;
		} else {
			//LLScrollListCtrl *grids = self->getChild<LLScrollListCtrl>("grid_selector");
			//grids->setCurrentByIndex(grids->getItemCount() - 1);
			return;
		}
	} else {
		llwarns << "Illegal state " << self->mState << '.' << llendl;
		return;
	}
	self->mCurGrid = ctrl->getValue().asString();
	self->refresh_grids();
}

//static
void LoginFloater::onClickDelete(void *data)
{
			llwarns << "onclickdelete" << llendl;
	LoginFloater* self = (LoginFloater*)data;
	if (self->mState == NORMAL)
		gHippoGridManager->deleteGrid(self->mCurGrid);
	self->update();
}

//static
void LoginFloater::onClickAdd(void *data)
{
			llwarns << "add" << llendl;
	LoginFloater* self = (LoginFloater*)data;
	self->mState = ADD_NEW;
	self->refresh_grids();
}


//static
void LoginFloater::onClickCopy(void *data)
{
			llwarns << "copy" << llendl;
	LoginFloater* self = (LoginFloater*)data;
	self->mState = ADD_COPY;
	self->refresh_grids();
}

// static
void LoginFloater::onClickOk(void* data)
{
	if(NULL==sInstance)
		return;

	sInstance->apply();
	sInstance->close();
}

//static
void LoginFloater::onClickApply(void *data)
{
	if(NULL==sInstance)
		return;

	sInstance->apply();
	refresh_grids();
}

//static
void LoginFloater::onClickDefault(void *data)
{
	sInstance->setDefault();
	sInstance->refresh_grids();
}

//static
void LoginFloater::onClickGridInfo(void *data)
{
	//HippoPanelGrids* self = (HippoPanelGrids*)data;
	sInstance->retrieveGridInfo();
}

//static
void LoginFloater::onClickCancel(void *data)
{
	sInstance->cancel();
}

void LoginFloater::setAlwaysRefresh(bool refresh)
{
	// wargames 2: dead code, LLPanelLogin compatibility
	return;
}

void LoginFloater::refreshLocation( bool force_visible )
{
	
		llwarns << "refreshLocation called" << llendl;
	
	if (!sInstance) return;

	LLComboBox* combo = sInstance->getChild<LLComboBox>("start_location_combo");
	
	if (LLURLSimString::parse())
	{
		combo->setCurrentByIndex( 3 );		// BUG?  Maybe 2?
		combo->setTextEntry(LLURLSimString::sInstance.mSimString);
	}
	else
	{
		BOOL login_last = gSavedSettings.getBOOL("LoginLastLocation");
		combo->setCurrentByIndex( login_last ? 1 : 0 );
	}
	
	BOOL show_start = TRUE;
	
	if ( ! force_visible )
		show_start = gSavedSettings.getBOOL("ShowStartLocation");
	
	sInstance->childSetVisible("start_location_combo", show_start);
	sInstance->childSetVisible("start_location_text", show_start);
	sInstance->childSetVisible("server_combo", TRUE);
}

void LoginFloater::newShow(const std::string &grid, bool initialLogin)
{
	
	llwarns << "newShow called" << llendl;
	if(NULL==sInstance) 
	{
		LoginFloater::sGrid = grid;
		LoginFloater::sIsInitialLogin = initialLogin;
		sInstance = new LoginFloater();
		
		llwarns << "sInstance assigned. sInstance=" << sInstance << llendl;
	}

	llwarns << "newshow called" << llendl;
	sInstance->mCurGrid = gHippoGridManager->getCurrentGridNick();
	refresh_grids();

		sInstance->open();	/*Flawfinder: ignore*/
	// we're important
	//sInstance->setFrontmost(TRUE);
	//sInstance->setFocus(TRUE);

}

void LoginFloater::show(const LLRect &rect, BOOL show_server, 
						void (*callback)(S32 option, void* user_data), 
						void* callback_data)
{
	// we don't need a grid passed in because this is old-style login
	std::string grid = "";
	newShow(grid, TRUE);
}

void LoginFloater::setFocus(BOOL b)
{
	if(b != hasFocus())
	{
		if(b)
		{
			LoginFloater::giveFocus();
		}
		else
		{
			LLPanel::setFocus(b);
		}
	}
}

void LoginFloater::giveFocus()
{
	LLScrollListCtrl *combo = NULL;
	
	if(NULL==sInstance)
	{
		llwarns << "giveFocus has no LoginFloater instance. sInstance=" << sInstance << llendl;
		return;
	}
	
	// for our combo box approach, selecting the combo box is almost always
	// the right thing to do on the floater receiving focus
	combo = sInstance->getChild<LLScrollListCtrl>("grid_selector");
	combo->setFocus(TRUE);
}

BOOL LoginFloater::isGridComboDirty()
{
	BOOL user_picked = FALSE;
	if (!sInstance)
	{
		llwarns << "Attempted getServer with no login view shown" << llendl;
	}
	else
	{
		LLComboBox* combo = sInstance->getChild<LLComboBox>("server_combo");
		user_picked = combo->isDirty();
	}
	return user_picked;
}

void LoginFloater::getLocation(std::string &location)
{
	if (!sInstance)
	{
		llwarns << "Attempted getLocation with no login view shown" << llendl;
		return;
	}
	
	LLComboBox* combo = sInstance->getChild<LLComboBox>("start_location_combo");
	location = combo->getValue().asString();
}

std::string& LoginFloater::getPassword()
{
	return mMungedPassword;
}

void LoginFloater::setPassword(std::string &password)
{
	mMungedPassword = password;
}

bool LoginFloater::isSamePassword(std::string &password)
{
	return mMungedPassword == password;
}

void LoginFloater::addServer(const std::string& server, S32 domain_name)
{
	if (!sInstance)
	{
		llwarns << "Attempted addServer with no login view shown" << llendl;
		return;
	}
	
	LLComboBox* combo = sInstance->getChild<LLComboBox>("server_combo");
	combo->add(server, LLSD(domain_name) );
	combo->setCurrentByIndex(0);
}

void LoginFloater::cancel_old()
{
	if(NULL==sInstance)
		return;
	
	if(sInstance->sIsInitialLogin)
	{
		// send a callback that indicates we're quitting or closing
		if(sInstance->mCallback)
			sInstance->mCallback(LOGIN_OPTION_QUIT, sInstance->mCallbackData);
		return;
	}
	
	sInstance->close();
}

void LoginFloater::hashPassword(const std::string& password, std::string& hashedPassword)
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

