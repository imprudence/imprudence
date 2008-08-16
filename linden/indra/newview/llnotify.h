/** 
 * @file llnotify.h
 * @brief Non-blocking notification that doesn't take keyboard focus.
 *
 * $LicenseInfo:firstyear=2003&license=viewergpl$
 * 
 * Copyright (c) 2003-2008, Linden Research, Inc.
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

#ifndef LL_LLNOTIFY_H
#define LL_LLNOTIFY_H

#include "llfontgl.h"
#include "llpanel.h"
#include "lltimer.h"
#include <vector>

class LLButton;
class LLNotifyBoxTemplate;

// NotifyBox - for notifications that require a response from the user.  
class LLNotifyBox : public LLPanel, public LLEventTimer
{
public:
	typedef void (*notify_callback_t)(S32 option, void* data);
	typedef std::vector<LLString> option_list_t;

	static LLNotifyBox* showXml( const LLString& xml_desc,
						 notify_callback_t callback = NULL, void *user_data = NULL);
	static LLNotifyBox* showXml( const LLString& xml_desc, const LLString::format_map_t& args, BOOL is_caution,
						 notify_callback_t callback = NULL, void *user_data = NULL);
	static LLNotifyBox* showXml( const LLString& xml_desc, const LLString::format_map_t& args,
						 notify_callback_t callback = NULL, void *user_data = NULL);
	// For script notifications:
	static LLNotifyBox* showXml( const LLString& xml_desc, const LLString::format_map_t& args,
						 notify_callback_t callback, void *user_data,
						 const option_list_t& options,
						 BOOL layout_script_dialog = FALSE);

	static bool parseNotify(const LLString& xml_filename);
	static const LLString getTemplateMessage(const LLString& xml_desc, const LLString::format_map_t& args);
	static const LLString getTemplateMessage(const LLString& xml_desc);
 	static BOOL getTemplateIsCaution(const LLString& xml_desc);
	
	BOOL isTip() const { return mIsTip; }
	BOOL isCaution() const { return mIsCaution; }
	/*virtual*/ void setVisible(BOOL visible);
	void stopAnimation() { mAnimating = FALSE; }

	notify_callback_t getNotifyCallback() { return mBehavior->mCallback; }
	void* getUserData() { return mBehavior->mData; }
	void close();

	static void cleanup();
	static void format(LLString& msg, const LLString::format_map_t& args);

protected:
	LLNotifyBox(LLPointer<LLNotifyBoxTemplate> notify_template, const LLString::format_map_t& args,
							 notify_callback_t callback, void* user_data,
 							 BOOL is_caution = FALSE,
							 const option_list_t& extra_options = option_list_t(),
							 BOOL layout_script_dialog = FALSE);
	/*virtual*/ ~LLNotifyBox();

	
	/*virtual*/ BOOL handleMouseUp(S32 x, S32 y, MASK mask);
	/*virtual*/ BOOL handleRightMouseDown(S32 x, S32 y, MASK mask);

	// Animate as sliding onto the screen.
	/*virtual*/ void draw();
	/*virtual*/ BOOL tick();

	void moveToBack(bool getfocus = false);

	// Returns the rect, relative to gNotifyView, where this
	// notify box should be placed.
	static LLRect getNotifyRect(S32 num_options, BOOL layout_script_dialog, BOOL is_caution);
	static LLRect getNotifyTipRect(const LLString &message);

	// internal handler for button being clicked
	static void onClickButton(void* data);

	// for "next" button
	static void onClickNext(void* data);

	static LLPointer<LLNotifyBoxTemplate> getTemplate(const LLString& xml_desc);
	static LLNotifyBox* findExistingNotify(LLPointer<LLNotifyBoxTemplate> notify_template, const LLString::format_map_t& args);

private:
	void drawBackground() const;

	static LLPointer<LLNotifyBoxTemplate> sDefaultTemplate;

protected:
	LLString mMessage;

	BOOL mIsTip;
	BOOL mIsCaution; // is this a caution notification?
	BOOL mAnimating; // Are we sliding onscreen?
	BOOL mUnique;

	// Time since this notification was displayed.
	// This is an LLTimer not a frame timer because I am concerned
	// that I could be out-of-sync by one frame in the animation.
	LLTimer mAnimateTimer;

	LLButton* mNextBtn;

	// keep response behavior isolated here
	struct LLNotifyBehavior
	{
		LLNotifyBehavior(notify_callback_t callback, void* data);

		notify_callback_t mCallback;
		void* mData;

	};
	LLNotifyBehavior* mBehavior;

	S32 mNumOptions;
	S32 mDefaultOption;

	// Used for callbacks
	struct InstanceAndS32
	{
		LLNotifyBox* mSelf;
		S32			mButton;
	};
	std::vector<InstanceAndS32*> mBtnCallbackData;

	typedef std::map<LLString, LLPointer<LLNotifyBoxTemplate> > template_map_t;
	static template_map_t sNotifyTemplates; // by mLabel
	
	static S32 sNotifyBoxCount;
	static const LLFontGL* sFont;
	static const LLFontGL* sFontSmall;

	typedef std::map<LLString, LLNotifyBox*> unique_map_t;
	static unique_map_t sOpenUniqueNotifyBoxes;
};

class LLNotifyBoxView : public LLUICtrl
{
public:
	LLNotifyBoxView(const LLString& name, const LLRect& rect, BOOL mouse_opaque, U32 follows=FOLLOWS_NONE);
	void showOnly(LLView * ctrl);
	LLNotifyBox * getFirstNontipBox() const;

	virtual EWidgetType getWidgetType() const { return WIDGET_TYPE_VIEW; };
	virtual LLString getWidgetTag() const { return LLString(); }
};

// This view contains the stack of notification windows.
extern LLNotifyBoxView* gNotifyBoxView;

class LLNotifyBoxTemplate : public LLRefCount
{
public:
	LLNotifyBoxTemplate(BOOL unique, F32 duration) :
		mIsTip(FALSE),
		mIsCaution(FALSE),
		mUnique(unique),
		mDuration(duration),
		mDefaultOption(0)
	{}

	void setMessage(const LLString& message)
	{
		mMessage = message;
	}
	
	void addOption(const LLString& label, BOOL is_default = FALSE)
	{
		if (is_default)
		{
			mDefaultOption = mOptions.size();
		}
		mOptions.push_back(label);
	}

public:
	LLString mLabel;			// Handle for access from code, etc
	LLString mMessage;			// Message to display
	BOOL mIsTip;
	BOOL mIsCaution;
	BOOL mUnique;
	F32	 mDuration;
	LLNotifyBox::option_list_t mOptions;
	S32 mDefaultOption;
};

#endif
