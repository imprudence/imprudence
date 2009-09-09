/*
 *  AuthenticationController.h
 *  SecondLife
 *
 *  Created by RMS on 7/1/08.
 *
 */

#ifndef PL_AuthenticationController_H
#define PL_AuthenticationController_H

#include "llfloater.h"

class AuthenticationController
{
public:
	AuthenticationController(const std::string& tg, void (*cb)(void*));
	virtual ~AuthenticationController();
	
	// line editor callbacks
	static void onCommitUser(LLUICtrl* ctrl, void* userdata);
	static void onCommitPassword(LLUICtrl* ctrl, void* userdata);
	static void onCommitRemember(LLUICtrl* ctrl, void* userdata);
	// button callbacks
	static void onAccept(void* userdata);
	static void onCancel(void* userdata);
	static void onClickRegister(void* userdata);
	
	void retrieveStoredAccountData(void* userdata);
	static std::string getTargetGrid();
	
private:
	static std::string target_grid;
	static std::string username;
	static std::string password;
	static BOOL store_pw;
	void (*callback)(void*);
};


#endif // PL_AuthenticationController_H

