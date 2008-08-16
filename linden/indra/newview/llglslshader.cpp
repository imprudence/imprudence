/** 
 * @file llglslshader.cpp
 * @brief GLSL helper functions and state.
 *
 * $LicenseInfo:firstyear=2005&license=viewergpl$
 * 
 * Copyright (c) 2005-2008, Linden Research, Inc.
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

#include "llviewerprecompiledheaders.h"

#include "llfeaturemanager.h"
#include "llglslshader.h"

#include "llviewerwindow.h"
#include "llviewercontrol.h"
#include "pipeline.h"
#include "llworld.h"
#include "llwlparammanager.h"
#include "llwaterparammanager.h"
#include "llsky.h"
#include "llvosky.h"
#include "llglimmediate.h"

#if LL_DARWIN
#include "OpenGL/OpenGL.h"
#endif

#ifdef LL_RELEASE_FOR_DOWNLOAD
#define UNIFORM_ERRS llwarns
#else
#define UNIFORM_ERRS llerrs
#endif

// Lots of STL stuff in here, using namespace std to keep things more readable
using std::vector;
using std::pair;
using std::make_pair;
using std::string;

/*
//utility shader objects (not shader programs)
GLhandleARB			gSumLightsVertex;
GLhandleARB			gLightVertex;
GLhandleARB			gLightFuncVertex;
GLhandleARB			gLightFragment;
GLhandleARB			gWaterFogFragment;

//utility WindLight shader objects (not shader programs)
GLhandleARB			gWindLightVertex;
GLhandleARB			gWindLightFragment;
GLhandleARB			gGammaFragment;
*/

LLVector4			gShinyOrigin;

//object shaders
LLGLSLShader		gObjectSimpleProgram;
LLGLSLShader		gObjectSimpleWaterProgram;
LLGLSLShader		gObjectFullbrightProgram;
LLGLSLShader		gObjectFullbrightWaterProgram;

LLGLSLShader		gObjectFullbrightShinyProgram;
LLGLSLShader		gObjectShinyProgram;
LLGLSLShader		gObjectShinyWaterProgram;

//environment shaders
LLGLSLShader		gTerrainProgram;
LLGLSLShader		gTerrainWaterProgram;
LLGLSLShader		gWaterProgram;
LLGLSLShader		gUnderWaterProgram;

//interface shaders
LLGLSLShader		gHighlightProgram;

//avatar skinning utility shader object
GLhandleARB			gAvatarSkinVertex;

//avatar shader handles
LLGLSLShader		gAvatarProgram;
LLGLSLShader		gAvatarWaterProgram;
LLGLSLShader		gAvatarEyeballProgram;
LLGLSLShader		gAvatarPickProgram;

// WindLight shader handles
LLGLSLShader			gWLSkyProgram;
LLGLSLShader			gWLCloudProgram;

// Effects Shaders
LLGLSLShader			gGlowProgram;
LLGLSLShader			gGlowExtractProgram;
LLGLSLShader			gPostColorFilterProgram;
LLGLSLShader			gPostNightVisionProgram;

// Deferred rendering shaders
LLGLSLShader			gDeferredDiffuseProgram;

//current avatar shader parameter pointer
GLint				gAvatarMatrixParam;

S32	LLShaderMgr::sVertexShaderLevel[SHADER_COUNT] = { 0 };

S32	LLShaderMgr::sMaxAvatarShaderLevel = 0;

std::map<string, GLhandleARB> LLShaderMgr::sShaderObjects;
vector<string> LLShaderMgr::sReservedAttribs;
vector<string> LLShaderMgr::sWLUniforms;
vector<string> LLShaderMgr::sTerrainUniforms;
vector<string> LLShaderMgr::sReservedUniforms;
vector<string> LLShaderMgr::sShinyUniforms;
vector<string> LLShaderMgr::sWaterUniforms;
vector<string> LLShaderMgr::sGlowUniforms;
vector<string> LLShaderMgr::sGlowExtractUniforms;
vector<string> LLShaderMgr::sAvatarAttribs;
vector<string> LLShaderMgr::sAvatarUniforms;
//vector< GLhandleARB > LLShaderMgr::sBaseObjects;

/// Make sure WL Sky is the first program
LLGLSLShader * const LLShaderMgr::sShaderList[] = 
{
	&gWLSkyProgram,
	&gWLCloudProgram,
	&gAvatarProgram,
	&gObjectShinyProgram,
	&gWaterProgram,
	&gAvatarEyeballProgram, 
	&gObjectSimpleProgram,
	&gObjectFullbrightProgram,
	&gObjectFullbrightShinyProgram,
	&gTerrainProgram,
	&gTerrainWaterProgram,
	&gObjectSimpleWaterProgram,
	&gObjectFullbrightWaterProgram,
	&gAvatarWaterProgram,
	&gObjectShinyWaterProgram,
	&gUnderWaterProgram,
};
const size_t LLShaderMgr::sNumShaders = sizeof(sShaderList) / sizeof(sShaderList[0]);


BOOL shouldChange(const LLVector4& v1, const LLVector4& v2)
{
	/*F32 dot = v1.mV[0] * v2.mV[0] +
				v1.mV[1] * v2.mV[1] +
				v1.mV[2] * v2.mV[2] +
				v1.mV[3] * v2.mV[3];

	F32 mag = v1.mV[0] * v1.mV[0] +
				v1.mV[1] * v1.mV[1] +
				v1.mV[2] * v1.mV[2] +
				v1.mV[3] * v1.mV[3];

	F32 val = (dot/mag);

	if (val > 2.0f || val < 0.1f)
	{
		return TRUE;
	}

	return FALSE;*/

	return v1 != v2;
}

LLShaderFeatures::LLShaderFeatures()
: calculatesLighting(false), isShiny(false), isFullbright(false), hasWaterFog(false),
hasTransport(false), hasSkinning(false), hasAtmospherics(false), isSpecular(false),
hasGamma(false), hasLighting(false), calculatesAtmospherics(false)
{
}

void LLShaderMgr::initAttribsAndUniforms(void)
{
	if (sReservedAttribs.empty())
	{
		sReservedAttribs.push_back("materialColor");
		sReservedAttribs.push_back("specularColor");
		sReservedAttribs.push_back("binormal");

		sAvatarAttribs.reserve(5);
		sAvatarAttribs.push_back("weight");
		sAvatarAttribs.push_back("clothing");
		sAvatarAttribs.push_back("gWindDir");
		sAvatarAttribs.push_back("gSinWaveParams");
		sAvatarAttribs.push_back("gGravity");

		sAvatarUniforms.push_back("matrixPalette");

		sReservedUniforms.reserve(24);
		sReservedUniforms.push_back("diffuseMap");
		sReservedUniforms.push_back("specularMap");
		sReservedUniforms.push_back("bumpMap");
		sReservedUniforms.push_back("environmentMap");
		sReservedUniforms.push_back("cloude_noise_texture");
		sReservedUniforms.push_back("fullbright");
		sReservedUniforms.push_back("lightnorm");
		sReservedUniforms.push_back("sunlight_color");
		sReservedUniforms.push_back("ambient");
		sReservedUniforms.push_back("blue_horizon");
		sReservedUniforms.push_back("blue_density");
		sReservedUniforms.push_back("haze_horizon");
		sReservedUniforms.push_back("haze_density");
		sReservedUniforms.push_back("cloud_shadow");
		sReservedUniforms.push_back("density_multiplier");
		sReservedUniforms.push_back("distance_multiplier");
		sReservedUniforms.push_back("max_y");
		sReservedUniforms.push_back("glow");
		sReservedUniforms.push_back("cloud_color");
		sReservedUniforms.push_back("cloud_pos_density1");
		sReservedUniforms.push_back("cloud_pos_density2");
		sReservedUniforms.push_back("cloud_scale");
		sReservedUniforms.push_back("gamma");
		sReservedUniforms.push_back("scene_light_strength");

		sWLUniforms.push_back("camPosLocal");

		sTerrainUniforms.reserve(5);
		sTerrainUniforms.push_back("detail_0");
		sTerrainUniforms.push_back("detail_1");
		sTerrainUniforms.push_back("detail_2");
		sTerrainUniforms.push_back("detail_3");
		sTerrainUniforms.push_back("alpha_ramp");

		sGlowUniforms.push_back("glowDelta");
		sGlowUniforms.push_back("glowStrength");

		sGlowExtractUniforms.push_back("minLuminance");
		sGlowExtractUniforms.push_back("maxExtractAlpha");
		sGlowExtractUniforms.push_back("lumWeights");
		sGlowExtractUniforms.push_back("warmthWeights");
		sGlowExtractUniforms.push_back("warmthAmount");

		sShinyUniforms.push_back("origin");

		sWaterUniforms.reserve(12);
		sWaterUniforms.push_back("screenTex");
		sWaterUniforms.push_back("screenDepth");
		sWaterUniforms.push_back("refTex");
		sWaterUniforms.push_back("eyeVec");
		sWaterUniforms.push_back("time");
		sWaterUniforms.push_back("d1");
		sWaterUniforms.push_back("d2");
		sWaterUniforms.push_back("lightDir");
		sWaterUniforms.push_back("specular");
		sWaterUniforms.push_back("lightExp");
		sWaterUniforms.push_back("fogCol");
		sWaterUniforms.push_back("kd");
		sWaterUniforms.push_back("refScale");
		sWaterUniforms.push_back("waterHeight");
	}	
}

