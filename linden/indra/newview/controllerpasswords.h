/*
 *  controllerpasswords.h
 *  SecondLife
 *
 *  Created by RMS on 8/5/08.
 *
 */

#include <string>
#include <list>
#include "llscrolllistctrl.h"
#include "llbutton.h"
#include "authentication_model.h"

#ifndef PL_controllerpasswords_H
#define PL_controllerpasswords_H
class PasswordsPrefPanel;

class PasswordsController
{
public:
	PasswordsController(PasswordsPrefPanel *panel);
	virtual ~PasswordsController();
	virtual void update();
protected:
	LLScrollListCtrl *accounts_list;
	LLButton *remove_btn;
private:
	AuthenticationModel::connection_t mModelConnection;
	
	PasswordsPrefPanel *mPanel;
	AuthenticationModel *mModel;
	
	std::list<std::string> mAccountData;
};

#endif // PL_controllerpasswords_H
