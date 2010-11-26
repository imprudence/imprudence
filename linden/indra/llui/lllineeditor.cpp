/** 
 * @file lllineeditor.cpp
 * @brief LLLineEditor base class
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

// Text editor widget to let users enter a single line.

#include "linden_common.h"
 
#include "lllineeditor.h"

#include "lltexteditor.h"
#include "llmath.h"
#include "llfontgl.h"
#include "llgl.h"
#include "lltimer.h"

#include "llcalc.h"
//#include "llclipboard.h"
#include "llcontrol.h"
#include "llbutton.h"
#include "llfocusmgr.h"
#include "llkeyboard.h"
#include "llrect.h"
#include "llresmgr.h"
#include "llstring.h"
#include "llwindow.h"
#include "llui.h"
#include "lluictrlfactory.h"
#include "llclipboard.h"

#include "../newview/lgghunspell_wrapper.h"
#include "../newview/lltranslate.h"
#include "../newview/llviewercontrol.h"
#include "../newview/lggautocorrect.h"

//
// Imported globals
//

//
// Constants
//

const S32	UI_LINEEDITOR_CURSOR_THICKNESS = 2;
const S32	UI_LINEEDITOR_H_PAD = 2;
const S32	UI_LINEEDITOR_V_PAD = 1;
const F32	CURSOR_FLASH_DELAY = 1.0f;  // in seconds
const S32	SCROLL_INCREMENT_ADD = 0;	// make space for typing
const S32   SCROLL_INCREMENT_DEL = 4;	// make space for baskspacing
const F32   AUTO_SCROLL_TIME = 0.05f;

const F32	PREEDIT_MARKER_BRIGHTNESS = 0.4f;
const S32	PREEDIT_MARKER_GAP = 1;
const S32	PREEDIT_MARKER_POSITION = 2;
const S32	PREEDIT_MARKER_THICKNESS = 1;
const F32	PREEDIT_STANDOUT_BRIGHTNESS = 0.6f;
const S32	PREEDIT_STANDOUT_GAP = 1;
const S32	PREEDIT_STANDOUT_POSITION = 2;
const S32	PREEDIT_STANDOUT_THICKNESS = 2;

static LLRegisterWidget<LLLineEditor> r1("line_editor");

/* static */ LLPointer<LLUIImage> LLLineEditor::sImage;

//
// Member functions
//
class LineChatTranslationReceiver : public LLTranslate::TranslationReceiver
{
public :
	LineChatTranslationReceiver(const std::string &toLang, LLLineEditor* line, const S32 start, const S32 len):
		LLTranslate::TranslationReceiver("", toLang),
		m_line(line),
		m_position(start),
		m_origLength(len)
	{
	}
 
	static boost::intrusive_ptr<LineChatTranslationReceiver> build(const std::string &toLang,LLLineEditor* line, const S32 start, const S32 len)
	{
		return boost::intrusive_ptr<LineChatTranslationReceiver>(new LineChatTranslationReceiver(toLang, line, start, len));
	}

protected:
	void handleResponse(const std::string &translation, const std::string &detectedLanguage)
	{
		m_line->translationReplace(translation, m_position, m_origLength);
	}
	void handleFailure()
	{
		LLTranslate::TranslationReceiver::handleFailure();
	}
private:
	LLLineEditor* m_line;
	S32 m_position;
	S32 m_origLength;
};
 
LLLineEditor::LLLineEditor(const std::string& name, const LLRect& rect,
						   const std::string& default_text, const LLFontGL* font,
						   S32 max_length_bytes,
						   void (*commit_callback)(LLUICtrl* caller, void* user_data ),
						   void (*keystroke_callback)(LLLineEditor* caller, void* user_data ),
						   void (*focus_lost_callback)(LLFocusableElement* caller, void* user_data ),
						   void* userdata,
						   LLLinePrevalidateFunc prevalidate_func,
						   LLViewBorder::EBevel border_bevel,
						   LLViewBorder::EStyle border_style,
						   S32 border_thickness)
	:
		LLUICtrl( name, rect, TRUE, commit_callback, userdata, FOLLOWS_TOP | FOLLOWS_LEFT ),
		mMaxLengthBytes(max_length_bytes),
		mPopupMenuHandle(),
		mCursorPos( 0 ),
		mScrollHPos( 0 ),
		mTextPadLeft(0),
		mTextPadRight(0),
		mCommitOnFocusLost( TRUE ),
		mRevertOnEsc( TRUE ),
		mKeystrokeCallback( keystroke_callback ),
		mIsSelecting( FALSE ),
		mSelectionStart( 0 ),
		mSelectionEnd( 0 ),
		mLastSelectionX(-1),
		mLastSelectionY(-1),
		mLastSelectionStart(-1),
		mLastSelectionEnd(-1),
		mLastContextMenuX(-1),
		mPrevalidateFunc( prevalidate_func ),
		mCursorColor(		LLUI::sColorsGroup->getColor( "TextCursorColor" ) ),
		mFgColor(			LLUI::sColorsGroup->getColor( "TextFgColor" ) ),
		mReadOnlyFgColor(	LLUI::sColorsGroup->getColor( "TextFgReadOnlyColor" ) ),
		mTentativeFgColor(	LLUI::sColorsGroup->getColor( "TextFgTentativeColor" ) ),
		mWriteableBgColor(	LLUI::sColorsGroup->getColor( "TextBgWriteableColor" ) ),
		mReadOnlyBgColor(	LLUI::sColorsGroup->getColor( "TextBgReadOnlyColor" ) ),
		mFocusBgColor(		LLUI::sColorsGroup->getColor( "TextBgFocusColor" ) ),
		mBorderThickness( border_thickness ),
		mIgnoreArrowKeys( FALSE ),
		mIgnoreTab( TRUE ),
		mDrawAsterixes( FALSE ),
		mHandleEditKeysDirectly( FALSE ),
		mSelectAllonFocusReceived( FALSE ),
		mSelectAllonCommit( TRUE ),
		mPassDelete(FALSE),
		mReadOnly(FALSE),
		mHaveHistory(FALSE),
		mImage( sImage ),
		mReplaceNewlinesWithSpaces( TRUE ),
		mSpellCheckable( FALSE ),
		mAllowTranslate(TRUE)
{
	llassert( max_length_bytes > 0 );

	// Initialize current history line iterator
	mCurrentHistoryLine = mLineHistory.begin();

	if (font)
	{
		mGLFont = font;
	}
	else
	{
		mGLFont = LLFontGL::getFontSansSerifSmall();
	}

	setFocusLostCallback(focus_lost_callback);

	setTextPadding(0, 0);

	mScrollTimer.reset();

	setText(default_text);
	
	setCursor(mText.length());

	// Scalable UI somehow made these rectangles off-by-one.
	// I don't know why. JC
	LLRect border_rect(0, getRect().getHeight()-1, getRect().getWidth()-1, 0);
	mBorder = new LLViewBorder( std::string("line ed border"), border_rect, border_bevel, border_style, mBorderThickness );
	addChild( mBorder );
	mBorder->setFollows(FOLLOWS_LEFT|FOLLOWS_RIGHT|FOLLOWS_TOP|FOLLOWS_BOTTOM);

	if( ! sImage)
	{
		sImage = LLUI::getUIImage("sm_rounded_corners_simple.tga");
	}
	mImage = sImage;

	// make the popup menu available
	//LLMenuGL* menu = LLUICtrlFactory::getInstance()->buildMenu("menu_texteditor.xml", parent_view);
	LLMenuGL* menu = LLUICtrlFactory::getInstance()->buildMenu("menu_rightclick_text.xml",this);
       if (!menu)
       {
                 menu = new LLMenuGL(LLStringUtil::null);
       }

       defineMenuCallbacks(menu);
       mPopupMenuHandle = menu->getHandle(); 
       menu->setBorderColor(gColors.getColor("MenuItemDisabledColor"));
       menu->setBackgroundColor(gColors.getColor("MenuPopupBgColor"));
}


LLLineEditor::~LLLineEditor()
{
	mCommitOnFocusLost = FALSE;

	gFocusMgr.releaseFocusIfNeeded( this );

	if( gEditMenuHandler == this )
	{
		gEditMenuHandler = NULL;
	}
	LLView::deleteViewByHandle(mPopupMenuHandle);
}


void LLLineEditor::onFocusReceived()
{
	LLUICtrl::onFocusReceived();
	updateAllowingLanguageInput();
}

void LLLineEditor::onFocusLost()
{
	// The call to updateAllowLanguageInput()
	// when loosing the keyboard focus *may*
	// indirectly invoke handleUnicodeCharHere(), 
	// so it must be called before onCommit.
	updateAllowingLanguageInput();

	if( mCommitOnFocusLost && mText.getString() != mPrevText) 
	{
		onCommit();
	}

	if( gEditMenuHandler == this )
	{
		gEditMenuHandler = NULL;
	}

	getWindow()->showCursorFromMouseMove();

	LLUICtrl::onFocusLost();
}

void LLLineEditor::onCommit()
{
	// put current line into the line history
	updateHistory();

	LLUICtrl::onCommit();

	// Selection on commit needs to be turned off when evaluating maths
	// expressions, to allow indication of the error position
	if (mSelectAllonCommit) selectAll();
}


// line history support
void LLLineEditor::updateHistory()
{
	// On history enabled line editors, remember committed line and
	// reset current history line number.
	// Be sure only to remember lines that are not empty and that are
	// different from the last on the list.
	if( mHaveHistory && getLength() )
	{
		if( !mLineHistory.empty() )
		{
			// When not empty, last line of history should always be blank.
			if( mLineHistory.back().empty() )
			{
				// discard the empty line
				mLineHistory.pop_back();
			}
			else
			{
				LL_WARNS("") << "Last line of history was not blank." << LL_ENDL;
			}
		}

		// Add text to history, ignoring duplicates
		if( mLineHistory.empty() || getText() != mLineHistory.back() )
		{
			mLineHistory.push_back( getText() );
		}

		// Restore the blank line and set mCurrentHistoryLine to point at it
		mLineHistory.push_back( "" );
		mCurrentHistoryLine = mLineHistory.end() - 1;
	}
}

void LLLineEditor::reshape(S32 width, S32 height, BOOL called_from_parent)
{
	LLUICtrl::reshape(width, height, called_from_parent);
	setTextPadding(mTextPadLeft, mTextPadRight); // For clamping side-effect.
	setCursor(mCursorPos); // For clamping side-effect.
}

void LLLineEditor::setEnabled(BOOL enabled)
{
	mReadOnly = !enabled;
	setTabStop(!mReadOnly);
	updateAllowingLanguageInput();
}


void LLLineEditor::setMaxTextLength(S32 max_text_length)
{
	S32 max_len = llmax(0, max_text_length);
	mMaxLengthBytes = max_len;
} 

void LLLineEditor::setTextPadding(S32 left, S32 right)
{
	mTextPadLeft = llclamp(left, 0, getRect().getWidth());
	mTextPadRight = llclamp(right, 0, getRect().getWidth());
	mMinHPixels = UI_LINEEDITOR_H_PAD + mTextPadLeft;
	mMaxHPixels = getRect().getWidth() - mMinHPixels - mTextPadRight;
}


void LLLineEditor::setText(const LLStringExplicit &new_text)
{
	// If new text is identical, don't copy and don't move insertion point
	if (mText.getString() == new_text)
	{
		return;
	}

	// Check to see if entire field is selected.
	S32 len = mText.length();
	BOOL all_selected = (len > 0)
		&& (( mSelectionStart == 0 && mSelectionEnd == len ) 
			|| ( mSelectionStart == len && mSelectionEnd == 0 ));

	// Do safe truncation so we don't split multi-byte characters
	// also consider entire string selected when mSelectAllonFocusReceived is set on an empty, focused line editor
	all_selected = all_selected || (len == 0 && hasFocus() && mSelectAllonFocusReceived);

	std::string truncated_utf8 = new_text;
	if (truncated_utf8.size() > (U32)mMaxLengthBytes)
	{	
		truncated_utf8 = utf8str_truncate(new_text, mMaxLengthBytes);
	}
	mText.assign(truncated_utf8);

	if (all_selected)
	{
		// ...keep whole thing selected
		selectAll();
	}
	else
	{
		// try to preserve insertion point, but deselect text
		deselect();
	}
	setCursor(llmin((S32)mText.length(), getCursor()));

	// Set current history line to end of history.
	// RC Fix, its really not safe to just take 1 of the end itterator, if end==begin
	// that leaves an invalid state upseting the secure STL checks
	if(mLineHistory.empty())
	{
		mCurrentHistoryLine = mLineHistory.begin();
	}
	else
	{
		mCurrentHistoryLine = mLineHistory.end() - 1;
	}

	mPrevText = mText;
}


