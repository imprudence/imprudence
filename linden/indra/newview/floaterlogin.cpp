/*
 *  floaterlogin.cpp
 *  SecondLife
 *
 *  Created by RMS on 7/15/08.
 *
 */

#include "llviewerprecompiledheaders.h"

#include <boost/algorithm/string.hpp>
#include "llviewercontrol.h"
#include "llviewerbuild.h"
#include "llcombobox.h"
#include "llscrolllistctrl.h"
#include "llmd5.h"
#include "llurlsimstring.h"
#include "lluictrlfactory.h"
#include "controllerlogin.h"
#include "floaterlogin.h"
#include "hippoGridManager.h"
#include "llviewernetwork.h"
#include "llpanellogin.h"

#define PASSWORD_FILLER "123456789!123456"

LoginFloater* LoginFloater::sInstance = NULL;
LoginController* LoginFloater::sController = NULL;
AuthenticationModel* LoginFloater::sModel = NULL;
bool LoginFloater::sIsInitialLogin;
std::string LoginFloater::sGrid;

LoginFloater::LoginFloater(void (*callback)(S32 option, void* user_data),
						   void *cb_data)
:	LLFloater("floater_login"), mCallback(callback), mCallbackData(cb_data)
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
	LLLineEditor* edit = getChild<LLLineEditor>("password_edit");
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
	
	LLTextBox* version_text = getChild<LLTextBox>("version_text");
	std::string version = llformat("%d.%d.%d (%d)",
								   LL_VERSION_MAJOR,
								   LL_VERSION_MINOR,
								   LL_VERSION_PATCH,
								   LL_VIEWER_BUILD );
	version_text->setText(version);
	
	LLTextBox* channel_text = getChild<LLTextBox>("channel_text");
	channel_text->setText(gSavedSettings.getString("VersionChannelName"));
	
	sendChildToBack(getChildView("channel_text"));
	sendChildToBack(getChildView("version_text"));
	sendChildToBack(getChildView("forgot_password_text"));
	
	setDefaultBtn("connect_btn");
	


}