BOOL LLShaderMgr::attachShaderFeatures(LLGLSLShader * shader)
{
	llassert_always(shader != NULL);
	LLShaderFeatures *features = & shader->mFeatures;
	
	//////////////////////////////////////
	// Attach Vertex Shader Features First
	//////////////////////////////////////
	
	// NOTE order of shader object attaching is VERY IMPORTANT!!!
	if (features->calculatesAtmospherics)
	{
		if (!shader->attachObject("windlight/atmosphericsVarsV.glsl"))
		{
			return FALSE;
		}
	}

	if (features->calculatesLighting)
	{
		if (!shader->attachObject("windlight/atmosphericsHelpersV.glsl"))
		{
			return FALSE;
		}
		
		if (features->isSpecular)
		{
			if (!shader->attachObject("lighting/lightFuncSpecularV.glsl"))
			{
				return FALSE;
			}
			
			if (!shader->attachObject("lighting/sumLightsSpecularV.glsl"))
			{
				return FALSE;
			}
			
			if (!shader->attachObject("lighting/lightSpecularV.glsl"))
			{
				return FALSE;
			}
		}
		else 
		{
			if (!shader->attachObject("lighting/lightFuncV.glsl"))
			{
				return FALSE;
			}
			
			if (!shader->attachObject("lighting/sumLightsV.glsl"))
			{
				return FALSE;
			}
			
			if (!shader->attachObject("lighting/lightV.glsl"))
			{
				return FALSE;
			}
		}
	}
	
	// NOTE order of shader object attaching is VERY IMPORTANT!!!
	if (features->calculatesAtmospherics)
	{
		if (!shader->attachObject("windlight/atmosphericsV.glsl"))
		{
			return FALSE;
		}
	}

	if (features->hasSkinning)
	{
		if (!shader->attachObject("avatar/avatarSkinV.glsl"))
		{
			return FALSE;
		}
	}
	
	///////////////////////////////////////
	// Attach Fragment Shader Features Next
	///////////////////////////////////////

	if(features->calculatesAtmospherics)
	{
		if (!shader->attachObject("windlight/atmosphericsVarsF.glsl"))
		{
			return FALSE;
		}
	}

	// NOTE order of shader object attaching is VERY IMPORTANT!!!
	if (features->hasGamma)
	{
		if (!shader->attachObject("windlight/gammaF.glsl"))
		{
			return FALSE;
		}
	}
	
	if (features->hasAtmospherics)
	{
		if (!shader->attachObject("windlight/atmosphericsF.glsl"))
		{
			return FALSE;
		}
	}
	
	if (features->hasTransport)
	{
		if (!shader->attachObject("windlight/transportF.glsl"))
		{
			return FALSE;
		}

		// Test hasFullbright and hasShiny and attach fullbright and 
		// fullbright shiny atmos transport if we split them out.
	}

	// NOTE order of shader object attaching is VERY IMPORTANT!!!
	if (features->hasWaterFog)
	{
		if (!shader->attachObject("environment/waterFogF.glsl"))
		{
			return FALSE;
		}
	}
	
	if (features->hasLighting)
	{
	
		if (features->hasWaterFog)
		{
			if (!shader->attachObject("lighting/lightWaterF.glsl"))
			{
				return FALSE;
			}
		}
		
		else
		{
			if (!shader->attachObject("lighting/lightF.glsl"))
			{
				return FALSE;
			}
		}		
	}
	
	// NOTE order of shader object attaching is VERY IMPORTANT!!!
	else if (features->isFullbright)
	{
	
		if (features->hasWaterFog)
		{
			if (!shader->attachObject("lighting/lightFullbrightWaterF.glsl"))
			{
				return FALSE;
			}
		}
		
		else if (features->isShiny)
		{
			if (!shader->attachObject("lighting/lightFullbrightShinyF.glsl"))
			{
				return FALSE;
			}
		}
		
		else
		{
			if (!shader->attachObject("lighting/lightFullbrightF.glsl"))
			{
				return FALSE;
			}
		}
	}

	// NOTE order of shader object attaching is VERY IMPORTANT!!!
	else if (features->isShiny)
	{
	
		if (features->hasWaterFog)
		{
			if (!shader->attachObject("lighting/lightShinyWaterF.glsl"))
			{
				return FALSE;
			}
		}
		
		else 
		{
			if (!shader->attachObject("lighting/lightShinyF.glsl"))
			{
				return FALSE;
			}
		}
	}
	return TRUE;
}

//============================================================================
// Set Levels

S32 LLShaderMgr::getVertexShaderLevel(S32 type)
{
	return LLPipeline::sDisableShaders ? 0 : sVertexShaderLevel[type];
}


//============================================================================
// Load Shader

static LLString get_object_log(GLhandleARB ret)
{
	LLString res;
	
	//get log length 
	GLint length;
	glGetObjectParameterivARB(ret, GL_OBJECT_INFO_LOG_LENGTH_ARB, &length);
	if (length > 0)
	{
		//the log could be any size, so allocate appropriately
		GLcharARB* log = new GLcharARB[length];
		glGetInfoLogARB(ret, length, &length, log);
		res = LLString((char *)log);
		delete[] log;
	}
	return res;
}

void LLShaderMgr::dumpObjectLog(GLhandleARB ret, BOOL warns) 
{
	LLString log = get_object_log(ret);
	if (warns)
	{
		llwarns << log << llendl;
	}
	else
	{
		llinfos << log << llendl;
	}
}

GLhandleARB LLShaderMgr::loadShaderFile(const LLString& filename, S32 & shader_level, GLenum type)
{
	GLenum error;
	error = glGetError();
	if (error != GL_NO_ERROR)
	{
		llwarns << "GL ERROR entering loadShaderFile(): " << error << llendl;
	}
	
	llinfos << "Loading shader file: " << filename << " class " << shader_level << llendl;

	if (filename.empty()) 
	{
		return 0;
	}


	//read in from file
	FILE* file = NULL;

	S32 try_gpu_class = shader_level;
	S32 gpu_class;

	//find the most relevant file
	for (gpu_class = try_gpu_class; gpu_class > 0; gpu_class--)
	{	//search from the current gpu class down to class 1 to find the most relevant shader
		std::stringstream fname;
		fname << gDirUtilp->getExpandedFilename(LL_PATH_APP_SETTINGS, "shaders/class");
		fname << gpu_class << "/" << filename;
		
 		llinfos << "Looking in " << fname.str().c_str() << llendl;
		file = fopen(fname.str().c_str(), "r");		/* Flawfinder: ignore */
		if (file)
		{
#if !LL_RELEASE_FOR_DOWNLOAD
			llinfos << "Found shader file: " << fname.str() << llendl;
#endif
			break; // done
		}
	}
	
	if (file == NULL)
	{
		llinfos << "GLSL Shader file not found: " << filename << llendl;
		return 0;
	}

	//we can't have any lines longer than 1024 characters 
	//or any shaders longer than 1024 lines... deal - DaveP
	GLcharARB buff[1024];
	GLcharARB* text[1024];
	GLuint count = 0;


	//copy file into memory
	while(fgets((char *)buff, 1024, file) != NULL && count < (sizeof(buff)/sizeof(buff[0]))) 
	{
		text[count++] = (GLcharARB *)strdup((char *)buff); 
	}
	fclose(file);

	//create shader object
	GLhandleARB ret = glCreateShaderObjectARB(type);
	error = glGetError();
	if (error != GL_NO_ERROR)
	{
		llwarns << "GL ERROR in glCreateShaderObjectARB: " << error << llendl;
	}
	else
	{
		//load source
		glShaderSourceARB(ret, count, (const GLcharARB**) text, NULL);
		error = glGetError();
		if (error != GL_NO_ERROR)
		{
			llwarns << "GL ERROR in glShaderSourceARB: " << error << llendl;
		}
		else
		{
			//compile source
			glCompileShaderARB(ret);
			error = glGetError();
			if (error != GL_NO_ERROR)
			{
				llwarns << "GL ERROR in glCompileShaderARB: " << error << llendl;
			}
		}
	}
	//free memory
	for (GLuint i = 0; i < count; i++)
	{
		free(text[i]);
	}
	if (error == GL_NO_ERROR)
	{
		//check for errors
		GLint success = GL_TRUE;
		glGetObjectParameterivARB(ret, GL_OBJECT_COMPILE_STATUS_ARB, &success);
		error = glGetError();
		if (error != GL_NO_ERROR || success == GL_FALSE) 
		{
			//an error occured, print log
			llwarns << "GLSL Compilation Error: (" << error << ") in " << filename << llendl;
			dumpObjectLog(ret);
			ret = 0;
		}
	}
	else
	{
		ret = 0;
	}
	stop_glerror();

	//successfully loaded, save results
	if (ret)
	{
		// Add shader file to map
		sShaderObjects[filename] = ret;
		shader_level = try_gpu_class;
	}
	else
	{
		if (shader_level > 1)
		{
			shader_level--;
			return loadShaderFile(filename,shader_level,type);
		}
		llwarns << "Failed to load " << filename << llendl;	
	}
	return ret;
}

BOOL LLShaderMgr::linkProgramObject(GLhandleARB obj, BOOL suppress_errors) 
{
	//check for errors
	glLinkProgramARB(obj);
	GLint success = GL_TRUE;
	glGetObjectParameterivARB(obj, GL_OBJECT_LINK_STATUS_ARB, &success);
	if (!suppress_errors && success == GL_FALSE) 
	{
		//an error occured, print log
		llwarns << "GLSL Linker Error:" << llendl;
	}

// NOTE: Removing LL_DARWIN block as it doesn't seem to actually give the correct answer, 
// but want it for reference once I move it.
#if 0
	// Force an evaluation of the gl state so the driver can tell if the shader will run in hardware or software
	// per Apple's suggestion   
	glBegin(gGL.mMode);
	glEnd();

	// Query whether the shader can or cannot run in hardware
	// http://developer.apple.com/qa/qa2007/qa1502.html
	long vertexGPUProcessing;
	CGLContextObj ctx = CGLGetCurrentContext();
	CGLGetParameter (ctx, kCGLCPGPUVertexProcessing, &vertexGPUProcessing);	
	long fragmentGPUProcessing;
	CGLGetParameter (ctx, kCGLCPGPUFragmentProcessing, &fragmentGPUProcessing);
	if (!fragmentGPUProcessing || !vertexGPUProcessing)
	{
		llwarns << "GLSL Linker: Running in Software:" << llendl;
		success = GL_FALSE;
		suppress_errors = FALSE;		
	}
	
#else
	LLString log = get_object_log(obj);
	LLString::toLower(log);
	if (log.find("software") != LLString::npos)
	{
		llwarns << "GLSL Linker: Running in Software:" << llendl;
		success = GL_FALSE;
		suppress_errors = FALSE;
	}
#endif
	if (!suppress_errors)
	{
        dumpObjectLog(obj, !success);
	}

	return success;
}

BOOL LLShaderMgr::validateProgramObject(GLhandleARB obj)
{
	//check program validity against current GL
	glValidateProgramARB(obj);
	GLint success = GL_TRUE;
	glGetObjectParameterivARB(obj, GL_OBJECT_VALIDATE_STATUS_ARB, &success);
	if (success == GL_FALSE)
	{
		llwarns << "GLSL program not valid: " << llendl;
		dumpObjectLog(obj);
	}
	else
	{
		dumpObjectLog(obj, FALSE);
	}

	return success;
}

//============================================================================
// Shader Management

