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

#ifndef LL_ALERTDIALOG_H
#define LL_ALERTDIALOG_H

#include "llpanel.h"
#include "llmodaldialog.h"
#include "llmemory.h"
#include "llui.h"

class LLButton;
class LLCheckBoxCtrl;
class LLAlertDialogTemplate;
class LLLineEditor;

// https://wiki.lindenlab.com/mediawiki/index.php?title=LLAlertDialog&oldid=81388
class LLAlertDialog : public LLModalDialog
{
public:
	typedef void (*alert_callback_t)(S32 option, void* user_data);
	typedef void (*alert_text_callback_t)(S32 option, const std::string& text, void* user_data);
	typedef bool (*display_callback_t)(S32 modal);
	typedef std::vector<std::string> options_list_t;
	
	enum { IGNORE_USE_DEFAULT=1, IGNORE_USE_SAVED=2, IGNORE_SHOW_AGAIN=3 };

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
	LLAlertDialog( const LLAlertDialogTemplate* xml_template, const LLStringUtil::format_map_t& args,
				   alert_callback_t callback = NULL, void *user_data = NULL);

	virtual BOOL	handleKeyHere(KEY key, MASK mask );

	virtual void	draw();
	virtual void	setVisible( BOOL visible );
	virtual void	onClose(bool app_quitting);

	bool 			setCheckBox( const std::string&, const std::string& );	
	void			setOptionEnabled( S32 option, BOOL enable );
	void			setCaution(BOOL val = TRUE) { mCaution = val; }
	// If mUnique==TRUE only one copy of this message should exist
	void			setUnique(BOOL val = TRUE) { mUnique = val; }
	void			setEditTextCallback(alert_text_callback_t callback, void *user_data);
	void			setEditTextArgs(const LLStringUtil::format_map_t& edit_args);
	void			setDrawAsterixes(BOOL enable);
	
	bool			show();	// May instantly destroy the message if it is unique (returns false)
	
	//statics
	static void		onButtonPressed(void* userdata);

	static LLAlertDialog* createXml( const std::string& xml_desc,
									 alert_callback_t callback = NULL, void *user_data = NULL);
	static LLAlertDialog* createXml( const std::string& xml_desc, const LLStringUtil::format_map_t& args,
									 alert_callback_t callback = NULL, void *user_data = NULL);
	
	static LLAlertDialog* 	showXml( const std::string& xml_desc,
							 alert_callback_t callback = NULL, void *user_data = NULL);
	static LLAlertDialog* 	showXml( const std::string& xml_desc, const LLStringUtil::format_map_t& args,
							 alert_callback_t callback = NULL, void *user_data = NULL);

	static LLAlertDialog* 	showCritical( const std::string& msg, alert_callback_t callback = NULL, void *user_data = NULL);
	
	static bool parseAlerts(const std::string& xml_filename, LLControlGroup* settings = NULL, BOOL settings_only = FALSE);
	static const std::string& getTemplateMessage(const std::string& xml_desc);

	static void setDisplayCallback(display_callback_t callback) { sDisplayCallback = callback; }

private:
	void format(std::string& msg, const LLStringUtil::format_map_t& args);

public:
	static LLControlGroup* sSettings;

	// use LLPointer so they delete themselves when sTemplates is destroyed
	typedef std::map<std::string, LLPointer<LLAlertDialogTemplate> > template_map_t;
	static template_map_t sAlertTemplates; // by mLabel
	static template_map_t sIgnorableTemplates; // by mIgnoreLabel

private:

	static std::map<std::string, LLAlertDialog*> sUniqueActiveMap;
	static display_callback_t sDisplayCallback;

	static std::string sStringSkipNextTime;
	static std::string sStringAlwaysChoose;

	void createDialog(const options_list_t& options_in, S32 default_option,
					  const std::string& msg, const LLStringUtil::format_map_t& args,
					  const std::string& edit_text);
	
	virtual ~LLAlertDialog();
	void handleCallbacks();
	// No you can't kill it.  It can only kill itself.

	// Does it have a readable title label, or minimize or close buttons?
	BOOL hasTitleBar() const;

	struct ButtonData
	{
		LLAlertDialog* mSelf;
		LLButton* mButton;
		S32 mOption;
	} * mButtonData;

	alert_callback_t mCallback;
	void*	mUserData;
	S32		mNumOptions;
	S32		mDefaultOption;
	BOOL	mOptionChosen;
	LLCheckBoxCtrl* mCheck;
	BOOL	mCaution;
	BOOL	mUnique;
	S32		mIgnorable;
	std::string mLabel;
	std::string mIgnoreLabel;
	LLFrameTimer mDefaultBtnTimer;
	// For Dialogs that take a line as text as input:
	LLLineEditor* mLineEditor;
	alert_text_callback_t mTextCallback;
	// For Dialogs linked to a URL
	std::string mURL;		 		// Some alerts will direct the resident to a URL
	S32 mURLOption;

private:
	static URLLoader* sURLLoader;
};

//============================================================================

class LLAlertDialogTemplate : public LLRefCount
{
public:
	LLAlertDialogTemplate() : mTitle(), mURLOption(0), mModal(FALSE), mCaution(FALSE), mUnique(FALSE), mIgnorable(0), mDefaultOption(0) {}
	
	void addOption(const std::string& label, const std::string& ignore_text, BOOL is_default = FALSE)
	{
		if (is_default)
		{
			mDefaultOption = mOptions.size();
		}
		mOptions.push_back(label);
		mOptionDefaultText.push_back(ignore_text);
	}

	S32 getIgnore()
	{
		if (mIgnorable)
		{
			return LLAlertDialog::sSettings->getWarning(mIgnoreLabel) ? FALSE : mIgnorable;
		}
		else
		{
			return FALSE;
		}
	}

	void setIgnore(bool state)
	{
		if (mIgnorable)
		{
			LLAlertDialog::sSettings->setWarning(mIgnoreLabel, !state);
		}
	}

	
public:
	std::string mLabel;			// Handle for access from code, etc
	std::string mTitle;			// (optional) text to display in title bar
	std::string mMessage;			// Message to display
	std::string mIgnoreListText; 	// Text to display in enable/disable dialog (if mIgnorable == TRUE)
	std::string mIgnoreLabel; 		// Handle for ignore variable (may be shared by multiple templates)
	std::string mURL;		 		// Some alerts will direct the resident to a URL
	S32 mURLOption;
	BOOL mModal;
	BOOL mCaution;
	BOOL mUnique;
	S32 mIgnorable; // 0 = Never Ignore, 1 = Do default option, 2 = Do saved option
	LLAlertDialog::options_list_t mOptions;
	LLAlertDialog::options_list_t mOptionDefaultText;
	S32 mDefaultOption;
	std::string mEditLineText;
};

#endif  // LL_ALERTDIALOG_H