// Picks a new cursor position based on the actual screen size of text being drawn.
S32 LLLineEditor::calculateCursorFromMouse( S32 local_mouse_x ) const
{
	const llwchar* wtext = mText.getWString().c_str();
	LLWString asterix_text;
	if (mDrawAsterixes)
	{
		for (S32 i = 0; i < mText.length(); i++)
		{
			asterix_text += (llwchar) 0x2022L;
		}
		wtext = asterix_text.c_str();
	}

	return mScrollHPos + 
		mGLFont->charFromPixelOffset(
			wtext, mScrollHPos,
			(F32)(local_mouse_x - mMinHPixels),
			(F32)(mMaxHPixels - mMinHPixels + 1)); // min-max range is inclusive

}
// Picks a new cursor position based on the actual screen size of text being drawn.
void LLLineEditor::setCursorAtLocalPos( S32 local_mouse_x )
{
	setCursor(calculateCursorFromMouse(local_mouse_x));
}

void LLLineEditor::setCursor( S32 pos )
{
	S32 old_cursor_pos = getCursor();
	mCursorPos = llclamp( pos, 0, mText.length());

	S32 pixels_after_scroll = findPixelNearestPos();
	if( pixels_after_scroll > mMaxHPixels )
	{
		S32 width_chars_to_left = mGLFont->getWidth(mText.getWString().c_str(), 0, mScrollHPos);
		S32 last_visible_char = mGLFont->maxDrawableChars(mText.getWString().c_str(), llmax(0.f, (F32)(mMaxHPixels - mMinHPixels + width_chars_to_left))); 
		S32 min_scroll = mGLFont->firstDrawableChar(mText.getWString().c_str(), (F32)(mMaxHPixels - mMinHPixels - UI_LINEEDITOR_CURSOR_THICKNESS - UI_LINEEDITOR_H_PAD), mText.length(), getCursor());
		if (old_cursor_pos == last_visible_char)
		{
			mScrollHPos = llmin(mText.length(), llmax(min_scroll, mScrollHPos + SCROLL_INCREMENT_ADD));
		}
		else
		{
			mScrollHPos = min_scroll;
		}
	}
	else if (getCursor() < mScrollHPos)
	{
		if (old_cursor_pos == mScrollHPos)
		{
			mScrollHPos = llmax(0, llmin(getCursor(), mScrollHPos - SCROLL_INCREMENT_DEL));
		}
		else
		{
			mScrollHPos = getCursor();
		}
	}
}


void LLLineEditor::setCursorToEnd()
{
	setCursor(mText.length());
	deselect();
}

BOOL LLLineEditor::canDeselect() const
{
	return hasSelection();
}

void LLLineEditor::deselect()
{
	mSelectionStart = 0;
	mSelectionEnd = 0;
	mIsSelecting = FALSE;
}

BOOL LLLineEditor::context_enable_cut(void* data)
{
	LLLineEditor* line = (LLLineEditor*)data;
	return (line && line->canCut());
}

void LLLineEditor::context_cut(void* data)
{
	LLLineEditor* line = (LLLineEditor*)data;
	if(line)line->cut();
}

BOOL LLLineEditor::context_enable_copy(void* data)
{
	LLLineEditor* line = (LLLineEditor*)data;
	return (line &&line->canCopy());
}

void LLLineEditor::context_copy(void* data)
{
	LLLineEditor* line = (LLLineEditor*)data;
	if(line)line->copy();
}

void LLLineEditor::spell_correct(void* data)
{
	SpellMenuBind* tempBind = (SpellMenuBind*)data;
	LLLineEditor* line = tempBind->origin;
	if(tempBind && line)
	{
		llinfos << ((LLMenuItemCallGL *)(tempBind->menuItem))->getName() << " : " << tempBind->origin->getName() << " : " << tempBind->word << llendl;
		if(line)line->spellReplace(tempBind);

	}
}

BOOL LLLineEditor::context_enable_translate(void* data)
{
	LLLineEditor* line = (LLLineEditor*)data;
	return line && line->canTranslate();
}

void LLLineEditor::context_translate(void * data)
{
	LLLineEditor* line = (LLLineEditor*)data;
	LLMenuGL* menu = line ? (LLMenuGL*)(line->mPopupMenuHandle.get()) : NULL;
	LLMenuGL* translate_menu = menu ? menu->getChildMenuByName("Translation Options", TRUE) : NULL;
	if (!translate_menu)
	{
		return;
	}
	const std::string to_lang = translate_menu->getHighlightedItem()->getName();

	bool has_text = false;
	S32 start, length;
	if (line->hasSelection())
	{
		// translate selection
		start = llmin(line->mSelectionStart, line->mSelectionEnd);
		length = abs(line->mSelectionEnd - line->mSelectionStart);
		has_text = length > 0;
	}
	else
	{
		// translate one word at click position
		S32 at = line->calculateCursorFromMouse(line->mLastContextMenuX);
		has_text = line->getWordBoundriesAt(at, &start, &length);
	}

	if (has_text)
	{
		std::string to_translate = wstring_to_utf8str(line->getWText().substr(start, length));
		LLHTTPClient::ResponderPtr result = LineChatTranslationReceiver::build(to_lang, line, start, length);
		LLTranslate::translateMessage(result,"", to_lang, to_translate);
	}
}

void LLLineEditor::spell_show(void * data)
{
	SpellMenuBind* tempBind = (SpellMenuBind*)data;
	LLLineEditor* line = tempBind->origin;

	if (tempBind && line)
	{
		BOOL show = (tempBind->word == "Show Misspellings");
		glggHunSpell->setSpellCheckHighlight(show);
	}
}

std::vector<S32> LLLineEditor::getMisspelledWordsPositions()
{
	std::vector<S32> thePosesOfBadWords;
    const LLWString& text = mText.getWString();

	//llinfos << "end of box is at " << cursorloc << " and end of text is at " << text.length() << llendl;
	S32 wordStart=0;
	S32 wordEnd=mStartSpellHere;
	while(wordEnd < mEndSpellHere)
	{
		//go through all the chars... XD	
		if( LLTextEditor::isPartOfWord( text[wordEnd] ) ) 

		{
			// Select word the cursor is over
			while ((wordEnd > 0) && LLTextEditor::isPartOfWord(text[wordEnd-1]))
			{
				wordEnd--;
			}
			wordStart=wordEnd;
			while ((wordEnd < (S32)text.length()) && LLTextEditor::isPartOfWord( text[wordEnd] ) )
			{
				wordEnd++;
			}	
			//got a word :D
			std::string selectedWord(std::string(text.begin(), 
				text.end()).substr(wordStart,wordEnd-wordStart));
			
			if(!glggHunSpell->isSpelledRight(selectedWord))
			{	
				//misspelled word here, and you have just right clicked on it!
				//get the center of this word..
				//S32 center =  llround( (wordEnd-wordStart)/2 ) + wordStart;
				//turn this cursor position into a pixel pos
				//center = findPixelNearestPos(center-getCursor());

				thePosesOfBadWords.push_back(
					wordStart);
				thePosesOfBadWords.push_back(wordEnd);
			}
		}
		wordEnd++;
	}
	return thePosesOfBadWords;
}

void LLLineEditor::spell_add(void* data)
{
	SpellMenuBind* tempBind = (SpellMenuBind*)data;
	if (tempBind)
	{
		glggHunSpell->addWordToCustomDictionary(tempBind->word);
		tempBind->origin->mPrevSpelledText="";//make it update
	}
}

BOOL LLLineEditor::context_enable_paste(void* data)
{
	LLLineEditor* line = (LLLineEditor*)data;
	return (line && line->canPaste());
}

void LLLineEditor::context_paste(void* data)
{
	LLLineEditor* line = (LLLineEditor*)data;
	if(line)line->paste();
}

BOOL LLLineEditor::context_enable_delete(void* data)
{
	LLLineEditor* line = (LLLineEditor*)data;
	return (line && line->canDoDelete());
}

void LLLineEditor::context_delete(void* data)
{
	LLLineEditor* line = (LLLineEditor*)data;
	if(line)line->doDelete();
}

BOOL LLLineEditor::context_enable_selectall(void* data)
{
	LLLineEditor* line = (LLLineEditor*)data;
	return (line && line->canSelectAll());
}

void LLLineEditor::context_selectall(void* data)
{
	LLLineEditor* line = (LLLineEditor*)data;
	if(line)line->selectAll();
}

void LLLineEditor::startSelection()
{
	mIsSelecting = TRUE;
	mSelectionStart = getCursor();
	mSelectionEnd = getCursor();
}

void LLLineEditor::endSelection()
{
	if( mIsSelecting )
	{
		mIsSelecting = FALSE;
		mSelectionEnd = getCursor();
	}
}

BOOL LLLineEditor::canSelectAll() const
{
	return TRUE;
}

void LLLineEditor::selectAll()
{
	mSelectionStart = mText.length();
	mSelectionEnd = 0;
	setCursor(mSelectionEnd);
	//mScrollHPos = 0;
	mIsSelecting = TRUE;
}


BOOL LLLineEditor::handleDoubleClick(S32 x, S32 y, MASK mask)
{
	setFocus( TRUE );

	if (mSelectionEnd == 0 && mSelectionStart == mText.length())
	{
		// if everything is selected, handle this as a normal click to change insertion point
		handleMouseDown(x, y, mask);
	}
	else
	{
		const LLWString& wtext = mText.getWString();

		BOOL doSelectAll = TRUE;

		// Select the word we're on
		if( LLTextEditor::isPartOfWord( wtext[mCursorPos] ) )
		{
			S32 old_selection_start = mLastSelectionStart;
			S32 old_selection_end = mLastSelectionEnd;

			// Select word the cursor is over
			while ((mCursorPos > 0) && LLTextEditor::isPartOfWord( wtext[mCursorPos-1] ))
			{	// Find the start of the word
				mCursorPos--;
			}
			startSelection();	

			while ((mCursorPos < (S32)wtext.length()) && LLTextEditor::isPartOfWord( wtext[mCursorPos] ) )
			{	// Find the end of the word
				mCursorPos++;
			}
			mSelectionEnd = mCursorPos;

			// If nothing changed, then the word was already selected.  Select the whole line.
			doSelectAll = (old_selection_start == mSelectionStart) &&  
						  (old_selection_end   == mSelectionEnd);
		}
		
		if ( doSelectAll )
		{	// Select everything
			selectAll();
		}
	}

	// We don't want handleMouseUp() to "finish" the selection (and thereby
	// set mSelectionEnd to where the mouse is), so we finish the selection 
	// here.
	mIsSelecting = FALSE;  

	// delay cursor flashing
	mKeystrokeTimer.reset();

	// take selection to 'primary' clipboard
	updatePrimary();

	return TRUE;
}


BOOL LLLineEditor::handleRightMouseDown( S32 x, S32 y, MASK mask )
{
	setFocus(TRUE);

	//setCursorAtLocalPos( x);
	S32 wordStart = 0;
	S32 wordLen = 0;
	S32 pos = calculateCursorFromMouse(x);

	LLMenuGL* menu = (LLMenuGL*)mPopupMenuHandle.get();
	if (menu)
	{
		if(menu->isOpen())
		{
			menu->setVisible(FALSE);
		}
		for (int i = 0;i<(int)suggestionMenuItems.size();i++)
		{
			SpellMenuBind * tempBind = suggestionMenuItems[i];
			if (tempBind)
			{
				menu->remove((LLMenuItemCallGL *)tempBind->menuItem);
				((LLMenuItemCallGL *)tempBind->menuItem)->die();
				//delete tempBind->menuItem;
				//tempBind->menuItem = NULL;
				delete tempBind;
			}
		}
		suggestionMenuItems.clear();

		// spell_check="true" in xui
		menu->setItemVisible("Spelsep", !mReadOnly && mSpellCheckable);
		if (!mReadOnly && mSpellCheckable)
		{
			// search for word matches
			bool is_word_part = getWordBoundriesAt(pos, &wordStart, &wordLen);
			if (is_word_part)
			{
				const LLWString& text = mText.getWString();
				std::string selectedWord(std::string(text.begin(), text.end()).substr(wordStart,wordLen));
				
				if (!glggHunSpell->isSpelledRight(selectedWord))
				{	
					//misspelled word here, and you have just right clicked on it!
					std::vector<std::string> suggs = glggHunSpell->getSuggestionList(selectedWord);

					for (int i = 0; i<(int)suggs.size() ;i++)
					{
						SpellMenuBind * tempStruct = new SpellMenuBind;
						tempStruct->origin = this;
						tempStruct->word = suggs[i];
						tempStruct->wordPositionEnd = wordStart + wordLen;
						tempStruct->wordPositionStart=wordStart;
						LLMenuItemCallGL * suggMenuItem = new LLMenuItemCallGL(
							tempStruct->word, spell_correct, NULL, tempStruct);
						//new LLMenuItemCallGL("Select All", context_selectall, NULL, this));
						tempStruct->menuItem = suggMenuItem;
						suggestionMenuItems.push_back(tempStruct);
						menu->append(suggMenuItem);
					}
					SpellMenuBind * tempStruct = new SpellMenuBind;
					tempStruct->origin = this;
					tempStruct->word = selectedWord;
					tempStruct->wordPositionEnd = wordStart + wordLen;
					tempStruct->wordPositionStart=wordStart;
					LLMenuItemCallGL * suggMenuItem = new LLMenuItemCallGL(
						"Add Word", spell_add, NULL, tempStruct);
					tempStruct->menuItem = suggMenuItem;
					suggestionMenuItems.push_back(tempStruct);
					menu->append(suggMenuItem);
				}
			}

			SpellMenuBind * tempStruct = new SpellMenuBind;
			tempStruct->origin = this;
			if (glggHunSpell->getSpellCheckHighlight())
			{
				tempStruct->word = "Hide Misspellings";
			}
			else
			{
				tempStruct->word = "Show Misspellings";
			}
			LLMenuItemCallGL * suggMenuItem = new LLMenuItemCallGL(
				tempStruct->word, spell_show, NULL, tempStruct);
			tempStruct->menuItem = suggMenuItem;
			suggestionMenuItems.push_back(tempStruct);
			menu->append(suggMenuItem);
		}

		mLastContextMenuX = x;
		menu->buildDrawLabels();
		menu->updateParent(LLMenuGL::sMenuContainer);
		LLMenuGL::showPopup(this, menu, x, y);
	}
	return TRUE;
}

