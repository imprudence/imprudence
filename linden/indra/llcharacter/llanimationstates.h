/** 
 * @file llanimationstates.h
 * @brief Implementation of animation state support.
 *
 * $LicenseInfo:firstyear=2001&license=viewergpl$
 * 
 * Copyright (c) 2001-2009, Linden Research, Inc.
 * 
 * Second Life Viewer Source Code
 * The source code in this file ("Source Code") is provided by Linden Lab
 * to you under the terms of the GNU General Public License, version 2.0
 * ("GPL"), unless you have obtained a separate licensing agreement
 * ("Other License"), formally executed by you and Linden Lab.  Terms of
 * the GPL can be found in doc/GPL-license.txt in this distribution, or
 * online at http://secondlifegrid.net/programs/open_source/licensing/gplv2
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
 * ALL LINDEN LAB SOURCE CODE IS PROVIDED "AS IS." LINDEN LAB MAKES NO
 * WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
 * COMPLETENESS OR PERFORMANCE.
 * $/LicenseInfo$
 */

#ifndef LL_LLANIMATIONSTATES_H
#define LL_LLANIMATIONSTATES_H

#include <map>

#include "string_table.h"
#include "lluuid.h"

//-----------------------------------------------------------------------------
// These bit flags are generally used to track the animation state
// of characters.  The simulator and viewer share these flags to interpret
// the Animation name/value attribute on agents.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Agent Animation State
//-----------------------------------------------------------------------------
const S32 MAX_CONCURRENT_ANIMS = 16;

