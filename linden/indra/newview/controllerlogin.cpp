/*
 *  controllerlogin.cpp
 *  SecondLife
 *
 *  Created by RMS on 7/16/08.
 *
 */
#include "llerror.h"
#include "llmd5.h"
#include <boost/bind.hpp>
#include "controllerlogin.h"

LoginController::LoginController(LoginFloater *floater, AuthenticationModel *authModel, const std::string &grid)
:	mFloater(floater), mModel(authModel), mGrid(grid)
{
	// set up the user interface subview pointers
	name_combo			= mFloater->getChild<LLComboBox>("name_combo");
	password_edit		= mFloater->getChild<LLLineEditor>("password_edit");
	start_location_combo= mFloater->getChild<LLComboBox>("start_location_combo");
	remember_check		= mFloater->getChild<LLCheckBoxCtrl>("remember_check");
	connect_btn			= mFloater->getChild<LLButton>("connect_btn");
	quit_btn			= mFloater->getChild<LLButton>("quit_btn");
	server_combo		= mFloater->getChild<LLComboBox>("server_combo");
	
	// callbacks
	// TODO: account creation and version information callbacks
	name_combo->setCommitCallback(onCommitName);
	name_combo->setCallbackUserData(this);
	password_edit->setCommitCallback(onCommitPassword);
	password_edit->setCallbackUserData(mFloater);
	connect_btn->setClickedCallback(onAccept, this);
	quit_btn->setClickedCallback(onCancel, this);
	
	// subscribe to the model
	mModelConnection = mModel->subscribeToModelUpdates(boost::bind(&LoginController::update, this));
	// request an initial update
	mModel->requestUpdate();
}

LoginController::~LoginController()
{
	mModel->unsubscribe(mModelConnection);
}

void LoginController::update()
{
	// when we want to update, we need to make sure it's relevant to our
	// interests and make the change as smooth as possible for the user
	std::set<std::string> newAccountNames;
	mModel->getAccountNames(mGrid, newAccountNames);
	
	if(mAccountNames == newAccountNames)
		return;
	
	name_combo->removeall();
	
	for(std::set<std::string>::iterator it = newAccountNames.begin();
		it != newAccountNames.end(); ++it)
	{
		name_combo->add(*it);
	}
	
	name_combo->sortByName();
	mAccountNames.swap(newAccountNames);
}

void LoginController::mungePassword(std::string &password)
{
	LLMD5 pass((unsigned char *)password.c_str());
	char munged_password[MD5HEX_STR_SIZE];
	pass.hex_digest(munged_password);
	password = munged_password;
}

// user interface callbacks

void LoginController::onCommitName(LLUICtrl *control, void *userdata)
{
	// look at this shit it fills in the password box if it finds a stored account
	// and auto checks remember password
	LoginController *controller = (LoginController *)userdata;
	LoginFloater *floater = controller->mFloater;
	
	std::string loginname = floater->childGetText("name_combo");
	std::set<std::string>::iterator it = controller->mAccountNames.find(loginname);
	if(it != controller->mAccountNames.end())
	{
		std::string loginpassword;
		
		controller->mModel->getPassword(controller->mGrid, loginname, loginpassword);
		LoginFloater::setFields(loginname, loginpassword, true);
	}
}

void LoginController::onCommitPassword(LLUICtrl *control, void *userdata)
{
	LoginFloater *floater = (LoginFloater *)userdata;
	LLLineEditor *editor = (LLLineEditor *)control;
	std::string password = editor->getText();
	
	// when we have a new password we need to MD5 it and tell the floater
	if(!floater->isSamePassword(password))
	{
		mungePassword(password);
		floater->setPassword(password);
	}
}

void LoginController::onAccept(void* userdata)
{
	// this here does the main work of telling the model we need to write
	// account data
	LoginController *controller = (LoginController *)userdata;
	LoginFloater *floater = controller->mFloater;
	
	if(!floater->childGetValue("remember_check"))
	{
		LoginFloater::accept();
		return;
	}
	
	std::string username = floater->childGetText("name_combo");
	std::string password = floater->getPassword();
	
	if(controller->mAccountNames.find(username) != controller->mAccountNames.end())
	{
		controller->mModel->changePassword(controller->mGrid, username, password);
	}
	
	else
	{
		controller->mModel->addAccount(controller->mGrid, username, password);
	}
	controller->mModel->savePersistentData();
	LoginFloater::accept();
}

void LoginController::onCancel(void* userdata)
{
	// if the user backs out of the dialog we tell it to clean up and such
	LoginFloater::cancel_old();
}