BOOL LLLineEditor::handleMouseDown(S32 x, S32 y, MASK mask)
{
	// Check first whether the "clear search" button wants to deal with this.
	if(childrenHandleMouseDown(x, y, mask) != NULL) 
	{
		return TRUE;
	}
	if (mSelectAllonFocusReceived
		&& gFocusMgr.getKeyboardFocus() != this)
	{
		setFocus( TRUE );
	}
	else
	{
		mLastSelectionStart = -1;
		mLastSelectionStart = -1;

		setFocus( TRUE );

		if (mask & MASK_SHIFT)
		{
			// Handle selection extension
			S32 old_cursor_pos = getCursor();
			setCursorAtLocalPos(x);

			if (hasSelection())
			{
				/* Mac-like behavior - extend selection towards the cursor
				if (getCursor() < mSelectionStart
					&& getCursor() < mSelectionEnd)
				{
					// ...left of selection
					mSelectionStart = llmax(mSelectionStart, mSelectionEnd);
					mSelectionEnd = getCursor();
				}
				else if (getCursor() > mSelectionStart
					&& getCursor() > mSelectionEnd)
				{
					// ...right of selection
					mSelectionStart = llmin(mSelectionStart, mSelectionEnd);
					mSelectionEnd = getCursor();
				}
				else
				{
					mSelectionEnd = getCursor();
				}
				*/
				// Windows behavior
				mSelectionEnd = getCursor();
			}
			else
			{
				mSelectionStart = old_cursor_pos;
				mSelectionEnd = getCursor();
			}
			// assume we're starting a drag select
			mIsSelecting = TRUE;
		}
		else
		{
			// Save selection for word/line selecting on double-click
			mLastSelectionStart = mSelectionStart;
			mLastSelectionEnd = mSelectionEnd;

			// Move cursor and deselect for regular click
			setCursorAtLocalPos( x );
			deselect();
			startSelection();
		}

		gFocusMgr.setMouseCapture( this );
	}

	// delay cursor flashing
	mKeystrokeTimer.reset();

	return TRUE;
}

BOOL LLLineEditor::handleMiddleMouseDown(S32 x, S32 y, MASK mask)
{
        // llinfos << "MiddleMouseDown" << llendl;
	setFocus( TRUE );
	if( canPastePrimary() )
	{
		setCursorAtLocalPos(x);
		pastePrimary();
	}
	return TRUE;
}

BOOL LLLineEditor::handleHover(S32 x, S32 y, MASK mask)
{
	BOOL handled = FALSE;
	// Check first whether the "clear search" button wants to deal with this.
	if(!hasMouseCapture())
	{
		if(childrenHandleHover(x, y, mask) != NULL) 
		{
			return TRUE;
		}
	}

	if( (hasMouseCapture()) && mIsSelecting )
	{
		if (x != mLastSelectionX || y != mLastSelectionY)
		{
			mLastSelectionX = x;
			mLastSelectionY = y;
		}
		// Scroll if mouse cursor outside of bounds
		if (mScrollTimer.hasExpired())
		{
			S32 increment = llround(mScrollTimer.getElapsedTimeF32() / AUTO_SCROLL_TIME);
			mScrollTimer.reset();
			mScrollTimer.setTimerExpirySec(AUTO_SCROLL_TIME);
			if( (x < mMinHPixels) && (mScrollHPos > 0 ) )
			{
				// Scroll to the left
				mScrollHPos = llclamp(mScrollHPos - increment, 0, mText.length());
			}
			else
			if( (x > mMaxHPixels) && (mCursorPos < (S32)mText.length()) )
			{
				// If scrolling one pixel would make a difference...
				S32 pixels_after_scrolling_one_char = findPixelNearestPos(1);
				if( pixels_after_scrolling_one_char >= mMaxHPixels )
				{
					// ...scroll to the right
					mScrollHPos = llclamp(mScrollHPos + increment, 0, mText.length());
				}
			}
		}

		setCursorAtLocalPos( x );
		mSelectionEnd = getCursor();

		// delay cursor flashing
		mKeystrokeTimer.reset();

		getWindow()->setCursor(UI_CURSOR_IBEAM);
		lldebugst(LLERR_USER_INPUT) << "hover handled by " << getName() << " (active)" << llendl;		
		handled = TRUE;
	}

	if( !handled  )
	{
		getWindow()->setCursor(UI_CURSOR_IBEAM);
		lldebugst(LLERR_USER_INPUT) << "hover handled by " << getName() << " (inactive)" << llendl;		
		handled = TRUE;
	}

	return handled;
}


BOOL LLLineEditor::handleMouseUp(S32 x, S32 y, MASK mask)
{
	BOOL	handled = FALSE;

	if( hasMouseCapture() )
	{
		gFocusMgr.setMouseCapture( NULL );
		handled = TRUE;
	}

	// Check first whether the "clear search" button wants to deal with this.
	if(!handled && childrenHandleMouseUp(x, y, mask) != NULL) 
	{
		return TRUE;
	}

	if( mIsSelecting )
	{
		setCursorAtLocalPos( x );
		mSelectionEnd = getCursor();

		handled = TRUE;
	}

	if( handled )
	{
		// delay cursor flashing
		mKeystrokeTimer.reset();

		// take selection to 'primary' clipboard
		updatePrimary();
	}

	return handled;
}


// Remove a single character from the text
void LLLineEditor::removeChar()
{
	if( getCursor() > 0 )
	{
		mText.erase(getCursor() - 1, 1);

		setCursor(getCursor() - 1);
	}
	else
	{
		reportBadKeystroke();
	}
}


void LLLineEditor::addChar(const llwchar uni_char)
{
	llwchar new_c = uni_char;
	if (hasSelection())
	{
		deleteSelection();
	}
	else if (LL_KIM_OVERWRITE == gKeyboard->getInsertMode())
	{
		mText.erase(getCursor(), 1);
	}

	S32 cur_bytes = mText.getString().size();
	S32 new_bytes = wchar_utf8_length(new_c);

	BOOL allow_char = TRUE;

	// Check byte length limit
	if ((new_bytes + cur_bytes) > mMaxLengthBytes)
	{
		allow_char = FALSE;
	}

	if (allow_char)
	{
		// Will we need to scroll?
		LLWString w_buf;
		w_buf.assign(1, new_c);

		mText.insert(getCursor(), w_buf);
		setCursor(getCursor() + 1);
	}
	else
	{
		reportBadKeystroke();
	}

	getWindow()->hideCursorUntilMouseMove();
}

// Extends the selection box to the new cursor position
void LLLineEditor::extendSelection( S32 new_cursor_pos )
{
	if( !mIsSelecting )
	{
		startSelection();
	}
	
	setCursor(new_cursor_pos);
	mSelectionEnd = getCursor();
}


void LLLineEditor::setSelection(S32 start, S32 end)
{
	S32 len = mText.length();

	mSelectionStart = llclamp(start, 0, len);
	mSelectionEnd = llclamp(end, 0, len);
	setCursor(end);
}

void LLLineEditor::setDrawAsterixes(BOOL b)
{
	mDrawAsterixes = b;
	updateAllowingLanguageInput();
}

S32 LLLineEditor::prevWordPos(S32 cursorPos) const
{
	const LLWString& wtext = mText.getWString();
	while( (cursorPos > 0) && (wtext[cursorPos-1] == ' ') )
	{
		cursorPos--;
	}
	while( (cursorPos > 0) && LLTextEditor::isPartOfWord( wtext[cursorPos-1] ) )
	{
		cursorPos--;
	}
	return cursorPos;
}

S32 LLLineEditor::nextWordPos(S32 cursorPos) const
{
	const LLWString& wtext = mText.getWString();
	while( (cursorPos < getLength()) && LLTextEditor::isPartOfWord( wtext[cursorPos] ) )
	{
		cursorPos++;
	} 
	while( (cursorPos < getLength()) && (wtext[cursorPos] == ' ') )
	{
		cursorPos++;
	}
	return cursorPos;
}

BOOL LLLineEditor::getWordBoundriesAt(const S32 at, S32* word_begin, S32* word_length) const
{
	const LLWString& wtext = mText.getWString();
	S32 pos = at;
	if (LLTextEditor::isPartOfWord(wtext[pos]))
	{
		while ( (pos > 0) && LLTextEditor::isPartOfWord(wtext[pos - 1]) )
		{
			pos--;
		}
		*word_begin = pos;
		while ( (pos < (S32)wtext.length()) && LLTextEditor::isPartOfWord(wtext[pos]) )
		{
			pos++;
		}
		*word_length = pos - *word_begin;
		return TRUE;
	}
	return FALSE;
}

BOOL LLLineEditor::handleSelectionKey(KEY key, MASK mask)
{
	BOOL handled = FALSE;

	if( mask & MASK_SHIFT )
	{
		handled = TRUE;

		switch( key )
		{
		case KEY_LEFT:
			if( 0 < getCursor() )
			{
				S32 cursorPos = getCursor() - 1;
				if( mask & MASK_CONTROL )
				{
					cursorPos = prevWordPos(cursorPos);
				}
				extendSelection( cursorPos );
			}
			else
			{
				reportBadKeystroke();
			}
			break;

		case KEY_RIGHT:
			if( getCursor() < mText.length())
			{
				S32 cursorPos = getCursor() + 1;
				if( mask & MASK_CONTROL )
				{
					cursorPos = nextWordPos(cursorPos);
				}
				extendSelection( cursorPos );
			}
			else
			{
				reportBadKeystroke();
			}
			break;

		case KEY_PAGE_UP:
		case KEY_HOME:
			extendSelection( 0 );
			break;
		
		case KEY_PAGE_DOWN:
		case KEY_END:
			{
				S32 len = mText.length();
				if( len )
				{
					extendSelection( len );
				}
				break;
			}

		default:
			handled = FALSE;
			break;
		}
	}

	if (!handled && mHandleEditKeysDirectly)
	{
		if( (MASK_CONTROL & mask) && ('A' == key) )
		{
			if( canSelectAll() )
			{
				selectAll();
			}
			else
			{
				reportBadKeystroke();
			}
			handled = TRUE;
		}
	}

	if(handled)
	{
		// take selection to 'primary' clipboard
		updatePrimary();
	}
 
	return handled;
}

void LLLineEditor::deleteSelection()
{
	if( !mReadOnly && hasSelection() )
	{
		S32 left_pos = llmin( mSelectionStart, mSelectionEnd );
		S32 selection_length = llabs( mSelectionStart - mSelectionEnd );

		mText.erase(left_pos, selection_length);
		deselect();
		setCursor(left_pos);
	}
}

BOOL LLLineEditor::canCut() const
{
	return !mReadOnly && !mDrawAsterixes && hasSelection();
}


