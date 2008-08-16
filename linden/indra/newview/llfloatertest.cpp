/** 
 * @file llfloatertest.cpp
 * @author James Cook
 * @brief Torture-test floater for all UI elements
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

#include "llfloatertest.h"

#include "llbutton.h"
#include "llcheckboxctrl.h"
#include "llcombobox.h"
#include "llfloater.h"
#include "lliconctrl.h"
#include "lllineeditor.h"
#include "llradiogroup.h"
#include "llscrollcontainer.h"
#include "llscrolllistctrl.h"
#include "llslider.h"
#include "llsliderctrl.h"
#include "lltabcontainer.h"
#include "lltextbox.h"
#include "lltexteditor.h"
#include "llview.h"
#include "llviewborder.h"

#include "llnamelistctrl.h"
#include "llvieweruictrlfactory.h"
#include "llviewercontrol.h"

class LLFloaterTestImpl : public LLFloater
{
public:
	LLFloaterTestImpl();

private:
	static void onClickButton(void*);
	static void onClickText(void*);
	static void onCloseTab(void*);
	static void onClickTab(void*, bool);
	static void onCommitCheck(LLUICtrl*, void*);
	static void onCommitCombo(LLUICtrl*, void*);
	static void onCommitLine(LLUICtrl*, void*);
	static void onKeyLine(LLLineEditor*, void*);
	static void onFocusLostLine(LLUICtrl*, void*);
	static void onChangeRadioGroup(LLUICtrl*, void*);

	LLButton* mBtnSimple;
	LLButton* mBtnUnicode;
	LLButton* mBtnImages;
	LLCheckBoxCtrl* mCheckSimple;
	LLCheckBoxCtrl* mCheckUnicode;
	LLComboBox* mCombo;
	LLIconCtrl* mIcon;
	LLLineEditor* mLineEditor;
	LLRadioGroup* mRadioGroup;
	LLRadioCtrl* mRadio1;
	LLRadioCtrl* mRadio2;
	LLScrollableContainerView* mScroll;
	LLScrollListCtrl* mScrollList;
	LLTabContainer* mTab;
	LLTextEditor* mTextEditor;
	LLView* mView;
	LLViewBorder* mViewBorder;

	LLNameListCtrl* mNameList;
};


LLFloaterTestImpl::LLFloaterTestImpl()
:	LLFloater("test", LLRect(0, 500, 700, 0), "Test UI")
{
	const S32 HPAD = 5;
	const S32 VPAD = 5;
	const S32 LEFT = HPAD;
	const S32 RIGHT = getRect().getWidth() - HPAD;
	const S32 TOP = getRect().getHeight() - LLFLOATER_HEADER_SIZE - VPAD;
	const S32 BOTTOM = VPAD;
	const S32 LINE = 15;

	//S32 x = LEFT;
	S32 y = TOP;

	LLTextBox* text = NULL;
	LLButton* btn = NULL;
	LLTabContainer* tab = NULL;
	LLPanel* panel = NULL;
	LLCheckBoxCtrl* check = NULL;
	LLComboBox* combo = NULL;

	btn = new LLButton("can't click",
		LLRect(LEFT+150, y, LEFT+150+100, y-LINE),
		NULL,	// LLString converts to ""
		onClickButton, this);
	btn->setFollows(FOLLOWS_LEFT|FOLLOWS_TOP);
	btn->setFont(LLFontGL::sSansSerifSmall);
	addChild(btn);

	text = new LLTextBox("simple_text", 
		"simple sans-serif text that is mouse opaque opaque opaque",
		50,	// max_width
		LLFontGL::sSansSerifSmall,
		TRUE);	// mouse_opaque
	text->setClickedCallback(onClickText);
	text->setRect(LLRect(LEFT, y, RIGHT, y-LINE));
	addChild(text);

	y -= VPAD + LINE;

	btn = new LLButton("can click",
		LLRect(LEFT+150, y, LEFT+150+100, y-LINE),
		NULL,	// LLString converts to ""
		onClickButton, this);
	btn->setFollows(FOLLOWS_LEFT|FOLLOWS_TOP);
	btn->setFont(LLFontGL::sSansSerifSmall);
	addChild(btn);

	text = new LLTextBox("simple2_text", 
		LLRect(LEFT, y, RIGHT, y-LINE),
		"monospaced, non-opaque text with tooltip, non-opaque non-opaque",
		LLFontGL::sMonospace,
		FALSE);	// mouse_opaque
	text->setToolTip("I'm a tooltip");
	addChild(text);

	y -= VPAD + LINE;

	tab = new LLTabContainer("test_tab", 
		LLRect(LEFT, y, RIGHT, BOTTOM),
		LLTabContainer::TOP,
		onCloseTab, this,
		"Tab Title",
		TRUE);	// bordered
	addChild(tab);
	mTab = tab;

	//-----------------------------------------------------------------------
	// First tab container panel
	//-----------------------------------------------------------------------
	panel = new LLPanel("first_tab_panel",
		LLRect(0, 400, 400, 0),	// dummy rect
		TRUE);	// bordered
	tab->addTabPanel(panel, "First", 
		TRUE,	// select
		onClickTab, this);

	y = panel->getRect().getHeight() - VPAD;

	text = new LLTextBox("unicode_text",
		LLRect(LEFT, y, RIGHT, y-LINE),
		LLString("File"),
		LLFontGL::sSansSerif,
		TRUE);	// mouse_opaque
	text->setToolTip("This should be Unicode text");
	panel->addChild(text);

	y -= VPAD + LINE;

	btn = new LLButton("unicode_btn", LLRect(LEFT, y, LEFT+100, y-20));
	btn->setLabelUnselected("unicode");
	btn->setLabelSelected("unicode");
	panel->addChild(btn);

	y -= VPAD + 20;

	btn = new LLButton("image_btn",
		LLRect(LEFT, y, LEFT+32, y-32),
		"tool_zoom.tga",
		"tool_zoom_active.tga",
		"",	// control_name,
		onClickButton, this,
		LLFontGL::sSansSerifSmall);
	btn->setFollows(FOLLOWS_LEFT | FOLLOWS_TOP);
	panel->addChild(btn);

	y -= VPAD + 32;

	check = new LLCheckBoxCtrl("simple_check",
		LLRect(LEFT, y, LEFT+150, y-LLCHECKBOXCTRL_HEIGHT),
		"Simple Checkbox",
		LLFontGL::sSansSerifSmall,
		onCommitCheck, this,
		TRUE,	// initial_value
		FALSE,	// radio_style
		"UIFloaterTestBool");	// control_which
	panel->addChild(check);

	y -= VPAD + LLCHECKBOXCTRL_HEIGHT;

	check = new LLCheckBoxCtrl("unicode_check",
		LLRect(LEFT, y, LEFT+150, y-LLCHECKBOXCTRL_HEIGHT),
		"TODO: Unicode Checkbox",
		LLFontGL::sSansSerifSmall,
		onCommitCheck, this,
		TRUE,	// initial_value
		FALSE,	// radio_style
		"");	// control_which
	panel->addChild(check);
	mCheckUnicode = check;

	y -= VPAD + LLCHECKBOXCTRL_HEIGHT;

	combo = new LLComboBox("combo",
		LLRect(LEFT, y, LEFT+100, y-LLCOMBOBOX_HEIGHT),
		"Combobox Label",
		onCommitCombo, this);
	combo->add("first item");
	combo->add("second item");
	combo->add("should go to the top", ADD_TOP);
	combo->add("disabled item", NULL, ADD_BOTTOM, FALSE);
	panel->addChild(combo);

	y -= VPAD + LLCOMBOBOX_HEIGHT;

	LLIconCtrl* icon = new LLIconCtrl(
		"test_icon",
		LLRect(LEFT, y, LEFT+32, y-32),
		"object_cone.tga" );
	panel->addChild(icon);
	mIcon = icon;

	y -= VPAD + 32;

	LLLineEditor* line = new LLLineEditor(
		"test_line",
		LLRect(LEFT, y, LEFT+200, y-20),
		"test some unicode text here",
		LLFontGL::sSansSerif,
		200,	// max_length_bytes
		onCommitLine,
		onKeyLine,
		onFocusLostLine,
		this,
		NULL,	// prevalidate func
		LLViewBorder::BEVEL_IN,
		LLViewBorder::STYLE_LINE,
		1);		// border thickness
	line->setHandleEditKeysDirectly(true);
	panel->addChild(line);

	y -= VPAD + 20;

	LLRadioGroup* group = new LLRadioGroup(
		"radio_group",
		LLRect(LEFT, y, LEFT+200, y - 50),
		0,	// initial_index
		onChangeRadioGroup, this,
		TRUE);	// border
	panel->addChild(group);

	S32 yy = 100;
	group->addRadioButton("Radio1", "Radio 1", LLRect(0, yy, 200, yy-LINE), LLFontGL::sSansSerifSmall);
	yy -= LINE;
	group->addRadioButton("Radio2", "Radio 2", LLRect(0, yy, 200, yy-LINE), LLFontGL::sSansSerifSmall);
	yy -= LINE;
	group->addRadioButton("Radio3", "Radio 3", LLRect(0, yy, 200, yy-LINE), LLFontGL::sSansSerifSmall);
	yy -= LINE;

	//-----------------------------------------------------------------------
	// Second tab container panel
	//-----------------------------------------------------------------------
	panel = new LLPanel("second_tab_panel",
		LLRect(0, 400, 400, 0),	// dummy rect
		TRUE);	// bordered
	tab->addTabPanel(panel, "Second", 
		FALSE,	// select
		onClickTab, this);

	y = panel->getRect().getHeight() - VPAD;

	btn = new LLButton("Simple Button",
		LLRect(LEFT, y, LEFT+100, y - 20),
		"",
		onClickButton, this);
	btn->setFollows(FOLLOWS_TOP|FOLLOWS_LEFT);
	panel->addChild(btn);
	mBtnSimple = btn;

	//-----------------------------------------------------------------------
	// Hook us up with the floater view
	//-----------------------------------------------------------------------
	open();		/* Flawfinder: ignore */
	center();
}

