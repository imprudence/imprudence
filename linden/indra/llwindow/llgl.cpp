/** 
 * @file llgl.cpp
 * @brief LLGL implementation
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

// This file sets some global GL parameters, and implements some 
// useful functions for GL operations.

#define GLH_EXT_SINGLE_FILE

#include "linden_common.h"

#include "llsys.h"

#include "llgl.h"

#include "llerror.h"
#include "llquaternion.h"
#include "llmath.h"
#include "m4math.h"
#include "llstring.h"

#include "llglheaders.h"

#if LL_LINUX && !LL_MESA_HEADLESS
// The __APPLE__ hack is to make glh_extensions.h not symbol-clash horribly
# define __APPLE__
# include "GL/glh_extensions.h"
# undef __APPLE__

/* Although SDL very likely ends up calling glXGetProcAddress() itself,
   if we do it ourselves then we avoid getting bogus addresses back on
   some systems.  Weird. */
/*# include "SDL/SDL.h"
  # define GLH_EXT_GET_PROC_ADDRESS(p) SDL_GL_GetProcAddress(p) */
#define GLX_GLXEXT_PROTOTYPES 1
# include "GL/glx.h"
# include "GL/glxext.h"
// Use glXGetProcAddressARB instead of glXGetProcAddress - the ARB symbol
// is considered 'legacy' but works on more machines.
# define GLH_EXT_GET_PROC_ADDRESS(p) glXGetProcAddressARB((const GLubyte*)(p)) 
#endif // LL_LINUX && !LL_MESA_HEADLESS


#ifdef _DEBUG
//#define GL_STATE_VERIFY
#endif

BOOL gClothRipple = FALSE;
BOOL gNoRender = FALSE;


#if (LL_WINDOWS || LL_LINUX) && !LL_MESA_HEADLESS
// ATI prototypes
// vertex blending prototypes
PFNGLWEIGHTPOINTERARBPROC			glWeightPointerARB = NULL;
PFNGLVERTEXBLENDARBPROC				glVertexBlendARB = NULL;
PFNGLWEIGHTFVARBPROC				glWeightfvARB = NULL;

// Vertex buffer object prototypes
PFNGLBINDBUFFERARBPROC				glBindBufferARB = NULL;
PFNGLDELETEBUFFERSARBPROC			glDeleteBuffersARB = NULL;
PFNGLGENBUFFERSARBPROC				glGenBuffersARB = NULL;
PFNGLISBUFFERARBPROC				glIsBufferARB = NULL;
PFNGLBUFFERDATAARBPROC				glBufferDataARB = NULL;
PFNGLBUFFERSUBDATAARBPROC			glBufferSubDataARB = NULL;
PFNGLGETBUFFERSUBDATAARBPROC		glGetBufferSubDataARB = NULL;
PFNGLMAPBUFFERARBPROC				glMapBufferARB = NULL;
PFNGLUNMAPBUFFERARBPROC				glUnmapBufferARB = NULL;
PFNGLGETBUFFERPARAMETERIVARBPROC	glGetBufferParameterivARB = NULL;
PFNGLGETBUFFERPOINTERVARBPROC		glGetBufferPointervARB = NULL;

// vertex object prototypes
PFNGLNEWOBJECTBUFFERATIPROC			glNewObjectBufferATI = NULL;
PFNGLISOBJECTBUFFERATIPROC			glIsObjectBufferATI = NULL;
PFNGLUPDATEOBJECTBUFFERATIPROC		glUpdateObjectBufferATI = NULL;
PFNGLGETOBJECTBUFFERFVATIPROC		glGetObjectBufferfvATI = NULL;
PFNGLGETOBJECTBUFFERIVATIPROC		glGetObjectBufferivATI = NULL;
PFNGLFREEOBJECTBUFFERATIPROC		glFreeObjectBufferATI = NULL;
PFNGLARRAYOBJECTATIPROC				glArrayObjectATI = NULL;
PFNGLVERTEXATTRIBARRAYOBJECTATIPROC	glVertexAttribArrayObjectATI = NULL;
PFNGLGETARRAYOBJECTFVATIPROC		glGetArrayObjectfvATI = NULL;
PFNGLGETARRAYOBJECTIVATIPROC		glGetArrayObjectivATI = NULL;
PFNGLVARIANTARRAYOBJECTATIPROC		glVariantObjectArrayATI = NULL;
PFNGLGETVARIANTARRAYOBJECTFVATIPROC	glGetVariantArrayObjectfvATI = NULL;
PFNGLGETVARIANTARRAYOBJECTIVATIPROC	glGetVariantArrayObjectivATI = NULL;

// GL_ARB_occlusion_query
PFNGLGENQUERIESARBPROC glGenQueriesARB = NULL;
PFNGLDELETEQUERIESARBPROC glDeleteQueriesARB = NULL;
PFNGLISQUERYARBPROC glIsQueryARB = NULL;
PFNGLBEGINQUERYARBPROC glBeginQueryARB = NULL;
PFNGLENDQUERYARBPROC glEndQueryARB = NULL;
PFNGLGETQUERYIVARBPROC glGetQueryivARB = NULL;
PFNGLGETQUERYOBJECTIVARBPROC glGetQueryObjectivARB = NULL;
PFNGLGETQUERYOBJECTUIVARBPROC glGetQueryObjectuivARB = NULL;

// GL_ARB_point_parameters
PFNGLPOINTPARAMETERFARBPROC glPointParameterfARB = NULL;
PFNGLPOINTPARAMETERFVARBPROC glPointParameterfvARB = NULL;

// GL_EXT_framebuffer_object
PFNGLISRENDERBUFFEREXTPROC glIsRenderbufferEXT = NULL;
PFNGLBINDRENDERBUFFEREXTPROC glBindRenderbufferEXT = NULL;
PFNGLDELETERENDERBUFFERSEXTPROC glDeleteRenderbuffersEXT = NULL;
PFNGLGENRENDERBUFFERSEXTPROC glGenRenderbuffersEXT = NULL;
PFNGLRENDERBUFFERSTORAGEEXTPROC glRenderbufferStorageEXT = NULL;
PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC glGetRenderbufferParameterivEXT = NULL;
PFNGLISFRAMEBUFFEREXTPROC glIsFramebufferEXT = NULL;
PFNGLBINDFRAMEBUFFEREXTPROC glBindFramebufferEXT = NULL;
PFNGLDELETEFRAMEBUFFERSEXTPROC glDeleteFramebuffersEXT = NULL;
PFNGLGENFRAMEBUFFERSEXTPROC glGenFramebuffersEXT = NULL;
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC glCheckFramebufferStatusEXT = NULL;
PFNGLFRAMEBUFFERTEXTURE1DEXTPROC glFramebufferTexture1DEXT = NULL;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC glFramebufferTexture2DEXT = NULL;
PFNGLFRAMEBUFFERTEXTURE3DEXTPROC glFramebufferTexture3DEXT = NULL;
PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC glFramebufferRenderbufferEXT = NULL;
PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC glGetFramebufferAttachmentParameterivEXT = NULL;
PFNGLGENERATEMIPMAPEXTPROC glGenerateMipmapEXT = NULL;

//shader object prototypes
PFNGLDELETEOBJECTARBPROC glDeleteObjectARB = NULL;
PFNGLGETHANDLEARBPROC glGetHandleARB = NULL;
PFNGLDETACHOBJECTARBPROC glDetachObjectARB = NULL;
PFNGLCREATESHADEROBJECTARBPROC glCreateShaderObjectARB = NULL;
PFNGLSHADERSOURCEARBPROC glShaderSourceARB = NULL;
PFNGLCOMPILESHADERARBPROC glCompileShaderARB = NULL;
PFNGLCREATEPROGRAMOBJECTARBPROC glCreateProgramObjectARB = NULL;
PFNGLATTACHOBJECTARBPROC glAttachObjectARB = NULL;
PFNGLLINKPROGRAMARBPROC glLinkProgramARB = NULL;
PFNGLUSEPROGRAMOBJECTARBPROC glUseProgramObjectARB = NULL;
PFNGLVALIDATEPROGRAMARBPROC glValidateProgramARB = NULL;
PFNGLUNIFORM1FARBPROC glUniform1fARB = NULL;
PFNGLUNIFORM2FARBPROC glUniform2fARB = NULL;
PFNGLUNIFORM3FARBPROC glUniform3fARB = NULL;
PFNGLUNIFORM4FARBPROC glUniform4fARB = NULL;
PFNGLUNIFORM1IARBPROC glUniform1iARB = NULL;
PFNGLUNIFORM2IARBPROC glUniform2iARB = NULL;
PFNGLUNIFORM3IARBPROC glUniform3iARB = NULL;
PFNGLUNIFORM4IARBPROC glUniform4iARB = NULL;
PFNGLUNIFORM1FVARBPROC glUniform1fvARB = NULL;
PFNGLUNIFORM2FVARBPROC glUniform2fvARB = NULL;
PFNGLUNIFORM3FVARBPROC glUniform3fvARB = NULL;
PFNGLUNIFORM4FVARBPROC glUniform4fvARB = NULL;
PFNGLUNIFORM1IVARBPROC glUniform1ivARB = NULL;
PFNGLUNIFORM2IVARBPROC glUniform2ivARB = NULL;
PFNGLUNIFORM3IVARBPROC glUniform3ivARB = NULL;
PFNGLUNIFORM4IVARBPROC glUniform4ivARB = NULL;
PFNGLUNIFORMMATRIX2FVARBPROC glUniformMatrix2fvARB = NULL;
PFNGLUNIFORMMATRIX3FVARBPROC glUniformMatrix3fvARB = NULL;
PFNGLUNIFORMMATRIX4FVARBPROC glUniformMatrix4fvARB = NULL;
PFNGLGETOBJECTPARAMETERFVARBPROC glGetObjectParameterfvARB = NULL;
PFNGLGETOBJECTPARAMETERIVARBPROC glGetObjectParameterivARB = NULL;
PFNGLGETINFOLOGARBPROC glGetInfoLogARB = NULL;
PFNGLGETATTACHEDOBJECTSARBPROC glGetAttachedObjectsARB = NULL;
PFNGLGETUNIFORMLOCATIONARBPROC glGetUniformLocationARB = NULL;
PFNGLGETACTIVEUNIFORMARBPROC glGetActiveUniformARB = NULL;
PFNGLGETUNIFORMFVARBPROC glGetUniformfvARB = NULL;
PFNGLGETUNIFORMIVARBPROC glGetUniformivARB = NULL;
PFNGLGETSHADERSOURCEARBPROC glGetShaderSourceARB = NULL;

