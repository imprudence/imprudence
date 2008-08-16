/** 
 * @file llcontainerview.cpp
 * @brief Container for all statistics info
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

#include "llviewerprecompiledheaders.h"

#include "llcontainerview.h"

#include "llerror.h"
#include "llfontgl.h"
#include "llgl.h"
#include "llui.h"
#include "llresmgr.h"
#include "llstring.h"

LLContainerView::LLContainerView(const std::string& name, const LLRect& rect)
:	LLView(name, rect, FALSE)
{
	mCollapsible = TRUE;
	mDisplayChildren = TRUE;
}

LLContainerView::~LLContainerView()
{
	// Children all cleaned up by default view destructor.
}

EWidgetType LLContainerView::getWidgetType() const
{
	return WIDGET_TYPE_CONTAINER_VIEW;
}

LLString LLContainerView::getWidgetTag() const
{
	return LL_CONTAINER_VIEW_TAG;
}

BOOL LLContainerView::handleMouseDown(S32 x, S32 y, MASK mask)
{
	BOOL handled = FALSE;
	if (mDisplayChildren)
	{
		handled = LLView::childrenHandleMouseDown(x, y, mask) != NULL;
	}
	if (!handled)
	{
		if( mCollapsible && (y >= mRect.getHeight() - 10) )
		{
			setDisplayChildren(!mDisplayChildren);
			reshape(mRect.getWidth(), mRect.getHeight(), FALSE);
		}
	}
	return TRUE;
}

BOOL LLContainerView::handleMouseUp(S32 x, S32 y, MASK mask)
{
	if (mDisplayChildren)
	{
		LLView::childrenHandleMouseUp(x, y, mask);
	}
	return TRUE;
}

void LLContainerView::draw()
{
	if (!getVisible())
	{
		return;
	}

	{
		LLGLSNoTexture gls_no_texture;

		gl_rect_2d(0, mRect.getHeight(), mRect.getWidth(), 0, LLColor4(0.f, 0.f, 0.f, 0.25f));
	}
		
	// Draw the label.
	gResMgr->getRes( LLFONT_OCRA )->renderUTF8(mLabel, 0, 2, mRect.getHeight() - 2, LLColor4(1,1,1,1), LLFontGL::LEFT, LLFontGL::TOP);

	LLView::draw();
}

void LLContainerView::reshape(S32 width, S32 height, BOOL called_from_parent)
{
	// Determine the sizes and locations of all contained views
	U32 total_height = 0;
	U32 top, left, right, bottom;
	//LLView *childp;

	// These will be used for the children
	left = 4;
	top = mRect.getHeight() - 4;
	right = mRect.getWidth() - 2;
	bottom = top;
	
	// Leave some space for the top label/grab handle
	total_height += 20;

	if (mDisplayChildren)
	{
		// Determine total height
		U32 child_height = 0;
		for (child_list_const_iter_t child_iter = getChildList()->begin();
			 child_iter != getChildList()->end(); ++child_iter)
		{
			LLView *childp = *child_iter;
			if (!childp->getVisible())
			{
				llwarns << "Incorrect visibility!" << llendl;
			}
			LLRect child_rect = childp->getRequiredRect();
			child_height += child_rect.getHeight();
			child_height += 2;
		}
		total_height += child_height;
	}
	
	
	if (followsTop())
	{
		mRect.mBottom = mRect.mTop - total_height;
	}
	else
	{
		mRect.mTop = mRect.mBottom + total_height;
	}
	mRect.mRight = mRect.mLeft + width;

	top = total_height - 20;
	bottom = top;

	if (mDisplayChildren)
	{
		// Iterate through all children, and put in container from top down.
		for (child_list_const_iter_t child_iter = getChildList()->begin();
			 child_iter != getChildList()->end(); ++child_iter)
		{
			LLView *childp = *child_iter;
			LLRect child_rect = childp->getRequiredRect();
			bottom -= child_rect.getHeight();
			LLRect r(left, bottom + child_rect.getHeight(), right, bottom);
			childp->setRect(r);
			childp->reshape(right - left, top - bottom);
			top = bottom - 2;
			bottom = top;
		}
	}
	
	if (!called_from_parent)
	{
		if (getParent())
		{
			getParent()->reshape(getParent()->getRect().getWidth(), getParent()->getRect().getHeight(), FALSE);
		}
	}
}

LLRect LLContainerView::getRequiredRect()
{
	LLRect req_rect;
	//LLView *childp;
	U32 total_height = 0;
	
	// Determine the sizes and locations of all contained views

	// Leave some space for the top label/grab handle

	if (mDisplayChildren)
	{
		total_height = 20;
		
		// Determine total height
		U32 child_height = 0;
		for (child_list_const_iter_t child_iter = getChildList()->begin();
			 child_iter != getChildList()->end(); ++child_iter)
		{
			LLView *childp = *child_iter;
			LLRect child_rect = childp->getRequiredRect();
			child_height += child_rect.getHeight();
			child_height += 2;
		}

		total_height += child_height;
	}
	else
	{
		total_height = 20;
	}

	req_rect.mTop = total_height;
	return req_rect;
}

void LLContainerView::setLabel(const LLString& label)
{
	mLabel = label;
}

void LLContainerView::setDisplayChildren(const BOOL displayChildren)
{
	mDisplayChildren = displayChildren;
	for (child_list_const_iter_t child_iter = getChildList()->begin();
		 child_iter != getChildList()->end(); ++child_iter)
	{
		LLView *childp = *child_iter;
		childp->setVisible(mDisplayChildren);
	}
}