void LLShaderMgr::setShaders()
{
	if (!gPipeline.mInitialized)
	{
		return;
	}
	// Make sure the compiled shader map is cleared before we recompile shaders.
	sShaderObjects.clear();
	
	initAttribsAndUniforms();
	gPipeline.releaseGLBuffers();

	if (gSavedSettings.getBOOL("VertexShaderEnable"))
	{
		LLPipeline::sDynamicReflections = gSavedSettings.getBOOL("RenderDynamicReflections") && gGLManager.mHasCubeMap && LLFeatureManager::getInstance()->isFeatureAvailable("RenderCubeMap");
		LLPipeline::sWaterReflections = gGLManager.mHasCubeMap;
		LLPipeline::sRenderGlow = gSavedSettings.getBOOL("RenderGlow"); 
	}
	else
	{
		LLPipeline::sDynamicReflections = 
			LLPipeline::sRenderGlow = 
			LLPipeline::sWaterReflections = FALSE;
	}
	
	//hack to reset buffers that change behavior with shaders
	gPipeline.resetVertexBuffers();

	if (gViewerWindow)
	{
		gViewerWindow->setCursor(UI_CURSOR_WAIT);
	}

	// Lighting
	gPipeline.setLightingDetail(-1);

	// Shaders
	llinfos << "\n~~~~~~~~~~~~~~~~~~\n Loading Shaders:\n~~~~~~~~~~~~~~~~~~" << llendl;
	for (S32 i = 0; i < SHADER_COUNT; i++)
	{
		sVertexShaderLevel[i] = 0;
	}
	sMaxAvatarShaderLevel = 0;

	if (LLFeatureManager::getInstance()->isFeatureAvailable("VertexShaderEnable") 
		&& gSavedSettings.getBOOL("VertexShaderEnable"))
	{
		S32 light_class = 2;
		S32 env_class = 2;
		S32 obj_class = 2;
		S32 effect_class = 2;
		S32 wl_class = 2;
		S32 water_class = 2;
		S32 deferred_class = 0;
		if (!gSavedSettings.getBOOL("WindLightUseAtmosShaders"))
		{
			// user has disabled WindLight in their settings, downgrade
			// windlight shaders to stub versions.
			wl_class = 1;

			// if class one or less, turn off more shaders
			// since higher end cards won't see any real gain
			// from turning off most of the shaders,
			// but class one would
			// TODO: Make water on class one cards color things
			// beneath it properly
			if(LLFeatureManager::getInstance()->getGPUClass() < GPU_CLASS_2)
			{
				// use lesser water and other stuff
				light_class = 2;
				env_class = 0;
				obj_class = 0;
				effect_class = 1;
				water_class = 1;
			}
		}

		if (gSavedSettings.getBOOL("RenderDeferred"))
		{
			light_class = 1;
			env_class = 0;
			obj_class = 0;
			water_class = 1;
			effect_class = 1;
			deferred_class = 1;
		}

		if(!gSavedSettings.getBOOL("EnableRippleWater"))
		{
			water_class = 0;
		}

		// Trigger a full rebuild of the fallback skybox / cubemap if we've toggled windlight shaders
		if (sVertexShaderLevel[SHADER_WINDLIGHT] != wl_class && gSky.mVOSkyp.notNull())
		{
			gSky.mVOSkyp->forceSkyUpdate();
		}

		// Load lighting shaders
		sVertexShaderLevel[SHADER_LIGHTING] = light_class;
		sVertexShaderLevel[SHADER_INTERFACE] = light_class;
		sVertexShaderLevel[SHADER_ENVIRONMENT] = env_class;
		sVertexShaderLevel[SHADER_WATER] = water_class;
		sVertexShaderLevel[SHADER_OBJECT] = obj_class;
		sVertexShaderLevel[SHADER_EFFECT] = effect_class;
		sVertexShaderLevel[SHADER_WINDLIGHT] = wl_class;
		sVertexShaderLevel[SHADER_DEFERRED] = deferred_class;

		BOOL loaded = loadBasicShaders();

		if (loaded)
		{
			gPipeline.mVertexShadersEnabled = TRUE;
			gPipeline.mVertexShadersLoaded = 1;

			// Load all shaders to set max levels
			loadShadersEnvironment();
			loadShadersWater();
			loadShadersObject();
			loadShadersWindLight();
			loadShadersEffects();
			loadShadersInterface();
			loadShadersDeferred();

			// Load max avatar shaders to set the max level
			sVertexShaderLevel[SHADER_AVATAR] = 3;
			sMaxAvatarShaderLevel = 3;
			loadShadersAvatar();
			
#if 0 && LL_DARWIN // force avatar shaders off for mac
			sVertexShaderLevel[SHADER_AVATAR] = 0;
			sMaxAvatarShaderLevel = 0;
#else
			if (gSavedSettings.getBOOL("RenderAvatarVP"))
			{
				BOOL avatar_cloth = gSavedSettings.getBOOL("RenderAvatarCloth");
				S32 avatar_class = 1;
				
				// cloth is a class3 shader
				if(avatar_cloth)
				{
					avatar_class = 3;
				}

				// Set the actual level
				sVertexShaderLevel[SHADER_AVATAR] = avatar_class;
				loadShadersAvatar();
				if (sVertexShaderLevel[SHADER_AVATAR] != avatar_class)
				{
					if (sVertexShaderLevel[SHADER_AVATAR] == 0)
					{
						gSavedSettings.setBOOL("RenderAvatarVP", FALSE);
					}
					if(llmax(sVertexShaderLevel[SHADER_AVATAR]-1,0) >= 3)
					{
						avatar_cloth = true;
					}
					else
					{
						avatar_cloth = false;
					}
					gSavedSettings.setBOOL("RenderAvatarCloth", avatar_cloth);
				}
			}
			else
			{
				sVertexShaderLevel[SHADER_AVATAR] = 0;
				gSavedSettings.setBOOL("RenderAvatarCloth", FALSE);
				loadShadersAvatar(); // unloads
			}
#endif
		}
		else
		{
			gPipeline.mVertexShadersEnabled = FALSE;
			gPipeline.mVertexShadersLoaded = 0;
			sVertexShaderLevel[SHADER_LIGHTING] = 0;
			sVertexShaderLevel[SHADER_INTERFACE] = 0;
			sVertexShaderLevel[SHADER_ENVIRONMENT] = 0;
			sVertexShaderLevel[SHADER_WATER] = 0;
			sVertexShaderLevel[SHADER_OBJECT] = 0;
			sVertexShaderLevel[SHADER_EFFECT] = 0;
			sVertexShaderLevel[SHADER_WINDLIGHT] = 0;
		}
	}
	else
	{
		gPipeline.mVertexShadersEnabled = FALSE;
		gPipeline.mVertexShadersLoaded = 0;
		sVertexShaderLevel[SHADER_LIGHTING] = 0;
		sVertexShaderLevel[SHADER_INTERFACE] = 0;
		sVertexShaderLevel[SHADER_ENVIRONMENT] = 0;
		sVertexShaderLevel[SHADER_WATER] = 0;
		sVertexShaderLevel[SHADER_OBJECT] = 0;
		sVertexShaderLevel[SHADER_EFFECT] = 0;
		sVertexShaderLevel[SHADER_WINDLIGHT] = 0;
	}
	
	if (gViewerWindow)
	{
		gViewerWindow->setCursor(UI_CURSOR_ARROW);
	}
	gPipeline.createGLBuffers();
}

void LLShaderMgr::unloadShaders()
{
	gObjectSimpleProgram.unload();
	gObjectSimpleWaterProgram.unload();
	gObjectFullbrightProgram.unload();
	gObjectFullbrightWaterProgram.unload();

	gObjectShinyProgram.unload();
	gObjectFullbrightShinyProgram.unload();
	gObjectShinyWaterProgram.unload();
	gWaterProgram.unload();
	gUnderWaterProgram.unload();
	gTerrainProgram.unload();
	gTerrainWaterProgram.unload();
	gGlowProgram.unload();
	gGlowExtractProgram.unload();
	gAvatarProgram.unload();
	gAvatarWaterProgram.unload();
	gAvatarEyeballProgram.unload();
	gAvatarPickProgram.unload();
	gHighlightProgram.unload();

	gWLSkyProgram.unload();
	gWLCloudProgram.unload();

	gPostColorFilterProgram.unload();
	gPostNightVisionProgram.unload();

	gDeferredDiffuseProgram.unload();

	sVertexShaderLevel[SHADER_LIGHTING] = 0;
	sVertexShaderLevel[SHADER_OBJECT] = 0;
	sVertexShaderLevel[SHADER_AVATAR] = 0;
	sVertexShaderLevel[SHADER_ENVIRONMENT] = 0;
	sVertexShaderLevel[SHADER_WATER] = 0;
	sVertexShaderLevel[SHADER_INTERFACE] = 0;

	gPipeline.mVertexShadersLoaded = 0;
}

