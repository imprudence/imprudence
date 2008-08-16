/** 
 * @file lllocalanimationobject.cpp
 * @brief LLHUDLocalAnimationObject class implementation
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

#include "llviewerprecompiledheaders.h"

#include "lllocalanimationobject.h"
#include "llglheaders.h"
#include "llsphere.h"
#include "llrand.h"
#include "llviewerobject.h"

const F32 OPACITY		= 1.0f;
const int _X_			= 0;
const int _Y_			= 1;
const int _Z_			= 2;
const F32 BODY_RADIUS	= 0.2f;
const F32 HEAD_RADIUS	= 0.1f;
const F32 ONE_HALF		= 0.5f;

/*
//-----------------------------------------------
// constructor
//-----------------------------------------------
LLHUDLocalAnimationObject::LLHUDLocalAnimationObject()
: LLHUDObject( LL_HUD_LOCAL_ANIMATION_OBJECT )// cal the base class and pass this in, K?
{
	mParentObject = NULL;

	F32 windSensitivity = 0.04f;

	//----------------------------------------------
	// trunk
	//----------------------------------------------
	LLFlexibleObjectAttributes trunkAttributes;
	trunkAttributes.mAnchorPositionOffset.setVec( 0.0f, 0.0f, -0.2f );
	trunkAttributes.mAnchorDirection.setVec( 0.0f, 0.0f, 1.0f );
	trunkAttributes.mColor = LLColor4( 0.4f, 0.3f, 0.2f, 1.0f );
	trunkAttributes.mAnchorDirection.normVec();
	trunkAttributes.mAnchorRadius		= 0.3f;
	trunkAttributes.mNumSections		= 3;
	trunkAttributes.mGravity			= 1.2f;
	trunkAttributes.mLength				= 9.0f;
	trunkAttributes.mTension			= 10.0f;
	trunkAttributes.mEndRadius			= 0.2f;
	trunkAttributes.mTextureFileName	= "JVTestTreeTrunk.tga";
	trunkAttributes.mRenderStyle		= "tube";

	mTrunk.setAttributes( trunkAttributes );



	//---------------------------------------------
	// frond 1
	//---------------------------------------------
	LLFlexibleObjectAttributes frond1Attributes;

	frond1Attributes.mAnchorPositionOffset.setVec( 0.0f, 0.0f, 0.0f );
	frond1Attributes.mAnchorDirection.setVec( 1.0f, 0.0f, 1.0f );
	frond1Attributes.mAnchorDirection.normVec();
	frond1Attributes.mColor				= LLColor4( 0.15f, 0.25f, 0.0f, 1.0f );
	frond1Attributes.mAnchorRadius		= 0.5f;
	frond1Attributes.mTension			= 2.0f;
	frond1Attributes.mNumSections		= 2;
	frond1Attributes.mGravity			= 2.0f;
	frond1Attributes.mLength			= 5.0f;
	frond1Attributes.mEndRadius			= 0.0f;
	frond1Attributes.mTextureFileName	= "JVTestPalmFrond.tga";
	frond1Attributes.mRenderStyle		= "billboard_tube";
	frond1Attributes.mWindSensitivity	= windSensitivity;

	mFrond1.setAttributes( frond1Attributes );

	//---------------------------------------------
	// frond 2
	//---------------------------------------------
	LLFlexibleObjectAttributes frond2Attributes;

	frond2Attributes.mAnchorPositionOffset.setVec( 0.0f, 0.0f, 0.0f );
	frond2Attributes.mAnchorDirection.setVec( 1.0f, 0.7f, 0.7f );
	frond2Attributes.mAnchorDirection.normVec();
	frond2Attributes.mColor				= LLColor4( 0.15f, 0.25f, 0.0f, 1.0f );
	frond2Attributes.mAnchorRadius		= 0.5f;
	frond2Attributes.mTension			= 2.0f;
	frond2Attributes.mNumSections		= 2;
	frond2Attributes.mGravity			= 2.0f;
	frond2Attributes.mLength			= 6.0f;
	frond2Attributes.mEndRadius			= 0.0f;
	frond2Attributes.mTextureFileName	= "JVTestPalmFrond.tga";
	frond2Attributes.mRenderStyle		= "billboard_tube";
	frond2Attributes.mWindSensitivity	= windSensitivity;

	mFrond2.setAttributes( frond2Attributes );

	//---------------------------------------------
	// frond 3
	//---------------------------------------------
	LLFlexibleObjectAttributes frond3Attributes;

	frond3Attributes.mAnchorPositionOffset.setVec( 0.0f, 0.0f, 0.0f );
	frond3Attributes.mAnchorDirection.setVec( 1.0f, 0.7f, -0.7f );
	frond3Attributes.mAnchorDirection.normVec();
	frond3Attributes.mColor				= LLColor4( 0.15f, 0.25f, 0.0f, 1.0f );
	frond3Attributes.mAnchorRadius		= 0.3f;
	frond3Attributes.mTension			= 2.0f;
	frond3Attributes.mNumSections		= 2;
	frond3Attributes.mGravity			= 2.0f;
	frond3Attributes.mLength			= 6.0f;
	frond3Attributes.mEndRadius			= 0.0f;
	frond3Attributes.mTextureFileName	= "JVTestPalmFrond.tga";
	frond3Attributes.mRenderStyle		= "billboard_tube";
	frond3Attributes.mWindSensitivity	= windSensitivity;

	mFrond3.setAttributes( frond3Attributes );


	//---------------------------------------------
	// frond 4
	//---------------------------------------------
	LLFlexibleObjectAttributes frond4Attributes;

	frond4Attributes.mAnchorPositionOffset.setVec( 0.0f, 0.0f, 0.0f );
	frond4Attributes.mAnchorDirection.setVec( 1.0f, -0.8f, 0.3f );
	frond4Attributes.mAnchorDirection.normVec();
	frond4Attributes.mColor				= LLColor4( 0.15f, 0.25f, 0.0f, 1.0f );
	frond4Attributes.mAnchorRadius		= 0.5f;
	frond4Attributes.mTension			= 2.0f;
	frond4Attributes.mNumSections		= 2;
	frond4Attributes.mGravity			= 2.0f;
	frond4Attributes.mLength			= 6.0f;
	frond4Attributes.mEndRadius			= 0.0f;
	frond4Attributes.mTextureFileName	= "JVTestPalmFrond.tga";
	frond4Attributes.mRenderStyle		= "billboard_tube";
	frond4Attributes.mWindSensitivity	= windSensitivity;


	mFrond4.setAttributes( frond4Attributes );


	//---------------------------------------------
	// frond 5
	//---------------------------------------------
	LLFlexibleObjectAttributes frond5Attributes;

	frond5Attributes.mAnchorPositionOffset.setVec( 0.0f, 0.0f, 0.0f );
	frond5Attributes.mAnchorDirection.setVec( 1.0f, -0.8f, -0.3f );
	frond5Attributes.mAnchorDirection.normVec();
	frond5Attributes.mColor			= LLColor4( 0.15f, 0.25f, 0.0f, 1.0f );
	frond5Attributes.mAnchorRadius	= 0.5f;
	frond5Attributes.mTension		= 2.0f;
	frond5Attributes.mNumSections	= 2;
	frond5Attributes.mGravity		= 2.0f;
	frond5Attributes.mLength		= 5.0f;
	frond5Attributes.mEndRadius		= 0.0f;
	frond5Attributes.mTextureFileName = "JVTestPalmFrond.tga";
	frond5Attributes.mRenderStyle		= "billboard_tube";
	frond5Attributes.mWindSensitivity	= windSensitivity;

	mFrond5.setAttributes( frond5Attributes );

}//------------------------------------------------------------------------------



//---------------------------------------------------------------------------------
void LLHUDLocalAnimationObject::update()
{
	if ( ! mParentObject )
	{
		return;
	}

	//--------------------------------------------------------------------------------------------------
	// now, setting the trunk position and orientation, and updating it...
	//--------------------------------------------------------------------------------------------------
	mTrunk.setParentPositionAndRotationDirectly( mParentObject->getRenderPosition(), mParentObject->getRenderRotation() );
	mTrunk.update();


	//--------------------------------------------------------------------------------------------------
	// now, setting the frond positions and orientations, and updating them...
	//--------------------------------------------------------------------------------------------------
	mFrond1.setParentPositionAndRotationDirectly( mTrunk.getEndPosition(), mTrunk.getEndRotation() );
	mFrond2.setParentPositionAndRotationDirectly( mTrunk.getEndPosition(), mTrunk.getEndRotation() );
	mFrond3.setParentPositionAndRotationDirectly( mTrunk.getEndPosition(), mTrunk.getEndRotation() );
	mFrond4.setParentPositionAndRotationDirectly( mTrunk.getEndPosition(), mTrunk.getEndRotation() );
	mFrond5.setParentPositionAndRotationDirectly( mTrunk.getEndPosition(), mTrunk.getEndRotation() );
	
	mFrond1.update();
	mFrond2.update();
	mFrond3.update();
	mFrond4.update();
	mFrond5.update();

}//------------------------------------------------------------------







//------------------------------------------------------------------
void LLHUDLocalAnimationObject::render()
{
	mTrunk.render();

	mFrond1.render();
	mFrond2.render();
	mFrond3.render();
	mFrond4.render();
	mFrond5.render();

}//------------------------------------------------------------------





//------------------------------------------------------------------
void LLHUDLocalAnimationObject::setAttributes( LLLocalAnimationObjectAttributes a )
{
	//mAttributes = a; // ???? // how do I do this?

	//mAttributes.mAnchorPositionOffset	= a.mAnchorPositionOffset;
	//mAttributes.mAnchorDirection		= a.mAnchorDirection;
	//mAttributes.mAnchorRadius			= a.mAnchorRadius;	
	//mAttributes.mNumSections			= a.mNumSections;			
	//mAttributes.mLength					= a.mLength;			
	//mAttributes.mGravity				= a.mGravity;			
	//mAttributes.mAirFriction			= a.mAirFriction;		
	//mAttributes.mTension				= a.mTension;			
	//mAttributes.mRadiusChange			= a.mRadiusChange;	

}//------------------------------------------------------------------



//------------------------------------------------------------------
void LLHUDLocalAnimationObject::setParentObject( LLViewerObject * p )
{
	printf( "Setting parent of localAnimationObject controls %x to object %x \n", this, p );
	mParentObject = p;

}//------------------------------------------------------------------


//------------------------------------------------------------------
LLViewerObject * LLHUDLocalAnimationObject::getParentObject()
{
 return mParentObject;

}//------------------------------------------------------------------



//------------------------------------------------------------------
void LLHUDLocalAnimationObject::markAsDead()
{
	mDead = TRUE;
	mParentObject = NULL;

}//------------------------------------------------------------------

*/






