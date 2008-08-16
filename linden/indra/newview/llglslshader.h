/** 
 * @file llglslshader.h
 * @brief GLSL shader wrappers
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

#ifndef LL_LLGLSLSHADER_H
#define LL_LLGLSLSHADER_H

#include "llgl.h"

class LLShaderFeatures
{
public:
	bool calculatesLighting;
	bool calculatesAtmospherics;
	bool hasLighting; // implies no transport (it's possible to have neither though)
	bool isShiny;
	bool isFullbright; // implies no lighting
	bool isSpecular;
	bool hasWaterFog; // implies no gamma
	bool hasTransport; // implies no lighting (it's possible to have neither though)
	bool hasSkinning;	
	bool hasAtmospherics;
	bool hasGamma;

	// char numLights;
	
	LLShaderFeatures();
};

class LLGLSLShader
{
public:

	enum 
	{
		SG_DEFAULT = 0,
		SG_SKY,
		SG_WATER
	};
	
	LLGLSLShader();

	void unload();
	BOOL createShader(std::vector<std::string> * attributes,
						std::vector<std::string> * uniforms);
	BOOL attachObject(std::string object);
	void attachObject(GLhandleARB object);
	void attachObjects(GLhandleARB* objects = NULL, S32 count = 0);
	BOOL mapAttributes(const std::vector<std::string> * attributes);
	BOOL mapUniforms(const std::vector<std::string> * uniforms);
	void mapUniform(GLint index, const std::vector<std::string> * uniforms);
	void uniform1f(U32 index, GLfloat v);
	void uniform2f(U32 index, GLfloat x, GLfloat y);
	void uniform3f(U32 index, GLfloat x, GLfloat y, GLfloat z);
	void uniform4f(U32 index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
	void uniform1fv(U32 index, U32 count, const GLfloat* v);
	void uniform2fv(U32 index, U32 count, const GLfloat* v);
	void uniform3fv(U32 index, U32 count, const GLfloat* v);
	void uniform4fv(U32 index, U32 count, const GLfloat* v);
	void uniform1f(const std::string& uniform, GLfloat v);
	void uniform2f(const std::string& uniform, GLfloat x, GLfloat y);
	void uniform3f(const std::string& uniform, GLfloat x, GLfloat y, GLfloat z);
	void uniform4f(const std::string& uniform, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
	void uniform1fv(const std::string& uniform, U32 count, const GLfloat* v);
	void uniform2fv(const std::string& uniform, U32 count, const GLfloat* v);
	void uniform3fv(const std::string& uniform, U32 count, const GLfloat* v);
	void uniform4fv(const std::string& uniform, U32 count, const GLfloat* v);
	void uniformMatrix2fv(U32 index, U32 count, GLboolean transpose, const GLfloat *v);
	void uniformMatrix3fv(U32 index, U32 count, GLboolean transpose, const GLfloat *v);
	void uniformMatrix4fv(U32 index, U32 count, GLboolean transpose, const GLfloat *v);
	void uniformMatrix2fv(const std::string& uniform, U32 count, GLboolean transpose, const GLfloat *v);
	void uniformMatrix3fv(const std::string& uniform, U32 count, GLboolean transpose, const GLfloat *v);
	void uniformMatrix4fv(const std::string& uniform, U32 count, GLboolean transpose, const GLfloat *v);

	void vertexAttrib4f(U32 index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
	void vertexAttrib4fv(U32 index, GLfloat* v);
	
	GLint getUniformLocation(const std::string& uniform);
	
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
	std::vector<GLint> mAttribute; //lookup table of attribute enum to attribute channel
	std::vector<GLint> mUniform;   //lookup table of uniform enum to uniform location
	std::map<std::string, GLint> mUniformMap;  //lookup map of uniform name to uniform location
	std::map<GLint, LLVector4> mValue; //lookup map of uniform location to last known value
	std::vector<GLint> mTexture;
	S32 mActiveTextureChannels;
	S32 mShaderLevel;
	S32 mShaderGroup;
	BOOL mUniformsDirty;
	LLShaderFeatures mFeatures;
	std::vector< std::pair< std::string, GLenum > > mShaderFiles;
	std::string mName;
};

class LLShaderMgr
{
	friend class LLGLSLShader;

public:
	static void initAttribsAndUniforms(void);
	static BOOL attachShaderFeatures(LLGLSLShader * shader);
	static void setShaders();
	static void unloadShaders();
	static void dumpObjectLog(GLhandleARB ret, BOOL warns = TRUE);
	static BOOL	linkProgramObject(GLhandleARB obj, BOOL suppress_errors = FALSE);
	static BOOL	validateProgramObject(GLhandleARB obj);
	static GLhandleARB loadShaderFile(const LLString& filename, S32 & shader_level, GLenum type);
	static S32 getVertexShaderLevel(S32 type);
	static BOOL loadBasicShaders();
	static BOOL loadShadersEffects();
	static BOOL loadShadersObject();
	static BOOL loadShadersAvatar();
	static BOOL loadShadersEnvironment();
	static BOOL loadShadersWater();
	static BOOL loadShadersInterface();
	static BOOL loadShadersWindLight();

	// simple model of forward iterator
	// http://www.sgi.com/tech/stl/ForwardIterator.html
	class shader_iter
	{
		friend bool operator == (shader_iter const & a, shader_iter const & b);
		friend bool operator != (shader_iter const & a, shader_iter const & b);
	public:
		shader_iter() : mPtr(NULL)
		{
		}

		shader_iter(LLGLSLShader * const * ptr) : mPtr(ptr)
		{
		}

		LLGLSLShader & operator * () const
		{
			return **mPtr;
		}

		LLGLSLShader * operator -> () const
		{
			return *mPtr;
		}

		shader_iter & operator++ ()
		{
			++mPtr;
			return *this;
		}

		shader_iter operator++ (int)
		{
			return mPtr++;
		}

	private:
		LLGLSLShader * const * mPtr;
	};

	static shader_iter beginShaders()
	{
		return sShaderList;
	}

	static shader_iter endShaders()
	{
		return sShaderList + sNumShaders;
	}

	static S32	sVertexShaderLevel[];
	static S32	sMaxAvatarShaderLevel;

	enum EShaderClass
	{
		SHADER_LIGHTING,
		SHADER_OBJECT,
		SHADER_AVATAR,
		SHADER_ENVIRONMENT,
		SHADER_INTERFACE,
		SHADER_EFFECT,
		SHADER_WINDLIGHT,
		SHADER_WATER,
		SHADER_COUNT
	};

	typedef enum 
	{
		MATERIAL_COLOR = 0,
		SPECULAR_COLOR,
		BINORMAL,
		END_RESERVED_ATTRIBS
	} eGLSLReservedAttribs;
	
	typedef enum
	{
		DIFFUSE_MAP = 0,
		SPECULAR_MAP,
		BUMP_MAP,
		ENVIRONMENT_MAP,
		CLOUD_NOISE_MAP,
		FULLBRIGHT,
		LIGHTNORM,
		SUNLIGHT_COLOR,
		AMBIENT,
		BLUE_HORIZON,
		BLUE_DENSITY,
		HAZE_HORIZON,
		HAZE_DENSITY,
		CLOUD_SHADOW,
		DENSITY_MULTIPLIER,
		DISTANCE_MULTIPLIER,
		MAX_Y,
		GLOW,
		CLOUD_COLOR,
		CLOUD_POS_DENSITY1,
		CLOUD_POS_DENSITY2,
		CLOUD_SCALE,
		GAMMA,
		SCENE_LIGHT_STRENGTH,
		END_RESERVED_UNIFORMS
	} eGLSLReservedUniforms;

	typedef enum
	{
		SHINY_ORIGIN = END_RESERVED_UNIFORMS
	} eShinyUniforms;

	typedef enum
	{
		WATER_SCREENTEX = END_RESERVED_UNIFORMS,
		WATER_SCREENDEPTH,
		WATER_REFTEX,
		WATER_EYEVEC,
		WATER_TIME,
		WATER_WAVE_DIR1,
		WATER_WAVE_DIR2,
		WATER_LIGHT_DIR,
		WATER_SPECULAR,
		WATER_SPECULAR_EXP,
		WATER_FOGCOLOR,
		WATER_FOGDENSITY,
		WATER_REFSCALE,
		WATER_WATERHEIGHT,
	} eWaterUniforms;

	typedef enum
	{
		WL_CAMPOSLOCAL = END_RESERVED_UNIFORMS,
		WL_WATERHEIGHT
	} eWLUniforms;

	typedef enum
	{
		TERRAIN_DETAIL0 = END_RESERVED_UNIFORMS,
		TERRAIN_DETAIL1,
		TERRAIN_DETAIL2,
		TERRAIN_DETAIL3,
		TERRAIN_ALPHARAMP
	} eTerrainUniforms;

	typedef enum
	{
		GLOW_DELTA = END_RESERVED_UNIFORMS
	} eGlowUniforms;

	typedef enum
	{
		AVATAR_WEIGHT = END_RESERVED_ATTRIBS,
		AVATAR_CLOTHING,
		AVATAR_WIND,
		AVATAR_SINWAVE,
		AVATAR_GRAVITY
	} eAvatarAttribs;

	typedef enum
	{
		AVATAR_MATRIX = END_RESERVED_UNIFORMS
	} eAvatarUniforms;

private:

	// Map of shader names to compiled
	static std::map<std::string, GLhandleARB> sShaderObjects;

	//global (reserved slot) shader parameters
	static std::vector<std::string> sReservedAttribs;

	static std::vector<std::string> sReservedUniforms;

	static std::vector<std::string> sShinyUniforms;

	//water parameters
	static std::vector<std::string> sWaterUniforms;

	static std::vector<std::string> sWLUniforms;

	//terrain parameters
	static std::vector<std::string> sTerrainUniforms;

	//glow parameters
	static std::vector<std::string> sGlowUniforms;

	static std::vector<std::string> sGlowExtractUniforms;

	//avatar shader parameter tables
	static std::vector<std::string> sAvatarAttribs;

	static std::vector<std::string> sAvatarUniforms;
	// static std::vector< GLhandleARB > sBaseObjects;

	// the list of shaders we need to propagate parameters to.
	static LLGLSLShader * const sShaderList[];

	// the size of our shader list for convenience.
	static const size_t sNumShaders;

}; //LLShaderMgr

inline bool operator == (LLShaderMgr::shader_iter const & a, LLShaderMgr::shader_iter const & b)
{
	return a.mPtr == b.mPtr;
}

inline bool operator != (LLShaderMgr::shader_iter const & a, LLShaderMgr::shader_iter const & b)
{
	return a.mPtr != b.mPtr;
}


extern LLVector4			gShinyOrigin;

//object shaders
extern LLGLSLShader			gObjectSimpleProgram;
extern LLGLSLShader			gObjectSimpleWaterProgram;
extern LLGLSLShader			gObjectFullbrightProgram;
extern LLGLSLShader			gObjectFullbrightWaterProgram;

extern LLGLSLShader			gObjectSimpleLODProgram;
extern LLGLSLShader			gObjectFullbrightLODProgram;

extern LLGLSLShader			gObjectFullbrightShinyProgram;
extern LLGLSLShader			gObjectShinyProgram;
extern LLGLSLShader			gObjectShinyWaterProgram;

//environment shaders
extern LLGLSLShader			gTerrainProgram;
extern LLGLSLShader			gTerrainWaterProgram;
extern LLGLSLShader			gWaterProgram;
extern LLGLSLShader			gUnderWaterProgram;
extern LLGLSLShader			gGlowProgram;
extern LLGLSLShader			gGlowExtractProgram;

//interface shaders
extern LLGLSLShader			gHighlightProgram;

// avatar shader handles
extern LLGLSLShader			gAvatarProgram;
extern LLGLSLShader			gAvatarWaterProgram;
extern LLGLSLShader			gAvatarEyeballProgram;
extern LLGLSLShader			gAvatarPickProgram;

// WindLight shader handles
extern LLGLSLShader			gWLSkyProgram;
extern LLGLSLShader			gWLCloudProgram;

// Post Process Shaders
extern LLGLSLShader			gPostColorFilterProgram;
extern LLGLSLShader			gPostNightVisionProgram;

//current avatar shader parameter pointer
extern GLint				gAvatarMatrixParam;

#endif
