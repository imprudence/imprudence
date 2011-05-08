/**
 * @file llfloaterteleporthistory.h
 * @author Zi Ree
 * @brief LLFloaterTeleportHistory class definition
 *
 * This class implements a floater where all visited teleport locations are
 * stored, so the resident can quickly see where they have been and go back
 * by selecting the location from the list.
 *
 * $LicenseInfo:firstyear=2007&license=viewergpl$
 *
 * Copyright (c) 2008, Linden Research, Inc.
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

#ifndef LL_LLFLOATERTELEPORTHISTORY_H
#define LL_LLFLOATERTELEPORTHISTORY_H

#include "linden_common.h"

#include "lldefs.h"
#include "llfloater.h"
#include "llscrolllistctrl.h"

class LLFloaterTeleportHistory : public LLFloater
{
	public:
		LLFloaterTeleportHistory();
		virtual ~LLFloaterTeleportHistory();

		/// @brief: reimplemented to check for selection changes in the places list scrolllist
		virtual void onFocusReceived();

		/// @brief: reimplemented to make the menu toggle work
		virtual void onClose(bool app_quitting);

		/// @brief: reimplemented to prevent this floater from closing while the viewer is shutting down
		virtual BOOL canClose();

		BOOL postBuild();

		/// @brief: adds a teleport destination to the list of visited places
		void addEntry(std::string regionName, S16 x, S16 y, S16 z,bool outList);

		void clearHistory();

	protected:
		static void onInPlacesSelected(LLUICtrl* ctrl, void* data);
		static void onOutPlacesSelected(LLUICtrl* ctrl, void* data);
		static void onClearHistory(void* data);
		static void onTeleport(void* data);
		static void onShowOnMap(void* data);
		static void onCopySLURL(void* data);

		/// @brief: enables or disables the "Teleport", "Show On Map" and "Copy To SLURL" buttons **/
		void setButtonsEnabled(BOOL on);

		void loadEntrys();
		void saveEntry(LLSD toSave);
		std::string getFileName();

		LLScrollListCtrl* mPlacesInList;
		LLScrollListCtrl* mPlacesOutList;
		LLScrollListItem* pItem;
		S32 id;

		/// @brief: to see if this was the first time setVisible() was called (at program startup)
		BOOL firstRun;
		LLSD tpList;
};

// globals
extern LLFloaterTeleportHistory* gFloaterTeleportHistory;

#endif