// vertex shader prototypes
#if LL_LINUX
PFNGLVERTEXATTRIB1DARBPROC glVertexAttrib1dARB = NULL;
PFNGLVERTEXATTRIB1DVARBPROC glVertexAttrib1dvARB = NULL;
PFNGLVERTEXATTRIB1FARBPROC glVertexAttrib1fARB = NULL;
PFNGLVERTEXATTRIB1FVARBPROC glVertexAttrib1fvARB = NULL;
PFNGLVERTEXATTRIB1SARBPROC glVertexAttrib1sARB = NULL;
PFNGLVERTEXATTRIB1SVARBPROC glVertexAttrib1svARB = NULL;
PFNGLVERTEXATTRIB2DARBPROC glVertexAttrib2dARB = NULL;
PFNGLVERTEXATTRIB2DVARBPROC glVertexAttrib2dvARB = NULL;
PFNGLVERTEXATTRIB2FARBPROC glVertexAttrib2fARB = NULL;
PFNGLVERTEXATTRIB2FVARBPROC glVertexAttrib2fvARB = NULL;
PFNGLVERTEXATTRIB2SARBPROC glVertexAttrib2sARB = NULL;
PFNGLVERTEXATTRIB2SVARBPROC glVertexAttrib2svARB = NULL;
PFNGLVERTEXATTRIB3DARBPROC glVertexAttrib3dARB = NULL;
PFNGLVERTEXATTRIB3DVARBPROC glVertexAttrib3dvARB = NULL;
PFNGLVERTEXATTRIB3FARBPROC glVertexAttrib3fARB = NULL;
PFNGLVERTEXATTRIB3FVARBPROC glVertexAttrib3fvARB = NULL;
PFNGLVERTEXATTRIB3SARBPROC glVertexAttrib3sARB = NULL;
PFNGLVERTEXATTRIB3SVARBPROC glVertexAttrib3svARB = NULL;
#endif // LL_LINUX
PFNGLVERTEXATTRIB4NBVARBPROC glVertexAttrib4nbvARB = NULL;
PFNGLVERTEXATTRIB4NIVARBPROC glVertexAttrib4nivARB = NULL;
PFNGLVERTEXATTRIB4NSVARBPROC glVertexAttrib4nsvARB = NULL;
PFNGLVERTEXATTRIB4NUBARBPROC glVertexAttrib4nubARB = NULL;
PFNGLVERTEXATTRIB4NUBVARBPROC glVertexAttrib4nubvARB = NULL;
PFNGLVERTEXATTRIB4NUIVARBPROC glVertexAttrib4nuivARB = NULL;
PFNGLVERTEXATTRIB4NUSVARBPROC glVertexAttrib4nusvARB = NULL;
#if LL_LINUX
PFNGLVERTEXATTRIB4BVARBPROC glVertexAttrib4bvARB = NULL;
PFNGLVERTEXATTRIB4DARBPROC glVertexAttrib4dARB = NULL;
PFNGLVERTEXATTRIB4DVARBPROC glVertexAttrib4dvARB = NULL;
PFNGLVERTEXATTRIB4FARBPROC glVertexAttrib4fARB = NULL;
PFNGLVERTEXATTRIB4FVARBPROC glVertexAttrib4fvARB = NULL;
PFNGLVERTEXATTRIB4IVARBPROC glVertexAttrib4ivARB = NULL;
PFNGLVERTEXATTRIB4SARBPROC glVertexAttrib4sARB = NULL;
PFNGLVERTEXATTRIB4SVARBPROC glVertexAttrib4svARB = NULL;
PFNGLVERTEXATTRIB4UBVARBPROC glVertexAttrib4ubvARB = NULL;
PFNGLVERTEXATTRIB4UIVARBPROC glVertexAttrib4uivARB = NULL;
PFNGLVERTEXATTRIB4USVARBPROC glVertexAttrib4usvARB = NULL;
PFNGLVERTEXATTRIBPOINTERARBPROC glVertexAttribPointerARB = NULL;
PFNGLENABLEVERTEXATTRIBARRAYARBPROC glEnableVertexAttribArrayARB = NULL;
PFNGLDISABLEVERTEXATTRIBARRAYARBPROC glDisableVertexAttribArrayARB = NULL;
PFNGLPROGRAMSTRINGARBPROC glProgramStringARB = NULL;
PFNGLBINDPROGRAMARBPROC glBindProgramARB = NULL;
PFNGLDELETEPROGRAMSARBPROC glDeleteProgramsARB = NULL;
PFNGLGENPROGRAMSARBPROC glGenProgramsARB = NULL;
PFNGLPROGRAMENVPARAMETER4DARBPROC glProgramEnvParameter4dARB = NULL;
PFNGLPROGRAMENVPARAMETER4DVARBPROC glProgramEnvParameter4dvARB = NULL;
PFNGLPROGRAMENVPARAMETER4FARBPROC glProgramEnvParameter4fARB = NULL;
PFNGLPROGRAMENVPARAMETER4FVARBPROC glProgramEnvParameter4fvARB = NULL;
PFNGLPROGRAMLOCALPARAMETER4DARBPROC glProgramLocalParameter4dARB = NULL;
PFNGLPROGRAMLOCALPARAMETER4DVARBPROC glProgramLocalParameter4dvARB = NULL;
PFNGLPROGRAMLOCALPARAMETER4FARBPROC glProgramLocalParameter4fARB = NULL;
PFNGLPROGRAMLOCALPARAMETER4FVARBPROC glProgramLocalParameter4fvARB = NULL;
PFNGLGETPROGRAMENVPARAMETERDVARBPROC glGetProgramEnvParameterdvARB = NULL;
PFNGLGETPROGRAMENVPARAMETERFVARBPROC glGetProgramEnvParameterfvARB = NULL;
PFNGLGETPROGRAMLOCALPARAMETERDVARBPROC glGetProgramLocalParameterdvARB = NULL;
PFNGLGETPROGRAMLOCALPARAMETERFVARBPROC glGetProgramLocalParameterfvARB = NULL;
PFNGLGETPROGRAMIVARBPROC glGetProgramivARB = NULL;
PFNGLGETPROGRAMSTRINGARBPROC glGetProgramStringARB = NULL;
PFNGLGETVERTEXATTRIBDVARBPROC glGetVertexAttribdvARB = NULL;
PFNGLGETVERTEXATTRIBFVARBPROC glGetVertexAttribfvARB = NULL;
PFNGLGETVERTEXATTRIBIVARBPROC glGetVertexAttribivARB = NULL;
PFNGLGETVERTEXATTRIBPOINTERVARBPROC glGetVertexAttribPointervARB = NULL;
PFNGLISPROGRAMARBPROC glIsProgramARB = NULL;
#endif // LL_LINUX
PFNGLBINDATTRIBLOCATIONARBPROC glBindAttribLocationARB = NULL;
PFNGLGETACTIVEATTRIBARBPROC glGetActiveAttribARB = NULL;
PFNGLGETATTRIBLOCATIONARBPROC glGetAttribLocationARB = NULL;

#if LL_WINDOWS
PFNWGLSWAPINTERVALEXTPROC			wglSwapIntervalEXT = NULL;
#endif

#if LL_LINUX
PFNGLCOLORTABLEEXTPROC glColorTableEXT = NULL;
#endif // LL_LINUX

#endif

LLGLManager gGLManager;

LLGLManager::LLGLManager()
{
	mInited = FALSE;
	mIsDisabled = FALSE;
	mHasCubeMap = FALSE;
	mHasMultitexture = FALSE;
	mHasMipMapGeneration = FALSE;
	mHasAnisotropic = FALSE;
	mHasCompressedTextures = FALSE;
	mHasARBEnvCombine = FALSE;
	mIsGF2or4MX = FALSE;
	mIsGF3 = FALSE;
	mIsGFFX = FALSE;
	mIsATI = FALSE;
	mATIOffsetVerticalLines = FALSE;
	mHasVertexShader = FALSE;
	mHasFragmentShader = FALSE;
	mHasShaderObjects = FALSE;
	mHasPointParameters = FALSE;

#if LL_WINDOWS
	mHasWGLARBPixelFormat = FALSE;
#endif // LL_WINDOWS

#if LL_DARWIN
	mHasAPPLEVertexArrayRange = FALSE;
	mHasAPPLEFence = FALSE;
	mHasAPPLEVAO = FALSE;
#endif

	mIsNVIDIA = FALSE;
	mIsIntel = FALSE;
		
	mDriverVersionMajor = 1;
	mDriverVersionMinor = 0;
	mDriverVersionRelease = 0;
	mGLVersion = 1.0f;
	
	mNumTextureUnits = 1;
	mVRAM = 0;
	mGLMaxVertexRange = 0;
	mGLMaxIndexRange = 0;
}

