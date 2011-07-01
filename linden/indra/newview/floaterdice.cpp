/**
* @file floaterdice.cpp
* @brief Dice window for Imprudence
*
* $LicenseInfo:firstyear=2009&license=viewergpl$
*
* Copyright (c) 2011, McCabe Maxsted
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

#include "floaterdice.h"

#include "llspinctrl.h"
#include "lluictrlfactory.h"

#include "llchat.h"
#include "llchatbar.h"
#include "llviewercontrol.h"


FloaterDice::FloaterDice(const LLSD& seed) : LLFloater("Roll Dice")
{
	LLUICtrlFactory::getInstance()->buildFloater(this, "floater_dice.xml");
}

FloaterDice::~FloaterDice()
{
}

BOOL FloaterDice::postBuild()
{
	childSetAction("btn_roll", onClickRoll, this);

	return TRUE;
}

// static
void FloaterDice::onClickRoll(void* data)
{
	FloaterDice* self = (FloaterDice*)data;
	if (self)
	{
		S32 dice_count = (S32)(self->getChild<LLSpinCtrl>("spin_dice_count")->getValue());
		S32 dice_sides = (S32)(self->getChild<LLSpinCtrl>("spin_dice_sides")->getValue());

		if (dice_count <= 0 || dice_sides <= 3)
		{
			llwarns << "Invalid dice roll! Someone's trying to cheat, perhaps?" << llendl;
			return;
		}
		else
		{
			S32 dice_total = 0;
			std::ostringstream rolls;
			S32 i = 0;
			do
			{
				// see the clamping rules for ll_rand
				S32 roll = ll_rand(dice_sides+1);
				if (roll > 0)
				{
					dice_total += roll;
					rolls << roll;
					if (i < dice_count - 1) rolls << ", ";
					++i;
				}
			}
			while (i < dice_count);

			std::string roll_text = llformat("/me rolled %dd%d for a total of %d", dice_count, dice_sides, dice_total);
			if (dice_count > 1)
			{
				roll_text += " (" + rolls.str() + ")";
			}
			gChatBar->sendChatFromViewer(roll_text, CHAT_TYPE_NORMAL, FALSE);
		}
	}
}
