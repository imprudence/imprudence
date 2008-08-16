/** 
 * @file llgroupnotify.cpp
 * @brief Non-blocking notification that doesn't take keyboard focus.
 *
 * Copyright (c) 2006-2007, Linden Research, Inc.
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

#include "llgroupnotify.h"

#include "llfocusmgr.h"

#include "llbutton.h"
#include "lliconctrl.h"
#include "llfloaterchat.h"	// for add_chat_history()
#include "llnotify.h"
#include "lltextbox.h"
#include "llviewertexteditor.h"
#include "lluiconstants.h"
#include "llui.h"
#include "llviewercontrol.h"
#include "llfloatergroupinfo.h"
#include "llinventoryview.h"
#include "llinventory.h"

#include "llglheaders.h"
#include "llagent.h"
// Globals
//LLView* gGroupNotifyBoxView = NULL;

const F32 ANIMATION_TIME = 0.333f;

S32 LLGroupNotifyBox::sGroupNotifyBoxCount = 0;

//---------------------------------------------------------------------------
// LLGroupNotifyBox
//---------------------------------------------------------------------------

char* g_formatted_time(const time_t& the_time, char* buffer)
{
	time_t t = the_time;
	if (!t) time(&t);
	LLString::copy(buffer, ctime(&t), 30);
	buffer[24] = '\0';
	return buffer;
}

// static
LLGroupNotifyBox* LLGroupNotifyBox::show(const char* subject,
							 const char* message,
							 const char* from_name,
							 const LLUUID& group_id,
							 const U32& t,
							 const bool& has_inventory,
							 const char* inventory_name,
							 LLOfferInfo* inventory_offer)
{
	// Get the group data
	LLGroupData group_data;
	if (!gAgent.getGroupData(group_id,group_data))
	{
		llwarns << "Group notice for unkown group: " << group_id << llendl;
		return NULL;
	}

	LLGroupNotifyBox* self;
	self = new LLGroupNotifyBox(subject, message, from_name, group_id, group_data.mInsigniaID, group_data.mName.c_str(),t,has_inventory,inventory_name,inventory_offer);
	gNotifyBoxView->addChild(self);

	// TODO: play a sound
	return self;
}

bool is_openable(LLAssetType::EType type)
{
	switch(type)
	{
	case LLAssetType::AT_LANDMARK:
	case LLAssetType::AT_NOTECARD:
	case LLAssetType::AT_IMAGE_JPEG:
	case LLAssetType::AT_IMAGE_TGA:
	case LLAssetType::AT_TEXTURE:
	case LLAssetType::AT_TEXTURE_TGA:
		return true;
	default:
		return false;
	}
}

LLGroupNotifyBox::LLGroupNotifyBox(const char* subject,
							 const char* message,
							 const char* from_name,
							 const LLUUID& group_id,
							 const LLUUID& group_insignia,
							 const char* group_name,
							 const U32& t,
							 const bool& has_inventory,
							 const char* inventory_name,
							 LLOfferInfo* inventory_offer)
:	LLPanel("groupnotify", LLGroupNotifyBox::getGroupNotifyRect(), BORDER_YES),
	mAnimating(TRUE),
	mTimer(),
	mGroupID(group_id),
	mHasInventory(has_inventory),
	mInventoryOffer(inventory_offer)
{
	mIsFocusRoot = TRUE;

	time_t timestamp = (time_t)t;

	char time_buf[30];		/*Flawfinder: ignore*/	
	g_formatted_time(timestamp, time_buf);

	setFollows(FOLLOWS_TOP|FOLLOWS_RIGHT);
	setBackgroundVisible(TRUE);
	setBackgroundOpaque(TRUE);

	// TODO: add a color for group notices
	setBackgroundColor( gColors.getColor("GroupNotifyBoxColor") );

	LLIconCtrl* icon;
	LLTextEditor* text;

	const S32 VPAD = 2;
	const S32 TOP = mRect.getHeight() - 32; // Get past the top menu bar
	const S32 BOTTOM_PAD = VPAD * 2;
	const S32 BTN_TOP = BOTTOM_PAD + BTN_HEIGHT + VPAD;
	const S32 RIGHT = mRect.getWidth() - HPAD - HPAD;
	const S32 LINE_HEIGHT = 16;

	const S32 LABEL_WIDTH = 64;
	const S32 ICON_WIDTH = 64;

	S32 y = TOP;
	S32 x = HPAD + HPAD;


	// Title
	LLTextBox* line;

	line = new LLTextBox("title",LLRect(x,y,RIGHT - HPAD,y - LINE_HEIGHT),"Group Notice",LLFontGL::sSansSerifHuge);
	line->setHAlign(LLFontGL::RIGHT);
	line->setFontStyle(LLFontGL::DROP_SHADOW_SOFT);
	line->setBorderVisible(FALSE);
	line->setColor(LLColor4::white);
	line->setBackgroundColor( gColors.getColor("GroupNotifyBoxColor") );
	
	addChild(line);

	y -= llfloor(1.5f*LINE_HEIGHT);

	x += HPAD + HPAD + ICON_WIDTH;

	std::stringstream from;
	from << "Sent by " << from_name << ", " << group_name;

	line = new LLTextBox("group",LLRect(x,y,RIGHT - HPAD,y - LINE_HEIGHT),from.str().c_str(),LLFontGL::sSansSerif);
	line->setFontStyle(LLFontGL::DROP_SHADOW_SOFT);
	line->setHAlign(LLFontGL::RIGHT);
	line->setBorderVisible(FALSE);
	line->setColor(LLColor4::white);
	line->setBackgroundColor( gColors.getColor("GroupNotifyBoxColor") );
	addChild(line);
	
	y -= (LINE_HEIGHT + VPAD);

	LLUUID icon_id(gViewerArt.getString("notify_box_icon.tga"));
	// TODO: change this to be the group icon.
	if (!group_insignia.isNull())
	{
		icon_id = group_insignia;
	}

	x = HPAD + HPAD;

	icon = new LLIconCtrl("icon",
						  LLRect(x, y, x+ICON_WIDTH, y-ICON_WIDTH),
						  icon_id);
	icon->setMouseOpaque(FALSE);
	addChild(icon);

	x += HPAD + HPAD + ICON_WIDTH;
	// If we have inventory with this message, leave room for the name.
	S32 box_bottom = BTN_TOP + (mHasInventory ? (LINE_HEIGHT + 2*VPAD) : 0);

	text = new LLViewerTextEditor("box",
		LLRect(x, y, RIGHT, box_bottom),
		DB_GROUP_NOTICE_MSG_STR_LEN,
		"",
		LLFontGL::sSansSerif,
		FALSE);

	LLStyle headerstyle(true,LLColor4::black,"SansSerifBig");
	LLStyle datestyle(true,LLColor4::black,"serif");

	text->appendStyledText(subject,false,false,&headerstyle);
	text->appendStyledText(time_buf,false,false,&datestyle);
	// Sadly, our LLTextEditor can't handle both styled and unstyled text
	// at the same time.  Hence this space must be styled. JC
	text->appendColoredText(" ",false,false,LLColor4::grey4);
	text->appendColoredText(message,false,false,LLColor4::grey4);

	LLColor4 semi_transparent(1.0f,1.0f,1.0f,0.8f);
	text->setCursor(0,0);
	text->setEnabled(FALSE);
	text->setWordWrap(TRUE);
	text->setTakesFocus(FALSE);
	text->setTabToNextField(TRUE);
	text->setMouseOpaque(TRUE);
	text->setBorderVisible(TRUE);
	text->setTakesNonScrollClicks(TRUE);
	text->setHideScrollbarForShortDocs(TRUE);
	text->setReadOnlyBgColor ( semi_transparent );
	text->setWriteableBgColor ( semi_transparent );
	
	addChild(text);

	y = box_bottom - VPAD;

	if (mHasInventory)
	{
			line = new LLTextBox("subjecttitle",LLRect(x,y,x + LABEL_WIDTH,y - LINE_HEIGHT),"Attached: ",LLFontGL::sSansSerif);
			line->setBorderVisible(FALSE);
			line->setColor(LLColor4::white);
			line->setFontStyle(LLFontGL::DROP_SHADOW_SOFT);
			line->setBackgroundColor( gColors.getColor("GroupNotifyBoxColor") );
			addChild(line);

			LLViewerImage* item_icon = get_item_icon(mInventoryOffer->mType,
													LLInventoryType::IT_TEXTURE,
													0, FALSE);


			x += LABEL_WIDTH + HPAD;

			std::stringstream ss;
			ss << "        " << inventory_name;
			line = new LLTextBox("object_name",LLRect(x,y,RIGHT - HPAD,y - LINE_HEIGHT),ss.str().c_str(),LLFontGL::sSansSerif);
			line->setEnabled(FALSE);
			line->setBorderVisible(TRUE);
			line->setDisabledColor(LLColor4::blue4);
			line->setFontStyle(LLFontGL::NORMAL);
			line->setBackgroundVisible(true);
			line->setBackgroundColor( semi_transparent );
			addChild(line);

			icon = new LLIconCtrl("icon",
									LLRect(x, y, x+16, y-16),
									item_icon->getID());
			icon->setMouseOpaque(FALSE);
			addChild(icon);
	}

	LLButton* btn;
	btn = new LLButton("next",
				LLRect(mRect.getWidth()-24, BOTTOM_PAD+16, mRect.getWidth()-8, BOTTOM_PAD),
				"notify_next.tga",
				"notify_next.tga",
				"",
				onClickNext,
				this,
				LLFontGL::sSansSerif);
	btn->setToolTip("Next");
	addChild(btn);
	mNextBtn = btn;

	S32 btn_width = 80;
	S32 wide_btn_width = 120;
	LLRect btn_rect;
	x = 3 * HPAD;

	btn_rect.setOriginAndSize(x,
								BOTTOM_PAD,
								btn_width,
								BTN_HEIGHT);

	btn = new LLButton("OK", btn_rect, "", onClickOk, this);
	addChild(btn, -1);
	setDefaultBtn(btn);

	x += btn_width + HPAD;

	
	btn_rect.setOriginAndSize(x,
								BOTTOM_PAD,
								wide_btn_width,
								BTN_HEIGHT);

	btn = new LLButton("Group Notices", btn_rect, "", onClickGroupInfo, this);
	btn->setToolTip("View past notices or opt-out of receiving these messages here.");
	addChild(btn, -1);

	if (mHasInventory && mInventoryOffer)
	{
		x += wide_btn_width + HPAD;

		btn_rect.setOriginAndSize(x,
									BOTTOM_PAD,
									wide_btn_width,
									BTN_HEIGHT);

		LLString btn_lbl("");
		if(is_openable(mInventoryOffer->mType))
		{
			btn_lbl = "Open Attachment";
		}
		else
		{
			btn_lbl = "Save Attachment";
		}
		mSaveInventoryBtn = new LLButton(btn_lbl, btn_rect, "", onClickSaveInventory, this);
		mSaveInventoryBtn->setVisible(mHasInventory);
		addChild(mSaveInventoryBtn);
	}

	sGroupNotifyBoxCount++;

	// If this is the only notify box, don't show the next button
	if (sGroupNotifyBoxCount == 1)
	{
		mNextBtn->setVisible(FALSE);
	}
}


