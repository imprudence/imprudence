/** 
 * @file llcape.cpp
 * @brief LLVOCloth class implementation
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

#include "llcape.h"

#include "llglheaders.h"
#include "llsphere.h"
#include "llrand.h"
#include "llvoavatar.h"
#include "llagent.h"
#include "llviewerobject.h"
#include "llimagegl.h"
#include "llviewerimagelist.h"
#include "llviewercontrol.h"
#include "llflexibleobject.h"

const int _X_			= 0;
const int _Y_			= 1;
const int _Z_			= 2;
const F32 ONE_HALF		= 0.5f;
const F32 STRAND_VISUALIZATION_ANCHOR_BALL_RADIUS = 0.01f;

//-----------------------------------------------
// constructor
//-----------------------------------------------
LLVOCloth::LLVOCloth(const LLUUID &id, const LLPCode type, LLViewerRegion *regionp)
	: LLViewerObject(id, type, regionp)
{
	mAttributes.mTextureFileName						= CLOTH_DEFAULT_TEXTURE_FILENAME;
	mAttributes.mTextureIndex							= CLOTH_DEFAULT_TEXTURE_INDEX;
	mAttributes.mNumStrands								= CLOTH_DEFAULT_NUM_STRANDS;
	mAttributes.mNumSegments							= CLOTH_DEFAULT_NUM_SEGMENTS;
	mAttributes.mVisualizeStrands						= CLOTH_DEFAULT_VISUALIZE_STRANDS;
	mAttributes.mVisualizeAvCollisionSphere				= CLOTH_DEFAULT_VISUALIZE_COLLISION_SPHERE;
	mAttributes.mWidth									= CLOTH_DEFAULT_WIDTH;
	mAttributes.mLength									= CLOTH_DEFAULT_LENGTH;
	mAttributes.mPitch									= CLOTH_DEFAULT_PITCH;
	mAttributes.mGravity								= CLOTH_DEFAULT_GRAVITY;
	mAttributes.mTension								= CLOTH_DEFAULT_TENSION;
	mAttributes.mAirFriction							= CLOTH_DEFAULT_AIR_FRICTION;
	mAttributes.mWindSensitivity						= CLOTH_DEFAULT_WIND_SENSITIVITY;
	mAttributes.mCircleWrapAmount						= CLOTH_DEFAULT_CIRCLE_WRAP_AMOUNT;
	mAttributes.mUsingAvatarCollisionSphere				= CLOTH_DEFAULT_USING_AVATAR_COLLISION_SPHERE;		
	//mAttributes.mAvatarCollisionSpherePositionOffset	= CLOTH_DEFAULT_AVATAR_COLLISION_SPHERE_POSITION_OFFSET;
	mAttributes.mAvatarCollisionSphereRadius			= CLOTH_DEFAULT_AVATAR_COLLISION_SPHERE_RADIUS;
	mAttributes.mAvatarCollisionSphereRightOffset		= CLOTH_DEFAULT_AVATAR_COLLISION_SPHERE_RIGHT_OFFSET;
	mAttributes.mAvatarCollisionSphereUpOffset			= CLOTH_DEFAULT_AVATAR_COLLISION_SPHERE_UP_OFFSET;
	mAttributes.mAvatarCollisionSphereForwardOffset		= CLOTH_DEFAULT_AVATAR_COLLISION_SPHERE_FORWARD_OFFSET;

	generateStrands();

}//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// this method is only called when attributes of the cloth are set, 
// and so it is not so important to try optimized it. 
//------------------------------------------------------------------------------
void LLVOCloth::generateStrands()
{
	LLFlexibleObjectData strandAttributes;

	bool	oddNumStrands	= (bool)( mAttributes.mNumStrands % 2 );
	F32		piOver180		= F_PI / 180.0f;
	F32		x				= 0.0f;
	F32		y				= 0.0f;
	F32		startAngle		= -mAttributes.mCircleWrapAmount * 180.0f;
	F32		angleDelta		= ( mAttributes.mCircleWrapAmount * 360.0f )	/ (F32)( mAttributes.mNumStrands - 1 );
	F32		subWidth		=   mAttributes.mWidth / (F32)( mAttributes.mNumStrands - 1 );

	F32	angle = 0.0f;

	if ( mAttributes.mNumStrands > 2 ) 
	{
		angle = startAngle;
	}

	//-------------------------------------------
	// displace x and y to start the process of 
	// iterating through the positional offsets
	//-------------------------------------------
	if ( ! oddNumStrands )
	{
		y += subWidth * ONE_HALF;
	}

	int half = mAttributes.mNumStrands / 2;
	for ( int s=0; s<half; s ++)
	{
		if ( mAttributes.mNumStrands > 2 ) 
		{
			angle += angleDelta;
		}

		F32 r = angle * piOver180;
		x -= subWidth * sin(r);
		y -= subWidth * cos(r);
	}


	if ( mAttributes.mNumStrands > 2 )
		{
			angle = startAngle;
		}
	else
		{
			angle = 0.0f;
		}

		
	for ( int s=0; s<mAttributes.mNumStrands; s ++)
	{
		//strandAttributes.mAnchorPositionOffset.setVec( x, y, 0.0f );

		if ( mAttributes.mNumStrands > 2 )
		{
			angle += angleDelta;
		}

		F32 r = angle * piOver180;
		F32 dX = subWidth * sin(r);
		F32 dY = subWidth * cos(r);
		x += dX;
		y += dY;

		//--------------------------------------------------
		// set anchor directions
		//--------------------------------------------------
		LLVector3 perpendicular;
		perpendicular.setVec( -dY, dX, 0.0f );
		perpendicular.normVec();

		//F32 pitchRadian = mAttributes.mPitch * F_PI_BY_TWO;

		/*strandAttributes.mAnchorDirection.setVec
			( 
				perpendicular.mV[_X_] *	cos( pitchRadian ), 
				perpendicular.mV[_Y_] *	cos( pitchRadian ), 
										sin( pitchRadian ) 
			);*/

		//strandAttributes.mAnchorDirection.normVec(); // just to be sure...

		/*strandAttributes.mTension				= mAttributes.mTension;
		strandAttributes.mSimulateLOD			= mAttributes.mNumSegments;
		strandAttributes.mGravity				= mAttributes.mGravity;
		strandAttributes.mAirFriction			= mAttributes.mAirFriction;
		strandAttributes.mWindSensitivity		= mAttributes.mWindSensitivity;
		strandAttributes.mUsingCollisionSphere	= mAttributes.mUsingAvatarCollisionSphere;*/
	}

}//------------------------------------------------------------------------------






