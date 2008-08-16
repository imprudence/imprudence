/** 
 * @file llalertdialog.cpp
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

#include "linden_common.h"

#include "llboost.h"

#include "llalertdialog.h"
#include "llfontgl.h"
#include "llresmgr.h"
#include "lltextbox.h"
#include "llbutton.h"
#include "llcheckboxctrl.h"
#include "llkeyboard.h"
#include "llfocusmgr.h"
#include "llui.h"
#include "llxmlnode.h"
#include "lllineeditor.h"
#include "lluictrlfactory.h"


const S32 MAX_ALLOWED_MSG_WIDTH = 400;
const F32 DEFAULT_BUTTON_DELAY = 0.5f;
const S32 MSG_PAD = 8;

/*static*/ LLAlertDialog::template_map_t LLAlertDialog::sAlertTemplates;
/*static*/ LLAlertDialog::template_map_t LLAlertDialog::sIgnorableTemplates;
/*static*/ LLControlGroup* LLAlertDialog::sSettings = NULL;
/*static*/ std::map<LLString,LLAlertDialog*> LLAlertDialog::sUniqueActiveMap;
/*static*/ LLAlertDialog::display_callback_t LLAlertDialog::sDisplayCallback;

//static
LLAlertDialog* LLAlertDialog::createXml( const LLString& xml_desc,
										 alert_callback_t callback, void *user_data)
{
	LLString::format_map_t args;
	return createXml(xml_desc, args, callback, user_data);
}

//static
LLAlertDialog* LLAlertDialog::createXml( const LLString& xml_desc, const LLString::format_map_t& args,
										 alert_callback_t callback, void *user_data)
{
	template_map_t::iterator iter = sAlertTemplates.find(xml_desc);
	if (iter != sAlertTemplates.end())
	{
		LLAlertDialogTemplate* xml_template = iter->second;
		// deletes itself
		llwarns << "Alert: [" << xml_desc << "] " << llendl;
		LLAlertDialog* dialog = new LLAlertDialog( xml_template, args, callback, user_data);
		return dialog;
	}
	else
	{
		LLString::format_map_t args;
		args["[ALERT_NAME]"] = xml_desc;
		llwarns << "Missing Alert: [" << xml_desc << "]" << llendl;
		LLAlertDialog* dialogp = LLAlertDialog::showXml("MissingAlert", args);
		if (dialogp == NULL)
		{
			llerrs << "Bad or missing alerts.xml!" << llendl;
		}
		return NULL;
	}
}

//static
LLAlertDialog* LLAlertDialog::showXml( const LLString& xml_desc,
							 alert_callback_t callback, void *user_data)
{
	LLString::format_map_t args;
	return showXml(xml_desc, args, callback, user_data);
}

//static
LLAlertDialog* LLAlertDialog::showXml( const LLString& xml_desc, const LLString::format_map_t& args,
							 alert_callback_t callback, void *user_data)
{
	LLAlertDialog* dialog = createXml(xml_desc, args, callback, user_data);
	return dialog && dialog->show() ? dialog : NULL;
}

//static
LLAlertDialog* LLAlertDialog::showCritical( const LLString& desc, alert_callback_t callback, void *user_data)
{
	LLAlertDialogTemplate xml_template;
	LLString::format_map_t args;
	xml_template.mTitle = "Critical Error";
	xml_template.mMessage = desc;
	xml_template.mModal = TRUE;
	xml_template.mOptions.push_back("Quit");
	LLAlertDialog* dialog = new LLAlertDialog( &xml_template, args, callback, user_data);
	return dialog && dialog->show() ? dialog : NULL;
}

//-----------------------------------------------------------------------------
// Private methods

static const S32 VPAD = 16;
static const S32 HPAD = 25;
static const S32 BTN_HPAD = 8;
static const LLFONT_ID font_name = LLFONT_SANSSERIF;