LoginFloater::~LoginFloater()
{
	delete LoginFloater::sController;
	
	LoginFloater::sModel = NULL;
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
	requires<LLComboBox>("grid_selector");
	requires<LLLineEditor>("gridnick");
	requires<LLComboBox>("platform");
	requires<LLLineEditor>("gridname");
	requires<LLLineEditor>("loginuri");
	requires<LLLineEditor>("loginpage");
	requires<LLLineEditor>("helperuri");
	requires<LLLineEditor>("website");
	requires<LLLineEditor>("first_name");
	requires<LLLineEditor>("last_name");
	requires<LLLineEditor>("password");
	//requires<LLLineEditor>("search");
	requires<LLButton>("btn_delete");
	requires<LLButton>("btn_add");
	requires<LLButton>("btn_copy");
	requires<LLButton>("set_default");
	requires<LLButton>("btn_gridinfo");
	requires<LLButton>("btn_help_render_compat");
	if (!checkRequirements()) return false;
	LLLineEditor* password_edit = getChild<LLLineEditor>("password");
	if (password_edit) password_edit->setDrawAsterixes(TRUE);

	childSetAction("btn_delete", onClickDelete, this);
	childSetAction("btn_add", onClickAdd, this);
	childSetAction("btn_copy", onClickCopy, this);
	childSetAction("btn_apply", onClickApply, this);
	childSetAction("set_default", onClickDefault, this);
	childSetAction("btn_cancel", onClickCancel, this);
//KOW	childSetAction("set_default", onClickDefault, this);
//KOW	childSetAction("btn_gridinfo", onClickGridInfo, this);
//KOW	childSetAction("btn_help_render_compat", onClickHelpRenderCompat, this);

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
			
	sInstance->childSetTextArg("default_grid", "[DEFAULT]", (defaultGrid != "")? defaultGrid: " ");

	sInstance->childSetEnabled("btn_delete", (selectIndex >= 0));
	sInstance->childSetEnabled("btn_copy", (sInstance->mState == NORMAL) && (selectIndex >= 0));
	sInstance->childSetEnabled("set_default", (sInstance->mState == NORMAL) && (selectIndex > 0));
	sInstance->childSetEnabled("gridnick", (sInstance->mState == ADD_NEW) || (sInstance->mState == ADD_COPY));


	LLComboBox *platform = sInstance->getChild<LLComboBox>("platform");
	platform->removeall();
	for (int p=HippoGridInfo::PLATFORM_OTHER; p<HippoGridInfo::PLATFORM_LAST; p++)
		platform->add(HippoGridInfo::getPlatformString(static_cast<HippoGridInfo::Platform>(p)));


	if (sInstance->mState == NORMAL) {
		HippoGridInfo *gridInfo = gHippoGridManager->getGrid(sInstance->mCurGrid);
		if (gridInfo) {
			sInstance->childSetText("gridnick", gridInfo->getGridNick());
			platform->setCurrentByIndex(gridInfo->getPlatform());
			//sInstance->childSetText("grid_name", gridInfo->getGridName());
			sInstance->childSetText("loginuri", gridInfo->getLoginUri());
			sInstance->childSetText("loginpage", gridInfo->getLoginPage());
			sInstance->childSetText("helperuri", gridInfo->getHelperUri());
			sInstance->childSetText("website", gridInfo->getWebSite());
			sInstance->childSetText("first_name", gridInfo->getFirstName());
			sInstance->childSetText("last_name", gridInfo->getLastName());
			if(gridInfo->getAvatarPassword().length() == 32)
				sInstance->childSetText("password", std::string(PASSWORD_FILLER));
			else if(gridInfo->getPasswordUrl().empty())
				sInstance->childSetText("password", std::string(""));
/*
            if (gridInfo->getPlatform() == HippoGridInfo::PLATFORM_SECONDLIFE) {
			    //childSetEnabled("search", false);
                //childSetText("search", LLStringExplicit(""));
			    childSetEnabled("render_compat", false);
    		    childSetValue("render_compat", false);
            } else {
			    //childSetEnabled("search", true);
                //childSetText("search", gridInfo->getSearchUrl());
			    childSetEnabled("render_compat", true);
    		    childSetValue("render_compat", gridInfo->isRenderCompat());
            }
		*/
		}	else {
			std::string empty = "";
			sInstance->childSetText("gridnick", empty);
			platform->setCurrentByIndex(HippoGridInfo::PLATFORM_OTHER);
			sInstance->childSetText("gridname", empty);
			sInstance->childSetText("loginuri", empty);
			sInstance->childSetText("loginpage", empty);
			sInstance->childSetText("helperuri", empty);
			sInstance->childSetText("website", empty);
			sInstance->childSetText("first_name", empty);
			sInstance->childSetText("last_name", empty);
			sInstance->childSetText("password", empty);
			sInstance->childSetEnabled("render_compat", true);
			sInstance->childSetValue("render_compat", true);
		}
	} else if (sInstance->mState == ADD_NEW) {
			llwarns << "ADD_NEW" << llendl;
		std::string required = "<required>";
		std::string empty = "";
		sInstance->childSetText("gridnick", required);
		platform->setCurrentByIndex(HippoGridInfo::PLATFORM_OTHER);
		sInstance->childSetText("gridname", empty);
		sInstance->childSetText("loginuri", required);
		sInstance->childSetText("loginpage", empty);
		sInstance->childSetText("helperuri", empty);
		sInstance->childSetText("website", empty);
		sInstance->childSetText("first_name", empty);
		sInstance->childSetText("last_name", empty);
		sInstance->childSetText("password", empty);
		//childSetEnabled("search", true);
		//childSetText("search", empty);
		sInstance->childSetEnabled("render_compat", true);
		sInstance->childSetValue("render_compat", true);
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
			gridInfo->setPlatform(childGetValue("platform"));
			gridInfo->setGridName(childGetValue("gridname"));
			gridInfo->setLoginUri(childGetValue("loginuri"));
			gridInfo->setLoginPage(childGetValue("loginpage"));
			gridInfo->setHelperUri(childGetValue("helperuri"));
			gridInfo->setWebSite(childGetValue("website"));
			gridInfo->setFirstName(childGetValue("first_name"));
			gridInfo->setLastName(childGetValue("last_name"));
			//gridInfo->setSearchUrl(childGetValue("search"));
			gridInfo->setRenderCompat(childGetValue("render_compat"));
			
			if(childGetValue("password").asString().empty())
				gridInfo->setPasswordUrl(std::string(""));
			else if(childGetValue("password").asString() != std::string(PASSWORD_FILLER))
			{
				// store account authentication data
				std::string auth_password = childGetValue("password");
				std::string hashed_password;
				hashPassword(auth_password, hashed_password);
				gridInfo->setAvatarPassword(hashed_password);
			}
			LLPanelLogin::setFields(gridInfo->getFirstName(), gridInfo->getLastName(),
									gridInfo->getAvatarPassword(), true);
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
	HippoGridInfo::cleanUpGridNick(gridnick);
	childSetValue("gridnick", (gridnick != "")? gridnick: "<required>");
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
	grid->setPlatform(childGetValue("platform"));
	grid->setGridName(childGetValue("gridname"));
	grid->setLoginUri(loginuri);
	grid->setLoginPage(childGetValue("loginpage"));
	grid->setHelperUri(childGetValue("helperuri"));
	grid->setWebSite(childGetValue("website"));
	grid->setFirstName(childGetValue("first_name"));
	grid->setLastName(childGetValue("last_name"));
	//grid->setSearchUrl(childGetValue("search"));
	grid->setRenderCompat(childGetValue("render_compat"));
	gHippoGridManager->addGrid(grid);
	
	if(childGetValue("password").asString().empty())
		grid->setAvatarPassword(std::string(""));
	else
	{
		std::string hashed_password;
		hashPassword(childGetValue("password"), hashed_password);
		grid->setAvatarPassword(hashed_password);
	}
	
	mCurGrid = gridnick;
	return true;
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
			llwarns << "I think me grid is " << mCurGrid << llendl;
	//LLPanelLogin::refreshLoginPage();
	gHippoGridManager->saveFile();
	LLPanelLogin::addServer(LLViewerLogin::getInstance()->getGridLabel());
}

void LoginFloater::cancel()
{
	gHippoGridManager->discardAndReload();
	LoginFloater::sModel->revert();
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
			//LLCtrlListInterface *grids = self->childGetListInterface("search_results");
			//if (!grids) return;

			//LLSD selected_value = grids->getSelectedValue();
			//std::string sim_name = selected_value.asString();

			LLComboBox *grids = self->getChild<LLComboBox>("grid_selector");
			grids->setCurrentByIndex(grids->getItemCount() - 1);
			return;
		}
	} else {
		llwarns << "Illegal state " << self->mState << '.' << llendl;
		return;
	}
	self->mCurGrid = ctrl->getValue().asString();
			llwarns << "I think me grid is " << self->mCurGrid << llendl;

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

