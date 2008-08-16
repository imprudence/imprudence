/** 
 * @file lldrawpoolground.cpp
 * @brief LLDrawPoolGround class implementation
 *
 * Copyright (c) 2002-2007, Linden Research, Inc.
 * 
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

#include "lldrawpoolground.h"

#include "llviewercontrol.h"

#include "llagparray.h"
#include "lldrawable.h"
#include "llface.h"
#include "llsky.h"
#include "llviewercamera.h"
#include "llviewerwindow.h"
#include "llworld.h"
#include "pipeline.h"

LLDrawPoolGround::LLDrawPoolGround() :
	LLDrawPool(POOL_GROUND, DATA_SIMPLE_IL_MASK, DATA_SIMPLE_NIL_MASK)
{
}

LLDrawPool *LLDrawPoolGround::instancePool()
{
	return new LLDrawPoolGround();
}

void LLDrawPoolGround::prerender()
{
	mVertexShaderLevel = gPipeline.getVertexShaderLevel(LLPipeline::SHADER_ENVIRONMENT);
}

void LLDrawPoolGround::render(S32 pass)
{
	if (mDrawFace.empty())
	{
		return;
	}

	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);

	bindGLVertexPointer();
	bindGLTexCoordPointer();

	LLGLSPipelineSkyBox gls_skybox;
	LLGLDepthTest gls_depth(GL_FALSE, GL_FALSE);

	glMatrixMode( GL_PROJECTION );

	glPushMatrix();
	gViewerWindow->setup3DRender();

	glMatrixMode(GL_MODELVIEW);

	LLGLState tex2d(GL_TEXTURE_2D, (mVertexShaderLevel > 0) ? TRUE : FALSE);
	LLViewerImage::bindTexture(gSky.mVOSkyp->getScatterMap(), 0);

	LLFace *facep = mDrawFace[0];

	if (!(mVertexShaderLevel > 0))
	{
		gPipeline.disableLights();
	}

	glColor4fv(facep->getFaceColor().mV);	

	facep->renderIndexed(getRawIndices());
	
	glMatrixMode( GL_PROJECTION );
	glPopMatrix();
	glMatrixMode( GL_MODELVIEW );
}

void LLDrawPoolGround::renderForSelect()
{
}