// method to define the associated callbacks
void LLLineEditor::defineMenuCallbacks(LLMenuGL* menu) {

       menu->setCtrlResponse(LLCallbackInformation::LL_MENU_ITEM_CALL_GL_ON_ENABLE,
                             "Cut Text",
                             this,
                             (void*)context_enable_cut);
       menu->setCtrlResponse(LLCallbackInformation::LL_MENU_ITEM_CALL_GL_ON_CLICK,
                             "Cut Text",
                             this,
                             (void*)context_cut);

       menu->setCtrlResponse(LLCallbackInformation::LL_MENU_ITEM_CALL_GL_ON_ENABLE,
                             "Copy Text",
                             this,
                             (void*)context_enable_copy);
       menu->setCtrlResponse(LLCallbackInformation::LL_MENU_ITEM_CALL_GL_ON_CLICK,
                             "Copy Text",
                             this,
                             (void*)context_copy);

       menu->setCtrlResponse(LLCallbackInformation::LL_MENU_ITEM_CALL_GL_ON_ENABLE,
                             "Paste Text",
                             this,
                             (void*)context_enable_paste);
       menu->setCtrlResponse(LLCallbackInformation::LL_MENU_ITEM_CALL_GL_ON_CLICK,
                             "Paste Text",
                             this,
                             (void*)context_paste);

       menu->setCtrlResponse(LLCallbackInformation::LL_MENU_ITEM_CALL_GL_ON_ENABLE,
                             "Delete Text",
                             this,
                             (void*)context_enable_delete);
       menu->setCtrlResponse(LLCallbackInformation::LL_MENU_ITEM_CALL_GL_ON_CLICK,
                             "Delete Text",
                             this,
                             (void*)context_delete);

       menu->setCtrlResponse(LLCallbackInformation::LL_MENU_ITEM_CALL_GL_ON_ENABLE,
                             "Select All Text",
                             this,
                             (void*)context_enable_selectall);
       menu->setCtrlResponse(1+LLCallbackInformation::LL_MENU_ITEM_CALL_GL_ON_CLICK,
                             "Select All Text",
                             this,
                             (void*)context_selectall);

       menu->setCtrlResponse(LLCallbackInformation::LL_MENU_ITEM_CALL_GL_ON_ENABLE,
                             "Translate Text",
                             this,
                             (void*)context_enable_translate);
       menu->setCtrlResponse(LLCallbackInformation::LL_MENU_ITEM_CALL_GL_TRANSLATE,
                             "Translate Text",
                             this,
                             (void*)context_translate);

}



// cut selection to clipboard
void LLLineEditor::cut()
{
	if( canCut() )
	{
		// Prepare for possible rollback
		LLLineEditorRollback rollback( this );


		S32 left_pos = llmin( mSelectionStart, mSelectionEnd );
		S32 length = llabs( mSelectionStart - mSelectionEnd );
		gClipboard.copyFromSubstring( mText.getWString(), left_pos, length );
		deleteSelection();

		// Validate new string and rollback the if needed.
		BOOL need_to_rollback = ( mPrevalidateFunc && !mPrevalidateFunc( mText.getWString() ) );
		if( need_to_rollback )
		{
			rollback.doRollback( this );
			reportBadKeystroke();
		}
		else
		if( mKeystrokeCallback )
		{
			mKeystrokeCallback( this, mCallbackUserData );
		}
	}
}

BOOL LLLineEditor::canCopy() const
{
	return !mDrawAsterixes && hasSelection();
}


// copy selection to clipboard
void LLLineEditor::copy()
{
	if( canCopy() )
	{
		S32 left_pos = llmin( mSelectionStart, mSelectionEnd );
		S32 length = llabs( mSelectionStart - mSelectionEnd );
		gClipboard.copyFromSubstring( mText.getWString(), left_pos, length );
	}
}

void LLLineEditor::spellReplace(SpellMenuBind* spellData)
{
	mText.erase(spellData->wordPositionStart,
		spellData->wordPositionEnd - spellData->wordPositionStart);
	insert(spellData->word,spellData->wordPositionStart);
	mCursorPos+=spellData->word.length() - (spellData->wordPositionEnd-spellData->wordPositionStart);


}

void LLLineEditor::translationReplace(const std::string &translation, const S32 orig_start, const S32 orig_length)
{
	//*TODO: should probably check if the content was modified since the http query
	//       was made, so we don't insert text in the wrong place.
	BOOL replace = gSavedSettings.getBOOL("EmeraldTranslateReplace");
	std::string text = replace ? translation : " (" + translation + ")";
	S32 pos = replace ? orig_start : orig_start + orig_length;
	if (replace)
	{
		mText.erase(orig_start, orig_length);
	}
	insert(text, pos);

	S32 text_wlen = utf8str_to_wstring(text).length();
	if (hasSelection())
	{
		setSelection(pos, pos + text_wlen);
	}
	setCursor(pos + text_wlen);
}

BOOL LLLineEditor::canTranslate() const
{
	// if allow_translate="true" in xui, and if other factors permit, we allow it
	S32 pos = calculateCursorFromMouse(mLastContextMenuX);
	const LLWString& wtext = getWText();
	bool is_word_part = (pos > -1) && LLTextEditor::isPartOfWord(wtext[pos]);
	return (mAllowTranslate && !mReadOnly && (is_word_part || hasSelection()));
}

void LLLineEditor::insert(std::string what, S32 wher)
{
	LLLineEditorRollback rollback(this);
	LLWString clean_string(utf8str_to_wstring(what));
	LLWStringUtil::replaceTabsWithSpaces(clean_string, 4);
	mText.insert(wher, clean_string);
	//see if we should move over the cursor acordingly
	// Validate new string and rollback the if needed.
	BOOL need_to_rollback = ( mPrevalidateFunc && !mPrevalidateFunc( mText.getWString() ) );
	if( need_to_rollback )
	{
		rollback.doRollback( this );
		reportBadKeystroke();
	}
	else if( mKeystrokeCallback )
		mKeystrokeCallback( this, mCallbackUserData );
}

BOOL LLLineEditor::canPaste() const
{
	return !mReadOnly && gClipboard.canPasteString(); 
}

void LLLineEditor::paste()
{
	bool is_primary = false;
	pasteHelper(is_primary);
}

void LLLineEditor::pastePrimary()
{
	bool is_primary = true;
	pasteHelper(is_primary);
}

// paste from primary (is_primary==true) or clipboard (is_primary==false)
void LLLineEditor::pasteHelper(bool is_primary)
{
	bool can_paste_it;
	if (is_primary)
	{
		can_paste_it = canPastePrimary();
	}
	else
	{
		can_paste_it = canPaste();
	}

	if (can_paste_it)
	{
		LLWString paste;
		if (is_primary)
		{
			paste = gClipboard.getPastePrimaryWString();
		}
		else 
		{
			paste = gClipboard.getPasteWString();
		}

		if (!paste.empty())
		{
			// Prepare for possible rollback
			LLLineEditorRollback rollback(this);
			
			// Delete any selected characters
			if ((!is_primary) && hasSelection())
			{
				deleteSelection();
			}

			// Clean up string (replace tabs and returns and remove characters that our fonts don't support.)
			LLWString clean_string(paste);
			LLWStringUtil::replaceTabsWithSpaces(clean_string, 4);
			//clean_string = wstring_detabify(paste, 1);
			LLWStringUtil::replaceChar(clean_string, '\n', mReplaceNewlinesWithSpaces ? ' ' : 182); // 182 == paragraph character

			// Insert the string

			// Check to see that the size isn't going to be larger than the max number of bytes
			U32 available_bytes = mMaxLengthBytes - wstring_utf8_length(mText);

			if ( available_bytes < (U32) wstring_utf8_length(clean_string) )
			{	// Doesn't all fit
				llwchar current_symbol = clean_string[0];
				U32 wchars_that_fit = 0;
				U32 total_bytes = wchar_utf8_length(current_symbol);

				//loop over the "wide" characters (symbols)
				//and check to see how large (in bytes) each symbol is.
				while ( total_bytes <= available_bytes )
				{
					//while we still have available bytes
					//"accept" the current symbol and check the size
					//of the next one
					current_symbol = clean_string[++wchars_that_fit];
					total_bytes += wchar_utf8_length(current_symbol);
				}
				// Truncate the clean string at the limit of what will fit
				clean_string = clean_string.substr(0, wchars_that_fit);
				reportBadKeystroke();
			}
 
			mText.insert(getCursor(), clean_string);
			setCursor( getCursor() + (S32)clean_string.length() );
			deselect();

			// Validate new string and rollback the if needed.
			BOOL need_to_rollback = ( mPrevalidateFunc && !mPrevalidateFunc( mText.getWString() ) );
			if( need_to_rollback )
			{
				rollback.doRollback( this );
				reportBadKeystroke();
			}
			else
			if( mKeystrokeCallback )
			{
				mKeystrokeCallback( this, mCallbackUserData );
			}
		}
	}
}

// copy selection to primary
void LLLineEditor::copyPrimary()
{
	if( canCopy() )
	{
		S32 left_pos = llmin( mSelectionStart, mSelectionEnd );
		S32 length = llabs( mSelectionStart - mSelectionEnd );
		gClipboard.copyFromPrimarySubstring( mText.getWString(), left_pos, length );
	}
}

BOOL LLLineEditor::canPastePrimary() const
{
	return !mReadOnly && gClipboard.canPastePrimaryString(); 
}

void LLLineEditor::updatePrimary()
{
	if(canCopy() )
	{
		copyPrimary();
	}
}

BOOL LLLineEditor::handleSpecialKey(KEY key, MASK mask)	
{
	BOOL handled = FALSE;

	switch( key )
	{
	case KEY_INSERT:
		if (mask == MASK_NONE)
		{
			gKeyboard->toggleInsertMode();
		}

		handled = TRUE;
		break;

	case KEY_BACKSPACE:
		if (!mReadOnly)
		{
			//llinfos << "Handling backspace" << llendl;
			if( hasSelection() )
			{
				deleteSelection();
			}
			else
			if( 0 < getCursor() )
			{
				removeChar();
			}
			else
			{
				reportBadKeystroke();
			}
		}
		handled = TRUE;
		break;

	case KEY_PAGE_UP:
	case KEY_HOME:
		if (!mIgnoreArrowKeys)
		{
			setCursor(0);
			handled = TRUE;
		}
		break;

	case KEY_PAGE_DOWN:
	case KEY_END:
		if (!mIgnoreArrowKeys)
		{
			S32 len = mText.length();
			if( len )
			{
				setCursor(len);
			}
			handled = TRUE;
		}
		break;

	case KEY_LEFT:
		if (mIgnoreArrowKeys && mask == MASK_NONE)
			break;
		if ((mask & MASK_ALT) == 0)
		{
			if( hasSelection() )
			{
				setCursor(llmin( getCursor() - 1, mSelectionStart, mSelectionEnd ));
			}
			else
			if( 0 < getCursor() )
			{
				S32 cursorPos = getCursor() - 1;
				if( mask & MASK_CONTROL )
				{
					cursorPos = prevWordPos(cursorPos);
				}
				setCursor(cursorPos);
			}
			else
			{
				reportBadKeystroke();
			}
			handled = TRUE;
		}
		break;

	case KEY_RIGHT:
		if (mIgnoreArrowKeys && mask == MASK_NONE)
			break;
		if ((mask & MASK_ALT) == 0)
		{
			if (hasSelection())
			{
				setCursor(llmax(getCursor() + 1, mSelectionStart, mSelectionEnd));
			}
			else
			if (getCursor() < mText.length())
			{
				S32 cursorPos = getCursor() + 1;
				if( mask & MASK_CONTROL )
				{
					cursorPos = nextWordPos(cursorPos);
				}
				setCursor(cursorPos);
			}
			else
			{
				reportBadKeystroke();
			}
			handled = TRUE;
		}
		break;

	// handle ctrl-uparrow if we have a history enabled line editor.
	case KEY_UP:
		if( mHaveHistory && ( MASK_CONTROL == mask ) )
		{
			if( mCurrentHistoryLine > mLineHistory.begin() )
			{
				mText.assign( *(--mCurrentHistoryLine) );
				setCursor(llmin((S32)mText.length(), getCursor()));
			}
			else
			{
				reportBadKeystroke();
			}
			handled = TRUE;
		}
		break;

	// handle ctrl-downarrow if we have a history enabled line editor
	case KEY_DOWN:
		if( mHaveHistory  && ( MASK_CONTROL == mask ) )
		{
			if( !mLineHistory.empty() && mCurrentHistoryLine < mLineHistory.end() - 1 )
			{
				mText.assign( *(++mCurrentHistoryLine) );
				setCursor(llmin((S32)mText.length(), getCursor()));
			}
			else
			{
				reportBadKeystroke();
			}
			handled = TRUE;
		}
		break;

	case KEY_RETURN:
		// store sent line in history
		updateHistory();
		break;

	case KEY_ESCAPE:
	    if (mRevertOnEsc && mText.getString() != mPrevText)
		{
			setText(mPrevText);
			// Note, don't set handled, still want to loose focus (won't commit becase text is now unchanged)
		}
		break;
		
	default:
		break;
	}

	if( !handled && mHandleEditKeysDirectly )
	{
		// Standard edit keys (Ctrl-X, Delete, etc,) are handled here instead of routed by the menu system.
		if( KEY_DELETE == key )
		{
			if( canDoDelete() )
			{
				doDelete();
			}
			else
			{
				reportBadKeystroke();
			}
			handled = TRUE;
		}
		else
		if( MASK_CONTROL & mask )
		{
			if( 'C' == key )
			{
				if( canCopy() )
				{
					copy();
				}
				else
				{
					reportBadKeystroke();
				}
				handled = TRUE;
			}
			else
			if( 'V' == key )
			{
				if( canPaste() )
				{
					paste();
				}
				else
				{
					reportBadKeystroke();
				}
				handled = TRUE;
			}
			else
			if( 'X' == key )
			{
				if( canCut() )
				{
					cut();
				}
				else
				{
					reportBadKeystroke();
				}
				handled = TRUE;
			}
		}
	}
	return handled;
}


