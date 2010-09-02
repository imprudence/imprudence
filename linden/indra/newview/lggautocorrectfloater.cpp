/* Copyright (c) 2009
*
* Greg Hendrickson (LordGregGreg Back). All rights reserved.
*
* Redistribution and use in source and binary forms, with or
* without modification, are permitted provided that the following
* conditions are met:
*
*   1. Redistributions of source code must retain the above copyright
*      notice, this list of conditions and the following disclaimer.
*   2. Redistributions in binary form must reproduce the above
*      copyright notice, this list of conditions and the following
*      disclaimer in the documentation and/or other materials provided
*      with the distribution.
*   3. Neither the name Modular Systems nor the names of its contributors
*      may be used to endorse or promote products derived from this
*      software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY MODULAR SYSTEMS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
* THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
* PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL MODULAR SYSTEMS OR CONTRIBUTORS
* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
* THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "llviewerprecompiledheaders.h"

#include "lggautocorrectfloater.h"

#include "llagentdata.h"
#include "llcommandhandler.h"
#include "llfloater.h"
#include "lluictrlfactory.h"
#include "llagent.h"
#include "llpanel.h"
#include "llbutton.h"
#include "llcolorswatch.h"
#include "llcombobox.h"
#include "llview.h"
#include "llprefsadvanced.h"
#include "llhttpclient.h"
#include "llbufferstream.h"
#include "llcheckboxctrl.h"
#include "llviewercontrol.h"

#include "floaterbusy.h" // InvDropTarget
#include "llui.h"
#include "llcontrol.h"
#include "llscrolllistctrl.h"
#include "llscrollingpanellist.h"
#include "lggautocorrect.h"
#include "llfilepicker.h"
#include "llfile.h"
#include "llsdserialize.h"
#include "llfloaterchat.h"
#include "llchat.h"
#include "llviewerinventory.h"
#include "llinventorymodel.h"
#include "llhost.h"
#include "llassetstorage.h"
#include "roles_constants.h"
#include "llviewertexteditor.h"
#include <boost/tokenizer.hpp>

#include <iosfwd>

class lggAutoCorrectFloater;
class lggAutoCorrectFloater : public LLFloater, public LLFloaterSingleton<lggAutoCorrectFloater>
{
public:
	lggAutoCorrectFloater(const LLSD& seed);
	virtual ~lggAutoCorrectFloater();
	BOOL postBuild(void);
	void setData(void * data);
	void updateEnabledStuff();
	void updateNamesList();
	void updateListControlsEnabled(BOOL selected);
	void updateItemsList();

	
	LLScrollListCtrl *namesList;

	LLScrollListCtrl *entryList;
	LLPrefsAdvanced * empanel;
private:
	static lggAutoCorrectFloater* sInstance;
	static InvDropTarget* mNotecardDropTarget;
	static void onBoxCommitEnabled(LLUICtrl* caller, void* user_data);
	static void onEntrySettingChange(LLUICtrl* caller, void* user_data);
	static void onSelectName(LLUICtrl* caller, void* user_data);
	static void ResponseItemDrop(LLViewerInventoryItem* item);
	static void onNotecardLoadComplete(LLVFS *vfs,const LLUUID& asset_uuid,LLAssetType::EType type,void* user_data, S32 status, LLExtStat ext_status);


	static void deleteEntry(void* data);
	static void addEntry(void* data);
	static void exportList(void* data);
	static void removeList(void* data);
	static void loadList(void* data);
	
};



InvDropTarget * lggAutoCorrectFloater::mNotecardDropTarget;
lggAutoCorrectFloater* lggAutoCorrectFloater::sInstance;

lggAutoCorrectFloater::~lggAutoCorrectFloater()
{
	sInstance = NULL;
	delete mNotecardDropTarget;
	mNotecardDropTarget = NULL;
}
lggAutoCorrectFloater::lggAutoCorrectFloater(const LLSD& seed)
{
	LLUICtrlFactory::getInstance()->buildFloater(this, "floater_autocorrect.xml");

	if (getRect().mLeft == 0 
		&& getRect().mBottom == 0)
	{
		center();
	}
	if(sInstance)delete sInstance;
	sInstance = this;

}

BOOL lggAutoCorrectFloater::postBuild(void)
{

	namesList = getChild<LLScrollListCtrl>("em_ac_list_name");
	entryList = getChild<LLScrollListCtrl>("em_ac_list_entry");

	childSetCommitCallback("em_ac_enable",onBoxCommitEnabled);

	childSetCommitCallback("em_ac_list_enabled",onEntrySettingChange);
	childSetCommitCallback("em_ac_list_show",onEntrySettingChange);
	childSetCommitCallback("em_ac_list_style",onEntrySettingChange);
	childSetCommitCallback("em_ac_priority",onEntrySettingChange);
	

	
	updateEnabledStuff();
	updateNamesList();	


	namesList->setCommitOnSelectionChange(TRUE);
	childSetCommitCallback("em_ac_list_name", onSelectName, this);
	
	childSetAction("em_ac_deletelist",removeList,this);
	childSetAction("em_ac_rementry",deleteEntry,this);
	childSetAction("em_ac_exportlist",exportList,this);
	childSetAction("em_ac_addentry",addEntry,this);
	childSetAction("em_ac_loadlist",loadList,this);

	LLView *target_view = getChild<LLView>("em_ac_notecard_target");
	if(target_view)
	{
		if (mNotecardDropTarget)//shouldn't happen
		{
			delete mNotecardDropTarget;
		}
		mNotecardDropTarget = new InvDropTarget("drop target", target_view->getRect(), ResponseItemDrop);//, mAvatarID);
		addChild(mNotecardDropTarget);
	}
	return true;
}
void lggAutoCorrectFloater::ResponseItemDrop(LLViewerInventoryItem* item)
{
	if(item)
	{
		if (gAgent.allowOperation(PERM_COPY, item->getPermissions(),GP_OBJECT_MANIPULATE) || gAgent.isGodlike())
		{
			if(!item->getAssetUUID().isNull())
			gAssetStorage->getInvItemAsset(LLHost::invalid,
				gAgent.getID(),
				gAgent.getSessionID(),
				item->getPermissions().getOwner(),
				LLUUID::null,
				item->getUUID(),
				item->getAssetUUID(),
				item->getType(),
				&onNotecardLoadComplete,
				(void*)item,
				TRUE);
			gSavedSettings.setBOOL("EmeraldEnableAutoCorrect",true);

		}
	}
}
void lggAutoCorrectFloater::onNotecardLoadComplete(LLVFS *vfs,const LLUUID& asset_uuid,LLAssetType::EType type,void* user_data, S32 status, LLExtStat ext_status)
{
	if(status == LL_ERR_NOERR)
	{
		S32 size = vfs->getSize(asset_uuid, type);
		U8* buffer = new U8[size];
		vfs->getData(asset_uuid, type, buffer, 0, size);

		if(type == LLAssetType::AT_NOTECARD)
		{
			LLViewerTextEditor* edit = new LLViewerTextEditor("",LLRect(0,0,0,0),S32_MAX,"");
			if(edit->importBuffer((char*)buffer, (S32)size))
			{
				llinfos << "decode success" << llendl;
				std::string card = edit->getText();
				edit->die();
				LLSD info;
				std::istringstream ins; // Declare an input string stream.
				ins.str(card);        // Specify string to read.

				LLSDSerialize::fromXML(info,ins);

				LGGAutoCorrect::getInstance()->addCorrectionList(info);
				llinfos << "read success" << llendl;	
				sInstance->updateEnabledStuff();
			}
			else
			{
				llinfos << "decode error" << llendl;
			}
		}
	}
	else
	{
		llinfos << "read error" << llendl;
	}
}


void lggAutoCorrectFloater::onSelectName(LLUICtrl* ctrl, void* user_data)
{
	sInstance->updateItemsList();
}
void lggAutoCorrectFloater::updateItemsList()
{
	entryList->deleteAllItems();
	if((namesList->getAllSelected().size())<=0)
	{

		updateListControlsEnabled(FALSE);
		return;
	}

	updateListControlsEnabled(TRUE);
	std::string listName= namesList->getFirstSelected()->getColumn(0)->getValue().asString();
	
	LLSD listData = LGGAutoCorrect::getInstance()->getAutoCorrectEntries(listName);
	childSetValue("em_ac_list_enabled",listData["enabled"].asBoolean());
	childSetValue("em_ac_list_style",listData["wordStyle"].asBoolean());
	childSetValue("em_ac_list_show",listData["announce"].asBoolean());
	childSetValue("em_ac_text_name",listName);
	childSetValue("em_ac_text_author",listData["author"]);
	childSetValue("em_ac_priority",listData["priority"]);
	static S32 *countAuto= rebind_llcontrol<S32>("EmeraldAutoCorrectCount", &gSavedSettings, true);
	childSetValue("em_ac_stats",*countAuto);
	
	LLSD autoCorrects = listData["data"];
	LLSD::map_const_iterator loc_it = autoCorrects.beginMap();
	LLSD::map_const_iterator loc_end = autoCorrects.endMap();
	for ( ; loc_it != loc_end; ++loc_it)
	{
		const std::string& wrong = (*loc_it).first;
		const std::string& right = (*loc_it).second;

		//std::string lentry(wrong+"=>"+right);

		LLSD element;
		element["id"] = wrong;
		LLSD& s_column = element["columns"][0];
		s_column["column"] = "Search";
		s_column["value"] = wrong;
		s_column["font"] = "SANSSERIF";
		LLSD& r_column = element["columns"][1];
		r_column["column"] = "Replace";
		r_column["value"] = right;
		r_column["font"] = "SANSSERIF";

		entryList->addElement(element, ADD_BOTTOM);
	}
	
}
void lggAutoCorrectFloater::updateNamesList()
{
	namesList->deleteAllItems();
	static BOOL *enabledd = rebind_llcontrol<BOOL>("EmeraldEnableAutoCorrect", &gSavedSettings, true);
	if(!(*enabledd))
	{
		updateItemsList();
		return;
	}
	static S32 *countAuto= rebind_llcontrol<S32>("EmeraldAutoCorrectCount", &gSavedSettings, true);
	childSetValue("em_ac_stats",*countAuto);
	LLSD autoCorrects = LGGAutoCorrect::getInstance()->getAutoCorrects();
	LLSD::map_const_iterator loc_it = autoCorrects.beginMap();
	LLSD::map_const_iterator loc_end = autoCorrects.endMap();
	for ( ; loc_it != loc_end; ++loc_it)
	{
		const std::string& listName = (*loc_it).first;

		LLSD element;
		element["id"] = listName;
		LLSD& friend_column = element["columns"][0];
		friend_column["column"] = "Entries";
		friend_column["value"] = listName;
		friend_column["font"] = "SANSSERIF";
		const LLSD& loc_map = (*loc_it).second;
		if(loc_map["enabled"].asBoolean())
			friend_column["font-style"] = "BOLD";
		else
			friend_column["font-style"] = "NORMAL";
		if(namesList)
		namesList->addElement(element, ADD_BOTTOM);
	}
	updateItemsList();
}
void lggAutoCorrectFloater::updateListControlsEnabled(BOOL selected)
{

		childSetEnabled("em_ac_text1",selected);
		childSetEnabled("em_ac_text2",selected);
		childSetEnabled("em_ac_text_name",selected);
		childSetEnabled("em_ac_text_author",selected);
		childSetEnabled("em_ac_list_enabled",selected);
		childSetEnabled("em_ac_list_show",selected);
		childSetEnabled("em_ac_list_style",selected);
		childSetEnabled("em_ac_deletelist",selected);
		childSetEnabled("em_ac_exportlist",selected);
		childSetEnabled("em_ac_addentry",selected);
		childSetEnabled("em_ac_rementry",selected);
		childSetEnabled("em_ac_priority",selected);
	
}
void lggAutoCorrectFloater::updateEnabledStuff()
{
	static BOOL *enabledd = rebind_llcontrol<BOOL>("EmeraldEnableAutoCorrect", &gSavedSettings, true);
	if(!(*enabledd))
	{
		getChild<LLCheckBoxCtrl>("em_ac_enable")->setEnabledColor(LLColor4(1.0f,0.0f,0.0f,1.0f));		
	}else
	{
		getChild<LLCheckBoxCtrl>("em_ac_enable")->setEnabledColor(LLUI::sColorsGroup->getColor( "LabelTextColor" ));
	}

	childSetEnabled("em_ac_list_name",*enabledd);
	childSetEnabled("em_ac_list_entry",*enabledd);
	updateListControlsEnabled(*enabledd);
	updateNamesList();
	LGGAutoCorrect::getInstance()->save();

}
void lggAutoCorrectFloater::setData(void * data)
{
	empanel = (LLPrefsAdvanced*)data;
}
void lggAutoCorrectFloater::onBoxCommitEnabled(LLUICtrl* caller, void* user_data)
{
	sInstance->updateEnabledStuff();
}
void lggAutoCorrectFloater::onEntrySettingChange(LLUICtrl* caller, void* user_data)
{
	std::string listName= sInstance->namesList->getFirstSelected()->getColumn(0)->getValue().asString();
	LGGAutoCorrect::getInstance()->setListEnabled(listName,sInstance->childGetValue("em_ac_list_enabled").asBoolean());
	LGGAutoCorrect::getInstance()->setListAnnounceeState(listName,sInstance->childGetValue("em_ac_list_show").asBoolean());
	LGGAutoCorrect::getInstance()->setListStyle(listName,sInstance->childGetValue("em_ac_list_style").asBoolean());
	LGGAutoCorrect::getInstance()->setListPriority(listName,sInstance->childGetValue("em_ac_priority").asInteger());
	
	//sInstance->updateEnabledStuff();
	sInstance->updateItemsList();
	LGGAutoCorrect::getInstance()->save();
}
void lggAutoCorrectFloater::deleteEntry(void* data)
{
	std::string listName= sInstance->namesList->getFirstSelected()->getColumn(0)->getValue().asString();

	if((sInstance->entryList->getAllSelected().size())>0)
	{	
		std::string wrong= sInstance->entryList->getFirstSelected()->getColumn(0)->getValue().asString();
       		LGGAutoCorrect::getInstance()->removeEntryFromList(wrong,listName);
		sInstance->updateItemsList();
		LGGAutoCorrect::getInstance()->save();
	}
}
void lggAutoCorrectFloater::loadList(void* data)
{
	LLFilePicker& picker = LLFilePicker::instance();

	if(!picker.getOpenFile( LLFilePicker::FFLOAD_XML) )
	{return;
	}	
	llifstream file;
	file.open(picker.getFirstFile().c_str());
	LLSD blankllsd;
	if (file.is_open())
	{
		LLSDSerialize::fromXMLDocument(blankllsd, file);
	}
	file.close();
	gSavedSettings.setBOOL("EmeraldEnableAutoCorrect",true);
	LGGAutoCorrect::getInstance()->addCorrectionList(blankllsd);
	sInstance->updateEnabledStuff();
}
void lggAutoCorrectFloater::removeList(void* data)
{
	std::string listName= sInstance->namesList->getFirstSelected()->getColumn(0)->getValue().asString();
	LGGAutoCorrect::getInstance()->removeCorrectionList(listName);
	sInstance->updateEnabledStuff();

}
void lggAutoCorrectFloater::exportList(void *data)
{
	std::string listName= sInstance->namesList->getFirstSelected()->getColumn(0)->getValue().asString();

	LLFilePicker& picker = LLFilePicker::instance();

	if(!picker.getSaveFile( LLFilePicker::FFSAVE_XML) )
	{return;
	}	
	llofstream file;
	file.open(picker.getFirstFile().c_str());
	LLSDSerialize::toPrettyXML(LGGAutoCorrect::getInstance()->exportList(listName), file);
	file.close();	
}
void lggAutoCorrectFloater::addEntry(void* data)
{
	std::string listName= sInstance->namesList->getFirstSelected()->getColumn(0)->getValue().asString();
	LLChat chat;
	chat.mText = "To add an entry, please type in chat \""+gSavedSettings.getString("CmdLineChatbarAutocorrect")+" "+listName+"|wrongWord|rightWord\"";
	chat.mSourceType = CHAT_SOURCE_SYSTEM;
	LLFloaterChat::addChat(chat, FALSE, FALSE);
}

void lggAutoCorrectFloaterStart::show(BOOL showin,void * data)
{
	if(showin)
	{
		lggAutoCorrectFloater* dic_floater = lggAutoCorrectFloater::showInstance();
		dic_floater->setData(data);
	}
}
