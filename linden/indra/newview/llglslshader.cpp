/** 
 * @file llglslshader.cpp
 * @brief GLSL helper functions and state.
 *
 * Copyright (c) 2005-2007, Linden Research, Inc.
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

#include "llviewerprecompiledheaders.h"

#include "llviewerwindow.h"
#include "llglslshader.h"
#include "llviewercontrol.h"
#include "pipeline.h"
#include "llworld.h"

//utility shader objects (not shader programs)
GLhandleARB			gLightVertex;
GLhandleARB			gLightFragment;
GLhandleARB			gScatterVertex;
GLhandleARB			gScatterFragment;

LLVector4			gShinyOrigin;

//object shaders
LLGLSLShader		gObjectSimpleProgram;
LLGLSLShader		gObjectAlphaProgram;
LLGLSLShader		gObjectBumpProgram;
LLGLSLShader		gObjectShinyProgram;

//environment shaders
LLGLSLShader		gTerrainProgram;
LLGLSLShader		gGlowProgram;
LLGLSLShader		gGroundProgram;
LLGLSLShader		gWaterProgram;

//interface shaders
LLGLSLShader		gHighlightProgram;

//avatar skinning utility shader object
GLhandleARB			gAvatarSkinVertex;

//avatar shader handles
LLGLSLShader		gAvatarProgram;
LLGLSLShader		gAvatarEyeballProgram;
LLGLSLShader		gAvatarPickProgram;

//current avatar shader parameter pointer
GLint				gAvatarMatrixParam;
GLint				gMaterialIndex;
GLint				gSpecularIndex;

S32	LLShaderMgr::sVertexShaderLevel[SHADER_COUNT] = { 0 };
S32	LLShaderMgr::sMaxVertexShaderLevel[SHADER_COUNT] = { 0 };

//glsl parameter tables
const char* LLShaderMgr::sReservedAttribs[] =
{
	"materialColor",
	"specularColor",
	"binormal"
};

U32 LLShaderMgr::sReservedAttribCount = LLShaderMgr::END_RESERVED_ATTRIBS;

const char* LLShaderMgr::sAvatarAttribs[] = 
{
	"weight",
	"clothing",
	"gWindDir",
	"gSinWaveParams",
	"gGravity"
};

U32 LLShaderMgr::sAvatarAttribCount =  sizeof(LLShaderMgr::sAvatarAttribs)/sizeof(char*);

const char* LLShaderMgr::sAvatarUniforms[] = 
{
	"matrixPalette"
};

U32 LLShaderMgr::sAvatarUniformCount = 1;

const char* LLShaderMgr::sReservedUniforms[] =
{
	"diffuseMap",
	"specularMap",
	"bumpMap",
	"environmentMap",
	"scatterMap"
};

U32 LLShaderMgr::sReservedUniformCount = LLShaderMgr::END_RESERVED_UNIFORMS;

const char* LLShaderMgr::sTerrainUniforms[] =
{
	"detail0",
	"detail1",
	"alphaRamp"
};

U32 LLShaderMgr::sTerrainUniformCount = sizeof(LLShaderMgr::sTerrainUniforms)/sizeof(char*);

const char* LLShaderMgr::sGlowUniforms[] =
{
	"delta"
};

U32 LLShaderMgr::sGlowUniformCount = sizeof(LLShaderMgr::sGlowUniforms)/sizeof(char*);

const char* LLShaderMgr::sShinyUniforms[] = 
{
	"origin"
};

U32 LLShaderMgr::sShinyUniformCount = sizeof(LLShaderMgr::sShinyUniforms)/sizeof(char*);

const char* LLShaderMgr::sWaterUniforms[] =
{
	"screenTex",
	"eyeVec",
	"time",
	"d1",
	"d2",
	"lightDir",
	"specular",
	"lightExp",
	"fbScale",
	"refScale"
};

U32 LLShaderMgr::sWaterUniformCount =  sizeof(LLShaderMgr::sWaterUniforms)/sizeof(char*);


//============================================================================
// Set Levels

S32 LLShaderMgr::getVertexShaderLevel(S32 type)
{
	return sVertexShaderLevel[type];
}

S32 LLShaderMgr::getMaxVertexShaderLevel(S32 type)
{
	return sMaxVertexShaderLevel[type];
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
		res = LLString(log);
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

GLhandleARB LLShaderMgr::loadShader(const LLString& filename, S32 cls, GLenum type)
{
	GLenum error;
	error = glGetError();
	if (error != GL_NO_ERROR)
	{
		llwarns << "GL ERROR entering loadShader(): " << error << llendl;
	}
	
	llinfos << "Loading shader file: " << filename << llendl;

	if (filename.empty()) 
	{
		return 0;
	}


	//read in from file
	FILE* file = NULL;

	S32 try_gpu_class = sVertexShaderLevel[cls];
	S32 gpu_class;

	//find the most relevant file
	for (gpu_class = try_gpu_class; gpu_class > 0; gpu_class--)
	{	//search from the current gpu class down to class 1 to find the most relevant shader
		std::stringstream fname;
		fname << gDirUtilp->getExpandedFilename(LL_PATH_APP_SETTINGS, "shaders/class");
		fname << gpu_class << "/" << filename;
		
// 		llinfos << "Looking in " << fname.str().c_str() << llendl;
		file = fopen(fname.str().c_str(), "r");		/* Flawfinder: ignore */
		if (file)
		{
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
	while(fgets(buff, 1024, file) != NULL) 
	{
		text[count++] = strdup(buff);
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
#if 1 // 1.9.1
	if (ret)
	{
		sVertexShaderLevel[cls] = try_gpu_class;
	}
	else
	{
		if (sVertexShaderLevel[cls] > 1)
		{
			sVertexShaderLevel[cls] = sVertexShaderLevel[cls] - 1;
			ret = loadShader(filename,cls,type);
			if (ret && sMaxVertexShaderLevel[cls] > sVertexShaderLevel[cls])
			{
				sMaxVertexShaderLevel[cls] = sVertexShaderLevel[cls];
			}
		}
	}
#else
	if (ret)
	{
		S32 max = -1;
		/*if (try_gpu_class == sMaxVertexShaderLevel[cls])
		{
			max = gpu_class;
		}*/
		saveVertexShaderLevel(cls,try_gpu_class,max);
	}
	else
	{
		if (sVertexShaderLevel[cls] > 1)
		{
			sVertexShaderLevel[cls] = sVertexShaderLevel[cls] - 1;
			ret = loadShader(f,cls,type);
			if (ret && sMaxVertexShaderLevel[cls] > sVertexShaderLevel[cls])
			{
				saveVertexShaderLevel(cls, sVertexShaderLevel[cls], sVertexShaderLevel[cls]);
			}
		}
	}
#endif
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

	LLString log = get_object_log(obj);
	LLString::toLower(log);
	if (log.find("software") != LLString::npos)
	{
		llwarns << "GLSL Linker: Running in Software:" << llendl;
		success = GL_FALSE;
		suppress_errors = FALSE;
	}
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
	
	if (gGLManager.mHasFramebufferObject)
	{
		LLPipeline::sDynamicReflections = gSavedSettings.getBOOL("RenderDynamicReflections");
		LLPipeline::sRenderGlow = gSavedSettings.getBOOL("RenderGlow");
	}
	else
	{
		LLPipeline::sDynamicReflections = LLPipeline::sRenderGlow = FALSE;
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
	for (S32 i=0; i<SHADER_COUNT; i++)
	{
		sVertexShaderLevel[i] = 0;
		sMaxVertexShaderLevel[i] = 0;
	}
	if (gPipeline.canUseVertexShaders() && gSavedSettings.getBOOL("VertexShaderEnable"))
	{
		S32 light_class = 2;
		S32 env_class = 2;
		S32 obj_class = 0;

		if (gPipeline.getLightingDetail() == 0)
		{
			light_class = 1;
		}
		// Load lighting shaders
		sVertexShaderLevel[SHADER_LIGHTING] = light_class;
		sMaxVertexShaderLevel[SHADER_LIGHTING] = light_class;
		sVertexShaderLevel[SHADER_ENVIRONMENT] = env_class;
		sMaxVertexShaderLevel[SHADER_ENVIRONMENT] = env_class;
		sVertexShaderLevel[SHADER_OBJECT] = obj_class;
		sMaxVertexShaderLevel[SHADER_OBJECT] = obj_class;

		BOOL loaded = loadShadersLighting();

		if (loaded)
		{
			gPipeline.mVertexShadersEnabled = TRUE;
			gPipeline.mVertexShadersLoaded = 1;

			// Load all shaders to set max levels
			loadShadersEnvironment();
			loadShadersObject();
			// Load max avatar shaders to set the max level
			sVertexShaderLevel[SHADER_AVATAR] = 3;
			sMaxVertexShaderLevel[SHADER_AVATAR] = 3;
			loadShadersAvatar();

			// Load shaders to correct levels
			if (!gSavedSettings.getBOOL("RenderRippleWater"))
			{
				if (gSavedSettings.getBOOL("RenderGlow"))
				{
					sVertexShaderLevel[SHADER_ENVIRONMENT] = 1;
				}
				else
				{
					sVertexShaderLevel[SHADER_ENVIRONMENT] = 0;
					loadShadersEnvironment(); // unloads
				}
			}

#if LL_DARWIN // force avatar shaders off for mac
			sVertexShaderLevel[SHADER_AVATAR] = 0;
			sMaxVertexShaderLevel[SHADER_AVATAR] = 0;
#else
			if (gSavedSettings.getBOOL("RenderAvatarVP"))
			{
				S32 avatar = gSavedSettings.getS32("RenderAvatarMode");
				S32 avatar_class = 1 + avatar;
				// Set the actual level
				sVertexShaderLevel[SHADER_AVATAR] = avatar_class;
				loadShadersAvatar();
				if (sVertexShaderLevel[SHADER_AVATAR] != avatar_class)
				{
					if (sVertexShaderLevel[SHADER_AVATAR] == 0)
					{
						gSavedSettings.setBOOL("RenderAvatarVP", FALSE);
					}
					avatar = llmax(sVertexShaderLevel[SHADER_AVATAR]-1,0);
					gSavedSettings.setS32("RenderAvatarMode", avatar);
				}
			}
			else
			{
				sVertexShaderLevel[SHADER_AVATAR] = 0;
				gSavedSettings.setS32("RenderAvatarMode", 0);
				loadShadersAvatar(); // unloads
			}
#endif
		}
		else
		{
			gPipeline.mVertexShadersEnabled = FALSE;
			gPipeline.mVertexShadersLoaded = 0;
		}
	}
	if (gViewerWindow)
	{
		gViewerWindow->setCursor(UI_CURSOR_ARROW);
	}
}

void LLShaderMgr::unloadShaders()
{
	gObjectSimpleProgram.unload();
	gObjectShinyProgram.unload();
	gObjectBumpProgram.unload();
	gObjectAlphaProgram.unload();
	gWaterProgram.unload();
	gTerrainProgram.unload();
	gGlowProgram.unload();
	gGroundProgram.unload();
	gAvatarProgram.unload();
	gAvatarEyeballProgram.unload();
	gAvatarPickProgram.unload();
	gHighlightProgram.unload();

	sVertexShaderLevel[SHADER_LIGHTING] = 0;
	sVertexShaderLevel[SHADER_OBJECT] = 0;
	sVertexShaderLevel[SHADER_AVATAR] = 0;
	sVertexShaderLevel[SHADER_ENVIRONMENT] = 0;
	sVertexShaderLevel[SHADER_INTERFACE] = 0;

	gLightVertex = gLightFragment = gScatterVertex = gScatterFragment = 0;
	gPipeline.mVertexShadersLoaded = 0;
}

BOOL LLShaderMgr::loadShadersLighting()
{
	// Load light dependency shaders first
	// All of these have to load for any shaders to function
	
    std::string lightvertex = "lighting/lightV.glsl";
	//get default light function implementation
	gLightVertex = loadShader(lightvertex, SHADER_LIGHTING, GL_VERTEX_SHADER_ARB);
	if( !gLightVertex )
	{
		llwarns << "Failed to load " << lightvertex << llendl;
		return FALSE;
	}
	
	std::string lightfragment = "lighting/lightF.glsl";
	gLightFragment = loadShader(lightfragment, SHADER_LIGHTING, GL_FRAGMENT_SHADER_ARB);
	if ( !gLightFragment )
	{
		llwarns << "Failed to load " << lightfragment << llendl;
		return FALSE;
	}

	// NOTE: Scatter shaders use the ENVIRONMENT detail level
	
	std::string scattervertex = "environment/scatterV.glsl";
	gScatterVertex = loadShader(scattervertex, SHADER_ENVIRONMENT, GL_VERTEX_SHADER_ARB);
	if ( !gScatterVertex )
	{
		llwarns << "Failed to load " << scattervertex << llendl;
		return FALSE;
	}

	std::string scatterfragment = "environment/scatterF.glsl";
	gScatterFragment = loadShader(scatterfragment, SHADER_ENVIRONMENT, GL_FRAGMENT_SHADER_ARB);
	if ( !gScatterFragment )
	{
		llwarns << "Failed to load " << scatterfragment << llendl;
		return FALSE;
	}
	
	return TRUE;
}

BOOL LLShaderMgr::loadShadersEnvironment()
{
	GLhandleARB baseObjects[] = 
	{
		gLightFragment,
		gLightVertex,
		gScatterFragment,
		gScatterVertex
	};
	S32 baseCount = 4;

	BOOL success = TRUE;

	if (sVertexShaderLevel[SHADER_ENVIRONMENT] == 0)
	{
		gWaterProgram.unload();
		gGroundProgram.unload();
		gTerrainProgram.unload();
		gGlowProgram.unload();
		return FALSE;
	}
	
	if (success)
	{
		//load water vertex shader
		std::string waterfragment = "environment/waterF.glsl";
		std::string watervertex = "environment/waterV.glsl";
		gWaterProgram.mProgramObject = glCreateProgramObjectARB();
		gWaterProgram.attachObjects(baseObjects, baseCount);
		gWaterProgram.attachObject(loadShader(watervertex, SHADER_ENVIRONMENT, GL_VERTEX_SHADER_ARB));
		gWaterProgram.attachObject(loadShader(waterfragment, SHADER_ENVIRONMENT, GL_FRAGMENT_SHADER_ARB));

		success = gWaterProgram.mapAttributes();	
		if (success)
		{
			success = gWaterProgram.mapUniforms(sWaterUniforms, sWaterUniformCount);
		}
		if (!success)
		{
			llwarns << "Failed to load " << watervertex << llendl;
		}
	}
	if (success)
	{
		//load ground vertex shader
		std::string groundvertex = "environment/groundV.glsl";
		std::string groundfragment = "environment/groundF.glsl";
		gGroundProgram.mProgramObject = glCreateProgramObjectARB();
		gGroundProgram.attachObjects(baseObjects, baseCount);
		gGroundProgram.attachObject(loadShader(groundvertex, SHADER_ENVIRONMENT, GL_VERTEX_SHADER_ARB));
		gGroundProgram.attachObject(loadShader(groundfragment, SHADER_ENVIRONMENT, GL_FRAGMENT_SHADER_ARB));
	
		success = gGroundProgram.mapAttributes();
		if (success)
		{
			success = gGroundProgram.mapUniforms();
		}
		if (!success)
		{
			llwarns << "Failed to load " << groundvertex << llendl;
		}
	}

	if (success)
	{
		//load terrain vertex shader
		std::string terrainvertex = "environment/terrainV.glsl";
		std::string terrainfragment = "environment/terrainF.glsl";
		gTerrainProgram.mProgramObject = glCreateProgramObjectARB();
		gTerrainProgram.attachObjects(baseObjects, baseCount);
		gTerrainProgram.attachObject(loadShader(terrainvertex, SHADER_ENVIRONMENT, GL_VERTEX_SHADER_ARB));
		gTerrainProgram.attachObject(loadShader(terrainfragment, SHADER_ENVIRONMENT, GL_FRAGMENT_SHADER_ARB));
		success = gTerrainProgram.mapAttributes();
		if (success)
		{
			success = gTerrainProgram.mapUniforms(sTerrainUniforms, sTerrainUniformCount);
		}
		if (!success)
		{
			llwarns << "Failed to load " << terrainvertex << llendl;
		}
	}

	if (success)
	{
		//load glow shader
		std::string glowvertex = "environment/glowV.glsl";
		std::string glowfragment = "environment/glowF.glsl";
		gGlowProgram.mProgramObject = glCreateProgramObjectARB();
		gGlowProgram.attachObjects(baseObjects, baseCount);
		gGlowProgram.attachObject(loadShader(glowvertex, SHADER_ENVIRONMENT, GL_VERTEX_SHADER_ARB));
		gGlowProgram.attachObject(loadShader(glowfragment, SHADER_ENVIRONMENT, GL_FRAGMENT_SHADER_ARB));
		success = gGlowProgram.mapAttributes();
		if (success)
		{
			success = gGlowProgram.mapUniforms(sGlowUniforms, sGlowUniformCount);
		}
		if (!success)
		{
			llwarns << "Failed to load " << glowvertex << llendl;
		}
	}

	if( !success )
	{
		sVertexShaderLevel[SHADER_ENVIRONMENT] = 0;
		sMaxVertexShaderLevel[SHADER_ENVIRONMENT] = 0;
		return FALSE;
	}
	
	if (gWorldPointer)
	{
		gWorldPointer->updateWaterObjects();
	}
	
	return TRUE;
}

BOOL LLShaderMgr::loadShadersObject()
{
	GLhandleARB baseObjects[] = 
	{
		gLightFragment,
		gLightVertex,
		gScatterFragment,
		gScatterVertex
	};
	S32 baseCount = 4;

	BOOL success = TRUE;

	if (sVertexShaderLevel[SHADER_OBJECT] == 0)
	{
		gObjectShinyProgram.unload();
		gObjectSimpleProgram.unload();
		gObjectBumpProgram.unload();
		gObjectAlphaProgram.unload();
		return FALSE;
	}

#if 0
	if (success)
	{
		//load object (volume/tree) vertex shader
		std::string simplevertex = "objects/simpleV.glsl";
		std::string simplefragment = "objects/simpleF.glsl";
		gObjectSimpleProgram.mProgramObject = glCreateProgramObjectARB();
		gObjectSimpleProgram.attachObjects(baseObjects, baseCount);
		gObjectSimpleProgram.attachObject(loadShader(simplevertex, SHADER_OBJECT, GL_VERTEX_SHADER_ARB));
		gObjectSimpleProgram.attachObject(loadShader(simplefragment, SHADER_OBJECT, GL_FRAGMENT_SHADER_ARB));
		success = gObjectSimpleProgram.mapAttributes();
		if (success)
		{
			success = gObjectSimpleProgram.mapUniforms();
		}
		if( !success )
		{
			llwarns << "Failed to load " << simplevertex << llendl;
		}
	}
	
	if (success)
	{
		//load object bumpy vertex shader
		std::string bumpshinyvertex = "objects/bumpshinyV.glsl";
		std::string bumpshinyfragment = "objects/bumpshinyF.glsl";
		gObjectBumpProgram.mProgramObject = glCreateProgramObjectARB();
		gObjectBumpProgram.attachObjects(baseObjects, baseCount);
		gObjectBumpProgram.attachObject(loadShader(bumpshinyvertex, SHADER_OBJECT, GL_VERTEX_SHADER_ARB));
		gObjectBumpProgram.attachObject(loadShader(bumpshinyfragment, SHADER_OBJECT, GL_FRAGMENT_SHADER_ARB));
		success = gObjectBumpProgram.mapAttributes();
		if (success)
		{
			success = gObjectBumpProgram.mapUniforms();
		}
		if( !success )
		{
			llwarns << "Failed to load " << bumpshinyvertex << llendl;
		}
	}

	if (success)
	{
		//load object alpha vertex shader
		std::string alphavertex = "objects/alphaV.glsl";
		std::string alphafragment = "objects/alphaF.glsl";
		gObjectAlphaProgram.mProgramObject = glCreateProgramObjectARB();
		gObjectAlphaProgram.attachObjects(baseObjects, baseCount);
		gObjectAlphaProgram.attachObject(loadShader(alphavertex, SHADER_OBJECT, GL_VERTEX_SHADER_ARB));
		gObjectAlphaProgram.attachObject(loadShader(alphafragment, SHADER_OBJECT, GL_FRAGMENT_SHADER_ARB));

		success = gObjectAlphaProgram.mapAttributes();
		if (success)
		{
			success = gObjectAlphaProgram.mapUniforms();
		}
		if( !success )
		{
			llwarns << "Failed to load " << alphavertex << llendl;
		}
	}
#endif

	if (success)
	{
		//load shiny vertex shader
		std::string shinyvertex = "objects/shinyV.glsl";
		std::string shinyfragment = "objects/shinyF.glsl";
		gObjectShinyProgram.mProgramObject = glCreateProgramObjectARB();
		gObjectShinyProgram.attachObjects(baseObjects, baseCount);
		gObjectShinyProgram.attachObject(loadShader(shinyvertex, SHADER_OBJECT, GL_VERTEX_SHADER_ARB));
		gObjectShinyProgram.attachObject(loadShader(shinyfragment, SHADER_OBJECT, GL_FRAGMENT_SHADER_ARB));

		success = gObjectShinyProgram.mapAttributes();
		if (success)
		{
			success = gObjectShinyProgram.mapUniforms(sShinyUniforms, sShinyUniformCount);
		}
		if( !success )
		{
			llwarns << "Failed to load " << shinyvertex << llendl;
		}
	}

	if( !success )
	{
		sVertexShaderLevel[SHADER_OBJECT] = 0;
		sMaxVertexShaderLevel[SHADER_OBJECT] = 0;
		return FALSE;
	}
	
	return TRUE;
}

BOOL LLShaderMgr::loadShadersAvatar()
{
	GLhandleARB baseObjects[] = 
	{
		gLightFragment,
		gLightVertex,
		gScatterFragment,
		gScatterVertex
	};
	S32 baseCount = 4;
	
	BOOL success = TRUE;

	if (sVertexShaderLevel[SHADER_AVATAR] == 0)
	{
		gAvatarProgram.unload();
		gAvatarEyeballProgram.unload();
		gAvatarPickProgram.unload();
		return FALSE;
	}
	
	/*if (success)
	{
		//load specular (eyeball) vertex program
		std::string eyeballvertex = "avatar/eyeballV.glsl";
		std::string eyeballfragment = "avatar/eyeballF.glsl";
		gAvatarEyeballProgram.mProgramObject = glCreateProgramObjectARB();
		gAvatarEyeballProgram.attachObjects(baseObjects, baseCount);
		gAvatarEyeballProgram.attachObject(loadShader(eyeballvertex, SHADER_AVATAR, GL_VERTEX_SHADER_ARB));
		gAvatarEyeballProgram.attachObject(loadShader(eyeballfragment, SHADER_AVATAR, GL_FRAGMENT_SHADER_ARB));
		success = gAvatarEyeballProgram.mapAttributes();
		if (success)
		{
			success = gAvatarEyeballProgram.mapUniforms();
		}
		if( !success )
		{
			llwarns << "Failed to load " << eyeballvertex << llendl;
		}
	}*/

	if (success)
	{
		gAvatarSkinVertex = loadShader("avatar/avatarSkinV.glsl", SHADER_AVATAR, GL_VERTEX_SHADER_ARB);
		//load avatar vertex shader
		std::string avatarvertex = "avatar/avatarV.glsl";
		std::string avatarfragment = "avatar/avatarF.glsl";
		
		gAvatarProgram.mProgramObject = glCreateProgramObjectARB();
		gAvatarProgram.attachObjects(baseObjects, baseCount);
		gAvatarProgram.attachObject(gAvatarSkinVertex);
		gAvatarProgram.attachObject(loadShader(avatarvertex, SHADER_AVATAR, GL_VERTEX_SHADER_ARB));
		gAvatarProgram.attachObject(loadShader(avatarfragment, SHADER_AVATAR, GL_FRAGMENT_SHADER_ARB));
		
		success = gAvatarProgram.mapAttributes(sAvatarAttribs, sAvatarAttribCount);
		if (success)
		{
			success = gAvatarProgram.mapUniforms(sAvatarUniforms, sAvatarUniformCount);
		}
		if( !success )
		{
			llwarns << "Failed to load " << avatarvertex << llendl;
		}
	}

	if (success)
	{
		//load avatar picking shader
		std::string pickvertex = "avatar/pickAvatarV.glsl";
		std::string pickfragment = "avatar/pickAvatarF.glsl";
		gAvatarPickProgram.mProgramObject = glCreateProgramObjectARB();
		gAvatarPickProgram.attachObject(loadShader(pickvertex, SHADER_AVATAR, GL_VERTEX_SHADER_ARB));
		gAvatarPickProgram.attachObject(loadShader(pickfragment, SHADER_AVATAR, GL_FRAGMENT_SHADER_ARB));
		gAvatarPickProgram.attachObject(gAvatarSkinVertex);

		success = gAvatarPickProgram.mapAttributes(sAvatarAttribs, sAvatarAttribCount);
		if (success)
		{
			success = gAvatarPickProgram.mapUniforms(sAvatarUniforms, sAvatarUniformCount);
		}
		if( !success )
		{
			llwarns << "Failed to load " << pickvertex << llendl;
		}
	}

	if( !success )
	{
		sVertexShaderLevel[SHADER_AVATAR] = 0;
		sMaxVertexShaderLevel[SHADER_AVATAR] = 0;
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
		//load highlighting shader
		std::string highlightvertex = "interface/highlightV.glsl";
		std::string highlightfragment = "interface/highlightF.glsl";
		gHighlightProgram.mProgramObject = glCreateProgramObjectARB();
		gHighlightProgram.attachObject(loadShader(highlightvertex, SHADER_INTERFACE, GL_VERTEX_SHADER_ARB));
		gHighlightProgram.attachObject(loadShader(highlightfragment, SHADER_INTERFACE, GL_FRAGMENT_SHADER_ARB));
	
		success = gHighlightProgram.mapAttributes();
		if (success)
		{
			success = gHighlightProgram.mapUniforms();
		}
		if( !success )
		{
			llwarns << "Failed to load " << highlightvertex << llendl;
		}
	}

	if( !success )
	{
		sVertexShaderLevel[SHADER_INTERFACE] = 0;
		sMaxVertexShaderLevel[SHADER_INTERFACE] = 0;
		return FALSE;
	}
	
	return TRUE;
}


//===============================
// LLGLSL Shader implementation
//===============================
LLGLSLShader::LLGLSLShader()
: mProgramObject(0)
{ }

void LLGLSLShader::unload()
{
	stop_glerror();
	mAttribute.clear();
	mTexture.clear();
	mUniform.clear();

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

BOOL LLGLSLShader::mapAttributes(const char** attrib_names, S32 count)
{
	//link the program
	BOOL res = link();

	mAttribute.clear();
	mAttribute.resize(LLShaderMgr::sReservedAttribCount + count, -1);
	
	if (res)
	{ //read back channel locations

		//read back reserved channels first
		for (S32 i = 0; i < (S32) LLShaderMgr::sReservedAttribCount; i++)
		{
			const char* name = LLShaderMgr::sReservedAttribs[i];
			S32 index = glGetAttribLocationARB(mProgramObject, name);
			if (index != -1)
			{
				mAttribute[i] = index;
				llinfos << "Attribute " << name << " assigned to channel " << index << llendl;
			}
		}

		for (S32 i = 0; i < count; i++)
		{
			const char* name = attrib_names[i];
			S32 index = glGetAttribLocationARB(mProgramObject, name);
			if (index != -1)
			{
				mAttribute[LLShaderMgr::sReservedAttribCount + i] = index;
				llinfos << "Attribute " << name << " assigned to channel " << index << llendl;
			}
		}

		return TRUE;
	}
	
	return FALSE;
}

void LLGLSLShader::mapUniform(GLint index, const char** uniform_names, S32 count)
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

	glGetActiveUniformARB(mProgramObject, index, 1024, &length, &size, &type, name);
	
	//find the index of this uniform
	for (S32 i = 0; i < (S32) LLShaderMgr::sReservedUniformCount; i++)
	{
		if (mUniform[i] == -1 && !strncmp(LLShaderMgr::sReservedUniforms[i],name, strlen(LLShaderMgr::sReservedUniforms[i])))		/* Flawfinder: ignore */
		{
			//found it
			S32 location = glGetUniformLocationARB(mProgramObject, name);
			mUniform[i] = location;
			llinfos << "Uniform " << name << " is at location " << location << llendl;
			mTexture[i] = mapUniformTextureChannel(location, type);
			return;
		}
	}

	for (S32 i = 0; i < count; i++)
	{
		if (mUniform[i+LLShaderMgr::sReservedUniformCount] == -1 && 
			!strncmp(uniform_names[i],name, strlen(uniform_names[i])))		/* Flawfinder: ignore */
		{
			//found it
			S32 location = glGetUniformLocationARB(mProgramObject, name);
			mUniform[i+LLShaderMgr::sReservedUniformCount] = location;
			llinfos << "Uniform " << name << " is at location " << location << " stored in index " << 
				(i+LLShaderMgr::sReservedUniformCount) << llendl;
			mTexture[i+LLShaderMgr::sReservedUniformCount] = mapUniformTextureChannel(location, type);
			return;
		}
	}

	//llinfos << "Unknown uniform: " << name << llendl;
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

BOOL LLGLSLShader::mapUniforms(const char** uniform_names,  S32 count)
{
	BOOL res = TRUE;
	
	mActiveTextureChannels = 0;
	mUniform.clear();
	mTexture.clear();

	//initialize arrays
	mUniform.resize(count + LLShaderMgr::sReservedUniformCount, -1);
	mTexture.resize(count + LLShaderMgr::sReservedUniformCount, -1);
	
	bind();

	//get the number of active uniforms
	GLint activeCount;
	glGetObjectParameterivARB(mProgramObject, GL_OBJECT_ACTIVE_UNIFORMS_ARB, &activeCount);

	for (S32 i = 0; i < activeCount; i++)
	{
		mapUniform(i, uniform_names, count);
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
	glUseProgramObjectARB(mProgramObject);
	if (mAttribute.size() > 0)
	{
		gMaterialIndex = mAttribute[0];
	}
}

void LLGLSLShader::unbind()
{
	for (U32 i = 0; i < mAttribute.size(); ++i)
	{
		vertexAttrib4f(i, 0,0,0,1);
	}
	glUseProgramObjectARB(0);
}

S32 LLGLSLShader::enableTexture(S32 uniform, S32 mode)
{
	if (uniform < 0 || uniform >= (S32)mTexture.size())
	{
		llerrs << "LLGLSLShader::enableTexture: uniform out of range: " << uniform << llendl;
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
	S32 index = mTexture[uniform];
	if (index != -1)
	{
		glActiveTextureARB(GL_TEXTURE0_ARB+index);
		glDisable(mode);
	}
	return index;
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

void LLScatterShader::init(GLhandleARB shader, int map_stage)
{
	glUseProgramObjectARB(shader);
	glUniform1iARB(glGetUniformLocationARB(shader, "scatterMap"), map_stage);
	glUseProgramObjectARB(0);
}

