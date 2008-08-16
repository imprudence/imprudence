/** 
 * @file lcd.cpp
 * @brief cLcd panel class
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

#include "linden_common.h"

#if LL_LCD_COMPILE

#include "EZ_LCD.h"
#include "../newview/res/resource.h"
#include "llcontrol.h"
extern LLControlGroup gSavedSettings;

#ifndef LL_LOGITECH_LCD_H
#include "lllogitechlcd.h"
#endif

#define WAIT_DURATION 7
#define DEBOUNCE_DURATION 0.3f

LLLCDPageGroup::LLLCDPageGroup(CEzLcd *LCD, int type, HICON SLIcon):
mType(type),
mSLIcon(SLIcon),
mDisplayPage(false),
mLCD(LCD)
{
	mPageArray.clear();	
}

LLLCDPageGroup::~LLLCDPageGroup()
{
	mPageArray.clear();
}

void llDefaultPageGroup::UpdateDetails()
{
	mLCD->ModifyControlsOnPage(mPageArray[0].mPageIndex);
}

void llDefaultPageGroup::GetDisplayable()
{

}

llDefaultPageGroup::llDefaultPageGroup(CEzLcd *LCD, int type, HICON SLIcon)
:LLLCDPageGroup(LCD, type, SLIcon)
{
	// create a new specific page
	LLLCDSpecificPage newPage;
	newPage.mPageIndex = mLCD->AddNewPage() - 1;
	mLCD->ModifyControlsOnPage(newPage.mPageIndex);

	// add in all the display parts for this specific page

	// add in the icon - all pages have this - we don't need to track this because we never update it
	HANDLE m_rightIcon = mLCD->AddIcon(mSLIcon, 32, 32);
	mLCD->SetOrigin(m_rightIcon, 0, 0);

	// add Title
	HANDLE title = mLCD->AddText(LG_STATIC_TEXT, LG_MEDIUM, DT_CENTER, 128);
	mLCD->SetOrigin(title, 32, 20);
	mLCD->SetText(title, _T("Second Life Display"));
	newPage.mDisplayItemArray.push_back(title);

	// and then insert it
	mPageArray.push_back(newPage);
}

LLLCD::LLLCD(HINSTANCE instance):
mInited(false),
mDisplayTimer(),
mDebounceTimer(),
mPageToShow(-1),
mInstance(instance),
mDestinationLCD(-1),
mFirstTimeThru(true)
{
	HRESULT     res_ = S_OK;

	// Create instance of EzLcd.
	mLCD = new CEzLcd();

	// Have it initialize itself
	res_ = mLCD->InitYourself(_T("Second Life"));

	if (res_ != S_OK)
	{
		// Something went wrong, when connecting to the LCD Manager software. We need to get out now
		delete mLCD;
		return;
	}
	mInited = true;

	// preload the Second Life Icon
	mSLIcon = static_cast<HICON>(LoadImage(mInstance,
		MAKEINTRESOURCE(IDI_LCD_LL_ICON),
		IMAGE_ICON, 
		32, 
		32, 
		LR_MONOCHROME));

	// have to do this first so screens are added to the list and updateDisplay actually does something.
	mLCD->Update();
}

LLLCD::~LLLCD()
{
	// remove the instance of the LCD controller
	if (mInited == true)
	{
		delete mLCD;

		// free up the used pages
		int loopSize = mPageGroupArray.size();
		for(int i= 0; i<loopSize; i++)
		{
			free (mPageGroupArray[i]);
		}
	}
}

LLLCDPageGroup *LLLCD::GetNextPageToDisplay()
{
	// find group with current page in it.

	int groupSize = mPageGroupArray.size();
	for(int x=1; x< groupSize; x++)
	{
		if (mPageGroupArray[x]->mDisplayPage)
		{
			// now walk all the pages in the page group we are looking at
			int numPages = mPageGroupArray[x]->mPageArray.size();
			for (int zx = 0; zx< numPages; zx++)
			{
				// found it.
				if (mPageToShow == mPageGroupArray[x]->mPageArray[zx].mPageIndex)
				{
					// move to the next one
					if (zx < numPages-1)
					{
						mPageToShow = mPageGroupArray[x]->mPageArray[zx+1].mPageIndex;
						return mPageGroupArray[x];
					}
					else
					{
						for(int y=x+1; y< groupSize; y++)
						{
							if (mPageGroupArray[y]->mDisplayPage)
							{
								mPageToShow = mPageGroupArray[y]->mPageArray[0].mPageIndex;
								return mPageGroupArray[y];
							}
						}
					}
				}
			}
		}
	}

	// ok, didn't find it. Start again at the beginning and find the first group that's enabled
	for(int x=1; x< groupSize; x++)
	{
		if (mPageGroupArray[x]->mDisplayPage)
		{
			mPageToShow = mPageGroupArray[x]->mPageArray[0].mPageIndex;
			return mPageGroupArray[x];
		}
	}
	// if we got this far, we should display the default screen
	mPageToShow = mPageGroupArray[0]->mPageArray[0].mPageIndex;
	return mPageGroupArray[0];
}

void LLLCD::SetUpDisplayPages()
{
	// work out if destination has changed
	int destinationLCD = gSavedSettings.getS32("LCDDestination");
	switch(destinationLCD)
	{
	case 0:
		destinationLCD = LGLCD_DEVICE_FAMILY_KEYBOARD_G15;
		break;
	case 1:
		destinationLCD = LGLCD_DEVICE_FAMILY_SPEAKERS_Z10;
		break;
	}
	// set destination if it's changed
	if (mDestinationLCD != destinationLCD)
	{
		mDestinationLCD = destinationLCD;
		mLCD->SetDeviceFamilyToUse(destinationLCD);
	}
	int loopSize = mPageGroupArray.size();
	for(int i= 0; i<loopSize; i++)
	{
		mPageGroupArray[i]->GetDisplayable();
	}
}

void LLLCD::UpdateDisplay()
{
	if (mInited)
	{
		// reset pages if anything has changed
		SetUpDisplayPages();
		if (mLCD->IsConnected())
		{		
			// Switching back and forth between the pages that we've created
			if (mDisplayTimer.getElapsedTimeF32() > WAIT_DURATION || mFirstTimeThru)
			{
				mCurrentGroupBeingShown = GetNextPageToDisplay();
				mDisplayTimer.reset();
				mFirstTimeThru = false;
			}

			// Check if button 1 triggered
			if ((mLCD->ButtonIsPressed(LG_BUTTON_1)
				|| mLCD->ButtonIsPressed(LG_BUTTON_2)
				|| mLCD->ButtonIsPressed(LG_BUTTON_3)
				|| mLCD->ButtonIsPressed(LG_BUTTON_4)
				) && mDebounceTimer.getElapsedTimeF32() > DEBOUNCE_DURATION)
			{
				// if so, move us on a page
				mCurrentGroupBeingShown = GetNextPageToDisplay();
				mDisplayTimer.reset();
				mDebounceTimer.reset();
			}

			// update that which is being show
			mCurrentGroupBeingShown->UpdateDetails();
			
			// set which page is shown
			mLCD->ShowPage(mPageToShow);

			// Must run the following every loop.
			mLCD->Update();
		}
	}
}


// accessor functions
bool LLLCD::Enabled()
{
	return mInited;
}

BOOL LLLCD::AreZ10Available()
{
	if (mInited == true)
	{
		return mLCD->AnyDeviceOfThisFamilyPresent(LGLCD_DEVICE_FAMILY_SPEAKERS_Z10);
	}
	return false;
}

BOOL LLLCD::IsG15Available()
{
	if (mInited == true)
	{
		return mLCD->AnyDeviceOfThisFamilyPresent(LGLCD_DEVICE_FAMILY_KEYBOARD_G15);
	}
	return false;
}

#endif
