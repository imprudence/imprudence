/** 
 * @file llvolumesliderctrl.cpp
 * @brief Horizontal volume slider.
 *
 * Copyright (c) 2004-2007, Linden Research, Inc.
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

#include "llvolumesliderctrl.h"

#include "llfocusmgr.h"

#include "llui.h"

const F32 VOL_DEFAULT = 0.125f;
const F32 VOL_MIN = 0.f;
const F32 VOL_MAX = 0.5f;
const F32 VOL_INC = 0.01f;

LLVolumeSliderCtrl::LLVolumeSliderCtrl(const std::string& name,
									   const LLRect& rect,
									   void (*commit_callback)(LLUICtrl*, void*),
									   void* callback_data)
									   :
LLSlider(name, rect, commit_callback, callback_data, 
		 VOL_DEFAULT, 
		 VOL_MIN,
		 VOL_MAX, 
		 VOL_INC)
{ }

LLVolumeSliderCtrl::~LLVolumeSliderCtrl()
{ }

// virtual
void LLVolumeSliderCtrl::draw()
{
	if(!getVisible()) return;

	F32 opacity = mEnabled ? 1.f : 0.3f;

	// Track
	LLRect track(0, mRect.getHeight(), mRect.getWidth(), 0);

	track.mBottom += 3;
	track.mTop -= 1;
	track.mRight -= 1;

	LLColor4 center_color = (mThumbCenterColor % opacity);
	LLColor4 outline_color = (mThumbOutlineColor % opacity);

	gl_triangle_2d(track.mLeft, track.mBottom,
				   track.mRight, track.mBottom,
				   track.mRight, track.mTop,
				   center_color,
				   TRUE);
	gl_triangle_2d(track.mLeft, track.mBottom,
				   track.mRight, track.mBottom,
				   track.mRight, track.mTop,
				   outline_color,
				   FALSE);

	if (hasMouseCapture())
	{
		// Thumb
		LLRect rect(mDragStartThumbRect);
		gl_rect_2d( rect, outline_color );
		rect.stretch(-1);
		gl_rect_2d( rect, mThumbCenterColor % 0.3f );

		// Thumb
		if (gFocusMgr.childHasKeyboardFocus(this))
		{
			LLRect thumb_rect = mThumbRect;
			thumb_rect.stretch(llround(lerp(1.f, 3.f, gFocusMgr.getFocusFlashAmt())));
			gl_rect_2d(thumb_rect, gFocusMgr.getFocusColor());
		}
		gl_rect_2d( mThumbRect, mThumbOutlineColor );
	}
	else
	{ 
		LLRect rect(mThumbRect);
		// Thumb
		if (gFocusMgr.childHasKeyboardFocus(this))
		{
			LLRect thumb_rect = mThumbRect;
			thumb_rect.stretch(llround(lerp(1.f, 3.f, gFocusMgr.getFocusFlashAmt())));
			gl_rect_2d(thumb_rect, gFocusMgr.getFocusColor());
		}

		// Thumb
		gl_rect_2d(rect, outline_color);
		rect.stretch(-1);
		gl_rect_2d( rect, center_color);
	}

	LLUICtrl::draw();
}

// virtual
LLXMLNodePtr LLVolumeSliderCtrl::getXML(bool save_children) const
{
	LLXMLNodePtr node = LLUICtrl::getXML();

	LLString	control_name = getControlName();
	if (!control_name.empty())
	{
		node->createChild("control_name", TRUE)->setStringValue(control_name);
	}
	node->createChild("initial_val", TRUE)->setFloatValue(getInitialValue());
	node->createChild("min_val", TRUE)->setFloatValue(getMinValue());
	node->createChild("max_val", TRUE)->setFloatValue(getMaxValue());
	node->createChild("increment", TRUE)->setFloatValue(getIncrement());

	return node;
}

LLView* LLVolumeSliderCtrl::fromXML(LLXMLNodePtr node, LLView *parent, LLUICtrlFactory *factory)
{	LLString name("volume_slider");
	
	node->getAttributeString("name", name);

	LLString label;
	node->getAttributeString("label", label);

	LLRect rect;
	createRect(node, rect, parent, LLRect());

	LLFontGL* font = LLView::selectFont(node);

	// HACK: Font might not be specified.
	if (!font)
	{
		font = LLFontGL::sSansSerifSmall;
	}

	S32 label_width = font->getWidth(label) + 8;
	node->getAttributeS32("label_width", label_width);

	LLString control_name;
	node->getAttributeString("control_name", control_name);

	BOOL show_text = TRUE;
	node->getAttributeBOOL("show_text", show_text);

	BOOL can_edit_text = FALSE;
	node->getAttributeBOOL("can_edit_text", can_edit_text);

	F32 initial_value = 0.f;
	node->getAttributeF32("initial_val", initial_value);

	F32 min_value = 0.f;
	node->getAttributeF32("min_val", min_value);

	F32 max_value = 1.f; 
	node->getAttributeF32("max_val", max_value);

	F32 increment = 0.1f;
	node->getAttributeF32("increment", increment);

	U32 precision = 3;
	node->getAttributeU32("decimal_digits", precision);


	LLVolumeSliderCtrl*	slider = new LLVolumeSliderCtrl(name,rect,NULL,NULL);
		
	slider->initFromXML(node, parent);

	return slider;
}