LLAlertDialog::LLAlertDialog( const LLAlertDialogTemplate* xml_template,
							  const LLString::format_map_t& args,
							  alert_callback_t callback, void *user_data)
	:	LLModalDialog( xml_template->mTitle, 100, 100, xml_template->mModal ),  // dummy size.  Will reshape below.
		mCallback( callback ),
		mUserData( user_data ),
		mNumOptions( 0 ),
		mDefaultOption( 0 ),
		mOptionChosen( -1 ),
		mCheck(NULL),
		mUnique(xml_template->mUnique),
		mIgnorable(xml_template->mIgnorable),
		mLabel(xml_template->mLabel),
		mIgnoreLabel(xml_template->mIgnoreLabel),
		mButtonData(NULL),
		mLineEditor(NULL),
		mTextCallback(NULL)
{
	createDialog(&(xml_template->mOptions), xml_template->mDefaultOption,
				 xml_template->mMessage, args,
				 xml_template->mEditLineText);
	setTitle(xml_template->mTitle);
	if (xml_template->mIgnorable)
	{
		//XUI:translate!
		LLString msg;
		if (xml_template->mIgnorable == IGNORE_USE_DEFAULT)
		{
			msg = "Skip this dialog next time";
		}
		else // xml_template->mIgnorable == IGNORE_USE_SAVED
		{
			msg = "Always choose this option";
		}
		setCheckBox(msg, xml_template->mIgnoreLabel);
	}
}

// All logic for deciding not to show an alert is done here,
// so that the alert is valid until show() is called.
bool LLAlertDialog::show()
{
	// If mModal, check to see if we are not displaying alerts,
	//   and do any application logic before showing modal alerts
	if (sDisplayCallback)
	{
		bool show = sDisplayCallback(mModal);
		if (show == false)
		{
			mOptionChosen = mDefaultOption;
			llinfos << "Alert: " << mLabel << llendl;
			delete this;
			return false;
		}
	}

	// Check to see if the user wants to ignore this alert
	if (mIgnorable > 0)
	{
		BOOL warn = sSettings->getWarning(mIgnoreLabel);
		if (!warn)
		{
			switch(mIgnorable)
			{
			case IGNORE_USE_DEFAULT:
				mOptionChosen = mDefaultOption;
				break;
			case IGNORE_USE_SAVED:
				mOptionChosen = sSettings->getS32("Default" + mIgnoreLabel);
				break;
			case IGNORE_SHOW_AGAIN:
				break;
			}
			delete this;
			return false;
		}
	}

	// Check to see if we are already displaying the alert
	if (mUnique)
	{
		std::map<LLString,LLAlertDialog*>::iterator iter = sUniqueActiveMap.find(mLabel);
		if (iter != sUniqueActiveMap.end())
		{
			gFloaterView->bringToFront(iter->second);
			mUnique = FALSE; // don't remove entry from map on destruction
			delete this;
			return false;
		}
		sUniqueActiveMap[mLabel] = this;
	}
	startModal();
	gFloaterView->adjustToFitScreen(this, FALSE);
	open();	/* Flawfinder: ignore */
 	setFocus(TRUE);
	if (mLineEditor)
	{
		mLineEditor->setFocus(TRUE);
		mLineEditor->selectAll();
	}
	if(mDefaultOption >= 0)
	{
		// delay before enabling default button
		mDefaultBtnTimer.start();
		mDefaultBtnTimer.setTimerExpirySec(DEFAULT_BUTTON_DELAY);
	}
	return true;
}

//static
void LLAlertDialog::format(LLString& msg, const LLString::format_map_t& args)
{
	// XUI:translate!
	LLString::format_map_t targs = args;
	targs["[SECOND_LIFE]"] = "Second Life";
	LLString::format(msg, targs);
}

