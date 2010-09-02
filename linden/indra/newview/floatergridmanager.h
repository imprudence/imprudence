/*
 *  floatergridmanager.h
 *  This is Meerkats grid manager.
 *  -Patrick Sapinski (Monday, August 17, 2009)
 *
 *  Modified by McCabe Maxsted for Imprudence
 */

#ifndef PL_floaterlogin_H
#define PL_floaterlogin_H

#define LOGIN_OPTION_CONNECT 0
#define LOGIN_OPTION_QUIT 1

#include "llfloater.h"

class LoginController;
class AuthenticationModel;

class FloaterGridManager : public LLFloater, public LLFloaterSingleton<FloaterGridManager>
{
public:
	FloaterGridManager(const LLSD& key);
	virtual ~FloaterGridManager();
	
	/*virtual*/ BOOL postBuild();

	static void refreshGrids();
	void apply();
	//void setDefault();
	void cancel();

	void clearInfo();

	virtual void draw();

	void refresh();

	// new-style login methods
	virtual std::string& getPassword();
	virtual void setPassword(std::string &password);
	virtual bool isSamePassword(std::string &password);
	static void getFields(std::string &loginname, std::string &password,
						  BOOL &remember);
	static void setFields(const std::string &loginname, const std::string &password,
						  BOOL remember);
	
	// LLLoginPanel compatibility
	/*static void setAlwaysRefresh(bool refresh);
	static void refreshLocation(bool force_visible);
	virtual void setFocus(BOOL b);
	static void giveFocus();*/
	static void getLocation(std::string &location);
	static BOOL isGridComboDirty();
	//static void addServer(const std::string& server, S32 domain_name);
	static void hashPassword(const std::string& password, std::string& hashedPassword);
protected:
	static bool sIsInitialLogin;
	static std::string sGrid;
private:
	enum State 
	{ 
		NORMAL, 
		ADD_NEW, 
		ADD_COPY 
	};

	State mState;
	void setState(const State& state) { mState = state; }
	State getState() { return mState; }

	std::string mCurGrid;
	void setCurGrid(const std::string& grid) { mCurGrid = grid; }
	std::string getCurGrid() { return mCurGrid; }

	std::string mIncomingPassword;
	std::string mMungedPassword;
	
	void applyChanges();
	bool createNewGrid();
	void update();
	void retrieveGridInfo();

	static void onSelectGrid(LLUICtrl* ctrl, void* data);
	static void onClickDelete(void* data);
	static void onClickAdd(void* data);
	static void onClickCopy(void* data);
	static void onClickOk(void* data);
	static void onClickApply(void* data);
	static void onClickDefault(void* data);
	static void onClickGridInfo(void* data);
	static void onClickCancel(void* data);
	static void onClickClear(void* data);

	static LoginController* sController;
	static AuthenticationModel* sModel;
};

#endif // PL_floaterlogin_H