//---------------------------------------------------------------------
// Global initialization for GL
//---------------------------------------------------------------------
void LLGLManager::initWGL()
{
	mHasPBuffer = FALSE;
#if LL_WINDOWS && !LL_MESA_HEADLESS
	if (!glh_init_extensions("WGL_ARB_pixel_format"))
	{
		llwarns << "No ARB pixel format extensions" << llendl;
	}

	if (ExtensionExists("WGL_EXT_swap_control", gGLHExts.mSysExts))
	{
        GLH_EXT_NAME(wglSwapIntervalEXT) = (PFNWGLSWAPINTERVALEXTPROC)GLH_EXT_GET_PROC_ADDRESS("wglSwapIntervalEXT");
	}

	mHasWGLARBPixelFormat = glh_init_extensions("WGL_ARB_pbuffer");
	if( !mHasWGLARBPixelFormat )
	{
		llwarns << "No ARB WGL PBuffer extensions" << llendl;
	}

	if( !glh_init_extensions("WGL_ARB_render_texture") )
	{
		llwarns << "No ARB WGL render texture extensions" << llendl;
	}

	mHasPBuffer = ExtensionExists("WGL_ARB_pbuffer", gGLHExts.mSysExts) &&
					ExtensionExists("WGL_ARB_render_texture", gGLHExts.mSysExts) &&
					ExtensionExists("WGL_ARB_pixel_format", gGLHExts.mSysExts);
#endif
}

// return false if unable (or unwilling due to old drivers) to init GL
bool LLGLManager::initGL()
{
	if (mInited)
	{
		llerrs << "Calling init on LLGLManager after already initialized!" << llendl;
	}

	GLint alpha_bits;
	glGetIntegerv( GL_ALPHA_BITS, &alpha_bits );
	if( 8 != alpha_bits )
	{
		llwarns << "Frame buffer has less than 8 bits of alpha.  Avatar texture compositing will fail." << llendl;
	}
	
	// This function uses at least one variable that's initialized below.
	// Moved this call down to after we figure out which card we're dealing with.  -- MBW 2003.10.07
//	initExtensions();

	// Extract video card strings and convert to upper case to
	// work around driver-to-driver variation in capitalization.
	mGLVendor = LLString((const char *)glGetString(GL_VENDOR));
	LLString::toUpper(mGLVendor);

	mGLRenderer = LLString((const char *)glGetString(GL_RENDERER));
	LLString::toUpper(mGLRenderer);

	parse_gl_version( &mDriverVersionMajor, 
		&mDriverVersionMinor, 
		&mDriverVersionRelease, 
		&mDriverVersionVendorString );

	mGLVersion = mDriverVersionMajor + mDriverVersionMinor * .1f;
	
	// Trailing space necessary to keep "nVidia Corpor_ati_on" cards
	// from being recognized as ATI.
	if (mGLVendor.substr(0,4) == "ATI ")
	{
		mGLVendorShort = "ATI";
		BOOL mobile = FALSE;
		if (mGLRenderer.find("MOBILITY") != LLString::npos)
		{
			mobile = TRUE;
		}
		mIsATI = TRUE;

#if LL_WINDOWS && !LL_MESA_HEADLESS
		if (mDriverVersionRelease < 3842)
		{
			mATIOffsetVerticalLines = TRUE;
		}
#endif // LL_WINDOWS
	}
	else if (mGLVendor.find("NVIDIA ") != LLString::npos)
	{
		mGLVendorShort = "NVIDIA";
		mIsNVIDIA = TRUE;
		if (   mGLRenderer.find("GEFORCE4 MX") != LLString::npos
			|| mGLRenderer.find("GEFORCE2") != LLString::npos
			|| mGLRenderer.find("GEFORCE 2") != LLString::npos
			|| mGLRenderer.find("GEFORCE4 460 GO") != LLString::npos
			|| mGLRenderer.find("GEFORCE4 440 GO") != LLString::npos
			|| mGLRenderer.find("GEFORCE4 420 GO") != LLString::npos)
		{
			mIsGF2or4MX = TRUE;
		}
		else if (mGLRenderer.find("GEFORCE FX") != LLString::npos
				 || mGLRenderer.find("QUADRO FX") != LLString::npos
				 || mGLRenderer.find("NV34") != LLString::npos)
		{
			mIsGFFX = TRUE;
		}
		else if(mGLRenderer.find("GEFORCE3") != LLString::npos)
		{
			mIsGF3 = TRUE;
		}

	}
	else if (mGLVendor.find("INTEL") != LLString::npos)
	{
		mGLVendorShort = "INTEL";
		mIsIntel = TRUE;
	}
	else
	{
		mGLVendorShort = "MISC";
	}
	
	// This is called here because it depends on the setting of mIsGF2or4MX, and sets up mHasMultitexture.
	initExtensions();

	if (mHasMultitexture)
	{
		GLint num_tex_units;		
		glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &num_tex_units);
		mNumTextureUnits = llmin(num_tex_units, (GLint)MAX_GL_TEXTURE_UNITS);
		if (mIsIntel)
		{
			mNumTextureUnits = llmin(mNumTextureUnits, 2);
		}
	}
	else
	{
		// We don't support cards that don't support the GL_ARB_multitexture extension
		llwarns << "GL Drivers do not support GL_ARB_multitexture" << llendl;
		return false;
	}
	

	initGLStates();
	return true;
}

LLString LLGLManager::getGLInfoString()
{
	LLString info_str;
	LLString all_exts, line;

	info_str += LLString("GL_VENDOR      ") + LLString((const char *)glGetString(GL_VENDOR)) + LLString("\n");
	info_str += LLString("GL_RENDERER    ") + LLString((const char *)glGetString(GL_RENDERER)) + LLString("\n");
	info_str += LLString("GL_VERSION     ") + LLString((const char *)glGetString(GL_VERSION)) + LLString("\n");

#if !LL_MESA_HEADLESS
	all_exts = (const char *)gGLHExts.mSysExts;
	LLString::replaceChar(all_exts, ' ', '\n');
	info_str += LLString("GL_EXTENSIONS:\n") + all_exts + LLString("\n");
#endif
	
	return info_str;
}

LLString LLGLManager::getRawGLString()
{
	LLString gl_string;
	gl_string.assign((char*)glGetString(GL_VENDOR));
	gl_string.append(" ");
	gl_string.append((char*)glGetString(GL_RENDERER));
	return gl_string;
}

void LLGLManager::shutdownGL()
{
	if (mInited)
	{
		stop_glerror();
		mInited = FALSE;
	}
}

// these are used to turn software blending on. They appear in the Debug/Avatar menu
// presence of vertex skinning/blending or vertex programs will set these to FALSE by default.

extern LLCPUInfo gSysCPU;