void LLAlertDialog::createDialog(const std::vector<LLString>* optionsp, S32 default_option,
								 const LLString& msg_in, const LLString::format_map_t& args,
								 const LLString& edit_text)
{
	setBackgroundVisible(TRUE);
	setBackgroundOpaque(TRUE);

	const LLFontGL* font = gResMgr->getRes( font_name );
	const S32 LINE_HEIGHT = llfloor(font->getLineHeight() + 0.99f);
	const S32 EDITOR_HEIGHT = 20;

	// buttons
	std::vector<LLString> default_option_list;

	mNumOptions = optionsp->size();
	
	if( 0 == mNumOptions )
	{
		default_option_list.push_back("Close");
		optionsp = &default_option_list;
		default_option = 0;
		mNumOptions = 1;
	}

	const std::vector<LLString>& options = *optionsp;
	
	mButtonData = new ButtonData[mNumOptions];

	// Calc total width of buttons
	S32 button_width = 0;
	S32 sp = font->getWidth("OO");
	for( S32 i = 0; i < mNumOptions; i++ )
	{
		S32 w = S32(font->getWidth( options[i] ) + 0.99f) + sp + 2 * LLBUTTON_H_PAD;
		button_width = llmax( w, button_width );
	}
	S32 btn_total_width = button_width;
	if( mNumOptions > 1 )
	{
		btn_total_width = (mNumOptions * button_width) + ((mNumOptions - 1) * BTN_HPAD);
	}

	// Split message into lines, separated by '\n'
	LLString msg = msg_in;
	LLAlertDialog::format(msg, args);
			 
	llwarns << "Alert: " << msg << llendl;

	S32 max_msg_width = 0;
	std::vector<LLString> msg_lines;

	boost_tokenizer tokens(msg, boost::char_separator<char>("\n"));
	for (boost_tokenizer::iterator token_iter = tokens.begin(); token_iter != tokens.end(); ++token_iter)
	{
		LLString line(*token_iter);
		boost_tokenizer line_toks(line, boost::char_separator<char>(" \t"));
		LLString cur_line;
		S32 cur_line_width = 0;
		for (boost_tokenizer::iterator token_iter2 = line_toks.begin(); token_iter2 != line_toks.end(); ++token_iter2)
		{
			LLString tok(*token_iter2);
			LLString word;
			if (cur_line_width > 0)
			{
				word = " ";
			}
			word += tok;
			S32 word_width = S32(font->getWidth( word ) + 0.99f);
			if (cur_line_width > 0 && cur_line_width + word_width > MAX_ALLOWED_MSG_WIDTH)
			{
				max_msg_width = llmax( max_msg_width, cur_line_width + MSG_PAD);
				msg_lines.push_back( cur_line );
				cur_line.clear();
				cur_line_width = 0;
				word = tok; // no ' '
			}
			cur_line += word;
			cur_line_width += word_width;
		}
		if (cur_line_width > 0)
		{
			max_msg_width = llmax( max_msg_width, cur_line_width );
			msg_lines.push_back( cur_line );
		}
	}

	// pad message box so we don't clip last character
	max_msg_width += 2;
	
	S32 dialog_width = llmax( btn_total_width, max_msg_width ) + 2 * HPAD;
	S32 dialog_height = LINE_HEIGHT * msg_lines.size() + 3 * VPAD + BTN_HEIGHT;

	if (hasTitleBar())
	{
		dialog_height += LINE_HEIGHT; // room for title bar
	}

	if (edit_text.size() > 0)
	{
		dialog_width = llmax(dialog_width, S32(font->getWidth( edit_text ) + 0.99f));
		dialog_height += EDITOR_HEIGHT;
	}
	
	reshape( dialog_width, dialog_height, FALSE );

	// Message
	S32 msg_x = (mRect.getWidth() - max_msg_width) / 2;
	S32 msg_y = mRect.getHeight() - VPAD - LINE_HEIGHT;
	if (hasTitleBar())
	{
		msg_y -= LINE_HEIGHT; // room for title
	}
	
	for( std::vector<LLString>::iterator iter = msg_lines.begin(); iter != msg_lines.end(); ++iter )
	{
		LLRect msg_rect;
		msg_rect.setOriginAndSize( msg_x, msg_y, max_msg_width, LINE_HEIGHT );
		LLTextBox* label_box = new LLTextBox( "Alert message", msg_rect, iter->c_str(), font );
		label_box->setColor( LLUI::sColorsGroup->getColor( "LabelTextColor" ) );
		addChild(label_box);
		msg_y -= LINE_HEIGHT;
	}

	// Buttons	
	S32 button_left = (mRect.getWidth() - btn_total_width) / 2;

	for( S32 i = 0; i < mNumOptions; i++ )
	{
		LLRect button_rect;
		button_rect.setOriginAndSize( button_left, VPAD, button_width, BTN_HEIGHT );

		LLButton* btn = new LLButton(
			"btn", button_rect,
			"","", "", 
			&LLAlertDialog::onButtonPressed, (void*)(&mButtonData[i]),
			font,
			options[i], 
			options[i]);

		mButtonData[i].mSelf = this;
		mButtonData[i].mButton = btn;
		mButtonData[i].mOption = i;
		
		addChild(btn);

		if( i == default_option )
		{
			btn->setFocus(TRUE);
		}

		button_left += button_width + BTN_HPAD;
	}

	// (Optional) Edit Box	
	if (edit_text.size() > 0)
	{
		S32 y = VPAD + BTN_HEIGHT + VPAD/2;
		mLineEditor = new LLLineEditor("lineeditor",
									   LLRect( HPAD, y+EDITOR_HEIGHT, dialog_width-HPAD, y),
									   edit_text,
									   LLFontGL::sSansSerif,
									   STD_STRING_STR_LEN);
		addChild(mLineEditor);
	}
	
}