//---------------------------------------------------------------------------------
void LLVOCloth::update()
{
	if ( mAttributes.mUsingAvatarCollisionSphere )
	{
		LLVector3 offset;
		offset.setVec
			( 
				mAttributes.mAvatarCollisionSphereForwardOffset, // in this strange world, forward is along X (I think that's kinda wierd)
				mAttributes.mAvatarCollisionSphereRightOffset,
				mAttributes.mAvatarCollisionSphereUpOffset
			);

		//------------------------------------------------------
		// rotate into avatar space
		//------------------------------------------------------
		offset *= gAgent.getAvatarObject()->getRenderRotation();

		mAvatarCollisionSpherePosition = gAgent.getAvatarObject()->getRenderPosition() + offset;	
	}

	if ( mParent )
	{
		// update the attachment of the strands to their parent object, and let them flop around! 
		for ( int s=0; s<mAttributes.mNumStrands; s++)
		{
			mStrand[s]->setParentPositionAndRotationDirectly( getRenderPosition(), getRenderRotation() );
			//mStrand[s]->update();
			
			if ( mAttributes.mUsingAvatarCollisionSphere )
			{
				// it's not efficient to create the exact same collision 
				// sphere for every strand, but it'll do for the time being
				mStrand[s]->setCollisionSphere( mAvatarCollisionSpherePosition, mAttributes.mAvatarCollisionSphereRadius );
			}			
		}
	}

}//------------------------------------------------------------------