// static
void LLFloaterTestImpl::onClickButton(void*)
{
	llinfos << "button clicked" << llendl;
}

// static
void LLFloaterTestImpl::onClickText(void*)
{
	llinfos << "text clicked" << llendl;
}

// static
void LLFloaterTestImpl::onCloseTab(void*)
{
	llinfos << "close tab" << llendl;
}

// static
void LLFloaterTestImpl::onClickTab(void*, bool)
{
	llinfos << "click tab" << llendl;
}

// static
void LLFloaterTestImpl::onCommitCheck(LLUICtrl*, void*)
{
	llinfos << "commit check" << llendl;
}

// static
void LLFloaterTestImpl::onCommitCombo(LLUICtrl* ctrl, void*)
{
	LLComboBox* combo = (LLComboBox*)ctrl;
	LLString name = combo->getSimple();
	LLSD value = combo->getValue();
	llinfos << "commit combo name " << name << " value " << value.asString() << llendl;
}

// static
void LLFloaterTestImpl::onCommitLine(LLUICtrl*, void*)
{
	llinfos << "commit line editor" << llendl;
}

// static
void LLFloaterTestImpl::onKeyLine(LLLineEditor*, void*)
{
	llinfos << "keystroke line editor" << llendl;
}

// static
void LLFloaterTestImpl::onFocusLostLine(LLUICtrl*, void*)
{
	llinfos << "focus lost line editor" << llendl;
}

// static
void LLFloaterTestImpl::onChangeRadioGroup(LLUICtrl*, void*)
{
	llinfos << "change radio group" << llendl;
}

//---------------------------------------------------------------------------

// static
void LLFloaterTest::show(void*)
{
	new LLFloaterTest();
}

LLFloaterTest::LLFloaterTest()
:	impl(* new LLFloaterTestImpl)
{
}

LLFloaterTest::~LLFloaterTest()
{
	delete &impl;
}