BOOL LLShaderMgr::loadBasicShaders()
{
	// Load basic dependency shaders first
	// All of these have to load for any shaders to function
	
#if LL_DARWIN // Mac can't currently handle all 8 lights, 
	S32 sum_lights_class = 2;
#else 
	S32 sum_lights_class = 3;

	// class one cards will get the lower sum lights
	// class zero we're not going to think about
	// since a class zero card COULD be a ridiculous new card
	// and old cards should have the features masked
	if(LLFeatureManager::getInstance()->getGPUClass() == GPU_CLASS_1)
	{
		sum_lights_class = 2;
	}
#endif

	// If we have sun and moon only checked, then only sum those lights.
	if (gPipeline.getLightingDetail() == 0)
	{
		sum_lights_class = 1;
	}

	// Load the Basic Vertex Shaders at the appropriate level. 
	// (in order of shader function call depth for reference purposes, deepest level first)

	vector< pair<string, S32> > shaders;
	shaders.reserve(10);
	shaders.push_back( make_pair( "windlight/atmosphericsVarsV.glsl",		sVertexShaderLevel[SHADER_WINDLIGHT] ) );
	shaders.push_back( make_pair( "windlight/atmosphericsHelpersV.glsl",	sVertexShaderLevel[SHADER_WINDLIGHT] ) );
	shaders.push_back( make_pair( "lighting/lightFuncV.glsl",				sVertexShaderLevel[SHADER_LIGHTING] ) );
	shaders.push_back( make_pair( "lighting/sumLightsV.glsl",				sum_lights_class ) );
	shaders.push_back( make_pair( "lighting/lightV.glsl",					sVertexShaderLevel[SHADER_LIGHTING] ) );
	shaders.push_back( make_pair( "lighting/lightFuncSpecularV.glsl",		sVertexShaderLevel[SHADER_LIGHTING] ) );
	shaders.push_back( make_pair( "lighting/sumLightsSpecularV.glsl",		sum_lights_class ) );
	shaders.push_back( make_pair( "lighting/lightSpecularV.glsl",			sVertexShaderLevel[SHADER_LIGHTING] ) );
	shaders.push_back( make_pair( "windlight/atmosphericsV.glsl",			sVertexShaderLevel[SHADER_WINDLIGHT] ) );
	shaders.push_back( make_pair( "avatar/avatarSkinV.glsl",				1 ) );

	// We no longer have to bind the shaders to global glhandles, they are automatically added to a map now.
	for (U32 i = 0; i < shaders.size(); i++)
	{
		// Note usage of GL_VERTEX_SHADER_ARB
		if (loadShaderFile(shaders[i].first, shaders[i].second, GL_VERTEX_SHADER_ARB) == 0)
		{
			return FALSE;
		}
	}

	// Load the Basic Fragment Shaders at the appropriate level. 
	// (in order of shader function call depth for reference purposes, deepest level first)

	shaders.clear();
	shaders.reserve(12);
	shaders.push_back( make_pair( "windlight/atmosphericsVarsF.glsl",		sVertexShaderLevel[SHADER_WINDLIGHT] ) );
	shaders.push_back( make_pair( "windlight/gammaF.glsl",					sVertexShaderLevel[SHADER_WINDLIGHT]) );
	shaders.push_back( make_pair( "windlight/atmosphericsF.glsl",			sVertexShaderLevel[SHADER_WINDLIGHT] ) );
	shaders.push_back( make_pair( "windlight/transportF.glsl",				sVertexShaderLevel[SHADER_WINDLIGHT] ) );	
	shaders.push_back( make_pair( "environment/waterFogF.glsl",				sVertexShaderLevel[SHADER_WATER] ) );
	shaders.push_back( make_pair( "lighting/lightF.glsl",					sVertexShaderLevel[SHADER_LIGHTING] ) );
	shaders.push_back( make_pair( "lighting/lightFullbrightF.glsl",			sVertexShaderLevel[SHADER_LIGHTING] ) );
	shaders.push_back( make_pair( "lighting/lightWaterF.glsl",				sVertexShaderLevel[SHADER_LIGHTING] ) );
	shaders.push_back( make_pair( "lighting/lightFullbrightWaterF.glsl",	sVertexShaderLevel[SHADER_LIGHTING] ) );
	shaders.push_back( make_pair( "lighting/lightShinyF.glsl",				sVertexShaderLevel[SHADER_LIGHTING] ) );
	shaders.push_back( make_pair( "lighting/lightFullbrightShinyF.glsl",	sVertexShaderLevel[SHADER_LIGHTING] ) );
	shaders.push_back( make_pair( "lighting/lightShinyWaterF.glsl",			sVertexShaderLevel[SHADER_LIGHTING] ) );
	
	for (U32 i = 0; i < shaders.size(); i++)
	{
		// Note usage of GL_FRAGMENT_SHADER_ARB
		if (loadShaderFile(shaders[i].first, shaders[i].second, GL_FRAGMENT_SHADER_ARB) == 0)
		{
			return FALSE;
		}
	}

	return TRUE;
}

BOOL LLShaderMgr::loadShadersEnvironment()
{
	BOOL success = TRUE;

	if (sVertexShaderLevel[SHADER_ENVIRONMENT] == 0)
	{
		gTerrainProgram.unload();
		return FALSE;
	}

	if (success)
	{
		gTerrainProgram.mName = "Terrain Shader";
		gTerrainProgram.mFeatures.calculatesLighting = true;
		gTerrainProgram.mFeatures.calculatesAtmospherics = true;
		gTerrainProgram.mFeatures.hasAtmospherics = true;
		gTerrainProgram.mFeatures.hasGamma = true;
		gTerrainProgram.mShaderFiles.clear();
		gTerrainProgram.mShaderFiles.push_back(make_pair("environment/terrainV.glsl", GL_VERTEX_SHADER_ARB));
		gTerrainProgram.mShaderFiles.push_back(make_pair("environment/terrainF.glsl", GL_FRAGMENT_SHADER_ARB));
		gTerrainProgram.mShaderLevel = sVertexShaderLevel[SHADER_ENVIRONMENT];
		success = gTerrainProgram.createShader(NULL, &sTerrainUniforms);
	}

	if (!success)
	{
		sVertexShaderLevel[SHADER_ENVIRONMENT] = 0;
		return FALSE;
	}
	
	LLWorld::getInstance()->updateWaterObjects();
	
	return TRUE;
}

BOOL LLShaderMgr::loadShadersWater()
{
	BOOL success = TRUE;
	BOOL terrainWaterSuccess = TRUE;

	if (sVertexShaderLevel[SHADER_WATER] == 0)
	{
		gWaterProgram.unload();
		gUnderWaterProgram.unload();
		gTerrainWaterProgram.unload();
		return FALSE;
	}

	if (success)
	{
		// load water shader
		gWaterProgram.mName = "Water Shader";
		gWaterProgram.mFeatures.calculatesAtmospherics = true;
		gWaterProgram.mFeatures.hasGamma = true;
		gWaterProgram.mFeatures.hasTransport = true;
		gWaterProgram.mShaderFiles.clear();
		gWaterProgram.mShaderFiles.push_back(make_pair("environment/waterV.glsl", GL_VERTEX_SHADER_ARB));
		gWaterProgram.mShaderFiles.push_back(make_pair("environment/waterF.glsl", GL_FRAGMENT_SHADER_ARB));
		gWaterProgram.mShaderLevel = sVertexShaderLevel[SHADER_WATER];
		success = gWaterProgram.createShader(NULL, &sWaterUniforms);
	}

	if (success)
	{
		//load under water vertex shader
		gUnderWaterProgram.mName = "Underwater Shader";
		gUnderWaterProgram.mFeatures.calculatesAtmospherics = true;
		gUnderWaterProgram.mShaderFiles.clear();
		gUnderWaterProgram.mShaderFiles.push_back(make_pair("environment/waterV.glsl", GL_VERTEX_SHADER_ARB));
		gUnderWaterProgram.mShaderFiles.push_back(make_pair("environment/underWaterF.glsl", GL_FRAGMENT_SHADER_ARB));
		gUnderWaterProgram.mShaderLevel = sVertexShaderLevel[SHADER_WATER];
		gUnderWaterProgram.mShaderGroup = LLGLSLShader::SG_WATER;
		
		success = gUnderWaterProgram.createShader(NULL, &sWaterUniforms);
	}

	if (success)
	{
		//load terrain water shader
		gTerrainWaterProgram.mName = "Terrain Water Shader";
		gTerrainWaterProgram.mFeatures.calculatesLighting = true;
		gTerrainWaterProgram.mFeatures.calculatesAtmospherics = true;
		gTerrainWaterProgram.mFeatures.hasAtmospherics = true;
		gTerrainWaterProgram.mFeatures.hasWaterFog = true;
		gTerrainWaterProgram.mShaderFiles.clear();
		gTerrainWaterProgram.mShaderFiles.push_back(make_pair("environment/terrainV.glsl", GL_VERTEX_SHADER_ARB));
		gTerrainWaterProgram.mShaderFiles.push_back(make_pair("environment/terrainWaterF.glsl", GL_FRAGMENT_SHADER_ARB));
		gTerrainWaterProgram.mShaderLevel = sVertexShaderLevel[SHADER_ENVIRONMENT];
		gTerrainWaterProgram.mShaderGroup = LLGLSLShader::SG_WATER;
		terrainWaterSuccess = gTerrainWaterProgram.createShader(NULL, &sTerrainUniforms);
	}	

	/// Keep track of water shader levels
	if (gWaterProgram.mShaderLevel != sVertexShaderLevel[SHADER_WATER]
		|| gUnderWaterProgram.mShaderLevel != sVertexShaderLevel[SHADER_WATER])
	{
		sVertexShaderLevel[SHADER_WATER] = llmin(gWaterProgram.mShaderLevel, gUnderWaterProgram.mShaderLevel);
	}

	if (!success)
	{
		sVertexShaderLevel[SHADER_WATER] = 0;
		return FALSE;
	}

	// if we failed to load the terrain water shaders and we need them (using class2 water),
	// then drop down to class1 water.
	if (sVertexShaderLevel[SHADER_WATER] > 1 && !terrainWaterSuccess)
	{
		sVertexShaderLevel[SHADER_WATER]--;
		return loadShadersWater();
	}
	
	LLWorld::getInstance()->updateWaterObjects();

	return TRUE;
}

