/**
* @file floaterbusy.cpp
* @brief Custom busy mode settings for Imprudence
*
* $LicenseInfo:firstyear=2009&license=viewergpl$
*
* Copyright (c) 2010, McCabe Maxsted
*
* Imprudence Viewer Source Code
* The source code in this file ("Source Code") is provided to you
* under the terms of the GNU General Public License, version 2.0
* ("GPL"). Terms of the GPL can be found in doc/GPL-license.txt in
* this distribution, or online at
* http://secondlifegrid.net/programs/open_source/licensing/gplv2
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
* ALL SOURCE CODE IS PROVIDED "AS IS." THE AUTHOR MAKES NO
* WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
* COMPLETENESS OR PERFORMANCE.
* $/LicenseInfo$
*/

#include "llviewerprecompiledheaders.h"

#include "floaterbusy.h"

#include "llinventorymodel.h"
#include "llstartup.h"
#include "lltexteditor.h"
#include "lluictrlfactory.h"
#include "llviewercontrol.h"
#include "llviewerinventory.h"

////////begin drop utility/////////////
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Class InvDropTarget
//
// This handy class is a simple way to drop something on another
// view. It handles drop events, always setting itself to the size of
// its parent.
//
// altered to support a callback so i can slap it in things and it just return the item to a func of my choice
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

InvDropTarget::InvDropTarget(const std::string& name, const LLRect& rect,
						  void (*callback)(LLViewerInventoryItem*)) :
	LLView(name, rect, NOT_MOUSE_OPAQUE, FOLLOWS_ALL),
	mDownCallback(callback)
{
}

InvDropTarget::~InvDropTarget()
{
}

void InvDropTarget::doDrop(EDragAndDropType cargo_type, void* cargo_data)
{
	llinfos << "InvDropTarget::doDrop()" << llendl;
}

BOOL InvDropTarget::handleDragAndDrop(S32 x, S32 y, MASK mask, BOOL drop,
									 EDragAndDropType cargo_type,
									 void* cargo_data,
									 EAcceptance* accept,
									 std::string& tooltip_msg)
{
	BOOL handled = FALSE;
	if (getParent())
	{
		handled = TRUE;
		// check the type
		//switch(cargo_type)
		//{
		//case DAD_ANIMATION:
		//{
			LLViewerInventoryItem* inv_item = (LLViewerInventoryItem*)cargo_data;
			if (gInventory.getItem(inv_item->getUUID()))
			{
				*accept = ACCEPT_YES_COPY_SINGLE;
				if (drop)
				{
					//printchat("accepted");
					mDownCallback(inv_item);
				}
			}
			else
			{
				*accept = ACCEPT_NO;
			}
		//	break;
		//}
		//default:
		//	*accept = ACCEPT_NO;
		//	break;
		//}
	}
	return handled;
}
////////end drop utility///////////////

// static 
InvDropTarget * FloaterBusy::mObjectDropTarget;

FloaterBusy::FloaterBusy(const LLSD& seed) : LLFloater("floater_busy")
{
	LLUICtrlFactory::getInstance()->buildFloater(this, "floater_busy.xml");
}

