/** 
 * @file llmenugl.h
 * @brief Declaration of the opengl based menu system.
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

#ifndef LL_LLMENUGL_H
#define LL_LLMENUGL_H

#include <list>

#include "llstring.h"
#include "v4color.h"
#include "llframetimer.h"
#include "llevent.h"

#include "llkeyboard.h"
#include "llfloater.h"
#include "lluistring.h"
#include "llview.h"


extern S32 MENU_BAR_HEIGHT;
extern S32 MENU_BAR_WIDTH;

// These callbacks are used by the LLMenuItemCallGL and LLMenuItemCheckGL
// classes during their work.
typedef void (*menu_callback)(void*);

// These callbacks are used by the LLMenuItemCallGL 
// classes during their work.
typedef void (*on_disabled_callback)(void*);

// This callback is used by the LLMenuItemCallGL and LLMenuItemCheckGL
// to determine if the current menu is enabled.
typedef BOOL (*enabled_callback)(void*);

// This callback is used by LLMenuItemCheckGL to determine it's
// 'checked' state.
typedef BOOL (*check_callback)(void*);

// This callback is potentially used by LLMenuItemCallGL. If provided,
// this function is called whenever it's time to determine the label's
// contents. Put the contents of the label in the provided parameter.
typedef void (*label_callback)(std::string&,void*);



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Class LLCallbackInformation
//
// The LLCallbackInformation class is used to keep track of callback 
// information for menus that might be requested at a future time.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class LLCallbackInformation 
{

public : 

  // Constants used for keeping track of what type of callback is required.
  static const U8 LL_MENU_ITEM_CALL_GL_NONE      = 0;
  static const U8 LL_MENU_ITEM_CALL_GL_ON_CLICK  = 1;
  static const U8 LL_MENU_ITEM_CALL_GL_ON_ENABLE = 2;
  static const U8 LL_MENU_ITEM_CALL_GL_TRANSLATE = 3;


  LLCallbackInformation(U8 theType=LLCallbackInformation::LL_MENU_ITEM_CALL_GL_NONE,
                       const std::string& theName = LLStringUtil::null,
                       const std::string& userData = LLStringUtil::null); 
  ~LLCallbackInformation() {};

  void setTypeOfCallback(U8 theType) {typeOfCallback = theType; }
  void setCallbackName(const std::string& name) {callbackName = name; }
  void setCallbackUserData(const std::string& userdata) {callbackUserData = userdata; }

  U8                 getTypeOfCallback() {return(typeOfCallback); }
  const std::string& getCallbackName() {return(callbackName); }
  const std::string& getCallbackUserData() {return(callbackUserData); }

  BOOL isTypeMatch(U8 type) { return(type == typeOfCallback);}
  BOOL isNameMatch(const std::string& name) { return(name == callbackName);}

protected:

private:

  U8 typeOfCallback;
  std::string callbackName;
  std::string callbackUserData;

};



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Class LLMenuItemGL
//
// The LLMenuItemGL represents a single menu item in a menu. 
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class LLMenuItemGL : public LLView
{
public:
	// static functions to control the global color scheme.
	static void setEnabledColor( const LLColor4& color ) { sEnabledColor = color; }
	static const LLColor4& getEnabledColor() { return sEnabledColor; }
	static void setDisabledColor( const LLColor4& color ) { sDisabledColor = color; }
	static const LLColor4& getDisabledColor() { return sDisabledColor; }
	static void setHighlightBGColor( const LLColor4& color ) { sHighlightBackground = color; }
	static const LLColor4& getHighlightBGColor() { return sHighlightBackground; }
	static void setHighlightFGColor( const LLColor4& color ) { sHighlightForeground = color; }
	static const LLColor4& getHighlightFGColor() { return sHighlightForeground; }

	LLMenuItemGL( const std::string& name, const std::string& label, KEY key = KEY_NONE, MASK = MASK_NONE );
        virtual ~LLMenuItemGL();

	virtual void setValue(const LLSD& value) { setLabel(value.asString()); }

	virtual LLXMLNodePtr getXML(bool save_children = true) const;

	virtual std::string getType() const	{ return "item"; }

	virtual BOOL handleHover(S32 x, S32 y, MASK mask);

	virtual BOOL handleAcceleratorKey(KEY key, MASK mask);

	void setJumpKey(KEY key);
	KEY getJumpKey() const { return mJumpKey; }
	
	// set the font used by this item.
	void setFont(const LLFontGL* font) { mFont = font; }
	const LLFontGL* getFont() const { return mFont; }
	void setFontStyle(U8 style) { mStyle = style; }
	U8 getFontStyle() const { return mStyle; }

	// returns the height in pixels for the current font.
	virtual U32 getNominalHeight( void ) const;
	
	// Marks item as not needing space for check marks or accelerator keys
	virtual void setBriefItem(BOOL brief) { mBriefItem = brief; }
	virtual BOOL isBriefItem() const { return mBriefItem; }

	virtual BOOL addToAcceleratorList(std::list<LLKeyBinding*> *listp);
	void setAllowKeyRepeat(BOOL allow) { mAllowKeyRepeat = allow; }
	BOOL getAllowKeyRepeat() const { return mAllowKeyRepeat; }

	// change the label
	void setLabel( const LLStringExplicit& label ) { mLabel = label; }	
	std::string getLabel( void ) const { return mLabel.getString(); }
	virtual BOOL setLabelArg( const std::string& key, const LLStringExplicit& text );

	// Get the parent menu for this item
	virtual class LLMenuGL*	getMenu();

	// returns the normal width of this control in pixels - this is
	// used for calculating the widest item, as well as for horizontal
	// arrangement.
	virtual U32 getNominalWidth( void ) const;

	// buildDrawLabel() - constructs the string used during the draw()
	// function. This reduces the overall string manipulation, but can
	// lead to visual errors if the state of the object changes
	// without the knowledge of the menu item. For example, if a
	// boolean being watched is changed outside of the menu item's
	// doIt() function, the draw buffer will not be updated and will
	// reflect the wrong value. If this ever becomes an issue, there
	// are ways to fix this.
	// Returns the enabled state of the item.
	virtual void buildDrawLabel( void );

	// for branching menu items, bring sub menus up to root level of menu hierarchy
	virtual void updateBranchParent( LLView* parentp ){};
	
	// doIt() - do the primary funcationality of the menu item.
	virtual void doIt( void );

	virtual void setHighlight( BOOL highlight );
	virtual BOOL getHighlight() const { return mHighlight; }

	// determine if this represents an active sub-menu
	virtual BOOL isActive( void ) const { return FALSE; }

	// determine if this represents an open sub-menu
	virtual BOOL isOpen( void ) const { return FALSE; }

	virtual void setEnabledSubMenus(BOOL enable){};

	// LLView Functionality
	virtual BOOL handleKeyHere( KEY key, MASK mask );
	virtual BOOL handleMouseDown( S32 x, S32 y, MASK mask );
	virtual BOOL handleMouseUp( S32 x, S32 y, MASK mask );
	virtual void draw( void );

	BOOL getHover() const { return mGotHover; }

	void setDrawTextDisabled(BOOL disabled) { mDrawTextDisabled = disabled; }
	BOOL getDrawTextDisabled() const { return mDrawTextDisabled; }

        // functionality for adding callbacks after the menu is constucted
        void addCallbackType(U8 theType,const std::string& name,const std::string& userdata);

        virtual BOOL setCtrlResponse(U8 llMenuItemCallType,
                                    const std::string& name,
                                    void* user_data,
                                    void *callback_fcn) = 0;


protected:
	void setHover(BOOL hover) { mGotHover = hover; }

	// This function appends the character string representation of
	// the current accelerator key and mask to the provided string.
	void appendAcceleratorString( std::string& st ) const;

	KEY mAcceleratorKey;
	MASK mAcceleratorMask;
	// mLabel contains the actual label specified by the user.
	LLUIString mLabel;

	// The draw labels contain some of the labels that we draw during
	// the draw() routine. This optimizes away some of the string
	// manipulation.
	LLUIString mDrawBoolLabel;
	LLUIString mDrawAccelLabel;
	LLUIString mDrawBranchLabel;

	BOOL mHighlight;

        // variables used for tracking callback types that will be
        // requested after the widget is in place.
        std::vector<LLCallbackInformation*> mFutureCallbackRequests;

private:
	static LLColor4 sEnabledColor;
	static LLColor4 sDisabledColor;
	static LLColor4 sHighlightBackground;
	static LLColor4 sHighlightForeground;

	// Keyboard and mouse variables
	BOOL mAllowKeyRepeat;
	BOOL mGotHover;

	// If true, suppress normal space for check marks on the left and accelerator
	// keys on the right.
	BOOL mBriefItem;

	// Font for this item
	const LLFontGL* mFont;
	U8	mStyle;
	BOOL mDrawTextDisabled;

	KEY mJumpKey;
};


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Class LLMenuItemCallGL
//
// The LLMenuItemCallerGL represents a single menu item in a menu that
// calls a user defined callback.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class LLMenuItemCallGL : public LLMenuItemGL, public LLObservable
{
public:
	// normal constructor
	LLMenuItemCallGL( const std::string& name,
 					  menu_callback clicked_cb, 
					  enabled_callback enabled_cb = NULL,
					  void* user_data = NULL, 
					  KEY key = KEY_NONE, MASK mask = MASK_NONE,
					  BOOL enabled = TRUE,
					  on_disabled_callback on_disabled_cb = NULL);
	LLMenuItemCallGL( const std::string& name,
					  const std::string& label,
 					  menu_callback clicked_cb, 
					  enabled_callback enabled_cb = NULL,
					  void* user_data = NULL, 
					  KEY key = KEY_NONE, MASK mask = MASK_NONE,
					  BOOL enabled = TRUE,
					  on_disabled_callback on_disabled_cb = NULL);

	// constructor for when you want to trap the arrange method.
	LLMenuItemCallGL( const std::string& name,
					  const std::string& label,
					  menu_callback clicked_cb,
					  enabled_callback enabled_cb,
					  label_callback label_cb,
					  void* user_data,
					  KEY key = KEY_NONE, MASK mask = MASK_NONE,
					  BOOL enabled = TRUE,
					  on_disabled_callback on_disabled_c = NULL);
	LLMenuItemCallGL( const std::string& name,
					  menu_callback clicked_cb,
					  enabled_callback enabled_cb,
					  label_callback label_cb,
					  void* user_data,
					  KEY key = KEY_NONE, MASK mask = MASK_NONE,
					  BOOL enabled = TRUE,
					  on_disabled_callback on_disabled_c = NULL);
	virtual LLXMLNodePtr getXML(bool save_children = true) const;

	virtual std::string getType() const	{ return "call"; }


	void setEnabledControl(std::string enabled_control, LLView *context);
	void setVisibleControl(std::string enabled_control, LLView *context);

	void setMenuCallback(menu_callback callback, void* data) { mCallback = callback;  mUserData = data; };
	menu_callback getMenuCallback() const { return mCallback; }

	void setEnabledCallback(enabled_callback callback) { mEnabledCallback = callback; };

	void setUserData(void *userdata)	{ mUserData = userdata; }
	void* getUserData() const { return mUserData; }

	// called to rebuild the draw label
	virtual void buildDrawLabel( void );

	// doIt() - do the primary funcationality of the menu item.
	virtual void doIt( void );

	virtual BOOL handleAcceleratorKey(KEY key, MASK mask);

	//virtual void draw();

        // Functionality for tracking callback types for setting after widget is in place.
        virtual BOOL setCtrlResponse(U8 llMenuItemCallType,
                                    const std::string& name,
                                    void* user_data,
                                    void *callback_fcn);


private:
	menu_callback			mCallback;
	// mEnabledCallback should return TRUE if the item should be enabled
	enabled_callback		mEnabledCallback;	
	label_callback			mLabelCallback;
	void*					mUserData;
	on_disabled_callback	mOnDisabledCallback;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Class LLMenuItemCheckGL
//
// The LLMenuItemCheckGL is an extension of the LLMenuItemCallGL
// class, by allowing another method to be specified which determines
// if the menu item should consider itself checked as true or not.  Be
// careful that the provided callback is fast - it needs to be VERY
// FUCKING EFFICIENT, because it may need to be checked a lot.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class LLMenuItemCheckGL 
:	public LLMenuItemCallGL
{
public:
	LLMenuItemCheckGL( const std::string& name, 
					   const std::string& label,
					   menu_callback callback,
					   enabled_callback enabled_cb,
					   check_callback check,
					   void* user_data,
					   KEY key = KEY_NONE, MASK mask = MASK_NONE );
	LLMenuItemCheckGL( const std::string& name, 
					   menu_callback callback,
					   enabled_callback enabled_cb,
					   check_callback check,
					   void* user_data,
					   KEY key = KEY_NONE, MASK mask = MASK_NONE );
	LLMenuItemCheckGL( const std::string& name, 
					   const std::string& label,
					   menu_callback callback,
					   enabled_callback enabled_cb,
					   std::string control_name,
					   LLView *context,
					   void* user_data,
					   KEY key = KEY_NONE, MASK mask = MASK_NONE );
	virtual LLXMLNodePtr getXML(bool save_children = true) const;

	void setCheckedControl(std::string checked_control, LLView *context);

	virtual void setValue(const LLSD& value);

	virtual std::string getType() const	{ return "check"; }

	// called to rebuild the draw label
	virtual void buildDrawLabel( void );

private:
	check_callback mCheckCallback;
	BOOL mChecked;
};


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Class LLMenuItemToggleGL
//
// The LLMenuItemToggleGL is a menu item that wraps around a user
// specified and controlled boolean.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class LLMenuItemToggleGL : public LLMenuItemGL
{
public:
	LLMenuItemToggleGL( const std::string& name, const std::string& label,
						BOOL* toggle, 
						KEY key = KEY_NONE, MASK mask = MASK_NONE );

	LLMenuItemToggleGL( const std::string& name,
						BOOL* toggle, 
						KEY key = KEY_NONE, MASK mask = MASK_NONE );

	// there is no getXML() because we cannot reference the toggled global variable by XML
	// use LLMenuItemCheckGL instead.

	virtual std::string getType() const	{ return "toggle"; }

	// called to rebuild the draw label
	virtual void buildDrawLabel( void );

	// doIt() - do the primary funcationality of the menu item.
	virtual void doIt( void );

	// LLView Functionality
	//virtual void draw( void );

        // Functionality for tracking callback types for setting after widget is in place.
        virtual BOOL setCtrlResponse(U8 llMenuItemCallType,
                                    const std::string& name,
                                    void* user_data,
				     void *callback_fcn) { return FALSE; };

private:
	BOOL* mToggle;
};


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Class LLMenuGL
//
// The Menu class represents a normal rectangular menu somewhere on
// screen. A Menu can have menu items (described above) or sub-menus
// attached to it. Sub-menus are implemented via a specialized
// menu-item type known as a branch. Since it's easy to do wrong, I've
// taken the branch functionality out of public view, and encapsulate
// it in the appendMenu() method.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class LLMenuGL 
:	public LLUICtrl
// TODO: The menu and menu item classes share a great deal of functionality and perhaps should be united.
// I think it may make the most sense to make LLMenuGL be a subclass of LLMenuItemGL. -MG
{
	// let branching menu items use my protected traversal methods
	friend class LLMenuItemBranchGL;
public:
	LLMenuGL( const std::string& name, const std::string& label, LLHandle<LLFloater> parent_floater = LLHandle<LLFloater>());
	LLMenuGL( const std::string& label, LLHandle<LLFloater> parent_floater = LLHandle<LLFloater>() );
	virtual ~LLMenuGL( void );
	virtual LLXMLNodePtr getXML(bool save_children = true) const;
	static LLView* fromXML(LLXMLNodePtr node, LLView *parent, LLUICtrlFactory *factory);

	void parseChildXML(LLXMLNodePtr child, LLView *parent, LLUICtrlFactory *factory);


	// LLView Functionality
	virtual BOOL handleUnicodeCharHere( llwchar uni_char );
	virtual BOOL handleHover( S32 x, S32 y, MASK mask );
	virtual void draw( void );
	virtual void drawBackground(LLMenuItemGL* itemp, LLColor4& color);
	virtual void setVisible(BOOL visible);

	virtual BOOL handleAcceleratorKey(KEY key, MASK mask);

	LLMenuGL* getChildMenuByName(const std::string& name, BOOL recurse) const;
	
	BOOL clearHoverItem();

	// return the name label
	const std::string& getLabel( void ) const { return mLabel.getString(); }
	void setLabel(const LLStringExplicit& label) { mLabel = label; }

	// background colors
	static void setDefaultBackgroundColor( const LLColor4& color ) { sDefaultBackgroundColor = color; }
        void setBackgroundColor( const LLColor4& color );
        void setBorderColor( const LLColor4& color );
	const LLColor4& getBackgroundColor() const { return mBackgroundColor; }
        const LLColor4& getBorderColor() const { return mBorderColor; }
	void setBackgroundVisible( BOOL b )	{ mBgVisible = b; }
	void setCanTearOff(BOOL tear_off, LLHandle<LLFloater> parent_floater_handle = LLHandle<LLFloater>());

	// Add the menu item to this menu.
	virtual BOOL append( LLMenuItemGL* item );

	// Remove a menu item from this menu.
	virtual BOOL remove( LLMenuItemGL* item );

	// *NOTE:Mani - appendNoArrange() should be removed when merging to skinning/viewer2.0
	// Its added as a fix to a viewer 1.23 bug that has already been address by skinning work.
	virtual BOOL appendNoArrange( LLMenuItemGL* item ); 

	// add a separator to this menu
	virtual BOOL appendSeparator( const std::string &separator_name = LLStringUtil::null );

	// add a menu - this will create a cascading menu
	virtual BOOL appendMenu( LLMenuGL* menu );

	// for branching menu items, bring sub menus up to root level of menu hierarchy
	virtual void updateParent( LLView* parentp );

	// setItemEnabled() - pass the name and the enable flag for a
	// menu item. TRUE will make sure it's enabled, FALSE will disable
	// it.
	void setItemEnabled( const std::string& name, BOOL enable ); 
	
	// propagate message to submenus
	void setEnabledSubMenus(BOOL enable);

	void setItemVisible( const std::string& name, BOOL visible);
	
	// sets the left,bottom corner of menu, useful for popups
	void setLeftAndBottom(S32 left, S32 bottom);

	virtual BOOL handleJumpKey(KEY key);

	virtual BOOL jumpKeysActive();

	virtual BOOL isOpen();

	// Shape this menu to fit the current state of the children, and
	// adjust the child rects to fit. This is called automatically
	// when you add items. *FIX: We may need to deal with visibility
	// arrangement.
	virtual void arrange( void );

	// remove all items on the menu
	void empty( void );

	// Rearrange the components, and do the right thing if the menu doesn't
	// fit in the bounds.
	// virtual void arrangeWithBounds(LLRect bounds);

	void			setItemLastSelected(LLMenuItemGL* item);	// must be in menu
	U32				getItemCount();				// number of menu items
	LLMenuItemGL*	getItem(S32 number);		// 0 = first item
	LLMenuItemGL*	getHighlightedItem();				

	LLMenuItemGL*	highlightNextItem(LLMenuItemGL* cur_item, BOOL skip_disabled = TRUE);
	LLMenuItemGL*	highlightPrevItem(LLMenuItemGL* cur_item, BOOL skip_disabled = TRUE);

	void buildDrawLabels();
	void createJumpKeys();

	// Show popup in global screen space based on last mouse location.
	static void showPopup(LLMenuGL* menu);

	// Show popup at a specific location.
	static void showPopup(LLView* spawning_view, LLMenuGL* menu, S32 x, S32 y);

	// Whether to drop shadow menu bar 
	void setDropShadowed( const BOOL shadowed );

	void setParentMenuItem( LLMenuItemGL* parent_menu_item ) { mParentMenuItem = parent_menu_item; }
	LLMenuItemGL* getParentMenuItem() const { return mParentMenuItem; }

	void setTornOff(BOOL torn_off);
	BOOL getTornOff() { return mTornOff; }

	BOOL getCanTearOff() { return mTearOffItem != NULL; }

	KEY getJumpKey() const { return mJumpKey; }
	void setJumpKey(KEY key) { mJumpKey = key; }

	static void setKeyboardMode(BOOL mode) { sKeyboardMode = mode; }
	static BOOL getKeyboardMode() { return sKeyboardMode; }

	static class LLMenuHolderGL* sMenuContainer;

         BOOL setCtrlResponse(U8 llMenuItemCallType,const std::string& name,void* user_data,void *callback_fcn);
	
protected:
	void createSpilloverBranch();
	void cleanupSpilloverBranch();

	// TODO: create accessor methods for these?
	typedef std::list< LLMenuItemGL* > item_list_t;
	item_list_t mItems;
	typedef std::map<KEY, LLMenuItemGL*> navigation_key_map_t;
	navigation_key_map_t mJumpKeys;
	S32				mLastMouseX;
	S32				mLastMouseY;
	S32				mMouseVelX;
	S32				mMouseVelY;
	BOOL			mHorizontalLayout;
	BOOL			mKeepFixedSize;

private:
	static LLColor4 sDefaultBackgroundColor;
	static BOOL		sKeyboardMode;

	LLColor4		mBackgroundColor;
        LLColor4                mBorderColor;
	BOOL			mBgVisible;
	LLMenuItemGL*	mParentMenuItem;
	LLUIString		mLabel;
	BOOL mDropShadowed; 	//  Whether to drop shadow 
	BOOL			mHasSelection;
	LLFrameTimer	mFadeTimer;
	BOOL			mTornOff;
	class LLMenuItemTearOffGL* mTearOffItem;
	class LLMenuItemBranchGL* mSpilloverBranch;
	LLMenuGL*		mSpilloverMenu;
	LLHandle<LLFloater>	mParentFloaterHandle;
	KEY				mJumpKey;
}; // end class LLMenuGL



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Class LLMenuItemBranchGL
//
// The LLMenuItemBranchGL represents a menu item that has a
// sub-menu. This is used to make cascading menus.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class LLMenuItemBranchGL : public LLMenuItemGL
{
public:
	LLMenuItemBranchGL( const std::string& name, const std::string& label, LLHandle<LLView> branch,
						KEY key = KEY_NONE, MASK mask = MASK_NONE );

	virtual ~LLMenuItemBranchGL();

	virtual LLXMLNodePtr getXML(bool save_children = true) const;

	virtual std::string getType() const { return "menu"; }

	virtual BOOL handleMouseUp(S32 x, S32 y, MASK mask);

	virtual BOOL handleAcceleratorKey(KEY key, MASK mask);

	// check if we've used these accelerators already
	virtual BOOL addToAcceleratorList(std::list <LLKeyBinding*> *listp);

	// called to rebuild the draw label
	virtual void buildDrawLabel( void );

	// doIt() - do the primary funcationality of the menu item.
	virtual void doIt( void );

	virtual BOOL handleKey(KEY key, MASK mask, BOOL called_from_parent);
	virtual BOOL handleUnicodeChar(llwchar uni_char, BOOL called_from_parent);

	// set the hover status (called by it's menu) and if the object is
	// active. This is used for behavior transfer.
	virtual void setHighlight( BOOL highlight );

	virtual BOOL handleKeyHere(KEY key, MASK mask);

	virtual BOOL isActive() const { return isOpen() && getBranch()->getHighlightedItem(); }

	virtual BOOL isOpen() const { return getBranch() && getBranch()->isOpen(); }

	LLMenuGL *getBranch() const { return (LLMenuGL*)(mBranch.get()); }

	virtual void updateBranchParent( LLView* parentp );

	// LLView Functionality
	virtual void onVisibilityChange( BOOL curVisibilityIn );

	virtual void draw();

	virtual void setEnabledSubMenus(BOOL enabled) { if(getBranch()) getBranch()->setEnabledSubMenus(enabled); }

	virtual void openMenu();

	virtual LLView* getChildView(const std::string& name, BOOL recurse = TRUE, BOOL create_if_missing = TRUE) const;

        virtual BOOL setCtrlResponse(U8 llMenuItemCallType,
                                    const std::string& name,
                                    void* user_data,
                                    void *callback_fcn);

private:
	LLHandle<LLView> mBranch;
}; // end class LLMenuItemBranchGL



//-----------------------------------------------------------------------------
// class LLPieMenu
// A circular menu of items, icons, etc.
//-----------------------------------------------------------------------------

class LLPieMenu
: public LLMenuGL
{
public:
	LLPieMenu(const std::string& name, const std::string& label);
	LLPieMenu(const std::string& name);
	virtual ~LLPieMenu() {}

	virtual LLXMLNodePtr getXML(bool save_children = true) const;
	void initXML(LLXMLNodePtr node, LLView *context, LLUICtrlFactory *factory);

	// LLView Functionality
	// can't set visibility directly, must call show or hide
	virtual void setVisible(BOOL visible);
	
	virtual BOOL handleHover( S32 x, S32 y, MASK mask );
	virtual BOOL handleMouseDown( S32 x, S32 y, MASK mask );
	virtual BOOL handleRightMouseDown(S32 x, S32 y, MASK mask);
	virtual BOOL handleRightMouseUp( S32 x, S32 y, MASK mask );
	virtual BOOL handleMouseUp( S32 x, S32 y, MASK mask );
	virtual void draw();
	virtual void drawBackground(LLMenuItemGL* itemp, LLColor4& color);

	virtual BOOL append(LLMenuItemGL* item);
	virtual BOOL appendSeparator( const std::string &separator_name = LLStringUtil::null );

	BOOL appendPieMenu(LLPieMenu *menu);

	virtual void arrange( void );

	// Display the menu centered on this point on the screen.
	void show(S32 x, S32 y, BOOL mouse_down);
	void hide(BOOL item_selected);

        virtual BOOL setCtrlResponse(U8 llMenuItemCallType,
                                    const std::string& name,
                                    void* user_data,
                                    void *callback_fcn) { return FALSE; } ;

private:
	LLMenuItemGL *pieItemFromXY(S32 x, S32 y);
	S32			  pieItemIndexFromXY(S32 x, S32 y);

	// These cause menu items to be spuriously selected by right-clicks
	// near the window edge at low frame rates.  I don't think they are
	// needed unless you shift the menu position in the draw() function. JC
	//S32				mShiftHoriz;	// non-zero if menu had to shift this frame
	//S32				mShiftVert;		// non-zero if menu had to shift this frame
	BOOL			mFirstMouseDown;	// true from show until mouse up
	BOOL			mUseInfiniteRadius;	// allow picking pie menu items anywhere outside of center circle
	LLMenuItemGL*	mHoverItem;
	BOOL			mHoverThisFrame;
	BOOL			mHoveredAnyItem;
	LLFrameTimer	mShrinkBorderTimer;
	F32				mOuterRingAlpha; // for rendering pie menus as both bounded and unbounded
	F32				mCurRadius;
	BOOL			mRightMouseDown;
};


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Class LLMenuBarGL
//
// A menu bar displays menus horizontally.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class LLMenuBarGL : public LLMenuGL
{
public:
	LLMenuBarGL( const std::string& name );
	virtual ~LLMenuBarGL();
	virtual LLXMLNodePtr getXML(bool save_children = true) const;
	static LLView* fromXML(LLXMLNodePtr node, LLView *parent, LLUICtrlFactory *factory);

	virtual BOOL handleAcceleratorKey(KEY key, MASK mask);
	virtual BOOL handleKeyHere(KEY key, MASK mask);
	virtual BOOL handleJumpKey(KEY key);
	virtual BOOL handleMouseDown(S32 x, S32 y, MASK mask);
	virtual BOOL handleRightMouseDown(S32 x, S32 y, MASK mask);

	// rearrange the child rects so they fit the shape of the menu
	// bar.
	virtual void arrange( void );
	virtual void draw();
	virtual BOOL jumpKeysActive();

	// add a vertical separator to this menu
	virtual BOOL appendSeparator( const std::string &separator_name = LLStringUtil::null );

	// add a menu - this will create a drop down menu.
	virtual BOOL appendMenu( LLMenuGL* menu );

	// LLView Functionality
	virtual BOOL handleHover( S32 x, S32 y, MASK mask );

	// Returns x position of rightmost child, usually Help menu
	S32 getRightmostMenuEdge();

	void resetMenuTrigger() { mAltKeyTrigger = FALSE; }

        virtual BOOL setCtrlResponse(U8 llMenuItemCallType,
                                    const std::string& name,
                                    void* user_data,
                                    void *callback_fcn) { return FALSE; } ;

private:
	void checkMenuTrigger();

	std::list <LLKeyBinding*>	mAccelerators;
	BOOL						mAltKeyTrigger;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Class LLMenuHolderGL
//
// High level view that serves as parent for all menus
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class LLMenuHolderGL : public LLPanel
{
public:
	LLMenuHolderGL();
	LLMenuHolderGL(const std::string& name, const LLRect& rect, BOOL mouse_opaque, U32 follows = FOLLOWS_NONE);
	virtual ~LLMenuHolderGL() {}

	virtual BOOL hideMenus();
	void reshape(S32 width, S32 height, BOOL called_from_parent = TRUE);
	void setCanHide(BOOL can_hide) { mCanHide = can_hide; }

	// LLView functionality
	virtual void draw();
	virtual BOOL handleMouseDown( S32 x, S32 y, MASK mask );
	virtual BOOL handleRightMouseDown( S32 x, S32 y, MASK mask );

	virtual const LLRect getMenuRect() const { return getLocalRect(); }
	virtual BOOL hasVisibleMenu() const;

	static void setActivatedItem(LLMenuItemGL* item);

private:
	static LLHandle<LLView> sItemLastSelectedHandle;
	static LLFrameTimer sItemActivationTimer;

	BOOL mCanHide;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Class LLTearOffMenu
//
// Floater that hosts a menu
// https://wiki.lindenlab.com/mediawiki/index.php?title=LLTearOffMenu&oldid=81344
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class LLTearOffMenu : public LLFloater
{
public:
	static LLTearOffMenu* create(LLMenuGL* menup);
	virtual ~LLTearOffMenu() {}
	virtual void onClose(bool app_quitting);
	virtual void draw(void);
	virtual void onFocusReceived();
	virtual void onFocusLost();
	virtual BOOL handleUnicodeChar(llwchar uni_char, BOOL called_from_parent);
	virtual BOOL handleKeyHere(KEY key, MASK mask);
	virtual void translate(S32 x, S32 y);

private:
	LLTearOffMenu(LLMenuGL* menup);

	LLView*		mOldParent;
	LLMenuGL*	mMenu;
	F32			mTargetHeight;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Class LLMenuItemTearOffGL
//
// This class represents a separator.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class LLMenuItemTearOffGL : public LLMenuItemGL
{
public:
	LLMenuItemTearOffGL( LLHandle<LLFloater> parent_floater_handle = LLHandle<LLFloater>());

	virtual LLXMLNodePtr getXML(bool save_children = true) const;
	virtual std::string getType() const { return "tearoff_menu"; }

	virtual void doIt(void);
	virtual void draw(void);
	virtual U32 getNominalHeight() const;

        virtual BOOL setCtrlResponse(U8 llMenuItemCallType,
                                    const std::string& name,
                                    void* user_data,
                                    void *callback_fcn) { return FALSE; } ;

private:
	LLHandle<LLFloater> mParentHandle;
};


// *TODO: this is currently working, so finish implementation
class LLEditMenuHandlerMgr
{
public:
	LLEditMenuHandlerMgr& getInstance() {
		static LLEditMenuHandlerMgr instance;
		return instance;
	}
	virtual ~LLEditMenuHandlerMgr() {}
private:
	LLEditMenuHandlerMgr() {};
};

#endif // LL_LLMENUGL_H
