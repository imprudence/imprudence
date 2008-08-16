/** 
 * @file llnotify.cpp
 * @brief Non-blocking notification that doesn't take keyboard focus.
 *
 * Copyright (c) 2003-2007, Linden Research, Inc.
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

#include "llviewerprecompiledheaders.h"

#include "llnotify.h"

#include "llchat.h"
#include "llfocusmgr.h"

#include "llbutton.h"
#include "llfocusmgr.h"
#include "llglheaders.h"
#include "lliconctrl.h"
#include "lltextbox.h"
#include "lltexteditor.h"
#include "lluiconstants.h"
#include "llui.h"
#include "llxmlnode.h"
#include "llalertdialog.h"
#include "llviewercontrol.h"
#include "llviewerimagelist.h"
#include "llfloaterchat.h"	// for add_chat_history()
#include "lloverlaybar.h" // for gOverlayBar
#include "lluictrlfactory.h"

// Globals
LLNotifyBoxView* gNotifyBoxView = NULL;

const F32 ANIMATION_TIME = 0.333f;

// statics
S32 LLNotifyBox::sNotifyBoxCount = 0;
const LLFontGL* LLNotifyBox::sFont = NULL;
const LLFontGL* LLNotifyBox::sFontSmall = NULL;

LLNotifyBox::template_map_t LLNotifyBox::sNotifyTemplates;

//---------------------------------------------------------------------------
// LLNotifyBox
//---------------------------------------------------------------------------

//static
void LLNotifyBox::showXml( const LLString& xml_desc, notify_callback_t callback, void *user_data)
{
	return showXml(xml_desc, LLString::format_map_t(), callback, user_data);
}

//static
void LLNotifyBox::showXml( const LLString& xml_desc, const LLString::format_map_t& args,
						   notify_callback_t callback, void *user_data)
{
	LLNotifyBox* notify = new LLNotifyBox(xml_desc, args, callback, user_data);
	gNotifyBoxView->addChild(notify);
}

//static
void LLNotifyBox::showXml( const LLString& xml_desc, const LLString::format_map_t& args,
						   notify_callback_t callback, void *user_data,
						   const option_list_t& options,
						   BOOL layout_script_dialog)
{
	LLNotifyBox* notify = new LLNotifyBox(xml_desc, args, callback, user_data, options, layout_script_dialog);
	gNotifyBoxView->addChild(notify);
}

LLPointer<LLNotifyBoxTemplate> LLNotifyBox::sDefaultTemplate;

void LLNotifyBox::cleanup()
{
	sDefaultTemplate = NULL;
}

//---------------------------------------------------------------------------

LLNotifyBox::LLNotifyBox(const LLString& xml_desc, const LLString::format_map_t& args,
						 notify_callback_t callback, void* user_data,
						 const option_list_t& extra_options,
						 BOOL layout_script_dialog)
	: LLPanel("notify", LLRect(), BORDER_NO),
	  LLEventTimer(gSavedSettings.getF32("NotifyTipDuration")),
	  mIsTip(FALSE),
	  mAnimating(TRUE),
	  mTimer(),
	  mNextBtn(NULL),
	  mCallback(callback),
	  mData(user_data),
	  mNumOptions(0),
	  mDefaultOption(0)
{
	// clicking on a button does not steal current focus
	setIsChrome(TRUE);

	// class init
	if (!sFont)
	{
		sFont = LLFontGL::sSansSerif;
		sFontSmall = LLFontGL::sSansSerifSmall;
	}

	// get template
	
	if (!sDefaultTemplate)
	{
		sDefaultTemplate = new LLNotifyBoxTemplate;
	}
	
	LLPointer<LLNotifyBoxTemplate> xml_template;
	template_map_t::iterator iter = sNotifyTemplates.find(xml_desc);
	if (iter != sNotifyTemplates.end())
	{
		xml_template = iter->second;
	}
	else
	{
		LLString tmsg = "[Notification template not found:\n " + xml_desc + " ]";
		sDefaultTemplate->setMessage(tmsg);
		xml_template = sDefaultTemplate;
	}

	// setup paramaters
	
	LLString message = xml_template->mMessage;
	LLAlertDialog::format(message, args);
	
	option_list_t options = xml_template->mOptions;
	options.insert(options.end(), extra_options.begin(), extra_options.end());

	// initialize
	
	mIsTip = xml_template->mIsTip;
	mIsFocusRoot = !mIsTip;
	mAnimating = TRUE;
	mCallback = callback;
	mData = user_data;
	mNumOptions = options.size();
	mDefaultOption = xml_template->mDefaultOption;
		  
	LLRect rect = mIsTip ? getNotifyTipRect(message)
		   		  		 : getNotifyRect(mNumOptions, layout_script_dialog);
	setRect(rect);
	setFollows(mIsTip ? (FOLLOWS_BOTTOM|FOLLOWS_RIGHT) : (FOLLOWS_TOP|FOLLOWS_RIGHT));
	setBackgroundVisible(FALSE);
	setBackgroundOpaque(TRUE);

	LLIconCtrl* icon;
	LLTextEditor* text;

	S32 x = HPAD + HPAD;
	const S32 TOP = mRect.getHeight() - (mIsTip ? (S32)sFont->getLineHeight() : 32);
	const S32 BOTTOM = (S32)sFont->getLineHeight();

	icon = new LLIconCtrl("icon",
						  LLRect(x, TOP, x+32, TOP-32),
						  mIsTip ? "notify_tip_icon.tga" : "notify_box_icon.tga");
	icon->setMouseOpaque(FALSE);
	addChild(icon);

	x += HPAD + HPAD + 32;

	const S32 BOTTOM_PAD = VPAD * 3;
	const S32 BTN_TOP = BOTTOM_PAD + (((mNumOptions-1+2)/3)) * (BTN_HEIGHT+VPAD);

	// Tokenization on \n is handled by LLTextBox

	const S32 MAX_LENGTH = 512 + 20 + 
		DB_FIRST_NAME_BUF_SIZE + 
		DB_LAST_NAME_BUF_SIZE +
		DB_INV_ITEM_NAME_BUF_SIZE;  // For script dialogs: add space for title.

	text = new LLTextEditor("box",
							LLRect(x, TOP, mRect.getWidth()-2, mIsTip ? BOTTOM : BTN_TOP+16),
							MAX_LENGTH,
							message,
							sFont,
							FALSE);
	text->setWordWrap(TRUE);
	text->setTakesFocus(FALSE);
	text->setMouseOpaque(FALSE);
	text->setBorderVisible(FALSE);
	text->setTakesNonScrollClicks(FALSE);
	text->setHideScrollbarForShortDocs(TRUE);
	text->setReadOnlyBgColor ( LLColor4::transparent ); // the background color of the box is manually 
	                                                    // rendered under the text box, therefore we want 
														// the actual text box to be transparent
	text->setReadOnlyFgColor ( gColors.getColor("NotifyTextColor") );
	text->setEnabled(FALSE); // makes it read-only
	text->setTabStop(FALSE); // can't tab to it (may be a problem for scrolling via keyboard)
	addChild(text);

	if (mIsTip)
	{
		// TODO: Make a separate archive for these.
		LLChat chat(message);
		chat.mSourceType = CHAT_SOURCE_SYSTEM;
		gFloaterChat->addChatHistory(chat);
	}
	else
	{
		LLButton* btn;
		btn = new LLButton("next",
						   LLRect(mRect.getWidth()-24, BOTTOM_PAD+16, mRect.getWidth()-8, BOTTOM_PAD),
						   "notify_next.tga",
						   "notify_next.tga",
						   "",
						   onClickNext,
						   this,
						   sFont);
		btn->setToolTip("Next");
		addChild(btn);
		mNextBtn = btn;

		S32 btn_width = 90;
		LLRect btn_rect;

		for (S32 i = 0; i < mNumOptions; i++)
		{
			S32 index = i;
			S32 btn_height= BTN_HEIGHT;
			const LLFontGL* font = sFont;
			S32 ignore_pad = 0;

			if (layout_script_dialog)
			{
				// Add two "blank" option spaces, before the "Ignore" button
				index = i + 2;
				if (i == 0)
				{
					// Ignore button is smaller, less wide
					btn_height = BTN_HEIGHT_SMALL;
					font = sFontSmall;
					ignore_pad = 10;
				}
			}

			btn_rect.setOriginAndSize(x + (index % 3) * (btn_width+HPAD+HPAD) + ignore_pad,
									  BOTTOM_PAD + (index / 3) * (BTN_HEIGHT+VPAD),
									  btn_width - 2*ignore_pad,
									  btn_height);

			InstanceAndS32* userdata = new InstanceAndS32;
			userdata->mSelf = this;
			userdata->mButton = i;

			mBtnCallbackData.put(userdata);

			btn = new LLButton(options[i], btn_rect, "", onClickButton, userdata);
			btn->setFont(font);
			addChild(btn, -1);

			if (i == mDefaultOption)
			{
				setDefaultBtn(btn);
			}
		}
		
		sNotifyBoxCount++;

		// If this is the only notify box, don't show the next button
		if (sNotifyBoxCount == 1
			&& mNextBtn)
		{
			mNextBtn->setVisible(FALSE);
		}
	}
}

// virtual
LLNotifyBox::~LLNotifyBox()
{
	S32 count = mBtnCallbackData.count();
	for (S32 i = 0; i < count; i++)
	{
		delete mBtnCallbackData[i];
	}
}

// virtual
BOOL LLNotifyBox::handleMouseUp(S32 x, S32 y, MASK mask)
{
	if (mIsTip)
	{
		close();
		return TRUE;
	}

	setFocus(TRUE);

	return LLPanel::handleMouseUp(x, y, mask);
}

// virtual
BOOL LLNotifyBox::handleRightMouseDown(S32 x, S32 y, MASK mask)
{
	if (!mIsTip && getVisible() && getEnabled() && pointInView(x,y))
	{
		moveToBack();
		return TRUE;
	}

	return LLPanel::handleRightMouseDown(x, y, mask);
}


// virtual
void LLNotifyBox::draw()
{
	F32 display_time = mTimer.getElapsedTimeF32();

	if (mAnimating && display_time < ANIMATION_TIME)
	{
		glMatrixMode(GL_MODELVIEW);
		LLUI::pushMatrix();

		S32 height = mRect.getHeight();
		F32 fraction = display_time / ANIMATION_TIME;
		F32 voffset = (1.f - fraction) * height;
		if (mIsTip) voffset *= -1.f;
		LLUI::translate(0.f, voffset, 0.f);
		
		drawBackground();

		LLPanel::draw();

		LLUI::popMatrix();
	}
	else
	{
		if(mAnimating)
		{
			mAnimating = FALSE;
			if(!mIsTip) 
			{
				// hide everyone behind me once I'm done animating
				gNotifyBoxView->showOnly(this);
			}
		}
		drawBackground();
		LLPanel::draw();
	}
}

void LLNotifyBox::drawBackground() const
{
	LLUUID image_id;
	image_id.set(gViewerArt.getString("rounded_square.tga"));
	LLViewerImage* imagep = gImageList.getImage(image_id, MIPMAP_FALSE, TRUE);
	if (imagep)
	{
		LLGLSTexture texture_enabled;
		LLViewerImage::bindTexture(imagep);
		LLColor4 color = gColors.getColor("NotifyBoxColor");
		if(gFocusMgr.childHasKeyboardFocus( this ))
		{
			const S32 focus_width = 2;
			color = gColors.getColor("FloaterFocusBorderColor");
			glColor4fv(color.mV);
			gl_segmented_rect_2d_tex(-focus_width, mRect.getHeight() + focus_width, 
									mRect.getWidth() + focus_width, -focus_width,
									imagep->getWidth(), imagep->getHeight(),
									16, mIsTip ? ROUNDED_RECT_TOP : ROUNDED_RECT_BOTTOM);
			color = gColors.getColor("ColorDropShadow");
			glColor4fv(color.mV);
			gl_segmented_rect_2d_tex(0, mRect.getHeight(), mRect.getWidth(), 0, imagep->getWidth(), imagep->getHeight(), 16, mIsTip ? ROUNDED_RECT_TOP : ROUNDED_RECT_BOTTOM);
			color = gColors.getColor("NotifyBoxColor");
			glColor4fv(color.mV);
			gl_segmented_rect_2d_tex(1, mRect.getHeight()-1, mRect.getWidth()-1, 1, imagep->getWidth(), imagep->getHeight(), 16, mIsTip ? ROUNDED_RECT_TOP : ROUNDED_RECT_BOTTOM);
		}
		else
		{
			glColor4fv(color.mV);
			gl_segmented_rect_2d_tex(0, mRect.getHeight(), mRect.getWidth(), 0, imagep->getWidth(), imagep->getHeight(), 16, mIsTip ? ROUNDED_RECT_TOP : ROUNDED_RECT_BOTTOM);
		}
	}
}


void LLNotifyBox::close()
{
	BOOL isTipTmp = mIsTip;

	if (!mIsTip)
	{
		sNotifyBoxCount--;
	}

	die();
	if(!isTipTmp)
	{
		LLNotifyBox * front = gNotifyBoxView->getFirstNontipBox();
		if(front)
		{
			gNotifyBoxView->showOnly(front);
			// we're assuming that close is only called by user action (for non-tips),
			// so we then give focus to the next close button
			front->mDefaultBtn->setFocus(TRUE);
			gFocusMgr.triggerFocusFlash(); // TODO it's ugly to call this here
		}
	}
}

/*virtual*/
void LLNotifyBox::tick()
{
	if (mIsTip)
	{
		close();
	}
}

