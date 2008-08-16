/** 
 * @file lllcd.cpp
 * @brief Lcd panel class
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


#include "llviewerprecompiledheaders.h"
#include "lllcd.h"

#include "llcontrol.h"
#include "llstatusbar.h"
#include "llviewerregion.h"
#include "llviewerstats.h"
#include "llvieweruictrlfactory.h"
#include "llviewercontrol.h"

#if LL_LCD_COMPILE

extern LLControlGroup gSavedSettings;

#define ICON_WIDTH 28

#if LL_WINDOWS
// for Logitech LCD keyboards / speakers
#include "EZ_LCD.h"
#include "lllogitechlcd.h"

LLLCD	*gLcdScreen = NULL; // Singleton-ish?

// not happy about this, but there's no local class to put it in.
LLPanel *bogus = NULL;

// oh yeah baby! Construct those LCD page group handlers - make them all and we just turn off displaying them according to the preferences set up
void CreateLCDDebugWindows()
{
	if (gLcdScreen->Enabled())
	{
		// load up the text so we are sure it's externalized and localized correctly.
		bogus = new LLPanel();
		gUICtrlFactory->buildPanel(bogus, "lcd_text.xml");

		LLLCDPageGroup *pageGroup = NULL;
		pageGroup = new llDefaultPageGroup(gLcdScreen->mLCD, LLLCD::kLCDDefault, gLcdScreen->mSLIcon);
		// push this new group onto the array
		gLcdScreen->mPageGroupArray.push_back(pageGroup);

		pageGroup = new LLChatPageGroup(gLcdScreen->mLCD, LLLCD::kLCDChat, gLcdScreen->mSLIcon);
		// push this new group onto the array
		gLcdScreen->mPageGroupArray.push_back(pageGroup);

		pageGroup = new LLIMPageGroup(gLcdScreen->mLCD, LLLCD::kLCDIM, gLcdScreen->mSLIcon);
		// push this new group onto the array
		gLcdScreen->mPageGroupArray.push_back(pageGroup);

		pageGroup = new LLDebugPageGroup(gLcdScreen->mLCD, LLLCD::kLCDDebug, gLcdScreen->mSLIcon);
		// push this new group onto the array
		gLcdScreen->mPageGroupArray.push_back(pageGroup);

		pageGroup = new LLLindenPageGroup(gLcdScreen->mLCD, LLLCD::kLCDLinden, gLcdScreen->mSLIcon);
		// push this new group onto the array
		gLcdScreen->mPageGroupArray.push_back(pageGroup);

		pageGroup = new LLRegionPageGroup(gLcdScreen->mLCD, LLLCD::kLCDRegion, gLcdScreen->mSLIcon);
		// push this new group onto the array
		gLcdScreen->mPageGroupArray.push_back(pageGroup);

		pageGroup = new LLDebugConsolePageGroup(gLcdScreen->mLCD, LLLCD::kLCDDebugConsole, gLcdScreen->mSLIcon);
		// push this new group onto the array
		gLcdScreen->mPageGroupArray.push_back(pageGroup);
	}
}

// functions that allow the chat and IM handler to insert new lines directly into the LCD page group handlers
void AddNewIMToLCD(const LLString &newLine)
{
	if (gLcdScreen->Enabled())
	{
		LLIMPageGroup *imGroup = (LLIMPageGroup *)gLcdScreen->mPageGroupArray[2];
		imGroup->InsertText(newLine);
	}
}

void AddNewChatToLCD(const LLString &newLine)
{
	if (gLcdScreen->Enabled())
	{
		LLChatPageGroup *chatGroup = (LLChatPageGroup *)gLcdScreen->mPageGroupArray[1];
		chatGroup->InsertText(newLine);
	}
}

void AddNewDebugConsoleToLCD(const LLWString &newLine)
{
	if (gLcdScreen->Enabled())
	{
 		LLDebugConsolePageGroup *debugGroup = (LLDebugConsolePageGroup *)gLcdScreen->mPageGroupArray[6];
		debugGroup->InsertText(newLine);
	}
}

/////////////////////////////////////
//
//	Debug Console Page group class
//
/////////////////////////////////////

void LLDebugConsolePageGroup::UpdateDetails()
{
	mLCD->ModifyControlsOnPage(mPageArray[0].mPageIndex); 
	mLCD->SetText(mPageArray[0].mDisplayItemArray[0], (LPCTSTR)(wstring_to_utf16str(mLine1).c_str()));
	mLCD->SetText(mPageArray[0].mDisplayItemArray[1], (LPCTSTR)(wstring_to_utf16str(mLine2).c_str()));
	mLCD->SetText(mPageArray[0].mDisplayItemArray[2], (LPCTSTR)(wstring_to_utf16str(mLine3).c_str()));
}

void LLDebugConsolePageGroup::GetDisplayable()
{
	mDisplayPage = gSavedSettings.getBOOL("DisplayDebugConsole");
}

void LLDebugConsolePageGroup::InsertText(const LLWString &newLine)
{
	mLine1 = mLine2;
	mLine2 = mLine3;
	mLine3 = newLine;
}

LLDebugConsolePageGroup::LLDebugConsolePageGroup(CEzLcd *LCD, int type, HICON SLIcon)
:LLLCDPageGroup(LCD, type, SLIcon)
{
	// create a new specific pagea
	LLLCDSpecificPage newPage;
	newPage.mPageIndex = mLCD->AddNewPage() - 1;
	mLCD->ModifyControlsOnPage(newPage.mPageIndex);

	// add in all the display parts for this specific page

	// add in the icon - all pages have this - we don't need to track this because we never update it
	HANDLE m_rightIcon = mLCD->AddIcon(mSLIcon, 32, 32);
	mLCD->SetOrigin(m_rightIcon, 0, 0);

	// add Title
	HANDLE title = mLCD->AddText(LG_STATIC_TEXT, LG_SMALL, DT_CENTER, 128);
	mLCD->SetOrigin(title, 32, 0);
	mLCD->SetText(title, _T("Debug Console"));

	// add line
	HANDLE chatLine = mLCD->AddText(LG_SCROLLING_TEXT, LG_SMALL, DT_LEFT, 160 - ICON_WIDTH);
	mLCD->SetOrigin(chatLine, ICON_WIDTH, 11);
	mLCD->SetText(chatLine, _T(""));
	newPage.mDisplayItemArray.push_back(chatLine);

	// add line
	chatLine = mLCD->AddText(LG_SCROLLING_TEXT, LG_SMALL, DT_LEFT, 160 - ICON_WIDTH);
	mLCD->SetOrigin(chatLine, ICON_WIDTH, 22);
	mLCD->SetText(chatLine, _T(""));
	newPage.mDisplayItemArray.push_back(chatLine);

	// add line
	chatLine = mLCD->AddText(LG_SCROLLING_TEXT, LG_SMALL, DT_LEFT, 160 - ICON_WIDTH);
	mLCD->SetOrigin(chatLine, ICON_WIDTH, 33);
	mLCD->SetText(chatLine, _T(""));
	newPage.mDisplayItemArray.push_back(chatLine);

	// and then insert it
	mPageArray.push_back(newPage);
}

/////////////////////////////////////
//
//	Network Comms Page group class
//
/////////////////////////////////////

void LLDebugPageGroup::UpdateDetails()
{
	mLCD->ModifyControlsOnPage(mPageArray[0].mPageIndex);
	LLString ping = llformat("1000");
	LLString packetsIn = llformat("0");
	LLString packetsOut = llformat("0");
	LLString packetLoss = llformat("0");
	LLString fps = llformat("0");
	LLString simfps = llformat("0");
	// region name
	if (gStatusBar)
	{ 
		ping = llformat("%d", gStatusBar->mRegionDetails.mPing);	
	}

	if (gViewerStats)
	{
		packetLoss = llformat("%.1f%%", gViewerStats->mPacketsLostPercentStat.getCurrent());
		simfps = llformat("%d", (int)gViewerStats->mSimFPS.getCurrent());
		fps = llformat("%.1f%", gViewerStats->mFPSStat.getMeanPerSec());
		packetsIn = llformat("%d", (int)gViewerStats->mSimInPPS.getCurrent());
		packetsOut = llformat("%d", (int)gViewerStats->mSimOutPPS.getCurrent());
	}

	// fps
	mLCD->SetText(mPageArray[0].mDisplayItemArray[0], (LPCTSTR)(utf8str_to_utf16str(fps).c_str()));
	// simfps
	mLCD->SetText(mPageArray[0].mDisplayItemArray[1], (LPCTSTR)(utf8str_to_utf16str(simfps).c_str()));
	// packets in
	mLCD->SetText(mPageArray[0].mDisplayItemArray[2], (LPCTSTR)(utf8str_to_utf16str(packetsIn).c_str()));
	// packets out
	mLCD->SetText(mPageArray[0].mDisplayItemArray[3], (LPCTSTR)(utf8str_to_utf16str(packetsOut).c_str()));
	// packet loss
	mLCD->SetText(mPageArray[0].mDisplayItemArray[4], (LPCTSTR)(utf8str_to_utf16str(packetLoss).c_str()));
	// ping
	mLCD->SetText(mPageArray[0].mDisplayItemArray[5], (LPCTSTR)(utf8str_to_utf16str(ping).c_str())); 
}

void LLDebugPageGroup::GetDisplayable()
{
	mDisplayPage = gSavedSettings.getBOOL("DisplayDebug");
}

LLDebugPageGroup::LLDebugPageGroup(CEzLcd *LCD, int type, HICON SLIcon)
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
	HANDLE title = mLCD->AddText(LG_STATIC_TEXT, LG_SMALL, DT_CENTER, 128);
	mLCD->SetOrigin(title, 32, 0);
	mLCD->SetText(title, (LPCTSTR)(utf8str_to_utf16str(bogus->childGetText("DebugInfo")).c_str()));

	HANDLE fpsStatic = mLCD->AddText(LG_STATIC_TEXT, LG_SMALL, DT_LEFT, 18);
	mLCD->SetOrigin(fpsStatic, ICON_WIDTH, 11);
	mLCD->SetText(fpsStatic, (LPCTSTR)(utf8str_to_utf16str(bogus->childGetText("FPS")).c_str()));

	HANDLE fps = mLCD->AddText(LG_STATIC_TEXT, LG_SMALL, DT_LEFT, 25);
	mLCD->SetOrigin(fps, ICON_WIDTH+25, 11);
	mLCD->SetText(fps, _T(""));
	newPage.mDisplayItemArray.push_back(fps);

	HANDLE simfpsStatic = mLCD->AddText(LG_STATIC_TEXT, LG_SMALL, DT_LEFT, 35);
	mLCD->SetOrigin(simfpsStatic, ICON_WIDTH+37+25, 11);
	mLCD->SetText(simfpsStatic, (LPCTSTR)(utf8str_to_utf16str(bogus->childGetText("SimFPS")).c_str()));

	HANDLE simfps = mLCD->AddText(LG_STATIC_TEXT, LG_SMALL, DT_LEFT, 25);
	mLCD->SetOrigin(simfps, ICON_WIDTH+37+27+37, 11);
	mLCD->SetText(simfps, _T(""));
	newPage.mDisplayItemArray.push_back(simfps);

	HANDLE packetsinStatic = mLCD->AddText(LG_STATIC_TEXT, LG_SMALL, DT_LEFT, 25);
	mLCD->SetOrigin(packetsinStatic, ICON_WIDTH, 22);
	mLCD->SetText(packetsinStatic, (LPCTSTR)(utf8str_to_utf16str(bogus->childGetText("Pin")).c_str()));

	HANDLE packetsin = mLCD->AddText(LG_STATIC_TEXT, LG_SMALL, DT_LEFT, 25);
	mLCD->SetOrigin(packetsin, ICON_WIDTH+32, 22);
	mLCD->SetText(packetsin, _T(""));
	newPage.mDisplayItemArray.push_back(packetsin);

	HANDLE packetsoutStatic = mLCD->AddText(LG_STATIC_TEXT, LG_SMALL, DT_LEFT, 30);
	mLCD->SetOrigin(packetsoutStatic, ICON_WIDTH+37+25, 22);
	mLCD->SetText(packetsoutStatic, (LPCTSTR)(utf8str_to_utf16str(bogus->childGetText("Pout")).c_str()));

	HANDLE packetsout = mLCD->AddText(LG_STATIC_TEXT, LG_SMALL, DT_LEFT, 25);
	mLCD->SetOrigin(packetsout, ICON_WIDTH+37+27+37, 22);
	mLCD->SetText(packetsout, _T(""));
	newPage.mDisplayItemArray.push_back(packetsout);

	HANDLE packetlossStatic = mLCD->AddText(LG_STATIC_TEXT, LG_SMALL, DT_LEFT, 30);
	mLCD->SetOrigin(packetlossStatic, ICON_WIDTH, 33);
	mLCD->SetText(packetlossStatic, (LPCTSTR)(utf8str_to_utf16str(bogus->childGetText("PLoss")).c_str()));

	HANDLE packetloss = mLCD->AddText(LG_STATIC_TEXT, LG_SMALL, DT_LEFT, 35);
	mLCD->SetOrigin(packetloss, ICON_WIDTH+33, 33);
	mLCD->SetText(packetloss, _T(""));
	newPage.mDisplayItemArray.push_back(packetloss);

	HANDLE pingStatic = mLCD->AddText(LG_STATIC_TEXT, LG_SMALL, DT_LEFT, 20);
	mLCD->SetOrigin(pingStatic, ICON_WIDTH+32+38, 33);
	mLCD->SetText(pingStatic,(LPCTSTR)(utf8str_to_utf16str(bogus->childGetText("Ping")).c_str()));

	HANDLE ping = mLCD->AddText(LG_STATIC_TEXT, LG_SMALL, DT_LEFT, 30);
	mLCD->SetOrigin(ping, ICON_WIDTH+37+27+37, 33);
	mLCD->SetText(ping, _T(""));
	newPage.mDisplayItemArray.push_back(ping);

	// and then insert it
	mPageArray.push_back(newPage);
}

/////////////////////////////////////
//
//	Linden Account group class
//
/////////////////////////////////////

void LLLindenPageGroup::UpdateDetails()
{
	mLCD->ModifyControlsOnPage(mPageArray[0].mPageIndex);
	LLString time = llformat("Unknown");
	LLString balance = llformat("Unknown");
	// region name
	if (gStatusBar)
	{ 
		time = gStatusBar->mRegionDetails.mTime;
		balance = llformat("%d", gStatusBar->mRegionDetails.mBalance);
	}

	// time name
	mLCD->SetText(mPageArray[0].mDisplayItemArray[0], (LPCTSTR)(utf8str_to_utf16str(time).c_str()));
	// balance
	mLCD->SetText(mPageArray[0].mDisplayItemArray[1], (LPCTSTR)(utf8str_to_utf16str(balance).c_str()));
}

void LLLindenPageGroup::GetDisplayable()
{
	mDisplayPage = gSavedSettings.getBOOL("DisplayLinden");
}

LLLindenPageGroup::LLLindenPageGroup(CEzLcd *LCD, int type, HICON SLIcon)
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
	HANDLE title = mLCD->AddText(LG_STATIC_TEXT, LG_SMALL, DT_CENTER, 128);
	mLCD->SetOrigin(title, 32, 0);
	mLCD->SetText(title, (LPCTSTR)(utf8str_to_utf16str(bogus->childGetText("AccountDetails")).c_str()));

	HANDLE timeStatic = mLCD->AddText(LG_STATIC_TEXT, LG_SMALL, DT_LEFT, 30);
	mLCD->SetOrigin(timeStatic, ICON_WIDTH, 11);
	mLCD->SetText(timeStatic, (LPCTSTR)(utf8str_to_utf16str(bogus->childGetText("Time")).c_str()));

	HANDLE time = mLCD->AddText(LG_STATIC_TEXT, LG_SMALL, DT_LEFT, 160 - ICON_WIDTH - 32);
	mLCD->SetOrigin(time, ICON_WIDTH+32, 11);
	mLCD->SetText(time, _T(""));
	newPage.mDisplayItemArray.push_back(time);

	HANDLE balanceStatic = mLCD->AddText(LG_STATIC_TEXT, LG_SMALL, DT_LEFT, 30);
	mLCD->SetOrigin(balanceStatic, ICON_WIDTH, 22);
	mLCD->SetText(balanceStatic, (LPCTSTR)(utf8str_to_utf16str(bogus->childGetText("LBal")).c_str()));

	HANDLE balance = mLCD->AddText(LG_STATIC_TEXT, LG_SMALL, DT_LEFT, 160 - ICON_WIDTH - 35);
	mLCD->SetOrigin(balance, ICON_WIDTH+35, 22);
	mLCD->SetText(balance, _T(""));
	newPage.mDisplayItemArray.push_back(balance);

	// and then insert it
	mPageArray.push_back(newPage);
}

/////////////////////////////////////
//
//	Region Group page
//
/////////////////////////////////////

void LLRegionPageGroup::UpdateDetails()
{
	LLString pos = llformat("Unknown");
	LLString parcel = llformat("Unknown");
	LLString region = llformat("Unknown");
	LLString owner = llformat("Unknown");
	LLString forsale = bogus->childGetText("No");
	LLString rtype = llformat("Unknown");
	LLString sqm = llformat("0");
	LLString traffic = llformat("0");

	// region name
	if (gStatusBar)
	{
		pos = llformat(" %d, %d, %d", gStatusBar->mRegionDetails.mX, gStatusBar->mRegionDetails.mY, gStatusBar->mRegionDetails.mZ);
		parcel = llformat("%s", gStatusBar->mRegionDetails.mParcelName);
		region = gStatusBar->mRegionDetails.mRegionName;
		rtype = llformat("%s", gStatusBar->mRegionDetails.mAccesString);
		sqm = llformat("%d", gStatusBar->mRegionDetails.mArea);
		if (gStatusBar->mRegionDetails.mForSale)
		{
			forsale = bogus->childGetText("Yes");
		}
		owner = llformat("%s", gStatusBar->mRegionDetails.mOwner);
		traffic = llformat("%d", (int)gStatusBar->mRegionDetails.mTraffic);
	}

	// update first page
	mLCD->ModifyControlsOnPage(mPageArray[0].mPageIndex);
	// region name
	mLCD->SetText(mPageArray[0].mDisplayItemArray[0], (LPCTSTR)(utf8str_to_utf16str(region).c_str()));
	// parcel name
	mLCD->SetText(mPageArray[0].mDisplayItemArray[1], (LPCTSTR)(utf8str_to_utf16str(parcel).c_str()));
	// position name
	mLCD->SetText(mPageArray[0].mDisplayItemArray[2], (LPCTSTR)(utf8str_to_utf16str(pos).c_str()));
	// Sqm
	mLCD->SetText(mPageArray[0].mDisplayItemArray[3], (LPCTSTR)(utf8str_to_utf16str(sqm).c_str()));

	// update second page
	mLCD->ModifyControlsOnPage(mPageArray[1].mPageIndex);
	// owner
	mLCD->SetText(mPageArray[1].mDisplayItemArray[0], (LPCTSTR)(utf8str_to_utf16str(owner).c_str()));
	// access type
	mLCD->SetText(mPageArray[1].mDisplayItemArray[1], (LPCTSTR)(utf8str_to_utf16str(rtype).c_str()));
	// forsale
	mLCD->SetText(mPageArray[1].mDisplayItemArray[2], (LPCTSTR)(utf8str_to_utf16str(forsale).c_str()));
	// traffic
	mLCD->SetText(mPageArray[1].mDisplayItemArray[3], (LPCTSTR)(utf8str_to_utf16str(traffic).c_str()));

}

void LLRegionPageGroup::GetDisplayable()
{
	mDisplayPage = gSavedSettings.getBOOL("DisplayRegion");
}

LLRegionPageGroup::LLRegionPageGroup(CEzLcd *LCD, int type, HICON SLIcon)
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
	HANDLE title = mLCD->AddText(LG_STATIC_TEXT, LG_SMALL, DT_CENTER, 128);
	mLCD->SetOrigin(title, 32, 0);
	mLCD->SetText(title, (LPCTSTR)(utf8str_to_utf16str(bogus->childGetText("LocationDetails1")).c_str()));

	HANDLE regionNameStatic = mLCD->AddText(LG_STATIC_TEXT, LG_SMALL, DT_LEFT, 30);
	mLCD->SetOrigin(regionNameStatic, ICON_WIDTH, 11);
	mLCD->SetText(regionNameStatic, (LPCTSTR)(utf8str_to_utf16str(bogus->childGetText("Region")).c_str()));

	HANDLE regionName = mLCD->AddText(LG_SCROLLING_TEXT, LG_SMALL, DT_LEFT, 160 - ICON_WIDTH - 30 - 2);
	mLCD->SetOrigin(regionName, ICON_WIDTH+32, 11);
	mLCD->SetText(regionName, _T(""));
	newPage.mDisplayItemArray.push_back(regionName);

	HANDLE parcelStatic = mLCD->AddText(LG_STATIC_TEXT, LG_SMALL, DT_LEFT, 30);
	mLCD->SetOrigin(parcelStatic, ICON_WIDTH, 22);
	mLCD->SetText(parcelStatic, (LPCTSTR)(utf8str_to_utf16str(bogus->childGetText("Parcel")).c_str()));

	HANDLE parcel = mLCD->AddText(LG_SCROLLING_TEXT, LG_SMALL, DT_CENTER, 160 - ICON_WIDTH - 30 - 2);
	mLCD->SetOrigin(parcel, ICON_WIDTH+32, 22);
	mLCD->SetText(parcel, _T(""));
	newPage.mDisplayItemArray.push_back(parcel);

	HANDLE positionStatic = mLCD->AddText(LG_STATIC_TEXT, LG_SMALL, DT_LEFT, 18);
	mLCD->SetOrigin(positionStatic, 0, 33);
	mLCD->SetText(positionStatic, (LPCTSTR)(utf8str_to_utf16str(bogus->childGetText("Parcel")).c_str()));

	HANDLE position = mLCD->AddText(LG_STATIC_TEXT, LG_SMALL, DT_LEFT, 60);
	mLCD->SetOrigin(position, 20, 33);
	mLCD->SetText(position, _T("100, 100, 100"));
	newPage.mDisplayItemArray.push_back(position);

	HANDLE sqmStatic = mLCD->AddText(LG_STATIC_TEXT, LG_SMALL, DT_LEFT, 18);
	mLCD->SetOrigin(sqmStatic, 90, 33);
	mLCD->SetText(sqmStatic, (LPCTSTR)(utf8str_to_utf16str(bogus->childGetText("Sqm")).c_str()));

	HANDLE sqm = mLCD->AddText(LG_STATIC_TEXT, LG_SMALL, DT_LEFT, 40);
	mLCD->SetOrigin(sqm, 90 + 20, 33);
	mLCD->SetText(sqm, _T("1000"));
	newPage.mDisplayItemArray.push_back(sqm);

	// and then insert it
	mPageArray.push_back(newPage);

	// create a new specific page
	LLLCDSpecificPage newPage2;
	newPage2.mPageIndex = mLCD->AddNewPage() - 1;
	mLCD->ModifyControlsOnPage(newPage2.mPageIndex);

	// add in all the display parts for this specific page

	// add in the icon - all pages have this - we don't need to track this because we never update it
	m_rightIcon = mLCD->AddIcon(mSLIcon, 32, 32);
	mLCD->SetOrigin(m_rightIcon, 0, 0);

	// add Title
	title = mLCD->AddText(LG_STATIC_TEXT, LG_SMALL, DT_CENTER, 128);
	mLCD->SetOrigin(title, 32, 0);
	mLCD->SetText(title, (LPCTSTR)(utf8str_to_utf16str(bogus->childGetText("LocationDetails2")).c_str()));

	HANDLE ownerStatic = mLCD->AddText(LG_STATIC_TEXT, LG_SMALL, DT_LEFT, 30);
	mLCD->SetOrigin(ownerStatic, ICON_WIDTH, 11);
	mLCD->SetText(ownerStatic, (LPCTSTR)(utf8str_to_utf16str(bogus->childGetText("Owner")).c_str()));

	HANDLE owner = mLCD->AddText(LG_SCROLLING_TEXT, LG_SMALL, DT_LEFT, 160 - 30 - ICON_WIDTH - 2);
	mLCD->SetOrigin(owner, ICON_WIDTH+32, 11);
	mLCD->SetText(owner, _T(""));
	newPage2.mDisplayItemArray.push_back(owner);

	HANDLE typeStatic = mLCD->AddText(LG_STATIC_TEXT, LG_SMALL, DT_LEFT, 30);
	mLCD->SetOrigin(typeStatic, ICON_WIDTH, 22);
	mLCD->SetText(typeStatic, (LPCTSTR)(utf8str_to_utf16str(bogus->childGetText("Type")).c_str()));

	HANDLE rtype = mLCD->AddText(LG_STATIC_TEXT, LG_SMALL, DT_LEFT, 160 - 30 - ICON_WIDTH - 2);
	mLCD->SetOrigin(rtype, ICON_WIDTH+32, 22);
	mLCD->SetText(rtype, _T(""));
	newPage2.mDisplayItemArray.push_back(rtype);

	HANDLE forsaleStatic = mLCD->AddText(LG_STATIC_TEXT, LG_SMALL, DT_LEFT, 40);
	mLCD->SetOrigin(forsaleStatic, 0, 33);
	mLCD->SetText(forsaleStatic, (LPCTSTR)(utf8str_to_utf16str(bogus->childGetText("Forsale")).c_str()));

	HANDLE forsale = mLCD->AddText(LG_STATIC_TEXT, LG_SMALL, DT_LEFT, 30);
	mLCD->SetOrigin(forsale, 42, 33);
	mLCD->SetText(forsale, (LPCTSTR)(utf8str_to_utf16str(bogus->childGetText("No")).c_str()));
	newPage2.mDisplayItemArray.push_back(forsale);

	HANDLE trafficStatic = mLCD->AddText(LG_STATIC_TEXT, LG_SMALL, DT_LEFT, 50);
	mLCD->SetOrigin(trafficStatic, 70, 33);
	mLCD->SetText(trafficStatic, (LPCTSTR)(utf8str_to_utf16str(bogus->childGetText("Traffic")).c_str()));

	HANDLE traffic = mLCD->AddText(LG_STATIC_TEXT, LG_SMALL, DT_LEFT, 30);
	mLCD->SetOrigin(traffic, 122, 33);
	mLCD->SetText(traffic, _T("200"));
	newPage2.mDisplayItemArray.push_back(traffic);

	// and then insert it
	mPageArray.push_back(newPage2);
}

/////////////////////////////////////
//
//	Chat group Page class
//
/////////////////////////////////////

void LLChatPageGroup::UpdateDetails()
{
	mLCD->ModifyControlsOnPage(mPageArray[0].mPageIndex);
	mLCD->SetText(mPageArray[0].mDisplayItemArray[0], (LPCTSTR)(utf8str_to_utf16str(mLine1).c_str()));
	mLCD->SetText(mPageArray[0].mDisplayItemArray[1], (LPCTSTR)(utf8str_to_utf16str(mLine2).c_str()));
	mLCD->SetText(mPageArray[0].mDisplayItemArray[2], (LPCTSTR)(utf8str_to_utf16str(mLine3).c_str()));
}

void LLChatPageGroup::GetDisplayable()
{
	mDisplayPage = gSavedSettings.getBOOL("DisplayChat");
}

void LLChatPageGroup::InsertText(const LLString &newLine)
{
	mLine1 = mLine2;
	mLine2 = mLine3;
	mLine3 = newLine;
}

LLChatPageGroup::LLChatPageGroup(CEzLcd *LCD, int type, HICON SLIcon)
:LLLCDPageGroup(LCD, type, SLIcon)
{
	mLine1 = llformat("");
	mLine2 = llformat("");
	mLine3 = llformat("");
	// create a new specific page
	LLLCDSpecificPage newPage;
	newPage.mPageIndex = mLCD->AddNewPage() - 1;
	mLCD->ModifyControlsOnPage(newPage.mPageIndex);

	// add in all the display parts for this specific page

	// add in the icon - all pages have this - we don't need to track this because we never update it
	HANDLE m_rightIcon = mLCD->AddIcon(mSLIcon, 32, 32);
	mLCD->SetOrigin(m_rightIcon, 0, 0);

	// add Title
	HANDLE title = mLCD->AddText(LG_STATIC_TEXT, LG_SMALL, DT_CENTER, 128);
	mLCD->SetOrigin(title, 32, 0);
	mLCD->SetText(title, (LPCTSTR)(utf8str_to_utf16str(bogus->childGetText("Last3ChatLines")).c_str()));

	// add line
	HANDLE chatLine = mLCD->AddText(LG_SCROLLING_TEXT, LG_SMALL, DT_LEFT, 160 - ICON_WIDTH);
	mLCD->SetOrigin(chatLine, ICON_WIDTH, 11);
	mLCD->SetText(chatLine, _T(""));
	newPage.mDisplayItemArray.push_back(chatLine);

	// add line
	chatLine = mLCD->AddText(LG_SCROLLING_TEXT, LG_SMALL, DT_LEFT, 160 - ICON_WIDTH);
	mLCD->SetOrigin(chatLine, ICON_WIDTH, 22);
	mLCD->SetText(chatLine, _T(""));
	newPage.mDisplayItemArray.push_back(chatLine);

	// add line
	chatLine = mLCD->AddText(LG_SCROLLING_TEXT, LG_SMALL, DT_LEFT, 160 - ICON_WIDTH);
	mLCD->SetOrigin(chatLine, ICON_WIDTH, 33);
	mLCD->SetText(chatLine, _T(""));
	newPage.mDisplayItemArray.push_back(chatLine);

	// and then insert it
	mPageArray.push_back(newPage);
}

/////////////////////////////////////
//
//	IM Page Group class
//
/////////////////////////////////////

void LLIMPageGroup::UpdateDetails()
{
	mLCD->ModifyControlsOnPage(mPageArray[0].mPageIndex);
	mLCD->SetText(mPageArray[0].mDisplayItemArray[0], (LPCTSTR)(utf8str_to_utf16str(mLine1).c_str()));
	mLCD->SetText(mPageArray[0].mDisplayItemArray[1], (LPCTSTR)(utf8str_to_utf16str(mLine2).c_str()));
	mLCD->SetText(mPageArray[0].mDisplayItemArray[2], (LPCTSTR)(utf8str_to_utf16str(mLine3).c_str()));
}

void LLIMPageGroup::GetDisplayable()
{
	mDisplayPage = gSavedSettings.getBOOL("DisplayIM");
}

void LLIMPageGroup::InsertText(const LLString &newLine)
{
	mLine1 = mLine2;
	mLine2 = mLine3;
	mLine3 = newLine;
}

LLIMPageGroup::LLIMPageGroup(CEzLcd *LCD, int type, HICON SLIcon)
:LLLCDPageGroup(LCD, type, SLIcon){
	mLine1 = llformat("");
	mLine2 = llformat("");
	mLine3 = llformat("");
	// create a new specific page
	LLLCDSpecificPage newPage;
	newPage.mPageIndex = mLCD->AddNewPage() - 1;
	mLCD->ModifyControlsOnPage(newPage.mPageIndex);

	// add in all the display parts for this specific page

	// add in the icon - all pages have this - we don't need to track this because we never update it
	HANDLE m_rightIcon = mLCD->AddIcon(mSLIcon, 32, 32);
	mLCD->SetOrigin(m_rightIcon, 0, 0);

	// add Title
	HANDLE title = mLCD->AddText(LG_STATIC_TEXT, LG_SMALL, DT_CENTER, 128);
	mLCD->SetOrigin(title, 32, 0);
	mLCD->SetText(title, (LPCTSTR)(utf8str_to_utf16str(bogus->childGetText("Last3IMLines")).c_str()));

	// add line
	HANDLE chatLine = mLCD->AddText(LG_SCROLLING_TEXT, LG_SMALL, DT_LEFT, 160 - ICON_WIDTH);
	mLCD->SetOrigin(chatLine, ICON_WIDTH, 11);
	mLCD->SetText(chatLine, _T(""));
	newPage.mDisplayItemArray.push_back(chatLine);

	// add line
	chatLine = mLCD->AddText(LG_SCROLLING_TEXT, LG_SMALL, DT_LEFT, 160 - ICON_WIDTH);
	mLCD->SetOrigin(chatLine, ICON_WIDTH, 22);
	mLCD->SetText(chatLine, _T(""));
	newPage.mDisplayItemArray.push_back(chatLine);

	// add line
	chatLine = mLCD->AddText(LG_SCROLLING_TEXT, LG_SMALL, DT_LEFT, 160 - ICON_WIDTH);
	mLCD->SetOrigin(chatLine, ICON_WIDTH, 33);
	mLCD->SetText(chatLine, _T(""));
	newPage.mDisplayItemArray.push_back(chatLine);

	// and then insert it
	mPageArray.push_back(newPage);
}

#endif //LL_WINDOWS
#endif //LL_LCD_COMPILE