/**
* @file viewertime.h
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

#ifndef VIEWERTIME_H
#define VIEWERTIME_H


class ViewerTime
{
public:
	ViewerTime();
	~ViewerTime();

	// Per-frame refresh of the time and date (called in LLStatusBar)
	void refresh();

	std::string getDayOfWeek(const U32& day_num);
	std::string getMonth(const U32& month_num);

	std::string getCurDay() { return mDay; }
	std::string getCurMonth() { return mMonth; }
	S32 getCurDayNum() { return mDayNum; }

	S32 getCurHour() { return mHour; }
	S32 getCurMin() { return mMinute; }
	S32 getCurSec() { return mSecond; }

	std::string getCurAMPM() { return mAMPM; }
	std::string getCurTZ() { return mTZ; }
	std::string getCurTimeStr() { return mTimeStr; }
	std::string getCurDateStr() { return mDateStr; }

	static bool sUse24HourTime;
	static bool sUseUTCTime;
	static bool sUseTimeOffset;
	static S32 sTimeOffset;
	static bool sTimeOffsetDST;

	void updateTimeFormat(const U32& index);

private:
	// simple method to setup the part that holds the date
	void setupDate();

	S32 mHour;
	S32 mMinute;
	S32 mSecond;
	std::string mTZ;
	std::string mAMPM;
	std::string mTimeStr;
	std::string mDay;
	S32 mDayNum;
	std::string mMonth;
	S32 mYear;
	std::string mDateStr;

	static std::vector<std::string> sDays;
	static std::vector<std::string> sMonths;
	static const U32 MAX_DATE_STRING_LENGTH;
};

extern ViewerTime* gViewerTime;

#endif // VIEWERTIME_H