//------------------------------------------------------------------
void LLVOCloth::render()
{
	if ( mAttributes.mTextureFileName != "not_specified" )
	{
		LLViewerImage* theTexture = gImageList.getImage( LLUUID( gViewerArt.getString( mAttributes.mTextureFileName ) ), MIPMAP_FALSE, TRUE);
		if  ( theTexture )
		{	
			/*if ( mAttributes.mVisualizeAvCollisionSphere )
			{
				for ( int s=0; s<mAttributes.mNumStrands; s++)
				{
					mStrand[s]->renderCollisionSphere();
				}
			}*/

			if ( mAttributes.mVisualizeStrands )
			{
				for ( int s=0; s<mAttributes.mNumStrands; s++)
				{
					//------------------------------------------------
					// draw a small sphere at the top of the strand
					//------------------------------------------------
					glPushMatrix();

					glTranslatef
						( 
							mStrand[s]->getAnchorPosition().mV[_X_],
							mStrand[s]->getAnchorPosition().mV[_Y_], 
							mStrand[s]->getAnchorPosition().mV[_Z_]
						);

					glScalef
						( 
							STRAND_VISUALIZATION_ANCHOR_BALL_RADIUS, 
							STRAND_VISUALIZATION_ANCHOR_BALL_RADIUS, 
							STRAND_VISUALIZATION_ANCHOR_BALL_RADIUS 
						);

					gSphere.render();
					glPopMatrix();

					//--------------------------------
					// draw the strand
					//--------------------------------
					//mStrand[s]->render();







					//--------------------------------------------
					// draw the segments between the strands
					//--------------------------------------------
					if ( s > 0 )
					{
						//--------------------------------
						// draw the top edge...
						//--------------------------------
						glBegin( GL_LINES );
							glVertex3fv( mStrand[s-1]->getAnchorPosition().mV );
							glVertex3fv( mStrand[s  ]->getAnchorPosition().mV );
						glEnd();

						//--------------------------------------------
						// draw the rest of the segments
						//--------------------------------------------
						for ( int i=0; i<mAttributes.mNumSegments; i++)
						{
							glBegin( GL_LINES );
								glVertex3fv( mStrand[s-1]->getNodePosition(i).mV );
								glVertex3fv( mStrand[s  ]->getNodePosition(i).mV );
							glEnd();
						}
					}
				}
			}

			//--------------------------------------------------------
			// now, render the cape itself
			//--------------------------------------------------------
			glColor4fv( LLColor4( 1.0f, 1.0f, 1.0f, 1.0f ).mV );

			for ( int s=1; s<mAttributes.mNumStrands; s++)
			{
				//to be optimized later (should be calculated only once and stored in an array)
				F32 x0 = (F32)(s-1)	/ (F32)( mAttributes.mNumStrands - 1 );
				F32 x1 = (F32)s		/ (F32)( mAttributes.mNumStrands - 1 );

				for ( int i=0; i<mAttributes.mNumSegments; i++)
				{
					LLVector3 leftBottom;
					LLVector3 rightBottom;
					LLVector3 leftTop;
					LLVector3 rightTop;

					F32 y0 = (F32)i		/ (F32)mAttributes.mNumSegments;
					F32 y1 = (F32)(i+1)	/ (F32)mAttributes.mNumSegments;

					y0 = 1.0f - y0;
					y1 = 1.0f - y1;

					if ( i == 0 )
					{
						leftBottom	= mStrand[ s-1	]->getAnchorPosition();
						rightBottom	= mStrand[ s	]->getAnchorPosition();
					}
					else
					{
						leftBottom	= mStrand[ s-1	]->getNodePosition( i-1	);
						rightBottom	= mStrand[ s	]->getNodePosition( i-1	);
					}

					leftTop		= mStrand[ s-1	]->getNodePosition( i );
					rightTop	= mStrand[ s	]->getNodePosition( i );

					//gGLSTexture.set(); //set render state to use texture
					LLViewerImage::bindTexture(theTexture);

					glBegin( GL_TRIANGLE_STRIP );
						glTexCoord2f( x0, y0 ); glVertex3fv( leftBottom.mV );
						glTexCoord2f( x1, y0 ); glVertex3fv( rightBottom.mV );
						glTexCoord2f( x0, y1 ); glVertex3fv( leftTop.mV );
					glEnd();
					glBegin( GL_TRIANGLE_STRIP );
						glTexCoord2f( x1, y0 ); glVertex3fv( rightBottom.mV );
						glTexCoord2f( x1, y1 ); glVertex3fv( rightTop.mV );
						glTexCoord2f( x0, y1 ); glVertex3fv( leftTop.mV );
					glEnd();

					theTexture->unbindTexture(0, GL_TEXTURE_2D);
				}
			}
		}
	}

}//------------------------------------------------------------------