bool LLAlertDialog::setCheckBox( const LLString& check_title, const LLString& check_control )
{
	const LLFontGL* font = gResMgr->getRes( font_name );
	const S32 LINE_HEIGHT = llfloor(font->getLineHeight() + 0.99f);
	
	// Extend dialog for "check next time"
	S32 max_msg_width = mRect.getWidth() - 2 * HPAD;		
	S32 check_width = S32(font->getWidth(check_title) + 0.99f) + 16;
	max_msg_width = llmax(max_msg_width, check_width);
	S32 dialog_width = max_msg_width + 2 * HPAD;

	S32 dialog_height = mRect.getHeight();
	dialog_height += LINE_HEIGHT;
	dialog_height += LINE_HEIGHT / 2;

	reshape( dialog_width, dialog_height, FALSE );

	S32 msg_x = (mRect.getWidth() - max_msg_width) / 2;
	
	LLRect check_rect;
	check_rect.setOriginAndSize(msg_x, VPAD+BTN_HEIGHT+LINE_HEIGHT/2, 
								max_msg_width, LINE_HEIGHT);

	mCheck = new LLCheckboxCtrl("check", check_rect, check_title, font);
	addChild(mCheck);

	// mCheck is sometimes "show again" and sometimes "hide" :-(
	// If it's "Show Again", and we showed it, it must be checked. JC
	if (mIgnorable == IGNORE_SHOW_AGAIN)
	{
		mCheck->setValue(TRUE);
	}

	return true;
}

void LLAlertDialog::setVisible( BOOL visible )
{
	LLModalDialog::setVisible( visible );
	
	if( visible )
	{
		centerOnScreen();
		make_ui_sound("UISndAlert");
	}
}

LLAlertDialog::~LLAlertDialog()
{
	if (mOptionChosen >= 0)
	{
		if (mTextCallback && mLineEditor)
		{
			mTextCallback(mOptionChosen, mLineEditor->getText(), mUserData);
		}
		else if (mCallback)
		{
			mCallback(mOptionChosen, mUserData);
		}

		// Only change warn state if we actually warned.
		if (mCheck
			&& sSettings->getWarning(mIgnoreLabel))
		{
			// mCheck sometimes means "hide and do the default" and
			// other times means "warn me again".  Yuck. JC
			BOOL check = mCheck->getValue();
			switch(mIgnorable)
			{
			case IGNORE_USE_DEFAULT:
				sSettings->setWarning(mIgnoreLabel, !check);
				break;
			case IGNORE_USE_SAVED:
				sSettings->setWarning(mIgnoreLabel, !check);
				sSettings->setS32("Default" + mIgnoreLabel, mOptionChosen);
				break;
			case IGNORE_SHOW_AGAIN:
				sSettings->setWarning(mIgnoreLabel, check);
				break;
			default:
				break;
			}
		}
	}
	delete[] mButtonData;
	if (mUnique)
	{
		sUniqueActiveMap.erase(mLabel);
	}
}

BOOL LLAlertDialog::hasTitleBar() const
{
	return (getTitle() != "" && getTitle() != " ")	// has title
			|| isMinimizeable()
			|| isCloseable();
}

