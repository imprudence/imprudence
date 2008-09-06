<<<<<<< .working
#error This file has been renamed llrender.cpp
=======
/** 
 * @file llglimmediate.cpp
 * @brief LLGLImmediate implementation
 *
 * $LicenseInfo:firstyear=2001&license=viewergpl$
 * 
 * Copyright (c) 2001-2008, Linden Research, Inc.
 * 
 * Second Life Viewer Source Code
 * The source code in this file ("Source Code") is provided by Linden Lab
 * to you under the terms of the GNU General Public License, version 2.0
 * ("GPL"), unless you have obtained a separate licensing agreement
 * ("Other License"), formally executed by you and Linden Lab.  Terms of
 * the GPL can be found in doc/GPL-license.txt in this distribution, or
 * online at http://secondlifegrid.net/programs/open_source/licensing/gplv2
 * 
 * There are special exceptions to the terms and conditions of the GPL as
 * it is applied to this Source Code. View the full text of the exception
 * in the file doc/FLOSS-exception.txt in this software distribution, or
 * online at http://secondlifegrid.net/programs/open_source/licensing/flossexception
 * 
 * By copying, modifying or distributing this software, you acknowledge
 * that you have read and understood your obligations described above,
 * and agree to abide by those obligations.
 * 
 * ALL LINDEN LAB SOURCE CODE IS PROVIDED "AS IS." LINDEN LAB MAKES NO
 * WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
 * COMPLETENESS OR PERFORMANCE.
 * $/LicenseInfo$
 */

#include "linden_common.h"

#include "llglimmediate.h"
#include "llvertexbuffer.h"

LLGLImmediate gGL;

const U32 immediate_mask = LLVertexBuffer::MAP_VERTEX | LLVertexBuffer::MAP_COLOR | LLVertexBuffer::MAP_TEXCOORD;

LLGLImmediate::LLGLImmediate()
{
	mCount = 0;
	mMode = LLVertexBuffer::TRIANGLES;
	mBuffer = new LLVertexBuffer(immediate_mask, 0);
	mBuffer->allocateBuffer(4096, 0, TRUE);
	mBuffer->getVertexStrider(mVerticesp);
	mBuffer->getTexCoordStrider(mTexcoordsp);
	mBuffer->getColorStrider(mColorsp);
}

void LLGLImmediate::translatef(const GLfloat& x, const GLfloat& y, const GLfloat& z)
{
	flush();
	glTranslatef(x,y,z);
}

void LLGLImmediate::scalef(const GLfloat& x, const GLfloat& y, const GLfloat& z)
{
	flush();
	glScalef(x,y,z);
}

void LLGLImmediate::pushMatrix()
{
	flush();
	glPushMatrix();
}

void LLGLImmediate::popMatrix()
{
	flush();
	glPopMatrix();
}

void LLGLImmediate::blendFunc(GLenum sfactor, GLenum dfactor)
{
	flush();
	glBlendFunc(sfactor, dfactor);
}

void LLGLImmediate::begin(const GLuint& mode)
{
	if (mode != mMode)
	{
		if (mMode == LLVertexBuffer::QUADS ||
			mMode == LLVertexBuffer::LINES ||
			mMode == LLVertexBuffer::TRIANGLES ||
			mMode == LLVertexBuffer::POINTS)
		{
			flush();
		}
		else if (mCount != 0)
		{
			llerrs << "gGL.begin() called redundantly." << llendl;
		}
		
		mMode = mode;
	}
}

void LLGLImmediate::end()
{ 
	if (mCount == 0)
	{
		return;
		//IMM_ERRS << "GL begin and end called with no vertices specified." << llendl;
	}

	if ((mMode != LLVertexBuffer::QUADS && 
		mMode != LLVertexBuffer::LINES &&
		mMode != LLVertexBuffer::TRIANGLES &&
		mMode != LLVertexBuffer::POINTS) ||
		mCount > 2048)
	{
		flush();
	}
}