//------------------------------------------------------------------
void LLVOCloth::setAttributes( LLClothAttributes a )
{
	mAttributes.mTextureFileName						= a.mTextureFileName;
	mAttributes.mTextureIndex							= a.mTextureIndex;
	mAttributes.mNumSegments							= a.mNumSegments;
	mAttributes.mNumStrands								= a.mNumStrands;
	mAttributes.mVisualizeStrands						= a.mVisualizeStrands;
	mAttributes.mVisualizeAvCollisionSphere				= a.mVisualizeAvCollisionSphere;
	mAttributes.mWidth									= a.mWidth;
	mAttributes.mLength									= a.mLength;
	mAttributes.mPitch									= a.mPitch;
	mAttributes.mTension								= a.mTension;
	mAttributes.mGravity								= a.mGravity;
	mAttributes.mAirFriction							= a.mAirFriction;	
	mAttributes.mWindSensitivity						= a.mWindSensitivity;	
	mAttributes.mCircleWrapAmount						= a.mCircleWrapAmount;	
	mAttributes.mUsingAvatarCollisionSphere				= a.mUsingAvatarCollisionSphere;		
//mAttributes.mAvatarCollisionSpherePositionOffset	= a.mAvatarCollisionSpherePositionOffset;
	mAttributes.mAvatarCollisionSphereRadius			= a.mAvatarCollisionSphereRadius;
	mAttributes.mAvatarCollisionSphereRightOffset		= a.mAvatarCollisionSphereRightOffset;
	mAttributes.mAvatarCollisionSphereUpOffset			= a.mAvatarCollisionSphereUpOffset;
	mAttributes.mAvatarCollisionSphereForwardOffset		= a.mAvatarCollisionSphereForwardOffset;


	if		( mAttributes.mTextureIndex		< CLOTH_MIN_TEXTURE_INDEX		){ mAttributes.mTextureIndex		= CLOTH_MIN_TEXTURE_INDEX;	}
	else if ( mAttributes.mTextureIndex		> CLOTH_MAX_TEXTURE_INDEX		){ mAttributes.mTextureIndex		= CLOTH_MAX_TEXTURE_INDEX;	}
	if		( mAttributes.mPitch			< CLOTH_MIN_PITCH				){ mAttributes.mPitch				= CLOTH_MIN_PITCH;			}
	else if ( mAttributes.mPitch			> CLOTH_MAX_PITCH				){ mAttributes.mPitch				= CLOTH_MAX_PITCH;			}
	if		( mAttributes.mWidth			< CLOTH_MIN_WIDTH				){ mAttributes.mWidth				= CLOTH_MIN_WIDTH;			}
	else if ( mAttributes.mWidth			> CLOTH_MAX_WIDTH				){ mAttributes.mWidth				= CLOTH_MAX_WIDTH;			}
	if		( mAttributes.mLength			< CLOTH_MIN_LENGTH				){ mAttributes.mLength				= CLOTH_MIN_LENGTH;			}
	else if ( mAttributes.mLength			> CLOTH_MAX_LENGTH				){ mAttributes.mLength				= CLOTH_MAX_LENGTH;			}
	if		( mAttributes.mNumSegments		< CLOTH_MIN_SEGMENTS			){ mAttributes.mNumSegments			= CLOTH_MIN_SEGMENTS;		}
	else if ( mAttributes.mNumSegments		> CLOTH_MAX_SEGMENTS			){ mAttributes.mNumSegments			= CLOTH_MAX_SEGMENTS;		}
	if		( mAttributes.mNumStrands		< CLOTH_MIN_STRANDS				){ mAttributes.mNumStrands			= CLOTH_MIN_STRANDS;		}
	else if ( mAttributes.mNumStrands		> CLOTH_MAX_STRANDS				){ mAttributes.mNumStrands			= CLOTH_MAX_STRANDS;		}
	if		( mAttributes.mTension			< CLOTH_MIN_TENSION				){ mAttributes.mTension				= CLOTH_MIN_TENSION;		}
	else if ( mAttributes.mTension			> CLOTH_MAX_TENSION				){ mAttributes.mTension				= CLOTH_MAX_TENSION;		}
	if		( mAttributes.mGravity			< CLOTH_MIN_GRAVITY				){ mAttributes.mGravity				= CLOTH_MIN_GRAVITY;		}
	else if ( mAttributes.mGravity			> CLOTH_MAX_GRAVITY				){ mAttributes.mGravity				= CLOTH_MAX_GRAVITY;		}
	if		( mAttributes.mAirFriction		< CLOTH_MIN_AIR_FRICTION		){ mAttributes.mAirFriction			= CLOTH_MIN_AIR_FRICTION;	}
	else if ( mAttributes.mAirFriction		> CLOTH_MAX_AIR_FRICTION		){ mAttributes.mAirFriction			= CLOTH_MAX_AIR_FRICTION;	}
	if		( mAttributes.mWindSensitivity	< CLOTH_MIN_WIND_SENSITIVITY	){ mAttributes.mWindSensitivity		= CLOTH_MIN_WIND_SENSITIVITY;}
	else if ( mAttributes.mWindSensitivity	> CLOTH_MAX_WIND_SENSITIVITY	){ mAttributes.mWindSensitivity		= CLOTH_MAX_WIND_SENSITIVITY;}
	if		( mAttributes.mCircleWrapAmount	< CLOTH_MIN_CIRCLE_WRAP_AMOUNT	){ mAttributes.mCircleWrapAmount	= CLOTH_MIN_CIRCLE_WRAP_AMOUNT;}
	else if ( mAttributes.mCircleWrapAmount	> CLOTH_MAX_CIRCLE_WRAP_AMOUNT	){ mAttributes.mCircleWrapAmount	= CLOTH_MAX_CIRCLE_WRAP_AMOUNT;}
	
	if		( mAttributes.mAvatarCollisionSphereRadius			< CLOTH_MIN_AVATAR_COLLISION_SPHERE_RADIUS			){ mAttributes.mAvatarCollisionSphereRadius			= CLOTH_MIN_AVATAR_COLLISION_SPHERE_RADIUS;}
	else if ( mAttributes.mAvatarCollisionSphereRadius			> CLOTH_MAX_AVATAR_COLLISION_SPHERE_RADIUS			){ mAttributes.mAvatarCollisionSphereRadius			= CLOTH_MAX_AVATAR_COLLISION_SPHERE_RADIUS;}
	if		( mAttributes.mAvatarCollisionSphereRightOffset		< CLOTH_MIN_AVATAR_COLLISION_SPHERE_RIGHT_OFFSET	){ mAttributes.mAvatarCollisionSphereRightOffset	= CLOTH_MIN_AVATAR_COLLISION_SPHERE_RIGHT_OFFSET;}
	else if ( mAttributes.mAvatarCollisionSphereRightOffset		> CLOTH_MAX_AVATAR_COLLISION_SPHERE_RIGHT_OFFSET	){ mAttributes.mAvatarCollisionSphereRightOffset	= CLOTH_MAX_AVATAR_COLLISION_SPHERE_RIGHT_OFFSET;}
	if		( mAttributes.mAvatarCollisionSphereUpOffset		< CLOTH_MIN_AVATAR_COLLISION_SPHERE_UP_OFFSET		){ mAttributes.mAvatarCollisionSphereUpOffset		= CLOTH_MIN_AVATAR_COLLISION_SPHERE_UP_OFFSET;}
	else if ( mAttributes.mAvatarCollisionSphereUpOffset		> CLOTH_MAX_AVATAR_COLLISION_SPHERE_UP_OFFSET		){ mAttributes.mAvatarCollisionSphereUpOffset		= CLOTH_MAX_AVATAR_COLLISION_SPHERE_UP_OFFSET;}
	if		( mAttributes.mAvatarCollisionSphereForwardOffset	< CLOTH_MIN_AVATAR_COLLISION_SPHERE_FORWARD_OFFSET	){ mAttributes.mAvatarCollisionSphereForwardOffset	= CLOTH_MIN_AVATAR_COLLISION_SPHERE_FORWARD_OFFSET;}
	else if ( mAttributes.mAvatarCollisionSphereForwardOffset	> CLOTH_MAX_AVATAR_COLLISION_SPHERE_FORWARD_OFFSET	){ mAttributes.mAvatarCollisionSphereForwardOffset	= CLOTH_MAX_AVATAR_COLLISION_SPHERE_FORWARD_OFFSET;}

	//-----------------------------------------------------------------------------------
	// based on the above attributes, the strands (flexible objects) are generated
	//-----------------------------------------------------------------------------------
	generateStrands();

}//------------------------------------------------------------------





