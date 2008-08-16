/** 
 * @file llvieweruictrlfactory.cpp
 * @brief factory class for creating UI controls
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

#include <fstream>
#include <boost/tokenizer.hpp>

#include "v4color.h"

#include "llvieweruictrlfactory.h"
#include "llcolorswatch.h"
#include "llinventoryview.h"
#include "llviewertexteditor.h"
#include "lltexturectrl.h"
#include "llnameeditor.h"
#include "llnamelistctrl.h"
#include "llwebbrowserctrl.h"
#include "llvolumesliderctrl.h"
#include "lljoystickbutton.h"
#include "llmediaremotectrl.h"
#include "v4color.h"
#include <boost/tokenizer.hpp>

LLUICtrlFactory* gUICtrlFactory = NULL;


//-----------------------------------------------------------------------------
// LLUICtrlFactory()
//-----------------------------------------------------------------------------
LLViewerUICtrlFactory::LLViewerUICtrlFactory()
:	LLUICtrlFactory()
{
	// Register controls
	LLUICtrlCreator<LLColorSwatchCtrl>::registerCreator(LL_COLOR_SWATCH_CTRL_TAG, this);
	LLUICtrlCreator<LLViewerTextEditor>::registerCreator(LL_TEXT_EDITOR_TAG, this);
	LLUICtrlCreator<LLTextureCtrl>::registerCreator(LL_TEXTURE_CTRL_TAG, this);
	LLUICtrlCreator<LLNameListCtrl>::registerCreator(LL_NAME_LIST_CTRL_TAG, this);
	LLUICtrlCreator<LLNameEditor>::registerCreator(LL_NAME_EDITOR_TAG, this);
	LLUICtrlCreator<LLInventoryPanel>::registerCreator(LL_INVENTORY_PANEL_TAG, this);
#if LL_LIBXUL_ENABLED
	LLUICtrlCreator<LLWebBrowserCtrl>::registerCreator(LL_WEB_BROWSER_CTRL_TAG, this);
#endif
	LLUICtrlCreator<LLVolumeSliderCtrl>::registerCreator(LL_VOLUME_SLIDER_CTRL_TAG, this);
	LLUICtrlCreator<LLJoystickAgentSlide>::registerCreator(LL_JOYSTICK_SLIDE, this);
	LLUICtrlCreator<LLJoystickAgentTurn>::registerCreator(LL_JOYSTICK_TURN, this);
	LLUICtrlCreator<LLMediaRemoteCtrl>::registerCreator(LL_MEDIA_REMOTE_CTRL_TAG, this);
}

//-----------------------------------------------------------------------------
// ~LLUICtrlFactory()
//-----------------------------------------------------------------------------
LLViewerUICtrlFactory::~LLViewerUICtrlFactory()
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------


LLColorSwatchCtrl*	LLViewerUICtrlFactory::getColorSwatchByName(LLPanel* panelp, const LLString& name)	
{ 
	return (LLColorSwatchCtrl*)	panelp->getCtrlByNameAndType(name, WIDGET_TYPE_COLOR_SWATCH); 
}

LLNameListCtrl*		LLViewerUICtrlFactory::getNameListByName(LLPanel* panelp, const LLString& name)		
{ 
	return (LLNameListCtrl*)panelp->getCtrlByNameAndType(name, WIDGET_TYPE_NAME_LIST); 
}

LLTextureCtrl*		LLViewerUICtrlFactory::getTexturePickerByName(LLPanel* panelp, const LLString& name)
{
	return (LLTextureCtrl*)panelp->getCtrlByNameAndType(name, WIDGET_TYPE_TEXTURE_PICKER); 
}

LLWebBrowserCtrl*	LLViewerUICtrlFactory::getWebBrowserByName(LLPanel* panelp, const LLString& name)
{
	return (LLWebBrowserCtrl*)panelp->getCtrlByNameAndType(name, WIDGET_TYPE_WEBBROWSER); 
}

LLVolumeSliderCtrl*		LLViewerUICtrlFactory::getVolumeSliderByName(LLPanel* panelp, const LLString& name)		
{ 
	return (LLVolumeSliderCtrl*)panelp->getCtrlByNameAndType(name, WIDGET_TYPE_VOLUME_SLIDER); 
}

LLViewerTextEditor* LLViewerUICtrlFactory::getViewerTextEditorByName(LLPanel* panelp, const LLString& name)		
{ 
	return (LLViewerTextEditor*)panelp->getCtrlByNameAndType(name, WIDGET_TYPE_TEXT_EDITOR); 
}

LLNameEditor* LLViewerUICtrlFactory::getNameEditorByName(LLPanel* panelp, const LLString& name)		
{ 
	return (LLNameEditor*)panelp->getCtrlByNameAndType(name, WIDGET_TYPE_NAME_EDITOR); 
}

LLMediaRemoteCtrl* LLViewerUICtrlFactory::getMediaRemoteByName(LLPanel* panelp, const LLString& name)		
{ 
	return (LLMediaRemoteCtrl*)panelp->getCtrlByNameAndType(name, WIDGET_TYPE_MEDIA_REMOTE); 
}

LLJoystickAgentTurn* LLViewerUICtrlFactory::getJoystickAgentTurnByName(LLPanel* panelp, const LLString& name)		
{ 
	return (LLJoystickAgentTurn*)panelp->getCtrlByNameAndType(name, WIDGET_TYPE_JOYSTICK_TURN); 
}

LLJoystickAgentSlide* LLViewerUICtrlFactory::getJoystickAgentSlideByName(LLPanel* panelp, const LLString& name)		
{ 
	return (LLJoystickAgentSlide*)panelp->getCtrlByNameAndType(name, WIDGET_TYPE_JOYSTICK_SLIDE); 
}