void LLGLImmediate::flush()
{
	if (mCount > 0)
	{
#if 0
		if (!glIsEnabled(GL_VERTEX_ARRAY))
		{
			llerrs << "foo 1" << llendl;
		}

		if (!glIsEnabled(GL_COLOR_ARRAY))
		{
			llerrs << "foo 2" << llendl;
		}

		if (!glIsEnabled(GL_TEXTURE_COORD_ARRAY))
		{
			llerrs << "foo 3" << llendl;
		}

		if (glIsEnabled(GL_NORMAL_ARRAY))
		{
			llerrs << "foo 7" << llendl;
		}

		GLvoid* pointer;

		glGetPointerv(GL_VERTEX_ARRAY_POINTER, &pointer);
		if (pointer != &(mBuffer[0].v))
		{
			llerrs << "foo 4" << llendl;
		}

		glGetPointerv(GL_COLOR_ARRAY_POINTER, &pointer);
		if (pointer != &(mBuffer[0].c))
		{
			llerrs << "foo 5" << llendl;
		}

		glGetPointerv(GL_TEXTURE_COORD_ARRAY_POINTER, &pointer);
		if (pointer != &(mBuffer[0].uv))
		{
			llerrs << "foo 6" << llendl;
		}
#endif
				
		mBuffer->setBuffer(immediate_mask);
		mBuffer->drawArrays(mMode, 0, mCount);

		mVerticesp[0] = mVerticesp[mCount];
		mTexcoordsp[0] = mTexcoordsp[mCount];
		mColorsp[0] = mColorsp[mCount];
		mCount = 0;
	}
}

void LLGLImmediate::vertex3f(const GLfloat& x, const GLfloat& y, const GLfloat& z)
{ 
	if (mCount >= 4096)
	{
	//	llwarns << "GL immediate mode overflow.  Some geometry not drawn." << llendl;
		return;
	}

	mVerticesp[mCount] = LLVector3(x,y,z);
	mCount++;
	if (mCount < 4096)
	{
		mVerticesp[mCount] = mVerticesp[mCount-1];
		mColorsp[mCount] = mColorsp[mCount-1];
		mTexcoordsp[mCount] = mTexcoordsp[mCount-1];
	}
}

void LLGLImmediate::vertex2i(const GLint& x, const GLint& y)
{
	vertex3f((GLfloat) x, (GLfloat) y, 0);	
}

void LLGLImmediate::vertex2f(const GLfloat& x, const GLfloat& y)
{ 
	vertex3f(x,y,0);
}

void LLGLImmediate::vertex2fv(const GLfloat* v)
{ 
	vertex3f(v[0], v[1], 0);
}

void LLGLImmediate::vertex3fv(const GLfloat* v)
{
	vertex3f(v[0], v[1], v[2]);
}

void LLGLImmediate::texCoord2f(const GLfloat& x, const GLfloat& y)
{ 
	mTexcoordsp[mCount] = LLVector2(x,y);
}

void LLGLImmediate::texCoord2i(const GLint& x, const GLint& y)
{ 
	texCoord2f((GLfloat) x, (GLfloat) y);
}

void LLGLImmediate::texCoord2fv(const GLfloat* tc)
{ 
	texCoord2f(tc[0], tc[1]);
}

void LLGLImmediate::color4ub(const GLubyte& r, const GLubyte& g, const GLubyte& b, const GLubyte& a)
{
	mColorsp[mCount] = LLColor4U(r,g,b,a);
}

void LLGLImmediate::color4ubv(const GLubyte* c)
{
	color4ub(c[0], c[1], c[2], c[3]);
}

void LLGLImmediate::color4f(const GLfloat& r, const GLfloat& g, const GLfloat& b, const GLfloat& a)
{
	color4ub((GLubyte) (llclamp(r, 0.f, 1.f)*255),
		(GLubyte) (llclamp(g, 0.f, 1.f)*255),
		(GLubyte) (llclamp(b, 0.f, 1.f)*255),
		(GLubyte) (llclamp(a, 0.f, 1.f)*255));
}

void LLGLImmediate::color4fv(const GLfloat* c)
{ 
	color4f(c[0],c[1],c[2],c[3]);
}

void LLGLImmediate::color3f(const GLfloat& r, const GLfloat& g, const GLfloat& b)
{ 
	color4f(r,g,b,1);
}

void LLGLImmediate::color3fv(const GLfloat* c)
{ 
	color4f(c[0],c[1],c[2],1);
}

>>>>>>> .merge-right.r88690
