/** 
 * @file llpanelmediahud.h
 * @brief Media hud panel
 *
 * $LicenseInfo:firstyear=2003&license=viewergpl$
 * 
 * Copyright (c) 2003-2009, Linden Research, Inc.
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

#ifndef LL_PANELMEDIAHUD_H
#define LL_PANELMEDIAHUD_H

#include "llpanel.h"
#include "llviewermedia.h"

class LLCoordWindow;
class LLViewerMediaImpl;

class LLPanelMediaHUD : public LLPanel
{
public:
	LLPanelMediaHUD(viewer_media_t media_impl);
	virtual ~LLPanelMediaHUD();
	/*virtual*/ BOOL postBuild();
	virtual void draw();
	virtual void setAlpha(F32 alpha);
	virtual BOOL handleScrollWheel(S32 x, S32 y, S32 clicks);
	void updateShape();
	bool isMouseOver();
	void setMediaFocus(bool b) { mMediaFocus = b; }
	void nextZoomLevel();
	void resetZoomLevel() { mCurrentZoom = ZOOM_NONE; }

	LLHandle<LLPanelMediaHUD>	getHandle() const { return mPanelHandle; }
	void setMediaImpl(viewer_media_t media_impl) { mMediaImpl = media_impl; }


	enum EZoomLevel
	{
		ZOOM_NONE = 0,
		ZOOM_MEDIUM = 1,
		ZOOM_END
	};
	enum EScrollDir
	{
		SCROLL_UP = 0,
		SCROLL_DOWN,
		SCROLL_LEFT,
		SCROLL_RIGHT,
		SCROLL_NONE
	};

private:
	static void onClickClose(void* user_data);
	static void onClickBack(void* user_data);
	static void onClickForward(void* user_data);
	static void onClickHome(void* user_data);
	static void onClickOpen(void* user_data);
	static void onClickReload(void* user_data);
	static void onClickPlay(void* user_data);
	static void onClickPause(void* user_data);
	static void onClickStop(void* user_data);
	static void onClickZoom(void* user_data);
	static void onScrollUp(void* user_data);
	static void onScrollUpHeld(void* user_data);
	static void onScrollLeft(void* user_data);
	static void onScrollLeftHeld(void* user_data);
	static void onScrollRight(void* user_data);
	static void onScrollRightHeld(void* user_data);
	static void onScrollDown(void* user_data);
	static void onScrollDownHeld(void* user_data);
	static void onScrollStop(void* user_data);

	bool mMediaFocus;
	LLMatrix4 mLastCameraMat;
	EZoomLevel mCurrentZoom;
	EScrollDir mScrollState;
	LLCoordWindow mLastCursorPos;
	LLFrameTimer mMouseMoveTimer;
	LLFrameTimer mFadeTimer;
	F32 mMouseInactiveTime;
	F32 mControlFadeTime;
	viewer_media_t mMediaImpl;
	LLRootHandle<LLPanelMediaHUD> mPanelHandle;
};

#endif // LL_PANELMEDIAHUD_H