void LLNotifyBox::setVisible(BOOL visible)
{
	// properly set the status of the next button
	if(visible && !mIsTip)
	{
		mNextBtn->setVisible(sNotifyBoxCount > 1);
		mNextBtn->setEnabled(sNotifyBoxCount > 1);
	}
	LLPanel::setVisible(visible);
}

void LLNotifyBox::moveToBack()
{
	// Move this dialog to the back.
	gNotifyBoxView->sendChildToBack(this);
	if(!mIsTip && mNextBtn)
	{
		mNextBtn->setVisible(FALSE);

		// And enable the next button on the frontmost one, if there is one
		if (gNotifyBoxView->getChildCount() > 0)
		{
			LLNotifyBox* front = gNotifyBoxView->getFirstNontipBox();
			gNotifyBoxView->showOnly(front);
			// assuming that moveToBack is only called by clicking the next button,
			// we give focus to the next next button
			front->mNextBtn->setFocus(TRUE);
			gFocusMgr.triggerFocusFlash(); // TODO: it's ugly to call this here
		}
	}
}


// static
LLRect LLNotifyBox::getNotifyRect(S32 num_options, BOOL layout_script_dialog)
{
	S32 notify_height = gSavedSettings.getS32("NotifyBoxHeight");
	const S32 NOTIFY_WIDTH = gSavedSettings.getS32("NotifyBoxWidth");

	const S32 TOP = gNotifyBoxView->getRect().getHeight();
	const S32 RIGHT = gNotifyBoxView->getRect().getWidth();
	const S32 LEFT = RIGHT - NOTIFY_WIDTH;

	if (num_options < 1)
	{
		num_options = 1;
	}

	// Add two "blank" option spaces.
	if (layout_script_dialog)
	{
		num_options += 2;
	}

	S32 additional_lines = (num_options-1) / 3;

	notify_height += additional_lines * (BTN_HEIGHT + VPAD);

	return LLRect(LEFT, TOP, RIGHT, TOP-notify_height);
}

