/*
 *  AuthenticationController.cpp
 *  SecondLife
 *
 *  Created by RMS on 7/1/08.
 *
 */

#include "llviewerprecompiledheaders.h"
#include "authentication_floater.h"
#include "llviewerobject.h"
#include "llcheckboxctrl.h"
#include "llselectmgr.h"
#include "authentication_controller.h"

// Statics
std::string AuthenticationController::target_grid;
std::string AuthenticationController::username;
std::string AuthenticationController::password;
BOOL AuthenticationController::store_pw = FALSE;

AuthenticationController::AuthenticationController(const std::string& tg, void (*cb)(void*))
{
	target_grid = tg;
	callback = cb;
}

AuthenticationController::~AuthenticationController()
{
}

// user interface callbacks: all static
void AuthenticationController::onCommitUser(LLUICtrl* ctrl, void* userdata)
{
    AuthenticationFloater *floater = (AuthenticationFloater*)userdata;
	username = floater->childGetText("User_edit");
}

void AuthenticationController::onCommitPassword(LLUICtrl* ctrl, void* userdata)
{
    AuthenticationFloater *floater = (AuthenticationFloater*)userdata;
    password = floater->childGetText("Password_edit");
}

void AuthenticationController::onCommitRemember(LLUICtrl* ctrl, void* userdata)
{
	LLViewerObject *object = LLSelectMgr::getInstance()->getSelection()->getFirstRootObject();
	if(!object) return;
	
	LLCheckBoxCtrl *check = (LLCheckBoxCtrl*)ctrl;
	store_pw = check->get();
}

void AuthenticationController::onAccept(void* userdata)
{
	
}

void AuthenticationController::onCancel(void* userdata)
{
	AuthenticationFloater *floater = (AuthenticationFloater*)userdata;
	floater->cancel();
	floater->close();
}

void AuthenticationController::onClickRegister(void* userdata)
{
	llinfos << "onClickRegister" << llendl;
}

void AuthenticationController::retrieveStoredAccountData(void* userdata)
{
	
}

// static
std::string AuthenticationController::getTargetGrid()
{
	return target_grid;
}
