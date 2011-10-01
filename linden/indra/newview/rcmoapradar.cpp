// MOAP Radar
// Robin Cornelius


#include "llviewerprecompiledheaders.h"

#include "llavatarconstants.h"
#include "llfloateravatarlist.h"

#include "lluictrlfactory.h"
#include "llviewerwindow.h"
#include "llscrolllistctrl.h"
#include "llradiogroup.h"
#include "llviewercontrol.h"
#include "llbutton.h"

#include "llvoavatar.h"
#include "llimview.h"
#include "rcmoapradar.h"
#include "llregionflags.h"
#include "llfloaterreporter.h"
#include "llagent.h"
#include "llviewerregion.h"
#include "lltracker.h"
#include "llviewerstats.h"
#include "llerror.h"
#include "llchat.h"
#include "llfloaterchat.h"
#include "llviewermessage.h"
#include "llweb.h"
#include "llviewerobjectlist.h"
#include "llmutelist.h"
#include "llcallbacklist.h"
#include "llmediaentry.h"

#include <time.h>
#include <string.h>

#include <map>

#include "llworld.h"

#include "llsdutil.h"

LLFloaterMOAPRadar* LLFloaterMOAPRadar::sInstance = NULL;

LLFloaterMOAPRadar::LLFloaterMOAPRadar() :  LLFloater(std::string("MOAPradar"))
{
	llassert_always(sInstance == NULL);
	sInstance = this;
	mUpdateRate = gSavedSettings.getU32("MOAPRadarUpdateRate") * 3 + 3;
	mTrackingRunning=false;
}

LLFloaterMOAPRadar::~LLFloaterMOAPRadar()
{
	gIdleCallbacks.deleteFunction(LLFloaterMOAPRadar::callbackIdle);
	sInstance = NULL;
}

//static
void LLFloaterMOAPRadar::toggle(void*)
{
	if (sInstance)
	{
		if (sInstance->getVisible())
		{
			sInstance->close(false);
		}
		else
		{
			sInstance->open();
		}
	}
	else
	{
		showInstance();
	}
}

//static
void LLFloaterMOAPRadar::showInstance()
{
	if (sInstance)
	{
		if (!sInstance->getVisible())
		{
			sInstance->open();
		}
	}
	else
	{
		sInstance = new LLFloaterMOAPRadar();
		LLUICtrlFactory::getInstance()->buildFloater(sInstance, "floater_moap_radar.xml");
	}
}

void LLFloaterMOAPRadar::draw()
{
	LLFloater::draw();
}

void LLFloaterMOAPRadar::onOpen()
{
	gSavedSettings.setBOOL("ShowMOAPRadar", TRUE);
	sInstance->setVisible(TRUE);
}

void LLFloaterMOAPRadar::onClose(bool app_quitting)
{
	sInstance->setVisible(FALSE);
	if (!app_quitting)
	{
		gSavedSettings.setBOOL("ShowMOAPRadar", FALSE);
	}
	if (!gSavedSettings.getBOOL("MOAPRadarKeepOpen") || app_quitting)
	{
		destroy();
	}
}


BOOL LLFloaterMOAPRadar::postBuild()
{

	mMOAPList = getChild<LLScrollListCtrl>("moap_list");
	mMOAPList->sortByColumn("distance", TRUE);

	mTrackBtn = getChild<LLButton>("track_btn");
	mTrackBtn->setLabel(LLStringExplicit("Track"));

	childSetAction("open_btn", onClickOpen, this);
	childSetAction("track_btn", onClickTrack, this);
	childSetAction("copy_btn", onClickCopy, this);

	mMOAPList->setCommitOnSelectionChange(TRUE);
	childSetCommitCallback("moap_list", onSelectMOAP, this);

	gIdleCallbacks.addFunction(LLFloaterMOAPRadar::callbackIdle);

	return TRUE;
}

//static
void LLFloaterMOAPRadar::callbackIdle(void *userdata) {
	if (LLFloaterMOAPRadar::sInstance != NULL)
	{
		// Do not update at every frame: this would be insane !
		if (gFrameCount % LLFloaterMOAPRadar::sInstance->mUpdateRate == 0)
		{
			LLFloaterMOAPRadar::sInstance->updateMOAPList();
		}
	}
}


