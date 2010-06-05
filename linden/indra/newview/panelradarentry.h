/**
 * @file panelradarentry.h
 * @brief PanelRadarEntry class (container for nearby agents)
 *
 * Copyright (c) 2010, McCabe Maxsted
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


#ifndef PANELRADARENTRY_H
#define PANELRADARENTRY_H


#include "llframetimer.h"

enum RADAR_STATUS
{
	RADAR_STATUS_NONE,
	RADAR_STATUS_TYPING,
	RADAR_STATUS_EMITTING_SOUNDS,
	RADAR_STATUS_EMITTING_PARTICLES,
	RADAR_STATUS_FLYING,
	RADAR_STATUS_BUSY,
	RADAR_STATUS_AWAY
};

enum RADAR_NOTIFIED
{
	RADAR_NOTIFIED_NONE,
	RADAR_NOTIFIED_SIM,
	RADAR_NOTIFIED_CHAT
};


class PanelRadarEntry
{
public:	
	PanelRadarEntry(const LLUUID& agent_id, const std::string& agent_name, const F32& distance, const LLVector3d& position, const RADAR_STATUS& status, const RADAR_NOTIFIED& notified);

	LLUUID			getID()			{ return mID; }
	std::string		getName()		{ return mName; }
	F32				getDistance()	{ return mDistance; }
	LLVector3d		getPosition()	{ return mPosition; }
	RADAR_STATUS	getStatus()		{ return mStatus; }
	RADAR_NOTIFIED	getNotified()	{ return mNotified; }
	LLFrameTimer	getStatusTimer(){ return mStatusTimer; }

	void setName(const std::string& name);
	void setDistance(const F32& distance);
	void setPosition(const LLVector3d& position);
	void setStatus(const RADAR_STATUS& status);
	void setNotified(const RADAR_NOTIFIED& notified);

private:
	LLUUID			mID;
	std::string		mName;
	F32				mDistance;
	LLVector3d		mPosition;
	RADAR_STATUS	mStatus;
	RADAR_NOTIFIED	mNotified;
	LLFrameTimer	mStatusTimer;
};

#endif // PANELRADARENTRY_H
