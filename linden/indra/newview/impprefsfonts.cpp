/**
 * @file impprefsfonts.cpp
 * @brief Font preferences panel
 *
 * Copyright (c) 2010, Jacek Antonelli
 *
 * The source code in this file ("Source Code") is provided to you
 * under the terms of the GNU General Public License, version 2.0
 * ("GPL"). Terms of the GPL can be found in doc/GPL-license.txt in
 * this distribution, or online at
 * http://secondlifegrid.net/programs/open_source/licensing/gplv2
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
 * ALL SOURCE CODE IS PROVIDED "AS IS." THE AUTHOR MAKES NO
 * WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
 * COMPLETENESS OR PERFORMANCE.
 */


#include "llviewerprecompiledheaders.h"
#include "impprefsfonts.h"

#include "llcheckboxctrl.h"
#include "llradiogroup.h"
#include "llspinctrl.h"
#include "lluictrlfactory.h"

#include "llviewercontrol.h"
#include "llviewerwindow.h"


ImpPrefsFonts::ImpPrefsFonts()
{
	LLUICtrlFactory::getInstance()->
		buildPanel(this, "panel_preferences_fonts.xml");
}

ImpPrefsFonts::~ImpPrefsFonts()
{
}


BOOL ImpPrefsFonts::postBuild()
{
	childSetValue("ui_scale_slider", gSavedSettings.getF32("UIScaleFactor"));
	childSetValue("ui_auto_scale", gSavedSettings.getBOOL("UIAutoScale"));
	childSetAction("reset_ui_size", onClickResetUISize, this);

	refresh();
	return true;
}


void ImpPrefsFonts::refresh()
{
	LLRadioGroup* fonts = getChild<LLRadioGroup>("fonts");
	if (fonts)
	{
		fonts->setValue( gSavedSettings.getString("FontChoice") );
	}

	LLSpinCtrl* font_mult = getChild<LLSpinCtrl>("font_mult");
	if (font_mult)
	{
		font_mult->setValue( gSavedSettings.getF32("FontSizeMultiplier") );
	}

	LLCheckBoxCtrl* font_round = getChild<LLCheckBoxCtrl>("font_round");
	if (font_round)
	{
		font_round->setValue( gSavedSettings.getBOOL("FontSizeRounding") );
	}
}

void ImpPrefsFonts::apply()
{
	gSavedSettings.setF32("UIScaleFactor", childGetValue("ui_scale_slider").asReal());
	gSavedSettings.setBOOL("UIAutoScale", childGetValue("ui_auto_scale"));

	bool changed = false;

	LLRadioGroup* fonts = getChild<LLRadioGroup>("fonts");
	if (fonts)
	{
		std::string font_choice = fonts->getValue().asString();
		if (font_choice != gSavedSettings.getString("FontChoice") &&
		    !font_choice.empty())
		{
			gSavedSettings.setString("FontChoice", font_choice);
			changed = true;
		}
	}

	LLSpinCtrl* font_mult = getChild<LLSpinCtrl>("font_mult");
	if (font_mult)
	{
		F32 mult = font_mult->getValue().asReal();
		if (mult != gSavedSettings.getF32("FontSizeMultiplier"))
		{
			gSavedSettings.setF32("FontSizeMultiplier", mult);
			changed = true;
		}
	}

	LLCheckBoxCtrl* font_round = getChild<LLCheckBoxCtrl>("font_round");
	if (font_round)
	{
		BOOL round = font_round->getValue().asBoolean();
		if (round != gSavedSettings.getBOOL("FontSizeRounding"))
		{
			gSavedSettings.setBOOL("FontSizeRounding", round);
			changed = true;
		}
	}

	if (changed)
	{
		refresh();
		LLNotifications::instance().add("ChangeFont");
	}

}

void ImpPrefsFonts::cancel()
{
}

// static
void ImpPrefsFonts::onClickResetUISize(void* user_data)
{
	ImpPrefsFonts* self = (ImpPrefsFonts*)user_data;
	F32 def = gSavedSettings.getControl("UIScaleFactor")->getDefault().asReal();
	self->childSetValue("ui_scale_slider", def);
}
