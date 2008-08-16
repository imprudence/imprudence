/** 
 * @file llanimalcontrols.cpp
 * @brief LLAnimalControl class implementation
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

#include "llviewerprecompiledheaders.h"

#include "llanimalcontrols.h"
#include "llglheaders.h"
#include "llsphere.h"
#include "llrand.h"
#include "llviewerobject.h"

/*
//const F32 ANIMAL_CONTROLS_MAX_TENSION_FORCE = 0.99f;// I'll explain in a minute...
//const F32 ANIMAL_CONTROLS_MINIMUM_RADIUS	= 0.005f;

const F32 OPACITY		= 1.0f;
const int _X_			= 0;
const int _Y_			= 1;
const int _Z_			= 2;
const F32 BODY_RADIUS	= 0.2f;
const F32 HEAD_RADIUS	= 0.1f;
const F32 ONE_HALF		= 0.5f;

//-----------------------------------------------
// constructor
//-----------------------------------------------
LLHUDAnimalControls::LLHUDAnimalControls()
: LLHUDObject( LL_HUD_ANIMAL_CONTROLS )// cal the base class and pass this in, K?
{
	mParentObject = NULL;

}//------------------------------------------------------------------------------




//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
// Virtual Server methods...
//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
LLAnimalControlsVirtualServer::LLAnimalControlsVirtualServer()
{
	focusPosition.setVec( 0.0f, 0.0f, 0.0f );
	animalPosition.setVec( 0.0f, 0.0f, 0.0f );
	animalRotation			= LLQuaternion::DEFAULT;
	fakeClock				= 0;
	settingFocusPosition	= false;

}//---------------------------------------------------------------------------------



//---------------------------------------------------------------------------------
void LLAnimalControlsVirtualServer::update()
{
	fakeClock ++;

	if ( settingFocusPosition )
	{
		settingFocusPosition = false;
	}

	if ( fakeClock == 20 )
	{
		fakeClock = 0;

		settingFocusPosition = true;

		LLMatrix3 bodyRotationMatrix = animalRotation.getMatrix3();

		LLVector3 forwardDirection	= bodyRotationMatrix.getLeftRow();
		LLVector3 leftDirection		= bodyRotationMatrix.getFwdRow();

		forwardDirection.mV[_Z_] = 0.0f;
		forwardDirection.normVec();
		
		focusPosition = animalPosition + forwardDirection * 5.0f;

		F32 leftRightSpan = 5.0f;

		F32 randomSpan = ll_frand( leftRightSpan );

		focusPosition += leftDirection * ( - leftRightSpan * ONE_HALF + randomSpan );
	}

}//------------------------------------------------------------------------------



//---------------------------------------------------------------------------------
bool LLAnimalControlsVirtualServer::getSettingFocusPosition()
{
	return settingFocusPosition;

}//------------------------------------------------------------------------------

//---------------------------------------------------------------------------------
LLVector3 LLAnimalControlsVirtualServer::getFocusPosition()
{
	return focusPosition;

}//------------------------------------------------------------------------------


//---------------------------------------------------------------------------------
void LLAnimalControlsVirtualServer::setParentPositionAndRotation( LLVector3 p, LLQuaternion r )
{
	animalPosition	= p;
	animalRotation	= r;

}//------------------------------------------------------------------------------

//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
// End of Virtual Server methods...
//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------




//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
// Head behavior methods...
//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
LLAnimalControlsHeadBehavior::LLAnimalControlsHeadBehavior()
{
	mPosition.setVec		( 0.0f, 0.0f, 0.0f );
	mFocusPosition.setVec	( 0.0f, 0.0f, 0.0f );
	mBodyPosition.setVec	( 0.0f, 0.0f, 0.0f );
	mRotation		= LLQuaternion::DEFAULT;
	mEyesBlinking	= true;
	mEyeBlinkRate	= 0.1f;
	mBodyRotation	= LLQuaternion::DEFAULT;

	LLFlexibleObjectAttributes headFeatherAttributes;
	headFeatherAttributes.mAnchorPositionOffset.setVec( 0.0f, 0.0f, 0.1f );
	headFeatherAttributes.mAnchorDirection.setVec( 0.0f, 1.0f, 1.0f );
	headFeatherAttributes.mAnchorDirection.normVec();
	headFeatherAttributes.mColor = LLColor4( 1.0f, 0.4f, 0.0f, 1.0f );
	headFeatherAttributes.mAnchorRadius = 0.01f;
	headFeatherAttributes.mLength = 0.4f;
	headFeatherAttributes.mEndRadius = 0.06f;
	headFeatherAttributes.mRenderStyle = "tube";

	mFeather.setAttributes( headFeatherAttributes );

}//------------------------------------------------------------------

//------------------------------------------------------------------
void LLAnimalControlsHeadBehavior::setBodyPositionAndRotation( LLVector3 p, LLQuaternion r )
{
	mBodyPosition = p;
	mBodyRotation = r;

}//------------------------------------------------------------------

//------------------------------------------------------------------
void LLAnimalControlsHeadBehavior::setFocusPosition( LLVector3 f )
{
	mFocusPosition = f;

}//------------------------------------------------------------------


//------------------------------------------------------------------
void LLAnimalControlsHeadBehavior::update()
{
	//--------------------------------------------------------------------------
	// Let's get the parent orientation and associated components
	//--------------------------------------------------------------------------
	LLQuaternion	parentOrientation		= mBodyRotation;
	LLMatrix3		parentMatrix			= parentOrientation.getMatrix3();
	LLVector3		parentForwardDirection	= parentMatrix.getLeftRow();
	LLVector3		parentUpwardDirection	= parentMatrix.getUpRow();

	//--------------------------------------------------------------------------
	// head position is based on this
	//--------------------------------------------------------------------------
	mPosition = mBodyPosition + parentForwardDirection * 0.5f + parentUpwardDirection * 0.5f;

	//--------------------------------------------------------------------------------------------------
	// let's figure out how the head would have to rotate in order to aim in the focus direction
	//--------------------------------------------------------------------------------------------------
	LLVector3 headFocusDirection = mFocusPosition - mPosition;
	LLQuaternion headRotationToFocusDirection;
	headRotationToFocusDirection.shortestArc( parentForwardDirection, headFocusDirection );

	//--------------------------------------------------------------------------------------------------
	// Now we rotate the head towards its focus direction
	//--------------------------------------------------------------------------------------------------
	mRotation = parentOrientation * headRotationToFocusDirection;

	//--------------------------------------------------------------------------------------------------
	// now, setting the head feather position and orientation, and updating it...
	//--------------------------------------------------------------------------------------------------
	mFeather.setParentPositionAndRotationDirectly( mPosition, mRotation );
	mFeather.update();

}//------------------------------------------------------------------

//------------------------------------------------------------------


{
	//--------------------------------------------------------------------------------------------------
	// let's get the head rotation components...
	//--------------------------------------------------------------------------------------------------
	LLMatrix3	headMatrix				= mRotation.getMatrix3();
	LLVector3	headForwardDirection	= headMatrix.getLeftRow();
	LLVector3	headUpwardDirection		= headMatrix.getUpRow();
	LLVector3	headLeftDirection		= headMatrix.getFwdRow();

	//------------------------------------------------------
	// show head ball
	//------------------------------------------------------
	glColor4fv( LLColor4( 1.0f, 1.0f, 1.0f, OPACITY ).mV );
	glPushMatrix();
	glTranslatef( mPosition.mV[_X_], mPosition.mV[_Y_], mPosition.mV[_Z_] );
	glScalef( HEAD_RADIUS, HEAD_RADIUS, HEAD_RADIUS );
	gSphere.render();
	glPopMatrix();

	//------------------------------------------------------
	// show head direction
	//------------------------------------------------------
	LLVector3 end = mPosition + headForwardDirection * 0.4f;
	glBegin( GL_LINES );
		glVertex3fv( mPosition.mV );
		glVertex3fv( end.mV );
	glEnd();


	//------------------------------------------------------
	// show eye behavior
	//------------------------------------------------------
	LLVector3 leftEyePosition	= mPosition + headForwardDirection * HEAD_RADIUS + headUpwardDirection * HEAD_RADIUS + headLeftDirection * HEAD_RADIUS * ONE_HALF;
	LLVector3 rightEyePosition	= mPosition + headForwardDirection * HEAD_RADIUS + headUpwardDirection * HEAD_RADIUS - headLeftDirection * HEAD_RADIUS * ONE_HALF;
	glColor4fv( LLColor4( 0.0f, 0.0f, 0.0f, 1.0f ).mV );

	glPushMatrix();
	glTranslatef( leftEyePosition.mV[_X_], leftEyePosition.mV[_Y_], leftEyePosition.mV[_Z_] );
	glScalef( 0.03f, 0.03f, 0.03f );
	gSphere.render();
	glPopMatrix();

	glPushMatrix();
	glTranslatef( rightEyePosition.mV[_X_], rightEyePosition.mV[_Y_], rightEyePosition.mV[_Z_] );
	glScalef( 0.03f, 0.03f, 0.03f );
	gSphere.render();
	glPopMatrix();

	//------------------------------------------------------------------
	// render feather
	//------------------------------------------------------------------
	mFeather.render();

}//------------------------------------------------------------------

//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
// End of head behavior methods...
//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------




//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
// Leg behavior methods...
//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
LLAnimalControlsLegBehavior::LLAnimalControlsLegBehavior()
{
	mBodyPosition.setVec		( 0.0f, 0.0f, 0.0f );
	mHipAnchorPosition.setVec	( 0.0f, 0.0f, 0.0f ); 
	mBodyRotation		= LLQuaternion::DEFAULT;
	mWalking			= false;
	mIsLeft				= true;

}//-------------------------------------------------------------


//------------------------------------------------------------------
void LLAnimalControlsLegBehavior::setBodyPositionAndRotation( LLVector3 p, LLQuaternion r )
{
	mBodyPosition = p;
	mBodyRotation = r;

}//------------------------------------------------------------------


//---------------------------------------------------------------------------------
void LLAnimalControlsLegBehavior::update()
{

}//-------------------------------------------------------------


//---------------------------------------------------------------------------------
void LLAnimalControlsLegBehavior::render()
{
}//-------------------------------------------------------------








//---------------------------------------------------------------------------------
void LLHUDAnimalControls::update()
{
	if ( ! mParentObject )
	{
		return;
	}

	//-----------------------------------------------------------------------------------
	// To help in development of this client-side functionality, 
	// we are pretending that the server is periodically sending us information,
	// such as focus position (what we want the animal head to look at)
	//-----------------------------------------------------------------------------------
	mVirtualServer.setParentPositionAndRotation( mParentObject->getRenderPosition(), mParentObject->getRenderRotation() ); // fake, hacky, temporary, just ignore this.
	mVirtualServer.update();

	// here is where we ask the virtual server stuff.
	if ( mVirtualServer.getSettingFocusPosition() )
	{
		mHeadBehavior.setFocusPosition( mVirtualServer.getFocusPosition() );
	}

	updateBodyBehavior();

	mHeadBehavior.setBodyPositionAndRotation	( mParentObject->getPosition(), mParentObject->getRenderRotation() );
	mLeftLegBehavior.setBodyPositionAndRotation	( mParentObject->getPosition(), mParentObject->getRenderRotation() );
	mRightLegBehavior.setBodyPositionAndRotation( mParentObject->getPosition(), mParentObject->getRenderRotation() );

	mHeadBehavior.update();
	mLeftLegBehavior.update();
	mRightLegBehavior.update();

}//------------------------------------------------------------------





//------------------------------------------------------------------
void LLHUDAnimalControls::updateBodyBehavior()
{

}//------------------------------------------------------------------




//------------------------------------------------------------------
void LLHUDAnimalControls::render()
{
	mHeadBehavior.render();
	mLeftLegBehavior.render();
	mRightLegBehavior.render();

}//------------------------------------------------------------------









//------------------------------------------------------------------
void LLHUDAnimalControls::setAttributes( LLAnimalControlsAttributes a )
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
void LLHUDAnimalControls::setParentObject( LLViewerObject * p )
{
	printf( "Setting parent of animal controls %x to object %x \n", this, p );
	mParentObject = p;

}//------------------------------------------------------------------


//------------------------------------------------------------------
LLViewerObject * LLHUDAnimalControls::getParentObject()
{
 return mParentObject;

}//------------------------------------------------------------------



//------------------------------------------------------------------
void LLHUDAnimalControls::markAsDead()
{
	mDead = TRUE;
	mParentObject = NULL;

}//------------------------------------------------------------------




*/



