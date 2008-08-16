/** 
 * @file lltabcontainervertical.h
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

#ifndef LL_TABCONTAINERVERTICAL_H
#define LL_TABCONTAINERVERTICAL_H

#include "lltabcontainer.h"

const S32 TABCNTRV_CLOSE_BTN_SIZE = 16;
const S32 TABCNTRV_HEADER_HEIGHT = LLPANEL_BORDER_WIDTH + TABCNTRV_CLOSE_BTN_SIZE;
const S32 TABCNTRV_TAB_WIDTH = 100;
// const S32 TABCNTRV_TAB_HEIGHT = 16;  Use BTN_HEIGHT instead, JC.
const S32 TABCNTRV_ARROW_BTN_SIZE = 16;
const S32 TABCNTRV_BUTTON_PANEL_OVERLAP = 1;  // how many pixels the tab buttons and tab panels overlap.
const S32 TABCNTRV_PAD = 0;

class LLButton;
class LLTextBox;

class LLTabContainerVertical : public LLTabContainerCommon
{
public:
	LLTabContainerVertical( const LLString& name, const LLRect& rect,
		void(*close_callback)(void*), void* callback_userdata, 
		U32 tab_width = TABCNTRV_TAB_WIDTH, BOOL bordered = TRUE);

	LLTabContainerVertical( const LLString& name, const LLString& rect_control,
		void(*close_callback)(void*), void* callback_userdata, 
		U32 tab_width = TABCNTRV_TAB_WIDTH, BOOL bordered = TRUE);

	/*virtual*/ void initButtons();
	
	/*virtual*/ ~LLTabContainerVertical();
	
	virtual LLXMLNodePtr getXML(bool save_children = true) const;
	/*virtual*/ void draw();

	/*virtual*/ void addTabPanel(LLPanel* child, 
								 const LLString& label, 
								 BOOL select = FALSE,  
								 void (*on_tab_clicked)(void*, bool) = NULL, 
								 void* userdata = NULL,
								 S32 indent = 0,
								 BOOL placeholder = FALSE,
								 eInsertionPoint insertion_point = END);

	/*virtual*/ BOOL selectTab(S32 which);
	/*virtual*/ void removeTabPanel( LLPanel* child );

	/*virtual*/ BOOL handleMouseDown( S32 x, S32 y, MASK mask );
	/*virtual*/ BOOL handleHover( S32 x, S32 y, MASK mask );
	/*virtual*/ BOOL handleMouseUp( S32 x, S32 y, MASK mask );
	/*virtual*/ BOOL handleKeyHere(KEY key, MASK mask, BOOL called_from_parent);

protected:
	U32					mTabWidth;

	LLButton*			mUpArrowBtn;
	LLButton*			mDownArrowBtn;

protected:
	virtual void	updateMaxScrollPos();
	virtual void	commitHoveredButton(S32 x, S32 y);
};


#endif