BOOL LLLineEditor::handleKeyHere(KEY key, MASK mask )
{
	BOOL	handled = FALSE;
	BOOL	selection_modified = FALSE;

	// SL-51858: Key presses are not being passed to the Popup menu.
	// A proper fix is non-trivial so instead just close the menu.
	LLMenuGL* menu = (LLMenuGL*)mPopupMenuHandle.get();
	if (menu && menu->isOpen())
	{
		LLMenuGL::sMenuContainer->hideMenus();
	}

	if ( gFocusMgr.getKeyboardFocus() == this )
	{
		LLLineEditorRollback rollback( this );

		if( !handled )
		{
			handled = handleSelectionKey( key, mask );
			selection_modified = handled;
		}
		
		// Handle most keys only if the text editor is writeable.
		if ( !mReadOnly )
		{
			if( !handled )
			{
				handled = handleSpecialKey( key, mask );
			}
		}

		if( handled )
		{
			mKeystrokeTimer.reset();

			// Most keystrokes will make the selection box go away, but not all will.
			if( !selection_modified &&
				KEY_SHIFT != key &&
				KEY_CONTROL != key &&
				KEY_ALT != key &&
				KEY_CAPSLOCK )
			{
				deselect();
			}

			BOOL need_to_rollback = FALSE;

			// If read-only, don't allow changes
			need_to_rollback |= (mReadOnly && (mText.getString() == rollback.getText()));

			// Validate new string and rollback the keystroke if needed.
			need_to_rollback |= (mPrevalidateFunc && !mPrevalidateFunc(mText.getWString()));

			if (need_to_rollback)
			{
				rollback.doRollback(this);

				reportBadKeystroke();
			}

			// Notify owner if requested
			if (!need_to_rollback && handled)
			{
				if (mKeystrokeCallback)
				{
					mKeystrokeCallback(this, mCallbackUserData);
				}
			}
		}
	}

	return handled;
}


BOOL LLLineEditor::handleUnicodeCharHere(llwchar uni_char)
{
	if ((uni_char < 0x20) || (uni_char == 0x7F)) // Control character or DEL
	{
		return FALSE;
	}

	BOOL	handled = FALSE;

	if ( (gFocusMgr.getKeyboardFocus() == this) && getVisible() && !mReadOnly)
	{
		// SL-51858: Key presses are not being passed to the Popup menu.
		// A proper fix is non-trivial so instead just close the menu.
		LLMenuGL* menu = (LLMenuGL*)mPopupMenuHandle.get();
		if (menu && menu->isOpen())
		{
			LLMenuGL::sMenuContainer->hideMenus();
		}
		handled = TRUE;

		LLLineEditorRollback rollback( this );

		addChar(uni_char);

		mKeystrokeTimer.reset();

		deselect();

		BOOL need_to_rollback = FALSE;

		// Validate new string and rollback the keystroke if needed.
		need_to_rollback |= ( mPrevalidateFunc && !mPrevalidateFunc( mText.getWString() ) );

		if( need_to_rollback )
		{
			rollback.doRollback( this );

			reportBadKeystroke();
		}

		// Notify owner if requested
		if( !need_to_rollback && handled )
		{
			if( mKeystrokeCallback )
			{
				// HACK! The only usage of this callback doesn't do anything with the character.
				// We'll have to do something about this if something ever changes! - Doug
				mKeystrokeCallback( this, mCallbackUserData );
			}
		}
	}
	return handled;
}


BOOL LLLineEditor::canDoDelete() const
{
	return ( !mReadOnly && (!mPassDelete || (hasSelection() || (getCursor() < mText.length()))) );
}

void LLLineEditor::doDelete()
{
	if (canDoDelete())
	{
		// Prepare for possible rollback
		LLLineEditorRollback rollback( this );

		if (hasSelection())
		{
			deleteSelection();
		}
		else if ( getCursor() < mText.length())
		{	
			setCursor(getCursor() + 1);
			removeChar();
		}

		// Validate new string and rollback the if needed.
		BOOL need_to_rollback = ( mPrevalidateFunc && !mPrevalidateFunc( mText.getWString() ) );
		if( need_to_rollback )
		{
			rollback.doRollback( this );
			reportBadKeystroke();
		}
		else
		{
			if( mKeystrokeCallback )
			{
				mKeystrokeCallback( this, mCallbackUserData );
			}
		}
	}
}

void LLLineEditor::autoCorrectText()
{
	static BOOL *doAnything = rebind_llcontrol<BOOL>("EmeraldEnableAutoCorrect", &gSavedSettings, true);
	if (!mReadOnly && *doAnything && isSpellDirty())
	{
		S32 wordStart = 0;
		S32 wordEnd = mCursorPos-1;
		//llinfos <<"Checking Word, Cursor is at "<<mCursorPos<<" and text is "<<mText.getString().c_str()<<llendl;
		if (wordEnd < 1)
		{
			return;
		}
		const LLWString& text = mText.getWString();
		if (text.size() < 1 || LLTextEditor::isPartOfWord(text[wordEnd]))
		{
			return;//we only check on word breaks
		}

		wordEnd--;
		if (LLTextEditor::isPartOfWord(text[wordEnd]))
		{
			while ((wordEnd > 0) && (' '!=text[wordEnd-1]))
			{
				wordEnd--;
			}
			wordStart = wordEnd;		
			while ((wordEnd < (S32)text.length()) && (' '!=text[wordEnd] ) )
			{
				wordEnd++;
			}
			std::string lastTypedWord(std::string(text.begin(), 
			text.end()).substr(wordStart, wordEnd-wordStart));
			//llinfos << " The last typed word has been chosen, it is "<<lastTypedWord.c_str()<<llendl;
		
			std::string correctedWord(LGGAutoCorrect::getInstance()->replaceWord(lastTypedWord));
			if(correctedWord != lastTypedWord)
			{
				int dif = correctedWord.length()-lastTypedWord.length();
				std::string regText(mText);
				//int wordStart = regText.find(lastTypedWord);
				regText.replace(wordStart,lastTypedWord.length(),correctedWord);
				mText=regText;
				mCursorPos+=dif;
			}
		}
	}
}

void LLLineEditor::drawMisspelled(LLRect background)
{
	if (!mReadOnly && mSpellCheckable)
	{
		S32 newStartSpellHere = mScrollHPos;
		S32 cursorloc = calculateCursorFromMouse(mMaxHPixels);
		S32 newStopSpellHere = (((S32)mText.length())>cursorloc) ? cursorloc : (S32)mText.length();

		F32 elapsed = mSpellTimer.getElapsedTimeF32();
		if (S32(elapsed / 1) & 1) 
		{
			if (isSpellDirty() || (newStartSpellHere != mStartSpellHere) || ( newStopSpellHere != mEndSpellHere))
			{
				mStartSpellHere = newStartSpellHere;
				mEndSpellHere = newStopSpellHere;
				resetSpellDirty();
				misspellLocations=getMisspelledWordsPositions();
			}
		}

		if (glggHunSpell->getSpellCheckHighlight())
		{
			for (int i =0; i<(int)misspellLocations.size(); i++)
			{
				S32 wstart =findPixelNearestPos( misspellLocations[i]-getCursor());
				S32 wend = findPixelNearestPos(misspellLocations[++i]-getCursor());
				S32 maxw = getRect().getWidth();

				if (wend > maxw)
				{
					wend = maxw;
				}
				if (wstart > maxw)
				{
					wstart = maxw;
				}
				gGL.color4ub(255,0,0,200);
				//3 line zig zags..
				while (wstart < wend)
				{
					gl_line_2d(wstart, background.mBottom-1, wstart+3, background.mBottom+2);
					gl_line_2d(wstart+3, background.mBottom+2, wstart+6, background.mBottom-1);
					wstart+=6;
				}
			}
		}
	}
}

