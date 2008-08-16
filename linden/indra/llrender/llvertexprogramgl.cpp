/** 
 * @file llvertexprogramgl.cpp
 * @brief LLVertexProgramGL base class
 *
 * Copyright (c) 2003-2007, Linden Research, Inc.
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

// This file contains the definition of LLVertexProgramGL, 
// for purposes of running vertex programs on GL hardware.

#include "linden_common.h"

#include "llvertexprogramgl.h"

#include "llglheaders.h"

LLVertexProgramGL::LLVertexProgramGL()
{
	mVertexProgramID = 0;
}

LLVertexProgramGL::~LLVertexProgramGL()
{
	glDeleteProgramsARB(1, (GLuint*) &mVertexProgramID);
}

BOOL LLVertexProgramGL::load(const char * filename)
{
	FILE *fp = fopen(filename, "r");
	if (!fp)
	{
		llwarns << "Unable to open vertex program " << filename << llendl;
		return FALSE;
	}

	fseek(fp,0,SEEK_END);
	S32 file_size = ftell(fp);
	fseek(fp,0,SEEK_SET);

	char *text_buffer = new char[file_size + 1];

	S32 num_read = (S32)fread(text_buffer, sizeof(char), file_size, fp);

	if (ferror(fp) || num_read == 0)
	{
		llwarns << "Unable to read vertex program " << filename << llendl;
		fclose(fp);
		delete[] text_buffer;
		return FALSE;
	}
	text_buffer[num_read] = '\0';
	fclose(fp);

	//Send program string to OpenGL
	glGenProgramsARB(1, (GLuint*) &mVertexProgramID);
	glBindProgramARB(GL_VERTEX_PROGRAM_ARB, mVertexProgramID);
	glProgramStringARB(GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, num_read, text_buffer);

	const GLubyte * program_error_string = glGetString(GL_PROGRAM_ERROR_STRING_ARB);

 	GLint error_pos;
	glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &error_pos);

	if(error_pos!=-1)
	{
		glGetError();
		S32 line_num = 1;
		char *next_token = strchr(text_buffer, '\n');
		while(NULL != next_token && next_token < (text_buffer + error_pos))
		{
			next_token++;
			line_num++;
			next_token = strchr(next_token, '\n');
		} 
		char bad_code[11];
		strncpy(bad_code, text_buffer + error_pos, 10);
		bad_code[10] = '\0';

		llerrs << filename << "(" << line_num << "): Vertex Program Error: "
			<< program_error_string << " at (" << bad_code<< ")"
			<< llendl;
		// clean up buffer
		delete[] text_buffer;
		return FALSE;
	}

	// clean up buffer
	delete[] text_buffer;
	return TRUE;
}

void LLVertexProgramGL::bind()
{
	glEnable(GL_VERTEX_PROGRAM_ARB);
	glBindProgramARB(GL_VERTEX_PROGRAM_ARB, mVertexProgramID);
}

void LLVertexProgramGL::unbind()
{
	glDisable(GL_VERTEX_PROGRAM_ARB);
}