//------------------------------------------------------------------
// set methods
//------------------------------------------------------------------

//------------------------------------------------------------------
void LLVOCloth::setNumStrands( int num )
{
	mAttributes.mNumStrands = num;
	generateStrands();
}//------------------------------------------------------------------
//------------------------------------------------------------------
void LLVOCloth::setNumSegments( int num )
{
	mAttributes.mNumSegments = num;
	generateStrands();
}//------------------------------------------------------------------
//------------------------------------------------------------------
void LLVOCloth::setLength( F32 le )
{
	mAttributes.mLength = le;
	generateStrands();
}//------------------------------------------------------------------
//------------------------------------------------------------------
void LLVOCloth::setCircleWrapAmount( F32 n )
{
	mAttributes.mCircleWrapAmount = n;
	generateStrands();
}//------------------------------------------------------------------
//------------------------------------------------------------------
void LLVOCloth::setGravity( F32 g )
{
	mAttributes.mGravity = g;
	generateStrands();
}//------------------------------------------------------------------
//------------------------------------------------------------------
void LLVOCloth::setPitch( F32 p )
{
	mAttributes.mPitch = p;
	generateStrands();
}//------------------------------------------------------------------
//------------------------------------------------------------------
void LLVOCloth::setAirFriction( F32 a )
{
	mAttributes.mAirFriction = a;
	generateStrands();
}//------------------------------------------------------------------
//------------------------------------------------------------------
void LLVOCloth::setWindSensitivity( F32 w )
{
	mAttributes.mWindSensitivity = w;
	generateStrands();
}//------------------------------------------------------------------
//------------------------------------------------------------------
void LLVOCloth::setTension( F32 t )
{
	mAttributes.mTension = t;
	generateStrands();
}//------------------------------------------------------------------
//------------------------------------------------------------------
void LLVOCloth::setWidth( F32 n )
{
	mAttributes.mWidth = n;
	generateStrands();
}//------------------------------------------------------------------
//------------------------------------------------------------------
void LLVOCloth::setVisualizingStrands( bool v )
{
	mAttributes.mVisualizeStrands = v;
}//------------------------------------------------------------------//------------------------------------------------------------------
void LLVOCloth::setVisualizingAvCollisionSphere( bool v )
{
	mAttributes.mVisualizeAvCollisionSphere = v;
}//------------------------------------------------------------------
//------------------------------------------------------------------
void LLVOCloth::setAvatarCollisionSphereRadius( F32 r )
{
	mAttributes.mAvatarCollisionSphereRadius = r;

}//------------------------------------------------------------------
//------------------------------------------------------------------
void LLVOCloth::setAvatarCollisionSphereRight( F32 r )
{
	mAttributes.mAvatarCollisionSphereRightOffset = r;

}//------------------------------------------------------------------
//------------------------------------------------------------------
void LLVOCloth::setAvatarCollisionSphereUp( F32 u )
{
	mAttributes.mAvatarCollisionSphereUpOffset = u;

}//------------------------------------------------------------------
//------------------------------------------------------------------
void LLVOCloth::setAvatarCollisionSphereForward( F32 f )
{
	mAttributes.mAvatarCollisionSphereForwardOffset = f;

}//------------------------------------------------------------------


