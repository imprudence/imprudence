/** 
 * @file lllcd.h
 * @brief Description of the LCDdisplay class.
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
#define LL_LLLCD_H

#ifndef LL_LOGITECH_LCD_H
#include "lllogitechlcd.h"
#endif


// class that defines the specific Debug page group of pages
class LLDebugPageGroup : public LLLCDPageGroup
{
public:
	LLDebugPageGroup(CEzLcd *LCD, int type, HICON SLIcon);
	virtual void UpdateDetails();
	virtual void GetDisplayable();
};

// class that defines the specific Debug Console group of pages
class LLDebugConsolePageGroup : public LLLCDPageGroup
{
public:
	LLDebugConsolePageGroup(CEzLcd *LCD, int type, HICON SLIcon);
	virtual void UpdateDetails();
	virtual void GetDisplayable();
	void InsertText(const LLWString &newLine);
private:
	LLWString mLine1;
	LLWString mLine2;
	LLWString mLine3;
};

// class that defines the specific Linden Account group of pages
class LLLindenPageGroup : public LLLCDPageGroup
{
public:
	LLLindenPageGroup(CEzLcd *LCD, int type, HICON SLIcon);
	virtual void UpdateDetails();
	virtual void GetDisplayable();
};

// class that defines the specific Chat group of pages
class LLChatPageGroup : public LLLCDPageGroup
{
public:
	LLChatPageGroup(CEzLcd *LCD, int type, HICON SLIcon);
	virtual void UpdateDetails();
	virtual void GetDisplayable();
	void InsertText(const LLString &newLine);
private:
	LLString mLine1;
	LLString mLine2;
	LLString mLine3;
};

// class that defines the specific IM group of pages
class LLIMPageGroup : public LLLCDPageGroup
{
public:
	LLIMPageGroup(CEzLcd *LCD, int type, HICON SLIcon);
	virtual void UpdateDetails();
	virtual void GetDisplayable();
	void InsertText(const LLString &newLine);
private:
	LLString mLine1;
	LLString mLine2;
	LLString mLine3;
};

// class that defines the specific Region group of pages
class LLRegionPageGroup : public LLLCDPageGroup
{
public:
	LLRegionPageGroup(CEzLcd *LCD, int type, HICON SLIcon);
	virtual void UpdateDetails();
	virtual void GetDisplayable();
};

class LLLCD;

// The following symbols are exported from lllcd.cpp
extern LLLCD	*gLcdScreen; 
extern void CreateLCDDebugWindows();

//#endif
