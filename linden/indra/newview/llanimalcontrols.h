/** 
 * @file llanimalcontrols.h
 * @brief LLAnimalControls class definition
 *
 * Copyright (c) 2005-2007, Linden Research, Inc.
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

#ifndef LL_LLANIMALCONTROLS_H
#define LL_LLANIMALCONTROLS_H

#include "llhudobject.h"
#include "llflexibleobject.h"

//-----------------------------------------------------------------------------------
// Default setting for the attributes of a animalControls object...
//-----------------------------------------------------------------------------------
//const LLVector3	ANIMAL_CONTROLS_DEFAULT_ANCHOR_DIRECTION		= LLVector3::z_axis;
//const LLVector3	ANIMAL_CONTROLS_DEFAULT_ANCHOR_POSITION_OFFSET	= LLVector3::zero;
//const LLColor4	ANIMAL_CONTROLS_DEFAULT_COLOR					= LLColor4( 1.0f, 1.0f, 1.0f, 1.0f );
//const F32		ANIMAL_CONTROLS_DEFAULT_GRAVITY					= 0.3f;
//const F32		ANIMAL_CONTROLS_DEFAULT_TENSION					= 10.0f;
//const F32		ANIMAL_CONTROLS_DEFAULT_AIR_FRICTION			= 10.0f;
//const F32		ANIMAL_CONTROLS_DEFAULT_LENGTH					= 1.0f;
//const int		ANIMAL_CONTROLS_DEFAULT_NUM_SECTIONS			= 4;
//const F32		ANIMAL_CONTROLS_DEFAULT_ANCHOR_RADIUS			= 0.05f;
//const F32		ANIMAL_CONTROLS_DEFAULT_RADIUS_CHANGE			= -0.01f;

class LLViewerObject;

/*struct LLAnimalControlsBodyBehavior
{
	LLVector3		mPosition;
	LLQuaternion	mHipRotation;
};

class LLAnimalControlsHeadBehavior
{
	private:
		LLVector3			mBodyPosition;
		LLQuaternion		mBodyRotation;
		LLHUDFlexibleObject	mFeather;
		LLVector3			mPosition;
		LLQuaternion		mRotation;
		LLVector3			mFocusPosition;
		bool				mEyesBlinking;
		F32					mEyeBlinkRate;

	public:
		LLAnimalControlsHeadBehavior();
		void setBodyPositionAndRotation( LLVector3 p, LLQuaternion r );
		void setFocusPosition( LLVector3 focusPosition );
		void update();
		void render();
};


class LLAnimalControlsLegBehavior
{
	private:
		LLVector3		mBodyPosition;
		LLQuaternion	mBodyRotation;
		LLVector3		mHipAnchorPosition; 
		bool			mWalking;
		bool			mIsLeft;

	public:
		LLAnimalControlsLegBehavior();
		void setBodyPositionAndRotation( LLVector3 p, LLQuaternion r );
		void update();
		void render();
};


class LLAnimalControlsVirtualServer
{
	private:
		int				fakeClock;
		LLVector3		focusPosition;
		bool			settingFocusPosition;
		LLVector3		animalPosition;
		LLQuaternion	animalRotation;

	public:
		LLAnimalControlsVirtualServer();
		bool		getSettingFocusPosition();
		LLVector3	getFocusPosition();
		void		update();
		void		setParentPositionAndRotation( LLVector3 p, LLQuaternion q );
};


//-------------------------------------------------
// This structure is also used in the part of the 
// code that creates new animalControls objects.
//-------------------------------------------------
struct LLAnimalControlsAttributes
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
	LLAnimalControlsAttributes()
	{
		//mAnchorPositionOffset	= ANIMAL_CONTROLS_DEFAULT_ANCHOR_DIRECTION;	
		//mAnchorDirection		= ANIMAL_CONTROLS_DEFAULT_ANCHOR_POSITION_OFFSET;
		//mAnchorRadius			= ANIMAL_CONTROLS_DEFAULT_ANCHOR_RADIUS;
		//mColor				= ANIMAL_CONTROLS_DEFAULT_COLOR;
		//mNumSections			= ANIMAL_CONTROLS_DEFAULT_NUM_SECTIONS;
		//mLength				= ANIMAL_CONTROLS_DEFAULT_LENGTH;
		//mGravity				= ANIMAL_CONTROLS_DEFAULT_GRAVITY;
		//mAirFriction			= ANIMAL_CONTROLS_DEFAULT_AIR_FRICTION;
		//mTension				= ANIMAL_CONTROLS_DEFAULT_TENSION;
		//mRadiusChange			= ANIMAL_CONTROLS_DEFAULT_RADIUS_CHANGE;	
	}
};// end of attributes structure



//---------------------------------------------------------
// The LLHUDAnimalControls class 
//---------------------------------------------------------
class LLHUDAnimalControls : public LLHUDObject
{
	public:
		LLHUDAnimalControls();
		void				setParentObject( LLViewerObject * );
		void				setAttributes( LLAnimalControlsAttributes );
		void				markAsDead();
		void				update();
		LLViewerObject *	getParentObject();
		void				render();

	private:
		//--------------------------------------
		// private members
		//--------------------------------------
		LLViewerObject*					mParentObject;
		LLAnimalControlsAttributes		mAttributes;
		LLAnimalControlsHeadBehavior	mHeadBehavior;
		LLAnimalControlsLegBehavior		mLeftLegBehavior;
		LLAnimalControlsLegBehavior		mRightLegBehavior;
		LLAnimalControlsVirtualServer	mVirtualServer;

		//--------------------------------------
		// private methods
		//--------------------------------------
		void updateVirtualServer();
		void updateBodyBehavior();
		void renderBodyBehavior();
		//void updateLegBehavior();
		//void renderLegBehavior();
		//void updateHeadBehavior();
		//void renderHeadBehavior();

		friend class LLHUDObject;

};// end of class definition

*/
#endif // LL_LLANIMALCONTROLS_H