// static
LLRect LLNotifyBox::getNotifyTipRect(const LLString &utf8message)
{
	S32 line_count = 1;
	LLWString message = utf8str_to_wstring(utf8message);
	S32 message_len = message.length();

	const S32 NOTIFY_WIDTH = gSavedSettings.getS32("NotifyBoxWidth");
	// Make room for the icon area.
	const S32 text_area_width = NOTIFY_WIDTH - HPAD * 4 - 32;

	const llwchar* wchars = message.c_str();
	const llwchar* start = wchars;
	const llwchar* end;
	S32 total_drawn = 0;
	BOOL done = FALSE;

	do
	{
		line_count++;

		for (end=start; *end != 0 && *end != '\n'; end++)
			;

		if( *end == 0 )
		{
			end = wchars + message_len;
			done = TRUE;
		}
		
		S32 remaining = end - start;
		while( remaining )
		{
			S32 drawn = sFont->maxDrawableChars( start, (F32)text_area_width, remaining, TRUE );

			if( 0 == drawn )
			{
				drawn = 1;  // Draw at least one character, even if it doesn't all fit. (avoids an infinite loop)
			}

			total_drawn += drawn; 
			start += drawn;
			remaining -= drawn;
			
			if( total_drawn < message_len )
			{
				if( (wchars[ total_drawn ] != '\n') )
				{
					// wrap because line was too long
					line_count++;
				}
			}
			else
			{
				done = TRUE;
			}
		}

		total_drawn++;	// for '\n'
		end++;
		start = end;
	} while( !done );

	const S32 MIN_NOTIFY_HEIGHT = 72;
	const S32 MAX_NOTIFY_HEIGHT = 600;
	S32 notify_height = llceil((F32) (line_count+1) * sFont->getLineHeight());
	if(gOverlayBar)
	{
		notify_height += gOverlayBar->getRect().getHeight();
	}
	else
	{
		// *FIX: this is derived from the padding caused by the
		// rounded rects, shouldn't be a const here.
		notify_height += 10;  
	}
	notify_height += VPAD;
	notify_height = llclamp(notify_height, MIN_NOTIFY_HEIGHT, MAX_NOTIFY_HEIGHT);

	const S32 RIGHT = gNotifyBoxView->getRect().getWidth();
	const S32 LEFT = RIGHT - NOTIFY_WIDTH;

	// Make sure it goes slightly offscreen
	return LLRect(LEFT, notify_height-1, RIGHT, -1);
}


