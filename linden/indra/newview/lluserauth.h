/** 
 * @file lluserauth.h
 * @brief LLUserAuth class header file
 *
 * $LicenseInfo:firstyear=2003&license=viewergpl$
 * 
 * Copyright (c) 2003-2009, Linden Research, Inc.
 * 
 * Second Life Viewer Source Code
 * The source code in this file ("Source Code") is provided by Linden Lab
 * to you under the terms of the GNU General Public License, version 2.0
 * ("GPL"), unless you have obtained a separate licensing agreement
 * ("Other License"), formally executed by you and Linden Lab.  Terms of
 * the GPL can be found in doc/GPL-license.txt in this distribution, or
 * online at http://secondlifegrid.net/programs/open_source/licensing/gplv2
 * 
 * There are special exceptions to the terms and conditions of the GPL as
 * it is applied to this Source Code. View the full text of the exception
 * in the file doc/FLOSS-exception.txt in this software distribution, or
 * online at
 * http://secondlifegrid.net/programs/open_source/licensing/flossexception
 * 
 * By copying, modifying or distributing this software, you acknowledge
 * that you have read and understood your obligations described above,
 * and agree to abide by those obligations.
 * 
 * ALL LINDEN LAB SOURCE CODE IS PROVIDED "AS IS." LINDEN LAB MAKES NO
 * WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
 * COMPLETENESS OR PERFORMANCE.
 * $/LicenseInfo$
 */

#ifndef LLUSERAUTH_H
#define LLUSERAUTH_H

#include <string>
#include <vector>
#include <map>
typedef struct _xmlrpc_value* XMLRPC_VALUE;
// forward ecl of types from xlrpc.h

class LLXMLRPCTransaction;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Class LLUserAuth
//
// This class encapsulates the authentication and initialization from
// the login server. Construct an instance of this object, and call
// the authenticate() method, and call authResponse() until it returns
// a non-negative value. If that method returns E_OK, you can start
// asking for responses via the getResponse() method. Here is some
// sample code that gets the session id if login was successful:
//
//  auth.authenticate(...);
//  while((auth.authResponse() < 0)) {sleep(1);}
//  LLUUID session_id;
//  if(0 == strcmp(auth.getResponse("login"), "true"))
//  {
//    const char* session_id_str = auth.getResponse("session-id");
//    if(session_id_str) session_id.set(session_id_str);
//  }
//
// Format for responses as maintained in login.cgi:
// login = 'true' | 'false'
// reason = [ 'data'  |    -- insufficient or poorly formatted data
//            'ban' |      -- user is banned
//            'update' |   -- viewer requires update
//            'optional' | -- optional viewer update
//            'key' |      -- mismatched first/last/passwd
// message = human readable message for client
// session-id = auth key
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class LLUserAuth : public LLSingleton<LLUserAuth>
{
public:
	LLUserAuth();
	~LLUserAuth();

	// These codes map to the curl return codes...
	typedef enum {
		E_NO_RESPONSE_YET = -2,
		E_DOWNLOADING = -1,
		E_OK = 0,
		E_COULDNT_RESOLVE_HOST,
		E_SSL_PEER_CERTIFICATE,
		E_SSL_CACERT,
		E_SSL_CONNECT_ERROR,
		E_UNHANDLED_ERROR,
		E_LAST						// never use!
	} UserAuthcode;

	// used for holding options
	typedef std::map<std::string, std::string> response_t;
	typedef std::vector<response_t> options_t;

	// viewer auth version
	void authenticate(
		const std::string& auth_uri,
		const std::string& auth_method,
		const std::string& firstname,
		const std::string& lastname,
		LLUUID web_login_key,
		const std::string& start,
		BOOL skip_optional_update,
		BOOL accept_tos,
		BOOL accept_critical_message,
		BOOL last_exec_froze, 
		const std::vector<const char*>& requested_options,
		const std::string& hashed_mac,
		const std::string& hashed_volume_serial);

	// legacy version
	void authenticate(
		const std::string& auth_uri,
		const std::string& auth_method,
		const std::string& firstname,
		const std::string& lastname,
		const std::string& password,
		const std::string& start,
		BOOL skip_optional_update,
		BOOL accept_tos,
		BOOL accept_critical_message,
		BOOL last_exec_froze, 
		const std::vector<const char*>& requested_options,
		const std::string& hashed_mac,
		const std::string& hashed_volume_serial);

	UserAuthcode authResponse();
	// mResult contains the responses we get through the login process
	//
	LLSD mResult; // OGPX : *way* different access of auth result in OGP.
	UserAuthcode mAuthResponse; // OGPX TODO: proper access functions
	 

	// clears out internal data cache.
	void reset();
	
	std::string errorMessage() const { return mErrorMessage; }

	// function to get a direct reponse from the login api by
	// name. returns NULL if the named response was not found.
	const std::string& getResponse(const std::string& name) const;
	BOOL getOptions(const std::string& name, options_t& options) const;

	F64 getLastTransferRateBPS() const { return mLastTransferRateBPS; }

private:
	LLXMLRPCTransaction* mTransaction;


	std::string mErrorMessage;
	
	// dealing with the XML
	typedef std::map<std::string, options_t> all_options_t;
	response_t mResponses;
	all_options_t mOptions;
	
	UserAuthcode parseResponse();

	F64 mLastTransferRateBPS;	// bits per second, only valid after a big transfer like inventory
};

#endif /* LLUSERAUTH_H */
