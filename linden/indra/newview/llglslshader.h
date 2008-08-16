/** 
 * @file llglslshader.h
 * @brief GLSL shader wrappers
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

#ifndef LL_LLGLSLSHADER_H
#define LL_LLGLSLSHADER_H

#include "llgl.h"

class LLGLSLShader
{
public:
	LLGLSLShader();

	void unload();
	void attachObject(GLhandleARB object);
	void attachObjects(GLhandleARB* objects = NULL, S32 count = 0);
	BOOL mapAttributes(const char** attrib_names = NULL, S32 count = 0);
	BOOL mapUniforms(const char** uniform_names = NULL,  S32 count = 0);
	void mapUniform(GLint index, const char** uniform_names = NULL,  S32 count = 0);
	void vertexAttrib4f(U32 index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
	void vertexAttrib4fv(U32 index, GLfloat* v);
	
	GLint mapUniformTextureChannel(GLint location, GLenum type);
	

	//enable/disable texture channel for specified uniform
	//if given texture uniform is active in the shader, 
	//the corresponding channel will be active upon return
	//returns channel texture is enabled in from [0-MAX)
	S32 enableTexture(S32 uniform, S32 mode = GL_TEXTURE_2D);
	S32 disableTexture(S32 uniform, S32 mode = GL_TEXTURE_2D); 
	
    BOOL link(BOOL suppress_errors = FALSE);
	void bind();
	void unbind();

	GLhandleARB mProgramObject;
	std::vector<GLint> mAttribute;
	std::vector<GLint> mUniform;
	std::vector<GLint> mTexture;
	S32 mActiveTextureChannels;
};

class LLScatterShader
{
public:
	static void init(GLhandleARB shader, int map_stage);
};

class LLShaderMgr
{
public:
	static void setShaders();
	static void unloadShaders();
	static void dumpObjectLog(GLhandleARB ret, BOOL warns = TRUE);
	static BOOL	linkProgramObject(GLhandleARB obj, BOOL suppress_errors = FALSE);
	static BOOL	validateProgramObject(GLhandleARB obj);
	static GLhandleARB loadShader(const LLString& filename, S32 cls, GLenum type);
	static S32 getVertexShaderLevel(S32 type);
	static S32 getMaxVertexShaderLevel(S32 type);
	static BOOL loadShadersLighting();
	static BOOL loadShadersObject();
	static BOOL loadShadersAvatar();
	static BOOL loadShadersEnvironment();
	static BOOL loadShadersInterface();
	static S32	sVertexShaderLevel[];
	static S32	sMaxVertexShaderLevel[];
	//global (reserved slot) shader parameters
	static const char* sReservedAttribs[];
	static U32 sReservedAttribCount;

	enum EShaderClass
	{
		SHADER_LIGHTING,
		SHADER_OBJECT,
		SHADER_AVATAR,
		SHADER_ENVIRONMENT,
		SHADER_INTERFACE,
		SHADER_COUNT
	};

	typedef enum 
	{
		MATERIAL_COLOR = 0,
		SPECULAR_COLOR,
		BINORMAL,
		END_RESERVED_ATTRIBS
	} eGLSLReservedAttribs;

	static const char* sReservedUniforms[];
	static U32 sReservedUniformCount;
	
	typedef enum
	{
		DIFFUSE_MAP = 0,
		SPECULAR_MAP,
		BUMP_MAP,
		ENVIRONMENT_MAP,
		END_RESERVED_UNIFORMS
	} eGLSLReservedUniforms;

	static const char* sShinyUniforms[];
	static U32 sShinyUniformCount;

	typedef enum
	{
		SHINY_ORIGIN = END_RESERVED_UNIFORMS
	} eShinyUniforms;

	//water parameters
	static const char* sWaterUniforms[];
	static U32 sWaterUniformCount;

	typedef enum
	{
		WATER_SCREENTEX = END_RESERVED_UNIFORMS,
		WATER_EYEVEC,
		WATER_TIME,
		WATER_WAVE_DIR1,
		WATER_WAVE_DIR2,
		WATER_LIGHT_DIR,
		WATER_SPECULAR,
		WATER_SPECULAR_EXP,
		WATER_FBSCALE,
		WATER_REFSCALE
	} eWaterUniforms;

	//terrain parameters
	static const char* sTerrainUniforms[];
	static U32 sTerrainUniformCount;

	typedef enum
	{
		TERRAIN_DETAIL0 = END_RESERVED_UNIFORMS,
		TERRAIN_DETAIL1,
		TERRAIN_ALPHARAMP
	} eTerrainUniforms;

	//glow parameters
	static const char* sGlowUniforms[];
	static U32 sGlowUniformCount;

	typedef enum
	{
		GLOW_DELTA = END_RESERVED_UNIFORMS
	} eGlowUniforms;

	//avatar shader parameter tables
	static const char* sAvatarAttribs[];
	static U32 sAvatarAttribCount;

	typedef enum
	{
		AVATAR_WEIGHT = END_RESERVED_ATTRIBS,
		AVATAR_CLOTHING,
		AVATAR_WIND,
		AVATAR_SINWAVE,
		AVATAR_GRAVITY
	} eAvatarAttribs;

	static const char* sAvatarUniforms[];
	static U32 sAvatarUniformCount;

	typedef enum
	{
		AVATAR_MATRIX = END_RESERVED_UNIFORMS
	} eAvatarUniforms;

}; //LLSL

//utility shader objects (not shader programs)
extern GLhandleARB			gLightVertex;
extern GLhandleARB			gLightFragment;
extern GLhandleARB			gScatterVertex;
extern GLhandleARB			gScatterFragment;

extern LLVector4			gShinyOrigin;

//object shaders
extern LLGLSLShader			gObjectSimpleProgram;
extern LLGLSLShader			gObjectAlphaProgram;
extern LLGLSLShader			gObjectBumpProgram;
extern LLGLSLShader			gObjectShinyProgram;

//environment shaders
extern LLGLSLShader			gTerrainProgram;
extern LLGLSLShader			gGlowProgram;
extern LLGLSLShader			gGroundProgram;
extern LLGLSLShader			gWaterProgram;

//interface shaders
extern LLGLSLShader			gHighlightProgram;

//avatar skinning utility shader object
extern GLhandleARB			gAvatarSkinVertex;

//avatar shader handles
extern LLGLSLShader			gAvatarProgram;
extern LLGLSLShader			gAvatarEyeballProgram;
extern LLGLSLShader			gAvatarPickProgram;

//current avatar shader parameter pointer
extern GLint				gAvatarMatrixParam;
extern GLint				gMaterialIndex;
extern GLint				gSpecularIndex;
	
#endif
