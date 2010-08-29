/**
 * @file panelradarentry.cpp
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

#include "llviewerprecompiledheaders.h"

#include "panelradarentry.h"

const F32 RADAR_STATUS_TIMEOUT = 30.0f; // 30 seconds seems to be a good timeout

PanelRadarEntry::PanelRadarEntry(const LLUUID& agent_id, const std::string& agent_name, const F32& distance, const LLVector3d& position, const RADAR_STATUS& status, const RADAR_NOTIFIED& notified) :
	mID(agent_id),
	mName(agent_name),
	mDistance(distance),
	mPosition(position),
	mStatus(status),
	mNotified(notified),
	mStatusTimer()
{
	mStatusTimer.setTimerExpirySec(RADAR_STATUS_TIMEOUT);
}

void PanelRadarEntry::setName(const std::string& name)
{
	mName = name;
}

void PanelRadarEntry::setDistance(const F32& distance)
{
	mDistance = distance;
}

void PanelRadarEntry::setPosition(const LLVector3d& position)
{
	mPosition = position;
}

void PanelRadarEntry::setStatus(const RADAR_STATUS& status)
{
	mStatus = status;

	if (mStatus != RADAR_STATUS_NONE || mStatusTimer.hasExpired())
	{
		mStatusTimer.start();
		mStatusTimer.setTimerExpirySec(RADAR_STATUS_TIMEOUT);
	}
	else
	{
		mStatusTimer.stop();
	}
}

void PanelRadarEntry::setNotified(const RADAR_NOTIFIED& notified)
{
	mNotified = notified;
}
