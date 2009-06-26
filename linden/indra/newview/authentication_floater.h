/*
 *  AuthenticationFloater.h
 *  Cross-grid authentication system view.
 *
 *  Created by RMS on 7/1/08.
 *
 */

#ifndef PL_AuthenticationFloater_H
#define PL_AuthenticationFloater_H

#include "llfloater.h"
#include "authentication_controller.h"

class AuthenticationFloater : public LLFloater
{
public:
	AuthenticationFloater();
	virtual ~AuthenticationFloater();
	
	static void show(void* userdata);
	static void accept();
	static void cancel();
	
	// data accessors
	static AuthenticationController* controller();
		
private:
	// static because we only need one floater
	static AuthenticationFloater* sInstance;
	static AuthenticationController* sController;
};


#endif // PL_AuthenticationFloater_H
