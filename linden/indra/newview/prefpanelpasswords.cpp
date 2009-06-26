/*
 *  prefpanelpasswords.cpp
 *  SecondLife
 *
 *  Created by RMS on 8/5/08.
 *
 */

#include "lluictrlfactory.h"

#include "controllerpasswords.h"
#include "prefpanelpasswords.h"

PasswordsPrefPanel::PasswordsPrefPanel()
:	LLPanel(std::string("panel_preferences_passwords"))
{
	LLUICtrlFactory::getInstance()->buildPanel(this, "panel_preferences_passwords.xml");
}

PasswordsPrefPanel::~PasswordsPrefPanel()
{
	delete mController;
	mController = NULL;
}

BOOL PasswordsPrefPanel::postBuild()
{
	mController = new PasswordsController(this);
	return TRUE;
}

void PasswordsPrefPanel::apply()
{
	
}

void PasswordsPrefPanel::cancel()
{
	
}
