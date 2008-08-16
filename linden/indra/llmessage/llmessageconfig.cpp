/** 
 * @file llmessageconfig.cpp
 * @brief Live file handling for messaging
 *
 * Copyright (c) 2000-2007, Linden Research, Inc.
 * 
 * Second Life Viewer Source Code
 * The source code in this file ("Source Code") is provided by Linden Lab
 * to you under the terms of the GNU General Public License, version 2.0
 * ("GPL"), unless you have obtained a separate licensing agreement
 * ("Other License"), formally executed by you and Linden Lab.  Terms of
 * the GPL can be found in doc/GPL-license.txt in this distribution, or
 * online at http://secondlife.com/developers/opensource/gplv2
 * 
 * There are special exceptions to the terms and conditions of the GPL as
 * it is applied to this Source Code. View the full text of the exception
 * in the file doc/FLOSS-exception.txt in this software distribution, or
 * online at http://secondlife.com/developers/opensource/flossexception
 * 
 * By copying, modifying or distributing this software, you acknowledge
 * that you have read and understood your obligations described above,
 * and agree to abide by those obligations.
 * 
 * ALL LINDEN LAB SOURCE CODE IS PROVIDED "AS IS." LINDEN LAB MAKES NO
 * WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
 * COMPLETENESS OR PERFORMANCE.
 */

#include "linden_common.h"

#include "llmessageconfig.h"
#include "llfile.h"
#include "lllivefile.h"
#include "llsd.h"
#include "llsdserialize.h"

static const char messageConfigFileName[] = "message.xml";
static const F32 messageConfigRefreshRate = 5.0; // seconds
static std::string sServerName = "";
static std::string sConfigDir = "";

class LLMessageConfigFile : public LLLiveFile
{
private:
	LLMessageConfigFile()
        : LLLiveFile(fileName(), messageConfigRefreshRate),
		  mChanged(false)
            { }

    static std::string fileName();
    
public:
	static LLMessageConfigFile& instance();
		// return the singleton configuration file

protected:
	/* virtual */ void loadFile();
	void loadServerDefaults(const LLSD& data);
	void loadMessages(const LLSD& data);

public:
    bool mChanged;
	
	std::string mServerDefault;
	LLSD mMessages;
};

std::string LLMessageConfigFile::fileName()
{
    std::ostringstream ostr;
	ostr << sConfigDir//gAppSimp->getOption("configdir").asString()
		<< "/" << messageConfigFileName;
	return ostr.str();
}

LLMessageConfigFile& LLMessageConfigFile::instance()
{
	static LLMessageConfigFile the_file;
	the_file.checkAndReload();
	return the_file;
}

// virtual
void LLMessageConfigFile::loadFile()
{
	LLSD data;
    {
        llifstream file(filename().c_str());
        if (file.is_open())
        {
			llinfos << "Loading message.xml file at " << fileName() << llendl;
            LLSDSerialize::fromXML(data, file);
        }

        if (data.isUndefined())
        {
            llinfos << "LLMessageConfigFile::loadFile: file missing,"
				" ill-formed, or simply undefined; not changing the"
				" file" << llendl;
            return;
        }
    }
	loadServerDefaults(data);
	loadMessages(data);
}

void LLMessageConfigFile::loadServerDefaults(const LLSD& data)
{
	mServerDefault = data["serverDefaults"][sServerName].asString();
	lldebugs << "loading default " << mServerDefault << llendl;
}

void LLMessageConfigFile::loadMessages(const LLSD& data)
{
	LLPointer<LLSDXMLFormatter> formatter = new LLSDXMLFormatter;
	std::ostringstream out;

	mMessages = data["messages"];
	formatter->format(mMessages, out);
	lldebugs << "loading ... " << out.str()
			<< " LLMessageConfigFile::loadMessages loaded "
			<< mMessages.size() << " messages" << llendl;
}


//---------------------------------------------------------------
// LLMessageConfig
//---------------------------------------------------------------

//static
void LLMessageConfig::initClass(const std::string& server_name,
								const std::string& config_dir)
{
	sServerName = server_name;
	sConfigDir = config_dir;
	(void) LLMessageConfigFile::instance();
	llinfos << "LLMessageConfig::intiClass config file "
			<< config_dir << "/" << messageConfigFileName << llendl;
}

//static
bool LLMessageConfig::isServerDefaultBuilderLLSD()
{
	if (sServerName.empty())
	{
		llerrs << "LLMessageConfig::isServerDefaultBuilderLLSD() before"
				<< " LLMessageConfig::initClass()" << llendl;
	}
	LLMessageConfigFile& file = LLMessageConfigFile::instance();
	return (file.mServerDefault == "llsd");
}

//static
bool LLMessageConfig::isServerDefaultBuilderTemplate()
{
	if (sServerName.empty())
	{
		llerrs << "LLMessageConfig::isServerDefaultBuilderTemplate() before"
				<< " LLMessageConfig::initClass()" << llendl;
	}
	LLMessageConfigFile& file = LLMessageConfigFile::instance();
	return (file.mServerDefault == "template");
}

//static
bool LLMessageConfig::isMessageBuiltLLSD(const std::string& msg_name)
{
	if (sServerName.empty())
	{
		llerrs << "LLMessageConfig::isMessageBuiltLLSD(name) before"
				<< " LLMessageConfig::initClass()" << llendl;
	}
	LLMessageConfigFile& file = LLMessageConfigFile::instance();
	LLSD config = file.mMessages[msg_name];
	if (!config.has("builder"))
	{
		return isServerDefaultBuilderLLSD();
	}
	return (config["builder"].asString() == "llsd");
}

//static
bool LLMessageConfig::isMessageBuiltTemplate(const std::string& msg_name)
{
	if (sServerName.empty())
	{
		llerrs << "LLMessageConfig::isMessageBuiltTemplate(name) before"
				<< " LLMessageConfig::initClass()" << llendl;
	}
	LLMessageConfigFile& file = LLMessageConfigFile::instance();
	LLSD config = file.mMessages[msg_name];
	if (!config.has("builder"))
	{
		return isServerDefaultBuilderTemplate();
	}
	return (config["builder"].asString() == "template");
}

//static
bool LLMessageConfig::isMessageTrusted(const std::string& msg_name)
{
	if (sServerName.empty())
	{
		llerrs << "LLMessageConfig::isMessageTrusted(name) before"
				<< " LLMessageConfig::initClass()" << llendl;
	}
	LLMessageConfigFile& file = LLMessageConfigFile::instance();
	LLSD config = file.mMessages[msg_name];
	if (!config.has("trusted-sender"))
	{
		return false;
	}
	return config["trusted-sender"].asBoolean();
}

//static
bool LLMessageConfig::isValidUntrustedMessage(const std::string& msg_name)
{
	if (sServerName.empty())
	{
		llerrs << "LLMessageConfig::isMessageTrusted(name) before"
				<< " LLMessageConfig::initClass()" << llendl;
	}
	LLMessageConfigFile& file = LLMessageConfigFile::instance();
	LLSD config = file.mMessages[msg_name];
	if (!config.has("trusted-sender"))
	{
		return false;
	}
	return !(config["trusted-sender"].asBoolean());
}