// static
void LLNotifyBox::onClickButton(void* data)
{
	InstanceAndS32* self_and_button = (InstanceAndS32*)data;
	LLNotifyBox* self = self_and_button->mSelf;
	S32 button = self_and_button->mButton;

	if (self->mCallback)
	{
		self->mCallback(button, self->mData);
	}

	self->close();
}


// static
void LLNotifyBox::onClickNext(void* data)
{
	LLNotifyBox* self = static_cast<LLNotifyBox*>(data);
	self->moveToBack();
}

//-----------------------------------------------------------------------------

//static
const LLString& LLNotifyBox::getTemplateMessage(const LLString& xml_desc)
{
	template_map_t::iterator iter = sNotifyTemplates.find(xml_desc);
	if (iter != sNotifyTemplates.end())
	{
		return iter->second->mMessage;
	}
	else
	{
		return xml_desc;
	}
}

//static
bool LLNotifyBox::parseNotify(const LLString& xml_filename)
{
	LLXMLNodePtr root;

	BOOL success  = LLUICtrlFactory::getLayeredXMLNode(xml_filename, root);

	if (!success || root.isNull() || !root->hasName( "notifications" ))
	{
		llerrs << "Problem reading UI Notify file: " << xml_filename << llendl;
		return false;
	}

	for (LLXMLNode* notify = root->getFirstChild();
		 notify != NULL; notify = notify->getNextSibling())
	{
		if (!notify->hasName("notify"))
		{
			continue;
		}
		
		LLPointer<LLNotifyBoxTemplate> xml_template = new LLNotifyBoxTemplate;

		// label=
		LLString notify_name;
		if (notify->getAttributeString("name", notify_name))
		{
			xml_template->mLabel = notify_name;
		}
		else
		{
			llwarns << "Unable to parse notify with no name" << llendl;
			continue;
		}
		// modal=
		BOOL tip;
		if (notify->getAttributeBOOL("tip", tip))
		{
			xml_template->mIsTip = tip;
		}
				
		S32 btn_idx = 0;
		for (LLXMLNode* child = notify->getFirstChild();
			 child != NULL; child = child->getNextSibling())
		{
			// <message>
			if (child->hasName("message"))
			{
				xml_template->mMessage = child->getTextContents();
			}

			// <option>
			if (child->hasName("option"))
			{
				LLString label = child->getValue();
				BOOL is_default = FALSE;
				child->getAttributeBOOL("default", is_default);
				LLString ignore_text;
				if (!child->getAttributeString("ignore", ignore_text))
				{
					ignore_text = label;
				}
				xml_template->addOption(label, is_default);
				btn_idx++;
			}
		}

		//XUI:translate
		if (xml_template->mOptions.empty())
		{
			xml_template->addOption("OK", FALSE);
		}
		sNotifyTemplates[xml_template->mLabel] = xml_template;
	}
	return true;
}

