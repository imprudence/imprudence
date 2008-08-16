/** 
 * @file llviewerreputation.cpp
 * @brief Viewer-side reputation system handling.
 *
 * Copyright (c) 2002-2007, Linden Research, Inc.
 * 
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

#include "llviewerreputation.h"

// linden library includes
#include "llerror.h"
#include "llnamevalue.h"
#include "message.h"
#include "lluuid.h"
#include "llinstantmessage.h"
#include "lldbstrings.h"

// viewer includes
#include "llagent.h"
#include "llviewerregion.h"

// constants
const F32 REP_AGENT_SCORE_POS =  1.0f;	// positive rating changes score by this much
const F32 REP_AGENT_SCORE_NEG = -1.0f;
const F32 REP_OBJECT_SCORE_POS = 1.0f;
const F32 REP_OBJECT_SCORE_NEG = -1.0f;


// Send a reputation assignment for an agent.
// Forward through the receiving agent's simulator.
void send_reputation_agent_assign(const LLUUID& rator_id, 
								  const LLUUID& ratee_id, 
								  F32 behavior,
								  F32 appearance,
								  F32 building,
								  const char* text)
{
	LLMessageSystem* msg = gMessageSystem;
	if (!msg) return;

	msg->newMessageFast(_PREHASH_ReputationAgentAssign);
	msg->nextBlockFast(_PREHASH_DataBlock);
	msg->addUUIDFast(_PREHASH_RatorID, rator_id );
	msg->addUUIDFast(_PREHASH_RateeID, ratee_id );
	msg->addF32Fast(_PREHASH_Behavior, behavior );
	msg->addF32Fast(_PREHASH_Appearance, appearance );
	msg->addF32Fast(_PREHASH_Building, building );
	msg->sendReliable( gAgent.getRegion()->getHost() );

	// Send instant message to the person rated.
	LLString name;
	gAgent.getName(name);

	BOOL any_positive = (behavior > 0.f || appearance > 0.f || building > 0.f);
	BOOL any_negative = (behavior < 0.f || appearance < 0.f || building < 0.f);

	LLString message(name);
	if (any_positive && !any_negative)
	{
		message.append(" rated you positively: ");
	}
	else if (any_negative && !any_positive)
	{
		message.append(" rated you negatively: ");
	}
	else
	{
		message.append(" rated you: ");
	}

	if (text && strlen(text) > 0)
	{
		message.append(text);
	}
	else
	{
		message.append("(no message)");
	}

	pack_instant_message(
		msg,
		gAgent.getID(),
		FALSE,
		gAgent.getSessionID(),
		ratee_id,
		SYSTEM_FROM,
		message.c_str(),
		IM_ONLINE,
		IM_CONSOLE_AND_CHAT_HISTORY);
	gAgent.sendReliableMessage();
}
