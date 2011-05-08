/**
* @file floatercommandline.cpp
* @brief change chatbar as a commandline preferences in Imprudence
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

#include "floatercommandline.h"

#include "lluictrlfactory.h"
#include "llviewercontrol.h"

FloaterCommandLine::FloaterCommandLine(const LLSD& seed) : LLFloater("floater_command_line")
{
	LLUICtrlFactory::getInstance()->buildFloater(this, "floater_command_line.xml");
}

FloaterCommandLine::~FloaterCommandLine()
{
}

BOOL FloaterCommandLine::postBuild()
{
	childSetAction("Help_CmdLine", onClickHelp, this);

	childSetCommitCallback("CmdLineChatbarPos", onCommitApplyControl);
	childSetCommitCallback("CmdLineChatbarGround", onCommitApplyControl);
	childSetCommitCallback("CmdLineChatbarHeight", onCommitApplyControl);
	childSetCommitCallback("CmdLineChatbarTeleportHome", onCommitApplyControl);
	childSetCommitCallback("CmdLineChatbarRezPlatform", onCommitApplyControl);
	childSetCommitCallback("CmdLineChatbarMapTo", onCommitApplyControl);	
	childSetCommitCallback("CmdLineChatbarCalc", onCommitApplyControl);

	childSetCommitCallback("CmdLineChatbarDrawDistance", onCommitApplyControl);
	childSetCommitCallback("CmdLineChatbarTeleportToCam", onCommitApplyControl);
	childSetCommitCallback("CmdLineChatbarKeyToName", onCommitApplyControl);
	childSetCommitCallback("CmdLineChatbarOfferTp", onCommitApplyControl);
	childSetCommitCallback("CmdLineChatbarTP2", onCommitApplyControl);
	childSetCommitCallback("CmdLineChatbarAO", onCommitApplyControl);
	childSetCommitCallback("CmdLineChatbarClearChat", onCommitApplyControl);

	return TRUE;
}

//workaround for lineeditor dumbness in regards to control_name
void FloaterCommandLine::onCommitApplyControl(LLUICtrl* caller, void* user_data)
{
	LLLineEditor* line = (LLLineEditor*)caller;
	if(line)
	{
		LLControlVariable *var = line->findControl(line->getControlName());
		if(var)var->setValue(line->getValue());
	}
}

void FloaterCommandLine::onClickHelp(void* data)
{
	LLNotifications::instance().add("Help_CmdLine");
}