//------------------------------------------------------------------
// get methods
//------------------------------------------------------------------

//------------------------------------------------------------------
int LLVOCloth::getNumStrands()
{
	return mAttributes.mNumStrands;
}//------------------------------------------------------------------
//------------------------------------------------------------------
int LLVOCloth::getNumSegments()
{
	return mAttributes.mNumSegments;
}//------------------------------------------------------------------
//------------------------------------------------------------------
F32 LLVOCloth::getLength()
{
	return mAttributes.mLength;
}//------------------------------------------------------------------
//------------------------------------------------------------------
F32 LLVOCloth::getWidth()
{
	return mAttributes.mWidth;
}//------------------------------------------------------------------
//------------------------------------------------------------------
F32 LLVOCloth::getCircleWrapAmount()
{
	return mAttributes.mCircleWrapAmount;
}//------------------------------------------------------------------
//------------------------------------------------------------------
F32 LLVOCloth::getPitch()
{
	return mAttributes.mPitch;
}//------------------------------------------------------------------
//------------------------------------------------------------------
F32 LLVOCloth::getTension()
{
	return mAttributes.mTension;
}//------------------------------------------------------------------
//------------------------------------------------------------------
F32 LLVOCloth::getGravity()
{
	return mAttributes.mGravity;
}//------------------------------------------------------------------
//------------------------------------------------------------------
F32 LLVOCloth::getAirFriction()
{
	return mAttributes.mAirFriction;
}//------------------------------------------------------------------
//------------------------------------------------------------------
F32 LLVOCloth::getWindSensitivity()
{
	return mAttributes.mWindSensitivity;
}//------------------------------------------------------------------
//------------------------------------------------------------------
bool LLVOCloth::getVisualizingStrands() const
{
	return mAttributes.mVisualizeStrands;
}//------------------------------------------------------------------
//------------------------------------------------------------------
bool LLVOCloth::getVisualizingAvCollisionSphere() const
{
	return mAttributes.mVisualizeAvCollisionSphere;
}//------------------------------------------------------------------
//------------------------------------------------------------------
F32 LLVOCloth::getAvatarCollisionSphereRadius()
{
	return mAttributes.mAvatarCollisionSphereRadius;
}//------------------------------------------------------------------
//------------------------------------------------------------------
F32 LLVOCloth::getAvatarCollisionSphereRight()
{
	return mAttributes.mAvatarCollisionSphereRightOffset;
}//------------------------------------------------------------------
//------------------------------------------------------------------
F32 LLVOCloth::getAvatarCollisionSphereUp()
{
	return mAttributes.mAvatarCollisionSphereUpOffset;
}//------------------------------------------------------------------
//------------------------------------------------------------------
F32 LLVOCloth::getAvatarCollisionSphereForward()
{
	return mAttributes.mAvatarCollisionSphereForwardOffset;
}//------------------------------------------------------------------



//------------------------------------------------------------------
void LLVOCloth::markAsDead()
{
	mDead = TRUE;

}//------------------------------------------------------------------








