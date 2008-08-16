/** 
 * @file llpanelevent.cpp
 * @brief Display for events in the finder
 *
 * Copyright (c) 2004-2007, Linden Research, Inc.
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

#include "llviewerprecompiledheaders.h"

#include "llpanelevent.h"
#include "message.h"
#include "llui.h"

#include "llagent.h"
#include "llviewerwindow.h"
#include "llbutton.h"
#include "llcachename.h"
#include "lleventflags.h"
#include "lleventnotifier.h"
#include "llfloater.h"
#include "llfloaterworldmap.h"
#include "llinventorymodel.h"
#include "llsecondlifeurls.h"
#include "lltextbox.h"
#include "llviewertexteditor.h"
#include "lluiconstants.h"
#include "llviewercontrol.h"
#include "llweb.h"
#include "llworldmap.h"
#include "llvieweruictrlfactory.h"

//static
std::list<LLPanelEvent*> LLPanelEvent::sAllPanels;

LLPanelEvent::LLPanelEvent() : LLPanel("Event Panel")
{
	sAllPanels.push_back(this);
}


LLPanelEvent::~LLPanelEvent()
{
	sAllPanels.remove(this);
}


BOOL LLPanelEvent::postBuild()
{
	mTBName = LLViewerUICtrlFactory::getTextBoxByName(this, "event_name");
	mTBName->setColor(gColors.getColor( "LabelSelectedColor" ));

	mTBCategory = LLViewerUICtrlFactory::getTextBoxByName(this, "event_category");
	mTBCategory->setColor(gColors.getColor( "LabelSelectedColor" ));

	mMatureText = LLViewerUICtrlFactory::getTextBoxByName(this, "event_mature");
	mMatureText->setColor(gColors.getColor( "LabelSelectedColor" ));
	
	mTBDate = LLViewerUICtrlFactory::getTextBoxByName(this, "event_date");
	mTBDate->setColor(gColors.getColor( "LabelSelectedColor" ));

	mTBDuration = LLViewerUICtrlFactory::getTextBoxByName(this, "event_duration");
	mTBDuration->setColor(gColors.getColor( "LabelSelectedColor" ));

	mTBDesc = LLUICtrlFactory::getTextEditorByName(this, "event_desc");
	mTBDesc->setWordWrap(TRUE);
	mTBDesc->setEnabled(FALSE);
	mTBDesc->setFgColor(gColors.getColor( "LabelSelectedColor" ));
	mTBDesc->setReadOnlyFgColor(LLColor4(1.f, 1.f, 1.f, 1.f));

	mTBRunBy = LLViewerUICtrlFactory::getTextBoxByName(this, "event_runby");
	mTBRunBy->setColor(gColors.getColor( "LabelSelectedColor" ));
	mTBLocation = LLViewerUICtrlFactory::getTextBoxByName(this, "event_location");
	mTBLocation->setColor(gColors.getColor( "LabelSelectedColor" ));
	mTBCover = LLViewerUICtrlFactory::getTextBoxByName(this, "event_cover");
	mTBCover->setColor(gColors.getColor( "LabelSelectedColor" ));

	mTeleportBtn = LLViewerUICtrlFactory::getButtonByName(this, "teleport_btn");
	mTeleportBtn->setClickedCallback(onClickTeleport);
	mTeleportBtn->setCallbackUserData(this);

	mMapBtn = LLViewerUICtrlFactory::getButtonByName(this, "map_btn");
	mMapBtn->setClickedCallback(onClickMap);
	mMapBtn->setCallbackUserData(this);

	//mLandmarkBtn = LLViewerUICtrlFactory::getButtonByName(this, "landmark_btn");
	//mLandmarkBtn->setClickedCallback(onClickLandmark);
	//mLandmarkBtn->setCallbackUserData(this);

	mNotifyBtn = LLViewerUICtrlFactory::getButtonByName(this, "notify_btn");
	mNotifyBtn->setClickedCallback(onClickNotify);
	mNotifyBtn->setCallbackUserData(this);

	mCreateEventBtn = LLViewerUICtrlFactory::getButtonByName(this, "create_event_btn");
	mCreateEventBtn->setClickedCallback(onClickCreateEvent);
	mCreateEventBtn->setCallbackUserData(this);

	return TRUE;
}


void LLPanelEvent::setEventID(const U32 event_id)
{
	mEventID = event_id;
	// Should reset all of the panel state here
	resetInfo();

	if (event_id != 0)
	{
		sendEventInfoRequest();
	}
}


void LLPanelEvent::sendEventInfoRequest()
{
	LLMessageSystem *msg = gMessageSystem;

	msg->newMessageFast(_PREHASH_EventInfoRequest);
	msg->nextBlockFast(_PREHASH_AgentData);
	msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID() );
	msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID() );
	msg->nextBlockFast(_PREHASH_EventData);
	msg->addU32Fast(_PREHASH_EventID, mEventID);
	gAgent.sendReliableMessage();
}


//static 
void LLPanelEvent::processEventInfoReply(LLMessageSystem *msg, void **)
{
	// extract the agent id
	LLUUID agent_id;
	msg->getUUIDFast(_PREHASH_AgentData, _PREHASH_AgentID, agent_id );

	U32 event_id;
	msg->getU32("EventData", "EventID", event_id);

	// look up all panels which have this avatar
	for (panel_list_t::iterator iter = sAllPanels.begin(); iter != sAllPanels.end(); ++iter)
	{
		LLPanelEvent* self = *iter;
		// Skip updating panels which aren't for this event
		if (self->mEventID != event_id)
		{
			continue;
		}
		char buffer[256];		/*Flawfinder: ignore*/

		self->mEventInfo.unpack(msg);
		self->mTBName->setText(self->mEventInfo.mName);
		self->mTBCategory->setText(self->mEventInfo.mCategoryStr);
		self->mTBDate->setText(self->mEventInfo.mTimeStr);
		self->mTBDesc->setText(self->mEventInfo.mDesc);

		snprintf(buffer, sizeof(buffer), "%d:%.2d", self->mEventInfo.mDuration / 60, self->mEventInfo.mDuration % 60);			/* Flawfinder: ignore */

		self->mTBDuration->setText(buffer);

		if (!self->mEventInfo.mHasCover)
		{
			self->mTBCover->setText("none");
		}
		else
		{
			snprintf(buffer, sizeof(buffer), "%d", self->mEventInfo.mCover);			/* Flawfinder: ignore */
			self->mTBCover->setText(buffer);
		}

		F32 global_x = (F32)self->mEventInfo.mPosGlobal.mdV[VX];
		F32 global_y = (F32)self->mEventInfo.mPosGlobal.mdV[VY];

		S32 region_x = llround(global_x) % REGION_WIDTH_UNITS;
		S32 region_y = llround(global_y) % REGION_WIDTH_UNITS;
		S32 region_z = llround((F32)self->mEventInfo.mPosGlobal.mdV[VZ]);
		
		snprintf(buffer, sizeof(buffer), "%s (%d, %d, %d)", self->mEventInfo.mSimName.c_str(), region_x, region_y, region_z);			/* Flawfinder: ignore */
		self->mTBLocation->setText(buffer);

		if (self->mEventInfo.mEventFlags & EVENT_FLAG_MATURE)
		{
			self->mMatureText->setText("Yes");
		}
		else
		{
			self->mMatureText->setText("No");
		}

		if (self->mEventInfo.mUnixTime < time_corrected())
		{
			self->mNotifyBtn->setEnabled(FALSE);
		}
		else
		{
			self->mNotifyBtn->setEnabled(TRUE);
		}
		
		if (gEventNotifier.hasNotification(self->mEventInfo.mID))
		{
			self->mNotifyBtn->setLabelSelected("Don't Notify");
			self->mNotifyBtn->setLabelUnselected("Don't Notify");
		}
		else
		{
			self->mNotifyBtn->setLabelSelected("Notify");
			self->mNotifyBtn->setLabelUnselected("Notify");
		}
	}
}


