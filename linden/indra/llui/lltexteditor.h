/** 
 * @file lltexteditor.h
 * @brief LLTextEditor base class
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

// Text editor widget to let users enter a a multi-line ASCII document//

#ifndef LL_LLTEXTEDITOR_H
#define LL_LLTEXTEDITOR_H

#include "llrect.h"
#include "llkeywords.h"
#include "lluictrl.h"
#include "llframetimer.h"
#include "lldarray.h"
#include "llstyle.h"
#include "lleditmenuhandler.h"
#include "lldarray.h"

#include "llpreeditor.h"
#include "llmenugl.h"

class LLFontGL;
class LLScrollbar;
class LLViewBorder;
class LLKeywordToken;
class LLTextCmd;
class LLUICtrlFactory;

class LLTextEditor : public LLUICtrl, LLEditMenuHandler, protected LLPreeditor
{
public:
	//
	// Constants
	//
	static const llwchar FIRST_EMBEDDED_CHAR = 0x100000;
	static const llwchar LAST_EMBEDDED_CHAR =  0x10ffff;
	static const S32 MAX_EMBEDDED_ITEMS = LAST_EMBEDDED_CHAR - FIRST_EMBEDDED_CHAR + 1;

	LLTextEditor(const std::string& name,
				 const LLRect& rect,
				 S32 max_length,
				 const std::string &default_text, 
				 const LLFontGL* glfont = NULL,
				 BOOL allow_embedded_items = FALSE);

	virtual ~LLTextEditor();

	virtual LLXMLNodePtr getXML(bool save_children = true) const;
	static LLView* fromXML(LLXMLNodePtr node, LLView *parent, class LLUICtrlFactory *factory);
	void    setTextEditorParameters(LLXMLNodePtr node);
	void	setParseHTML(BOOL parsing) {mParseHTML=parsing;}
	void	setParseHighlights(BOOL parsing) {mParseHighlights=parsing;}

	// mousehandler overrides
	virtual BOOL	handleMouseDown(S32 x, S32 y, MASK mask);
	virtual BOOL	handleMouseUp(S32 x, S32 y, MASK mask);
	virtual BOOL	handleHover(S32 x, S32 y, MASK mask);
	virtual BOOL	handleScrollWheel(S32 x, S32 y, S32 clicks);
	virtual BOOL	handleDoubleClick(S32 x, S32 y, MASK mask );
	virtual BOOL	handleRightMouseDown( S32 x, S32 y, MASK mask );
	virtual BOOL	handleMiddleMouseDown(S32 x,S32 y,MASK mask);

	virtual BOOL	handleKeyHere(KEY key, MASK mask );
	virtual BOOL	handleUnicodeCharHere(llwchar uni_char);

	virtual BOOL	handleToolTip(S32 x, S32 y, std::string& msg, LLRect* sticky_rect);
	virtual BOOL	handleDragAndDrop(S32 x, S32 y, MASK mask, BOOL drop,
									  EDragAndDropType cargo_type, void *cargo_data,
									  EAcceptance *accept, std::string& tooltip_msg);
	virtual void	onMouseCaptureLost();

	// view overrides
	virtual void	reshape(S32 width, S32 height, BOOL called_from_parent = TRUE);
	virtual void	draw();
	virtual void	onFocusReceived();
	virtual void	onFocusLost();
	virtual void	setEnabled(BOOL enabled);

	// uictrl overrides
	virtual void	onTabInto();
	virtual void	clear();
	virtual void	setFocus( BOOL b );
	virtual BOOL	acceptsTextInput() const;
	virtual BOOL	isDirty() const { return( mLastCmd != NULL || (mPristineCmd && (mPristineCmd != mLastCmd)) ); }
	BOOL	isSpellDirty() const { return mWText != mPrevSpelledText; }	// Returns TRUE if user changed value at all
	void	resetSpellDirty() { mPrevSpelledText = mWText; }		// Clear dirty state

	struct SpellMenuBind
	{
		LLTextEditor* origin;
		LLMenuItemCallGL * menuItem;
		std::string word;
		S32 wordPositionStart;
		S32 wordPositionEnd;
		S32 wordY;
	};
	
	// LLEditMenuHandler interface
	virtual void	undo();
	virtual BOOL	canUndo() const;
	virtual void	redo();
	virtual BOOL	canRedo() const;
	virtual void	cut();
	virtual BOOL	canCut() const;
	virtual void	copy();
	virtual BOOL	canCopy() const;
	virtual void	paste();
	virtual BOOL	canPaste() const;
 
	virtual void	spellReplace(SpellMenuBind* spellData);
	virtual void	translationReplace(const std::string &translation, const S32 orig_start, const S32 orig_length);
	virtual BOOL	canTranslate() const;
 
	virtual void	updatePrimary();
	virtual void	copyPrimary();
	virtual void	pastePrimary();
	virtual BOOL	canPastePrimary() const;

	virtual void	doDelete();
	virtual BOOL	canDoDelete() const;
	virtual void	selectAll();
	virtual BOOL	canSelectAll()	const;
	virtual void	deselect();
	virtual BOOL	canDeselect() const;

	static BOOL context_enable_cut(void* data);
	static void context_cut(void* data);
	static BOOL context_enable_copy(void* data);
	static void context_copy(void* data);
	static BOOL context_enable_paste(void* data);
	static void context_paste(void* data);
	static BOOL context_enable_delete(void* data);
	static void context_delete(void* data);
	static BOOL context_enable_selectall(void* data);
	static void context_selectall(void* data);
	static BOOL context_enable_translate(void * data);
	static void context_translate(void * data);
	static void spell_correct(void* data);
	static void spell_add(void* data);
	static void spell_show(void* data);
	std::vector<S32> getMisspelledWordsPositions();
  void defineMenuCallbacks(LLMenuGL* menu);

	void			selectNext(const std::string& search_text_in, BOOL case_insensitive, BOOL wrap = TRUE);
	BOOL			replaceText(const std::string& search_text, const std::string& replace_text, BOOL case_insensitive, BOOL wrap = TRUE);
	void			replaceTextAll(const std::string& search_text, const std::string& replace_text, BOOL case_insensitive);
	
	// Undo/redo stack
	void			blockUndo();

	// Text editing
	virtual void	makePristine();
	BOOL			isPristine() const;
	BOOL			allowsEmbeddedItems() const { return mAllowEmbeddedItems; }

	// inserts text at cursor
	void			insertText(const std::string &text, BOOL deleteSelection = TRUE);
	// appends text at end
	void 			appendText(const std::string &wtext, bool allow_undo, bool prepend_newline,
							   const LLStyleSP stylep = NULL);

	void 			appendColoredText(const std::string &wtext, bool allow_undo, 
									  bool prepend_newline,
									  const LLColor4 &color,
									  const std::string& font_name = LLStringUtil::null);
	// if styled text starts a line, you need to prepend a newline.
	void 			appendStyledText(const std::string &new_text, bool allow_undo, 
									 bool prepend_newline,
									 LLStyleSP stylep = NULL);
	void			appendHighlightedText(const std::string &new_text,  bool allow_undo, 
										  bool prepend_newline,	 S32  highlight_part,
										  LLStyleSP stylep);
	
	// Removes text from the end of document
	// Does not change highlight or cursor position.
	void 			removeTextFromEnd(S32 num_chars);

	BOOL			tryToRevertToPristineState();

	void			setCursor(S32 row, S32 column);
	void			setCursorPos(S32 offset);
	void			setCursorAndScrollToEnd();
	void            scrollToPos(S32 pos);

	void			getLineAndColumnForPosition( S32 position,  S32* line, S32* col, BOOL include_wordwrap );
	void			getCurrentLineAndColumn( S32* line, S32* col, BOOL include_wordwrap );
	S32				getLineForPosition(S32 position);
	S32				getCurrentLine();

	void			loadKeywords(const std::string& filename,
								 const std::vector<std::string>& funcs,
								 const std::vector<std::string>& tooltips,
								 const LLColor3& func_color);
	void addToken(LLKeywordToken::TOKEN_TYPE type,
					const std::string& key,
					const LLColor3& color,
					const std::string& tool_tip = LLStringUtil::null,
					const std::string& delimiter = LLStringUtil::null);
	LLKeywords::keyword_iterator_t keywordsBegin()	{ return mKeywords.begin(); }
	LLKeywords::keyword_iterator_t keywordsEnd()	{ return mKeywords.end(); }

	// Color support
	void 			setCursorColor(const LLColor4& c)			{ mCursorColor = c; }
	void 			setFgColor( const LLColor4& c )				{ mFgColor = c; }
	void			setTextDefaultColor( const LLColor4& c )				{ mDefaultColor = c; }
	void 			setReadOnlyFgColor( const LLColor4& c )		{ mReadOnlyFgColor = c; }
	void 			setWriteableBgColor( const LLColor4& c )	{ mWriteableBgColor = c; }
	void 			setReadOnlyBgColor( const LLColor4& c )		{ mReadOnlyBgColor = c; }
	void			setTrackColor( const LLColor4& color );
	void			setThumbColor( const LLColor4& color );
	void			setHighlightColor( const LLColor4& color );
	void			setShadowColor( const LLColor4& color );
	void			setSpellCheckable(BOOL b)					{ mSpellCheckable = b; }

	// Hacky methods to make it into a word-wrapping, potentially scrolling,
	// read-only text box.
	void			setBorderVisible(BOOL b);
	BOOL			isBorderVisible() const;
	void			setTakesNonScrollClicks(BOOL b) { mTakesNonScrollClicks = b; }
	void			setHideScrollbarForShortDocs(BOOL b);

	void			setWordWrap( BOOL b );
	void			setTabsToNextField(BOOL b)				{ mTabsToNextField = b; }
	BOOL			tabsToNextField() const					{ return mTabsToNextField; }
	void			setCommitOnFocusLost(BOOL b)			{ mCommitOnFocusLost = b; }

	// Hack to handle Notecards
	virtual BOOL	importBuffer(const char* buffer, S32 length );
	virtual BOOL	exportBuffer(std::string& buffer );

	// If takes focus, will take keyboard focus on click.
	void			setTakesFocus(BOOL b)					{ mTakesFocus = b; }

	void			setSourceID(const LLUUID& id) 			{ mSourceID = id; }
	const LLUUID&	getSourceID() const						{ return mSourceID; }
	void 			setAcceptCallingCardNames(BOOL enable)	{ mAcceptCallingCardNames = enable; }
	BOOL			acceptsCallingCardNames() const			{ return mAcceptCallingCardNames; }

	void			setHandleEditKeysDirectly( BOOL b ) 	{ mHandleEditKeysDirectly = b; }

	// Callbacks
	static void		setLinkColor(LLColor4 color) { mLinkColor = color; }
	static void		setURLCallbacks(void (*callback1) (const std::string& url), 
									bool (*callback2) (const std::string& url),      
									bool (*callback3) (const std::string& url)	) 
									{ mURLcallback = callback1; mSecondlifeURLcallback = callback2; mSecondlifeURLcallbackRightClick = callback3;}

	void			setOnScrollEndCallback(void (*callback)(void*), void* userdata);

	// new methods
	void 			setValue(const LLSD& value);
	LLSD 			getValue() const;

 	const std::string&	getText() const;
	
	// Non-undoable
	void			setText(const LLStringExplicit &utf8str);
	void			setWText(const LLWString &wtext);
	
	// Returns byte length limit
	S32				getMaxLength() const 			{ return mMaxTextByteLength; }

	// Change cursor
	void			startOfLine();
	void			endOfLine();
	void			endOfDoc();

	BOOL			isScrolledToTop();
	BOOL			isScrolledToBottom();

	// Getters
	const LLWString& getWText() const { return mWText; }
	llwchar			getWChar(S32 pos) const { return mWText[pos]; }
	LLWString		getWSubString(S32 pos, S32 len) const { return mWText.substr(pos, len); }
	
	const LLTextSegment*	getCurrentSegment() const { return getSegmentAtOffset(mCursorPos); }
	const LLTextSegment*	getPreviousSegment() const;
	void getSelectedSegments(std::vector<const LLTextSegment*>& segments) const;

	static bool		isPartOfWord(llwchar c) { return ( (c == '_')  || (c == '\'') || LLStringOps::isAlnum((char)c)); }

	BOOL isReadOnly() { return mReadOnly; }
protected:
	//
	// Methods
	//

	LLHandle<LLView>					mPopupMenuHandle;

	S32				getLength() const { return mWText.length(); }
	void			getSegmentAndOffset( S32 startpos, S32* segidxp, S32* offsetp ) const;
	void			drawPreeditMarker();
public:
	void			updateLineStartList(S32 startpos = 0);
protected:
	void			updateScrollFromCursor();
	void			updateTextRect();
	const LLRect&	getTextRect() const { return mTextRect; }

	void 			assignEmbedded(const std::string &s);
	BOOL 			truncate();				// Returns true if truncation occurs
	
	void			removeCharOrTab();
	void			setCursorAtLocalPos(S32 x, S32 y, BOOL round);
	S32				getCursorPosFromLocalCoord( S32 local_x, S32 local_y, BOOL round ) const;

	void			indentSelectedLines( S32 spaces );
	S32				indentLine( S32 pos, S32 spaces );
	void			unindentLineBeforeCloseBrace();

	S32				getSegmentIdxAtOffset(S32 offset) const;
	const LLTextSegment*	getSegmentAtLocalPos(S32 x, S32 y) const;
	const LLTextSegment*	getSegmentAtOffset(S32 offset) const;

	void			reportBadKeystroke() { make_ui_sound("UISndBadKeystroke"); }

	BOOL			handleNavigationKey(const KEY key, const MASK mask);
	BOOL			handleSpecialKey(const KEY key, const MASK mask, BOOL* return_key_hit);
	BOOL			handleSelectionKey(const KEY key, const MASK mask);
	BOOL			handleControlKey(const KEY key, const MASK mask);
	BOOL			handleEditKey(const KEY key, const MASK mask);
	// <edit>
public:
	// </edit>
	BOOL			hasSelection() const		{ return (mSelectionStart !=mSelectionEnd); }
	// <edit>
	protected:
	// </edit>
	BOOL			selectionContainsLineBreaks();
	void			startSelection();
	void			endSelection();
	void			deleteSelection(BOOL transient_operation);

	S32				prevWordPos(S32 cursorPos) const;
	S32				nextWordPos(S32 cursorPos) const;
	BOOL			getWordBoundriesAt(const S32 at, S32* word_begin, S32* word_length) const;

	S32 			getLineCount() const { return mLineStartList.size(); }
	S32 			getLineStart( S32 line ) const;
	void			getLineAndOffset(S32 pos, S32* linep, S32* offsetp) const;
	S32				getPos(S32 line, S32 offset);

	void			changePage(S32 delta);
	void			changeLine(S32 delta);

	void			autoIndent();
	
	void			findEmbeddedItemSegments();
	
	virtual BOOL	handleMouseUpOverSegment(S32 x, S32 y, MASK mask);

	virtual llwchar	pasteEmbeddedItem(llwchar ext_char) { return ext_char; }
	virtual void	bindEmbeddedChars(const LLFontGL* font) const {}
	virtual void	unbindEmbeddedChars(const LLFontGL* font) const {}
	
	S32				findHTMLToken(const std::string &line, S32 pos, BOOL reverse) const;
	BOOL			findHTML(const std::string &line, S32 *begin, S32 *end, std::string& url) const;

	// Abstract inner base class representing an undoable editor command.
	// Concrete sub-classes can be defined for operations such as insert, remove, etc.
	// Used as arguments to the execute() method below.
	class LLTextCmd
	{
	public:
		LLTextCmd( S32 pos, BOOL group_with_next ) : mPos(pos), mGroupWithNext(group_with_next) {}
		virtual			~LLTextCmd() {}
		virtual BOOL	execute(LLTextEditor* editor, S32* delta) = 0;
		virtual S32		undo(LLTextEditor* editor) = 0;
		virtual S32		redo(LLTextEditor* editor) = 0;
		virtual BOOL	canExtend(S32 pos) const { return FALSE; }
		virtual void	blockExtensions() {}
		virtual BOOL	extendAndExecute( LLTextEditor* editor, S32 pos, llwchar c, S32* delta ) { llassert(0); return 0; }
		virtual BOOL	hasExtCharValue( llwchar value ) const { return FALSE; }

		// Defined here so they can access protected LLTextEditor editing methods
		S32				insert(LLTextEditor* editor, S32 pos, const LLWString &wstr) { return editor->insertStringNoUndo( pos, wstr ); }
		S32 			remove(LLTextEditor* editor, S32 pos, S32 length) { return editor->removeStringNoUndo( pos, length ); }
		S32				overwrite(LLTextEditor* editor, S32 pos, llwchar wc) { return editor->overwriteCharNoUndo(pos, wc); }
		
		S32				getPosition() const { return mPos; }
		BOOL			groupWithNext() const { return mGroupWithNext; }
		
	private:
		const S32		mPos;
		BOOL			mGroupWithNext;
	};
	// Here's the method that takes and applies text commands.
	S32 			execute(LLTextCmd* cmd);

	// Undoable operations
	void			addChar(llwchar c); // at mCursorPos
	S32				addChar(S32 pos, llwchar wc);
	S32				overwriteChar(S32 pos, llwchar wc);
	void			removeChar();
	S32 			removeChar(S32 pos);
	S32				insert(const S32 pos, const LLWString &wstr, const BOOL group_with_next_op);
	S32				remove(const S32 pos, const S32 length, const BOOL group_with_next_op);
	S32				append(const LLWString &wstr, const BOOL group_with_next_op);
	
	// Direct operations
	S32				insertStringNoUndo(S32 pos, const LLWString &wstr); // returns num of chars actually inserted
	S32 			removeStringNoUndo(S32 pos, S32 length);
	S32				overwriteCharNoUndo(S32 pos, llwchar wc);

	void			resetKeystrokeTimer() { mKeystrokeTimer.reset(); }

	void			updateAllowingLanguageInput();
	BOOL			hasPreeditString() const;

	// Overrides LLPreeditor
	virtual void	resetPreedit();
	virtual void	updatePreedit(const LLWString &preedit_string,
						const segment_lengths_t &preedit_segment_lengths, const standouts_t &preedit_standouts, S32 caret_position);
	virtual void	markAsPreedit(S32 position, S32 length);
	virtual void	getPreeditRange(S32 *position, S32 *length) const;
	virtual void	getSelectionRange(S32 *position, S32 *length) const;
	virtual BOOL	getPreeditLocation(S32 query_offset, LLCoordGL *coord, LLRect *bounds, LLRect *control) const;
	virtual S32		getPreeditFontSize() const;
	//
	// Protected data
	//
	// Probably deserves serious thought to hiding as many of these 
	// as possible behind protected accessor methods.
	//

	// I-beam is just after the mCursorPos-th character.
public:
	S32				mCursorPos;
protected:
	// Use these to determine if a click on an embedded item is a drag or not.
	S32				mMouseDownX;
	S32				mMouseDownY;
	
	// Are we in the middle of a drag-select?  To figure out if there is a current
	// selection, call hasSelection().
	BOOL			mIsSelecting;
	S32				mSelectionStart;
	S32				mSelectionEnd;
	S32				mLastSelectionX;
	S32				mLastSelectionY;

	BOOL			mParseHTML;
	BOOL			mParseHighlights;
	std::string		mHTML;

	typedef std::vector<LLTextSegment *> segment_list_t;
	segment_list_t mSegments;
	const LLTextSegment*	mHoverSegment;
	
	// Scrollbar data
	class LLScrollbar*	mScrollbar;
	BOOL			mHideScrollbarForShortDocs;
	BOOL			mTakesNonScrollClicks;
	void			(*mOnScrollEndCallback)(void*);
	void			*mOnScrollEndData;

	LLWString			mPreeditWString;
	LLWString			mPreeditOverwrittenWString;
	std::vector<S32> 	mPreeditPositions;
	std::vector<BOOL> 	mPreeditStandouts;
	
private:

	//
	// Methods
	//
	void	                pasteHelper(bool is_primary);

	void			updateSegments();
	void			pruneSegments();

	void			drawBackground();
	void			drawSelectionBackground();
	void			drawCursor();
	void			autoCorrectText();
	void			drawMisspelled();
	void			drawText();
	void			drawClippedSegment(const LLWString &wtext, S32 seg_start, S32 seg_end, F32 x, F32 y, S32 selection_left, S32 selection_right, const LLStyleSP& color, F32* right_x);

	void			needsReflow() 
	{ 
		mReflowNeeded = TRUE; 
		// cursor might have moved, need to scroll
		mScrollNeeded = TRUE;
	}
	void			needsScroll() { mScrollNeeded = TRUE; }

	//
	// Data
	//
	LLKeywords		mKeywords;
	static LLColor4 mLinkColor;
	static void			(*mURLcallback) (const std::string& url);
	static bool			(*mSecondlifeURLcallback) (const std::string& url);
	static bool			(*mSecondlifeURLcallbackRightClick) (const std::string& url);

	// Concrete LLTextCmd sub-classes used by the LLTextEditor base class
	class LLTextCmdInsert;
	class LLTextCmdAddChar;
	class LLTextCmdOverwriteChar;
	class LLTextCmdRemove;

	LLWString		mWText;
	mutable std::string mUTF8Text;
	mutable BOOL	mTextIsUpToDate;
	
	LLWString		mPrevSpelledText;		// saved string so we know whether to respell or not
	S32 spellStart;
	S32 spellEnd;
	std::vector<S32> misspellLocations;     // where all the mispelled words are
	BOOL			mSpellCheckable;			// set in xui as "spell_check". Default value for a field
	BOOL			mAllowTranslate;		// set in xui as "allow_translate".
	
	S32				mMaxTextByteLength;		// Maximum length mText is allowed to be in bytes

	const LLFontGL*	mGLFont;

	class LLViewBorder*	mBorder;

	BOOL			mBaseDocIsPristine;
	LLTextCmd*		mPristineCmd;

	LLTextCmd*		mLastCmd;

	typedef std::deque<LLTextCmd*> undo_stack_t;
	undo_stack_t	mUndoStack;

	S32				mDesiredXPixel;			// X pixel position where the user wants the cursor to be
	LLRect			mTextRect;				// The rect in which text is drawn.  Excludes borders.
	// List of offsets and segment index of the start of each line.  Always has at least one node (0).
	struct line_info
	{
		line_info(S32 segment, S32 offset) : mSegment(segment), mOffset(offset) {}
		S32 mSegment;
		S32 mOffset;
	};
	struct line_info_compare
	{
		bool operator()(const line_info& a, const line_info& b) const
		{
			if (a.mSegment < b.mSegment)
				return true;
			else if (a.mSegment > b.mSegment)
				return false;
			else
				return a.mOffset < b.mOffset;
		}
	};
	typedef std::vector<line_info> line_list_t;

	//to keep track of what we have to remove before showing menu
	std::vector<SpellMenuBind* > suggestionMenuItems;
	S32 mLastContextMenuX;
	S32 mLastContextMenuY;

	line_list_t mLineStartList;
	BOOL			mReflowNeeded;
	BOOL			mScrollNeeded;

	LLFrameTimer	mKeystrokeTimer;
	LLFrameTimer	mSpellTimer;

	LLColor4		mCursorColor;

	LLColor4		mFgColor;
	LLColor4		mDefaultColor;
	LLColor4		mReadOnlyFgColor;
	LLColor4		mWriteableBgColor;
	LLColor4		mReadOnlyBgColor;
	LLColor4		mFocusBgColor;

	BOOL			mReadOnly;
	BOOL			mWordWrap;
	BOOL			mShowLineNumbers;

	BOOL			mTabsToNextField;		// if true, tab moves focus to next field, else inserts spaces
	BOOL			mCommitOnFocusLost;
	BOOL			mTakesFocus;
	BOOL			mTrackBottom;			// if true, keeps scroll position at bottom during resize
	BOOL			mScrolledToBottom;

	BOOL			mAllowEmbeddedItems;

	BOOL 			mAcceptCallingCardNames;

	LLUUID			mSourceID;

	// If true, the standard edit keys (Ctrl-X, Delete, etc,) are handled here 
	//instead of routed by the menu system
	BOOL			mHandleEditKeysDirectly;  

	LLCoordGL		mLastIMEPosition;		// Last position of the IME editor
}; // end class LLTextEditor



class LLTextSegment
{
public:
	// for creating a compare value
	LLTextSegment(S32 start);
	LLTextSegment( const LLStyleSP& style, S32 start, S32 end );
	LLTextSegment( const LLColor4& color, S32 start, S32 end, BOOL is_visible);
	LLTextSegment( const LLColor4& color, S32 start, S32 end );
	LLTextSegment( const LLColor3& color, S32 start, S32 end );

	S32					getStart() const					{ return mStart; }
	S32					getEnd() const						{ return mEnd; }
	void				setEnd( S32 end )					{ mEnd = end; }
	const LLColor4&		getColor() const					{ return mStyle->getColor(); }
	void 				setColor(const LLColor4 &color)		{ mStyle->setColor(color); }
	const LLStyleSP&	getStyle() const					{ return mStyle; }
	void 				setStyle(const LLStyleSP &style)	{ mStyle = style; }
	void 				setIsDefault(BOOL b)   				{ mIsDefault = b; }
	BOOL 				getIsDefault() const   				{ return mIsDefault; }
	void				setToken( LLKeywordToken* token )	{ mToken = token; }
	LLKeywordToken*		getToken() const					{ return mToken; }
	BOOL				getToolTip( std::string& msg ) const;

	void				dump() const;

	struct compare
	{
		bool operator()(const LLTextSegment* a, const LLTextSegment* b) const
		{
			return a->mStart < b->mStart;
		}
	};
	
private:
	LLStyleSP	mStyle;
	S32			mStart;
	S32			mEnd;
	LLKeywordToken* mToken;
	BOOL		mIsDefault;
};


#endif  // LL_TEXTEDITOR_
