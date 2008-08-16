/** 
 * @file llalertdialog.h
 * @brief LLAlertDialog base class
 *
 * Copyright (c) 2001-2007, Linden Research, Inc.
 * 
 * Second Life Viewer Source Code
 * The source code in this file ("Source Code") is provided by Linden Lab
 * to you under the terms of the GNU General Public License, version 2.0
 * ("GPL"), unless you have obtained a separate licensing agreement
 * ("Other License"), formally executed by you and Linden Lab.  Terms of
 * the GPL can be found in doc/GPL-license.txt in this distribution, or
 * online at http://secondlife.com/developers/opensource/gplv2
 * 
 * There are special exceptions to the terms and conditions of the GPL as
 * it is applied to this Source Code. View the full text of the exception
 * in the file doc/FLOSS-exception.txt in this software distribution, or
 * online at http://secondlife.com/developers/opensource/flossexception
 * 
 * By copying, modifying or distributing this software, you acknowledge
 * that you have read and understood your obligations described above,
 * and agree to abide by those obligations.
 * 
 * ALL LINDEN LAB SOURCE CODE IS PROVIDED "AS IS." LINDEN LAB MAKES NO
 * WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
 * COMPLETENESS OR PERFORMANCE.
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

class LLAlertDialog : public LLModalDialog
{
public:
	typedef void (*alert_callback_t)(S32 option, void* user_data);
	typedef void (*alert_text_callback_t)(S32 option, const LLString& text, void* user_data);
	typedef bool (*display_callback_t)(S32 modal);
	enum { IGNORE_USE_DEFAULT=1, IGNORE_USE_SAVED=2, IGNORE_SHOW_AGAIN=3 };

protected:
	struct ButtonData
	{
		LLAlertDialog* mSelf;
		LLButton* mButton;
		S32 mOption;
	};
	
public:
	virtual BOOL	handleKeyHere(KEY key, MASK mask, BOOL called_from_parent );

	virtual void	draw();
	virtual void	setVisible( BOOL visible );

	bool 			setCheckBox( const LLString&, const LLString& );	
	void			setOptionEnabled( S32 option, BOOL enable );
	// If mUnique==TRUE only one copy of this message should exist
	void			setUnique(BOOL val = TRUE) { mUnique = val; }
	void			setEditTextCallback(alert_text_callback_t callback, void *user_data);
	void			setEditTextArgs(const LLString::format_map_t& edit_args);
	void			setDrawAsterixes(BOOL enable);
	
	bool			show();	// May instantly destroy the message if it is unique (returns false)
	
	//statics
	static void		onButtonPressed(void* userdata);

	static LLAlertDialog* createXml( const LLString& xml_desc,
									 alert_callback_t callback = NULL, void *user_data = NULL);
	static LLAlertDialog* createXml( const LLString& xml_desc, const LLString::format_map_t& args,
									 alert_callback_t callback = NULL, void *user_data = NULL);
	
	static LLAlertDialog* 	showXml( const LLString& xml_desc,
							 alert_callback_t callback = NULL, void *user_data = NULL);
	static LLAlertDialog* 	showXml( const LLString& xml_desc, const LLString::format_map_t& args,
							 alert_callback_t callback = NULL, void *user_data = NULL);

	static LLAlertDialog* 	showCritical( const LLString& msg, alert_callback_t callback = NULL, void *user_data = NULL);
	
	static bool parseAlerts(const LLString& xml_filename, LLControlGroup* settings = NULL, BOOL settings_only = FALSE);
	static const LLString& getTemplateMessage(const LLString& xml_desc);

	static void setDisplayCallback(display_callback_t callback) { sDisplayCallback = callback; }
	
	// Must call show() after creating these
	LLAlertDialog( const LLAlertDialogTemplate* xml_template, const LLString::format_map_t& args,
				   alert_callback_t callback = NULL, void *user_data = NULL);

	static void format(LLString& msg, const LLString::format_map_t& args);
	
protected:
	void createDialog(const std::vector<LLString>* options, S32 default_option,
					  const LLString& msg, const LLString::format_map_t& args,
					  const LLString& edit_text);
	
	virtual ~LLAlertDialog();	// No you can't kill it.  It can only kill itself.

	// Does it have a readable title label, or minimize or close buttons?
	BOOL hasTitleBar() const;

protected:
	alert_callback_t mCallback;
	void*	mUserData;
	S32		mNumOptions;
	S32		mDefaultOption;
	BOOL	mOptionChosen;
	LLCheckBoxCtrl* mCheck;
	BOOL	mUnique;
	S32		mIgnorable;
	LLString mLabel;
	LLString mIgnoreLabel;
	ButtonData* mButtonData;
	LLFrameTimer mDefaultBtnTimer;
	// For Dialogs that take a line as text as input:
	LLLineEditor* mLineEditor;
	alert_text_callback_t mTextCallback;
	
public:
	// use LLPointer so they delete themselves when sTemplates is destroyed
	typedef std::map<LLString, LLPointer<LLAlertDialogTemplate> > template_map_t;
	static template_map_t sAlertTemplates; // by mLabel
	static template_map_t sIgnorableTemplates; // by mIgnoreLabel
	static LLControlGroup* sSettings;
	static std::map<LLString, LLAlertDialog*> sUniqueActiveMap;
	static display_callback_t sDisplayCallback;
};

//============================================================================

class LLAlertDialogTemplate : public LLRefCount
{
public:
	LLAlertDialogTemplate() : mTitle(""), mModal(FALSE), mUnique(FALSE), mIgnorable(0), mDefaultOption(0) {}
	
	void addOption(const LLString& label, const LLString& ignore_text, BOOL is_default = FALSE)
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
	
public:
	LLString mLabel;			// Handle for access from code, etc
	LLString mTitle;			// (optional) text to display in title bar
	LLString mMessage;			// Message to display
	LLString mIgnoreListText; 	// Text to display in enable/disable dialog (if mIgnorable == TRUE)
	LLString mIgnoreLabel; 		// Handle for ignore variable (may be shared by multiple templates)
	BOOL mModal;
	BOOL mUnique;
	S32 mIgnorable; // 0 = Never Ignore, 1 = Do default option, 2 = Do saved option
	std::vector<LLString> mOptions;
	std::vector<LLString> mOptionDefaultText;
	S32 mDefaultOption;
	LLString mEditLineText;
};

#endif  // LL_ALERTDIALOG_H