// virtual
LLGroupNotifyBox::~LLGroupNotifyBox()
{
	sGroupNotifyBoxCount--;
}

// virtual
BOOL LLGroupNotifyBox::handleRightMouseDown(S32 x, S32 y, MASK mask)
{
	if (getVisible() && getEnabled() && pointInView(x,y))
	{
		moveToBack();
		return TRUE;
	}

	return LLPanel::handleRightMouseDown(x, y, mask);
}


// virtual
void LLGroupNotifyBox::draw()
{
	F32 display_time = mTimer.getElapsedTimeF32();

	if (mAnimating && display_time < ANIMATION_TIME)
	{
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();

		S32 height = mRect.getHeight();
		F32 fraction = display_time / ANIMATION_TIME;
		F32 voffset = (1.f - fraction) * height;

		glTranslatef(0.f, voffset, 0.f);

		LLPanel::draw();

		glPopMatrix();
	}
	else
	{
		mAnimating = FALSE;
		LLPanel::draw();
	}
}


void LLGroupNotifyBox::close()
{
	// The group notice dialog may be an inventory offer.
	// If it has an inventory save button and that button is still enabled
	// Then we need to send the inventory declined message
	if(mHasInventory)
	{
		inventory_offer_callback(IOR_DECLINE , mInventoryOffer); 
	}
	gNotifyBoxView->removeChild(this);

	die();
}