BOOL LLShaderMgr::loadShadersEffects()
{
	BOOL success = TRUE;

	if (sVertexShaderLevel[SHADER_EFFECT] == 0)
	{
		gGlowProgram.unload();
		gGlowExtractProgram.unload();
		gPostColorFilterProgram.unload();	
		gPostNightVisionProgram.unload();
		return FALSE;
	}

	if (success)
	{
		gGlowProgram.mName = "Glow Shader (Post)";
		gGlowProgram.mShaderFiles.clear();
		gGlowProgram.mShaderFiles.push_back(make_pair("effects/glowV.glsl", GL_VERTEX_SHADER_ARB));
		gGlowProgram.mShaderFiles.push_back(make_pair("effects/glowF.glsl", GL_FRAGMENT_SHADER_ARB));
		gGlowProgram.mShaderLevel = sVertexShaderLevel[SHADER_EFFECT];
		success = gGlowProgram.createShader(NULL, &sGlowUniforms);
		if (!success)
		{
			LLPipeline::sRenderGlow = FALSE;
		}
	}
	
	if (success)
	{
		gGlowExtractProgram.mName = "Glow Extract Shader (Post)";
		gGlowExtractProgram.mShaderFiles.clear();
		gGlowExtractProgram.mShaderFiles.push_back(make_pair("effects/glowExtractV.glsl", GL_VERTEX_SHADER_ARB));
		gGlowExtractProgram.mShaderFiles.push_back(make_pair("effects/glowExtractF.glsl", GL_FRAGMENT_SHADER_ARB));
		gGlowExtractProgram.mShaderLevel = sVertexShaderLevel[SHADER_EFFECT];
		success = gGlowExtractProgram.createShader(NULL, &sGlowExtractUniforms);
		if (!success)
		{
			LLPipeline::sRenderGlow = FALSE;
		}
	}
	
#if 0
	// disabling loading of postprocess shaders until we fix
	// ATI sampler2DRect compatibility.
	
	//load Color Filter Shader
	if (success)
	{
		vector<string> shaderUniforms;
		shaderUniforms.reserve(7);
		shaderUniforms.push_back("RenderTexture");
		shaderUniforms.push_back("gamma");
		shaderUniforms.push_back("brightness");
		shaderUniforms.push_back("contrast");
		shaderUniforms.push_back("contrastBase");
		shaderUniforms.push_back("saturation");
		shaderUniforms.push_back("lumWeights");

		gPostColorFilterProgram.mName = "Color Filter Shader (Post)";
		gPostColorFilterProgram.mShaderFiles.clear();
		gPostColorFilterProgram.mShaderFiles.push_back(make_pair("effects/colorFilterF.glsl", GL_FRAGMENT_SHADER_ARB));
		gPostColorFilterProgram.mShaderFiles.push_back(make_pair("effects/drawQuadV.glsl", GL_VERTEX_SHADER_ARB));
		gPostColorFilterProgram.mShaderLevel = sVertexShaderLevel[SHADER_EFFECT];
		success = gPostColorFilterProgram.createShader(NULL, &shaderUniforms);
	}

	//load Night Vision Shader
	if (success)
	{
		vector<string> shaderUniforms;
		shaderUniforms.reserve(5);
		shaderUniforms.push_back("RenderTexture");
		shaderUniforms.push_back("NoiseTexture");
		shaderUniforms.push_back("brightMult");
		shaderUniforms.push_back("noiseStrength");
		shaderUniforms.push_back("lumWeights");

		gPostNightVisionProgram.mName = "Night Vision Shader (Post)";
		gPostNightVisionProgram.mShaderFiles.clear();
		gPostNightVisionProgram.mShaderFiles.push_back(make_pair("effects/nightVisionF.glsl", GL_FRAGMENT_SHADER_ARB));
		gPostNightVisionProgram.mShaderFiles.push_back(make_pair("effects/drawQuadV.glsl", GL_VERTEX_SHADER_ARB));
		gPostNightVisionProgram.mShaderLevel = sVertexShaderLevel[SHADER_EFFECT];
		success = gPostNightVisionProgram.createShader(NULL, &shaderUniforms);
	}
	#endif

	return success;

}

BOOL LLShaderMgr::loadShadersDeferred()
{
	if (sVertexShaderLevel[SHADER_DEFERRED] == 0)
	{
		gDeferredDiffuseProgram.unload();
		return FALSE;
	}

	BOOL success = TRUE;

	if (success)
	{
		gDeferredDiffuseProgram.mName = "Deffered Diffuse Shader";
		gDeferredDiffuseProgram.mShaderFiles.clear();
		gDeferredDiffuseProgram.mShaderFiles.push_back(make_pair("deferred/diffuseV.glsl", GL_VERTEX_SHADER_ARB));
		gDeferredDiffuseProgram.mShaderFiles.push_back(make_pair("deferred/diffuseF.glsl", GL_FRAGMENT_SHADER_ARB));
		gDeferredDiffuseProgram.mShaderLevel = sVertexShaderLevel[SHADER_DEFERRED];
		success = gDeferredDiffuseProgram.createShader(NULL, NULL);
	}

	return success;
}

BOOL LLShaderMgr::loadShadersObject()
{
	BOOL success = TRUE;

	if (sVertexShaderLevel[SHADER_OBJECT] == 0)
	{
		gObjectShinyProgram.unload();
		gObjectFullbrightShinyProgram.unload();
		gObjectShinyWaterProgram.unload();
		gObjectSimpleProgram.unload();
		gObjectSimpleWaterProgram.unload();
		gObjectFullbrightProgram.unload();
		gObjectFullbrightWaterProgram.unload();
		return FALSE;
	}

	if (success)
	{
		gObjectSimpleProgram.mName = "Simple Shader";
		gObjectSimpleProgram.mFeatures.calculatesLighting = true;
		gObjectSimpleProgram.mFeatures.calculatesAtmospherics = true;
		gObjectSimpleProgram.mFeatures.hasGamma = true;
		gObjectSimpleProgram.mFeatures.hasAtmospherics = true;
		gObjectSimpleProgram.mFeatures.hasLighting = true;
		gObjectSimpleProgram.mShaderFiles.clear();
		gObjectSimpleProgram.mShaderFiles.push_back(make_pair("objects/simpleV.glsl", GL_VERTEX_SHADER_ARB));
		gObjectSimpleProgram.mShaderFiles.push_back(make_pair("objects/simpleF.glsl", GL_FRAGMENT_SHADER_ARB));
		gObjectSimpleProgram.mShaderLevel = sVertexShaderLevel[SHADER_OBJECT];
		success = gObjectSimpleProgram.createShader(NULL, NULL);
	}
	
	if (success)
	{
		gObjectSimpleWaterProgram.mName = "Simple Water Shader";
		gObjectSimpleWaterProgram.mFeatures.calculatesLighting = true;
		gObjectSimpleWaterProgram.mFeatures.calculatesAtmospherics = true;
		gObjectSimpleWaterProgram.mFeatures.hasWaterFog = true;
		gObjectSimpleWaterProgram.mFeatures.hasAtmospherics = true;
		gObjectSimpleWaterProgram.mFeatures.hasLighting = true;
		gObjectSimpleWaterProgram.mShaderFiles.clear();
		gObjectSimpleWaterProgram.mShaderFiles.push_back(make_pair("objects/simpleV.glsl", GL_VERTEX_SHADER_ARB));
		gObjectSimpleWaterProgram.mShaderFiles.push_back(make_pair("objects/simpleWaterF.glsl", GL_FRAGMENT_SHADER_ARB));
		gObjectSimpleWaterProgram.mShaderLevel = sVertexShaderLevel[SHADER_OBJECT];
		gObjectSimpleWaterProgram.mShaderGroup = LLGLSLShader::SG_WATER;
		success = gObjectSimpleWaterProgram.createShader(NULL, NULL);
	}
	
	if (success)
	{
		gObjectFullbrightProgram.mName = "Fullbright Shader";
		gObjectFullbrightProgram.mFeatures.calculatesAtmospherics = true;
		gObjectFullbrightProgram.mFeatures.hasGamma = true;
		gObjectFullbrightProgram.mFeatures.hasTransport = true;
		gObjectFullbrightProgram.mFeatures.isFullbright = true;
		gObjectFullbrightProgram.mShaderFiles.clear();
		gObjectFullbrightProgram.mShaderFiles.push_back(make_pair("objects/fullbrightV.glsl", GL_VERTEX_SHADER_ARB));
		gObjectFullbrightProgram.mShaderFiles.push_back(make_pair("objects/fullbrightF.glsl", GL_FRAGMENT_SHADER_ARB));
		gObjectFullbrightProgram.mShaderLevel = sVertexShaderLevel[SHADER_OBJECT];
		success = gObjectFullbrightProgram.createShader(NULL, NULL);
	}

	if (success)
	{
		gObjectFullbrightWaterProgram.mName = "Fullbright Water Shader";
		gObjectFullbrightWaterProgram.mFeatures.calculatesAtmospherics = true;
		gObjectFullbrightWaterProgram.mFeatures.isFullbright = true;
		gObjectFullbrightWaterProgram.mFeatures.hasWaterFog = true;		
		gObjectFullbrightWaterProgram.mFeatures.hasTransport = true;
		gObjectFullbrightWaterProgram.mShaderFiles.clear();
		gObjectFullbrightWaterProgram.mShaderFiles.push_back(make_pair("objects/fullbrightV.glsl", GL_VERTEX_SHADER_ARB));
		gObjectFullbrightWaterProgram.mShaderFiles.push_back(make_pair("objects/fullbrightWaterF.glsl", GL_FRAGMENT_SHADER_ARB));
		gObjectFullbrightWaterProgram.mShaderLevel = sVertexShaderLevel[SHADER_OBJECT];
		gObjectFullbrightWaterProgram.mShaderGroup = LLGLSLShader::SG_WATER;
		success = gObjectFullbrightWaterProgram.createShader(NULL, NULL);
	}

	if (success)
	{
		gObjectShinyProgram.mName = "Shiny Shader";
		gObjectShinyProgram.mFeatures.calculatesAtmospherics = true;
		gObjectShinyProgram.mFeatures.calculatesLighting = true;
		gObjectShinyProgram.mFeatures.hasGamma = true;
		gObjectShinyProgram.mFeatures.hasAtmospherics = true;
		gObjectShinyProgram.mFeatures.isShiny = true;
		gObjectShinyProgram.mShaderFiles.clear();
		gObjectShinyProgram.mShaderFiles.push_back(make_pair("objects/shinyV.glsl", GL_VERTEX_SHADER_ARB));
		gObjectShinyProgram.mShaderFiles.push_back(make_pair("objects/shinyF.glsl", GL_FRAGMENT_SHADER_ARB));		
		gObjectShinyProgram.mShaderLevel = sVertexShaderLevel[SHADER_OBJECT];
		success = gObjectShinyProgram.createShader(NULL, &sShinyUniforms);
	}

	if (success)
	{
		gObjectShinyWaterProgram.mName = "Shiny Water Shader";
		gObjectShinyWaterProgram.mFeatures.calculatesAtmospherics = true;
		gObjectShinyWaterProgram.mFeatures.calculatesLighting = true;
		gObjectShinyWaterProgram.mFeatures.isShiny = true;
		gObjectShinyWaterProgram.mFeatures.hasWaterFog = true;
		gObjectShinyWaterProgram.mFeatures.hasAtmospherics = true;
		gObjectShinyWaterProgram.mShaderFiles.clear();
		gObjectShinyWaterProgram.mShaderFiles.push_back(make_pair("objects/shinyWaterF.glsl", GL_FRAGMENT_SHADER_ARB));
		gObjectShinyWaterProgram.mShaderFiles.push_back(make_pair("objects/shinyV.glsl", GL_VERTEX_SHADER_ARB));
		gObjectShinyWaterProgram.mShaderLevel = sVertexShaderLevel[SHADER_OBJECT];
		gObjectShinyWaterProgram.mShaderGroup = LLGLSLShader::SG_WATER;
		success = gObjectShinyWaterProgram.createShader(NULL, &sShinyUniforms);
	}
	
	if (success)
	{
		gObjectFullbrightShinyProgram.mName = "Fullbright Shiny Shader";
		gObjectFullbrightShinyProgram.mFeatures.calculatesAtmospherics = true;
		gObjectFullbrightShinyProgram.mFeatures.isFullbright = true;
		gObjectFullbrightShinyProgram.mFeatures.isShiny = true;
		gObjectFullbrightShinyProgram.mFeatures.hasGamma = true;
		gObjectFullbrightShinyProgram.mFeatures.hasTransport = true;
		gObjectFullbrightShinyProgram.mShaderFiles.clear();
		gObjectFullbrightShinyProgram.mShaderFiles.push_back(make_pair("objects/fullbrightShinyV.glsl", GL_VERTEX_SHADER_ARB));
		gObjectFullbrightShinyProgram.mShaderFiles.push_back(make_pair("objects/fullbrightShinyF.glsl", GL_FRAGMENT_SHADER_ARB));
		gObjectFullbrightShinyProgram.mShaderLevel = sVertexShaderLevel[SHADER_OBJECT];
		success = gObjectFullbrightShinyProgram.createShader(NULL, &sShinyUniforms);
	}


	if( !success )
	{
		sVertexShaderLevel[SHADER_OBJECT] = 0;
		return FALSE;
	}
	
	return TRUE;
}

