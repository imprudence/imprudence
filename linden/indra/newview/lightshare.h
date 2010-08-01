/**
 * @file lightshare.h
 * @brief WindlightMessage class definition.
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

#include <string>

struct Meta7WindlightPacket;
class LLMessageSystem;
class LLSD;
class LLTimer;
class LLUUID;
class LLWaterParamSet;
class LLWLParamSet;


// Encapsulates a "Windlight" (LightShare) message sent from the
// region, allowing the settings to be applied at a later time.
//
class WindlightMessage
{
	public:

	// The meanings of the LightShareAllowed user setting.
	enum LIGHTSHARE_ALLOWED
	{
		LIGHTSHARE_NEVER  = 0,
		LIGHTSHARE_ASK    = 1,
		LIGHTSHARE_ALWAYS = 2,
	};

	// Constructs a new WindlightMessage instance from a GenericMessage
	// with the "Windlight" method, such as those sent by a
	// Lightshare-enabled OpenSim region.
	WindlightMessage( LLMessageSystem* msg );

	~WindlightMessage();

	// The name of the water preset where the region settings are stored.
	static const std::string sWaterPresetName;

	// The name of the sky preset where the region settings are stored.
	static const std::string sSkyPresetName;

	// Message handler for GenericMessage with the "Windlight" method.
	// Creates and applies a new WindlightMessage (or prompts user).
	static void processWindlight(LLMessageSystem* msg, void**);

	// Callback when the user interacts with the notification.
	static bool applyCallback(const LLSD& notification,
	                          const LLSD& response);

	// Called after the user has entered a new region, to reset the
	// "ignore while in this region" state.
	static void resetRegion();

	// Applies/activates the Windlight settings from the message.
	bool apply();

	// Returns true if the message contains valid Windlight settings.
	// (But there's no real validation yet, so this is always true.)
	bool isValid();


	protected:

	// Restart the timer for temporarily ignoring settings.
	static void restartIgnoreTimer();

	// Returns true if the ignore timer has expired (i.e. new settings
	// should not be ignored anymore).
	static bool ignoreTimerHasExpired();


	private:

	static WindlightMessage* sMostRecent;
	static LLTimer* sIgnoreTimer;
	static bool sIgnoreRegion;

	Meta7WindlightPacket* mPacket;
	LLWaterParamSet* mWater;
	LLWLParamSet* mSky;
	LLUUID* mWaterNormal;
	bool mIsValid;

	// Converts the message's raw bytes into a Meta7WindlightPacket.
	void process_packet( char* buf );

	// Constructs a LLWaterParamSet from the Meta7WindlightPacket.
	void process_water();

	// Constructs a LLWLParamSet from the Meta7WindlightPacket.
	void process_sky();

};

#endif
