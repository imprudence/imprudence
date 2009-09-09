/*
 *  controllerpasswords.cpp
 *  SecondLife
 *
 *  Created by RMS on 8/5/08.
 *
 */

#include "authentication_model.h"
#include "prefpanelpasswords.h"
#include "controllerpasswords.h"

PasswordsController::PasswordsController(PasswordsPrefPanel *panel)
:	mPanel(panel)
{
	accounts_list = mPanel->getChild<LLScrollListCtrl>("accounts_list");
	remove_btn = mPanel->getChild<LLButton>("remove_btn");
	mModel = AuthenticationModel::getInstance();
	
	// subscribe to the model
	mModelConnection = mModel->subscribeToModelUpdates(boost::bind(&PasswordsController::update, this));
	// request an initial update
	mModel->requestUpdate();
}

PasswordsController::~PasswordsController()
{
	mModel->unsubscribe(mModelConnection);
	mModel = NULL;
}

void PasswordsController::update()
{
	std::list<std::string> newAccountData;
	mModel->getAllAccountNames(newAccountData);
	
	if(mAccountData == newAccountData)
		return;
	
	accounts_list->deleteAllItems();
}