void LLGLManager::initExtensions()
{
#if LL_MESA_HEADLESS
# if GL_ARB_multitexture
	mHasMultitexture = TRUE;
# else
	mHasMultitexture = FALSE;
# endif
# if GL_ARB_texture_env_combine
	mHasARBEnvCombine = TRUE;	
# else
	mHasARBEnvCombine = FALSE;
# endif
# if GL_ARB_texture_compression
	mHasCompressedTextures = TRUE;
# else
	mHasCompressedTextures = FALSE;
# endif
# if GL_ARB_vertex_buffer_object
	mHasVertexBufferObject = TRUE;
# else
	mHasVertexBufferObject = FALSE;
# endif
# if GL_EXT_framebuffer_object
	mHasFramebufferObject = TRUE;
# else
	mHasFramebufferObject = FALSE;
# endif
	mHasMipMapGeneration = FALSE;
	mHasPalettedTextures = FALSE;
	mHasSeparateSpecularColor = FALSE;
	mHasAnisotropic = FALSE;
	mHasCubeMap = FALSE;
	mHasOcclusionQuery = FALSE;
	mHasPointParameters = FALSE;
	mHasShaderObjects = FALSE;
	mHasVertexShader = FALSE;
	mHasFragmentShader = FALSE;
#else // LL_MESA_HEADLESS
	mHasMultitexture = glh_init_extensions("GL_ARB_multitexture");
	mHasMipMapGeneration = glh_init_extensions("GL_SGIS_generate_mipmap");
	mHasPalettedTextures = glh_init_extension("GL_EXT_paletted_texture");
	mHasSeparateSpecularColor = glh_init_extensions("GL_EXT_separate_specular_color");
	mHasAnisotropic = glh_init_extensions("GL_EXT_texture_filter_anisotropic");
	glh_init_extensions("GL_ARB_texture_cube_map");
	mHasCubeMap = ExtensionExists("GL_ARB_texture_cube_map", gGLHExts.mSysExts);
	mHasARBEnvCombine = ExtensionExists("GL_ARB_texture_env_combine", gGLHExts.mSysExts);
	mHasCompressedTextures = glh_init_extensions("GL_ARB_texture_compression");
	mHasOcclusionQuery = ExtensionExists("GL_ARB_occlusion_query", gGLHExts.mSysExts);
	mHasVertexBufferObject = ExtensionExists("GL_ARB_vertex_buffer_object", gGLHExts.mSysExts);
	mHasFramebufferObject = ExtensionExists("GL_EXT_framebuffer_object", gGLHExts.mSysExts);
#if !LL_DARWIN
	mHasPointParameters = !mIsATI && ExtensionExists("GL_ARB_point_parameters", gGLHExts.mSysExts);
#endif
	mHasShaderObjects = ExtensionExists("GL_ARB_shader_objects", gGLHExts.mSysExts) && ExtensionExists("GL_ARB_shading_language_100", gGLHExts.mSysExts);
	mHasVertexShader = ExtensionExists("GL_ARB_vertex_program", gGLHExts.mSysExts) && ExtensionExists("GL_ARB_vertex_shader", gGLHExts.mSysExts)
						&& ExtensionExists("GL_ARB_shading_language_100", gGLHExts.mSysExts);
	mHasFragmentShader = ExtensionExists("GL_ARB_fragment_shader", gGLHExts.mSysExts) && ExtensionExists("GL_ARB_shading_language_100", gGLHExts.mSysExts);
#endif

#if LL_LINUX
	// Our extension support for the Linux Client is very young with some
	// potential driver gotchas, so offer a semi-secret way to turn it off.
	if (getenv("LL_GL_NOEXT"))	/* Flawfinder: ignore */
	{
		//mHasMultitexture = FALSE; // NEEDED!
		mHasARBEnvCombine = FALSE;
		mHasCompressedTextures = FALSE;
		mHasVertexBufferObject = FALSE;
		mHasFramebufferObject = FALSE;
		mHasMipMapGeneration = FALSE;
		mHasPalettedTextures = FALSE;
		mHasSeparateSpecularColor = FALSE;
		mHasAnisotropic = FALSE;
		mHasCubeMap = FALSE;
		mHasOcclusionQuery = FALSE;
		mHasPointParameters = FALSE;
		mHasShaderObjects = FALSE;
		mHasVertexShader = FALSE;
		mHasFragmentShader = FALSE;
		llwarns << "GL extension support DISABLED via LL_GL_NOEXT" <<
			llendl;
	}
	else if (getenv("LL_GL_BASICEXT"))	/* Flawfinder: ignore */
	{
		// This switch attempts to turn off all support for exotic
		// extensions which I believe correspond to fatal driver
		// bug reports.  This should be the default until we get a
		// proper blacklist/whitelist on Linux.
		mHasMipMapGeneration = FALSE;
		mHasPalettedTextures = FALSE;
		mHasAnisotropic = FALSE;
		mHasCubeMap = FALSE; // apparently fatal on Intel 915 & similar
		mHasOcclusionQuery = FALSE; // source of many ATI system hangs
		mHasShaderObjects = FALSE;
		mHasVertexShader = FALSE;
		mHasFragmentShader = FALSE;
		llwarns << "GL extension support forced to SIMPLE level via LL_GL_BASICEXT" <<
			llendl;
	}
	if (getenv("LL_GL_BLACKLIST"))	/* Flawfinder: ignore */
	{
		// This lets advanced troubleshooters disable specific
		// GL extensions to isolate problems with their hardware.
		// SL-28126
		const char *const blacklist = getenv("LL_GL_BLACKLIST");	/* Flawfinder: ignore */
		llwarns << "GL extension support partially disabled via LL_GL_BLACKLIST: " << blacklist << llendl;
		if (strchr(blacklist,'a')) mHasARBEnvCombine = FALSE;
		if (strchr(blacklist,'b')) mHasCompressedTextures = FALSE;
		if (strchr(blacklist,'c')) mHasVertexBufferObject = FALSE;
		if (strchr(blacklist,'d')) mHasMipMapGeneration = FALSE;//S
		if (strchr(blacklist,'e')) mHasPalettedTextures = FALSE;//S
// 		if (strchr(blacklist,'f')) mHasNVVertexArrayRange = FALSE;//S
// 		if (strchr(blacklist,'g')) mHasNVFence = FALSE;//S
		if (strchr(blacklist,'h')) mHasSeparateSpecularColor = FALSE;
		if (strchr(blacklist,'i')) mHasAnisotropic = FALSE;//S
		if (strchr(blacklist,'j')) mHasCubeMap = FALSE;//S
// 		if (strchr(blacklist,'k')) mHasATIVAO = FALSE;//S
		if (strchr(blacklist,'l')) mHasOcclusionQuery = FALSE;
		if (strchr(blacklist,'m')) mHasShaderObjects = FALSE;//S
		if (strchr(blacklist,'n')) mHasVertexShader = FALSE;//S
		if (strchr(blacklist,'o')) mHasFragmentShader = FALSE;//S
		if (strchr(blacklist,'p')) mHasPointParameters = FALSE;//S
		if (strchr(blacklist,'q')) mHasFramebufferObject = FALSE;//S
	}
#endif // LL_LINUX

#if LL_DARWIN || LL_LINUX
	// MBW -- 12/4/2003 -- Using paletted textures causes a bunch of avatar rendering problems on the Mac.
	// Not sure if this is due to driver problems or incorrect use of the extension, but I'm disabling it for now.
	// Tofu - 2006-10-03 -- Same problem on Linux.
	mHasPalettedTextures = false;
#endif
	
	if (!mHasMultitexture)
	{
		llinfos << "Couldn't initialize multitexturing" << llendl;
	}
	if (!mHasMipMapGeneration)
	{
		llinfos << "Couldn't initialize mipmap generation" << llendl;
	}
	if (!mHasARBEnvCombine)
	{
		llinfos << "Couldn't initialize GL_ARB_texture_env_combine" << llendl;
	}
	if (!mHasPalettedTextures)
	{
		llinfos << "Couldn't initialize GL_EXT_paletted_texture" << llendl;
	}
	if (!mHasSeparateSpecularColor)
	{
		llinfos << "Couldn't initialize separate specular color" << llendl;
	}
	if (!mHasAnisotropic)
	{
		llinfos << "Couldn't initialize anisotropic filtering" << llendl;
	}
	if (!mHasCompressedTextures)
	{
		llinfos << "Couldn't initialize GL_ARB_texture_compression" << llendl;
	}
	if (!mHasOcclusionQuery)
	{
		llinfos << "Couldn't initialize GL_ARB_occlusion_query" << llendl;
	}
	if (!mHasPointParameters)
	{
		llinfos << "Couldn't initialize GL_ARB_point_parameters" << llendl;
	}
	if (!mHasShaderObjects)
	{
		llinfos << "Couldn't initialize GL_ARB_shader_objects" << llendl;
	}
	if (!mHasVertexShader)
	{
		llinfos << "Couldn't initialize GL_ARB_vertex_shader" << llendl;
	}
	if (!mHasFragmentShader)
	{
		llinfos << "Couldn't initialize GL_ARB_fragment_shader" << llendl;
	}

	// Disable certain things due to known bugs
	if (mIsIntel && mHasMipMapGeneration)
	{
		llinfos << "Disabling mip-map generation for Intel GPUs" << llendl;
		mHasMipMapGeneration = FALSE;
	}
	if (mIsATI && mHasMipMapGeneration)
	{
		llinfos << "Disabling mip-map generation for ATI GPUs (performance opt)" << llendl;
		mHasMipMapGeneration = FALSE;
	}
	
	// Misc
	glGetIntegerv(GL_MAX_ELEMENTS_VERTICES, (GLint*) &mGLMaxVertexRange);
	glGetIntegerv(GL_MAX_ELEMENTS_INDICES, (GLint*) &mGLMaxIndexRange);
	
	// Apple specific
#if LL_DARWIN
	mHasAPPLEVertexArrayRange = glh_init_extensions("GL_APPLE_vertex_array_range");
	if (!mHasAPPLEVertexArrayRange)
	{
		llinfos << "Couldn't initialize GL_APPLE_vertex_array_range" << llendl;
	}

	mHasAPPLEFence = glh_init_extensions("GL_APPLE_fence");
	if (!mHasAPPLEFence)
	{
		llinfos << "Couldn't initialize GL_APPLE_fence" << llendl;
	}
	
	mHasAPPLEVAO = glh_init_extensions("GL_APPLE_vertex_array_object");
	if (mHasAPPLEVAO)
	{
		llinfos << "Has GL_APPLE_vertex_array_object!" << llendl;
	}

#endif // LL_DARWIN

#if (LL_WINDOWS || LL_LINUX) && !LL_MESA_HEADLESS
	llinfos << "GL Probe: Getting symbols" << llendl;
	if (mHasVertexBufferObject)
	{
		glBindBufferARB = (PFNGLBINDBUFFERARBPROC)GLH_EXT_GET_PROC_ADDRESS("glBindBufferARB");
		if (glBindBufferARB)
		{
			glDeleteBuffersARB = (PFNGLDELETEBUFFERSARBPROC)GLH_EXT_GET_PROC_ADDRESS("glDeleteBuffersARB");
			glGenBuffersARB = (PFNGLGENBUFFERSARBPROC)GLH_EXT_GET_PROC_ADDRESS("glGenBuffersARB");
			glIsBufferARB = (PFNGLISBUFFERARBPROC)GLH_EXT_GET_PROC_ADDRESS("glIsBufferARB");
			glBufferDataARB = (PFNGLBUFFERDATAARBPROC)GLH_EXT_GET_PROC_ADDRESS("glBufferDataARB");
			glBufferSubDataARB = (PFNGLBUFFERSUBDATAARBPROC)GLH_EXT_GET_PROC_ADDRESS("glBufferSubDataARB");
			glGetBufferSubDataARB = (PFNGLGETBUFFERSUBDATAARBPROC)GLH_EXT_GET_PROC_ADDRESS("glGetBufferSubDataARB");
			glMapBufferARB = (PFNGLMAPBUFFERARBPROC)GLH_EXT_GET_PROC_ADDRESS("glMapBufferARB");
			glUnmapBufferARB = (PFNGLUNMAPBUFFERARBPROC)GLH_EXT_GET_PROC_ADDRESS("glUnmapBufferARB");
			glGetBufferParameterivARB = (PFNGLGETBUFFERPARAMETERIVARBPROC)GLH_EXT_GET_PROC_ADDRESS("glGetBufferParameterivARB");
			glGetBufferPointervARB = (PFNGLGETBUFFERPOINTERVARBPROC)GLH_EXT_GET_PROC_ADDRESS("glGetBufferPointervARB");
		}
		else
		{
			mHasVertexBufferObject = FALSE;
		}
	}
	if (mHasFramebufferObject)
	{
		glIsRenderbufferEXT = (PFNGLISRENDERBUFFEREXTPROC) GLH_EXT_GET_PROC_ADDRESS("glIsRenderbufferEXT");
		glBindRenderbufferEXT = (PFNGLBINDRENDERBUFFEREXTPROC) GLH_EXT_GET_PROC_ADDRESS("glBindRenderbufferEXT");
		glDeleteRenderbuffersEXT = (PFNGLDELETERENDERBUFFERSEXTPROC) GLH_EXT_GET_PROC_ADDRESS("glDeleteRenderbuffersEXT");
		glGenRenderbuffersEXT = (PFNGLGENRENDERBUFFERSEXTPROC) GLH_EXT_GET_PROC_ADDRESS("glGenRenderbuffersEXT");
		glRenderbufferStorageEXT = (PFNGLRENDERBUFFERSTORAGEEXTPROC) GLH_EXT_GET_PROC_ADDRESS("glRenderbufferStorageEXT");
		glGetRenderbufferParameterivEXT = (PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC) GLH_EXT_GET_PROC_ADDRESS("glGetRenderbufferParameterivEXT");
		glIsFramebufferEXT = (PFNGLISFRAMEBUFFEREXTPROC) GLH_EXT_GET_PROC_ADDRESS("glIsFramebufferEXT");
		glBindFramebufferEXT = (PFNGLBINDFRAMEBUFFEREXTPROC) GLH_EXT_GET_PROC_ADDRESS("glBindFramebufferEXT");
		glDeleteFramebuffersEXT = (PFNGLDELETEFRAMEBUFFERSEXTPROC) GLH_EXT_GET_PROC_ADDRESS("glDeleteFramebuffersEXT");
		glGenFramebuffersEXT = (PFNGLGENFRAMEBUFFERSEXTPROC) GLH_EXT_GET_PROC_ADDRESS("glGenFramebuffersEXT");
		glCheckFramebufferStatusEXT = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC) GLH_EXT_GET_PROC_ADDRESS("glCheckFramebufferStatusEXT");
		glFramebufferTexture1DEXT = (PFNGLFRAMEBUFFERTEXTURE1DEXTPROC) GLH_EXT_GET_PROC_ADDRESS("glFramebufferTexture1DEXT");
		glFramebufferTexture2DEXT = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC) GLH_EXT_GET_PROC_ADDRESS("glFramebufferTexture2DEXT");
		glFramebufferTexture3DEXT = (PFNGLFRAMEBUFFERTEXTURE3DEXTPROC) GLH_EXT_GET_PROC_ADDRESS("glFramebufferTexture3DEXT");
		glFramebufferRenderbufferEXT = (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC) GLH_EXT_GET_PROC_ADDRESS("glFramebufferRenderbufferEXT");
		glGetFramebufferAttachmentParameterivEXT = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC) GLH_EXT_GET_PROC_ADDRESS("glGetFramebufferAttachmentParameterivEXT");
		glGenerateMipmapEXT = (PFNGLGENERATEMIPMAPEXTPROC) GLH_EXT_GET_PROC_ADDRESS("glGenerateMipmapEXT");
	}
