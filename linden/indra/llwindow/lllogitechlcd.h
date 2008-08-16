/** 
 * @file lcd.h
 * @brief Description of the LCD owner class.
 *
 * $LicenseInfo:firstyear=2001&license=viewergpl$
 * 
 * Copyright (c) 2001-2008, Linden Research, Inc.
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
 * online at http://secondlifegrid.net/programs/open_source/licensing/flossexception
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

#ifndef LL_LOGITECH_LCD_H
#define LL_LOGITECH_LCD_H

#include <vector>
#include "llFrametimer.h"

class CEzLcd;

// TO Add a new display page groups, duplicate the Region or Debug or Chat class, then modify what's actually displayed (ie how many pages and whats in them)
// in the constructor and update display functions.
// Remember to add in a new enum type for the new display type, and add in functions to actually create/update or remove this page in the SetupDisplayPages function
// And, of course, add the new option to the Menu in llpanelLCDoption.xml, and save details (duplicate the ones for debug or region).

// this defines one pages worth of display items.
class LLLCDSpecificPage
{
public:
	int mPageIndex;
	typedef std::vector<HANDLE> displayItemArray_t;
	// array of indexes that come from the lcd display sdk for specific items being displayed on a given page
	displayItemArray_t mDisplayItemArray;
};

// this defines a group of pages - associated with a specific type of display, like Debug, Linden Account info, region etc
// NOTE this can have more than one page associated with it - so it has an array of cLCDSpecificPage for each page.
class LLLCDPageGroup
{
public:
	LLLCDPageGroup(CEzLcd *LCD, int type, HICON SLIcon);
	virtual ~LLLCDPageGroup();
	virtual void UpdateDetails() = 0; 
	virtual void GetDisplayable() = 0;
	int		mType;
	HANDLE	mHandle;
	typedef std::vector<LLLCDSpecificPage> pageArray_t;
	pageArray_t mPageArray;
	BOOL	mDisplayPage;
protected:

	CEzLcd	*mLCD;
	HICON	mSLIcon;
};


// class that defines the Default page - used if nothing else is enabled
class llDefaultPageGroup : public LLLCDPageGroup
{
public:
	llDefaultPageGroup(CEzLcd *LCD, int type, HICON SLIcon);
	virtual void UpdateDetails();
	virtual void GetDisplayable();
};

// Root class - contains pointers to actual LCD display object, and arrays of page groups to be displayed
class LLLCD
{
public:
	enum
	{
		kLCDDefault,
		kLCDDebug,
		kLCDChat,
		kLCDRegion,
		kLCDLinden,
		kLCDIM,
		kLCDDebugConsole,
		kMaxLCDPageGroups
	};
	LLLCD(HINSTANCE instance);
	~LLLCD();
	void UpdateDisplay();
	bool Enabled();
	BOOL AreZ10Available();
	BOOL IsG15Available();

	typedef std::vector<LLLCDPageGroup *> pageGroupArray_t;
	pageGroupArray_t mPageGroupArray;
	HICON mSLIcon;
	CEzLcd	*mLCD;
private:

	// member functions to display data
	void SetUpDisplayPages();
	LLLCDPageGroup *GetNextPageToDisplay();

	LLLCDPageGroup *mCurrentGroupBeingShown;

	// members
	bool mInited;
	
	int	mPageToShow;
	LLFrameTimer	mDisplayTimer;
	LLFrameTimer	mDebounceTimer;
	HINSTANCE mInstance;		// necessary for loading some icons for the dot matrix LCD display to use
	bool mFirstTimeThru;


	// actual display set up variables
	int mDestinationLCD;
};



#endif
