/*
 *  controllerlogin.h
 *  SecondLife
 *
 *  Created by RMS on 7/16/08.
 *
 */
#ifndef PL_controllerlogin_H
#define PL_controllerlogin_H

#include <set>
#include <string>
#include "llcombobox.h"
#include "lllineeditor.h"
#include "llcheckboxctrl.h"
#include "llbutton.h"
#include "floaterlogin.h"
#include "authentication_model.h"

class LoginController
{
public:
	LoginController(LoginFloater *floater, AuthenticationModel *authModel, const std::string &grid);
	virtual ~LoginController();
	virtual void update();
protected:
	LoginFloater *mFloater;
	AuthenticationModel *mModel;
	std::string mGrid;
private:
	AuthenticationModel::connection_t mModelConnection;
	
	static void mungePassword(std::string &password);
	
	// UI subview pointers
	LLComboBox *name_combo;
	LLLineEditor *password_edit;
	LLComboBox *start_location_combo;
	LLCheckBoxCtrl *remember_check;
	LLButton *connect_btn;
	LLButton *quit_btn;
	LLComboBox *server_combo;
	
	// state
	std::set<std::string> mAccountNames;
	
	// user interface callbacks
	// TODO: find an alternative to linden callbacks
	static void onCommitName(LLUICtrl *control, void *userdata);
	static void onCommitPassword(LLUICtrl *control, void *userdata);
	static void onAccept(void* userdata);
	static void onCancel(void* userdata);
};

#endif // PL_controllerlogin_H
