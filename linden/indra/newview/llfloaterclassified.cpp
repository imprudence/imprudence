/** 
* @file llfloaterclassified.cpp
* @brief LLFloaterClassifiedInfo class implementation
*
* Classified information as shown in a floating window from secondlife:// command
* handler.  
*
* $LicenseInfo:firstyear=2002&license=internal$
* 
* Copyright (c) 2002-2008, Linden Research, Inc.
* 
* The following source code is PROPRIETARY AND CONFIDENTIAL. Use of
* this source code is governed by the Linden Lab Source Code Disclosure
* Agreement ("Agreement") previously entered between you and Linden
* Lab. By accessing, using, copying, modifying or distributing this
* software, you acknowledge that you have been informed of your
* obligations under the Agreement and agree to abide by those obligations.
* 
* ALL LINDEN LAB SOURCE CODE IS PROVIDED "AS IS." LINDEN LAB MAKES NO
* WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
* COMPLETENESS OR PERFORMANCE.
* $/LicenseInfo$
*/

#include "llviewerprecompiledheaders.h"

#include "llfloaterclassified.h"

// viewer project includes
#include "llcommandhandler.h"
#include "llpanelclassified.h"
#include "llvieweruictrlfactory.h"

// linden library includes
#include "lluuid.h"

//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------

LLMap< const LLUUID, LLFloaterClassifiedInfo* > gClassifiedInfoInstances;
////////////////////////////////////////////////////////////////////////////
// LLFloaterEventDisplay

class LLClassifiedHandler : public LLCommandHandler
{
public:
	LLClassifiedHandler() : LLCommandHandler("classified") { }
	bool handle(const LLSD& tokens, const LLSD& queryMap)
	{
		if (tokens.size() < 2)
		{
			return false;
		}
		LLUUID classified_id;
		if (!classified_id.set(tokens[0], FALSE))
		{
			return false;
		}

		if (tokens[1].asString() == "about")
		{
			LLFloaterClassifiedInfo::show(classified_id);
			return true;
		}
		return false;
	}
};
LLClassifiedHandler gClassifiedHandler;

LLFloaterClassifiedInfo::LLFloaterClassifiedInfo(const std::string& name, const LLUUID &id)
:	LLFloater(name),
mClassifiedID( id )
{
	mFactoryMap["classified_details_panel"] = LLCallbackMap(LLFloaterClassifiedInfo::createClassifiedDetail, this);
	gUICtrlFactory->buildFloater(this, "floater_preview_classified.xml", &getFactoryMap());
	gClassifiedInfoInstances.addData(id, this);
}

LLFloaterClassifiedInfo::~LLFloaterClassifiedInfo()
{
	// child views automatically deleted
	gClassifiedInfoInstances.removeData(mClassifiedID);

}

void LLFloaterClassifiedInfo::displayClassifiedInfo(const LLUUID& classified_id)
{
	mClassifiedPanel->setClassifiedID(classified_id);
	mClassifiedPanel->sendClassifiedInfoRequest();
	this->setFrontmost(true);
}

// static
void* LLFloaterClassifiedInfo::createClassifiedDetail(void* userdata)
{
	LLFloaterClassifiedInfo *self = (LLFloaterClassifiedInfo*)userdata;
	self->mClassifiedPanel = new LLPanelClassified(TRUE, TRUE);
	self->mClassifiedPanel->childSetValue("classified_url", self->mClassifiedID);
	return self->mClassifiedPanel;
}

// static
LLFloaterClassifiedInfo* LLFloaterClassifiedInfo::show(const LLUUID &classified_id)
{
	if (classified_id.isNull())
	{
		return NULL;
	}

	LLFloaterClassifiedInfo *floater;
	if (gClassifiedInfoInstances.checkData(classified_id))
	{
		// ...bring that window to front
		floater = gClassifiedInfoInstances.getData(classified_id);
		floater->open();	/*Flawfinder: ignore*/
		floater->setFrontmost(true);
	}
	else
	{
		floater =  new LLFloaterClassifiedInfo("calssifiedinfo", classified_id );
		floater->center();
		floater->open();	/*Flawfinder: ignore*/
		floater->displayClassifiedInfo(classified_id);
		floater->setFrontmost(true);
	}

	return floater;
}