BOOL LLShaderMgr::loadShadersAvatar()
{
	BOOL success = TRUE;

	if (sVertexShaderLevel[SHADER_AVATAR] == 0)
	{
		gAvatarProgram.unload();
		gAvatarWaterProgram.unload();
		gAvatarEyeballProgram.unload();
		gAvatarPickProgram.unload();
		return FALSE;
	}

	if (success)
	{
		gAvatarProgram.mName = "Avatar Shader";
		gAvatarProgram.mFeatures.hasSkinning = true;
		gAvatarProgram.mFeatures.calculatesAtmospherics = true;
		gAvatarProgram.mFeatures.calculatesLighting = true;
		gAvatarProgram.mFeatures.hasGamma = true;
		gAvatarProgram.mFeatures.hasAtmospherics = true;
		gAvatarProgram.mFeatures.hasLighting = true;
		gAvatarProgram.mShaderFiles.clear();
		gAvatarProgram.mShaderFiles.push_back(make_pair("avatar/avatarV.glsl", GL_VERTEX_SHADER_ARB));
		gAvatarProgram.mShaderFiles.push_back(make_pair("avatar/avatarF.glsl", GL_FRAGMENT_SHADER_ARB));
		gAvatarProgram.mShaderLevel = sVertexShaderLevel[SHADER_AVATAR];
		success = gAvatarProgram.createShader(&sAvatarAttribs, &sAvatarUniforms);
			
		if (success)
		{
			gAvatarWaterProgram.mName = "Avatar Water Shader";
			gAvatarWaterProgram.mFeatures.hasSkinning = true;
			gAvatarWaterProgram.mFeatures.calculatesAtmospherics = true;
			gAvatarWaterProgram.mFeatures.calculatesLighting = true;
			gAvatarWaterProgram.mFeatures.hasWaterFog = true;
			gAvatarWaterProgram.mFeatures.hasAtmospherics = true;
			gAvatarWaterProgram.mFeatures.hasLighting = true;
			gAvatarWaterProgram.mShaderFiles.clear();
			gAvatarWaterProgram.mShaderFiles.push_back(make_pair("avatar/avatarV.glsl", GL_VERTEX_SHADER_ARB));
			gAvatarWaterProgram.mShaderFiles.push_back(make_pair("objects/simpleWaterF.glsl", GL_FRAGMENT_SHADER_ARB));
			// Note: no cloth under water:
			gAvatarWaterProgram.mShaderLevel = llmin(sVertexShaderLevel[SHADER_AVATAR], 1);	
			gAvatarWaterProgram.mShaderGroup = LLGLSLShader::SG_WATER;				
			success = gAvatarWaterProgram.createShader(&sAvatarAttribs, &sAvatarUniforms);
		}

		/// Keep track of avatar levels
		if (gAvatarProgram.mShaderLevel != sVertexShaderLevel[SHADER_AVATAR])
		{
			sMaxAvatarShaderLevel = sVertexShaderLevel[SHADER_AVATAR] = gAvatarProgram.mShaderLevel;
		}
	}

	if (success)
	{
		gAvatarPickProgram.mName = "Avatar Pick Shader";
		gAvatarPickProgram.mFeatures.hasSkinning = true;
		gAvatarPickProgram.mShaderFiles.clear();
		gAvatarPickProgram.mShaderFiles.push_back(make_pair("avatar/pickAvatarV.glsl", GL_VERTEX_SHADER_ARB));
		gAvatarPickProgram.mShaderFiles.push_back(make_pair("avatar/pickAvatarF.glsl", GL_FRAGMENT_SHADER_ARB));
		gAvatarPickProgram.mShaderLevel = sVertexShaderLevel[SHADER_AVATAR];
		success = gAvatarPickProgram.createShader(&sAvatarAttribs, &sAvatarUniforms);
	}

	if (success)
	{
		gAvatarEyeballProgram.mName = "Avatar Eyeball Program";
		gAvatarEyeballProgram.mFeatures.calculatesLighting = true;
		gAvatarEyeballProgram.mFeatures.isSpecular = true;
		gAvatarEyeballProgram.mFeatures.calculatesAtmospherics = true;
		gAvatarEyeballProgram.mFeatures.hasGamma = true;
		gAvatarEyeballProgram.mFeatures.hasAtmospherics = true;
		gAvatarEyeballProgram.mFeatures.hasLighting = true;
		gAvatarEyeballProgram.mShaderFiles.clear();
		gAvatarEyeballProgram.mShaderFiles.push_back(make_pair("avatar/eyeballV.glsl", GL_VERTEX_SHADER_ARB));
		gAvatarEyeballProgram.mShaderFiles.push_back(make_pair("avatar/eyeballF.glsl", GL_FRAGMENT_SHADER_ARB));
		gAvatarEyeballProgram.mShaderLevel = sVertexShaderLevel[SHADER_AVATAR];
		success = gAvatarEyeballProgram.createShader(NULL, NULL);
	}

	if( !success )
	{
		sVertexShaderLevel[SHADER_AVATAR] = 0;
		sMaxAvatarShaderLevel = 0;
		return FALSE;
	}
	
	return TRUE;
}

BOOL LLShaderMgr::loadShadersInterface()
{
	BOOL success = TRUE;

	if (sVertexShaderLevel[SHADER_INTERFACE] == 0)
	{
		gHighlightProgram.unload();
		return FALSE;
	}
	
	if (success)
	{
		gHighlightProgram.mName = "Highlight Shader";
		gHighlightProgram.mShaderFiles.clear();
		gHighlightProgram.mShaderFiles.push_back(make_pair("interface/highlightV.glsl", GL_VERTEX_SHADER_ARB));
		gHighlightProgram.mShaderFiles.push_back(make_pair("interface/highlightF.glsl", GL_FRAGMENT_SHADER_ARB));
		gHighlightProgram.mShaderLevel = sVertexShaderLevel[SHADER_INTERFACE];		
		success = gHighlightProgram.createShader(NULL, NULL);
	}

	if( !success )
	{
		sVertexShaderLevel[SHADER_INTERFACE] = 0;
		return FALSE;
	}
	
	return TRUE;
}

BOOL LLShaderMgr::loadShadersWindLight()
{	
	BOOL success = TRUE;

	if (sVertexShaderLevel[SHADER_WINDLIGHT] < 2)
	{
		gWLSkyProgram.unload();
		gWLCloudProgram.unload();
		return FALSE;
	}

	if (success)
	{
		gWLSkyProgram.mName = "Windlight Sky Shader";
		//gWLSkyProgram.mFeatures.hasGamma = true;
		gWLSkyProgram.mShaderFiles.clear();
		gWLSkyProgram.mShaderFiles.push_back(make_pair("windlight/skyV.glsl", GL_VERTEX_SHADER_ARB));
		gWLSkyProgram.mShaderFiles.push_back(make_pair("windlight/skyF.glsl", GL_FRAGMENT_SHADER_ARB));
		gWLSkyProgram.mShaderLevel = sVertexShaderLevel[SHADER_WINDLIGHT];
		gWLSkyProgram.mShaderGroup = LLGLSLShader::SG_SKY;
		success = gWLSkyProgram.createShader(NULL, &sWLUniforms);
	}

	if (success)
	{
		gWLCloudProgram.mName = "Windlight Cloud Program";
		//gWLCloudProgram.mFeatures.hasGamma = true;
		gWLCloudProgram.mShaderFiles.clear();
		gWLCloudProgram.mShaderFiles.push_back(make_pair("windlight/cloudsV.glsl", GL_VERTEX_SHADER_ARB));
		gWLCloudProgram.mShaderFiles.push_back(make_pair("windlight/cloudsF.glsl", GL_FRAGMENT_SHADER_ARB));
		gWLCloudProgram.mShaderLevel = sVertexShaderLevel[SHADER_WINDLIGHT];
		gWLCloudProgram.mShaderGroup = LLGLSLShader::SG_SKY;
		success = gWLCloudProgram.createShader(NULL, &sWLUniforms);
	}

	return success;
}


//===============================
// LLGLSL Shader implementation
//===============================
LLGLSLShader::LLGLSLShader()
: mProgramObject(0), mShaderLevel(0), mShaderGroup(SG_DEFAULT)
{
}

void LLGLSLShader::unload()
{
	stop_glerror();
	mAttribute.clear();
	mTexture.clear();
	mUniform.clear();
	mShaderFiles.clear();

	if (mProgramObject)
	{
		GLhandleARB obj[1024];
		GLsizei count;

		glGetAttachedObjectsARB(mProgramObject, 1024, &count, obj);
		for (GLsizei i = 0; i < count; i++)
		{
			glDeleteObjectARB(obj[i]);
		}

		glDeleteObjectARB(mProgramObject);

		mProgramObject = 0;
	}
	
	//hack to make apple not complain
	glGetError();
	
	stop_glerror();
}

