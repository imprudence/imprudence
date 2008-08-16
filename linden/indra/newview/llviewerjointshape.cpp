/** 
 * @file llviewerjointshape.cpp
 * @brief Implementation of LLViewerJointShape class
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

//-----------------------------------------------------------------------------
// Header Files
//-----------------------------------------------------------------------------
#include "llviewerprecompiledheaders.h"

#include "llviewerjointshape.h"

#include "llbox.h"
#include "llsphere.h"
#include "llcylinder.h"
#include "llgldbg.h"
#include "llglheaders.h"

F32 LLViewerJointShape::sColorScale = 1.0f;

//-----------------------------------------------------------------------------
// LLViewerJointShape()
//-----------------------------------------------------------------------------
LLViewerJointShape::LLViewerJointShape()
{
	mType = ST_NULL;
	mColor[0] = 1.0f;
	mColor[1] = 1.0f;
	mColor[2] = 1.0f;
	mColor[3] = 1.0f;
	mTexture = NULL;
}


//-----------------------------------------------------------------------------
// LLViewerJointShape()
//-----------------------------------------------------------------------------
LLViewerJointShape::LLViewerJointShape( ShapeType type, F32 red, F32 green, F32 blue, F32 alpha )
{
	mType = type;
	mColor[0] = red * sColorScale;
	mColor[1] = green * sColorScale;
	mColor[2] = blue * sColorScale;
	mColor[3] = alpha;
	mTexture = NULL;
}


//-----------------------------------------------------------------------------
// ~LLViewerJointShape()
// Class Destructor
//-----------------------------------------------------------------------------
LLViewerJointShape::~LLViewerJointShape()
{
}


//--------------------------------------------------------------------
// getType()
//--------------------------------------------------------------------
LLViewerJointShape::ShapeType LLViewerJointShape::getType()
{
	return mType;
}


//--------------------------------------------------------------------
// setType()
//--------------------------------------------------------------------
void LLViewerJointShape::setType( ShapeType type )
{
	mType = type;
}


//--------------------------------------------------------------------
// getColor()
//--------------------------------------------------------------------
void LLViewerJointShape::getColor( F32 *red, F32 *green, F32 *blue, F32 *alpha )
{
	*red   = mColor[0];
	*green = mColor[1];
	*blue  = mColor[2];
	*alpha = mColor[3];
}

//--------------------------------------------------------------------
// setColor()
//--------------------------------------------------------------------
void LLViewerJointShape::setColor( F32 red, F32 green, F32 blue, F32 alpha )
{
	mColor[0] = red  * sColorScale;
	mColor[1] = green * sColorScale;
	mColor[2] = blue * sColorScale;
	mColor[3] = alpha;
}


//--------------------------------------------------------------------
// getTexture()
//--------------------------------------------------------------------
LLViewerImage *LLViewerJointShape::getTexture()
{
	return mTexture;
}

//--------------------------------------------------------------------
// setTexture()
//--------------------------------------------------------------------
void LLViewerJointShape::setTexture( LLViewerImage *texture )
{
	mTexture = texture;
}


//--------------------------------------------------------------------
// drawBone()
//--------------------------------------------------------------------
void LLViewerJointShape::drawBone()
{
}


//--------------------------------------------------------------------
// isTransparent()
//--------------------------------------------------------------------
BOOL LLViewerJointShape::isTransparent()
{
	return (	(mColor[3] < 1.0f) ||
				(!mTexture.isNull() && (mTexture->getComponents()==4)) );
}

//--------------------------------------------------------------------
// drawShape()
//--------------------------------------------------------------------
U32 LLViewerJointShape::drawShape( F32 pixelArea, BOOL first_pass )
{
	U32 triangle_count = 0;

	//----------------------------------------------------------------
	// render ST_NULL
	//----------------------------------------------------------------
	if (mType == ST_NULL)
	{
		return triangle_count;
	}

	//----------------------------------------------------------------
	// setup current color
	//----------------------------------------------------------------
	glColor4fv(mColor.mV);

	//----------------------------------------------------------------
	// setup current texture
	//----------------------------------------------------------------
	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadIdentity();
	if (mType == ST_SPHERE)
	{
		glTranslatef(-0.25f, 0.0f, 0.0f);
	}
	glMatrixMode(GL_MODELVIEW);
	LLViewerImage::bindTexture(mTexture);

	//----------------------------------------------------------------
	// update pixel area
	//----------------------------------------------------------------
	F32 s1 = llmax( getScale().mV[VX], llmax( getScale().mV[VY], getScale().mV[VZ] ) );
	F32 s2 = llmin( getScale().mV[VX], llmax( getScale().mV[VY], getScale().mV[VZ] ) );
	pixelArea *= s1 * s2;

	//----------------------------------------------------------------
	// render shape
	//----------------------------------------------------------------
	switch ( mType )
	{
	case ST_CUBE:
		gBox.render();
		break;

	case ST_SPHERE:
		gSphere.render( pixelArea );
		break;

	case ST_CYLINDER:
		gCylinder.render( pixelArea );
		break;

	default:
		break;
	}

	//----------------------------------------------------------------
	// disable texture
	//----------------------------------------------------------------
	if ( mTexture )
	{
		glMatrixMode(GL_TEXTURE);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
	}

	return triangle_count;
}

// End
