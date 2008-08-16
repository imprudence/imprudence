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

#ifdef LL_RELEASE_FOR_DOWNLOAD
#define IMM_ERRS llwarns
#else
#define IMM_ERRS llerrs
#endif

bool LLGLImmediate::sClever = false;
BOOL LLGLImmediate::sStarted = FALSE;

LLGLImmediate::LLGLImmediate()
{
	mCount = 0;
	mMode = GL_TRIANGLES;
	memset(mBuffer, 0, sizeof(Vertex)*4096);
}

void LLGLImmediate::start()
{
	if(sClever)
	{
		if (sStarted)
		{
			llerrs << "Redundant start." << llendl;
		}
		
		LLVertexBuffer::unbind();
		sStarted = TRUE;
		
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);

		const U32 stride = sizeof(Vertex);

		glVertexPointer(3, GL_FLOAT, stride, &(mBuffer[0].v));
		glTexCoordPointer(2, GL_FLOAT, stride, &(mBuffer[0].uv));
		glColorPointer(4, GL_UNSIGNED_BYTE, stride, &(mBuffer[0].c));
		
		color4f(1,1,1,1);
	}
}

void LLGLImmediate::stop()
{
	if (sClever)
	{
		if (!sStarted)
		{
			llerrs << "Redundant stop." << llendl;
		}
		
		flush();
		
		sStarted = FALSE;
		
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
	}
}

void LLGLImmediate::translatef(const GLfloat& x, const GLfloat& y, const GLfloat& z)
{
	flush();
	glTranslatef(x,y,z);
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
	if (sStarted)
	{
		flush();
	}
	glBlendFunc(sfactor, dfactor);
}

void LLGLImmediate::begin(const GLuint& mode)
{
	if (sClever)
	{
		if (mode != mMode)
		{
			if (mMode == GL_QUADS ||
				mMode == GL_LINES ||
				mMode == GL_TRIANGLES ||
				mMode == GL_POINTS)
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
	else
	{
		glBegin(mode);
	}
}

void LLGLImmediate::end()
{ 
	if (sClever)
	{
		if (mCount == 0)
		{
			IMM_ERRS << "GL begin and end called with no vertices specified." << llendl;
		}

		if ((mMode != GL_QUADS && 
			mMode != GL_LINES &&
			mMode != GL_TRIANGLES &&
			mMode != GL_POINTS) ||
			mCount > 2048)
		{
			flush();
		}
	}
	else
	{
		glEnd();
	}
}

void LLGLImmediate::flush()
{
	if (sClever)
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
			if (!sStarted)
			{
				llerrs << "Drawing call issued outside start/stop." << llendl;
			}
			glDrawArrays(mMode, 0, mCount);
			mBuffer[0] = mBuffer[mCount];
			mCount = 0;
		}
	}
}

void LLGLImmediate::vertex3f(const GLfloat& x, const GLfloat& y, const GLfloat& z)
{ 
	if (sClever)
	{
		if (mCount >= 4096)
		{
		//	llwarns << "GL immediate mode overflow.  Some geometry not drawn." << llendl;
			return;
		}

		mBuffer[mCount].v[0] = x;
		mBuffer[mCount].v[1] = y;
		mBuffer[mCount].v[2] = z;
		mCount++;
		if (mCount < 4096)
		{
			mBuffer[mCount] = mBuffer[mCount-1];
		}
	}
	else
	{
		glVertex3f(x,y,z);
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
	if (sClever)
	{
		mBuffer[mCount].uv[0] = x;
		mBuffer[mCount].uv[1] = y;
	}
	else
	{
		glTexCoord2f(x,y);
	}
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
	if (sClever)
	{
		mBuffer[mCount].c[0] = r;
		mBuffer[mCount].c[1] = g;
		mBuffer[mCount].c[2] = b;
		mBuffer[mCount].c[3] = a;
	}
	else
	{
		glColor4ub(r,g,b,a);
	}
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

void LLGLImmediate::setClever(bool do_clever)
{
	llassert(!sStarted);
	llassert(mCount == 0);

	sClever = do_clever;
}