BOOL LLAlertDialog::handleKeyHere(KEY key, MASK mask, BOOL called_from_parent )
{
	if( KEY_RETURN == key && mask == MASK_NONE )
	{
		// Warning: handleKeyHere may result in the default button
		// being committed, which will destroy this object.
		// Everything works, but the call stack will pass through 
		// the very end of functions that belong to deleted objects.
		// Should find a less fragile way to do this.
		LLModalDialog::handleKeyHere( key, mask , called_from_parent );
		return TRUE;
	}
	else if (KEY_RIGHT == key)
	{
		focusNextItem(FALSE);
		return TRUE;
	}
	else if (KEY_LEFT == key)
	{
		focusPrevItem(FALSE);
		return TRUE;
	}
	else if (KEY_TAB == key && mask == MASK_NONE)
	{
		focusNextItem(FALSE);
		return TRUE;
	}
	else if (KEY_TAB == key && mask == MASK_SHIFT)
	{
		focusPrevItem(FALSE);
		return TRUE;
	}
	else
	{
		return LLModalDialog::handleKeyHere( key, mask , called_from_parent );
	}
}

// virtual
void LLAlertDialog::draw()
{
	// if the default button timer has just expired, activate the default button
	if(mDefaultBtnTimer.hasExpired() && mDefaultBtnTimer.getStarted())
	{
		mDefaultBtnTimer.stop();  // prevent this block from being run more than once
		setDefaultBtn(mButtonData[mDefaultOption].mButton);
	}
	if (getVisible())
	{
		LLColor4 shadow_color = LLUI::sColorsGroup->getColor("ColorDropShadow");
		S32 shadow_lines = LLUI::sConfigGroup->getS32("DropShadowFloater");

		gl_drop_shadow( 0, mRect.getHeight(), mRect.getWidth(), 0,
			shadow_color, shadow_lines);

		LLModalDialog::draw();
	}
}

void LLAlertDialog::setOptionEnabled( S32 option, BOOL enable )
{
	if( (option >= 0) && (option < mNumOptions) )
	{
		mButtonData[option].mButton->setEnabled( enable );
	}
}

void LLAlertDialog::setEditTextCallback(alert_text_callback_t callback, void *user_data)
{
	if (mLineEditor)
	{
		mTextCallback = callback;
		mUserData = user_data;
	}
	else
	{
		llwarns << "LLAlertDialog::setEditTextCallback called on dialog with no line editor" << llendl;
	}
}

void LLAlertDialog::setEditTextArgs(const LLString::format_map_t& edit_args)
{
	if (mLineEditor)
	{
		LLString msg = mLineEditor->getText();
		LLAlertDialog::format(msg, edit_args);
		mLineEditor->setText(msg);
	}
	else
	{
		llwarns << "LLAlertDialog::setEditTextArgs called on dialog with no line editor" << llendl;
	}
}

void LLAlertDialog::setDrawAsterixes(BOOL enable)
{
	if (mLineEditor)
	{
		if (enable)
		{
			mLineEditor->clear();
		}
		mLineEditor->setDrawAsterixes(enable);
	}
}

// static 
void LLAlertDialog::onButtonPressed( void* userdata )
{
	ButtonData* button_data = (ButtonData*)userdata;
	LLAlertDialog* self = button_data->mSelf;

	self->mOptionChosen = button_data->mOption;
	self->close(); // deletes self
}

//=============================================================================

//static
const LLString& LLAlertDialog::getTemplateMessage(const LLString& xml_desc)
{
	template_map_t::iterator iter = sAlertTemplates.find(xml_desc);
	if (iter != sAlertTemplates.end())
	{
		return iter->second->mMessage;
	}
	else
	{
		return xml_desc;
	}
}