void LLFloaterMOAPRadar::updateMOAPList()
{
	if (sInstance != this) return;
	
	mMOAPList->deleteAllItems();

	S32 obj_count=gObjectList.getNumObjects();

	int count=0;

	for(int objnum=0;objnum<obj_count;objnum++)
	{
		LLPointer<LLViewerObject> obj = gObjectList.getObject(objnum);

		if(obj.isNull())
			continue;

		if(obj->isDead() || obj->isOrphaned())
			continue;

		if(obj->getMediaType() == LLViewerObject::MEDIA_TYPE_NONE)
			continue;

		LLSD element;
		
		element["id"] = obj->getID();

		for(int face=0;face<obj->getNumTEs();face++)
		{
			const LLTextureEntry * te=obj->getTE(face);
			if(te==NULL)
				continue;

			if(te->hasMedia())
			{
				LLMediaEntry* media=te->getMediaData();
				
				if(media==NULL)
					continue;

				LLSD media_details;
				media->asLLSD(media_details);

				element["columns"][LIST_URL]["column"] = "URL";
				element["columns"][LIST_URL]["type"] = "text";
				std::string URL=media->getCurrentURL();
				if(URL=="")
					URL="(Waiting....)";

				element["columns"][LIST_URL]["value"] = URL;

				if(mTrackingRunning && mTrackedID==obj->getID() && mTrackedFace==face)
				{
					element["columns"][LIST_URL]["font-style"] = "BOLD";
				}
				else
				{
					element["columns"][LIST_URL]["font-style"] = "NORMAL";
				}

				element["columns"][LIST_FACE]["column"] = "face";
				element["columns"][LIST_FACE]["type"] = "text";
				std::stringstream face_buf;
				face_buf << face;
				element["columns"][LIST_FACE]["value"] = face_buf.str();

				LLVector3d offset = gAgent.getPositionGlobal()-obj->getPositionGlobal();
				S32 dist= offset.length();

				element["columns"][LIST_DISTANCE]["column"] = "distance";
				element["columns"][LIST_DISTANCE]["type"] = "text";
				std::stringstream dist_buf;
				dist_buf << std::fixed << std::setprecision(2) << dist<<"m";
				element["columns"][LIST_DISTANCE]["value"] = dist_buf.str();
				
				if(dist<25)
					element["columns"][LIST_DISTANCE]["color"] = LLColor4::green.getValue();
				else if(dist<50)
					element["columns"][LIST_DISTANCE]["color"] = LLColor4::blue1.getValue();
				else if(dist<100)
					element["columns"][LIST_DISTANCE]["color"] = LLColor4::black.getValue();
				else if(dist<256)
					element["columns"][LIST_DISTANCE]["color"] = LLColor4::yellow.getValue();
				else
					element["columns"][LIST_DISTANCE]["color"] = LLColor4::red.getValue();


				element["columns"][LIST_POSITION]["column"] = "position";
				element["columns"][LIST_POSITION]["type"] = "text";
				element["columns"][LIST_POSITION]["value"] = "";
 
				element["columns"][LIST_ALTITUDE]["column"] = "altitude";
				element["columns"][LIST_ALTITUDE]["type"] = "text";
				std::stringstream alt_buf;
				alt_buf << std::fixed << std::setprecision(2) << obj->getPositionGlobal().mdV[2]<<"m";
				element["columns"][LIST_ALTITUDE]["value"] = alt_buf.str();

				element["columns"][LIST_POSITION]["column"]="position";
				element["columns"][LIST_POSITION]["type"] = "text";
				std::stringstream pos_buf;

				S32 moap_x = (S32)offset.mdV[VX];
				S32 moap_y = (S32)offset.mdV[VY];
				if (moap_x >= -256 && moap_x <= 256 && moap_y >= -256 && moap_y <= 256)
				{
					pos_buf<< std::fixed << std::setprecision(2)<<obj->getPositionRegion().mV[VX]<<","<<obj->getPositionRegion().mV[VY];
				}
				else
				{
					if (moap_y < 0)
						pos_buf<<"S";
					else if (moap_y > 256)
						pos_buf<<"N";

					if (moap_x < 0)
						pos_buf<<"W";
					else if (moap_x > 256)
						pos_buf<<"E";
				}
				

				element["columns"][LIST_POSITION]["value"] = pos_buf.str();

				mMOAPList->addElement(element, ADD_BOTTOM);

				if(mSelectedObjID==obj->getID() && mSelectedFace==face)
				{
					mMOAPList->selectNthItem(count);
				}

				count++;
			}
		}
	}

		updatetrackbtn();
}

