/**
 * @file llfloaterteleporthistory.cpp
 * @author Zi Ree
 * @brief LLFloaterTeleportHistory class implementation
 *
 * $LicenseInfo:firstyear=2001&license=viewergpl$
 *
 * Copyright (c) 2008, Linden Research, Inc.
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
 * online at http://secondlifegrid.net/programs/open_source/licensing/flossexception
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

#include "linden_common.h"

#include "llfloaterteleporthistory.h"
#include "llfloaterworldmap.h"
#include "lltimer.h"
#include "lluictrlfactory.h"
#include "llurldispatcher.h"
#include "llurlsimstring.h"
#include "llviewercontrol.h"   // gSavedSettings
#include "llviewerwindow.h"
#include "llweb.h"

#include "apr_time.h"

// [RLVa:KB] - Emerald specific
#include "rlvhandler.h"
#include "llsdserialize.h"
// [/RLVa:KB]

// globals
LLFloaterTeleportHistory* gFloaterTeleportHistory;

LLFloaterTeleportHistory::LLFloaterTeleportHistory()
:	LLFloater(std::string("teleporthistory")),
	mPlacesInList(NULL),
	mPlacesOutList(NULL),
	pItem(NULL),
	id(0)
{
	LLUICtrlFactory::getInstance()->buildFloater(this, "floater_teleport_history.xml", NULL);
}

// virtual
LLFloaterTeleportHistory::~LLFloaterTeleportHistory()
{
}

// virtual
void LLFloaterTeleportHistory::onFocusReceived()
{
	// take care to enable or disable buttons depending on the selection in the places list
	if(pItem)
	{
		setButtonsEnabled(TRUE);
	}
	else
	{
		setButtonsEnabled(FALSE);
	}
	LLFloater::onFocusReceived();
}

BOOL LLFloaterTeleportHistory::postBuild()
{
	// make sure the cached pointer to the scroll list is valid
	mPlacesInList=getChild<LLScrollListCtrl>("places_list_in");
	if(!mPlacesInList)
	{
		llwarns << "coud not get pointer to places list in" << llendl;
		return FALSE;
	}
	mPlacesOutList=getChild<LLScrollListCtrl>("places_list_out");
	if(!mPlacesOutList)
	{
		llwarns << "coud not get pointer to places list out" << llendl;
		return FALSE;
	}

	// setup callbacks for the scroll list
	mPlacesInList->setDoubleClickCallback(onTeleport);
	mPlacesOutList->setDoubleClickCallback(onTeleport);
	childSetCommitCallback("places_list_in", onInPlacesSelected, this);
	childSetCommitCallback("places_list_out", onOutPlacesSelected, this);
	childSetAction("teleport", onTeleport, this);
	childSetAction("show_on_map", onShowOnMap, this);
	childSetAction("copy_slurl", onCopySLURL, this);
	childSetAction("clear_history", onClearHistory,this);
	loadEntrys();

	return TRUE;
}
void LLFloaterTeleportHistory::saveEntry(LLSD toSave)
{
	tpList.append(toSave);
	std::string filename=getFileName();
	llofstream file;
	file.open(filename.c_str());
	LLSDSerialize::toPrettyXML(tpList, file);
	file.close();
}
void LLFloaterTeleportHistory::clearHistory()
{
	tpList.clear();
	saveEntry(tpList);
	mPlacesOutList->clearRows();
	mPlacesInList->clearRows();
}
std::string LLFloaterTeleportHistory::getFileName()
{
	std::string path=gDirUtilp->getExpandedFilename(LL_PATH_PER_SL_ACCOUNT, "");

	if (!path.empty())
	{
		path = gDirUtilp->getExpandedFilename(LL_PATH_PER_SL_ACCOUNT, "teleport_history.xml");
	}
	return path;
}
void LLFloaterTeleportHistory::loadEntrys()
{
	std::string filename=getFileName();
	if (filename.empty())
	{
		llinfos << "no valid user directory." << llendl;
		return;
	}
	llifstream file;
	file.open(filename.c_str());
	if (file.is_open())
		LLSDSerialize::fromXML(tpList, file);
	file.close();

	for(int i = 0;i<(int)tpList.size();i++)
	{
		LLSD data = tpList[i];
		LLScrollListCtrl* pItemPointer;
		if(data["out"].asBoolean())
			pItemPointer=mPlacesOutList;
		else
			pItemPointer=mPlacesInList;

		pItemPointer->addElement(data, ADD_TOP);
		pItemPointer->deselectAllItems(TRUE);
		setButtonsEnabled(FALSE);
		id++;
	}
}
void LLFloaterTeleportHistory::addEntry(std::string regionName, S16 x, S16 y, S16 z,bool outList)
{
	LLScrollListCtrl* pItemPointer;
	if(outList)
		pItemPointer=mPlacesOutList;
	else
		pItemPointer=mPlacesInList;
	// only if the cached scroll list pointer is valid
	if(pItemPointer)
	{
		// prepare display of position
		std::string position=llformat("%d, %d, %d", x, y, z);
		// prepare simstring for later parsing
		std::string simString = regionName + llformat("/%d/%d/%d", x, y, z);
		simString = LLWeb::escapeURL(simString);

		// check if we are in daylight savings time
		std::string timeZone = "PST";
		if(is_daylight_savings()) timeZone = "PDT";

		// do all time related stuff as closely together as possible, because every other operation
		// might change the internal tm* buffer
		struct tm* internal_time;
		internal_time = utc_to_pacific_time(time_corrected(), is_daylight_savings());
		std::string timeString=llformat("%02d/%02d/%04d - %02d:%02d:%02d ",internal_time->tm_mon+1,internal_time->tm_mday,internal_time->tm_year+1900,internal_time->tm_hour, internal_time->tm_min, internal_time->tm_sec)+timeZone;

		// build the list entry
		LLSD value;
		value["id"] = id;
		value["columns"][0]["column"] = "region";
		value["columns"][0]["value"] = regionName;
		value["columns"][1]["column"] = "position";
		value["columns"][1]["value"] = position;
		value["columns"][2]["column"] = "visited";
		value["columns"][2]["value"] = timeString;

		// these columns are hidden and serve as data storage for simstring and SLURL
		value["columns"][3]["column"] = "slurl";
		value["columns"][3]["value"] = LLURLDispatcher::buildSLURL(regionName, x, y, z);
		value["columns"][4]["column"] = "simstring";
		value["columns"][4]["value"] = simString;
		value["out"]=outList;

// [RLVa:KB] - Alternate: Imprudence-1.4
		if (gRlvHandler.hasBehaviour(RLV_BHVR_SHOWLOC))
		{
			value["columns"][0]["value"] = RlvStrings::getString(RLV_STRING_HIDDEN_REGION);
			value["columns"][1]["value"] = RlvStrings::getString(RLV_STRING_HIDDEN);
			value["columns"][3]["value"] = RlvStrings::getString(RLV_STRING_HIDDEN);
			value["columns"][4]["value"] = RlvStrings::getString(RLV_STRING_HIDDEN);
		}
// [/RLVa:KB]
		saveEntry(value);
		// add the new list entry on top of the list, deselect all and disable the buttons
		pItemPointer->addElement(value, ADD_TOP);
		pItemPointer->deselectAllItems(TRUE);
		setButtonsEnabled(FALSE);
		id++;
	}
	else
	{
		llwarns << "pointer to places list is NULL" << llendl;
	}
}

void LLFloaterTeleportHistory::setButtonsEnabled(BOOL on)
{
// [RLVa:KB] - Alternate: Imprudence-1.4
	if (rlv_handler_t::isEnabled())
	{
		if ( (pItem) && (pItem->getColumn(4)) && (RlvStrings::getString(RLV_STRING_HIDDEN) == pItem->getColumn(4)->getValue().asString()) )
		{
			on = FALSE;
		}
	}
// [/RLVa:K]

	// enable or disable buttons
	childSetEnabled("teleport", on);
	childSetEnabled("show_on_map", on);
	childSetEnabled("copy_slurl", on);
}

// virtual
void LLFloaterTeleportHistory::onClose(bool app_quitting)
{
	LLFloater::setVisible(FALSE);
}

// virtual
BOOL LLFloaterTeleportHistory::canClose()
{
	return !LLApp::isExiting();
}

// callbacks

// static
void LLFloaterTeleportHistory::onInPlacesSelected(LLUICtrl* /* ctrl */, void* data)
{
	LLFloaterTeleportHistory* self = (LLFloaterTeleportHistory*) data;
	self->mPlacesOutList->deselectAllItems();
	self->pItem = self->mPlacesInList->getFirstSelected();
	// on selection change check if we need to enable or disable buttons
	if(self->pItem)
	{
		self->setButtonsEnabled(TRUE);
	}
	else
	{
		self->setButtonsEnabled(FALSE);
	}
}

