/** 
 * @file llgl.h
 * @brief LLGL definition
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

#ifndef LL_LLGL_H
#define LL_LLGL_H

// This file contains various stuff for handling gl extensions and other gl related stuff.

#include <string>
#include <map>

#include "llerror.h"
#include "v4color.h"
#include "llstring.h"
#include "stdtypes.h"
#include "v4math.h"
#include "llgltypes.h"

#define LL_DEBUG_GL 1

#define LL_GL_ERRS llerrs

// Manage GL extensions...
class LLGLManager
{
public:
	LLGLManager();

	bool initGL();
	void shutdownGL();

	void initWGL(); // Initializes stupid WGL extensions

	LLString getRawGLString(); // For sending to simulator

	BOOL mInited;
	BOOL mIsDisabled;

	// Extensions used by everyone
	BOOL mHasMultitexture;
	S32	 mNumTextureUnits;
	BOOL mHasMipMapGeneration;
	BOOL mHasPalettedTextures;
	BOOL mHasCompressedTextures;
	BOOL mHasFramebufferObject;

	// ARB Extensions
	BOOL mHasVertexBufferObject;
	BOOL mHasPBuffer;
	BOOL mHasShaderObjects;
	BOOL mHasVertexShader;
	BOOL mHasFragmentShader;
	BOOL mHasOcclusionQuery;
	BOOL mHasPointParameters;

	// Other extensions.
	BOOL mHasAnisotropic;
	BOOL mHasARBEnvCombine;
	BOOL mHasCubeMap;

	// Vender specific extensions
	BOOL mIsATI;
	BOOL mIsNVIDIA;
	BOOL mIsIntel;
	BOOL mIsGF2or4MX;
	BOOL mIsGF3;
	BOOL mIsGFFX;
	BOOL mATIOffsetVerticalLines;

#if LL_WINDOWS
	BOOL mHasWGLARBPixelFormat;
#endif // LL_WINDOWS

#if LL_DARWIN
	// Apple extensions.
	BOOL mHasAPPLEVertexArrayRange;
	BOOL mHasAPPLEFence;
	BOOL mHasAPPLEVAO;
#endif

	// Misc exitensions
	BOOL mHasSeparateSpecularColor;
	
	S32 mDriverVersionMajor;
	S32 mDriverVersionMinor;
	S32 mDriverVersionRelease;
	F32 mGLVersion; // e.g = 1.4
	LLString mDriverVersionVendorString;

	S32 mVRAM; // VRAM in MB
	S32 mGLMaxVertexRange;
	S32 mGLMaxIndexRange;
	
	void getPixelFormat(); // Get the best pixel format

	LLString getGLInfoString();

	// In ALL CAPS
	LLString mGLVendor;
	LLString mGLVendorShort;

	// In ALL CAPS
	LLString mGLRenderer;

private:
	void initExtensions();
	void initGLStates();
	void initGLImages();
};

extern LLGLManager gGLManager;

class LLQuaternion;
class LLMatrix4;

void rotate_quat(LLQuaternion& rotation);

void flush_glerror(); // Flush GL errors when we know we're handling them correctly.

void assert_glerror();

void clear_glerror();

#if LL_DEBUG
# define stop_glerror() assert_glerror()
# define llglassertok() assert_glerror()
#else
# define stop_glerror()
# define llglassertok()
#endif

#define llglassertok_always() assert_glerror()

////////////////////////
//
// Note: U32's are GLEnum's...
//

// This is a class for GL state management

/*
	GL STATE MANAGEMENT DESCRIPTION

	LLGLState and its two subclasses, LLGLEnable and LLGLDisable, manage the current 
	enable/disable states of the GL to prevent redundant setting of state within a 
	render path or the accidental corruption of what state the next path expects.

	Essentially, wherever you would call glEnable set a state and then
	subsequently reset it by calling glDisable (or vice versa), make an instance of 
	LLGLEnable with the state you want to set, and assume it will be restored to its
	original state when that instance of LLGLEnable is destroyed.  It is good practice
	to exploit stack frame controls for optimal setting/unsetting and readability of 
	code.  In llglstates.h, there are a collection of helper classes that define groups
	of enables/disables that can cause multiple states to be set with the creation of
	one instance.  

	Sample usage:

	//disable lighting for rendering hud objects
	//INCORRECT USAGE
	LLGLEnable lighting(GL_LIGHTING);
	renderHUD();
	LLGLDisable lighting(GL_LIGHTING);

	//CORRECT USAGE
	{
		LLGLEnable lighting(GL_LIGHTING);
		renderHUD();
	}

	If a state is to be set on a conditional, the following mechanism
	is useful:

	{
		LLGLEnable lighting(light_hud ? GL_LIGHTING : 0);
		renderHUD();
	}

	A LLGLState initialized with a parameter of 0 does nothing.

	LLGLState works by maintaining a map of the current GL states, and ignoring redundant
	enables/disables.  If a redundant call is attempted, it becomes a noop, otherwise,
	it is set in the constructor and reset in the destructor.  

	For debugging GL state corruption, running with debug enabled will trigger asserts
	if the existing GL state does not match the expected GL state.

*/
class LLGLState
{
public:
	static void initClass();
	static void restoreGL();

	static void resetTextureStates();
	static void dumpStates();
	static void checkStates();
	static void checkTextureChannels();
	static void checkClientArrays(U32 data_mask = 0x0001);
	
protected:
	static std::map<LLGLenum, LLGLboolean> sStateMap;
	
public:
	enum { CURRENT_STATE = -2 };
	LLGLState(LLGLenum state, S32 enabled = CURRENT_STATE);
	~LLGLState();
	void setEnabled(S32 enabled);
	void enable() { setEnabled(TRUE); }
	void disable() { setEnabled(FALSE); }
protected:
	LLGLenum mState;
	BOOL mWasEnabled;
	BOOL mIsEnabled;
};

class LLGLEnable : public LLGLState
{
public:
	LLGLEnable(LLGLenum state) : LLGLState(state, TRUE) {}
};

class LLGLDisable : public LLGLState
{
public:
	LLGLDisable(LLGLenum state) : LLGLState(state, FALSE) {}
};

#include "llglstates.h"

void init_glstates();
void enable_vertex_weighting(const S32 index);
void disable_vertex_weighting(const S32 index);
void enable_binormals(const S32 index);
void disable_binormals(const S32 index);
void enable_cloth_weights(const S32 index);
void disable_cloth_weights(const S32 index);
void set_vertex_weights(const S32 index, const U32 stride, const F32 *weights);
void set_vertex_clothing_weights(const S32 index, const U32 stride, const LLVector4 *weights);
void set_binormals(const S32 index, const U32 stride, const LLVector3 *binormals);
void set_palette(U8* palette_data);
void parse_gl_version( S32* major, S32* minor, S32* release, LLString* vendor_specific );

extern BOOL gClothRipple;
extern BOOL gNoRender;
#endif // LL_LLGL_H
