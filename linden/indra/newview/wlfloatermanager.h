/** 
 * @file wlfloaterwindlightsend.h
 * @brief WLFloaterWindLightSend class definition
 *
 * $LicenseInfo:firstyear=2007&license=viewergpl$
 * 
 * Copyright (c) 2007-2009, Linden Research, Inc.
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

/*
 * Menu for adjusting the atmospheric settings of the world
 */

#include "llfloater.h"

#include <vector>
#include "llwlparamset.h"

struct WLColorControl;
struct WLFloatControl;


/// Menuing system for all of windlight's functionality
class WLFloaterManager : public LLFloater
{
public:

	WLFloaterManager();
	virtual ~WLFloaterManager();
	
	/// initialize all
	void initCallbacks(void);

	/// one and one instance only
	static WLFloaterManager* instance();

	// help button stuff
	static void onClickHelp(void* data);
	void initHelpBtn(const std::string& name, const std::string& xml_alert);

	static void onGetThisRegion(void* userData);
	static void onGetThisParcel(void* userData);
	static void onGetAllParcels(void* userData);
	static void onShow(void* userData);
	static void onSetToCurrent(void* userData);
	static void onRemove(void* userData);
	
	//// menu management

	/// show off our menu
	static void show();

	/// return if the menu exists or not
	static bool isOpen();

	/// stuff to do on exit
	virtual void onClose(bool app_quitting);

	static void UpdateFloater();
	static std::map<std::string, LLWLParamSet*> mWLParamList;	
	static std::map<std::string, LLWaterParamSet*> mWaterParamList;	
	static std::map<std::string, LLUUID*> mWaterNormalParamList;	
	static std::map<std::string, LLSD> mMinAltParamList;	
	static std::map<std::string, LLSD> mMaxAltParamList;	
	static std::map<std::string, LLSD> mFadeParamList;	

private:
	// one instance on the inside
	static WLFloaterManager* sWindLight;
};