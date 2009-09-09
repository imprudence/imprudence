/*
 *  authentication_model.h
 *  SecondLife
 *
 *  Created by RMS on 7/17/08.
 *
 */

#ifndef PL_authentication_model_H
#define PL_authentication_model_H

#include <string>
#include <set>
#include <list>
#include <boost/signal.hpp>
#include <boost/bind.hpp>
#include "lluuid.h"
#include "llmemory.h"
#include "llsd.h"

class AuthenticationModel : public LLSingleton<AuthenticationModel>
{
public:
	typedef boost::signal<void ()> event_t;
	typedef boost::signals::connection connection_t;
	
	AuthenticationModel();
	virtual ~AuthenticationModel();
	
	void loadPersistentData();
	void savePersistentData();
	void revert();
	
	/* generic update, pull model: */
	connection_t subscribeToModelUpdates(event_t::slot_function_type subscriber);
	void unsubscribe(connection_t subscriber);
	
	/* setters */
	void addAccount(const std::string &grid, const std::string &accountName, const std::string &accountPassword);
	void removeAccount(const std::string &grid, const std::string &accountName);
	void changePassword(const std::string &grid, const std::string &accountName, const std::string &newPassword);
	
	/* getters */
	void getAllAccountNames(std::list<std::string> &names);
	void getAccountNames(const std::string &grid, std::set<std::string> &names);
	void getPassword(const std::string &grid, const std::string &accountName, std::string &password);
	void requestUpdate();
protected:
	LLSD mAuthLLSD;
private:
	event_t mEventUpdate;
};
#endif // PL_authentication_model_H