void LLLineEditor::draw()
{
	autoCorrectText();
	S32 text_len = mText.length();

	std::string saved_text;
	if (mDrawAsterixes)
	{
		saved_text = mText.getString();
		std::string text;
		for (S32 i = 0; i < mText.length(); i++)
		{
			text += "\xe2\x80\xa2";
		}
		mText = text;
	}

	// draw rectangle for the background
	LLRect background( 0, getRect().getHeight(), getRect().getWidth(), 0 );
	background.stretch( -mBorderThickness );

	LLColor4 bg_color = mReadOnlyBgColor;

#if 0 // for when we're ready for image art.
	if( hasFocus())
	{
		mImage->drawBorder(0, 0, getRect().getWidth(), getRect().getHeight(), gFocusMgr.getFocusColor(), gFocusMgr.getFocusFlashWidth());
	}
	mImage->draw(getLocalRect(), mReadOnly ? mReadOnlyBgColor : mWriteableBgColor  );
#else // the old programmer art.
	// drawing solids requires texturing be disabled
	{
		gGL.getTexUnit(0)->unbind(LLTexUnit::TT_TEXTURE);
		// draw background for text
		if( !mReadOnly )
		{
			if( gFocusMgr.getKeyboardFocus() == this )
			{
				bg_color = mFocusBgColor;
			}
			else
			{
				bg_color = mWriteableBgColor;
			}
		}
		gl_rect_2d(background, bg_color);
	}
#endif

	// draw text

	S32 cursor_bottom = background.mBottom + 1;
	S32 cursor_top = background.mTop - 1;

	LLColor4 text_color;
	if (!mReadOnly)
	{
		if (!getTentative())
		{
			text_color = mFgColor;
		}
		else
		{
			text_color = mTentativeFgColor;
		}
	}
	else
	{
		text_color = mReadOnlyFgColor;
	}
	LLColor4 label_color = mTentativeFgColor;

	if (hasPreeditString())
	{
		// Draw preedit markers.  This needs to be before drawing letters.
		for (U32 i = 0; i < mPreeditStandouts.size(); i++)
		{
			const S32 preedit_left = mPreeditPositions[i];
			const S32 preedit_right = mPreeditPositions[i + 1];
			if (preedit_right > mScrollHPos)
			{
				S32 preedit_pixels_left = findPixelNearestPos(llmax(preedit_left, mScrollHPos) - getCursor());
				S32 preedit_pixels_right = llmin(findPixelNearestPos(preedit_right - getCursor()), background.mRight);
				if (preedit_pixels_left >= background.mRight)
				{
					break;
				}
				if (mPreeditStandouts[i])
				{
					gl_rect_2d(preedit_pixels_left + PREEDIT_STANDOUT_GAP,
						background.mBottom + PREEDIT_STANDOUT_POSITION,
						preedit_pixels_right - PREEDIT_STANDOUT_GAP - 1,
						background.mBottom + PREEDIT_STANDOUT_POSITION - PREEDIT_STANDOUT_THICKNESS,
						(text_color * PREEDIT_STANDOUT_BRIGHTNESS + bg_color * (1 - PREEDIT_STANDOUT_BRIGHTNESS)).setAlpha(1.0f));
				}
				else
				{
					gl_rect_2d(preedit_pixels_left + PREEDIT_MARKER_GAP,
						background.mBottom + PREEDIT_MARKER_POSITION,
						preedit_pixels_right - PREEDIT_MARKER_GAP - 1,
						background.mBottom + PREEDIT_MARKER_POSITION - PREEDIT_MARKER_THICKNESS,
						(text_color * PREEDIT_MARKER_BRIGHTNESS + bg_color * (1 - PREEDIT_MARKER_BRIGHTNESS)).setAlpha(1.0f));
				}
			}
		}
	}

	S32 rendered_text = 0;
	F32 rendered_pixels_right = (F32)mMinHPixels;
	F32 text_bottom = (F32)background.mBottom + (F32)UI_LINEEDITOR_V_PAD;

	if( (gFocusMgr.getKeyboardFocus() == this) && hasSelection() )
	{
		S32 select_left;
		S32 select_right;
		if( mSelectionStart < getCursor() )
		{
			select_left = mSelectionStart;
			select_right = getCursor();
		}
		else
		{
			select_left = getCursor();
			select_right = mSelectionStart;
		}
		
		if( select_left > mScrollHPos )
		{
			// unselected, left side
			rendered_text = mGLFont->render( 
				mText, mScrollHPos,
				rendered_pixels_right, text_bottom,
				text_color,
				LLFontGL::LEFT, LLFontGL::BOTTOM,
				LLFontGL::NORMAL,
				select_left - mScrollHPos,
				mMaxHPixels - llround(rendered_pixels_right),
				&rendered_pixels_right);
		}
		
		if( (rendered_pixels_right < (F32)mMaxHPixels) && (rendered_text < text_len) )
		{
			LLColor4 color(1.f - bg_color.mV[0], 1.f - bg_color.mV[1], 1.f - bg_color.mV[2], 1.f);
			// selected middle
			S32 width = mGLFont->getWidth(mText.getWString().c_str(), mScrollHPos + rendered_text, select_right - mScrollHPos - rendered_text);
			width = llmin(width, mMaxHPixels - llround(rendered_pixels_right));
			gl_rect_2d(llround(rendered_pixels_right), cursor_top, llround(rendered_pixels_right)+width, cursor_bottom, color);

			rendered_text += mGLFont->render( 
				mText, mScrollHPos + rendered_text,
				rendered_pixels_right, text_bottom,
				LLColor4( 1.f - text_color.mV[0], 1.f - text_color.mV[1], 1.f - text_color.mV[2], 1 ),
				LLFontGL::LEFT, LLFontGL::BOTTOM,
				LLFontGL::NORMAL,
				select_right - mScrollHPos - rendered_text,
				mMaxHPixels - llround(rendered_pixels_right),
				&rendered_pixels_right);
		}

		if( (rendered_pixels_right < (F32)mMaxHPixels) && (rendered_text < text_len) )
		{
			// unselected, right side
			mGLFont->render( 
				mText, mScrollHPos + rendered_text,
				rendered_pixels_right, text_bottom,
				text_color,
				LLFontGL::LEFT, LLFontGL::BOTTOM,
				LLFontGL::NORMAL,
				S32_MAX,
				mMaxHPixels - llround(rendered_pixels_right),
				&rendered_pixels_right);
		}
	}
	else
	{
		mGLFont->render( 
			mText, mScrollHPos, 
			rendered_pixels_right, text_bottom,
			text_color,
			LLFontGL::LEFT, LLFontGL::BOTTOM,
			LLFontGL::NORMAL,
			S32_MAX,
			mMaxHPixels - llround(rendered_pixels_right),
			&rendered_pixels_right);
	}
#if 0 // for when we're ready for image art.
	mBorder->setVisible(FALSE); // no more programmatic art.
#endif

	drawMisspelled(background);
	resetSpellDirty();

	// If we're editing...
	if( gFocusMgr.getKeyboardFocus() == this)
	{
		//mBorder->setVisible(TRUE); // ok, programmer art just this once.
		// (Flash the cursor every half second)
		if (gShowTextEditCursor && !mReadOnly)
		{
			F32 elapsed = mKeystrokeTimer.getElapsedTimeF32();
			if( (elapsed < CURSOR_FLASH_DELAY ) || (S32(elapsed * 2) & 1) )
			{
				S32 cursor_left = findPixelNearestPos();
				cursor_left -= UI_LINEEDITOR_CURSOR_THICKNESS / 2;
				S32 cursor_right = cursor_left + UI_LINEEDITOR_CURSOR_THICKNESS;
				if (LL_KIM_OVERWRITE == gKeyboard->getInsertMode() && !hasSelection())
				{
					const LLWString space(utf8str_to_wstring(std::string(" ")));
					S32 wswidth = mGLFont->getWidth(space.c_str());
					S32 width = mGLFont->getWidth(mText.getWString().c_str(), getCursor(), 1) + 1;
					cursor_right = cursor_left + llmax(wswidth, width);
				}
				// Use same color as text for the Cursor
				gl_rect_2d(cursor_left, cursor_top,
					cursor_right, cursor_bottom, text_color);
				if (LL_KIM_OVERWRITE == gKeyboard->getInsertMode() && !hasSelection())
				{
					mGLFont->render(mText, getCursor(), (F32)(cursor_left + UI_LINEEDITOR_CURSOR_THICKNESS / 2), text_bottom, 
						LLColor4( 1.f - text_color.mV[0], 1.f - text_color.mV[1], 1.f - text_color.mV[2], 1 ),
						LLFontGL::LEFT, LLFontGL::BOTTOM,
						LLFontGL::NORMAL,
						1);
				}

				// Make sure the IME is in the right place
				S32 pixels_after_scroll = findPixelNearestPos();	// RCalculcate for IME position
				LLRect screen_pos = getScreenRect();
				LLCoordGL ime_pos( screen_pos.mLeft + pixels_after_scroll, screen_pos.mTop - UI_LINEEDITOR_V_PAD );

				ime_pos.mX = (S32) (ime_pos.mX * LLUI::sGLScaleFactor.mV[VX]);
				ime_pos.mY = (S32) (ime_pos.mY * LLUI::sGLScaleFactor.mV[VY]);
				getWindow()->setLanguageTextInput( ime_pos );
			}
		}

		//draw label if no text is provided
		//but we should draw it in a different color
		//to give indication that it is not text you typed in
		if (0 == mText.length() && mReadOnly)
		{
			mGLFont->render(mLabel.getWString(), 0,
							mMinHPixels, (F32)text_bottom,
							label_color,
							LLFontGL::LEFT,
							LLFontGL::BOTTOM,
							LLFontGL::NORMAL,
							S32_MAX,
							mMaxHPixels - llround(rendered_pixels_right),
							&rendered_pixels_right, FALSE);
		}


		// Draw children (border)
		//mBorder->setVisible(TRUE);
		mBorder->setKeyboardFocusHighlight( TRUE );
		LLView::draw();
		mBorder->setKeyboardFocusHighlight( FALSE );
		//mBorder->setVisible(FALSE);
	}
	else // does not have keyboard input
	{
		// draw label if no text provided
		if (0 == mText.length())
		{
			mGLFont->render(mLabel.getWString(), 0,
							mMinHPixels, (F32)text_bottom,
							label_color,
							LLFontGL::LEFT,
							LLFontGL::BOTTOM,
							LLFontGL::NORMAL,
							S32_MAX,
							mMaxHPixels - llround(rendered_pixels_right),
							&rendered_pixels_right, FALSE);
		}
		// Draw children (border)
		LLView::draw();
	}

	if (mDrawAsterixes)
	{
		mText = saved_text;
	}
}


// Returns the local screen space X coordinate associated with the text cursor position.
S32 LLLineEditor::findPixelNearestPos(const S32 cursor_offset) const
{
	S32 dpos = getCursor() - mScrollHPos + cursor_offset;
	S32 result = mGLFont->getWidth(mText.getWString().c_str(), mScrollHPos, dpos) + mMinHPixels;
	return result;
}

void LLLineEditor::reportBadKeystroke()
{
	make_ui_sound("UISndBadKeystroke");
}

//virtual
void LLLineEditor::clear()
{
	mText.clear();
	setCursor(0);
}

//virtual
void LLLineEditor::onTabInto()
{
	selectAll();
}

//virtual
BOOL LLLineEditor::acceptsTextInput() const
{
	return TRUE;
}

// Start or stop the editor from accepting text-editing keystrokes
void LLLineEditor::setFocus( BOOL new_state )
{
	BOOL old_state = hasFocus();

	if (!new_state)
	{
		getWindow()->allowLanguageTextInput(this, FALSE);
	}


	// getting focus when we didn't have it before, and we want to select all
	if (!old_state && new_state && mSelectAllonFocusReceived)
	{
		selectAll();
		// We don't want handleMouseUp() to "finish" the selection (and thereby
		// set mSelectionEnd to where the mouse is), so we finish the selection 
		// here.
		mIsSelecting = FALSE;
	}

	if( new_state )
	{
		gEditMenuHandler = this;

		// Don't start the cursor flashing right away
		mKeystrokeTimer.reset();
	}
	else
	{
		// Not really needed, since loss of keyboard focus should take care of this,
		// but limited paranoia is ok.
		if( gEditMenuHandler == this )
		{
			gEditMenuHandler = NULL;
		}

		endSelection();
	}

	LLUICtrl::setFocus( new_state );

	if (new_state)
	{
		// Allow Language Text Input only when this LineEditor has
		// no prevalidate function attached.  This criterion works
		// fine on 1.15.0.2, since all prevalidate func reject any
		// non-ASCII characters.  I'm not sure on future versions,
		// however.
		getWindow()->allowLanguageTextInput(this, mPrevalidateFunc == NULL);
	}
}

//virtual 
void LLLineEditor::setRect(const LLRect& rect)
{
	LLUICtrl::setRect(rect);
	if (mBorder)
	{
		LLRect border_rect = mBorder->getRect();
		// Scalable UI somehow made these rectangles off-by-one.
		// I don't know why. JC
		border_rect.setOriginAndSize(border_rect.mLeft, border_rect.mBottom, 
				rect.getWidth()-1, rect.getHeight()-1);
		mBorder->setRect(border_rect);
	}
}

void LLLineEditor::setPrevalidate(BOOL (*func)(const LLWString &))
{
	mPrevalidateFunc = func;
	updateAllowingLanguageInput();
}

// Limits what characters can be used to [1234567890.-] with [-] only valid in the first position.
// Does NOT ensure that the string is a well-formed number--that's the job of post-validation--for
// the simple reasons that intermediate states may be invalid even if the final result is valid.
// 
// static
BOOL LLLineEditor::prevalidateFloat(const LLWString &str)
{
	LLLocale locale(LLLocale::USER_LOCALE);

	BOOL success = TRUE;
	LLWString trimmed = str;
	LLWStringUtil::trim(trimmed);
	S32 len = trimmed.length();
	if( 0 < len )
	{
		// May be a comma or period, depending on the locale
		llwchar decimal_point = (llwchar)LLResMgr::getInstance()->getDecimalPoint();

		S32 i = 0;

		// First character can be a negative sign
		if( '-' == trimmed[0] )
		{
			i++;
		}

		for( ; i < len; i++ )
		{
			if( (decimal_point != trimmed[i] ) && !LLStringOps::isDigit( trimmed[i] ) )
			{
				success = FALSE;
				break;
			}
		}
	}		

	return success;
}

// static
BOOL LLLineEditor::postvalidateFloat(const std::string &str)
{
	LLLocale locale(LLLocale::USER_LOCALE);

	BOOL success = TRUE;
	BOOL has_decimal = FALSE;
	BOOL has_digit = FALSE;

	LLWString trimmed = utf8str_to_wstring(str);
	LLWStringUtil::trim(trimmed);
	S32 len = trimmed.length();
	if( 0 < len )
	{
		S32 i = 0;

		// First character can be a negative sign
		if( '-' == trimmed[0] )
		{
			i++;
		}

		// May be a comma or period, depending on the locale
		llwchar decimal_point = (llwchar)LLResMgr::getInstance()->getDecimalPoint();

		for( ; i < len; i++ )
		{
			if( decimal_point == trimmed[i] )
			{
				if( has_decimal )
				{
					// can't have two
					success = FALSE;
					break;
				}
				else
				{
					has_decimal = TRUE;
				}
			}
			else
			if( LLStringOps::isDigit( trimmed[i] ) )
			{
				has_digit = TRUE;
			}
			else
			{
				success = FALSE;
				break;
			}
		}
	}		

	// Gotta have at least one
	success = has_digit;

	return success;
}

// Limits what characters can be used to [1234567890-] with [-] only valid in the first position.
// Does NOT ensure that the string is a well-formed number--that's the job of post-validation--for
// the simple reasons that intermediate states may be invalid even if the final result is valid.
//
// static
BOOL LLLineEditor::prevalidateInt(const LLWString &str)
{
	LLLocale locale(LLLocale::USER_LOCALE);

	BOOL success = TRUE;
	LLWString trimmed = str;
	LLWStringUtil::trim(trimmed);
	S32 len = trimmed.length();
	if( 0 < len )
	{
		S32 i = 0;

		// First character can be a negative sign
		if( '-' == trimmed[0] )
		{
			i++;
		}

		for( ; i < len; i++ )
		{
			if( !LLStringOps::isDigit( trimmed[i] ) )
			{
				success = FALSE;
				break;
			}
		}
	}		

	return success;
}

