/** 
 * @file llfloaterhtml.cpp
 * @brief In-world web browser
 *
 * Copyright (c) 2005-2007, Linden Research, Inc.
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

#include "llfloaterhtml.h"

#include "lldir.h"

#include "llbutton.h"
#include "llviewertexteditor.h"
#include "lllineeditor.h"
#include "lltextbox.h"
#include "llvieweruictrlfactory.h"
#include "llviewercontrol.h"
#include "llwebbrowserctrl.h"
#include "llviewerwindow.h"	// incBusyCount()
#include "llfloaterworldmap.h" //for sl urls
#include "viewer.h"

const S32 LINE = 16;
const S32 HPAD = 4;
const S32 HPAD_SMALL = 2;
const S32 HSEPARATOR = 3 * HPAD;
const S32 VPAD = 4;

const S32 SCROLLER_HPAD = 3;

BOOL process_secondlife_url(LLString url)
{
	S32 strpos, strpos2;

	LLString slurlID = "slurl.com/secondlife/";
	strpos = url.find(slurlID);
	
	if (strpos < 0)
	{
		slurlID="secondlife://";
		strpos = url.find(slurlID);
	}
	
	if (strpos >= 0) 
	{
		LLString simname;

		strpos+=slurlID.length();
		strpos2=url.find("/",strpos);
		if (strpos2 < strpos) strpos2=url.length();
		simname="secondlife://" + url.substr(strpos,url.length() - strpos);

		LLURLSimString::setString( simname );
		LLURLSimString::parse();

		// if there is a world map
		if ( gFloaterWorldMap )
		{
			// mark where the destination is
			gFloaterWorldMap->trackURL( LLURLSimString::sInstance.mSimName.c_str(),
										LLURLSimString::sInstance.mX,
										LLURLSimString::sInstance.mY,
										LLURLSimString::sInstance.mZ );

			// display map
			LLFloaterWorldMap::show( NULL, TRUE );
		};

		return TRUE;
	}
	return FALSE;
}