//static
void LoginFloater::onClickApply(void *data)
{
	sInstance->apply();
}

//static
void LoginFloater::onClickDefault(void *data)
{
	sInstance->setDefault();
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

void LoginFloater::newShow(const std::string &grid, bool initialLogin,
						void (*callback)(S32 option, void* user_data), 
						void* callback_data)
{
	
		llwarns << "newShow called" << llendl;
	if(NULL==sInstance) 
	{
		LoginFloater::sGrid = grid;
		LoginFloater::sIsInitialLogin = initialLogin;
		sInstance = new LoginFloater(callback, callback_data);
		
		llwarns << "sInstance assigned. sInstance=" << sInstance << llendl;
	}
	
	// floater controller requires initialized floater and model
	if(NULL==sModel)
		sModel = AuthenticationModel::getInstance();
	if(NULL==sController)
		//sController = new LoginController(sInstance, sModel, sGrid);



		llwarns << "newshow called" << llendl;
		sInstance->mCurGrid = gHippoGridManager->getCurrentGridNick();
		refresh_grids();

		// we're important
		sInstance->setFrontmost(TRUE);
		sInstance->setFocus(TRUE);

}

void LoginFloater::testShow(void *lies)
{
	// this is if we want to call LoginFloater from a menu option
	// or you know whatever
	newShow(std::string("Test"), false, testCallback, NULL);
}

void LoginFloater::testCallback(S32 option, void *user_data)
{
	// test callback, referenced by testShow()
	if(LOGIN_OPTION_CONNECT == option)
	{
		llinfos << "this is how we connect to a METAVERSE" << llendl;
		std::string first, last, password;
		BOOL remember = TRUE;
		getFields(first, last, password, remember);
		llinfos << "first\t\tlast\t\tpassword" << llendl;
		llinfos << first << "\t\t" << last << "\t\t" << password << llendl;
	}
	else if(LOGIN_OPTION_QUIT == option)
	{
		llinfos << "my login, she die" << llendl;
		llinfos << ":(" << llendl;
		close();
	}
}

void LoginFloater::show(const LLRect &rect, BOOL show_server, 
						void (*callback)(S32 option, void* user_data), 
						void* callback_data)
{
	// we don't need a grid passed in because this is old-style login
	std::string grid = "";
	newShow(grid, TRUE, callback, callback_data);
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
	LLComboBox *combo = NULL;
	
	if(NULL==sInstance)
	{
		llwarns << "giveFocus has no LoginFloater instance. sInstance=" << sInstance << llendl;
		return;
	}
	
	// for our combo box approach, selecting the combo box is almost always
	// the right thing to do on the floater receiving focus
	combo = sInstance->getChild<LLComboBox>("name_combo");
	combo->setFocus(TRUE);
}

void LoginFloater::getFields(std::string &firstname, std::string &lastname, std::string &password,
							 BOOL &remember)
{
	if (!sInstance)
	{
		llwarns << "Attempted getFields with no login view shown" << llendl;
		return;
	}
	
	std::string loginname = sInstance->childGetText("name_combo");
	
	LLStringUtil::replaceTabsWithSpaces(loginname, 1);
	LLStringUtil::trim(loginname);
	std::vector<std::string> loginVec;
	boost::split(loginVec, loginname, boost::is_any_of(" "), boost::token_compress_on);
	if(loginVec.size() == 2)
	{
		firstname = loginVec[0];
		lastname = loginVec[1];
	}
	
	password = sInstance->mMungedPassword;
	remember = sInstance->childGetValue("remember_check");
}

void LoginFloater::getFields(std::string &loginname, std::string &password, BOOL &remember)
{
	std::string first, last, pass;
	BOOL rem;
	getFields(first, last, pass, rem);
	loginname = first + " " + last;
	password = pass;
	remember = rem;
}

void LoginFloater::setFields(const std::string& firstname, const std::string& lastname, const std::string& password,
							 BOOL remember)
{
	if (!sInstance)
	{
		llwarns << "Attempted setFields with no login view shown" << llendl;
		return;
	}
	
	std::string loginname = firstname + " " + lastname;
	sInstance->childSetText("name_combo", loginname);
	
	// Max "actual" password length is 16 characters.
	// Hex digests are always 32 characters.
	if (password.length() == 32)
	{
		// This is a MD5 hex digest of a password.
		// We don't actually use the password input field, 
		// fill it with MAX_PASSWORD characters so we get a 
		// nice row of asterixes.
		const std::string filler("123456789!123456");
		sInstance->childSetText("password_edit", filler);
		sInstance->mIncomingPassword = filler;
		sInstance->mMungedPassword = password;
	}
	else
	{
		// this is a normal text password
		sInstance->childSetText("password_edit", password);
		sInstance->mIncomingPassword = password;
		LLMD5 pass((unsigned char *)password.c_str());
		char munged_password[MD5HEX_STR_SIZE];
		pass.hex_digest(munged_password);
		sInstance->mMungedPassword = munged_password;
	}
	
	sInstance->childSetValue("remember_check", remember);
}

void LoginFloater::setFields(const std::string &loginname, const std::string &password, BOOL remember)
{
	std::vector<std::string> loginVec;
	boost::split(loginVec, loginname, boost::is_any_of(" "), boost::token_compress_on);
	setFields(loginVec[0], loginVec[1], password, remember);
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

void LoginFloater::accept()
{
	if(NULL==sInstance || NULL==sInstance->mCallback)
		return;
	
	sInstance->setFocus(FALSE);
	
	std::string name_combo = sInstance->childGetText("name_combo");
	if(!name_combo.empty())
	{
		sInstance->mCallback(LOGIN_OPTION_CONNECT, sInstance->mCallbackData);
	}
	else
	{
		// TODO: new account call goes here
		return;
	}
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

