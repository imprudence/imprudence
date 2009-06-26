/*
 *  authentication_model.cpp
 *  SecondLife
 *
 *  Created by RMS on 7/17/08.
 *
 */

#include "llviewerprecompiledheaders.h"

#include "lldir.h"
#include "llfile.h"
#include "llsdserialize.h"
#include "authentication_model.h"

AuthenticationModel::AuthenticationModel()
{
	loadPersistentData();
}

AuthenticationModel::~AuthenticationModel()
{
	savePersistentData();
}

void AuthenticationModel::loadPersistentData()
{
	std::string filename = gDirUtilp->getExpandedFilename(LL_PATH_USER_SETTINGS, 
														  "cross_grid_authentication.xml");
	LLSD auth_llsd;
	llifstream file;
	file.open(filename);
	if(file.is_open())
		LLSDSerialize::fromXML(mAuthLLSD, file);
}

void AuthenticationModel::savePersistentData()
{
	std::string filename = gDirUtilp->getExpandedFilename(LL_PATH_USER_SETTINGS, 
														  "cross_grid_authentication.xml");
	llofstream ofile;
	ofile.open(filename);
	LLSDSerialize::toPrettyXML(mAuthLLSD, ofile);
}

void AuthenticationModel::revert()
{
	loadPersistentData();
}

AuthenticationModel::connection_t AuthenticationModel::subscribeToModelUpdates
										(event_t::slot_function_type subscriber)
{
	return mEventUpdate.connect(subscriber);
}

void AuthenticationModel::unsubscribe(connection_t subscriber)
{
	subscriber.disconnect();
}

/* setters */
void AuthenticationModel::addAccount(const std::string &grid, const std::string &accountName, 
									 const std::string &accountPassword)
{
	mAuthLLSD[grid][accountName] = LLSD::LLSD(accountPassword);
	mEventUpdate();
}

void AuthenticationModel::removeAccount(const std::string &grid, const std::string &accountName)
{
	mAuthLLSD[grid].erase(accountName);
	mEventUpdate();
}

void AuthenticationModel::changePassword(const std::string &grid, const std::string &accountName, 
										 const std::string &newPassword)
{
	mAuthLLSD[grid][accountName] = newPassword;
	// no event necessary: passwords aren't displayed in any view
}

/* getters */

void AuthenticationModel::getAllAccountNames(std::list<std::string> &names)
{
	// TODO: implement this for account management
}

void AuthenticationModel::getAccountNames(const std::string &grid, std::set<std::string> &names)
{
	if(!mAuthLLSD.has(grid))
		return;
	
	for(LLSD::map_const_iterator it = mAuthLLSD[grid].beginMap();
		it != mAuthLLSD[grid].endMap(); ++it)
	{
		names.insert(it->first);
	}
}

void AuthenticationModel::getPassword(const std::string &grid, const std::string &accountName, 
											 std::string &password)
{
	password = mAuthLLSD[grid][accountName].asString();
}

void AuthenticationModel::requestUpdate()
{
	mEventUpdate();
}