//static
bool LLAlertDialog::parseAlerts(const LLString& xml_filename, LLControlGroup* settings, BOOL settings_only)
{
	LLXMLNodePtr root;
	BOOL success  = LLUICtrlFactory::getLayeredXMLNode(xml_filename, root);

	if (!success || root.isNull() || !root->hasName( "alerts" ))
	{
		llerrs << "Problem reading UI Alerts file: " << xml_filename << llendl;
		return false;
	}

	BOOL add_settings = FALSE;
	if (settings)
	{
		sSettings = settings;
		add_settings = TRUE;
	}
	llassert(sSettings);
	
	for (LLXMLNode* alert = root->getFirstChild();
		 alert != NULL; alert = alert->getNextSibling())
	{
		if (!alert->hasName("alert"))
		{
			continue;
		}
		
		LLAlertDialogTemplate* xml_template = settings_only ? NULL : new LLAlertDialogTemplate;

		// name=
		LLString alert_name;
		if (alert->getAttributeString("name", alert_name))
		{
			if (xml_template)
			{
				xml_template->mLabel = alert_name;
			}
		}
		else
		{
			llwarns << "Unable to parse alert with no name" << llendl;
			delete xml_template;
			continue;
		}
		// title=
		LLString title;
		if (alert->getAttributeString("title", title))
		{
			if (xml_template)
			{
				xml_template->mTitle = title;
			}
		}
		// modal=
		BOOL modal;
		if (alert->getAttributeBOOL("modal", modal))
		{
			if (xml_template)
			{
				xml_template->mModal = modal;
			}
		}
		// unique=
		BOOL unique;
		if (alert->getAttributeBOOL("unique", unique))
		{
			if (xml_template)
			{
				xml_template->mUnique = unique;
			}
		}
				
		S32 default_option = 0;
		BOOL nodefault;
		if (alert->getAttributeBOOL("nodefault", nodefault))
		{
			if (nodefault)
			{
				if (xml_template)
				{
					xml_template->mDefaultOption = -1;
				}
				default_option = -1;
			}
		}

		S32 btn_idx = 0;
		for (LLXMLNode* child = alert->getFirstChild();
			 child != NULL; child = child->getNextSibling())
		{
			// <message>
			if (child->hasName("message"))
			{
				if (xml_template)
				{
					xml_template->mMessage = child->getTextContents();
				}
			}

			// <option>
			if (child->hasName("option"))
			{
				LLString label = child->getTextContents();
				BOOL is_default = FALSE;
				child->getAttributeBOOL("default", is_default);
				LLString ignore_text;
				if (!child->getAttributeString("ignore", ignore_text))
				{
					ignore_text = label;
				}
				if (xml_template)
				{
					xml_template->addOption(label, ignore_text, is_default);
				}
				if (is_default)
				{
					default_option = btn_idx;
				}
				btn_idx++;
			}
		
			// <editline>
			if (child->hasName("editline"))
			{
				if (xml_template)
				{
					xml_template->mEditLineText = child->getTextContents();
					if (xml_template->mEditLineText.empty())
					{
						xml_template->mEditLineText = " ";
					}
				}
			}
			
			// <ignore>
			if (child->hasName("ignore"))
			{
				LLString ignore_text = child->getTextContents();
				// label=
				LLString name;
				child->getAttributeString("name", name);
				
				//always set to alert_name for the sake of i18n
				//if (name.empty())
				name = alert_name;
				
				if (xml_template)
				{
					xml_template->mIgnorable = LLAlertDialog::IGNORE_USE_DEFAULT;
					xml_template->mIgnoreListText = ignore_text;
					xml_template->mIgnoreLabel = name;
				}
				if (!ignore_text.empty())
				{
					if (add_settings)
					{
						settings->addWarning(name);
					}
					if (xml_template)
					{
						sIgnorableTemplates[name] = xml_template; // will override any previous entry
					}
				}
				// save_option=
				BOOL save_option = FALSE;
				child->getAttributeBOOL("save_option", save_option);
				if (save_option)
				{
					if (xml_template)
					{
						xml_template->mIgnorable = LLAlertDialog::IGNORE_USE_SAVED;
					}
					if (add_settings)
					{
						settings->declareS32("Default" + name, default_option, "Default option number for this alert dialog");
					}
				}
			}
		}
		if (xml_template)
		{
			xml_template->mDefaultOption = default_option;
			sAlertTemplates[xml_template->mLabel] = xml_template;
		}
	}
	return true;
}