LLNotifyBoxView::LLNotifyBoxView(const LLString& name, const LLRect& rect, BOOL mouse_opaque, U32 follows)
	: LLUICtrl(name,rect,mouse_opaque,NULL,NULL,follows) 
{
}

LLNotifyBox * LLNotifyBoxView::getFirstNontipBox() const
{
	// assumes every child is a notify box
	for(child_list_const_iter_t iter = getChildList()->begin();
			iter != getChildList()->end();
			iter++)
	{
		LLNotifyBox * box = static_cast<LLNotifyBox*>(*iter);
		if(!box->isTip() && !box->isDead())
		{
			return box;
		}
	}
	return NULL;
}

void LLNotifyBoxView::showOnly(LLView * view)
{
	if(view) 
	{
		// assumes that the argument is actually a child
		LLNotifyBox * shown = static_cast<LLNotifyBox*>(view);
		// make every other notification invisible
		for(child_list_const_iter_t iter = getChildList()->begin();
			iter != getChildList()->end();
			iter++)
		{
			LLNotifyBox * box = static_cast<LLNotifyBox*>(*iter);
			if(box != view && box->getVisible() && !box->isTip())
			{
				box->setVisible(FALSE);
			}
		}
		shown->setVisible(TRUE);
		sendChildToFront(shown);
	}
}
