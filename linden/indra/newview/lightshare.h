/**
 * @file lightshare.h
 * @brief Public interface for lightshare.cpp
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


#ifndef LIGHTSHARE_H
#define LIGHTSHARE_H

#include "message.h"
#include "meta7windlight.h"
#include "llwaterparamset.h"
#include "llwlparamset.h"


// Encapsulates a "Windlight" (LightShare) message sent from the
// server, allowing the settings to be applied at a later time.
//
class WindlightMessage
{
	public:

	static const std::string sWaterPresetName;
	static const std::string sSkyPresetName;

	WindlightMessage( LLMessageSystem* msg );
	~WindlightMessage();
	static void processWindlight(LLMessageSystem* msg, void**);
	static bool applyCallback(const LLSD& notification,
	                          const LLSD& response,
	                          WindlightMessage* wl);

	bool apply();
	bool isValid();

	private:

	Meta7WindlightPacket* mPacket;
	LLWaterParamSet* mWater;
	LLWLParamSet* mSky;
	LLUUID mWaterNormal;
	bool mIsValid;

	void process_packet( char* buf );
	void process_water();
	void process_sky();
};

#endif
