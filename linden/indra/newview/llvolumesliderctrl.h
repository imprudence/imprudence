/** 
 * @file llvolumesliderctrl.h
 * @brief A horizontal volume slider.
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

#ifndef LL_LLVOLUMESLIDERCTRL_H
#define LL_LLVOLUMESLIDERCTRL_H

#include "llslider.h"


class LLVolumeSliderCtrl
: public LLSlider
{
public:
	
	virtual EWidgetType getWidgetType() const { return WIDGET_TYPE_VOLUME_SLIDER; }
	virtual LLString getWidgetTag() const { return LL_VOLUME_SLIDER_CTRL_TAG; }

	virtual LLXMLNodePtr getXML(bool save_children = true) const;
	static LLView* fromXML(LLXMLNodePtr node, LLView *parent, LLUICtrlFactory *factory);

	LLVolumeSliderCtrl(const std::string& name,
					   const LLRect& rect,
					   void (*commit_callback)(LLUICtrl* ctrl, void* data),
					   void* callback_data);
	virtual ~LLVolumeSliderCtrl();

	virtual void draw();
};


#endif
