/** 
 * @file lllocalanimationobject.h
 * @brief LLHUDLocalAnimationObject class definition
 *
 * Copyright (c) 2006-2007, Linden Research, Inc.
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

#ifndef LL_LOCALANIMATIONOBJECT_H
#define LL_LOCALANIMATIONOBJECT_H

#include "llhudobject.h"
#include "llflexibleobject.h"

//-----------------------------------------------------------------------------------
// Default setting for the attributes of a localAnimationObjectControls object...
//-----------------------------------------------------------------------------------
//const LLVector3	LOCAL_ANIMATION_OBJECT_DEFAULT_ANCHOR_DIRECTION		= LLVector3::z_axis;
//const LLVector3	LOCAL_ANIMATION_OBJECT_DEFAULT_ANCHOR_POSITION_OFFSET	= LLVector3::zero;
//const LLColor4	LOCAL_ANIMATION_OBJECT_DEFAULT_COLOR					= LLColor4( 1.0f, 1.0f, 1.0f, 1.0f );
//const F32		LOCAL_ANIMATION_OBJECT_DEFAULT_GRAVITY					= 0.3f;
//const F32		LOCAL_ANIMATION_OBJECT_DEFAULT_TENSION					= 10.0f;
//const F32		LOCAL_ANIMATION_OBJECT_DEFAULT_AIR_FRICTION			= 10.0f;
//const F32		LOCAL_ANIMATION_OBJECT_DEFAULT_LENGTH					= 1.0f;
//const int		LOCAL_ANIMATION_OBJECT_DEFAULT_NUM_SECTIONS			= 4;
//const F32		LOCAL_ANIMATION_OBJECT_DEFAULT_ANCHOR_RADIUS			= 0.05f;
//const F32		LOCAL_ANIMATION_OBJECT_DEFAULT_RADIUS_CHANGE			= -0.01f;

class LLViewerObject;
/*

//-------------------------------------------------
// This structure is also used in the part of the 
// code that creates new localAnimationObjectControls objects.
//-------------------------------------------------
struct LLLocalAnimationObjectAttributes
{
	//LLVector3		mAnchorPositionOffset;
	//LLVector3		mAnchorDirection;
	//LLColor4		mColor;
	//F32				mAnchorRadius;
	//S32				mNumSections;
	//F32				mLength;
	//F32				mGravity;
	//F32				mAirFriction;
	//F32				mTension;
	//F32				mRadiusChange;	
	bool			mUsingBodyControls;
	bool			mUsingHeadControls;
	bool			mUsingTailControls;
	bool			mUsingLegControls;

	//------ the constructor for the structure ------------
	LLLocalAnimationObjectAttributes()
	{
		//mAnchorPositionOffset	= LOCAL_ANIMATION_OBJECT_DEFAULT_ANCHOR_DIRECTION;	
		//mAnchorDirection		= LOCAL_ANIMATION_OBJECT_DEFAULT_ANCHOR_POSITION_OFFSET;
		//mAnchorRadius			= LOCAL_ANIMATION_OBJECT_DEFAULT_ANCHOR_RADIUS;
		//mColor				= LOCAL_ANIMATION_OBJECT_DEFAULT_COLOR;
		//mNumSections			= LOCAL_ANIMATION_OBJECT_DEFAULT_NUM_SECTIONS;
		//mLength				= LOCAL_ANIMATION_OBJECT_DEFAULT_LENGTH;
		//mGravity				= LOCAL_ANIMATION_OBJECT_DEFAULT_GRAVITY;
		//mAirFriction			= LOCAL_ANIMATION_OBJECT_DEFAULT_AIR_FRICTION;
		//mTension				= LOCAL_ANIMATION_OBJECT_DEFAULT_TENSION;
		//mRadiusChange			= LOCAL_ANIMATION_OBJECT_DEFAULT_RADIUS_CHANGE;	
	}
};// end of attributes structure



//---------------------------------------------------------
// The LLHUDLocalAnimationObject class 
//---------------------------------------------------------
class LLHUDLocalAnimationObject : public LLHUDObject
{
	public:
		LLHUDLocalAnimationObject();
		void				setParentObject( LLViewerObject * );
		void				setAttributes( LLLocalAnimationObjectAttributes );
		void				markAsDead();
		void				update();
		LLViewerObject *	getParentObject();
		void				render();

	private:
		//--------------------------------------
		// private members
		//--------------------------------------
		LLViewerObject*						mParentObject;
		LLLocalAnimationObjectAttributes	mAttributes;
		LLHUDFlexibleObject					mTrunk;
		LLHUDFlexibleObject					mFrond1;
		LLHUDFlexibleObject					mFrond2;
		LLHUDFlexibleObject					mFrond3;
		LLHUDFlexibleObject					mFrond4;
		LLHUDFlexibleObject					mFrond5;

		//--------------------------------------
		// private methods
		//--------------------------------------
		void updateVirtualServer();

		friend class LLHUDObject;

};// end of class definition
*/

#endif // LL_LOCALANIMATIONOBJECT_H
