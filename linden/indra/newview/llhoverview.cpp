/** 
 * @file llhoverview.cpp
 * @brief LLHoverView class implementation
 *
 * $LicenseInfo:firstyear=2001&license=viewergpl$
 * 
 * Copyright (c) 2001-2008, Linden Research, Inc.
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
 * online at http://secondlifegrid.net/programs/open_source/licensing/flossexception
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

#include "llviewerprecompiledheaders.h"

// self include
#include "llhoverview.h"

// Library includes
#include "llfontgl.h"
#include "message.h"
#include "llgl.h"
#include "llglimmediate.h"
#include "llfontgl.h"
#include "llparcel.h"
#include "lldbstrings.h"
#include "llclickaction.h"

// Viewer includes
#include "llagent.h"
#include "llcachename.h"
#include "llviewercontrol.h"
#include "lldrawable.h"
#include "llpermissions.h"
#include "llresmgr.h"
#include "llselectmgr.h"
#include "lltoolmgr.h"
#include "lltoolpie.h"
#include "lltoolselectland.h"
#include "llui.h"
#include "llviewercamera.h"
#include "llviewerobject.h"
#include "llviewerobjectlist.h"
#include "llviewerparcelmgr.h"
#include "llviewerregion.h"
#include "llviewerwindow.h"
#include "llglheaders.h"
#include "llviewerimagelist.h"
//#include "lltoolobjpicker.h"
#include "llhudmanager.h"	// HACK for creating flex obj's

#include "llhudmanager.h" // For testing effects
#include "llhudeffect.h"

//
// Constants
//
const char* DEFAULT_DESC = "(No Description)";
const F32 DELAY_BEFORE_SHOW_TIP = 0.35f;

//
// Local globals
//

LLHoverView *gHoverView = NULL;

//
// Static member functions
//
BOOL LLHoverView::sShowHoverTips = TRUE;

//
// Member functions
//

LLHoverView::LLHoverView(const std::string& name, const LLRect& rect)
:	LLView(name, rect, FALSE)
{
	mDoneHoverPick = FALSE;
	mStartHoverPickTimer = FALSE;
	mHoverActive = FALSE;
	mUseHover = TRUE;
	mTyping = FALSE;
	mHoverOffset.clearVec();
}

LLHoverView::~LLHoverView()
{
}

EWidgetType LLHoverView::getWidgetType() const
{
	return WIDGET_TYPE_HOVER_VIEW;
}

LLString LLHoverView::getWidgetTag() const
{
	return LL_HOVER_VIEW_TAG;
}

void LLHoverView::updateHover(LLTool* current_tool)
{
	BOOL picking_tool = (	current_tool == gToolPie 
							|| current_tool == gToolParcel );

	mUseHover = !gAgent.cameraMouselook() 
				&& picking_tool 
				&& !mTyping;
	if (mUseHover)
	{
		if ((gViewerWindow->getMouseVelocityStat()->getPrev(0) < 0.01f)
			&& (gCamera->getAngularVelocityStat()->getPrev(0) < 0.01f)
			&& (gCamera->getVelocityStat()->getPrev(0) < 0.01f))
		{
			if (!mStartHoverPickTimer)
			{
				mStartHoverTimer.reset();
				mStartHoverPickTimer = TRUE;
				//  Clear the existing text so that we do not briefly show the wrong data.
				mText.clear();
			}

			if (mDoneHoverPick)
			{
				// Just update the hover data
				updateText();
			}
			else if (mStartHoverTimer.getElapsedTimeF32() > DELAY_BEFORE_SHOW_TIP)
			{
				gViewerWindow->hitObjectOrLandGlobalAsync(gViewerWindow->getCurrentMouseX(),
													 gViewerWindow->getCurrentMouseY(), 0, pickCallback );
			}
		}
		else
		{
			cancelHover();
		}
	}

}

void LLHoverView::pickCallback(S32 x, S32 y, MASK mask)
{
	LLViewerObject* hit_obj = gViewerWindow->lastObjectHit();

	if (hit_obj)
	{
		gHoverView->setHoverActive(TRUE);
		gSelectMgr->setHoverObject(hit_obj);
		gHoverView->mLastHoverObject = hit_obj;
		gHoverView->mHoverOffset = gViewerWindow->lastObjectHitOffset();
	}
	else
	{
		gHoverView->mLastHoverObject = NULL;
	}

	// We didn't hit an object, but we did hit land.
	if (!hit_obj && gLastHitPosGlobal != LLVector3d::zero)
	{
		gHoverView->setHoverActive(TRUE);
		gHoverView->mHoverLandGlobal = gLastHitPosGlobal;
		gParcelMgr->requestHoverParcelProperties( gHoverView->mHoverLandGlobal );
	}
	else
	{
		gHoverView->mHoverLandGlobal = LLVector3d::zero;
	}

	gHoverView->mDoneHoverPick = TRUE;
}

void LLHoverView::setTyping(BOOL b)
{
	mTyping = b;
}


void LLHoverView::cancelHover()
{
	mStartHoverTimer.reset();
	mDoneHoverPick = FALSE;
	mStartHoverPickTimer = FALSE;

	gSelectMgr->setHoverObject(NULL);
	// Can't do this, some code relies on hover object still being
	// set after the hover is cancelled!  Dammit.  JC
	// mLastHoverObject = NULL;

	setHoverActive(FALSE);
}

void LLHoverView::resetLastHoverObject()
{
	mLastHoverObject = NULL;
}

void LLHoverView::updateText()
{
	LLViewerObject* hit_object = getLastHoverObject();
	std::string line;

	mText.clear();
	if ( hit_object )
	{
		if ( hit_object->isHUDAttachment() )
		{
			// no hover tips for HUD elements, since they can obscure
			// what the HUD is displaying
			return;
		}

		if ( hit_object->isAttachment() )
		{
			// get root of attachment then parent, which is avatar
			LLViewerObject* root_edit = hit_object->getRootEdit();
			if (!root_edit)
			{
				// Strange parenting issue, don't show any text
				return;
			}
			hit_object = (LLViewerObject*)root_edit->getParent();
			if (!hit_object)
			{
				// another strange parenting issue, bail out
				return;
			}
		}

		line.clear();
		if (hit_object->isAvatar())
		{
			LLNameValue* title = hit_object->getNVPair("Title");
			LLNameValue* firstname = hit_object->getNVPair("FirstName");
			LLNameValue* lastname =  hit_object->getNVPair("LastName");
			if (firstname && lastname)
			{
				if (title)
				{
					line.append(title->getString());
					line.append(1, ' ');
				}
				line.append(firstname->getString());
				line.append(1, ' ');
				line.append(lastname->getString());
			}
			else
			{
				line.append("Person");
			}
			mText.push_back(line);
		}
		else
		{
			//
			//  We have hit a regular object (not an avatar or attachment)
			// 

			//
			//  Default prefs will suppress display unless the object is interactive
			//
			BOOL suppressObjectHoverDisplay = !gSavedSettings.getBOOL("ShowAllObjectHoverTip");			
			
			LLSelectNode *nodep = gSelectMgr->getHoverNode();;
			if (nodep)
			{
				line.clear();
				if (nodep->mName.empty())
				{
					line.append("(no name)");
				}
				else
				{
					line.append( nodep->mName );
				}
				mText.push_back(line);

				if (!nodep->mDescription.empty()
					&& nodep->mDescription != DEFAULT_DESC)
				{
					mText.push_back( nodep->mDescription );
				}

				// Line: "Owner: James Linden"
				line.clear();
				line.append("Owner: ");

				if (nodep->mValid)
				{
					LLUUID owner;
					std::string name;
					if (!nodep->mPermissions->isGroupOwned())
					{
						owner = nodep->mPermissions->getOwner();
						if (LLUUID::null == owner)
						{
							line.append("Public");
						}
						else if(gCacheName->getFullName(owner, name))
						{
							line.append(name);
						}
						else
						{
							line.append("Retrieving...");
						}
					}
					else
					{
						std::string name;
						owner = nodep->mPermissions->getGroup();
						if (gCacheName->getGroupName(owner, name))
						{
							line.append(name);
							line.append("(Group)");
						}
						else
						{
							line.append("Retrieving...");
						}
					}
				}
				else
				{
					line.append("Retrieving...");
				}
				mText.push_back(line);

				// Build a line describing any special properties of this object.
				LLViewerObject *object = hit_object;
				LLViewerObject *parent = (LLViewerObject *)object->getParent();

				if (object &&
					(object->usePhysics() ||
					 object->flagScripted() || 
					 object->flagHandleTouch() || (parent && parent->flagHandleTouch()) ||
					 object->flagTakesMoney() || (parent && parent->flagTakesMoney()) ||
					 object->flagAllowInventoryAdd() ||
					 object->flagTemporary() ||
					 object->flagPhantom()) )
				{
					line.clear();
					if (object->flagScripted())
					{
						line.append("Script ");
					}

					if (object->usePhysics())
					{
						line.append("Physics ");
					}

					if (object->flagHandleTouch() || (parent && parent->flagHandleTouch()) )
					{
						line.append("Touch ");
						suppressObjectHoverDisplay = FALSE;		//  Show tip
					}

					if (object->flagTakesMoney() || (parent && parent->flagTakesMoney()) )
					{
						line.append("L$ ");
						suppressObjectHoverDisplay = FALSE;		//  Show tip
					}

					if (object->flagAllowInventoryAdd())
					{
						line.append("Drop Inventory ");
						suppressObjectHoverDisplay = FALSE;		//  Show tip
					}

					if (object->flagPhantom())
					{
						line.append("Phantom ");
					}

					if (object->flagTemporary())
					{
						line.append("Temporary ");
					}

					if (object->usePhysics() || 
						object->flagHandleTouch() ||
						(parent && parent->flagHandleTouch()) )
					{
						line.append("(Right-click for menu) ");
					}
					mText.push_back(line);
				}

				// Free to copy / For Sale: L$
				line.clear();
				if (nodep->mValid)
				{
					BOOL for_copy = nodep->mPermissions->getMaskEveryone() & PERM_COPY && object->permCopy();
					BOOL for_sale = nodep->mSaleInfo.isForSale() &&
									nodep->mPermissions->getMaskOwner() & PERM_TRANSFER &&
									(nodep->mPermissions->getMaskOwner() & PERM_COPY ||
									 nodep->mSaleInfo.getSaleType() != LLSaleInfo::FS_COPY);
					if (for_copy)
					{
						line.append("Free to copy");
						suppressObjectHoverDisplay = FALSE;		//  Show tip
					}
					else if (for_sale)
					{
						line.append(llformat("For Sale: L$%d", nodep->mSaleInfo.getSalePrice()));
						suppressObjectHoverDisplay = FALSE;		//  Show tip
					}
					else
					{
						// Nothing if not for sale
						// line.append("Not for sale");
					}
				}
				else
				{
					line.append("For Sale: Retrieving...");
				}
				mText.push_back(line);
			}
			
			//  If the hover tip shouldn't be shown, delete all the object text
			if (suppressObjectHoverDisplay)
			{
				mText.clear();
			}
		}
	}
	else if ( mHoverLandGlobal != LLVector3d::zero )
	{
		// 
		//  Do not show hover for land unless prefs are set to allow it.
		// 
		
		if (!gSavedSettings.getBOOL("ShowLandHoverTip")) return; 

		// Didn't hit an object, but since we have a land point we
		// must be hovering over land.

		LLParcel* hover_parcel = gParcelMgr->getHoverParcel();
		LLUUID owner;
		S32 width = 0;
		S32 height = 0;

		if ( hover_parcel )
		{
			owner = hover_parcel->getOwnerID();
			width = S32(gParcelMgr->getHoverParcelWidth());
			height = S32(gParcelMgr->getHoverParcelHeight());
		}

		// Line: "Land"
		line.clear();
		line.append("Land: ");
		if (hover_parcel)
		{
			line.append(hover_parcel->getName());
		}
		mText.push_back(line);

		// Line: "Owner: James Linden"
		line.clear();
		line.append("Owner: ");

		if ( hover_parcel )
		{
			std::string name;
			if (LLUUID::null == owner)
			{
				line.append("Public");
			}
			else if (hover_parcel->getIsGroupOwned())
			{
				if (gCacheName->getGroupName(owner, name))
				{
					line.append(name);
					line.append("(Group)");
				}
				else
				{
					line.append("Retrieving...");
				}
			}
			else if(gCacheName->getFullName(owner, name))
			{
				line.append(name);
			}
			else
			{
				line.append("Retrieving...");
			}
		}
		else
		{
			line.append("Retrieving...");
		}
		mText.push_back(line);

		// Line: "no fly, not safe, no build"

		// Don't display properties for your land.  This is just
		// confusing, because you can do anything on your own land.
		if ( hover_parcel && owner != gAgent.getID() )
		{
			S32 words = 0;
			
			line.clear();
			// JC - Keep this in the same order as the checkboxes
			// on the land info panel
			if ( !hover_parcel->getAllowModify() )
			{
				if ( hover_parcel->getAllowGroupModify() )
				{
					line.append("Group Build");
				}
				else
				{
					line.append("No Build");
				}
					
				words++;
			}

			if ( !hover_parcel->getAllowTerraform() )
			{
				if (words) line.append(", ");
				line.append("No Edit");
				words++;
			}

			if ( hover_parcel->getAllowDamage() )
			{
				if (words) line.append(", ");
				line.append("Not Safe");
				words++;
			}

			// Maybe we should reflect the estate's block fly bit here as well?  DK 12/1/04
			if ( !hover_parcel->getAllowFly() )
			{
				if (words) line.append(", ");
				line.append("No Fly");
				words++;
			}

			if ( !hover_parcel->getAllowOtherScripts() )
			{
				if (words) line.append(", ");
				if ( hover_parcel->getAllowGroupScripts() )
				{
					line.append("Group Scripts");
				}
				else
				{
					line.append("No Scripts");
				}
				
				words++;
			}

			if (words) 
			{
				mText.push_back(line);
			}
		}

		// Line: "Size: 1x4"
		// Only show for non-public land
		/*
		if ( hover_parcel && LLUUID::null != owner)
		{
			line = llformat("Size: %dx%d", width, height );
			mText.push_back(line);
		}
		*/
		if (hover_parcel && hover_parcel->getParcelFlag(PF_FOR_SALE))
		{
			line = llformat("For Sale: L$%d", hover_parcel->getSalePrice() );
			mText.push_back(line);
		}
	}
}


