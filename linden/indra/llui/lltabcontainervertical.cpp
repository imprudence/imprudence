/** 
 * @file lltabcontainervertical.cpp
 * @brief LLTabContainerVertical base class
 *
 * Copyright (c) 2001-2007, Linden Research, Inc.
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

// Fear my script-fu!

#include "linden_common.h"

#include "lltabcontainervertical.h"

#include "llfocusmgr.h"
#include "llfontgl.h"
#include "llgl.h"

#include "llbutton.h"
#include "llrect.h"
#include "llpanel.h"
#include "llresmgr.h"
#include "llkeyboard.h"
#include "llui.h"
#include "lltextbox.h"
#include "llcontrol.h"
#include "llcriticaldamp.h"

#include "llglheaders.h"

LLTabContainerVertical::LLTabContainerVertical( 
	const LLString& name, const LLRect& rect,
	void(*close_callback)(void*), void* callback_userdata,
	U32 tab_width, BOOL bordered)
	: 
	LLTabContainerCommon(name, rect, LEFT, close_callback, callback_userdata, bordered),
	mTabWidth(tab_width),
	mUpArrowBtn(NULL),
	mDownArrowBtn(NULL)
{
	initButtons();
}

LLTabContainerVertical::LLTabContainerVertical( 
	const LLString& name, const LLString& rect_control,
	void(*close_callback)(void*), void* callback_userdata,
	U32 tab_width, BOOL bordered)
	: 
	LLTabContainerCommon(name, rect_control, LEFT, close_callback, callback_userdata, bordered),
	mTabWidth(tab_width)
{
	initButtons();
}

// Called from all constructors
void LLTabContainerVertical::initButtons()
{
	// Hack:
	if (mRect.getHeight() == 0 || mUpArrowBtn)
	{
		return; // Don't have a rect yet or already got called
	}
	
	LLString out_id;
	LLString in_id;

	//S32 arrow_fudge = 1;		//  match new art better 

	// Left and right scroll arrows (for when there are too many tabs to show all at once).
	S32 btn_top = mRect.getHeight();
	S32 btn_top_lower = mRect.mBottom+TABCNTRV_ARROW_BTN_SIZE;

	LLRect up_arrow_btn_rect;
	up_arrow_btn_rect.setLeftTopAndSize( mTabWidth/2 , btn_top, TABCNTRV_ARROW_BTN_SIZE, TABCNTRV_ARROW_BTN_SIZE );

	LLRect down_arrow_btn_rect;
	down_arrow_btn_rect.setLeftTopAndSize( mTabWidth/2 , btn_top_lower, TABCNTRV_ARROW_BTN_SIZE, TABCNTRV_ARROW_BTN_SIZE );

	out_id = "UIImgBtnScrollUpOutUUID";
	in_id = "UIImgBtnScrollUpInUUID";
	mUpArrowBtn = new LLButton(
		"Up Arrow", up_arrow_btn_rect,
		out_id, in_id, "",
		&onPrevBtn, this, NULL );
	mUpArrowBtn->setHeldDownCallback(onPrevBtnHeld);
	mUpArrowBtn->setSaveToXML(false);
	mUpArrowBtn->setFollowsTop();
	mUpArrowBtn->setFollowsLeft();
	mUpArrowBtn->setTabStop(FALSE);
	addChild(mUpArrowBtn);

	out_id = "UIImgBtnScrollDownOutUUID";
	in_id = "UIImgBtnScrollDownInUUID";
	mDownArrowBtn = new LLButton(
		"Down Arrow", down_arrow_btn_rect,
		out_id, in_id, "",
		&onNextBtn, this, NULL );
	mDownArrowBtn->setHeldDownCallback(onNextBtnHeld);
	mDownArrowBtn->setSaveToXML(false);
	mDownArrowBtn->setFollowsBottom();
	mDownArrowBtn->setFollowsLeft();
	mDownArrowBtn->setTabStop(FALSE);
	addChild(mDownArrowBtn);

	// set default tab group to be panel contents
	mDefaultTabGroup = 1;
}

LLTabContainerVertical::~LLTabContainerVertical()
{ }

void LLTabContainerVertical::addTabPanel(LLPanel* child, const LLString& label,
										BOOL select, 
										void (*on_tab_clicked)(void*, bool), void* userdata, 
										S32 indent,
										BOOL placeholder, eInsertionPoint insertion_point)
{
	if (child->getParent() == this)
	{
		// already a child of mine
		return;
	}

	const LLFontGL* font = gResMgr->getRes( LLFONT_SANSSERIF );

	// Store the original label for possible xml export.
	child->setLabel(label);
	// Replace long label with truncated version (e.g., "FooBa...")
	LLString trimmed_label = label;
	LLString::trim(trimmed_label);

	// Tab panel
	S32 tab_panel_top;
	S32 tab_panel_bottom;
	tab_panel_top = mRect.getHeight() 
					- mTopBorderHeight 
					- (BTN_HEIGHT - TABCNTRV_BUTTON_PANEL_OVERLAP);
	tab_panel_bottom = LLPANEL_BORDER_WIDTH;
	
	LLRect tab_panel_rect( 
		mTabWidth + (LLPANEL_BORDER_WIDTH * 2) + TABCNTRV_PAD, 
		mRect.getHeight() - LLPANEL_BORDER_WIDTH,
		mRect.getWidth() - LLPANEL_BORDER_WIDTH,
		LLPANEL_BORDER_WIDTH);

	child->setFollowsAll();
	child->translate( tab_panel_rect.mLeft - child->getRect().mLeft, tab_panel_rect.mBottom - child->getRect().mBottom);
	child->reshape( tab_panel_rect.getWidth(), tab_panel_rect.getHeight(), TRUE );
	child->setBackgroundVisible( FALSE );  // No need to overdraw

	child->setVisible( FALSE );  // Will be made visible when selected

	// Tab button
	LLRect btn_rect;
	btn_rect.setLeftTopAndSize(
		TABCNTRV_PAD + LLPANEL_BORDER_WIDTH + 2,	// JC - Fudge factor
		(mRect.getHeight() - mTopBorderHeight - LLPANEL_BORDER_WIDTH - 1) - ((BTN_HEIGHT + TABCNTRV_PAD) * mTabList.size()),
		mTabWidth,
		BTN_HEIGHT);

	if (!placeholder)
	{
		LLButton *btn = new LLButton("vert tab button",
			btn_rect,
			"tab_left.tga",
			"tab_left_selected.tga", 
			"", 
			&LLTabContainerVertical::onTabBtn, NULL,
			font,
			trimmed_label, trimmed_label);
		btn->setSaveToXML(false);
		btn->setFixedBorder(16, 16);
		btn->setScaleImage(TRUE);
		btn->setHAlign(LLFontGL::LEFT);
		btn->setFollows(FOLLOWS_TOP | FOLLOWS_LEFT);
		btn->setTabStop(FALSE);
		if (indent)
		{
			btn->setLeftHPad(indent);
		}

		LLTabTuple* tuple = new LLTabTuple( this, child, btn, on_tab_clicked, userdata );
		insertTuple( tuple, insertion_point ); 

		btn->setCallbackUserData( tuple );
		addChild( btn, 0 );
		addChild(child, 1);

		if( select )
		{
			selectTab( mTabList.size()-1 );
		}
	}
	else
	{
		btn_rect.translate(0, -LLBUTTON_V_PAD-2);
		LLString box_label = trimmed_label;
		LLTextBox* text = new LLTextBox(box_label, btn_rect, box_label, font);
		text->setSaveToXML(false);
		addChild( text, 0 );

		LLButton* btn = new LLButton("", LLRect(0,0,0,0));
		btn->setSaveToXML(false);
		addChild(btn, 0);
		addChild(child, 1);

		LLTabTuple* tuple = new LLTabTuple( this, child, btn, on_tab_clicked, userdata, text );
		insertTuple( tuple, insertion_point ); 
	}

	updateMaxScrollPos();
}

void LLTabContainerVertical::removeTabPanel(LLPanel* child)
{
	LLTabContainerCommon::removeTabPanel(child);

	// Fix-up button sizes
	S32 tab_count = 0;
	for(std::vector<LLTabTuple*>::iterator iter = mTabList.begin(); iter != mTabList.end(); ++iter)
	{
		LLTabTuple* tuple = *iter;
		LLRect rect;
		rect.setLeftTopAndSize(TABCNTRV_PAD + LLPANEL_BORDER_WIDTH + 2,	// JC - Fudge factor
							   (mRect.getHeight() - LLPANEL_BORDER_WIDTH - 1) - ((BTN_HEIGHT + TABCNTRV_PAD) * (tab_count)),
							   mTabWidth,
							   BTN_HEIGHT);
		if (tuple->mPlaceholderText)
		{
			tuple->mPlaceholderText->setRect(rect);
		}
		else
		{
			tuple->mButton->setRect(rect);
		}
		tab_count++;
	}
}

void LLTabContainerVertical::updateMaxScrollPos()
{
	S32 tab_total_height = (BTN_HEIGHT + TABCNTRV_PAD) * mTabList.size();
	S32 available_height = mRect.getHeight() - mTopBorderHeight;
	if( tab_total_height > available_height )
	{
		S32 available_height_with_arrows = mRect.getHeight() - 2*(TABCNTRV_ARROW_BTN_SIZE + 3*TABCNTRV_PAD);
		S32 additional_needed = tab_total_height - available_height_with_arrows;
		mMaxScrollPos = S32( ceil(additional_needed / float(BTN_HEIGHT) ) );
	}
	else
	{
		mMaxScrollPos = 0;
		mScrollPos = 0;
	}
	if (mScrollPos > mMaxScrollPos)
	{
		mScrollPos = mMaxScrollPos;
	}
}

void LLTabContainerVertical::commitHoveredButton(S32 x, S32 y)
{
	if (hasMouseCapture())
	{
		for(std::vector<LLTabTuple*>::iterator iter = mTabList.begin(); iter != mTabList.end(); ++iter)
		{
			LLTabTuple* tuple = *iter;
			tuple->mButton->setVisible( TRUE );
			S32 local_x = x - tuple->mButton->getRect().mLeft;
			S32 local_y = y - tuple->mButton->getRect().mBottom;
			if (tuple->mButton->pointInView(local_x, local_y) && tuple->mButton->getEnabled() && !tuple->mTabPanel->getVisible())
			{
				tuple->mButton->onCommit();
			}
		}
	}
}

BOOL LLTabContainerVertical::selectTab(S32 which)
{
	if (which >= (S32)mTabList.size()) return FALSE;
	if (which < 0) return FALSE;

	//if( gFocusMgr.childHasKeyboardFocus( this ) )
	//{
	//	gFocusMgr.setKeyboardFocus( NULL, NULL );
	//}

	LLTabTuple* selected_tuple = mTabList[which];
	if (!selected_tuple)
	{
		return FALSE;
	}
	
	BOOL is_visible = FALSE;
	if (which != mCurrentTabIdx)
	{
		mCurrentTabIdx = which;

		S32 i = 0;
		for(tuple_list_t::iterator iter = mTabList.begin(); iter != mTabList.end(); ++iter)
		{
			LLTabTuple* tuple = *iter;
			BOOL is_selected = ( tuple == selected_tuple );
			tuple->mTabPanel->setVisible( is_selected );
// 			tuple->mTabPanel->setFocus(is_selected); // not clear that we want to do this here.
			tuple->mButton->setToggleState( is_selected );
			// RN: this limits tab-stops to active button only, which would require arrow keys to switch tabs
			tuple->mButton->setTabStop( is_selected );
			
			if( is_selected )
			{
				// Make sure tab is within scroll
				S32 num_visible = mTabList.size() - mMaxScrollPos;
				if( i >= mScrollPos && i <= mScrollPos + num_visible)
				{
					mCurrentTabIdx = which;
					is_visible = TRUE;
				}
				else
				{
					is_visible = FALSE;
				}
			}
			i++;
		}
		if( selected_tuple->mOnChangeCallback )
		{
			selected_tuple->mOnChangeCallback( selected_tuple->mUserData, false );
		}
	}
	if(mCurrentTabIdx >= 0)
	{
		LLTabTuple* tuple = mTabList[mCurrentTabIdx];
		tuple->mTabPanel->setVisible( TRUE );
		tuple->mButton->setToggleState( TRUE );
	}
	return is_visible;
}



void LLTabContainerVertical::draw()
{
	S32 target_pixel_scroll = mScrollPos * (BTN_HEIGHT + TABCNTRV_PAD);

	mScrollPosPixels = (S32)lerp((F32)mScrollPosPixels, (F32)target_pixel_scroll, LLCriticalDamp::getInterpolant(0.08f));
	if( getVisible() )
	{
		BOOL has_scroll_arrows = (mMaxScrollPos > 0) || (mScrollPosPixels > 0);
		mUpArrowBtn->setVisible( has_scroll_arrows );
		mDownArrowBtn->setVisible( has_scroll_arrows );

		// Set the topmost position of the tab buttons.
		S32 top = mRect.getHeight() - mTopBorderHeight - LLPANEL_BORDER_WIDTH - 1 - (has_scroll_arrows ? TABCNTRV_ARROW_BTN_SIZE : 0);
		top += mScrollPosPixels;

		// Hide all the buttons
		for(std::vector<LLTabTuple*>::iterator iter = mTabList.begin(); iter != mTabList.end(); ++iter)
		{
			LLTabTuple* tuple = *iter;
			tuple->mButton->setVisible( FALSE );
		}

		LLPanel::draw();

		// Show all the buttons
		for(std::vector<LLTabTuple*>::iterator iter = mTabList.begin(); iter != mTabList.end(); ++iter)
		{
			LLTabTuple* tuple = *iter;
			tuple->mButton->setVisible( TRUE );
		}

		// Draw some of the buttons...

		LLGLEnable scissor_test(has_scroll_arrows ? GL_SCISSOR_TEST : GL_FALSE);
		
		if( has_scroll_arrows )
		{
			// ...but clip them.
			S32 x1 = mRect.mLeft;
			S32 y1 = mDownArrowBtn->getRect().mTop + 3*TABCNTRV_PAD;
			S32 x2 = mRect.mRight;
			S32 y2 = mUpArrowBtn->getRect().mBottom - 3*TABCNTRV_PAD;
			LLUI::setScissorRegionLocal(LLRect(x1, y2, x2, y1));
		}

		//S32 max_scroll_visible = mTabList.size() - mMaxScrollPos + mScrollPos;
		S32 idx = 0;
		for(std::vector<LLTabTuple*>::iterator iter = mTabList.begin(); iter != mTabList.end(); ++iter)
		{
			LLTabTuple* tuple = *iter;
			tuple->mButton->translate( 0 , top - tuple->mButton->getRect().mTop);
			top -= BTN_HEIGHT + TABCNTRV_PAD;

			LLUI::pushMatrix();
			{
				LLUI::translate((F32)tuple->mButton->getRect().mLeft, (F32)tuple->mButton->getRect().mBottom, 0.f);
				tuple->mButton->draw();
			}
			LLUI::popMatrix();

			idx++;
		}

		if( has_scroll_arrows )
		{
			// Redraw the arrows so that they appears on top.
			glPushMatrix();
			glTranslatef((F32)mUpArrowBtn->getRect().mLeft, (F32)mUpArrowBtn->getRect().mBottom, 0.f);
			mUpArrowBtn->draw();
			glPopMatrix();

			glPushMatrix();
			glTranslatef((F32)mDownArrowBtn->getRect().mLeft, (F32)mDownArrowBtn->getRect().mBottom, 0.f);
			mDownArrowBtn->draw();
			glPopMatrix();
		}
	}
}

BOOL LLTabContainerVertical::handleMouseDown( S32 x, S32 y, MASK mask )
{
	BOOL handled = FALSE;
	BOOL has_scroll_arrows = (mMaxScrollPos > 0);

	if (has_scroll_arrows)
	{
		if (mUpArrowBtn->getRect().pointInRect(x, y))
		{
			S32 local_x = x - mUpArrowBtn->getRect().mLeft;
			S32 local_y = y - mUpArrowBtn->getRect().mBottom;
			handled = mUpArrowBtn->handleMouseDown(local_x, local_y, mask);
		}
		else if (mDownArrowBtn->getRect().pointInRect(x, y))
		{
			S32 local_x = x - mDownArrowBtn->getRect().mLeft;
			S32 local_y = y - mDownArrowBtn->getRect().mBottom;
			handled = mDownArrowBtn->handleMouseDown(local_x, local_y, mask);
		}
	}
	if (!handled)
	{
		handled = LLPanel::handleMouseDown( x, y, mask );
	}

	if (mTabList.size() > 0)
	{
		LLTabTuple* firsttuple = mTabList[0];		
		LLRect tab_rect(firsttuple->mButton->getRect().mLeft,
						has_scroll_arrows ? mUpArrowBtn->getRect().mBottom - TABCNTRV_PAD : mUpArrowBtn->getRect().mTop,
						firsttuple->mButton->getRect().mRight,
						has_scroll_arrows ? mDownArrowBtn->getRect().mTop + TABCNTRV_PAD : mDownArrowBtn->getRect().mBottom );
		if( tab_rect.pointInRect( x, y ) )
		{
			LLButton* tab_button = mTabList[getCurrentPanelIndex()]->mButton;
			gFocusMgr.setMouseCapture(this);
			gFocusMgr.setKeyboardFocus(tab_button, NULL);
		}
	}
	return handled;
}

BOOL LLTabContainerVertical::handleHover( S32 x, S32 y, MASK mask )
{
	BOOL handled = FALSE;
	BOOL has_scroll_arrows = (mMaxScrollPos > 0);

	if (has_scroll_arrows)
	{
		if (mUpArrowBtn->getRect().pointInRect(x, y))
		{
			S32 local_x = x - mUpArrowBtn->getRect().mLeft;
			S32 local_y = y - mUpArrowBtn->getRect().mBottom;
			handled = mUpArrowBtn->handleHover(local_x, local_y, mask);
		}
		else if (mDownArrowBtn->getRect().pointInRect(x, y))
		{
			S32 local_x = x - mDownArrowBtn->getRect().mLeft;
			S32 local_y = y - mDownArrowBtn->getRect().mBottom;
			handled = mDownArrowBtn->handleHover(local_x, local_y, mask);
		}
	}
	if (!handled)
	{
		handled = LLPanel::handleHover(x, y, mask);
	}

	commitHoveredButton(x, y);
	return handled;
}

BOOL LLTabContainerVertical::handleMouseUp( S32 x, S32 y, MASK mask )
{
	BOOL handled = FALSE;
	BOOL has_scroll_arrows = (mMaxScrollPos > 0);

	if (has_scroll_arrows)
	{
		if (mUpArrowBtn->getRect().pointInRect(x, y))
		{
			S32 local_x = x - mUpArrowBtn->getRect().mLeft;
			S32 local_y = y - mUpArrowBtn->getRect().mBottom;
			handled = mUpArrowBtn->handleMouseUp(local_x, local_y, mask);
		}
		else if (mDownArrowBtn->getRect().pointInRect(x, y))
		{
			S32 local_x = x - mDownArrowBtn->getRect().mLeft;
			S32 local_y = y - mDownArrowBtn->getRect().mBottom;
			handled = mDownArrowBtn->handleMouseUp(local_x, local_y, mask);
		}
	}
	if (!handled)
	{
		handled = LLPanel::handleMouseUp( x, y, mask );
	}

	commitHoveredButton(x, y);
	LLPanel* cur_panel = getCurrentPanel();
	if (hasMouseCapture())
	{
		if (cur_panel)
		{
			if (!cur_panel->focusFirstItem(FALSE))
			{
				mTabList[getCurrentPanelIndex()]->mButton->setFocus(TRUE);
			}
		}
		gFocusMgr.setMouseCapture(NULL);
	}

	return handled;
}

BOOL LLTabContainerVertical::handleKeyHere(KEY key, MASK mask, BOOL called_from_parent)
{
	BOOL handled = FALSE;
	if (getEnabled())
	{
		if (key == KEY_LEFT && mask == MASK_ALT)
		{
			selectPrevTab();
			handled = TRUE;
		}
		else if (key == KEY_RIGHT && mask == MASK_ALT)
		{
			selectNextTab();
			handled = TRUE;
		}

		// focus is on button
		if (!handled && !gFocusMgr.childHasKeyboardFocus(getCurrentPanel()))
		{
			switch(key)
			{
			case KEY_UP:
				selectPrevTab();
				handled = TRUE;
				break;
			case KEY_DOWN:
				selectNextTab();
				handled = TRUE;
				break;
			case KEY_LEFT:
				handled = TRUE;
				break;
			case KEY_RIGHT:
				if (getTabPosition() == LEFT && getCurrentPanel())
				{
					getCurrentPanel()->setFocus(TRUE);
				}
				handled = TRUE;
				break;
			default:
				break;
			}
		}
	}
	return handled;
}

// virtual
LLXMLNodePtr LLTabContainerVertical::getXML(bool save_children) const
{
	LLXMLNodePtr node = LLTabContainerCommon::getXML();

	// TomY TODO Is this redundant or will it be used later?
	node->createChild("tab_position", TRUE)->setStringValue("left");

	return node;
}