BOOL FloaterBusy::postBuild()
{
	childSetAction("btn_ok", onClickOK, this);
	childSetAction("btn_cancel", onClickCancel, this);

	LLView *target_view = getChild<LLView>("im_give_drop_target_rect");
	if (target_view)
	{
		if (mObjectDropTarget)//shouldn't happen
		{
			delete mObjectDropTarget;
		}
		mObjectDropTarget = new InvDropTarget("drop target", target_view->getRect(), IMAutoResponseItemDrop);//, mAvatarID);
		addChild(mObjectDropTarget);
	}

	if (LLStartUp::getStartupState() == STATE_STARTED)
	{
		LLUUID itemid = (LLUUID)gSavedPerAccountSettings.getString("InstantMessageResponseItemData");
		LLViewerInventoryItem* item = gInventory.getItem(itemid);

		if (item)
		{
			childSetValue("im_give_disp_rect_txt","Currently set to: "+item->getName());
		}
		else if (itemid.isNull())
		{
			childSetValue("im_give_disp_rect_txt","Currently not set");
		}
		else
		{
			childSetValue("im_give_disp_rect_txt","Currently set to a item not on this account");
		}
	}
	else
	{
		childSetValue("im_give_disp_rect_txt","Not logged in");
	}

	LLWString auto_response = utf8str_to_wstring( gSavedPerAccountSettings.getString("InstantMessageResponse") );
	LLWStringUtil::replaceChar(auto_response, '^', '\n');
	LLWStringUtil::replaceChar(auto_response, '%', ' ');
	childSetText("im_response", wstring_to_utf8str(auto_response));

	childSetValue("InstantMessageResponseFriends", gSavedPerAccountSettings.getBOOL("InstantMessageResponseFriends"));
	childSetValue("InstantMessageResponseMuted", gSavedPerAccountSettings.getBOOL("InstantMessageResponseMuted"));
	childSetValue("InstantMessageResponseAnyone", gSavedPerAccountSettings.getBOOL("InstantMessageResponseAnyone"));
	childSetValue("InstantMessageShowResponded", gSavedPerAccountSettings.getBOOL("InstantMessageShowResponded"));
	childSetValue("InstantMessageShowOnTyping", gSavedPerAccountSettings.getBOOL("InstantMessageShowOnTyping"));
	childSetValue("InstantMessageResponseRepeat", gSavedPerAccountSettings.getBOOL("InstantMessageResponseRepeat" ));
	childSetValue("InstantMessageResponseItem", gSavedPerAccountSettings.getBOOL("InstantMessageResponseItem"));
	childSetValue("InstantMessageAnnounceIncoming", gSavedPerAccountSettings.getBOOL("InstantMessageAnnounceIncoming"));
	childSetValue("InstantMessageAnnounceStealFocus", gSavedPerAccountSettings.getBOOL("InstantMessageAnnounceStealFocus"));

	return TRUE;
}

FloaterBusy::~FloaterBusy()
{
	delete mObjectDropTarget;
	mObjectDropTarget = NULL;
}

void FloaterBusy::IMAutoResponseItemDrop(LLViewerInventoryItem* item)
{
	gSavedPerAccountSettings.setString("InstantMessageResponseItemData", item->getUUID().asString());
	FloaterBusy::getInstance()->childSetValue("im_give_disp_rect_txt","Currently set to: "+item->getName());
}

// static
void FloaterBusy::onClickOK(void* userdata)
{
	FloaterBusy* self = (FloaterBusy*)userdata;
	self->apply();
	self->close();
}

// static
void FloaterBusy::onClickCancel(void* userdata)
{
	FloaterBusy* self = (FloaterBusy*)userdata;
	self->cancel();
}

void FloaterBusy::cancel()
{
	close();
}

void FloaterBusy::apply()
{
	LLTextEditor* im = getChild<LLTextEditor>("im_response");
	LLWString im_response;
	if (im) im_response = im->getWText(); 
	LLWStringUtil::replaceTabsWithSpaces(im_response, 4);
	LLWStringUtil::replaceChar(im_response, '\n', '^');
	LLWStringUtil::replaceChar(im_response, ' ', '%');
	gSavedPerAccountSettings.setString("InstantMessageResponse", std::string(wstring_to_utf8str(im_response)));
	gSavedPerAccountSettings.setBOOL("InstantMessageResponseMuted", childGetValue("InstantMessageResponseMuted").asBoolean());
	gSavedPerAccountSettings.setBOOL("InstantMessageResponseFriends", childGetValue("InstantMessageResponseFriends").asBoolean());
	gSavedPerAccountSettings.setBOOL("InstantMessageResponseMuted", childGetValue("InstantMessageResponseMuted").asBoolean());
	gSavedPerAccountSettings.setBOOL("InstantMessageResponseAnyone", childGetValue("InstantMessageResponseAnyone").asBoolean());
	gSavedPerAccountSettings.setBOOL("InstantMessageShowResponded", childGetValue("InstantMessageShowResponded").asBoolean());
	gSavedPerAccountSettings.setBOOL("InstantMessageShowOnTyping", childGetValue("InstantMessageShowOnTyping").asBoolean());
	gSavedPerAccountSettings.setBOOL("InstantMessageResponseRepeat", childGetValue("InstantMessageResponseRepeat").asBoolean());
	gSavedPerAccountSettings.setBOOL("InstantMessageResponseItem", childGetValue("InstantMessageResponseItem").asBoolean());
	gSavedPerAccountSettings.setBOOL("InstantMessageAnnounceIncoming", childGetValue("InstantMessageAnnounceIncoming").asBoolean());
	gSavedPerAccountSettings.setBOOL("InstantMessageAnnounceStealFocus", childGetValue("InstantMessageAnnounceStealFocus").asBoolean());
}
