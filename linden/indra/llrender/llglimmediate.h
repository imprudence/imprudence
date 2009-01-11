<<<<<<< .working
#error This file has been renamed llrender.h
=======
/** 
 * @file llglimmediate.h
 * @brief LLGLImmediate definition
 *
 *	This class acts as a wrapper for OpenGL immediate calls concerning glBegin and glEnd.
 *	The goal of this class is to minimize the number of api calls due to legacy rendering
 *	code, and to define an interface for a multiple rendering API abstraction of the UI
 *	rendering.
 *
 * $LicenseInfo:firstyear=2001&license=viewergpl$
 * 
 * Copyright (c) 2001-2009, Linden Research, Inc.
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

#ifndef LL_LLGLIMMEDIATE_H
#define LL_LLGLIMMEDIATE_H

#include "stdtypes.h"
#include "llgltypes.h"
#include "llglheaders.h"
#include "llvertexbuffer.h"

class LLGLImmediate
{
public:
	LLGLImmediate();

	void translatef(const GLfloat& x, const GLfloat& y, const GLfloat& z);
	void scalef(const GLfloat& x, const GLfloat& y, const GLfloat& z);
	void pushMatrix();
	void popMatrix();
	void blendFunc(GLenum sfactor, GLenum dfactor);
	void flush();

	void begin(const GLuint& mode);
	void end();
	void vertex2i(const GLint& x, const GLint& y);
	void vertex2f(const GLfloat& x, const GLfloat& y);
	void vertex3f(const GLfloat& x, const GLfloat& y, const GLfloat& z);
	void vertex2fv(const GLfloat* v);
	void vertex3fv(const GLfloat* v);
	
	void texCoord2i(const GLint& x, const GLint& y);
	void texCoord2f(const GLfloat& x, const GLfloat& y);
	void texCoord2fv(const GLfloat* tc);

	void color4ub(const GLubyte& r, const GLubyte& g, const GLubyte& b, const GLubyte& a);
	void color4f(const GLfloat& r, const GLfloat& g, const GLfloat& b, const GLfloat& a);
	void color4fv(const GLfloat* c);
	void color3f(const GLfloat& r, const GLfloat& g, const GLfloat& b);
	void color3fv(const GLfloat* c);
	void color4ubv(const GLubyte* c);
	
	typedef struct
	{
		GLfloat v[3];
		GLubyte c[4];
		GLfloat uv[2];
	} Vertex;

private:
	U32 mCount;
	U32 mMode;
	LLPointer<LLVertexBuffer> mBuffer;
	LLStrider<LLVector3> mVerticesp;
	LLStrider<LLVector2> mTexcoordsp;
	LLStrider<LLColor4U> mColorsp;
};

extern LLGLImmediate gGL;

#endif
>>>>>>> .merge-right.r88690