extern LLUUID const ANIM_AGENT_AFRAID;
extern LLUUID const ANIM_AGENT_AIM_BAZOOKA_R;
extern LLUUID const ANIM_AGENT_AIM_BOW_L;
extern LLUUID const ANIM_AGENT_AIM_HANDGUN_R;
extern LLUUID const ANIM_AGENT_AIM_RIFLE_R;
extern LLUUID const ANIM_AGENT_ANGRY;
extern LLUUID const ANIM_AGENT_AWAY;
extern LLUUID const ANIM_AGENT_BACKFLIP;
extern LLUUID const ANIM_AGENT_BELLY_LAUGH;
extern LLUUID const ANIM_AGENT_BLOW_KISS;
extern LLUUID const ANIM_AGENT_BORED;
extern LLUUID const ANIM_AGENT_BOW;
extern LLUUID const ANIM_AGENT_BRUSH;
extern LLUUID const ANIM_AGENT_BUSY;
extern LLUUID const ANIM_AGENT_CLAP;
extern LLUUID const ANIM_AGENT_COURTBOW;
extern LLUUID const ANIM_AGENT_CROUCH;
extern LLUUID const ANIM_AGENT_CROUCHWALK;
extern LLUUID const ANIM_AGENT_CRY;
extern LLUUID const ANIM_AGENT_CUSTOMIZE;
extern LLUUID const ANIM_AGENT_CUSTOMIZE_DONE;
extern LLUUID const ANIM_AGENT_DANCE1;
extern LLUUID const ANIM_AGENT_DANCE2;
extern LLUUID const ANIM_AGENT_DANCE3;
extern LLUUID const ANIM_AGENT_DANCE4;
extern LLUUID const ANIM_AGENT_DANCE5;
extern LLUUID const ANIM_AGENT_DANCE6;
extern LLUUID const ANIM_AGENT_DANCE7;
extern LLUUID const ANIM_AGENT_DANCE8;
extern LLUUID const ANIM_AGENT_DEAD;
extern LLUUID const ANIM_AGENT_DRINK;
extern LLUUID const ANIM_AGENT_EMBARRASSED;
extern LLUUID const ANIM_AGENT_EXPRESS_AFRAID;
extern LLUUID const ANIM_AGENT_EXPRESS_ANGER;
extern LLUUID const ANIM_AGENT_EXPRESS_BORED;
extern LLUUID const ANIM_AGENT_EXPRESS_CRY;
extern LLUUID const ANIM_AGENT_EXPRESS_DISDAIN;
extern LLUUID const ANIM_AGENT_EXPRESS_EMBARRASSED;
extern LLUUID const ANIM_AGENT_EXPRESS_FROWN;
extern LLUUID const ANIM_AGENT_EXPRESS_KISS;
extern LLUUID const ANIM_AGENT_EXPRESS_LAUGH;
extern LLUUID const ANIM_AGENT_EXPRESS_OPEN_MOUTH;
extern LLUUID const ANIM_AGENT_EXPRESS_REPULSED;
extern LLUUID const ANIM_AGENT_EXPRESS_SAD;
extern LLUUID const ANIM_AGENT_EXPRESS_SHRUG;
extern LLUUID const ANIM_AGENT_EXPRESS_SMILE;
extern LLUUID const ANIM_AGENT_EXPRESS_SURPRISE;
extern LLUUID const ANIM_AGENT_EXPRESS_TONGUE_OUT;
extern LLUUID const ANIM_AGENT_EXPRESS_TOOTHSMILE;
extern LLUUID const ANIM_AGENT_EXPRESS_WINK;
extern LLUUID const ANIM_AGENT_EXPRESS_WORRY;
extern LLUUID const ANIM_AGENT_FALLDOWN;
extern LLUUID const ANIM_AGENT_FEMALE_WALK;
extern LLUUID const ANIM_AGENT_FINGER_WAG;
extern LLUUID const ANIM_AGENT_FIST_PUMP;
extern LLUUID const ANIM_AGENT_FLY;
extern LLUUID const ANIM_AGENT_FLYSLOW;
extern LLUUID const ANIM_AGENT_HELLO;
extern LLUUID const ANIM_AGENT_HOLD_BAZOOKA_R;
extern LLUUID const ANIM_AGENT_HOLD_BOW_L;
extern LLUUID const ANIM_AGENT_HOLD_HANDGUN_R;
extern LLUUID const ANIM_AGENT_HOLD_RIFLE_R;
extern LLUUID const ANIM_AGENT_HOLD_THROW_R;
extern LLUUID const ANIM_AGENT_HOVER;
extern LLUUID const ANIM_AGENT_HOVER_DOWN;
extern LLUUID const ANIM_AGENT_HOVER_UP;
extern LLUUID const ANIM_AGENT_IMPATIENT;
extern LLUUID const ANIM_AGENT_JUMP;
extern LLUUID const ANIM_AGENT_JUMP_FOR_JOY;
extern LLUUID const ANIM_AGENT_KISS_MY_BUTT;
extern LLUUID const ANIM_AGENT_LAND;
extern LLUUID const ANIM_AGENT_LAUGH_SHORT;
extern LLUUID const ANIM_AGENT_MEDIUM_LAND;
extern LLUUID const ANIM_AGENT_MOTORCYCLE_SIT;
extern LLUUID const ANIM_AGENT_MUSCLE_BEACH;
extern LLUUID const ANIM_AGENT_NO;
extern LLUUID const ANIM_AGENT_NO_UNHAPPY;
extern LLUUID const ANIM_AGENT_NYAH_NYAH;
extern LLUUID const ANIM_AGENT_ONETWO_PUNCH;
extern LLUUID const ANIM_AGENT_PEACE;
extern LLUUID const ANIM_AGENT_POINT_ME;
extern LLUUID const ANIM_AGENT_POINT_YOU;
extern LLUUID const ANIM_AGENT_PRE_JUMP;
extern LLUUID const ANIM_AGENT_PUNCH_LEFT;
extern LLUUID const ANIM_AGENT_PUNCH_RIGHT;
extern LLUUID const ANIM_AGENT_REPULSED;
extern LLUUID const ANIM_AGENT_ROUNDHOUSE_KICK;
extern LLUUID const ANIM_AGENT_RPS_COUNTDOWN;
extern LLUUID const ANIM_AGENT_RPS_PAPER;
extern LLUUID const ANIM_AGENT_RPS_ROCK;
extern LLUUID const ANIM_AGENT_RPS_SCISSORS;
extern LLUUID const ANIM_AGENT_RUN;
extern LLUUID const ANIM_AGENT_SAD;
extern LLUUID const ANIM_AGENT_SALUTE;
extern LLUUID const ANIM_AGENT_SHOOT_BOW_L;
extern LLUUID const ANIM_AGENT_SHOUT;
extern LLUUID const ANIM_AGENT_SHRUG;
extern LLUUID const ANIM_AGENT_SIT;
extern LLUUID const ANIM_AGENT_SIT_FEMALE;
extern LLUUID const ANIM_AGENT_SIT_GENERIC;
extern LLUUID const ANIM_AGENT_SIT_GROUND;
extern LLUUID const ANIM_AGENT_SIT_GROUND_CONSTRAINED;
extern LLUUID const ANIM_AGENT_SIT_TO_STAND;
extern LLUUID const ANIM_AGENT_SLEEP;
extern LLUUID const ANIM_AGENT_SMOKE_IDLE;
extern LLUUID const ANIM_AGENT_SMOKE_INHALE;
extern LLUUID const ANIM_AGENT_SMOKE_THROW_DOWN;
extern LLUUID const ANIM_AGENT_SNAPSHOT;
extern LLUUID const ANIM_AGENT_STAND;
extern LLUUID const ANIM_AGENT_STANDUP;
extern LLUUID const ANIM_AGENT_STAND_1;
extern LLUUID const ANIM_AGENT_STAND_2;
extern LLUUID const ANIM_AGENT_STAND_3;
extern LLUUID const ANIM_AGENT_STAND_4;
extern LLUUID const ANIM_AGENT_STRETCH;
extern LLUUID const ANIM_AGENT_STRIDE;
extern LLUUID const ANIM_AGENT_SURF;
extern LLUUID const ANIM_AGENT_SURPRISE;
extern LLUUID const ANIM_AGENT_SWORD_STRIKE;
extern LLUUID const ANIM_AGENT_TALK;
extern LLUUID const ANIM_AGENT_TANTRUM;
extern LLUUID const ANIM_AGENT_THROW_R;
extern LLUUID const ANIM_AGENT_TRYON_SHIRT;
extern LLUUID const ANIM_AGENT_TURNLEFT;
extern LLUUID const ANIM_AGENT_TURNRIGHT;
extern LLUUID const ANIM_AGENT_TYPE;
extern LLUUID const ANIM_AGENT_WALK;
extern LLUUID const ANIM_AGENT_WHISPER;
extern LLUUID const ANIM_AGENT_WHISTLE;
extern LLUUID const ANIM_AGENT_WINK;
extern LLUUID const ANIM_AGENT_WINK_HOLLYWOOD;
extern LLUUID const ANIM_AGENT_WORRY;
extern LLUUID const ANIM_AGENT_YES;
extern LLUUID const ANIM_AGENT_YES_HAPPY;
extern LLUUID const ANIM_AGENT_YOGA_FLOAT;

