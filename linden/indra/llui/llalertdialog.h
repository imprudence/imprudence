/** 
 * @file llalertdialog.h
 * @brief LLAlertDialog base class
 *
 * $LicenseInfo:firstyear=2001&license=viewergpl$
 * 
 * Copyright (c) 2001-2009, Linden Research, Inc.
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

#ifndef LL_ALERTDIALOG_H
#define LL_ALERTDIALOG_H

#include "llpanel.h"
#include "llmodaldialog.h"
#include "llmemory.h"
#include "llui.h"
#include "llnotifications.h"

class LLButton;
class LLCheckBoxCtrl;
class LLAlertDialogTemplate;
class LLLineEditor;

// https://wiki.lindenlab.com/mediawiki/index.php?title=LLAlertDialog&oldid=81388
class LLAlertDialog 
	: public LLModalDialog,
	  public LLInitClass<LLAlertDialog>,
	  public LLInstanceTracker<LLAlertDialog, LLUUID>

{
public:
	typedef bool (*display_callback_t)(S32 modal);

	class URLLoader
	{
	public:
		virtual void load(const std::string& url) = 0;
		virtual ~URLLoader() {}
	};
	
	static void setURLLoader(URLLoader* loader)
	{
		sURLLoader = loader;
	}
	
public:
	// User's responsibility to call show() after creating these.
	LLAlertDialog( LLNotificationPtr notep, bool is_modal );

	virtual BOOL	handleKeyHere(KEY key, MASK mask );

	virtual void	draw();
	virtual void	setVisible( BOOL visible );
	virtual void	onClose(bool app_quitting);

	bool 			setCheckBox( const std::string&, const std::string& );	
	void			setCaution(BOOL val = TRUE) { mCaution = val; }
	// If mUnique==TRUE only one copy of this message should exist
	void			setUnique(BOOL val = TRUE) { mUnique = val; }
	void			setEditTextArgs(const LLSD& edit_args);
	
	bool			show();	// May instantly destroy the message if it is unique (returns false)
	
	//statics
	static void initClass();
	static bool onNewNotification(const LLSD& notify, bool is_modal);
	static void	onButtonPressed(void* userdata);
	static void onClickIgnore(LLUICtrl* ctrl, void* user_data);

private:
	LLNotificationPtr mNote;

	static std::map<std::string, LLAlertDialog*> sUniqueActiveMap;

	virtual ~LLAlertDialog();
	// No you can't kill it.  It can only kill itself.

	// Does it have a readable title label, or minimize or close buttons?
	BOOL hasTitleBar() const;

private:
	static URLLoader* sURLLoader;
	static LLControlGroup* sSettings;

	struct ButtonData
	{
		LLAlertDialog* mSelf;
		LLButton* mButton;
		std::string mURL;
	};
	std::vector<ButtonData> mButtonData;

	S32				mDefaultOption;
	LLCheckBoxCtrl* mCheck;
	BOOL			mCaution;
	BOOL			mUnique;
	std::string		mLabel;
	LLFrameTimer	mDefaultBtnTimer;
	// For Dialogs that take a line as text as input:
	LLLineEditor* mLineEditor;

};


#endif  // LL_ALERTDIALOG_H
