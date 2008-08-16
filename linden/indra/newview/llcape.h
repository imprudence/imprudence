/** 
 * @file llcape.h
 * @brief LLVOCloth class definition
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

#ifndef LLCAPE_H
#define LLCAPE_H

#include "llviewerobject.h"

class LLVolumeImplFlexible;

//-----------------------------------------------------------------------------------
// Default setting for the attributes of a cloth object...
//-----------------------------------------------------------------------------------
const F32		CLOTH_MIN_LENGTH				= 0.2f;
const F32		CLOTH_DEFAULT_LENGTH			= 1.5f;
const F32		CLOTH_MAX_LENGTH				= 2.0f;

const F32		CLOTH_MIN_TENSION				= 0.0f;
const F32		CLOTH_DEFAULT_TENSION			= 1.0f;
const F32		CLOTH_MAX_TENSION				= 10.0f;

const F32		CLOTH_MIN_GRAVITY				= FLEXIBLE_OBJECT_MIN_GRAVITY;
const F32		CLOTH_DEFAULT_GRAVITY			= 0.3f;
const F32		CLOTH_MAX_GRAVITY				= FLEXIBLE_OBJECT_MAX_GRAVITY;

const F32		CLOTH_MIN_PITCH					= -1.0f;
const F32		CLOTH_DEFAULT_PITCH				= 0.1f;
const F32		CLOTH_MAX_PITCH					= 1.0f;

const F32		CLOTH_MIN_AIR_FRICTION			= FLEXIBLE_OBJECT_MIN_AIR_FRICTION;
const F32		CLOTH_DEFAULT_AIR_FRICTION		= 6.0f;
const F32		CLOTH_MAX_AIR_FRICTION			= FLEXIBLE_OBJECT_MAX_AIR_FRICTION;

const F32		CLOTH_MIN_WIND_SENSITIVITY			= FLEXIBLE_OBJECT_MIN_WIND_SENSITIVITY;
const F32		CLOTH_DEFAULT_WIND_SENSITIVITY		= 0.0f;
const F32		CLOTH_MAX_WIND_SENSITIVITY			= FLEXIBLE_OBJECT_MAX_WIND_SENSITIVITY;

const F32		CLOTH_MIN_CIRCLE_WRAP_AMOUNT	= 0.0f;
const F32		CLOTH_DEFAULT_CIRCLE_WRAP_AMOUNT= 0.5f;
const F32		CLOTH_MAX_CIRCLE_WRAP_AMOUNT	= 1.0f;

const F32		CLOTH_MIN_WIDTH					= 0.01f;
const F32		CLOTH_DEFAULT_WIDTH				= 0.4f;
const F32		CLOTH_MAX_WIDTH					= 1.0f;

const int		CLOTH_MIN_STRANDS				= 2;
const int		CLOTH_DEFAULT_NUM_STRANDS		= 4;
const int		CLOTH_MAX_STRANDS				= 8;

const int		CLOTH_MIN_SEGMENTS				= 1;
const int		CLOTH_DEFAULT_NUM_SEGMENTS		= 4;
const int		CLOTH_MAX_SEGMENTS				= 8;

const int		CLOTH_MIN_TEXTURE_INDEX			= 1;
const int		CLOTH_DEFAULT_TEXTURE_INDEX		= 1;
const int		CLOTH_MAX_TEXTURE_INDEX			= 7;

const F32		CLOTH_MIN_AVATAR_COLLISION_SPHERE_RADIUS	= 0.0f;
const F32		CLOTH_DEFAULT_AVATAR_COLLISION_SPHERE_RADIUS= 0.1f;
const F32		CLOTH_MAX_AVATAR_COLLISION_SPHERE_RADIUS	= 3.0f;

const F32		CLOTH_MIN_AVATAR_COLLISION_SPHERE_RIGHT_OFFSET		= -2.0f;
const F32		CLOTH_DEFAULT_AVATAR_COLLISION_SPHERE_RIGHT_OFFSET	= 0.0f;
const F32		CLOTH_MAX_AVATAR_COLLISION_SPHERE_RIGHT_OFFSET		= 2.0f;

const F32		CLOTH_MIN_AVATAR_COLLISION_SPHERE_UP_OFFSET		= -2.0f;
const F32		CLOTH_DEFAULT_AVATAR_COLLISION_SPHERE_UP_OFFSET	= 0.0f;
const F32		CLOTH_MAX_AVATAR_COLLISION_SPHERE_UP_OFFSET		= 2.0f;

const F32		CLOTH_MIN_AVATAR_COLLISION_SPHERE_FORWARD_OFFSET		= -2.0f;
const F32		CLOTH_DEFAULT_AVATAR_COLLISION_SPHERE_FORWARD_OFFSET	= 0.0f;
const F32		CLOTH_MAX_AVATAR_COLLISION_SPHERE_FORWARD_OFFSET		= 2.0f;

const LLString	CLOTH_DEFAULT_TEXTURE_FILENAME				= "not_specified";
const bool		CLOTH_DEFAULT_VISUALIZE_STRANDS				= false;
const bool		CLOTH_DEFAULT_VISUALIZE_COLLISION_SPHERE	= false;

const bool		CLOTH_DEFAULT_USING_AVATAR_COLLISION_SPHERE				= false;
const LLVector3	CLOTH_DEFAULT_AVATAR_COLLISION_SPHERE_POSITION_OFFSET	= LLVector3::zero;


//-------------------------------------------------
// This structure is also used in the part of the 
// code that creates new cloth objects.
//-------------------------------------------------
struct LLClothAttributes
{
	int				mTextureIndex;
	LLString		mTextureFileName;
	bool			mVisualizeStrands;
	bool			mVisualizeAvCollisionSphere;
	int				mNumStrands;
	int				mNumSegments;
	F32				mWidth;
	F32				mPitch;
	F32				mLength;
	F32				mTension;
	F32				mGravity;
	F32				mAirFriction;	
	F32				mWindSensitivity;	
	F32				mCircleWrapAmount;
	bool			mUsingAvatarCollisionSphere;
	F32				mAvatarCollisionSphereRightOffset;
	F32				mAvatarCollisionSphereUpOffset;
	F32				mAvatarCollisionSphereForwardOffset;
	F32				mAvatarCollisionSphereRadius;



	//------ the constructor for the structure ------------
	LLClothAttributes()
	{
		mTextureFileName		= CLOTH_DEFAULT_TEXTURE_FILENAME;
		mVisualizeStrands		= CLOTH_DEFAULT_VISUALIZE_STRANDS;
		mNumStrands				= CLOTH_DEFAULT_NUM_STRANDS;
		mNumSegments			= CLOTH_DEFAULT_NUM_SEGMENTS;
		mWidth					= CLOTH_DEFAULT_WIDTH;
		mLength					= CLOTH_DEFAULT_LENGTH;
		mPitch					= CLOTH_DEFAULT_PITCH;
		mTension				= CLOTH_DEFAULT_TENSION;
		mGravity				= CLOTH_DEFAULT_GRAVITY;
		mAirFriction			= CLOTH_DEFAULT_AIR_FRICTION;
		mWindSensitivity		= CLOTH_DEFAULT_WIND_SENSITIVITY;
		mCircleWrapAmount		= CLOTH_DEFAULT_CIRCLE_WRAP_AMOUNT;
		mTextureIndex			= CLOTH_DEFAULT_TEXTURE_INDEX;
	}
};// end of attributes structure



//---------------------------------------------------------
// The LLVOCloth class 
//---------------------------------------------------------
class LLVOCloth : public LLViewerObject
{
	public:
		LLVOCloth(const LLUUID &id, const LLPCode type, LLViewerRegion *regionp);
		void	update();
		void	render();
		void	markAsDead();

		//----------------------------------------------
		// set stuff
		//----------------------------------------------
		void				setAttributes( LLClothAttributes );
		void				setNumStrands( int num ); 
		void				setNumSegments( int num ); 
		void				setLength( F32 length ); 
		void				setTension( F32 t ); 
		void				setGravity( F32 g ); 
		void				setCircleWrapAmount( F32 n ); 
		void				setAirFriction( F32 a ); 
		void				setWindSensitivity( F32 w ); 
		void				setPitch( F32 p ); 
		void				setWidth( F32 n ); 
		void				setVisualizingStrands( bool v ); 
		void				setVisualizingAvCollisionSphere( bool v ); 
		void				setAvatarCollisionSphereRadius( F32 r ); 
		void				setAvatarCollisionSphereRight( F32 r ); 
		void				setAvatarCollisionSphereUp( F32 u ); 
		void				setAvatarCollisionSphereForward( F32 f); 

		//----------------------------------------------
		// get stuff
		//----------------------------------------------
		bool				getVisualizingStrands() const; 
		bool				getVisualizingAvCollisionSphere() const; 
		int					getNumStrands();
		int					getNumSegments();
		F32					getLength();
		F32					getWidth();
		F32					getCircleWrapAmount();
		F32					getPitch();
		F32					getTension();
		F32					getGravity();
		F32					getAirFriction();
		F32					getWindSensitivity();
		F32					getAvatarCollisionSphereRadius(); 
		F32					getAvatarCollisionSphereRight(); 
		F32					getAvatarCollisionSphereUp(); 
		F32					getAvatarCollisionSphereForward(); 

	private:
		//--------------------------------------
		// private members
		//--------------------------------------
		LLClothAttributes	mAttributes;					// these determine the characteristics of the cloth
		LLVector3			mAvatarCollisionSpherePosition;	// derived from the avatar
		LLViewerObject*			mDummyVO [ CLOTH_MAX_STRANDS ];
		LLVolumeImplFlexible*	mStrand	[ CLOTH_MAX_STRANDS ];	// the array of strands that make up the cloth


		//--------------------------------------
		// private methods
		//--------------------------------------
		void updateVirtualServer();
		void generateStrands();

};// end of class definition


#endif // LL_CLOTH_H