#if !LL_LINUX
	// This is expected to be a static symbol on Linux GL implementations
	glDrawRangeElements = (PFNGLDRAWRANGEELEMENTSPROC)GLH_EXT_GET_PROC_ADDRESS("glDrawRangeElements");
	if (!glDrawRangeElements)
	{
		mGLMaxVertexRange = 0;
		mGLMaxIndexRange = 0;
	}
#endif // !LL_LINUX
#if LL_LINUX
	// On Linux we need to get glColorTableEXT dynamically.
	if (mHasPalettedTextures)
	{
		glColorTableEXT = (PFNGLCOLORTABLEEXTPROC)GLH_EXT_GET_PROC_ADDRESS("glColorTableEXT");
	}
#endif // LL_LINUX
	if (mHasOcclusionQuery)
	{
		glGenQueriesARB = (PFNGLGENQUERIESARBPROC)GLH_EXT_GET_PROC_ADDRESS("glGenQueriesARB");
		glDeleteQueriesARB = (PFNGLDELETEQUERIESARBPROC)GLH_EXT_GET_PROC_ADDRESS("glDeleteQueriesARB");
		glIsQueryARB = (PFNGLISQUERYARBPROC)GLH_EXT_GET_PROC_ADDRESS("glIsQueryARB");
		glBeginQueryARB = (PFNGLBEGINQUERYARBPROC)GLH_EXT_GET_PROC_ADDRESS("glBeginQueryARB");
		glEndQueryARB = (PFNGLENDQUERYARBPROC)GLH_EXT_GET_PROC_ADDRESS("glEndQueryARB");
		glGetQueryivARB = (PFNGLGETQUERYIVARBPROC)GLH_EXT_GET_PROC_ADDRESS("glGetQueryivARB");
		glGetQueryObjectivARB = (PFNGLGETQUERYOBJECTIVARBPROC)GLH_EXT_GET_PROC_ADDRESS("glGetQueryObjectivARB");
		glGetQueryObjectuivARB = (PFNGLGETQUERYOBJECTUIVARBPROC)GLH_EXT_GET_PROC_ADDRESS("glGetQueryObjectuivARB");
	}
	if (mHasPointParameters)
	{
		glPointParameterfARB = (PFNGLPOINTPARAMETERFARBPROC)GLH_EXT_GET_PROC_ADDRESS("glPointParameterfARB");
		glPointParameterfvARB = (PFNGLPOINTPARAMETERFVARBPROC)GLH_EXT_GET_PROC_ADDRESS("glPointParameterfvARB");
	}
	if (mHasShaderObjects)
	{
		glDeleteObjectARB = (PFNGLDELETEOBJECTARBPROC) GLH_EXT_GET_PROC_ADDRESS("glDeleteObjectARB");
		glGetHandleARB = (PFNGLGETHANDLEARBPROC) GLH_EXT_GET_PROC_ADDRESS("glGetHandleARB");
		glDetachObjectARB = (PFNGLDETACHOBJECTARBPROC) GLH_EXT_GET_PROC_ADDRESS("glDetachObjectARB");
		glCreateShaderObjectARB = (PFNGLCREATESHADEROBJECTARBPROC) GLH_EXT_GET_PROC_ADDRESS("glCreateShaderObjectARB");
		glShaderSourceARB = (PFNGLSHADERSOURCEARBPROC) GLH_EXT_GET_PROC_ADDRESS("glShaderSourceARB");
		glCompileShaderARB = (PFNGLCOMPILESHADERARBPROC) GLH_EXT_GET_PROC_ADDRESS("glCompileShaderARB");
		glCreateProgramObjectARB = (PFNGLCREATEPROGRAMOBJECTARBPROC) GLH_EXT_GET_PROC_ADDRESS("glCreateProgramObjectARB");
		glAttachObjectARB = (PFNGLATTACHOBJECTARBPROC) GLH_EXT_GET_PROC_ADDRESS("glAttachObjectARB");
		glLinkProgramARB = (PFNGLLINKPROGRAMARBPROC) GLH_EXT_GET_PROC_ADDRESS("glLinkProgramARB");
		glUseProgramObjectARB = (PFNGLUSEPROGRAMOBJECTARBPROC) GLH_EXT_GET_PROC_ADDRESS("glUseProgramObjectARB");
		glValidateProgramARB = (PFNGLVALIDATEPROGRAMARBPROC) GLH_EXT_GET_PROC_ADDRESS("glValidateProgramARB");
		glUniform1fARB = (PFNGLUNIFORM1FARBPROC) GLH_EXT_GET_PROC_ADDRESS("glUniform1fARB");
		glUniform2fARB = (PFNGLUNIFORM2FARBPROC) GLH_EXT_GET_PROC_ADDRESS("glUniform2fARB");
		glUniform3fARB = (PFNGLUNIFORM3FARBPROC) GLH_EXT_GET_PROC_ADDRESS("glUniform3fARB");
		glUniform4fARB = (PFNGLUNIFORM4FARBPROC) GLH_EXT_GET_PROC_ADDRESS("glUniform4fARB");
		glUniform1iARB = (PFNGLUNIFORM1IARBPROC) GLH_EXT_GET_PROC_ADDRESS("glUniform1iARB");
		glUniform2iARB = (PFNGLUNIFORM2IARBPROC) GLH_EXT_GET_PROC_ADDRESS("glUniform2iARB");
		glUniform3iARB = (PFNGLUNIFORM3IARBPROC) GLH_EXT_GET_PROC_ADDRESS("glUniform3iARB");
		glUniform4iARB = (PFNGLUNIFORM4IARBPROC) GLH_EXT_GET_PROC_ADDRESS("glUniform4iARB");
		glUniform1fvARB = (PFNGLUNIFORM1FVARBPROC) GLH_EXT_GET_PROC_ADDRESS("glUniform1fvARB");
		glUniform2fvARB = (PFNGLUNIFORM2FVARBPROC) GLH_EXT_GET_PROC_ADDRESS("glUniform2fvARB");
		glUniform3fvARB = (PFNGLUNIFORM3FVARBPROC) GLH_EXT_GET_PROC_ADDRESS("glUniform3fvARB");
		glUniform4fvARB = (PFNGLUNIFORM4FVARBPROC) GLH_EXT_GET_PROC_ADDRESS("glUniform4fvARB");
		glUniform1ivARB = (PFNGLUNIFORM1IVARBPROC) GLH_EXT_GET_PROC_ADDRESS("glUniform1ivARB");
		glUniform2ivARB = (PFNGLUNIFORM2IVARBPROC) GLH_EXT_GET_PROC_ADDRESS("glUniform2ivARB");
		glUniform3ivARB = (PFNGLUNIFORM3IVARBPROC) GLH_EXT_GET_PROC_ADDRESS("glUniform3ivARB");
		glUniform4ivARB = (PFNGLUNIFORM4IVARBPROC) GLH_EXT_GET_PROC_ADDRESS("glUniform4ivARB");
		glUniformMatrix2fvARB = (PFNGLUNIFORMMATRIX2FVARBPROC) GLH_EXT_GET_PROC_ADDRESS("glUniformMatrix2fvARB");
		glUniformMatrix3fvARB = (PFNGLUNIFORMMATRIX3FVARBPROC) GLH_EXT_GET_PROC_ADDRESS("glUniformMatrix3fvARB");
		glUniformMatrix4fvARB = (PFNGLUNIFORMMATRIX4FVARBPROC) GLH_EXT_GET_PROC_ADDRESS("glUniformMatrix4fvARB");
		glGetObjectParameterfvARB = (PFNGLGETOBJECTPARAMETERFVARBPROC) GLH_EXT_GET_PROC_ADDRESS("glGetObjectParameterfvARB");
		glGetObjectParameterivARB = (PFNGLGETOBJECTPARAMETERIVARBPROC) GLH_EXT_GET_PROC_ADDRESS("glGetObjectParameterivARB");
		glGetInfoLogARB = (PFNGLGETINFOLOGARBPROC) GLH_EXT_GET_PROC_ADDRESS("glGetInfoLogARB");
		glGetAttachedObjectsARB = (PFNGLGETATTACHEDOBJECTSARBPROC) GLH_EXT_GET_PROC_ADDRESS("glGetAttachedObjectsARB");
		glGetUniformLocationARB = (PFNGLGETUNIFORMLOCATIONARBPROC) GLH_EXT_GET_PROC_ADDRESS("glGetUniformLocationARB");
		glGetActiveUniformARB = (PFNGLGETACTIVEUNIFORMARBPROC) GLH_EXT_GET_PROC_ADDRESS("glGetActiveUniformARB");
		glGetUniformfvARB = (PFNGLGETUNIFORMFVARBPROC) GLH_EXT_GET_PROC_ADDRESS("glGetUniformfvARB");
		glGetUniformivARB = (PFNGLGETUNIFORMIVARBPROC) GLH_EXT_GET_PROC_ADDRESS("glGetUniformivARB");
		glGetShaderSourceARB = (PFNGLGETSHADERSOURCEARBPROC) GLH_EXT_GET_PROC_ADDRESS("glGetShaderSourceARB");
	}
	if (mHasVertexShader)
	{
		glGetAttribLocationARB = (PFNGLGETATTRIBLOCATIONARBPROC) GLH_EXT_GET_PROC_ADDRESS("glGetAttribLocationARB");
		glBindAttribLocationARB = (PFNGLBINDATTRIBLOCATIONARBPROC) GLH_EXT_GET_PROC_ADDRESS("glBindAttribLocationARB");
		glGetActiveAttribARB = (PFNGLGETACTIVEATTRIBARBPROC) GLH_EXT_GET_PROC_ADDRESS("glGetActiveAttribARB");
		glVertexAttrib1dARB = (PFNGLVERTEXATTRIB1DARBPROC) GLH_EXT_GET_PROC_ADDRESS("glVertexAttrib1dARB");
		glVertexAttrib1dvARB = (PFNGLVERTEXATTRIB1DVARBPROC) GLH_EXT_GET_PROC_ADDRESS("glVertexAttrib1dvARB");
		glVertexAttrib1fARB = (PFNGLVERTEXATTRIB1FARBPROC) GLH_EXT_GET_PROC_ADDRESS("glVertexAttrib1fARB");
		glVertexAttrib1fvARB = (PFNGLVERTEXATTRIB1FVARBPROC) GLH_EXT_GET_PROC_ADDRESS("glVertexAttrib1fvARB");
		glVertexAttrib1sARB = (PFNGLVERTEXATTRIB1SARBPROC) GLH_EXT_GET_PROC_ADDRESS("glVertexAttrib1sARB");
		glVertexAttrib1svARB = (PFNGLVERTEXATTRIB1SVARBPROC) GLH_EXT_GET_PROC_ADDRESS("glVertexAttrib1svARB");
		glVertexAttrib2dARB = (PFNGLVERTEXATTRIB2DARBPROC) GLH_EXT_GET_PROC_ADDRESS("glVertexAttrib2dARB");
		glVertexAttrib2dvARB = (PFNGLVERTEXATTRIB2DVARBPROC) GLH_EXT_GET_PROC_ADDRESS("glVertexAttrib2dvARB");
		glVertexAttrib2fARB = (PFNGLVERTEXATTRIB2FARBPROC) GLH_EXT_GET_PROC_ADDRESS("glVertexAttrib2fARB");
		glVertexAttrib2fvARB = (PFNGLVERTEXATTRIB2FVARBPROC) GLH_EXT_GET_PROC_ADDRESS("glVertexAttrib2fvARB");
		glVertexAttrib2sARB = (PFNGLVERTEXATTRIB2SARBPROC) GLH_EXT_GET_PROC_ADDRESS("glVertexAttrib2sARB");
		glVertexAttrib2svARB = (PFNGLVERTEXATTRIB2SVARBPROC) GLH_EXT_GET_PROC_ADDRESS("glVertexAttrib2svARB");
		glVertexAttrib3dARB = (PFNGLVERTEXATTRIB3DARBPROC) GLH_EXT_GET_PROC_ADDRESS("glVertexAttrib3dARB");
		glVertexAttrib3dvARB = (PFNGLVERTEXATTRIB3DVARBPROC) GLH_EXT_GET_PROC_ADDRESS("glVertexAttrib3dvARB");
		glVertexAttrib3fARB = (PFNGLVERTEXATTRIB3FARBPROC) GLH_EXT_GET_PROC_ADDRESS("glVertexAttrib3fARB");
		glVertexAttrib3fvARB = (PFNGLVERTEXATTRIB3FVARBPROC) GLH_EXT_GET_PROC_ADDRESS("glVertexAttrib3fvARB");
		glVertexAttrib3sARB = (PFNGLVERTEXATTRIB3SARBPROC) GLH_EXT_GET_PROC_ADDRESS("glVertexAttrib3sARB");
		glVertexAttrib3svARB = (PFNGLVERTEXATTRIB3SVARBPROC) GLH_EXT_GET_PROC_ADDRESS("glVertexAttrib3svARB");
		glVertexAttrib4nbvARB = (PFNGLVERTEXATTRIB4NBVARBPROC) GLH_EXT_GET_PROC_ADDRESS("glVertexAttrib4nbvARB");
		glVertexAttrib4nivARB = (PFNGLVERTEXATTRIB4NIVARBPROC) GLH_EXT_GET_PROC_ADDRESS("glVertexAttrib4nivARB");
		glVertexAttrib4nsvARB = (PFNGLVERTEXATTRIB4NSVARBPROC) GLH_EXT_GET_PROC_ADDRESS("glVertexAttrib4nsvARB");
		glVertexAttrib4nubARB = (PFNGLVERTEXATTRIB4NUBARBPROC) GLH_EXT_GET_PROC_ADDRESS("glVertexAttrib4nubARB");
		glVertexAttrib4nubvARB = (PFNGLVERTEXATTRIB4NUBVARBPROC) GLH_EXT_GET_PROC_ADDRESS("glVertexAttrib4nubvARB");
		glVertexAttrib4nuivARB = (PFNGLVERTEXATTRIB4NUIVARBPROC) GLH_EXT_GET_PROC_ADDRESS("glVertexAttrib4nuivARB");
		glVertexAttrib4nusvARB = (PFNGLVERTEXATTRIB4NUSVARBPROC) GLH_EXT_GET_PROC_ADDRESS("glVertexAttrib4nusvARB");
		glVertexAttrib4bvARB = (PFNGLVERTEXATTRIB4BVARBPROC) GLH_EXT_GET_PROC_ADDRESS("glVertexAttrib4bvARB");
		glVertexAttrib4dARB = (PFNGLVERTEXATTRIB4DARBPROC) GLH_EXT_GET_PROC_ADDRESS("glVertexAttrib4dARB");
		glVertexAttrib4dvARB = (PFNGLVERTEXATTRIB4DVARBPROC) GLH_EXT_GET_PROC_ADDRESS("glVertexAttrib4dvARB");
		glVertexAttrib4fARB = (PFNGLVERTEXATTRIB4FARBPROC) GLH_EXT_GET_PROC_ADDRESS("glVertexAttrib4fARB");
		glVertexAttrib4fvARB = (PFNGLVERTEXATTRIB4FVARBPROC) GLH_EXT_GET_PROC_ADDRESS("glVertexAttrib4fvARB");
		glVertexAttrib4ivARB = (PFNGLVERTEXATTRIB4IVARBPROC) GLH_EXT_GET_PROC_ADDRESS("glVertexAttrib4ivARB");
		glVertexAttrib4sARB = (PFNGLVERTEXATTRIB4SARBPROC) GLH_EXT_GET_PROC_ADDRESS("glVertexAttrib4sARB");
		glVertexAttrib4svARB = (PFNGLVERTEXATTRIB4SVARBPROC) GLH_EXT_GET_PROC_ADDRESS("glVertexAttrib4svARB");
		glVertexAttrib4ubvARB = (PFNGLVERTEXATTRIB4UBVARBPROC) GLH_EXT_GET_PROC_ADDRESS("glVertexAttrib4ubvARB");
		glVertexAttrib4uivARB = (PFNGLVERTEXATTRIB4UIVARBPROC) GLH_EXT_GET_PROC_ADDRESS("glVertexAttrib4uivARB");
		glVertexAttrib4usvARB = (PFNGLVERTEXATTRIB4USVARBPROC) GLH_EXT_GET_PROC_ADDRESS("glVertexAttrib4usvARB");
		glVertexAttribPointerARB = (PFNGLVERTEXATTRIBPOINTERARBPROC) GLH_EXT_GET_PROC_ADDRESS("glVertexAttribPointerARB");
		glEnableVertexAttribArrayARB = (PFNGLENABLEVERTEXATTRIBARRAYARBPROC) GLH_EXT_GET_PROC_ADDRESS("glEnableVertexAttribArrayARB");
		glDisableVertexAttribArrayARB = (PFNGLDISABLEVERTEXATTRIBARRAYARBPROC) GLH_EXT_GET_PROC_ADDRESS("glDisableVertexAttribArrayARB");
		glProgramStringARB = (PFNGLPROGRAMSTRINGARBPROC) GLH_EXT_GET_PROC_ADDRESS("glProgramStringARB");
		glBindProgramARB = (PFNGLBINDPROGRAMARBPROC) GLH_EXT_GET_PROC_ADDRESS("glBindProgramARB");
		glDeleteProgramsARB = (PFNGLDELETEPROGRAMSARBPROC) GLH_EXT_GET_PROC_ADDRESS("glDeleteProgramsARB");
		glGenProgramsARB = (PFNGLGENPROGRAMSARBPROC) GLH_EXT_GET_PROC_ADDRESS("glGenProgramsARB");
		glProgramEnvParameter4dARB = (PFNGLPROGRAMENVPARAMETER4DARBPROC) GLH_EXT_GET_PROC_ADDRESS("glProgramEnvParameter4dARB");
		glProgramEnvParameter4dvARB = (PFNGLPROGRAMENVPARAMETER4DVARBPROC) GLH_EXT_GET_PROC_ADDRESS("glProgramEnvParameter4dvARB");
		glProgramEnvParameter4fARB = (PFNGLPROGRAMENVPARAMETER4FARBPROC) GLH_EXT_GET_PROC_ADDRESS("glProgramEnvParameter4fARB");
		glProgramEnvParameter4fvARB = (PFNGLPROGRAMENVPARAMETER4FVARBPROC) GLH_EXT_GET_PROC_ADDRESS("glProgramEnvParameter4fvARB");
		glProgramLocalParameter4dARB = (PFNGLPROGRAMLOCALPARAMETER4DARBPROC) GLH_EXT_GET_PROC_ADDRESS("glProgramLocalParameter4dARB");
		glProgramLocalParameter4dvARB = (PFNGLPROGRAMLOCALPARAMETER4DVARBPROC) GLH_EXT_GET_PROC_ADDRESS("glProgramLocalParameter4dvARB");
		glProgramLocalParameter4fARB = (PFNGLPROGRAMLOCALPARAMETER4FARBPROC) GLH_EXT_GET_PROC_ADDRESS("glProgramLocalParameter4fARB");
		glProgramLocalParameter4fvARB = (PFNGLPROGRAMLOCALPARAMETER4FVARBPROC) GLH_EXT_GET_PROC_ADDRESS("glProgramLocalParameter4fvARB");
		glGetProgramEnvParameterdvARB = (PFNGLGETPROGRAMENVPARAMETERDVARBPROC) GLH_EXT_GET_PROC_ADDRESS("glGetProgramEnvParameterdvARB");
		glGetProgramEnvParameterfvARB = (PFNGLGETPROGRAMENVPARAMETERFVARBPROC) GLH_EXT_GET_PROC_ADDRESS("glGetProgramEnvParameterfvARB");
		glGetProgramLocalParameterdvARB = (PFNGLGETPROGRAMLOCALPARAMETERDVARBPROC) GLH_EXT_GET_PROC_ADDRESS("glGetProgramLocalParameterdvARB");
		glGetProgramLocalParameterfvARB = (PFNGLGETPROGRAMLOCALPARAMETERFVARBPROC) GLH_EXT_GET_PROC_ADDRESS("glGetProgramLocalParameterfvARB");
		glGetProgramivARB = (PFNGLGETPROGRAMIVARBPROC) GLH_EXT_GET_PROC_ADDRESS("glGetProgramivARB");
		glGetProgramStringARB = (PFNGLGETPROGRAMSTRINGARBPROC) GLH_EXT_GET_PROC_ADDRESS("glGetProgramStringARB");
		glGetVertexAttribdvARB = (PFNGLGETVERTEXATTRIBDVARBPROC) GLH_EXT_GET_PROC_ADDRESS("glGetVertexAttribdvARB");
		glGetVertexAttribfvARB = (PFNGLGETVERTEXATTRIBFVARBPROC) GLH_EXT_GET_PROC_ADDRESS("glGetVertexAttribfvARB");
		glGetVertexAttribivARB = (PFNGLGETVERTEXATTRIBIVARBPROC) GLH_EXT_GET_PROC_ADDRESS("glGetVertexAttribivARB");
		glGetVertexAttribPointervARB = (PFNGLGETVERTEXATTRIBPOINTERVARBPROC) GLH_EXT_GET_PROC_ADDRESS("glgetVertexAttribPointervARB");
		glIsProgramARB = (PFNGLISPROGRAMARBPROC) GLH_EXT_GET_PROC_ADDRESS("glIsProgramARB");
	}
	llinfos << "GL Probe: Got symbols" << llendl;