void LLGroupNotifyBox::moveToBack()
{
	// Move this dialog to the back.
	gNotifyBoxView->removeChild(this);
	gNotifyBoxView->addChildAtEnd(this);
	mNextBtn->setVisible(FALSE);

	// And enable the next button on the frontmost one, if there is one
	if (sGroupNotifyBoxCount > 1)
	{
		LLView* view = gNotifyBoxView->getFirstChild();
		LLGroupNotifyBox* front = (LLGroupNotifyBox*)view;
		front->mNextBtn->setVisible(TRUE);
	}
}


// static
LLRect LLGroupNotifyBox::getGroupNotifyRect()
{
	S32 notify_height = gSavedSettings.getS32("GroupNotifyBoxHeight");
	const S32 NOTIFY_WIDTH = gSavedSettings.getS32("GroupNotifyBoxWidth");

	const S32 TOP = gNotifyBoxView->getRect().getHeight();
	const S32 RIGHT = gNotifyBoxView->getRect().getWidth();
	const S32 LEFT = RIGHT - NOTIFY_WIDTH;

	return LLRect(LEFT, TOP, RIGHT, TOP-notify_height);
}


// static
void LLGroupNotifyBox::onClickOk(void* data)
{
	LLGroupNotifyBox* self = (LLGroupNotifyBox*)data;
	if (self) self->close();
}

void LLGroupNotifyBox::onClickGroupInfo(void* data)
{
	LLGroupNotifyBox* self = (LLGroupNotifyBox*)data;

	if (self)
	{
		LLFloaterGroupInfo::showFromUUID(self->mGroupID, "notices_tab");
	}

	//Leave notice open until explicitly closed
}

void LLGroupNotifyBox::onClickSaveInventory(void* data)
{
	LLGroupNotifyBox* self = (LLGroupNotifyBox*)data;

	inventory_offer_callback( IOR_ACCEPT , self->mInventoryOffer); 

	// inventory_offer_callback will delete the offer, so make sure we aren't still pointing to it.
	self->mInventoryOffer = NULL;
	// Each item can only be received once, so disable the button.
	self->mSaveInventoryBtn->setEnabled(FALSE);
}

// static
void LLGroupNotifyBox::onClickNext(void* data)
{
	LLGroupNotifyBox* self = (LLGroupNotifyBox*)data;
	self->moveToBack();
}