void LLHoverView::draw()
{
	if( !getVisible() )
	{
		return;
	}

	if ( !isHovering() )
	{
		return;
	}

	// To toggle off hover tips, you have to just suppress the draw.
	// The picking is still needed to do cursor changes over physical
	// and scripted objects.  JC
	if (!sShowHoverTips) return;

	const F32 MAX_HOVER_DISPLAY_SECS = 5.f;
	if (mHoverTimer.getElapsedTimeF32() > MAX_HOVER_DISPLAY_SECS)
	{
		return;
	}

	const F32 MAX_ALPHA = 0.9f;
	//const F32 STEADY_ALPHA = 0.3f;
	F32 alpha;
	if (mHoverActive)
	{
		alpha = 1.f;

		if (isHoveringObject())
		{
			// look at object
			LLViewerObject *hover_object = getLastHoverObject();
			if (hover_object->isAvatar())
			{
				gAgent.setLookAt(LOOKAT_TARGET_HOVER, getLastHoverObject(), LLVector3::zero);
			}
			else
			{
				LLVector3 local_offset((F32)mHoverOffset.mdV[VX], (F32)mHoverOffset.mdV[VY], (F32)mHoverOffset.mdV[VZ]);
				gAgent.setLookAt(LOOKAT_TARGET_HOVER, getLastHoverObject(), local_offset);
			}
		}
	}
	else
	{
		alpha = llmax(0.f, MAX_ALPHA - mHoverTimer.getElapsedTimeF32()*2.f);
	}

	// Bail out if no text to display
	if (mText.empty())
	{
		return;
	}

	// Don't draw if no alpha
	if (alpha <= 0.f)
	{
		return;
	}

	LLViewerImage* box_imagep = gImageList.getImage(LLUUID(gViewerArt.getString("rounded_square.tga")), MIPMAP_FALSE, TRUE);
	LLViewerImage* shadow_imagep = gImageList.getImage(LLUUID(gViewerArt.getString("rounded_square_soft.tga")), MIPMAP_FALSE, TRUE);

	const LLFontGL* fontp = gResMgr->getRes(LLFONT_SANSSERIF_SMALL);

	// Render text.
	LLColor4 text_color = gColors.getColor("ToolTipTextColor");
	// LLColor4 border_color = gColors.getColor("ToolTipBorderColor");
	LLColor4 bg_color = gColors.getColor("ToolTipBgColor");
	LLColor4 shadow_color = gColors.getColor("ColorDropShadow");

	// Could decrease the alpha here. JC
	//text_color.mV[VALPHA] = alpha;
	//border_color.mV[VALPHA] = alpha;
	//bg_color.mV[VALPHA] = alpha;

	S32 max_width = 0;
	S32 num_lines = mText.size();
	for (text_list_t::iterator iter = mText.begin(); iter != mText.end(); ++iter)
	{
		max_width = llmax(max_width, (S32)fontp->getWidth(*iter));
	}

	S32 left	= mHoverPos.mX + 10;
	S32 top		= mHoverPos.mY - 16;
	S32 right	= mHoverPos.mX + max_width + 30;
	S32 bottom	= mHoverPos.mY - 24 - llfloor(num_lines*fontp->getLineHeight());

	// Push down if there's a one-click icon
	if (mHoverActive
		&& isHoveringObject()
		&& mLastHoverObject->getClickAction() != CLICK_ACTION_NONE)
	{
		const S32 CLICK_OFFSET = 10;
		top -= CLICK_OFFSET;
		bottom -= CLICK_OFFSET;
	}

	// Make sure the rect is completely visible
	LLRect old_rect = getRect();
	setRect( LLRect(left, top, right, bottom ) );
	translateIntoRect( gViewerWindow->getVirtualWindowRect(), FALSE );
	left = getRect().mLeft;
	top = getRect().mTop;
	right = getRect().mRight;
	bottom = getRect().mBottom;
	setRect(old_rect);

	LLGLSUIDefault gls_ui;

	shadow_color.mV[VALPHA] = 0.7f * alpha;
	S32 shadow_offset = gSavedSettings.getS32("DropShadowTooltip");
	gGL.color4fv(shadow_color.mV);
	LLViewerImage::bindTexture(shadow_imagep);
	gl_segmented_rect_2d_tex(left + shadow_offset, top - shadow_offset, right + shadow_offset, bottom - shadow_offset, shadow_imagep->getWidth(), shadow_imagep->getHeight(), 16);

	bg_color.mV[VALPHA] = alpha;
	gGL.color4fv(bg_color.mV);
	LLViewerImage::bindTexture(box_imagep);
	gl_segmented_rect_2d_tex(left, top, right, bottom, box_imagep->getWidth(), box_imagep->getHeight(), 16);

	S32 cur_offset = top - 4;
	for (text_list_t::iterator iter = mText.begin(); iter != mText.end(); ++iter)
	{
		fontp->renderUTF8(*iter, 0, left + 10, cur_offset, text_color, LLFontGL::LEFT, LLFontGL::TOP);
		cur_offset -= llfloor(fontp->getLineHeight());
	}
}

void LLHoverView::setHoverActive(const BOOL active)
{
	if (active != mHoverActive)
	{
		mHoverTimer.reset();
	}

	mHoverActive = active;

	if (active)
	{
		mHoverPos = gViewerWindow->getCurrentMouse();
	}
}


BOOL LLHoverView::isHoveringLand() const
{
	return !mHoverLandGlobal.isExactlyZero();
}


BOOL LLHoverView::isHoveringObject() const
{
	return !mLastHoverObject.isNull() && !mLastHoverObject->isDead();
}


LLViewerObject* LLHoverView::getLastHoverObject() const
{
	if (!mLastHoverObject.isNull() && !mLastHoverObject->isDead())
	{
		return mLastHoverObject;
	}
	else
	{
		return NULL;
	}
}

// EOF
