
#include "hippoUpdate.h"

#include <cstdio>
#include <list>
#include <vector>

#include <stdtypes.h>
#include <llhttpclient.h>
#include <llmemory.h>
//#include <llversionviewer.h>
#include "llviewercontrol.h"
#include "llviewernetwork.h"
#include "llweb.h"
#include <llwindow.h>


std::string gHippoChannel;


// static
bool HippoUpdate::checkUpdate()
{
	llinfos << "Hippo Update Check..." << llendl;

	// get channel name
	gHippoChannel = gSavedSettings.getString("ChannelName");

	// get mac address
	char macAddress[18];
	sprintf(macAddress, "%02x:%02x:%02x:%02x:%02x:%02x",
		gMACAddress[0], gMACAddress[1], gMACAddress[2], gMACAddress[3], gMACAddress[4], gMACAddress[5]);

	// build URL for update check
	char url[1000];
	snprintf(url, 1000, 
/*		"http://update.mjm.game-host.org/os/viewer.php?"
		"product=%s&channel=%s&"
		"version_major=%d&version_minor=%d&version_patch=%d&version_base=%s&"
		"platform=%s&mac=%s",
		LL_PRODUCT, LL_CHANNEL_CSTR,
		LL_VERSION_MAJOR, LL_VERSION_MINOR, LL_VERSION_PATCH, LL_VERSION_BASE,
		LL_PLATFORM*/"", macAddress);

	// query update server
	std::string escaped_url = LLWeb::escapeURL(url);
	LLSD response = LLHTTPClient::blockingGet(escaped_url.c_str());

	// check response, return on error
	S32 status = response["status"].asInteger();
	if ((status != 200) || !response["body"].isMap()) {
		llinfos << "Hippo Update failed (" << status << "): "
			<< (response["body"].isString()? response["body"].asString(): "<unknown error>")
			<< llendl;
		return true;
	}

	// get data from response
	LLSD data = response["body"];
	std::string webpage = (data.has("webpage") && data["webpage"].isString())? data["webpage"].asString(): "";
	std::string message = (data.has("message") && data["message"].isString())? data["message"].asString(): "";
	std::string yourVersion = (data.has("yourVersion") && data["yourVersion"].isString())? data["yourVersion"].asString(): "";
	std::string curVersion = (data.has("curVersion") && data["curVersion"].isString())? data["curVersion"].asString(): "";
	bool update = (data.has("update") && data["update"].isBoolean())? data["update"].asBoolean(): false;
	bool mandatory = (data.has("mandatory") && data["mandatory"].isBoolean())? data["mandatory"].asBoolean(): false;

	// log and return, if no update available
	llinfos << "Your version is " << yourVersion << ", current version is " << curVersion << '.' << llendl;
	if (!update) return true;
	llinfos << "Update is " << (mandatory? "mandatory.": "optional.") << llendl;

	// show update dialog
	char msg[1000];
	snprintf(msg, 1000,
		"There is a new viewer version available.\n"
		"\n"
		"Your version: %s\n"
		"Current version: %s\n"
		"%s\n"
		"Do you want to visit the web site?",
		yourVersion.c_str(), curVersion.c_str(),
		mandatory? "\nThis is a mandatory update.\n": "");
	S32 button = OSMessageBox(msg, "Hippo OpenSim Viewer Update", OSMB_YESNO);
	if (button == OSBTN_YES) {
		llinfos << "Taking user to " << webpage << llendl;
		LLWeb::loadURLExternal(webpage);
		// exit the viewer
		return false;
	}

	return !mandatory;
}