extern LLUUID AGENT_WALK_ANIMS[];
extern S32 NUM_AGENT_WALK_ANIMS;

extern LLUUID AGENT_GUN_HOLD_ANIMS[];
extern S32 NUM_AGENT_GUN_HOLD_ANIMS;

extern LLUUID AGENT_GUN_AIM_ANIMS[];
extern S32 NUM_AGENT_GUN_AIM_ANIMS;

extern LLUUID AGENT_NO_ROTATE_ANIMS[];
extern S32 NUM_AGENT_NO_ROTATE_ANIMS;

extern LLUUID AGENT_STAND_ANIMS[];
extern S32 NUM_AGENT_STAND_ANIMS;

class LLAnimationLibrary
{
private:
	LLStringTable mAnimStringTable;

	typedef std::map<LLUUID, char *> anim_map_t;
	anim_map_t mAnimMap;

public:
	LLAnimationLibrary();
	~LLAnimationLibrary();

	//-----------------------------------------------------------------------------
	// Return the text name of a single animation state,
	// Return NULL if the state is invalid
	//-----------------------------------------------------------------------------
	const char *animStateToString( const LLUUID& state );

	//-----------------------------------------------------------------------------
	// Return the animation state for the given name.
	// Retun NULL if the name is invalid.
	//-----------------------------------------------------------------------------
	LLUUID stringToAnimState( const std::string& name, BOOL allow_ids = TRUE );
};

struct LLAnimStateEntry
{
	LLAnimStateEntry(const char* name, const LLUUID& id) :
		mName(name),
		mID(id)
	{ 
		// LABELS:
		// Look to newview/LLAnimStateLabels.* for how to get the labels.
		// The labels should no longer be stored in this structure. The server
		// shouldn't care about the local friendly name of an animation, and
		// this is common code.
	}


	const char* mName;
	const LLUUID mID;
};

// Animation states that the user can trigger
extern const LLAnimStateEntry gUserAnimStates[];
extern const S32 gUserAnimStatesCount;
extern LLAnimationLibrary gAnimLibrary;


#endif // LL_LLANIMATIONSTATES_H