// static
void LLFloaterTeleportHistory::onOutPlacesSelected(LLUICtrl* /* ctrl */, void* data)
{
	LLFloaterTeleportHistory* self = (LLFloaterTeleportHistory*) data;
	self->mPlacesInList->deselectAllItems();
	self->pItem = self->mPlacesOutList->getFirstSelected();
	// on selection change check if we need to enable or disable buttons
	if(self->pItem)
	{
		self->setButtonsEnabled(TRUE);
	}
	else
	{
		self->setButtonsEnabled(FALSE);
	}
}

// static
void LLFloaterTeleportHistory::onTeleport(void* data)
{
	LLFloaterTeleportHistory* self = (LLFloaterTeleportHistory*) data;

	// build secondlife::/app link from simstring for instant teleport to destination
	std::string slapp="secondlife:///app/teleport/" + self->pItem->getColumn(4)->getValue().asString();
	LLURLDispatcher::dispatch(slapp, NULL, true);
}

// static
void LLFloaterTeleportHistory::onClearHistory(void* data)
{
	LLFloaterTeleportHistory* self = (LLFloaterTeleportHistory*) data;
	self->clearHistory();
}

// static
void LLFloaterTeleportHistory::onShowOnMap(void* data)
{
	LLFloaterTeleportHistory* self = (LLFloaterTeleportHistory*) data;

	// get simstring from selected entry and parse it for its components
	std::string simString = self->pItem->getColumn(4)->getValue().asString();
	std::string region = "";
	S32 x = 128;
	S32 y = 128;
	S32 z = 20;

	LLURLSimString::parse(simString, &region, &x, &y, &z);

	// point world map at position
	gFloaterWorldMap->trackURL(region, x, y, z);
	LLFloaterWorldMap::show(NULL, TRUE);
}

// static
void LLFloaterTeleportHistory::onCopySLURL(void* data)
{
	LLFloaterTeleportHistory* self = (LLFloaterTeleportHistory*) data;

	// get SLURL of the selected entry and copy it to the clipboard
	std::string SLURL=self->pItem->getColumn(3)->getValue().asString();
	gViewerWindow->mWindow->copyTextToClipboard(utf8str_to_wstring(SLURL));
}