// static
void LLFloaterMOAPRadar::onClickOpen(void* userdata)
{
	LLFloaterMOAPRadar *self = (LLFloaterMOAPRadar*)userdata;

	LLDynamicArray<LLUUID> ids = self->mMOAPList->getSelectedIDs();

	if(ids.empty())
	{
		return;
	}

	LLUUID id=ids.front();

	LLPointer<LLViewerObject> obj = gObjectList.findObject(id);
	if(obj.notNull())
	{

		const LLTextureEntry * te=obj->getTE(self->mSelectedFace);
		if(te==NULL)
			return;

		if(te->hasMedia())
		{
			LLMediaEntry* media=te->getMediaData();
			if(media)
			{
				//gViewerWindow->mWindow->copyTextToClipboard(utf8str_to_wstring(media->getCurrentURL()));
				LLWeb::loadURL(media->getCurrentURL());
			}
		}			
	}

}

// static
void LLFloaterMOAPRadar::onClickTrack(void* userdata)
{
	LLFloaterMOAPRadar *self = (LLFloaterMOAPRadar*)userdata;
	
	LLDynamicArray<LLUUID> ids = self->mMOAPList->getSelectedIDs();

	if(self->mTrackingRunning)
	{
		LLTracker::stopTracking(NULL);
		self->mTrackingRunning=false;
	}

	if(ids.empty())
	{
		self->mTrackedID=LLUUID::null;
		self->mTrackingRunning=false;
		return;
	}

	LLUUID id=ids.front();

	if(id==self->mTrackedID && self->mSelectedFace==self->mTrackedFace)
	{
		self->mTrackedID=LLUUID::null;
		self->mTrackingRunning=false;
		return;
	}

	self->mTrackedID=LLUUID::null;

	LLPointer<LLViewerObject> obj = gObjectList.findObject(id);
	if(obj.notNull())
	{
		LLTracker::trackLocation(obj->getPositionGlobal(),"MOAP Tracking","",LLTracker::LOCATION_ITEM);
		self->mTrackingRunning=true;
		self->mTrackedID=id;
		self->mTrackedFace=self->mSelectedFace;
	}
}

// static
void LLFloaterMOAPRadar::onClickCopy(void* userdata)
{
	LLFloaterMOAPRadar *self = (LLFloaterMOAPRadar*)userdata;

	LLDynamicArray<LLUUID> ids = self->mMOAPList->getSelectedIDs();

	if(ids.empty())
	{
		LLTracker::stopTracking(NULL);
		return;
	}

	LLUUID id=ids.front();

	LLPointer<LLViewerObject> obj = gObjectList.findObject(id);

	if(obj.notNull())
	{

		const LLTextureEntry * te=obj->getTE(self->mSelectedFace);
		if(te==NULL)
			return;

		if(te->hasMedia())
		{
			LLMediaEntry* media=te->getMediaData();
			if(media)
			{
				gViewerWindow->mWindow->copyTextToClipboard(utf8str_to_wstring(media->getCurrentURL()));
			}
		}			
	}

}

//static
void LLFloaterMOAPRadar::onSelectMOAP(LLUICtrl*, void* userdata)
{
	LLFloaterMOAPRadar *self = (LLFloaterMOAPRadar*)userdata;

 	LLScrollListItem *item = self->mMOAPList->getFirstSelected();
	if (item)
	{
		self->mSelectedObjID = item->getUUID();
		self->mSelectedFace = item->getColumn(LIST_FACE)->getValue().asInteger();
	}

	self->updatetrackbtn();
}

void LLFloaterMOAPRadar::updatetrackbtn()
{
	if( mTrackingRunning)
	{
		if( mTrackedID==mSelectedObjID && mTrackedFace==mSelectedFace)
		{
			mTrackBtn->setLabel(LLStringExplicit("Untrack"));
		}
		else
		{
			mTrackBtn->setLabel(LLStringExplicit("New Track"));
		}
	}
	else
	{
		mTrackBtn->setLabel(LLStringExplicit("Track"));
	}

}