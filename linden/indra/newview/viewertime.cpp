/**
* @file viewertime.cpp
* @brief sets the viewer time and time format
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

#include <sstream> 
#include "llviewerprecompiledheaders.h"

#include "llappviewer.h" // for gPacificDaylightTime
#include "lltrans.h"
#include "llviewercontrol.h"
#include "viewertime.h"

// system includes
#include <iomanip>

// WIP -- MC
// TODO: localize time
// TODO: accept time_t and return the appropriate format for timestamps
// TODO: support multiple date formats

ViewerTime* gViewerTime = 0;
// We use statics here for speed reasons
bool ViewerTime::sUse24HourTime = false;
bool ViewerTime::sUseUTCTime = false;
bool ViewerTime::sUseTimeOffset = false;
S32 ViewerTime::sTimeOffset = 0;
bool ViewerTime::sTimeOffsetDST = false;

std::vector<std::string> ViewerTime::sDays;
std::vector<std::string> ViewerTime::sMonths;
const U32 ViewerTime::MAX_DATE_STRING_LENGTH = 2000;

ViewerTime::ViewerTime() :
	mHour(0),
	mMinute(0),
	mSecond(0),
	mTZ(LLStringUtil::null),
	mTimeStr(LLStringUtil::null),
	mAMPM(" AM"),
	mDay(LLStringUtil::null),
	mDayNum(1),
	mMonth(LLStringUtil::null),
	mYear(1900),
	mDateStr(LLStringUtil::null)
{
	// size of day of the weeks and year
	sDays.reserve(7);
	sMonths.reserve(12);

	setupDate();
	refresh();
}

ViewerTime::~ViewerTime()
{
}

void ViewerTime::refresh()
{
	// Get current UTC time, adjusted for the user's clock
	// being off.
	time_t utc_time;
	utc_time = time_corrected();

	// There's only one internal tm buffer.
	struct tm* internal_time;

	if (sUseUTCTime)
	{
		time(&utc_time);
		internal_time = gmtime(&utc_time);
	}
	else if (sUseTimeOffset)
	{
		//Its a UTC offset, deal with it
		internal_time = utc_to_offset_time(utc_time, sTimeOffset, sTimeOffsetDST);
	}
	else
	{
		// Convert to Pacific, based on server's opinion of whether
		// it's daylight savings time there.
		internal_time = utc_to_pacific_time(utc_time, gPacificDaylightTime);
	}

	if(NULL == internal_time)
	{
		llwarns << "internal_time == NULL - Kaboom!" << llendl;
		return;
	}

	mMinute = internal_time->tm_min;
	mSecond = internal_time->tm_sec;
	S32 hour = internal_time->tm_hour;

	mAMPM = "";
	char leading = '0';

	if (!sUseUTCTime) 
	{
		if (!sUse24HourTime)
		{
			leading = ' ';
			mAMPM = " AM";
			if (hour > 11)
			{
				hour -= 12;
				mAMPM = " PM";
			}
			// Zero hour is 12 AM
			if (hour == 0) hour = 12;
		}

		if (sUseTimeOffset)
		{
			std::stringstream myString;
			myString << "UTC " << sTimeOffset;
			mTZ = myString.str();
		}
		else
		{
			mTZ = "PST";
			if (gPacificDaylightTime)
			{
				mTZ = "PDT";
			}
		}
	}
	else // just UTC
	{
		mTZ = "UTC";
	}
	mHour = hour;

	std::ostringstream t;
	t	<< std::setfill(leading) << std::setw(2) << mHour << ":" 
		<< std::setfill('0') << std::setw(2) << mMinute
		<< mAMPM << " " << mTZ;
	mTimeStr = t.str();

	// Year starts at 1900
	mDay = sDays[internal_time->tm_wday];
	mDayNum = internal_time->tm_mday;
	mMonth = sMonths[internal_time->tm_mon];
	mYear = internal_time->tm_year + 1900;

	std::ostringstream date;
	date	<< mDay << ", "
			<< std::setfill('0') << std::setw(2) << mDayNum << " "
			<< mMonth << " "
			<< mYear;

	mDateStr = date.str();
}

// sets the static variables necessary for the date
void ViewerTime::setupDate()
{
	// fill the day array with what's in the xui
	std::string day_list = LLTrans::getString("StatBarDaysOfWeek");
	size_t length = day_list.size();
	
	// quick input check
	if(length < MAX_DATE_STRING_LENGTH)
	{
		// tokenize it and put it in the array
		std::string cur_word;
		for(size_t i = 0; i < length; ++i)
		{
			if(day_list[i] == ':')
			{
				sDays.push_back(cur_word);
				cur_word.clear();
			}
			else
			{
				cur_word.append(1, day_list[i]);
			}
		}
		sDays.push_back(cur_word);
	}
	
	// fill the day array with what's in the xui	
	std::string month_list = LLTrans::getString("StatBarMonthsOfYear");
	length = month_list.size();
	
	// quick input check
	if(length < MAX_DATE_STRING_LENGTH)
	{
		// tokenize it and put it in the array
		std::string cur_word;
		for(size_t i = 0; i < length; ++i)
		{
			if(month_list[i] == ':')
			{
				sMonths.push_back(cur_word);
				cur_word.clear();
			}
			else
			{
				cur_word.append(1, month_list[i]);
			}
		}
		sMonths.push_back(cur_word);
	}
	
	// make sure we have at least 7 days and 12 months
	if(sDays.size() < 7)
	{
		sDays.resize(7);
	}
	
	if(sMonths.size() < 12)
	{
		sMonths.resize(12);
	}
}

std::string ViewerTime::getDayOfWeek(const U32& day_num)
{
	if (day_num < 7 && !sDays.empty())
	{
		return sDays[day_num];
	}
	return LLStringUtil::null;
}

std::string ViewerTime::getMonth(const U32& month_num)
{
	if (month_num < 12 && !sMonths.empty())
	{
		return sMonths[month_num];
	}
	return LLStringUtil::null;
}

void ViewerTime::updateTimeFormat(const U32& index)
{
	/*
	Time Format: 
	0 - sim 12 hour time
	1 - sim 24 hour time
	2 - UTC time
	*/
	switch (index)
	{
	case 2:
		sUse24HourTime = false;
		sUseUTCTime = true;
		break;
	case 1:
		sUse24HourTime = true;
		sUseUTCTime = false;
		break;
	case 0:
	default:
		sUse24HourTime = false;
		sUseUTCTime = false;
		break;
	}

	gSavedSettings.setBOOL("Use24HourTime", sUse24HourTime);
	gSavedSettings.setBOOL("UseUTCTime", sUseUTCTime);
}
