/** 
 * @file llglstates.h
 * @brief LLGL states definitions
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

//THIS HEADER SHOULD ONLY BE INCLUDED FROM llgl.h
#ifndef LL_LLGLSTATES_H
#define LL_LLGLSTATES_H

#ifdef WIN32
#	define WIN32_LEAN_AND_MEAN
#	include <winsock2.h>
#	include <windows.h>
#endif

#if LL_DARWIN
#include <AGL/gl.h>
#else
#include "llglheaders.h"
#endif

//----------------------------------------------------------------------------

class LLGLDepthTest
{
	// Enabled by default
public:
	LLGLDepthTest(GLboolean depth_enabled, GLboolean write_enabled = GL_TRUE, GLenum depth_func = GL_LEQUAL)
		: mPrevDepthEnabled(sDepthEnabled), mPrevDepthFunc(sDepthFunc), mPrevWriteEnabled(sWriteEnabled)
	{
		if (depth_enabled != sDepthEnabled)
		{
			if (depth_enabled) glEnable(GL_DEPTH_TEST);
			else glDisable(GL_DEPTH_TEST);
			sDepthEnabled = depth_enabled;
		}
		if (depth_func != sDepthFunc)
		{
			glDepthFunc(depth_func);
			sDepthFunc = depth_func;
		}
		if (write_enabled != sWriteEnabled)
		{
			glDepthMask(write_enabled);
			sWriteEnabled = write_enabled;
		}
	}
	~LLGLDepthTest()
	{
		if (sDepthEnabled != mPrevDepthEnabled )
		{
			if (mPrevDepthEnabled) glEnable(GL_DEPTH_TEST);
			else glDisable(GL_DEPTH_TEST);
			sDepthEnabled = mPrevDepthEnabled;
		}
		if (sDepthFunc != mPrevDepthFunc)
		{
			glDepthFunc(mPrevDepthFunc);
			sDepthFunc = mPrevDepthFunc;
		}
		if (sWriteEnabled != mPrevWriteEnabled )
		{
			glDepthMask(mPrevWriteEnabled);
			sWriteEnabled = mPrevWriteEnabled;
		}
	}
	GLboolean mPrevDepthEnabled;
	GLenum mPrevDepthFunc;
	GLboolean mPrevWriteEnabled;
private:
	static GLboolean sDepthEnabled; // defaults to GL_FALSE
	static GLenum sDepthFunc; // defaults to GL_LESS
	static GLboolean sWriteEnabled; // defaults to GL_TRUE
};

//----------------------------------------------------------------------------

class LLGLSDefault
{
protected:
	LLGLEnable mTexture2D, mColorMaterial;
	LLGLDisable mAlphaTest, mBlend, mCullFace, mDither, mFog, 
		mLineSmooth, mLineStipple, mNormalize, mPolygonSmooth,
		mTextureGenQ, mTextureGenR, mTextureGenS, mTextureGenT;
public:
	LLGLSDefault()
		:
		// Enable
		mTexture2D(GL_TEXTURE_2D),
		mColorMaterial(GL_COLOR_MATERIAL),
		// Disable
		mAlphaTest(GL_ALPHA_TEST),
		mBlend(GL_BLEND), 
		mCullFace(GL_CULL_FACE),
		mDither(GL_DITHER),
		mFog(GL_FOG), 
		mLineSmooth(GL_LINE_SMOOTH),
		mLineStipple(GL_LINE_STIPPLE),
		mNormalize(GL_NORMALIZE),
		mPolygonSmooth(GL_POLYGON_SMOOTH),
		mTextureGenQ(GL_TEXTURE_GEN_Q), 
		mTextureGenR(GL_TEXTURE_GEN_R),
		mTextureGenS(GL_TEXTURE_GEN_S), 
		mTextureGenT(GL_TEXTURE_GEN_T)
	{ }
};

class LLGLSTexture 
{
protected:
	LLGLEnable mTexture2D;
public:
	LLGLSTexture()
		: mTexture2D(GL_TEXTURE_2D)
	{}
};


class LLGLSNoTexture 
{
protected:
	LLGLDisable mTexture2D;
public:
	LLGLSNoTexture()
		: mTexture2D(GL_TEXTURE_2D)
	{}
};

class LLGLSObjectSelect // : public LLGLSDefault
{ 
protected:
	LLGLDisable mBlend, mFog, mTexture2D, mAlphaTest;
	LLGLEnable mCullFace;
public:
	LLGLSObjectSelect()
		: mBlend(GL_BLEND), mFog(GL_FOG), mTexture2D(GL_TEXTURE_2D), 
		  mAlphaTest(GL_ALPHA_TEST),
		  mCullFace(GL_CULL_FACE)
	{}
};

class LLGLSObjectSelectAlpha // : public LLGLSObjectSelect
{
protected:
	LLGLEnable mTexture2D, mAlphaTest;
public:
	LLGLSObjectSelectAlpha()
		: mTexture2D(GL_TEXTURE_2D), mAlphaTest(GL_ALPHA_TEST)
	{}
};

//----------------------------------------------------------------------------

class LLGLSUIDefault // : public LLGLSDefault
{ 
protected:
	LLGLEnable mBlend, mAlphaTest, mTexture2D;
	LLGLDisable mCullFace;
	LLGLDepthTest mDepthTest;
public:
	LLGLSUIDefault() 
		: mBlend(GL_BLEND), mAlphaTest(GL_ALPHA_TEST),
		  mTexture2D(GL_TEXTURE_2D), 
		  mCullFace(GL_CULL_FACE),
		  mDepthTest(GL_FALSE, GL_TRUE, GL_LEQUAL)
	{}
};

class LLGLSNoAlphaTest // : public LLGLSUIDefault
{
protected:
	LLGLDisable mAlphaTest;
public:
	LLGLSNoAlphaTest()
		: mAlphaTest(GL_ALPHA_TEST)
	{}
};

class LLGLSNoTextureNoAlphaTest // : public LLGLSUIDefault
{
protected:
	LLGLDisable mAlphaTest;
	LLGLDisable mTexture2D;
public:
	LLGLSNoTextureNoAlphaTest()
		: mAlphaTest(GL_ALPHA_TEST),
		  mTexture2D(GL_TEXTURE_2D)
	{}
};

//----------------------------------------------------------------------------

class LLGLSFog
{
protected:
	LLGLEnable mFog;
public:
	LLGLSFog()
		: mFog(GL_FOG)
	{}
};

class LLGLSNoFog
{
protected:
	LLGLDisable mFog;
public:
	LLGLSNoFog()
		: mFog(GL_FOG)
	{}
};

//----------------------------------------------------------------------------

class LLGLSPipeline // : public LLGLSDefault
{ 
protected:
	LLGLEnable mCullFace;
	LLGLDepthTest mDepthTest;
public:
	LLGLSPipeline()
		: mCullFace(GL_CULL_FACE),
		  mDepthTest(GL_TRUE, GL_TRUE, GL_LEQUAL)
	{ }		
};

class LLGLSPipelineAlpha // : public LLGLSPipeline
{ 
protected:
	LLGLEnable mBlend, mAlphaTest;
public:
	LLGLSPipelineAlpha()
		: mBlend(GL_BLEND),
		  mAlphaTest(GL_ALPHA_TEST)
	{ }
};

class LLGLSPipelineEmbossBump // : public LLGLSPipelineAlpha
{
protected:
	LLGLDisable mFog;
public:
	LLGLSPipelineEmbossBump()
		: mFog(GL_FOG)
	{ }
};

class LLGLSPipelineSelection // : public LLGLSPipelineAlpha
{ 
protected:
	LLGLDisable mCullFace;
public:
	LLGLSPipelineSelection()
		: mCullFace(GL_CULL_FACE)
	{}
};

class LLGLSPipelineAvatar // : public LLGLSPipeline
{
protected:
	LLGLEnable mNormalize;
public:
	LLGLSPipelineAvatar()
		: mNormalize(GL_NORMALIZE)
	{}
};

class LLGLSPipelineSkyBox // : public LLGLSPipeline
{ 
protected:
	LLGLDisable mAlphaTest, mCullFace, mFog;
public:
	LLGLSPipelineSkyBox()
		: mAlphaTest(GL_ALPHA_TEST), mCullFace(GL_CULL_FACE), mFog(GL_FOG)
	{ }
};

class LLGLSTracker // : public LLGLSDefault
{
protected:
	LLGLEnable mCullFace, mBlend, mAlphaTest;
	LLGLDisable mTexture2D;
public:
	LLGLSTracker() :
		mCullFace(GL_CULL_FACE),
		mBlend(GL_BLEND),
		mAlphaTest(GL_ALPHA_TEST),
		mTexture2D(GL_TEXTURE_2D)		
	{}
};

//----------------------------------------------------------------------------

class LLGLSSpecular
{
public:
	LLGLSSpecular(const LLColor4& color, F32 shininess)
	{
		if (shininess > 0.0f)
		{
			glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, color.mV);
			S32 shiny = (S32)(shininess*128.f);
			shiny = llclamp(shiny,0,128);
			glMateriali(GL_FRONT_AND_BACK, GL_SHININESS, shiny);
		}
	}
	~LLGLSSpecular()
	{
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, LLColor4(0.f,0.f,0.f,0.f).mV);
		glMateriali(GL_FRONT_AND_BACK, GL_SHININESS, 0);
	}
};

//----------------------------------------------------------------------------


#endif