#endif

	mInited = TRUE;
}

void rotate_quat(LLQuaternion& rotation)
{
	F32 angle_radians, x, y, z;
	rotation.getAngleAxis(&angle_radians, &x, &y, &z);
	glRotatef(angle_radians * RAD_TO_DEG, x, y, z);
}

void flush_glerror()
{
	glGetError();
}

void assert_glerror()
{
	if (gNoRender) 
	{
		return;
	}
	if (!gGLManager.mInited)
	{
		llerrs << "GL not initialized" << llendl;
	}
	//  Create or update texture to be used with this data 
	GLenum error;
	error = glGetError();
	if (error)
	{
#ifndef LL_LINUX // *FIX: !  This should be an error for linux as well.
		llerrs << "GL Error:" << gluErrorString(error) << llendl;
#endif
	}
}

void clear_glerror()
{
	//  Create or update texture to be used with this data 
	GLenum error;
	error = glGetError();
}

//============================================================================

//
// LLGLState
//

// Static members
std::map<LLGLenum, LLGLboolean> LLGLState::sStateMap;

GLboolean LLGLDepthTest::sDepthEnabled = GL_FALSE; // OpenGL default
GLenum LLGLDepthTest::sDepthFunc = GL_LESS; // OpenGL default
GLboolean LLGLDepthTest::sWriteEnabled = GL_TRUE; // OpenGL default

