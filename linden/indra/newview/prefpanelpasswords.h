/*
 *  prefpanelpasswords.h
 *  SecondLife
 *
 *  Created by RMS on 8/5/08.
 *
 */

#include "llpanel.h"

#ifndef PL_prefpanelpasswords_H
#define PL_prefpanelpasswords_H

class PasswordsController;

class PasswordsPrefPanel : public LLPanel
{
public:
	PasswordsPrefPanel();
	virtual ~PasswordsPrefPanel();
	
	BOOL postBuild();
	
	void apply();
	void cancel();
protected:
	PasswordsController *mController;
};

#endif // PL_prefpanelpasswords_H