BOOL LLGLSLShader::createShader(vector<string> * attributes,
								vector<string> * uniforms)
{
	llassert_always(!mShaderFiles.empty());
	BOOL success = TRUE;

	// Create program
	mProgramObject = glCreateProgramObjectARB();
	
	// Attach existing objects
	if (!LLShaderMgr::attachShaderFeatures(this))
	{
		return FALSE;
	}

	vector< pair<string,GLenum> >::iterator fileIter = mShaderFiles.begin();
	for ( ; fileIter != mShaderFiles.end(); fileIter++ )
	{
		GLhandleARB shaderhandle = LLShaderMgr::loadShaderFile((*fileIter).first, mShaderLevel, (*fileIter).second);
		lldebugs << "SHADER FILE: " << (*fileIter).first << " mShaderLevel=" << mShaderLevel << llendl;
		if (mShaderLevel > 0)
		{
			attachObject(shaderhandle);
		}
		else
		{
			success = FALSE;
		}
	}

	// Map attributes and uniforms
	if (success)
	{
		success = mapAttributes(attributes);
	}
	if (success)
	{
		success = mapUniforms(uniforms);
	}
	if( !success )
	{
		llwarns << "Failed to link shader: " << mName << llendl;

		// Try again using a lower shader level;
		if (mShaderLevel > 0)
		{
			llwarns << "Failed to link using shader level " << mShaderLevel << ". Trying again using shader level " << (mShaderLevel - 1) << "." << llendl;
			mShaderLevel--;
			return createShader(attributes,uniforms);
		}
	}
	return success;
}

BOOL LLGLSLShader::attachObject(std::string object)
{
	if (LLShaderMgr::sShaderObjects.count(object) > 0)
	{
		stop_glerror();
		glAttachObjectARB(mProgramObject, LLShaderMgr::sShaderObjects[object]);
		stop_glerror();
		return TRUE;
	}
	else
	{
		llwarns << "Attempting to attach shader object that hasn't been compiled: " << object << llendl;
		return FALSE;
	}
}

void LLGLSLShader::attachObject(GLhandleARB object)
{
	if (object != 0)
	{
		stop_glerror();
		glAttachObjectARB(mProgramObject, object);
		stop_glerror();
	}
	else
	{
		llwarns << "Attempting to attach non existing shader object. " << llendl;
	}
}

void LLGLSLShader::attachObjects(GLhandleARB* objects, S32 count)
{
	for (S32 i = 0; i < count; i++)
	{
		attachObject(objects[i]);
	}
}

BOOL LLGLSLShader::mapAttributes(const vector<string> * attributes)
{
	//link the program
	BOOL res = link();

	mAttribute.clear();
	U32 numAttributes = (attributes == NULL) ? 0 : attributes->size();
	mAttribute.resize(LLShaderMgr::sReservedAttribs.size() + numAttributes, -1);
	
	if (res)
	{ //read back channel locations

		//read back reserved channels first
		for (U32 i = 0; i < (S32) LLShaderMgr::sReservedAttribs.size(); i++)
		{
			const char* name = LLShaderMgr::sReservedAttribs[i].c_str();
			S32 index = glGetAttribLocationARB(mProgramObject, (GLcharARB *)name);
			if (index != -1)
			{
				mAttribute[i] = index;
				// llinfos << "Attribute " << name << " assigned to channel " << index << llendl;
			}
		}
		if (attributes != NULL)
		{
			for (U32 i = 0; i < numAttributes; i++)
			{
				const char* name = (*attributes)[i].c_str();
				S32 index = glGetAttribLocationARB(mProgramObject, name);
				if (index != -1)
				{
					mAttribute[LLShaderMgr::sReservedAttribs.size() + i] = index;
					// llinfos << "Attribute " << name << " assigned to channel " << index << llendl;
				}
			}
		}

		return TRUE;
	}
	
	return FALSE;
}

void LLGLSLShader::mapUniform(GLint index, const vector<string> * uniforms)
{
	if (index == -1)
	{
		return;
	}

	GLenum type;
	GLsizei length;
	GLint size;
	char name[1024];		/* Flawfinder: ignore */
	name[0] = 0;

	glGetActiveUniformARB(mProgramObject, index, 1024, &length, &size, &type, (GLcharARB *)name);
	S32 location = glGetUniformLocationARB(mProgramObject, name);
	if (location != -1)
	{
		mUniformMap[name] = location;
#if 0 // !LL_RELEASE_FOR_DOWNLOAD
		llinfos << "Uniform " << name << " is at location " << location << llendl;
#endif
	
		//find the index of this uniform
		for (S32 i = 0; i < (S32) LLShaderMgr::sReservedUniforms.size(); i++)
		{
			if ( (mUniform[i] == -1)
				&& (LLShaderMgr::sReservedUniforms[i].compare(0, length, name, LLShaderMgr::sReservedUniforms[i].length()) == 0))
			{
				//found it
				mUniform[i] = location;
				mTexture[i] = mapUniformTextureChannel(location, type);
				return;
			}
		}

		if (uniforms != NULL)
		{
			for (U32 i = 0; i < uniforms->size(); i++)
			{
				if ( (mUniform[i+LLShaderMgr::sReservedUniforms.size()] == -1)
					&& ((*uniforms)[i].compare(0, length, name, (*uniforms)[i].length()) == 0))
				{
					//found it
					mUniform[i+LLShaderMgr::sReservedUniforms.size()] = location;
					mTexture[i+LLShaderMgr::sReservedUniforms.size()] = mapUniformTextureChannel(location, type);
					return;
				}
			}
		}
	}
 }

GLint LLGLSLShader::mapUniformTextureChannel(GLint location, GLenum type)
{
	if (type >= GL_SAMPLER_1D_ARB && type <= GL_SAMPLER_2D_RECT_SHADOW_ARB)
	{	//this here is a texture
		glUniform1iARB(location, mActiveTextureChannels);
		llinfos << "Assigned to texture channel " << mActiveTextureChannels << llendl;
		return mActiveTextureChannels++;
	}
	return -1;
}

BOOL LLGLSLShader::mapUniforms(const vector<string> * uniforms)
{
	BOOL res = TRUE;
	
	mActiveTextureChannels = 0;
	mUniform.clear();
	mUniformMap.clear();
	mTexture.clear();
	mValue.clear();
	//initialize arrays
	U32 numUniforms = (uniforms == NULL) ? 0 : uniforms->size();
	mUniform.resize(numUniforms + LLShaderMgr::sReservedUniforms.size(), -1);
	mTexture.resize(numUniforms + LLShaderMgr::sReservedUniforms.size(), -1);
	
	bind();

	//get the number of active uniforms
	GLint activeCount;
	glGetObjectParameterivARB(mProgramObject, GL_OBJECT_ACTIVE_UNIFORMS_ARB, &activeCount);

	for (S32 i = 0; i < activeCount; i++)
	{
		mapUniform(i, uniforms);
	}

	unbind();

	return res;
}

BOOL LLGLSLShader::link(BOOL suppress_errors)
{
	return LLShaderMgr::linkProgramObject(mProgramObject, suppress_errors);
}

void LLGLSLShader::bind()
{
	if (gGLManager.mHasShaderObjects)
	{
		glUseProgramObjectARB(mProgramObject);

		if (mUniformsDirty)
		{
			LLWLParamManager::instance()->updateShaderUniforms(this);
			LLWaterParamManager::instance()->updateShaderUniforms(this);
			mUniformsDirty = FALSE;
		}
	}
}

void LLGLSLShader::unbind()
{
	if (gGLManager.mHasShaderObjects)
	{
		for (U32 i = 0; i < mAttribute.size(); ++i)
		{
			vertexAttrib4f(i, 0,0,0,1);
		}
		glUseProgramObjectARB(0);
	}
}

S32 LLGLSLShader::enableTexture(S32 uniform, S32 mode)
{
	if (uniform < 0 || uniform >= (S32)mTexture.size())
	{
		UNIFORM_ERRS << "LLGLSLShader::enableTexture: uniform out of range: " << uniform << llendl;
		return -1;
	}
	S32 index = mTexture[uniform];
	if (index != -1)
	{
		glActiveTextureARB(GL_TEXTURE0_ARB+index);
		glEnable(mode);
	}
	return index;
}

S32 LLGLSLShader::disableTexture(S32 uniform, S32 mode)
{
	if (uniform < 0 || uniform >= (S32)mTexture.size())
	{
		UNIFORM_ERRS << "LLGLSLShader::disableTexture: uniform out of range: " << uniform << llendl;
		return -1;
	}
	S32 index = mTexture[uniform];
	if (index != -1)
	{
		glActiveTextureARB(GL_TEXTURE0_ARB+index);
		glDisable(mode);
	}
	return index;
}

void LLGLSLShader::uniform1f(U32 index, GLfloat x)
{
	if (mProgramObject > 0)
	{	
		if (mUniform.size() <= index)
		{
			UNIFORM_ERRS << "Uniform index out of bounds." << llendl;
			return;
		}

		if (mUniform[index] >= 0)
		{
			std::map<GLint, LLVector4>::iterator iter = mValue.find(mUniform[index]);
			if (iter == mValue.end() || iter->second.mV[0] != x)
			{
				glUniform1fARB(mUniform[index], x);
				mValue[mUniform[index]] = LLVector4(x,0.f,0.f,0.f);
			}
		}
	}
}

void LLGLSLShader::uniform2f(U32 index, GLfloat x, GLfloat y)
{
	if (mProgramObject > 0)
	{	
		if (mUniform.size() <= index)
		{
			UNIFORM_ERRS << "Uniform index out of bounds." << llendl;
			return;
		}

		if (mUniform[index] >= 0)
		{
			std::map<GLint, LLVector4>::iterator iter = mValue.find(mUniform[index]);
			LLVector4 vec(x,y,0.f,0.f);
			if (iter == mValue.end() || shouldChange(iter->second,vec))
			{
				glUniform2fARB(mUniform[index], x, y);
				mValue[mUniform[index]] = vec;
			}
		}
	}
}

void LLGLSLShader::uniform3f(U32 index, GLfloat x, GLfloat y, GLfloat z)
{
	if (mProgramObject > 0)
	{	
		if (mUniform.size() <= index)
		{
			UNIFORM_ERRS << "Uniform index out of bounds." << llendl;
			return;
		}

		if (mUniform[index] >= 0)
		{
			std::map<GLint, LLVector4>::iterator iter = mValue.find(mUniform[index]);
			LLVector4 vec(x,y,z,0.f);
			if (iter == mValue.end() || shouldChange(iter->second,vec))
			{
				glUniform3fARB(mUniform[index], x, y, z);
				mValue[mUniform[index]] = vec;
			}
		}
	}
}