//static
void LLGLState::initClass() 
{
	sStateMap[GL_DITHER] = GL_TRUE;
}

//static
void LLGLState::restoreGL()
{
	sStateMap.clear();
	initClass();
}

//static
// Really shouldn't be needed, but seems we sometimes do.
void LLGLState::resetTextureStates()
{
	GLint maxTextureUnits;
	glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &maxTextureUnits);
	for (S32 j = maxTextureUnits-1; j >=0; j--)
	{
		glActiveTextureARB(GL_TEXTURE0_ARB+j);
		glClientActiveTextureARB(GL_TEXTURE0_ARB+j);
		j == 0 ? glEnable(GL_TEXTURE_2D) : glDisable(GL_TEXTURE_2D);
	}
}

void LLGLState::dumpStates() 
{
	llinfos << "GL States:" << llendl;
	for (std::map<LLGLenum, LLGLboolean>::iterator iter = sStateMap.begin();
		 iter != sStateMap.end(); ++iter)
	{
		llinfos << llformat(" 0x%04x : %s",(S32)iter->first,iter->second?"TRUE":"FALSE") << llendl;
	}
}

void LLGLState::checkStates()  
{
#if LL_DEBUG_GL
	stop_glerror();

	GLint activeTexture;
	glGetIntegerv(GL_ACTIVE_TEXTURE_ARB, &activeTexture);
	
	if (activeTexture != GL_TEXTURE0_ARB)
	{
		LL_GL_ERRS << "Texture channel corrupted. " << llendl;
	}
	
	GLint src;
	GLint dst;
	glGetIntegerv(GL_BLEND_SRC, &src);
	glGetIntegerv(GL_BLEND_DST, &dst);
	
	if (src != GL_SRC_ALPHA || dst != GL_ONE_MINUS_SRC_ALPHA)
	{
		LL_GL_ERRS << "Blend function corrupted: " << std::hex << src << " " << std::hex << dst << llendl;
	}
	
	for (std::map<LLGLenum, LLGLboolean>::iterator iter = sStateMap.begin();
		 iter != sStateMap.end(); ++iter)
	{
		LLGLenum state = iter->first;
		LLGLboolean cur_state = iter->second;
		LLGLboolean gl_state = glIsEnabled(state);
		if(cur_state != gl_state)
		{
			dumpStates();
			LL_GL_ERRS << llformat("LLGLState error. State: 0x%04x",state) << llendl;
		}
	}
	
	stop_glerror();
#endif
}

