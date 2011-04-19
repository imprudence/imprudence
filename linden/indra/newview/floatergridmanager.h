/**
* @file floatergridmanager.h
* @brief UI for managing grid information
*
* $LicenseInfo:firstyear=2011&license=viewergpl$
*
* Copyright (c) 2011, McCabe Maxsted
* based on Meerkat's grid manager by Patrick Sapinski
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

#ifndef PL_floaterlogin_H
#define PL_floaterlogin_H

#include "llfloater.h"

class FloaterGridManager : public LLFloater, public LLFloaterSingleton<FloaterGridManager>
{
public:
	FloaterGridManager(const LLSD& key);
	virtual ~FloaterGridManager();
	
	/*virtual*/ BOOL postBuild();

	static void refreshGrids();
	void apply();
	void cancel();

	virtual void draw();

	void refresh();

	// clears either the loginuri fetched info or all the info in the grid manager
	void clearGridInfo(bool clear_all);
	
	//static void getLocation(std::string &location);
	//void refreshLocation(bool force_visible)
	static BOOL isGridComboDirty();

private:

	enum EGridState 
	{ 
		GRID_STATE_NORMAL, 
		GRID_STATE_NEW, 
		GRID_STATE_COPY 
	};

	void setGridState(EGridState state) { mState = state; }
	EGridState getGridState() { return mState; }
	EGridState mState;
	std::string mCurGrid;

	void setCurGrid(std::string grid) { mCurGrid = grid; }
	std::string getCurGrid() { return mCurGrid; }
	
	void applyChanges();
	void createNewGrid();
	void setupNewGridEntry();
	void setupCopyGridEntry();
	void update();
	void retrieveGridInfo();

	static void onSelectGrid(LLUICtrl* ctrl, void* data);
	static void onClickDelete(void* data);
	static void onClickAdd(void* data);
	static void onClickCopy(void* data);
	static void onClickOk(void* data);
	static void onClickApply(void* data);
	static void onClickGridInfo(void* data);
	static void onClickCancel(void* data);
	static void onClickClear(void* data);

	static bool newGridCreationCallback(const LLSD& notification, const LLSD& response);
};

#endif // PL_floaterlogin_H