void LLGLSLShader::uniform4f(U32 index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	if (mProgramObject > 0)
	{	
		if (mUniform.size() <= index)
		{
			UNIFORM_ERRS << "Uniform index out of bounds." << llendl;
			return;
		}

		if (mUniform[index] >= 0)
		{
			std::map<GLint, LLVector4>::iterator iter = mValue.find(mUniform[index]);
			LLVector4 vec(x,y,z,w);
			if (iter == mValue.end() || shouldChange(iter->second,vec))
			{
				glUniform4fARB(mUniform[index], x, y, z, w);
				mValue[mUniform[index]] = vec;
			}
		}
	}
}

void LLGLSLShader::uniform1fv(U32 index, U32 count, const GLfloat* v)
{
	if (mProgramObject > 0)
	{	
		if (mUniform.size() <= index)
		{
			UNIFORM_ERRS << "Uniform index out of bounds." << llendl;
			return;
		}

		if (mUniform[index] >= 0)
		{
			std::map<GLint, LLVector4>::iterator iter = mValue.find(mUniform[index]);
			LLVector4 vec(v[0],0.f,0.f,0.f);
			if (iter == mValue.end() || shouldChange(iter->second,vec) || count != 1)
			{
				glUniform1fvARB(mUniform[index], count, v);
				mValue[mUniform[index]] = vec;
			}
		}
	}
}

void LLGLSLShader::uniform2fv(U32 index, U32 count, const GLfloat* v)
{
	if (mProgramObject > 0)
	{	
		if (mUniform.size() <= index)
		{
			UNIFORM_ERRS << "Uniform index out of bounds." << llendl;
			return;
		}

		if (mUniform[index] >= 0)
		{
			std::map<GLint, LLVector4>::iterator iter = mValue.find(mUniform[index]);
			LLVector4 vec(v[0],v[1],0.f,0.f);
			if (iter == mValue.end() || shouldChange(iter->second,vec) || count != 1)
			{
				glUniform2fvARB(mUniform[index], count, v);
				mValue[mUniform[index]] = vec;
			}
		}
	}
}

void LLGLSLShader::uniform3fv(U32 index, U32 count, const GLfloat* v)
{
	if (mProgramObject > 0)
	{	
		if (mUniform.size() <= index)
		{
			UNIFORM_ERRS << "Uniform index out of bounds." << llendl;
			return;
		}

		if (mUniform[index] >= 0)
		{
			std::map<GLint, LLVector4>::iterator iter = mValue.find(mUniform[index]);
			LLVector4 vec(v[0],v[1],v[2],0.f);
			if (iter == mValue.end() || shouldChange(iter->second,vec) || count != 1)
			{
				glUniform3fvARB(mUniform[index], count, v);
				mValue[mUniform[index]] = vec;
			}
		}
	}
}

void LLGLSLShader::uniform4fv(U32 index, U32 count, const GLfloat* v)
{
	if (mProgramObject > 0)
	{	
		if (mUniform.size() <= index)
		{
			UNIFORM_ERRS << "Uniform index out of bounds." << llendl;
			return;
		}

		if (mUniform[index] >= 0)
		{
			std::map<GLint, LLVector4>::iterator iter = mValue.find(mUniform[index]);
			LLVector4 vec(v[0],v[1],v[2],v[3]);
			if (iter == mValue.end() || shouldChange(iter->second,vec) || count != 1)
			{
				glUniform4fvARB(mUniform[index], count, v);
				mValue[mUniform[index]] = vec;
			}
		}
	}
}

void LLGLSLShader::uniformMatrix2fv(U32 index, U32 count, GLboolean transpose, const GLfloat *v)
{
	if (mProgramObject > 0)
	{	
		if (mUniform.size() <= index)
		{
			UNIFORM_ERRS << "Uniform index out of bounds." << llendl;
			return;
		}

		if (mUniform[index] >= 0)
		{
			glUniformMatrix2fvARB(mUniform[index], count, transpose, v);
		}
	}
}

void LLGLSLShader::uniformMatrix3fv(U32 index, U32 count, GLboolean transpose, const GLfloat *v)
{
	if (mProgramObject > 0)
	{	
		if (mUniform.size() <= index)
		{
			UNIFORM_ERRS << "Uniform index out of bounds." << llendl;
			return;
		}

		if (mUniform[index] >= 0)
		{
			glUniformMatrix3fvARB(mUniform[index], count, transpose, v);
		}
	}
}

void LLGLSLShader::uniformMatrix4fv(U32 index, U32 count, GLboolean transpose, const GLfloat *v)
{
	if (mProgramObject > 0)
	{	
		if (mUniform.size() <= index)
		{
			UNIFORM_ERRS << "Uniform index out of bounds." << llendl;
			return;
		}

		if (mUniform[index] >= 0)
		{
			glUniformMatrix4fvARB(mUniform[index], count, transpose, v);
		}
	}
}

GLint LLGLSLShader::getUniformLocation(const string& uniform)
{
	if (mProgramObject > 0)
	{
		std::map<string, GLint>::iterator iter = mUniformMap.find(uniform);
		if (iter != mUniformMap.end())
		{
			llassert(iter->second == glGetUniformLocationARB(mProgramObject, uniform.c_str()));
			return iter->second;
		}
	}

	return -1;
}

void LLGLSLShader::uniform1f(const string& uniform, GLfloat v)
{
	GLint location = getUniformLocation(uniform);
				
	if (location >= 0)
	{
		std::map<GLint, LLVector4>::iterator iter = mValue.find(location);
		LLVector4 vec(v,0.f,0.f,0.f);
		if (iter == mValue.end() || shouldChange(iter->second,vec))
		{
			glUniform1fARB(location, v);
			mValue[location] = vec;
		}
	}
}

void LLGLSLShader::uniform2f(const string& uniform, GLfloat x, GLfloat y)
{
	GLint location = getUniformLocation(uniform);
				
	if (location >= 0)
	{
		std::map<GLint, LLVector4>::iterator iter = mValue.find(location);
		LLVector4 vec(x,y,0.f,0.f);
		if (iter == mValue.end() || shouldChange(iter->second,vec))
		{
			glUniform2fARB(location, x,y);
			mValue[location] = vec;
		}
	}

}

void LLGLSLShader::uniform3f(const string& uniform, GLfloat x, GLfloat y, GLfloat z)
{
	GLint location = getUniformLocation(uniform);
				
	if (location >= 0)
	{
		std::map<GLint, LLVector4>::iterator iter = mValue.find(location);
		LLVector4 vec(x,y,z,0.f);
		if (iter == mValue.end() || shouldChange(iter->second,vec))
		{
			glUniform3fARB(location, x,y,z);
			mValue[location] = vec;
		}
	}
}

void LLGLSLShader::uniform4f(const string& uniform, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	GLint location = getUniformLocation(uniform);

	if (location >= 0)
	{
		std::map<GLint, LLVector4>::iterator iter = mValue.find(location);
		LLVector4 vec(x,y,z,w);
		if (iter == mValue.end() || shouldChange(iter->second,vec))
		{
			glUniform4fARB(location, x,y,z,w);
			mValue[location] = vec;
		}
	}
}

void LLGLSLShader::uniform1fv(const string& uniform, U32 count, const GLfloat* v)
{
	GLint location = getUniformLocation(uniform);

	if (location >= 0)
	{
		std::map<GLint, LLVector4>::iterator iter = mValue.find(location);
		LLVector4 vec(v[0],0.f,0.f,0.f);
		if (iter == mValue.end() || shouldChange(iter->second,vec) || count != 1)
		{
			glUniform1fvARB(location, count, v);
			mValue[location] = vec;
		}
	}
}

void LLGLSLShader::uniform2fv(const string& uniform, U32 count, const GLfloat* v)
{
	GLint location = getUniformLocation(uniform);
				
	if (location >= 0)
	{
		std::map<GLint, LLVector4>::iterator iter = mValue.find(location);
		LLVector4 vec(v[0],v[1],0.f,0.f);
		if (iter == mValue.end() || shouldChange(iter->second,vec) || count != 1)
		{
			glUniform2fvARB(location, count, v);
			mValue[location] = vec;
		}
	}
}

void LLGLSLShader::uniform3fv(const string& uniform, U32 count, const GLfloat* v)
{
	GLint location = getUniformLocation(uniform);
				
	if (location >= 0)
	{
		std::map<GLint, LLVector4>::iterator iter = mValue.find(location);
		LLVector4 vec(v[0],v[1],v[2],0.f);
		if (iter == mValue.end() || shouldChange(iter->second,vec) || count != 1)
		{
			glUniform3fvARB(location, count, v);
			mValue[location] = vec;
		}
	}
}

void LLGLSLShader::uniform4fv(const string& uniform, U32 count, const GLfloat* v)
{
	GLint location = getUniformLocation(uniform);

	if (location >= 0)
	{
		LLVector4 vec(v);
		std::map<GLint, LLVector4>::iterator iter = mValue.find(location);
		if (iter == mValue.end() || shouldChange(iter->second,vec) || count != 1)
		{
			glUniform4fvARB(location, count, v);
			mValue[location] = vec;
		}
	}
}

void LLGLSLShader::uniformMatrix2fv(const string& uniform, U32 count, GLboolean transpose, const GLfloat* v)
{
	GLint location = getUniformLocation(uniform);
				
	if (location >= 0)
	{
		glUniformMatrix2fvARB(location, count, transpose, v);
	}
}

void LLGLSLShader::uniformMatrix3fv(const string& uniform, U32 count, GLboolean transpose, const GLfloat* v)
{
	GLint location = getUniformLocation(uniform);
				
	if (location >= 0)
	{
		glUniformMatrix3fvARB(location, count, transpose, v);
	}
}

void LLGLSLShader::uniformMatrix4fv(const string& uniform, U32 count, GLboolean transpose, const GLfloat* v)
{
	GLint location = getUniformLocation(uniform);
				
	if (location >= 0)
	{
		glUniformMatrix4fvARB(location, count, transpose, v);
	}
}


void LLGLSLShader::vertexAttrib4f(U32 index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	if (mAttribute[index] > 0)
	{
		glVertexAttrib4fARB(mAttribute[index], x, y, z, w);
	}
}

void LLGLSLShader::vertexAttrib4fv(U32 index, GLfloat* v)
{
	if (mAttribute[index] > 0)
	{
		glVertexAttrib4fvARB(mAttribute[index], v);
	}
}
