/*
 *  floaterlogin.h
 *  SecondLife
 *
 *  Created by RMS on 7/15/08.
 *
 */
#ifndef PL_floaterlogin_H
#define PL_floaterlogin_H

#define LOGIN_OPTION_CONNECT 0
#define LOGIN_OPTION_QUIT 1

#include "llfloater.h"

class LoginController;
class AuthenticationModel;

class LoginFloater : public LLFloater
{
public:
	LoginFloater(void (*callback)(S32 option, void *user_data),
				 void *callback_data);
	virtual ~LoginFloater();
	
	virtual BOOL postBuild();

	static void refresh_grids();
	void apply();
	void setDefault();
	void cancel();

	// new-style login methods
	static void newShow(const std::string &grid, bool initialLogin,
					 void (*callback)(S32 option, void *user_data), 
					 void *callback_data);
	static void testShow(void *lies);
	static void testCallback(S32 option, void *user_data);
	virtual std::string& getPassword();
	virtual void setPassword(std::string &password);
	virtual bool isSamePassword(std::string &password);
	static void getFields(std::string &loginname, std::string &password,
						  BOOL &remember);
	static void setFields(const std::string &loginname, const std::string &password,
						  BOOL remember);
	
	// LLLoginPanel compatibility
	//TODO: Make this not suck
	static void show(const LLRect &rect, BOOL show_server, 
					 void (*callback)(S32 option, void *user_data), 
					 void *callback_data);
	static void close();
	static void setAlwaysRefresh(bool refresh);
	static void refreshLocation(bool force_visible);
	virtual void setFocus(BOOL b);
	static void giveFocus();
	static void getFields(std::string& firstname, std::string& lastname,
						  std::string& password, BOOL& remember);
	static void setFields(const std::string& firstname, const std::string &lastname,
						  const std::string& password, BOOL remember);
	static void getLocation(std::string &location);
	static BOOL isGridComboDirty();
	static void addServer(const std::string& server, S32 domain_name);
	static void accept();
	static void cancel_old();
	static void hashPassword(const std::string& password, std::string& hashedPassword);
protected:
	static bool sIsInitialLogin;
	static std::string sGrid;
private:
	enum State { NORMAL, ADD_NEW, ADD_COPY };
	State mState;
	std::string mCurGrid;

	std::string mIncomingPassword;
	std::string mMungedPassword;
	
	void applyChanges();
	bool createNewGrid();
	void update();

	static void onSelectGrid(LLUICtrl *ctrl, void *data);
	static void onClickDelete(void *data);
	static void onClickAdd(void *data);
	static void onClickCopy(void *data);
	static void onClickApply(void *data);
	static void onClickDefault(void *data);
	static void onClickCancel(void *data);

	static LoginFloater *sInstance;
	static LoginController *sController;
	static AuthenticationModel *sModel;
	
	void (*mCallback)(S32 option, void *userdata);
	void *mCallbackData;
};

#endif // PL_floaterlogin_H
