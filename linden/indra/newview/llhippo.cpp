/** 
 * @file llhippo.cpp
 * @brief Create a bug for our users to find
 *
 * Copyright (c) 2003-2007, Linden Research, Inc.
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

#include "llhippo.h"

#include "llchat.h"

#include "llagent.h"
#include "llviewerregion.h"
#include "llfloaterchat.h"	// for add_chat

void trigger_hippo_bug(void*)
{
	BOOL chat = FALSE;
	LLViewerRegion* region = gAgent.getRegion();
	const char* name = (region ? region->getName().c_str() : NULL);

	if (name)
	{
		if (   name[0] == 'A'
			|| name[0] == 'E'
			|| name[0] == 'I'
			|| name[0] == 'O'
			|| name[0] == 'U')
		{
			chat = TRUE;
		}
		else if (ll_frand() > 0.5f)
		{
			chat = TRUE;
		}
	}

	if (chat)
	{
		LLChat chat("hippos!");
		chat.mFromName = "Ghostly Bug";
		LLFloaterChat::addChat(chat);
	}
}