void LLPanelEvent::draw()
{
	char firstname[DB_FIRST_NAME_BUF_SIZE];		/*Flawfinder: ignore*/
	char lastname[DB_LAST_NAME_BUF_SIZE];		/*Flawfinder: ignore*/
	gCacheName->getName(mEventInfo.mRunByID, firstname, lastname);

	LLString name;
	name = firstname;
	name += " ";
	name += lastname;
	mTBRunBy->setText(name);

	LLPanel::draw();
}

void LLPanelEvent::resetInfo()
{
	// Clear all of the text fields.
}

// static
void LLPanelEvent::onClickTeleport(void* data)
{
	LLPanelEvent* self = (LLPanelEvent*)data;

	if (!self->mEventInfo.mPosGlobal.isExactlyZero())
	{
		gAgent.teleportViaLocation(self->mEventInfo.mPosGlobal);
		gFloaterWorldMap->trackLocation(self->mEventInfo.mPosGlobal);
	}
}


// static
void LLPanelEvent::onClickMap(void* data)
{
	LLPanelEvent* self = (LLPanelEvent*)data;

	if (!self->mEventInfo.mPosGlobal.isExactlyZero())
	{
		gFloaterWorldMap->trackLocation(self->mEventInfo.mPosGlobal);
		LLFloaterWorldMap::show(NULL, TRUE);
	}
}


// static
/*
void LLPanelEvent::onClickLandmark(void* data)
{
	LLPanelEvent* self = (LLPanelEvent*)data;
	//create_landmark(self->mTBName->getText(), "", self->mEventInfo.mPosGlobal);
	LLMessageSystem* msg = gMessageSystem;
	msg->newMessage("CreateLandmarkForEvent");
	msg->nextBlockFast(_PREHASH_AgentData);
	msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
	msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
	msg->nextBlockFast(_PREHASH_EventData);
	msg->addU32Fast(_PREHASH_EventID, self->mEventID);
	msg->nextBlockFast(_PREHASH_InventoryBlock);
	LLUUID folder_id;
	folder_id = gInventory.findCategoryUUIDForType(LLAssetType::AT_LANDMARK);
	msg->addUUIDFast(_PREHASH_FolderID, folder_id);
	msg->addStringFast(_PREHASH_Name, self->mTBName->getText());
	gAgent.sendReliableMessage();
}
*/

// static
void LLPanelEvent::onClickCreateEvent(void* data)
{
	LLPanelEvent* self = (LLPanelEvent*)data;
	
	gViewerWindow->alertXml("PromptGoToEventsPage",
		callbackCreateEventWebPage, 
		self); 
}

// static
void LLPanelEvent::onClickNotify(void *data)
{
	LLPanelEvent* self = (LLPanelEvent*)data;

	if (!gEventNotifier.hasNotification(self->mEventID))
	{
		gEventNotifier.add(self->mEventInfo);
		self->mNotifyBtn->setLabelSelected("Don't Notify");
		self->mNotifyBtn->setLabelUnselected("Don't Notify");
	}
	else
	{
		gEventNotifier.remove(self->mEventInfo.mID);
		self->mNotifyBtn->setLabelSelected("Notify");
		self->mNotifyBtn->setLabelUnselected("Notify");
	}
}

// static
void LLPanelEvent::callbackCreateEventWebPage(S32 option, void* data)
{
	if (0 == option)
	{
		llinfos << "Loading events page " << EVENTS_URL << llendl;

		LLWeb::loadURL(EVENTS_URL);
	}
}
