/** 
 * @file lldrawpooltreenew.cpp
 * @brief LLDrawPoolTreeNew class implementation
 *
 * Copyright (c) 2003-2007, Linden Research, Inc.
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

#include "lldrawpooltreenew.h"

#include "llagparray.h"
#include "lldrawable.h"
#include "llface.h"
#include "llsky.h"
#include "llvotreenew.h"
#include "pipeline.h"

LLDrawPoolTreeNew::LLDrawPoolTreeNew(LLViewerImage *texturep) :
	LLDrawPool(POOL_TREE_NEW,
				DATA_VERTICES_MASK | DATA_TEX_COORDS0_MASK | DATA_NORMALS_MASK,
				0),
	mTexturep(texturep)
{
}

LLDrawPool *LLDrawPoolTreeNew::instancePool()
{
	return new LLDrawPoolTreeNew(mTexturep);
}


void LLDrawPoolTreeNew::prerender()
{
	mVertexShaderLevel = gPipeline.getVertexShaderLevel(LLPipeline::SHADER_OBJECT);
}

void LLDrawPoolTreeNew::render( S32 pass )
{
	if (mDrawFace.empty())
	{
		return;
	}

	LLGLSPipelineAlpha gls_pipeline_alpha;

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

	bindGLVertexPointer();
	bindGLTexCoordPointer();
	bindGLNormalPointer();

	LLOverrideFaceColor color(this, 1.f, 1.f, 1.f, 1.f);
	
	renderTree();
}


void LLDrawPoolTreeNew::renderForSelect()
{
	if (mDrawFace.empty())
	{
		return;
	}

	LLGLSPipelineAlpha gls_pipeline_alpha;

	glEnableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);

	bindGLVertexPointer();

	renderTree(TRUE);

	glDisableClientState(GL_VERTEX_ARRAY);

}




void LLDrawPoolTreeNew::renderTree(BOOL selecting)
{

	// Bind the texture for this tree.
	LLViewerImage::bindTexture(mTexturep);

	glMatrixMode(GL_MODELVIEW);
	for (std::vector<LLFace*>::iterator iter = mDrawFace.begin();
		 iter != mDrawFace.end(); iter++)
	{
		LLFace *face = *iter;
		LLDrawable *drawablep = face->getDrawable();

		if (drawablep->isDead())
		{
			continue;
		}
		// Render each of the trees
		LLVOTreeNew *treep = (LLVOTreeNew *)drawablep->getVObj();
		if (!selecting || treep->mGLName != 0)
		{
			glPushMatrix();

			// Translate to tree base  HACK - adjustment in Z plants tree underground
			const LLVector3 &pos_agent = treep->getPositionAgent();
			glTranslatef(pos_agent.mV[VX], pos_agent.mV[VY], pos_agent.mV[VZ] - 0.1f);

			// Rotate to tree position
			F32 angle_radians, x, y, z;
			treep->getRotation().getAngleAxis(&angle_radians, &x, &y, &z);
			glRotatef(angle_radians * RAD_TO_DEG, x, y, z);

			//glAlphaFunc(GL_GREATER, 0.5f);

			treep->drawTree(*this);
			glPopMatrix();
		}
	}

	glAlphaFunc(GL_GREATER, 0.01f);
}


S32 LLDrawPoolTreeNew::rebuild()
{
	mRebuildTime++;
	if (mRebuildTime > mRebuildFreq)
	{
		// Flush AGP to force an AGP realloc and reduce AGP fragmentation
		flushAGP();
		mRebuildTime = 0;
	}

	return 0;
}

BOOL LLDrawPoolTreeNew::verify() const
{
	BOOL ok = TRUE;

	// shared geometry.  Just verify that it's there and correct.

	// Verify all indices in the pool are in the right range
	const U32 *indicesp = getRawIndices();
	U32 i;
	for (i = 0; i < getIndexCount(); i++)
	{
		if (indicesp[i] > getVertexCount())
		{
			ok = FALSE;
			llinfos << "Bad index in tree pool!" << llendl;
		}
	}
	
	if (!ok)
	{
		printDebugInfo();
	}
	return ok;
}

LLViewerImage *LLDrawPoolTreeNew::getTexture()
{
	return mTexturep;
}

LLViewerImage *LLDrawPoolTreeNew::getDebugTexture()
{
	return mTexturep;
}


LLColor3 LLDrawPoolTreeNew::getDebugColor() const
{
	return LLColor3(1.f, 0.f, 1.f);
}
