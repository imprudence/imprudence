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

#include "llradiogroup.h"
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
}

void ImpPrefsFonts::apply()
{
	LLRadioGroup* fonts = getChild<LLRadioGroup>("fonts");

	if (fonts)
	{
		std::string font_choice = fonts->getValue().asString();

		if (font_choice != gSavedSettings.getString("FontChoice") &&
		    !font_choice.empty())
		{
			gSavedSettings.setString("FontChoice", font_choice);
			LLNotifications::instance().add("ChangeFont");
			refresh();
		}
	}
}

void ImpPrefsFonts::cancel()
{
}
