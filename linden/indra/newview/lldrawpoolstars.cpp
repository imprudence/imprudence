/** 
 * @file lldrawpoolstars.cpp
 * @brief LLDrawPoolStars class implementation
 *
 * Copyright (c) 2002-2007, Linden Research, Inc.
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

#include "lldrawpoolstars.h"

#include "llface.h"
#include "llsky.h"
#include "llvostars.h"
#include "pipeline.h"
#include "llviewercamera.h"
#include "llglslshader.h"

LLDrawPoolStars::LLDrawPoolStars() :
	LLFacePool(POOL_STARS)
{
}

LLDrawPool *LLDrawPoolStars::instancePool()
{
	return new LLDrawPoolStars();
}

void LLDrawPoolStars::prerender()
{
	mVertexShaderLevel = LLShaderMgr::getVertexShaderLevel(LLShaderMgr::SHADER_ENVIRONMENT);
}

void LLDrawPoolStars::render(S32 pass)
{
 	if (!(gPipeline.hasRenderType(LLPipeline::RENDER_TYPE_SKY)))
	{
		return;
	}
	
	if (mDrawFace.empty())
	{
		return;
	}
	
	LLGLSPipelineSkyBox gls_sky;
	LLGLDepthTest gls_depth(GL_FALSE, GL_FALSE);
	LLGLEnable blend(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	LLGLDisable gl_texture_2d(GL_TEXTURE_2D);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);

	gPipeline.disableLights();

	GLint* viewport = (GLint*) gGLViewport;

	if (viewport[2] > 512 && viewport[3] > 512)
	{
		glPointSize(2.f);
	}

	LLVector3 origin = gCamera->getOrigin();
	glPushMatrix();
	glTranslatef(origin.mV[0], origin.mV[1], origin.mV[2]);

	glEnableClientState(GL_COLOR_ARRAY);
	
	S32 face_count = (S32)mDrawFace.size();
	for (S32 curr_face = 0; curr_face < face_count; curr_face++)
	{
		LLFace* face = mDrawFace[curr_face];
		if (!face->getGeomCount())
		{
			continue;
		}
	
		//  render the stars as a sphere centered at viewer camera 
		if (face->mVertexBuffer.notNull())
		{
			face->mVertexBuffer->setBuffer(getVertexDataMask());
			U32* indicesp = (U32*) face->mVertexBuffer->getIndicesPointer();
			glDrawElements(GL_POINTS, face->getIndicesCount(), GL_UNSIGNED_INT, indicesp);
			mIndicesDrawn += face->getIndicesCount();
		}
	}
	glDisableClientState(GL_COLOR_ARRAY);
	glPointSize(1.f);
	glPopMatrix();
}


void LLDrawPoolStars::renderForSelect()
{
}