void LLGLState::checkTextureChannels()
{
#if LL_DEBUG_GL
	GLint activeTexture;
	glGetIntegerv(GL_ACTIVE_TEXTURE_ARB, &activeTexture);
	
	BOOL error = FALSE;

	if (activeTexture != GL_TEXTURE0_ARB)
	{
		error = TRUE;
 		llwarns << "Active texture channel corrupted. " << llendl;
	}
	
	GLint maxTextureUnits;
	glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &maxTextureUnits);

	static const char* label[] =
	{
		"GL_TEXTURE_2D",
		"GL_TEXTURE_COORD_ARRAY",
		"GL_TEXTURE_1D",
		"GL_TEXTURE_CUBE_MAP_ARB",
		"GL_TEXTURE_GEN_S",
		"GL_TEXTURE_GEN_T",
		"GL_TEXTURE_GEN_Q",
		"GL_TEXTURE_GEN_R"
	};

	static GLint value[] =
	{
		GL_TEXTURE_2D,
		GL_TEXTURE_COORD_ARRAY,
		GL_TEXTURE_1D,
		GL_TEXTURE_CUBE_MAP_ARB,
		GL_TEXTURE_GEN_S,
		GL_TEXTURE_GEN_T,
		GL_TEXTURE_GEN_Q,
		GL_TEXTURE_GEN_R
	};

	GLint stackDepth = 0;
	LLMatrix4 identity;
	LLMatrix4 matrix;

	for (GLint i = 0; i < maxTextureUnits; i++)
	{
		glActiveTextureARB(GL_TEXTURE0_ARB+i);
		glClientActiveTextureARB(GL_TEXTURE0_ARB+i);

		glGetIntegerv(GL_TEXTURE_STACK_DEPTH, &stackDepth);

		if (stackDepth != 1)
		{
			error = TRUE;
			llwarns << "Texture matrix stack corrupted." << llendl;
		}

		glGetFloatv(GL_TEXTURE_MATRIX, (GLfloat*) matrix.mMatrix);

		if (matrix != identity)
		{
			error = TRUE;
			llwarns << "Texture matrix in channel " << i << " corrupt." << llendl;
		}

		for (S32 j = (i == 0 ? 2 : 0); j < 8; j++)
		{
			if (glIsEnabled(value[j]))
			{
				error = TRUE;
				llwarns << "Texture channel " << i << " still has " << label[j] << " enabled." << llendl;
			}
		}
	}

	glActiveTextureARB(GL_TEXTURE0_ARB);
	glClientActiveTextureARB(GL_TEXTURE0_ARB);

	if (error)
	{
		LL_GL_ERRS << "GL texture state corruption detected." << llendl;
	}
#endif
}

void LLGLState::checkClientArrays(U32 data_mask)
{
#if LL_DEBUG_GL
	stop_glerror();
	BOOL error = FALSE;
	static const char* label[] =
	{
		"GL_VERTEX_ARRAY",
		"GL_NORMAL_ARRAY",
		"GL_COLOR_ARRAY",
		"GL_TEXTURE_COORD_ARRAY"
	};

	static GLint value[] =
	{
		GL_VERTEX_ARRAY,
		GL_NORMAL_ARRAY,
		GL_COLOR_ARRAY,
		GL_TEXTURE_COORD_ARRAY
	};

	 U32 mask[] = 
	{ //copied from llvertexbuffer.h
		0x0001, //MAP_VERTEX,
		0x0002, //MAP_NORMAL,
		0x0010, //MAP_COLOR,
		0x0004, //MAP_TEXCOORD
	};


	for (S32 j = 0; j < 4; j++)
	{
		if (glIsEnabled(value[j]))
		{
			if (!(mask[j] & data_mask))
			{
				error = TRUE;
				llwarns << "GL still has " << label[j] << " enabled." << llendl;
			}
		}
		else
		{
			if (mask[j] & data_mask)
			{
				error = TRUE;
				llwarns << "GL does not have " << label[j] << " enabled." << llendl;
			}
		}
	}

	glClientActiveTextureARB(GL_TEXTURE1_ARB);
	glActiveTextureARB(GL_TEXTURE1_ARB);
	if (glIsEnabled(GL_TEXTURE_COORD_ARRAY))
	{
		if (!(data_mask & 0x0008))
		{
			error = TRUE;
			llwarns << "GL still has GL_TEXTURE_COORD_ARRAY enabled on channel 1." << llendl;
		}
	}
	else
	{
		if (data_mask & 0x0008)
		{
			error = TRUE;
			llwarns << "GL does not have GL_TEXTURE_COORD_ARRAY enabled on channel 1." << llendl;
		}
	}

	if (glIsEnabled(GL_TEXTURE_2D))
	{
		if (!(data_mask & 0x0008))
		{
			error = TRUE;
			llwarns << "GL still has GL_TEXTURE_2D enabled on channel 1." << llendl;
		}
	}
	else
	{
		if (data_mask & 0x0008)
		{
			error = TRUE;
			llwarns << "GL does not have GL_TEXTURE_2D enabled on channel 1." << llendl;
		}
	}

	glClientActiveTextureARB(GL_TEXTURE0_ARB);
	glActiveTextureARB(GL_TEXTURE0_ARB);

	if (error)
	{
		LL_GL_ERRS << "GL client array corruption detected." << llendl;
	}
#endif
}

//============================================================================

LLGLState::LLGLState(LLGLenum state, S32 enabled)
{
	stop_glerror();
	mState = state;
	if (state)
	{
		mWasEnabled = sStateMap[state];
		llassert(mWasEnabled == glIsEnabled(state));
		setEnabled(enabled);
		stop_glerror();
	}
}

void LLGLState::setEnabled(S32 enabled)
{
	if (!mState)
	{
		return;
	}
	if (enabled == CURRENT_STATE)
	{
		enabled = sStateMap[mState] == GL_TRUE ? TRUE : FALSE;
	}
	else if (enabled == TRUE && sStateMap[mState] != GL_TRUE)
	{
		glEnable(mState);
		sStateMap[mState] = GL_TRUE;
	}
	else if (enabled == FALSE && sStateMap[mState] != GL_FALSE)
	{
		glDisable(mState);
		sStateMap[mState] = GL_FALSE;
	}
	mIsEnabled = enabled;
}

LLGLState::~LLGLState() 
{
	stop_glerror();
	if (mState)
	{
#if LL_DEBUG_GL
		llassert(sStateMap[mState] == glIsEnabled(mState));
#endif
		if (mIsEnabled != mWasEnabled)
		{
			if (mWasEnabled)
			{
				glEnable(mState);
				sStateMap[mState] = GL_TRUE;
			}
			else
			{
				glDisable(mState);
				sStateMap[mState] = GL_FALSE;
			}
		}
	}
	stop_glerror();
}

//============================================================================

void LLGLManager::initGLStates()
{
	//gl states moved to classes in llglstates.h
	LLGLState::initClass();
}

//============================================================================

void enable_vertex_weighting(const S32 index)
{
#if GL_ARB_vertex_program
	if (index > 0) glEnableVertexAttribArrayARB(index);	// vertex weights
#endif
}

void disable_vertex_weighting(const S32 index)
{
#if GL_ARB_vertex_program
	if (index > 0) glDisableVertexAttribArrayARB(index);	// vertex weights
#endif
}

void enable_binormals(const S32 index)
{
#if GL_ARB_vertex_program
	if (index > 0)
	{
		glEnableVertexAttribArrayARB(index);	// binormals
	}
#endif
}

void disable_binormals(const S32 index)
{
#if GL_ARB_vertex_program
	if (index > 0)
	{
		glDisableVertexAttribArrayARB(index);	// binormals
	}
#endif
}


void enable_cloth_weights(const S32 index)
{
#if GL_ARB_vertex_program
	if (index > 0)	glEnableVertexAttribArrayARB(index);
#endif
}

void disable_cloth_weights(const S32 index)
{
#if GL_ARB_vertex_program
	if (index > 0) glDisableVertexAttribArrayARB(index);
#endif
}

void set_vertex_weights(const S32 index, const U32 stride, const F32 *weights)
{
#if GL_ARB_vertex_program
	if (index > 0) glVertexAttribPointerARB(index, 1, GL_FLOAT, FALSE, stride, weights);
	stop_glerror();
#endif
}

void set_vertex_clothing_weights(const S32 index, const U32 stride, const LLVector4 *weights)
{
#if GL_ARB_vertex_program
	if (index > 0) glVertexAttribPointerARB(index, 4, GL_FLOAT, TRUE, stride, weights);
	stop_glerror();
#endif
}

void set_binormals(const S32 index, const U32 stride,const LLVector3 *binormals)
{
#if GL_ARB_vertex_program
	if (index > 0) glVertexAttribPointerARB(index, 3, GL_FLOAT, FALSE, stride, binormals);
	stop_glerror();
#endif
}


void set_palette(U8 *palette_data)
{
	if (gGLManager.mHasPalettedTextures)
	{
		glColorTableEXT(GL_TEXTURE_2D, GL_RGBA8, 256, GL_RGBA, GL_UNSIGNED_BYTE, palette_data);
	}
}


void parse_gl_version( S32* major, S32* minor, S32* release, LLString* vendor_specific )
{
	// GL_VERSION returns a null-terminated string with the format: 
	// <major>.<minor>[.<release>] [<vendor specific>]

	const char* version = (const char*) glGetString(GL_VERSION);
	*major = 0;
	*minor = 0;
	*release = 0;
	vendor_specific->assign("");

	if( !version )
	{
		return;
	}

	LLString ver_copy( version );
	S32 len = (S32)strlen( version );	/* Flawfinder: ignore */
	S32 i = 0;
	S32 start;
	// Find the major version
	start = i;
	for( ; i < len; i++ )
	{
		if( '.' == version[i] )
		{
			break;
		}
	}
	LLString major_str = ver_copy.substr(start,i-start);
	LLString::convertToS32(major_str, *major);

	if( '.' == version[i] )
	{
		i++;
	}

	// Find the minor version
	start = i;
	for( ; i < len; i++ )
	{
		if( ('.' == version[i]) || isspace(version[i]) )
		{
			break;
		}
	}
	LLString minor_str = ver_copy.substr(start,i-start);
	LLString::convertToS32(minor_str, *minor);

	// Find the release number (optional)
	if( '.' == version[i] )
	{
		i++;

		start = i;
		for( ; i < len; i++ )
		{
			if( isspace(version[i]) )
			{
				break;
			}
		}

		LLString release_str = ver_copy.substr(start,i-start);
		LLString::convertToS32(release_str, *release);
	}

	// Skip over any white space
	while( version[i] && isspace( version[i] ) )
	{
		i++;
	}

	// Copy the vendor-specific string (optional)
	if( version[i] )
	{
		vendor_specific->assign( version + i );
	}
}
