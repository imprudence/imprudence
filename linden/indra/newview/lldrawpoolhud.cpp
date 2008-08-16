/** 
 * @file lldrawpoolhud.cpp
 * @brief LLDrawPoolHUD class implementation
 *
 * Copyright (c) 2006-2007, Linden Research, Inc.
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

#include "lldrawpoolhud.h"

#include "llagparray.h"
#include "llagent.h"
#include "llbbox.h"
#include "llcubemap.h"
#include "lldrawable.h"
#include "lldrawpoolalpha.h"
#include "lldrawpoolbump.h"
#include "llface.h"
#include "llhudtext.h"
#include "llsky.h"
#include "lltoolmgr.h"
#include "llvoavatar.h"
#include "llviewercamera.h"
#include "llviewerobjectlist.h"
#include "llviewerwindow.h"
#include "pipeline.h"

BOOL LLDrawPoolHUD::sShowHUDAttachments = TRUE;

LLDrawPoolHUD::LLDrawPoolHUD() :
	LLDrawPool(POOL_HUD,
			   DATA_BUMP_IL_MASK | DATA_COLORS_MASK, // HUD objects may be bumpmapped
			   DATA_SIMPLE_NIL_MASK),
	mRenderMode(POOL_SIMPLE)
{
}

LLDrawPool* LLDrawPoolHUD::instancePool()
{
	return new LLDrawPoolHUD();
}


void LLDrawPoolHUD::prerender()
{
	mVertexShaderLevel = gPipeline.getVertexShaderLevel(LLPipeline::SHADER_OBJECT);
}

void LLDrawPoolHUD::beginRenderPass(S32 pass)
{
}

static BOOL setup_matrices(BOOL for_select)
{
	LLVOAvatar* my_avatarp = gAgent.getAvatarObject();
	if (my_avatarp && my_avatarp->hasHUDAttachment())
	{
		if (!for_select)
		{
			// clamp target zoom level to reasonable values
			my_avatarp->mHUDTargetZoom = llclamp(my_avatarp->mHUDTargetZoom, 0.1f, 1.f);
			// smoothly interpolate current zoom level
			my_avatarp->mHUDCurZoom = lerp(my_avatarp->mHUDCurZoom, my_avatarp->mHUDTargetZoom, LLCriticalDamp::getInterpolant(0.03f));
		}

		F32 zoom_level = my_avatarp->mHUDCurZoom;
		// clear z buffer and set up transform for hud
		if (!for_select)
		{
			glClear(GL_DEPTH_BUFFER_BIT);
		}
		LLBBox hud_bbox = my_avatarp->getHUDBBox();

		// set up transform to encompass bounding box of HUD
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		F32 hud_depth = llmax(1.f, hud_bbox.getExtentLocal().mV[VX] * 1.1f);
		if (for_select)
		{
			//RN: reset viewport to window extents so ortho screen is calculated with proper reference frame
			gViewerWindow->setupViewport();
		}
		glOrtho(-0.5f * gCamera->getAspect(), 0.5f * gCamera->getAspect(), -0.5f, 0.5f, 0.f, hud_depth);

		// apply camera zoom transform (for high res screenshots)
		F32 zoom_factor = gCamera->getZoomFactor();
		S16 sub_region = gCamera->getZoomSubRegion();
		if (zoom_factor > 1.f)
		{
			float offset = zoom_factor - 1.f;
			int pos_y = sub_region / llceil(zoom_factor);
			int pos_x = sub_region - (pos_y*llceil(zoom_factor));
			glTranslatef(gCamera->getAspect() * 0.5f * (offset - (F32)pos_x * 2.f), 0.5f * (offset - (F32)pos_y * 2.f), 0.f);
			glScalef(zoom_factor, zoom_factor, 1.f);
		}

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
		glLoadMatrixf(OGL_TO_CFR_ROTATION);		// Load Cory's favorite reference frame
		glTranslatef(-hud_bbox.getCenterLocal().mV[VX] + (hud_depth * 0.5f), 0.f, 0.f);
		glScalef(zoom_level, zoom_level, zoom_level);

		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

static void restore_matrices()
{
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}


void LLDrawPoolHUD::render(S32 pass)
{
	LLFastTimer ftm(LLFastTimer::FTM_RENDER_HUD);
	if (!sShowHUDAttachments)
	{
		return;
	}

	if (mDrawFace.empty() && mAlphaFace.empty() && mBumpFaces.empty())
	{
		return;
	}

	if (!setup_matrices(FALSE))
	{
		return;
	}

	const U32* index_array = getRawIndices();

	bindGLVertexPointer();
	bindGLNormalPointer();
	bindGLTexCoordPointer();

	S32 diffTex = 0;
	S32 envTex = -1;
	// Non alpha faces	
	if (!mDrawFace.empty())
	{
		mRenderMode = POOL_SIMPLE;
		if (mVertexShaderLevel > 0)
		{
			gPipeline.mObjectSimpleProgram.bind();
			S32 scatterTex = gPipeline.mObjectSimpleProgram.enableTexture(LLPipeline::GLSL_SCATTER_MAP);
			LLViewerImage::bindTexture(gSky.mVOSkyp->getScatterMap(), scatterTex);
			diffTex = gPipeline.mObjectSimpleProgram.enableTexture(LLPipeline::GLSL_DIFFUSE_MAP);
		}

		LLGLEnable blend(GL_BLEND); // for invisi-prim hack, I think

		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_NORMAL_ARRAY);
		glActiveTextureARB(GL_TEXTURE0_ARB);
		glClientActiveTextureARB(GL_TEXTURE0_ARB);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		// Render
		mIndicesDrawn += drawLoopSetTex(mDrawFace, index_array, diffTex);

		if (mVertexShaderLevel > 0)
		{
			gPipeline.mObjectSimpleProgram.disableTexture(LLPipeline::GLSL_SCATTER_MAP);
			gPipeline.mObjectSimpleProgram.disableTexture(LLPipeline::GLSL_DIFFUSE_MAP);
		}
	}
	
	// Bump Mapped Faces
	if (!mBumpFaces.empty())
	{
		mRenderMode = POOL_BUMP;
		if (mVertexShaderLevel > 0)
		{
			gPipeline.mObjectBumpProgram.bind();
		}
		
		S32 bump_passes = LLDrawPoolBump::numBumpPasses();
		if (bump_passes >= 1)
		{
			// Texture unit 0
			LLDrawPoolBump::beginPass0(this);
			for (bump_face_map_t::iterator iter = mBumpFaces.begin();
				 iter != mBumpFaces.end(); iter++)
			{
				LLViewerImage* tex = iter->first;
				face_array_t& faces = iter->second;
				mIndicesDrawn += LLDrawPoolBump::renderPass0(this, faces, index_array, tex);
			}
			LLDrawPoolBump::endPass0(this);
		}
		
		if (bump_passes >= 2)
		{
			LLDrawPoolBump::beginPass1();
			for (bump_face_map_t::iterator iter = mBumpFaces.begin();
				 iter != mBumpFaces.end(); iter++)
			{
				LLViewerImage* tex = iter->first;
				face_array_t& faces = iter->second;
				mIndicesDrawn += LLDrawPoolBump::renderPass1(faces, index_array, tex);
			}
			LLDrawPoolBump::endPass1();
		}

		if (bump_passes >= 3)
		{
			bindGLVertexPointer();
			// Texture unit 0
			glActiveTextureARB(GL_TEXTURE0_ARB);
			glClientActiveTextureARB(GL_TEXTURE0_ARB);
			bindGLTexCoordPointer();
			// Texture unit 1
			glActiveTextureARB(GL_TEXTURE1_ARB);
			glClientActiveTextureARB(GL_TEXTURE1_ARB);
			bindGLTexCoordPointer(1);
			LLDrawPoolBump::beginPass2();
			for (bump_face_map_t::iterator iter = mBumpFaces.begin();
				 iter != mBumpFaces.end(); iter++)
			{
				LLViewerImage* tex = iter->first;
				face_array_t& faces = iter->second;
				mIndicesDrawn += LLDrawPoolBump::renderPass2(faces, index_array, tex);
			}
			LLDrawPoolBump::endPass2();
		}
	}
	
	// Alpha faces
	if (!mAlphaFace.empty())
	{
		// Sort
		std::sort(mAlphaFace.begin(), mAlphaFace.end(), LLFace::CompareDistanceGreater());

		// Render
		mRenderMode = POOL_ALPHA;
		
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_NORMAL_ARRAY);
		glActiveTextureARB(GL_TEXTURE0_ARB);
		glClientActiveTextureARB(GL_TEXTURE0_ARB);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		if (mVertexShaderLevel > 0)
		{
			gPipeline.mObjectAlphaProgram.bind();
			envTex = gPipeline.mObjectAlphaProgram.enableTexture(LLPipeline::GLSL_ENVIRONMENT_MAP, GL_TEXTURE_CUBE_MAP_ARB);
			LLCubeMap* cube_map = gSky.mVOSkyp->getCubeMap();
			if (envTex >= 0 && cube_map)
			{
				cube_map->bind();
				cube_map->setMatrix(1);
			}

			GLint specularIndex = gPipeline.mObjectAlphaProgram.mAttribute[LLPipeline::GLSL_SPECULAR_COLOR];
			if (specularIndex > 0)
			{
				glVertexAttrib4fARB(specularIndex, 0, 0, 0, 0);
			}
			
			S32 scatterTex = gPipeline.mObjectAlphaProgram.enableTexture(LLPipeline::GLSL_SCATTER_MAP);
			LLViewerImage::bindTexture(gSky.mVOSkyp->getScatterMap(), scatterTex);
			
			diffTex = gPipeline.mObjectAlphaProgram.enableTexture(LLPipeline::GLSL_DIFFUSE_MAP);
		}
		
		LLGLEnable blend(GL_BLEND);
		LLGLSPipelineAlpha alphastate;
		mIndicesDrawn += drawLoopSetTex(mAlphaFace, index_array, diffTex);
		if (mVertexShaderLevel > 0)
		{
			gPipeline.mObjectAlphaProgram.disableTexture(LLPipeline::GLSL_ENVIRONMENT_MAP, GL_TEXTURE_CUBE_MAP_ARB);
			LLCubeMap* cube_map = gSky.mVOSkyp->getCubeMap();
			if (envTex >= 0 && cube_map)
			{
				cube_map->restoreMatrix();
			}

			gPipeline.mObjectAlphaProgram.disableTexture(LLPipeline::GLSL_SCATTER_MAP);
			gPipeline.mObjectAlphaProgram.disableTexture(LLPipeline::GLSL_DIFFUSE_MAP);

			glClientActiveTextureARB(GL_TEXTURE0_ARB);
			glActiveTextureARB(GL_TEXTURE0_ARB);
			glEnable(GL_TEXTURE_2D);
		}
		
		if (LLDrawPoolAlpha::sShowDebugAlpha)
		{
			gPipeline.disableLights();
			if ((mVertexShaderLevel > 0))
			{
				gPipeline.mHighlightProgram.bind();
				gPipeline.mHighlightProgram.vertexAttrib4f(LLPipeline::GLSL_MATERIAL_COLOR,1,0,0,1);
			}
			
			LLViewerImage::sSmokeImagep->bind();
			LLOverrideFaceColor override_color(this, 1.f, 0.f, 0.f, 1.f);
			glDisableClientState(GL_COLOR_ARRAY);
			
			for (std::vector<LLFace*>::iterator iter = mAlphaFace.begin();
				 iter != mAlphaFace.end(); iter++)
			{
				LLFace *facep = *iter;
				facep->renderIndexed(index_array);
			}
		
			if ((mVertexShaderLevel > 0))
			{
				gPipeline.mHighlightProgram.unbind();
			}
		}
	}

	glDisableClientState(GL_NORMAL_ARRAY);
	glActiveTextureARB(GL_TEXTURE0_ARB);
	glEnable(GL_TEXTURE_2D);
	glClientActiveTextureARB(GL_TEXTURE0_ARB);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	if ((mVertexShaderLevel > 0))
	{
		glUseProgramObjectARB(0);
	}

	LLHUDText::renderAllHUD();
	restore_matrices();
}

void LLDrawPoolHUD::endRenderPass(S32 pass)
{
}

void LLDrawPoolHUD::renderForSelect()
{
	if (!sShowHUDAttachments)
	{
		return;
	}

	if (!gToolMgr->inEdit() && (gKeyboard->currentMask(TRUE) & MASK_ALT)) 
	{
		return;
	}

	if ((mDrawFace.empty() && mAlphaFace.empty() && mBumpFaces.empty()) || !mMemory.count())
	{
		return;
	}

	if (!setup_matrices(TRUE))
	{
		return;
	}
	
	glEnableClientState ( GL_VERTEX_ARRAY );
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	bindGLVertexPointer();

	LLGLSObjectSelect gls_objectselect;
	LLGLDepthTest gls_depth(GL_TRUE);

	for (std::vector<LLFace*>::iterator iter = mDrawFace.begin();
		 iter != mDrawFace.end(); iter++)
	{
		LLFace *facep = *iter;
		LLDrawable *drawable = facep->getDrawable();
		if (drawable && !drawable->isDead() && facep->getViewerObject() && facep->getViewerObject()->mGLName)
		{
			facep->renderForSelect();
		}
	}

	for (bump_face_map_t::iterator iter = mBumpFaces.begin();
		 iter != mBumpFaces.end(); iter++)
	{
		face_array_t& faces = iter->second;
		for (std::vector<LLFace*>::iterator iter = faces.begin();
			 iter != faces.end(); iter++)
		{
			LLFace *facep = *iter;
			LLDrawable *drawable = facep->getDrawable();
			if (drawable && !drawable->isDead() && facep->getViewerObject() && facep->getViewerObject()->mGLName)
			{
				facep->renderForSelect();
			}
		}
	}

	// ALPHA
	{
		glEnableClientState (GL_VERTEX_ARRAY);
		glEnableClientState (GL_TEXTURE_COORD_ARRAY);

		LLGLSObjectSelectAlpha* gls_selectalpha = NULL;
		if (!LLDrawPoolAlpha::sShowDebugAlpha)
		{
			gls_selectalpha = new LLGLSObjectSelectAlpha;
		}

		glBlendFunc(GL_ONE, GL_ZERO);
		glAlphaFunc(gPickTransparent ? GL_GEQUAL : GL_GREATER, 0.0f);

		bindGLVertexPointer();
		bindGLTexCoordPointer();

		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,		GL_COMBINE_ARB);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB,		GL_REPLACE);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_ARB,		GL_MODULATE);

		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB,		GL_PREVIOUS);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB,		GL_SRC_COLOR);

		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_ARB,		GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_ARB,	GL_SRC_ALPHA);

		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA_ARB,		GL_PREVIOUS);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA_ARB,	GL_SRC_ALPHA);
		
		for (std::vector<LLFace*>::iterator iter = mAlphaFace.begin();
			 iter != mAlphaFace.end(); iter++)
		{
			LLFace *facep = *iter;
			LLDrawable *drawable = facep->getDrawable();
			if (drawable && !drawable->isDead() && facep->getViewerObject() && facep->getViewerObject()->mGLName)
			{
				facep->bindTexture();
				facep->renderForSelect();
			}
		}

		glAlphaFunc(GL_GREATER, 0.01f);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

		glDisableClientState (GL_VERTEX_ARRAY);
		glDisableClientState (GL_TEXTURE_COORD_ARRAY);

		delete gls_selectalpha;
	}
	
	restore_matrices();
}


void LLDrawPoolHUD::renderFaceSelected(LLFace *facep, LLImageGL *image, const LLColor4 &color,
									   const S32 index_offset, const S32 index_count)
{
	facep->renderSelected(image, color, index_offset, index_count);
}

void LLDrawPoolHUD::resetDrawOrders()
{
	LLDrawPool::resetDrawOrders();
	mBumpFaces.clear();
	mAlphaFace.clear();
}

void LLDrawPoolHUD::dirtyTexture(const LLViewerImage *texturep)
{
	for (std::vector<LLFace*>::iterator iter = mReferences.begin();
		 iter != mReferences.end(); iter++)
	{	
		LLFace *facep = *iter;
		if (facep->getTexture() == texturep)
		{
			gPipeline.markTextured(facep->getDrawable());
		}
	}
}

void LLDrawPoolHUD::enqueue(LLFace *facep)
{
	if (facep->isState(LLFace::BACKLIST))
	{
		mMoveFace.put(facep);
	}
	else
	{
		const LLTextureEntry* te = facep->getTextureEntry();
		LLViewerImage* imagep = facep->getTexture();
		bool alpha = te->getColor().mV[3] < 0.999f;
		alpha = alpha || (imagep->getComponents() == 4) || (imagep->getComponents() == 2);
		if (alpha)
		{
			mAlphaFace.push_back(facep);
		}
		else if ((te->getBumpmap() || te->getShiny()))
		{
			mBumpFaces[imagep].push_back(facep);
		}
		else
		{
			mDrawFace.push_back(facep);
		}
	}
	facep->mDistance = facep->mCenterAgent[VX];
}

// virtual
BOOL LLDrawPoolHUD::addFace(LLFace *facep)
{
	facep->setState(LLFace::HUD_RENDER);
	return LLDrawPool::addFace(facep);
}

// virtual
BOOL LLDrawPoolHUD::removeFace(LLFace *facep)
{
	facep->clearState(LLFace::HUD_RENDER);	
	return LLDrawPool::removeFace(facep);
}

LLColor3 LLDrawPoolHUD::getDebugColor() const
{
	return LLColor3(1.f, 0.5f, 0.5f);
}

S32 LLDrawPoolHUD::getMaterialAttribIndex()
{
	switch (mRenderMode)
	{
	  case POOL_BUMP:
		return gPipeline.mObjectBumpProgram.mAttribute[LLPipeline::GLSL_MATERIAL_COLOR];
	  case POOL_ALPHA:
		return gPipeline.mObjectAlphaProgram.mAttribute[LLPipeline::GLSL_MATERIAL_COLOR];
	  case POOL_SIMPLE:
		return gPipeline.mObjectSimpleProgram.mAttribute[LLPipeline::GLSL_MATERIAL_COLOR];
	  default:
		return 0;
		break;
	}
}