// static
BOOL LLLineEditor::prevalidatePositiveS32(const LLWString &str)
{
	LLLocale locale(LLLocale::USER_LOCALE);

	LLWString trimmed = str;
	LLWStringUtil::trim(trimmed);
	S32 len = trimmed.length();
	BOOL success = TRUE;
	if(0 < len)
	{
		if(('-' == trimmed[0]) || ('0' == trimmed[0]))
		{
			success = FALSE;
		}
		S32 i = 0;
		while(success && (i < len))
		{
			if(!LLStringOps::isDigit(trimmed[i++]))
			{
				success = FALSE;
			}
		}
	}
	if (success)
	{
		S32 val = strtol(wstring_to_utf8str(trimmed).c_str(), NULL, 10);
		if (val <= 0)
		{
			success = FALSE;
		}
	}
	return success;
}

BOOL LLLineEditor::prevalidateNonNegativeS32(const LLWString &str)
{
	LLLocale locale(LLLocale::USER_LOCALE);

	LLWString trimmed = str;
	LLWStringUtil::trim(trimmed);
	S32 len = trimmed.length();
	BOOL success = TRUE;
	if(0 < len)
	{
		if('-' == trimmed[0])
		{
			success = FALSE;
		}
		S32 i = 0;
		while(success && (i < len))
		{
			if(!LLStringOps::isDigit(trimmed[i++]))
			{
				success = FALSE;
			}
		}
	}
	if (success)
	{
		S32 val = strtol(wstring_to_utf8str(trimmed).c_str(), NULL, 10);
		if (val < 0)
		{
			success = FALSE;
		}
	}
	return success;
}

BOOL LLLineEditor::prevalidateAlphaNum(const LLWString &str)
{
	LLLocale locale(LLLocale::USER_LOCALE);

	BOOL rv = TRUE;
	S32 len = str.length();
	if(len == 0) return rv;
	while(len--)
	{
		if( !LLStringOps::isAlnum((char)str[len]) )
		{
			rv = FALSE;
			break;
		}
	}
	return rv;
}

// static
BOOL LLLineEditor::prevalidateAlphaNumSpace(const LLWString &str)
{
	LLLocale locale(LLLocale::USER_LOCALE);

	BOOL rv = TRUE;
	S32 len = str.length();
	if(len == 0) return rv;
	while(len--)
	{
		if(!(LLStringOps::isAlnum((char)str[len]) || (' ' == str[len])))
		{
			rv = FALSE;
			break;
		}
	}
	return rv;
}

// static
BOOL LLLineEditor::prevalidatePrintableNotPipe(const LLWString &str)
{
	BOOL rv = TRUE;
	S32 len = str.length();
	if(len == 0) return rv;
	while(len--)
	{
		if('|' == str[len])
		{
			rv = FALSE;
			break;
		}
		if(!((' ' == str[len]) || LLStringOps::isAlnum((char)str[len]) || LLStringOps::isPunct((char)str[len])))
		{
			rv = FALSE;
			break;
		}
	}
	return rv;
}


// static
BOOL LLLineEditor::prevalidatePrintableNoSpace(const LLWString &str)
{
	BOOL rv = TRUE;
	S32 len = str.length();
	if(len == 0) return rv;
	while(len--)
	{
		if(LLStringOps::isSpace(str[len]))
		{
			rv = FALSE;
			break;
		}
		if( !(LLStringOps::isAlnum((char)str[len]) ||
		      LLStringOps::isPunct((char)str[len]) ) )
		{
			rv = FALSE;
			break;
		}
	}
	return rv;
}

// static
BOOL LLLineEditor::prevalidateASCII(const LLWString &str)
{
	BOOL rv = TRUE;
	S32 len = str.length();
	while(len--)
	{
		if (str[len] < 0x20 || str[len] > 0x7f)
		{
			rv = FALSE;
			break;
		}
	}
	return rv;
}

BOOL LLLineEditor::evaluateFloat()
{
	bool success = false;
	std::string expr = getText();
	LLStringUtil::toUpper(expr);

	// user deleted the contents, nothing to evaluate -- MC
	if (expr.empty())
	{
		return success;
	}
	else
	{
		F32 result = 0.f;
		success = LLCalc::getInstance()->evalString(expr, result);

		if (!success)
		{
			// Move the cursor to near the error on failure
			setCursor(LLCalc::getInstance()->getLastErrorPos());
			// *TODO: Translated error message indicating the type of error? Select error text?
		}
		else
		{
			// Replace the expression with the result
			std::ostringstream result_str;
			result_str << result;
			setText(result_str.str());
			selectAll();
		}

		return success;
	}
}

void LLLineEditor::onMouseCaptureLost()
{
	endSelection();
}


void LLLineEditor::setSelectAllonFocusReceived(BOOL b)
{
	mSelectAllonFocusReceived = b;
}


void LLLineEditor::setKeystrokeCallback(void (*keystroke_callback)(LLLineEditor* caller, void* user_data))
{
	mKeystrokeCallback = keystroke_callback;
}

// virtual
LLXMLNodePtr LLLineEditor::getXML(bool save_children) const
{
	LLXMLNodePtr node = LLUICtrl::getXML();

	node->setName(LL_LINE_EDITOR_TAG);

	node->createChild("max_length", TRUE)->setIntValue(mMaxLengthBytes);

	node->createChild("font", TRUE)->setStringValue(LLFontGL::nameFromFont(mGLFont));

	if (mBorder)
	{
		std::string bevel;
		switch(mBorder->getBevel())
		{
		default:
		case LLViewBorder::BEVEL_NONE:	bevel = "none"; break;
		case LLViewBorder::BEVEL_IN:	bevel = "in"; break;
		case LLViewBorder::BEVEL_OUT:	bevel = "out"; break;
		case LLViewBorder::BEVEL_BRIGHT:bevel = "bright"; break;
		}
		node->createChild("bevel_style", TRUE)->setStringValue(bevel);

		std::string style;
		switch(mBorder->getStyle())
		{
		default:
		case LLViewBorder::STYLE_LINE:		style = "line"; break;
		case LLViewBorder::STYLE_TEXTURE:	style = "texture"; break;
		}
		node->createChild("border_style", TRUE)->setStringValue(style);

		node->createChild("border_thickness", TRUE)->setIntValue(mBorder->getBorderWidth());
	}

	if (!mLabel.empty())
	{
		node->createChild("label", TRUE)->setStringValue(mLabel.getString());
	}

	node->createChild("select_all_on_focus_received", TRUE)->setBoolValue(mSelectAllonFocusReceived);

	node->createChild("handle_edit_keys_directly", TRUE)->setBoolValue(mHandleEditKeysDirectly );

	addColorXML(node, mCursorColor, "cursor_color", "TextCursorColor");
	addColorXML(node, mFgColor, "text_color", "TextFgColor");
	addColorXML(node, mReadOnlyFgColor, "text_readonly_color", "TextFgReadOnlyColor");
	addColorXML(node, mTentativeFgColor, "text_tentative_color", "TextFgTentativeColor");
	addColorXML(node, mReadOnlyBgColor, "bg_readonly_color", "TextBgReadOnlyColor");
	addColorXML(node, mWriteableBgColor, "bg_writeable_color", "TextBgWriteableColor");
	addColorXML(node, mFocusBgColor, "bg_focus_color", "TextBgFocusColor");

	node->createChild("select_on_focus", TRUE)->setBoolValue(mSelectAllonFocusReceived );

	return node;
}

// static
LLView* LLLineEditor::fromXML(LLXMLNodePtr node, LLView *parent, LLUICtrlFactory *factory)
{
	std::string name("line_editor");
	node->getAttributeString("name", name);

	LLRect rect;
	createRect(node, rect, parent, LLRect());

	S32 max_text_length = 128;
	node->getAttributeS32("max_length", max_text_length);

	LLFontGL* font = LLView::selectFont(node);

	std::string text = node->getTextContents().substr(0, max_text_length - 1);

	LLViewBorder::EBevel bevel_style = LLViewBorder::BEVEL_IN;
	LLViewBorder::getBevelFromAttribute(node, bevel_style);
	
	LLViewBorder::EStyle border_style = LLViewBorder::STYLE_LINE;
	std::string border_string;
	node->getAttributeString("border_style", border_string);
	LLStringUtil::toLower(border_string);

	if (border_string == "texture")
	{
		border_style = LLViewBorder::STYLE_TEXTURE;
	}

	S32 border_thickness = 1;
	node->getAttributeS32("border_thickness", border_thickness);

	LLUICtrlCallback commit_callback = NULL;

	LLLineEditor* line_editor = new LLLineEditor(name,
								rect, 
								text, 
								font,
								max_text_length,
								commit_callback,
								NULL,
								NULL,
								NULL,
								NULL,
								bevel_style,
								border_style,
								border_thickness);

	std::string label;
	if(node->getAttributeString("label", label))
	{
		line_editor->setLabel(label);
	}
	BOOL select_all_on_focus_received = FALSE;
	if (node->getAttributeBOOL("select_all_on_focus_received", select_all_on_focus_received))
	{
		line_editor->setSelectAllonFocusReceived(select_all_on_focus_received);
	}
	BOOL handle_edit_keys_directly = FALSE;
	if (node->getAttributeBOOL("handle_edit_keys_directly", handle_edit_keys_directly))
	{
		line_editor->setHandleEditKeysDirectly(handle_edit_keys_directly);
	}
	BOOL commit_on_focus_lost = TRUE;
	if (node->getAttributeBOOL("commit_on_focus_lost", commit_on_focus_lost))
	{
		line_editor->setCommitOnFocusLost(commit_on_focus_lost);
	}
	BOOL spell_checking = FALSE;
	if (node->getAttributeBOOL("spell_check", spell_checking))
	{
		line_editor->setSpellCheckable(spell_checking);
	}
	BOOL allow_translate = TRUE;
	if (node->getAttributeBOOL("allow_translate", allow_translate))
	{
		line_editor->setAllowTranslate(allow_translate);
	}
	
	line_editor->setColorParameters(node);
	
	if(node->hasAttribute("select_on_focus"))
	{
		BOOL selectall = FALSE;
		node->getAttributeBOOL("select_on_focus", selectall);
		line_editor->setSelectAllonFocusReceived(selectall);
	}

	std::string prevalidate;
	if(node->getAttributeString("prevalidate", prevalidate))
	{
		LLStringUtil::toLower(prevalidate);

		if ("ascii" == prevalidate)
		{
			line_editor->setPrevalidate( LLLineEditor::prevalidateASCII );
		}
		else if ("float" == prevalidate)
		{
			line_editor->setPrevalidate( LLLineEditor::prevalidateFloat );
		}
		else if ("int" == prevalidate)
		{
			line_editor->setPrevalidate( LLLineEditor::prevalidateInt );
		}
		else if ("positive_s32" == prevalidate)
		{
			line_editor->setPrevalidate( LLLineEditor::prevalidatePositiveS32 );
		}
		else if ("non_negative_s32" == prevalidate)
		{
			line_editor->setPrevalidate( LLLineEditor::prevalidateNonNegativeS32 );
		}
		else if ("alpha_num" == prevalidate)
		{
			line_editor->setPrevalidate( LLLineEditor::prevalidateAlphaNum );
		}
		else if ("alpha_num_space" == prevalidate)
		{
			line_editor->setPrevalidate( LLLineEditor::prevalidateAlphaNumSpace );
		}
		else if ("printable_not_pipe" == prevalidate)
		{
			line_editor->setPrevalidate( LLLineEditor::prevalidatePrintableNotPipe );
		}
		else if ("printable_no_space" == prevalidate)
		{
			line_editor->setPrevalidate( LLLineEditor::prevalidatePrintableNoSpace );
		}
	}
	
	line_editor->initFromXML(node, parent);
	
	return line_editor;
}

//static
void LLLineEditor::cleanupLineEditor()
{
	sImage = NULL;
}

/* static */ 
LLPointer<LLUIImage> LLLineEditor::parseImage(std::string name, LLXMLNodePtr from, LLPointer<LLUIImage> def)
{
	std::string xml_name;
	if (from->hasAttribute(name.c_str())) from->getAttributeString(name.c_str(), xml_name);
	if (xml_name == LLStringUtil::null) return def;
	LLPointer<LLUIImage> image = LLUI::getUIImage(xml_name);
	return image.isNull() ? def : image;
}

