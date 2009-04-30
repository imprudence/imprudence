/** 
 * @file llpreviewlandmark.h
 * @brief LLPreviewLandmark class implementation
 *
 * $LicenseInfo:firstyear=2002&license=viewergpl$
 * 
 * Copyright (c) 2002-2009, Linden Research, Inc.
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

#ifndef LL_LLPREVIEWLANDMARK_H
#define LL_LLPREVIEWLANDMARK_H

#include <deque>

#include "lllandmark.h"

#include "llfloater.h"
#include "llmap.h"
#include "llstring.h"
#include "lluuid.h"
#include "v3dmath.h"
#include "v4coloru.h"

#include "llhudtext.h"
#include "llpreview.h"

class LLIconCtrl;
class LLInventoryItem;
class LLLandmarkList;
class LLLineEditor;
class LLMessageSystem;
class LLPreviewLandmark;
class LLPanelPlace;

const S32 PREVIEW_LANDMARK_NUM_COLORS = 6;

typedef std::deque< LLPreviewLandmark* > LLPreviewLandmarkList;

class LLPreviewLandmark : public LLPreview
{
public:
	LLPreviewLandmark(const std::string& name, const LLRect& rect, const std::string& title,
					  const LLUUID& item_uuid,
					  BOOL show_keep_discard = FALSE,
					  LLViewerInventoryItem* inv_item = NULL);
	virtual ~LLPreviewLandmark();

	/*virtual*/ void			draw();

	const std::string&				getName() const;
	const LLColor4&				getMarkerColor() const;

	LLVector3d getPositionGlobal() const;

	//static S32 getNumInstances() { return LLPreviewLandmark::sOrderedInstances.getLength(); }
	//static const LLPreviewLandmark* getFirst() { return LLPreviewLandmark::sOrderedInstances.getFirstData(); }
	//static const LLPreviewLandmark* getNext() { return LLPreviewLandmark::sOrderedInstances.getNextData(); }

	static void* createPlaceDetail(void* userdata);

	/*virtual*/ void loadAsset();
	/*virtual*/ EAssetStatus getAssetStatus();

protected:
	void getDegreesAndDist(F32* degrees, F64* horiz_dist, F64* vert_dist) const;

	virtual const char *getTitleName() const { return "Landmark"; }

private:
//	void renderBeacon();
//	LLPointer<LLHUDText> mBeaconText;

	LLIconCtrl*	mIconLandmark;
	LLPanelPlace* mPlacePanel;
	LLLandmark* mLandmark;
	LLColor4 mMarkerColor;

	static LLPreviewLandmarkList sOrderedInstances;
};

#endif
