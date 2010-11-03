/** 
 * @file llfloaterclassified.cpp
 * @brief Classified information as shown in a floating window from 
 * secondlife:// command handler.  
 *
 * $LicenseInfo:firstyear=2002&license=viewergpl$
 * 
 * Copyright (c) 2002-2009, Linden Research, Inc.
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

#include "llviewerprecompiledheaders.h"

#include "llfloaterclassified.h"

// viewer project includes
#include "llcommandhandler.h"
#include "llpanelclassified.h"

// linden library includes
#include "lluuid.h"
#include "lluictrlfactory.h"

//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------

LLMap< const LLUUID, LLFloaterClassifiedInfo* > gClassifiedInfoInstances;
////////////////////////////////////////////////////////////////////////////
// LLFloaterEventDisplay

class LLClassifiedHandler : public LLCommandHandler
{
public:
	// requires trusted browser to trigger
	LLClassifiedHandler() : LLCommandHandler("classified", true) { }
	bool handle(const LLSD& tokens, const LLSD& query_map,
				LLMediaCtrl* web)
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
	LLUICtrlFactory::getInstance()->buildFloater(this, "floater_preview_classified.xml", &getFactoryMap());
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
	self->mClassifiedPanel = new LLPanelClassified(true, true);
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