void LLLineEditor::setColorParameters(LLXMLNodePtr node)
{
	// overrides default image if supplied.
	mImage = parseImage(std::string("image"), node, mImage);

	LLColor4 color;
	if (LLUICtrlFactory::getAttributeColor(node,"cursor_color", color)) 
	{
		setCursorColor(color);
	}
	if(node->hasAttribute("text_color"))
	{
		LLUICtrlFactory::getAttributeColor(node,"text_color", color);
		setFgColor(color);
	}
	if(node->hasAttribute("text_readonly_color"))
	{
		LLUICtrlFactory::getAttributeColor(node,"text_readonly_color", color);
		setReadOnlyFgColor(color);
	}
	if (LLUICtrlFactory::getAttributeColor(node,"text_tentative_color", color))
	{
		setTentativeFgColor(color);
	}
	if(node->hasAttribute("bg_readonly_color"))
	{
		LLUICtrlFactory::getAttributeColor(node,"bg_readonly_color", color);
		setReadOnlyBgColor(color);
	}
	if(node->hasAttribute("bg_writeable_color"))
	{
		LLUICtrlFactory::getAttributeColor(node,"bg_writeable_color", color);
		setWriteableBgColor(color);
	}
}

BOOL LLLineEditor::setTextArg( const std::string& key, const LLStringExplicit& text )
{
	mText.setArg(key, text);
	return TRUE;
}

BOOL LLLineEditor::setLabelArg( const std::string& key, const LLStringExplicit& text )
{
	mLabel.setArg(key, text);
	return TRUE;
}


void LLLineEditor::updateAllowingLanguageInput()
{
	// Allow Language Text Input only when this LineEditor has
	// no prevalidate function attached (as long as other criteria
	// common to LLTextEditor).  This criterion works
	// fine on 1.15.0.2, since all prevalidate func reject any
	// non-ASCII characters.  I'm not sure on future versions,
	// however...
	if (hasFocus() && !mReadOnly && !mDrawAsterixes && mPrevalidateFunc == NULL)
	{
		getWindow()->allowLanguageTextInput(this, TRUE);
	}
	else
	{
		getWindow()->allowLanguageTextInput(this, FALSE);
	}
}

BOOL LLLineEditor::hasPreeditString() const
{
	return (mPreeditPositions.size() > 1);
}

void LLLineEditor::resetPreedit()
{
	if (hasPreeditString())
	{
		if (hasSelection())
		{
			llwarns << "Preedit and selection!" << llendl;
			deselect();
		}

		const S32 preedit_pos = mPreeditPositions.front();
		mText.erase(preedit_pos, mPreeditPositions.back() - preedit_pos);
		mText.insert(preedit_pos, mPreeditOverwrittenWString);
		setCursor(preedit_pos);
		
		mPreeditWString.clear();
		mPreeditOverwrittenWString.clear();
		mPreeditPositions.clear();

		// Don't reset key stroke timer nor invoke keystroke callback,
		// because a call to updatePreedit should be follow soon in 
		// normal course of operation, and timer and callback will be 
		// maintained there.  Doing so here made an odd sound.  (VWR-3410) 
	}
}

void LLLineEditor::updatePreedit(const LLWString &preedit_string,
		const segment_lengths_t &preedit_segment_lengths, const standouts_t &preedit_standouts, S32 caret_position)
{
	// Just in case.
	if (mReadOnly)
	{
		return;
	}

	// Note that call to updatePreedit is always preceeded by resetPreedit,
	// so we have no existing selection/preedit.

	S32 insert_preedit_at = getCursor();

	mPreeditWString = preedit_string;
	mPreeditPositions.resize(preedit_segment_lengths.size() + 1);
	S32 position = insert_preedit_at;
	for (segment_lengths_t::size_type i = 0; i < preedit_segment_lengths.size(); i++)
	{
		mPreeditPositions[i] = position;
		position += preedit_segment_lengths[i];
	}
	mPreeditPositions.back() = position;
	if (LL_KIM_OVERWRITE == gKeyboard->getInsertMode())
	{
		mPreeditOverwrittenWString.assign( LLWString( mText, insert_preedit_at, mPreeditWString.length() ) );
		mText.erase(insert_preedit_at, mPreeditWString.length());
	}
	else
	{
		mPreeditOverwrittenWString.clear();
	}
	mText.insert(insert_preedit_at, mPreeditWString);

	mPreeditStandouts = preedit_standouts;

	setCursor(position);
	setCursor(mPreeditPositions.front() + caret_position);

	// Update of the preedit should be caused by some key strokes.
	mKeystrokeTimer.reset();
	if( mKeystrokeCallback )
	{
		mKeystrokeCallback( this, mCallbackUserData );
	}
}

BOOL LLLineEditor::getPreeditLocation(S32 query_offset, LLCoordGL *coord, LLRect *bounds, LLRect *control) const
{
	if (control)
	{
		LLRect control_rect_screen;
		localRectToScreen(getRect(), &control_rect_screen);
		LLUI::screenRectToGL(control_rect_screen, control);
	}

	S32 preedit_left_column, preedit_right_column;
	if (hasPreeditString())
	{
		preedit_left_column = mPreeditPositions.front();
		preedit_right_column = mPreeditPositions.back();
	}
	else
	{
		preedit_left_column = preedit_right_column = getCursor();
	}
	if (preedit_right_column < mScrollHPos)
	{
		// This should not occure...
		return FALSE;
	}

	const S32 query = (query_offset >= 0 ? preedit_left_column + query_offset : getCursor());
	if (query < mScrollHPos || query < preedit_left_column || query > preedit_right_column)
	{
		return FALSE;
	}

	if (coord)
	{
		S32 query_local = findPixelNearestPos(query - getCursor());
		S32 query_screen_x, query_screen_y;
		localPointToScreen(query_local, getRect().getHeight() / 2, &query_screen_x, &query_screen_y);
		LLUI::screenPointToGL(query_screen_x, query_screen_y, &coord->mX, &coord->mY);
	}

	if (bounds)
	{
		S32 preedit_left_local = findPixelNearestPos(llmax(preedit_left_column, mScrollHPos) - getCursor());
		S32 preedit_right_local = llmin(findPixelNearestPos(preedit_right_column - getCursor()), getRect().getWidth() - mBorderThickness);
		if (preedit_left_local > preedit_right_local)
		{
			// Is this condition possible?
			preedit_right_local = preedit_left_local;
		}

		LLRect preedit_rect_local(preedit_left_local, getRect().getHeight(), preedit_right_local, 0);
		LLRect preedit_rect_screen;
		localRectToScreen(preedit_rect_local, &preedit_rect_screen);
		LLUI::screenRectToGL(preedit_rect_screen, bounds);
	}

	return TRUE;
}

void LLLineEditor::getPreeditRange(S32 *position, S32 *length) const
{
	if (hasPreeditString())
	{
		*position = mPreeditPositions.front();
		*length = mPreeditPositions.back() - mPreeditPositions.front();
	}
	else
	{
		*position = mCursorPos;
		*length = 0;
	}
}

void LLLineEditor::getSelectionRange(S32 *position, S32 *length) const
{
	if (hasSelection())
	{
		*position = llmin(mSelectionStart, mSelectionEnd);
		*length = llabs(mSelectionStart - mSelectionEnd);
	}
	else
	{
		*position = mCursorPos;
		*length = 0;
	}
}

void LLLineEditor::markAsPreedit(S32 position, S32 length)
{
	deselect();
	setCursor(position);
	if (hasPreeditString())
	{
		llwarns << "markAsPreedit invoked when hasPreeditString is true." << llendl;
	}
	mPreeditWString.assign( LLWString( mText.getWString(), position, length ) );
	if (length > 0)
	{
		mPreeditPositions.resize(2);
		mPreeditPositions[0] = position;
		mPreeditPositions[1] = position + length;
		mPreeditStandouts.resize(1);
		mPreeditStandouts[0] = FALSE;
	}
	else
	{
		mPreeditPositions.clear();
		mPreeditStandouts.clear();
	}
	if (LL_KIM_OVERWRITE == gKeyboard->getInsertMode())
	{
		mPreeditOverwrittenWString = mPreeditWString;
	}
	else
	{
		mPreeditOverwrittenWString.clear();
	}
}

S32 LLLineEditor::getPreeditFontSize() const
{
	return llround(mGLFont->getLineHeight() * LLUI::sGLScaleFactor.mV[VY]);
}

void LLLineEditor::setReplaceNewlinesWithSpaces(BOOL replace)
{
	mReplaceNewlinesWithSpaces = replace;
}

LLWString LLLineEditor::getConvertedText() const
{
	LLWString text = getWText();
	LLWStringUtil::trim(text);
	if (!mReplaceNewlinesWithSpaces)
	{
		LLWStringUtil::replaceChar(text,182,'\n'); // Convert paragraph symbols back into newlines.
	}
	return text;
}

static LLRegisterWidget<LLSearchEditor> r2("search_editor");


LLSearchEditor::LLSearchEditor(const std::string& name, 
		const LLRect& rect,
		S32 max_length_bytes,
		void (*search_callback)(const std::string& search_string, void* user_data),
		void* userdata)
	: 
		LLUICtrl(name, rect, TRUE, NULL, userdata),
		mSearchCallback(search_callback)
{
	LLRect search_edit_rect(0, getRect().getHeight(), getRect().getWidth(), 0);
	mSearchEdit = new LLLineEditor(std::string("search edit"),
								   search_edit_rect,
								   LLStringUtil::null,
								   NULL,
								   max_length_bytes,
								   NULL,
								   onSearchEdit,
								   NULL,
								   this);

	mSearchEdit->setFollowsAll();
	mSearchEdit->setSelectAllonFocusReceived(TRUE);

	addChild(mSearchEdit);

	S32 btn_width = rect.getHeight(); // button is square, and as tall as search editor
	LLRect clear_btn_rect(rect.getWidth() - btn_width, rect.getHeight(), rect.getWidth(), 0);
	mClearSearchButton = new LLButton(std::string("clear search"), 
								clear_btn_rect, 
								std::string("icn_clear_lineeditor.tga"),
								std::string("UIImgBtnCloseInactiveUUID"),
								LLStringUtil::null,
								onClearSearch,
								this,
								NULL,
								LLStringUtil::null);
	mClearSearchButton->setFollowsRight();
	mClearSearchButton->setFollowsTop();
	mClearSearchButton->setImageColor(LLUI::sColorsGroup->getColor("TextFgTentativeColor"));
	mClearSearchButton->setTabStop(FALSE);
	mSearchEdit->addChild(mClearSearchButton);

	mSearchEdit->setTextPadding(0, btn_width);
}


//virtual
void LLSearchEditor::setValue(const LLSD& value )
{
	mSearchEdit->setValue(value);
}

//virtual
LLSD LLSearchEditor::getValue() const
{
	return mSearchEdit->getValue();
}

//virtual
BOOL LLSearchEditor::setTextArg( const std::string& key, const LLStringExplicit& text )
{
	return mSearchEdit->setTextArg(key, text);
}

//virtual
BOOL LLSearchEditor::setLabelArg( const std::string& key, const LLStringExplicit& text )
{
	return mSearchEdit->setLabelArg(key, text);
}

//virtual
void LLSearchEditor::clear()
{
	if (mSearchEdit)
	{
		mSearchEdit->clear();
	}
}

void LLSearchEditor::draw()
{
	mClearSearchButton->setVisible(!mSearchEdit->getWText().empty());

	LLUICtrl::draw();
}


//static
void LLSearchEditor::onSearchEdit(LLLineEditor* caller, void* user_data )
{
	LLSearchEditor* search_editor = (LLSearchEditor*)user_data;
	if (search_editor->mSearchCallback)
	{
		search_editor->mSearchCallback(caller->getText(), search_editor->mCallbackUserData);
	}
}

//static
void LLSearchEditor::onClearSearch(void* user_data)
{
	LLSearchEditor* search_editor = (LLSearchEditor*)user_data;

	search_editor->setText(LLStringUtil::null);
	if (search_editor->mSearchCallback)
	{
		search_editor->mSearchCallback(LLStringUtil::null, search_editor->mCallbackUserData);
	}
}

// virtual
LLXMLNodePtr LLSearchEditor::getXML(bool save_children) const
{
	LLXMLNodePtr node = LLUICtrl::getXML();

	node->setName(LL_SEARCH_EDITOR_TAG);

	return node;
}

// static
LLView* LLSearchEditor::fromXML(LLXMLNodePtr node, LLView *parent, LLUICtrlFactory *factory)
{
	std::string name("search_editor");
	node->getAttributeString("name", name);

	LLRect rect;
	createRect(node, rect, parent, LLRect());

	S32 max_text_length = 128;
	node->getAttributeS32("max_length", max_text_length);

	std::string text = node->getValue().substr(0, max_text_length - 1);

	LLSearchEditor* search_editor = new LLSearchEditor(name,
								rect, 
								max_text_length,
								NULL, NULL);

	std::string label;
	if(node->getAttributeString("label", label))
	{
		search_editor->mSearchEdit->setLabel(label);
	}
	
	search_editor->setText(text);

	search_editor->initFromXML(node, parent);
	
	return search_editor;
}
