/*
 *  AuthenticationFloater.cpp
 *  Cross-grid authentication system view.
 *
 *  Created by RMS on 7/1/08.
 *
 */


#include "llviewerprecompiledheaders.h"
#include "authentication_floater.h"
#include "lluictrlfactory.h"

// Statics
AuthenticationFloater* AuthenticationFloater::sInstance = NULL;
AuthenticationController* AuthenticationFloater::sController = NULL;

AuthenticationFloater::AuthenticationFloater()
:   LLFloater("floater_authentication")
{
    LLUICtrlFactory::getInstance()->buildFloater(this, "floater_authentication.xml");
	
	childSetTextArg("Intro_text", "[TARGET_GRID]", sController->getTargetGrid());
	
    childSetCommitCallback("User_edit", controller()->onCommitUser, this);
	childSetCommitCallback("Password_edit", controller()->onCommitPassword, this);
	childSetCommitCallback("Remember_check", controller()->onCommitRemember, this);
	
    childSetAction("OK", controller()->onAccept, this);
	childSetAction("Cancel", controller()->onCancel, this);
	childSetAction("Register", controller()->onClickRegister, this);
	
    setDefaultBtn("OK");
}

AuthenticationFloater::~AuthenticationFloater()
{
    sInstance = NULL;
	delete sController;
	sController = NULL;
}

// static
void AuthenticationFloater::show(void* userdata)
{
	std::string target_grid;
	void (*cb)(void*) = NULL;
	
	if (!userdata)
	{
		target_grid = "Authentication Test";
	}
	
    if (!sInstance)
		sInstance = new AuthenticationFloater();
	if (!sController)
		sController = new AuthenticationController(target_grid, cb);
	
    sInstance->open();
}

void AuthenticationFloater::accept()
{
    llinfos << "accept" << llendl;
}

void AuthenticationFloater::cancel()
{
    llinfos << "cancel" << llendl;
}

AuthenticationController* AuthenticationFloater::controller()
{
	return sController;
}
