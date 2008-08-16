/** 
 * @file lldrawpoolsimple.h
 * @brief LLDrawPoolSimple class definition
 *
 * Copyright (c) 2002-2007, Linden Research, Inc.
 * 
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

#ifndef LL_LLDRAWPOOLSIMPLE_H
#define LL_LLDRAWPOOLSIMPLE_H

#include "lldrawpool.h"

class LLFRInfo
{
public:
	U32 mPrimType;
	U32 mGeomIndex;
	U32 mGeomIndexEnd;
	U32 mNumIndices;
	U32 mIndicesStart;

	LLFRInfo()
	{
	}

	LLFRInfo(const U32 pt, const U32 gi, const U32 gc, const U32 ni, const U32 is) :
		mPrimType(pt),
		mGeomIndex(gi),
		mGeomIndexEnd(gi+gc),
		mNumIndices(ni),
		mIndicesStart(is)
	{
	}
};

class LLDrawPoolSimple : public LLDrawPool
{
	LLPointer<LLViewerImage> mTexturep;
public:
	enum
	{
		SHADER_LEVEL_LOCAL_LIGHTS = 2
	};
	
	LLDrawPoolSimple(LLViewerImage *texturep);

	/*virtual*/ LLDrawPool *instancePool();

	/*virtual*/ void beginRenderPass(S32 pass);
	/*virtual*/ void endRenderPass(S32 pass);
	/*virtual*/ void render(S32 pass = 0);
	/*virtual*/ void renderFaceSelected(LLFace *facep, 
									LLImageGL *image, 
									const LLColor4 &color,
									const S32 index_offset = 0, const S32 index_count = 0);
	/*virtual*/ void prerender();
	/*virtual*/ void renderForSelect();
	/*virtual*/ void dirtyTexture(const LLViewerImage *texturep);
	/*virtual*/ LLViewerImage *getTexture();
	/*virtual*/ LLViewerImage *getDebugTexture();
	/*virtual*/ LLColor3 getDebugColor() const; // For AGP debug display
	/*virtual*/ BOOL match(LLFace* last_face, LLFace* facep);

	/*virtual*/ void enableShade();
	/*virtual*/ void disableShade();
	/*virtual*/ void setShade(F32 shade);

	virtual S32 getMaterialAttribIndex();

	static S32 sDiffTex;
};

#endif // LL_LLDRAWPOOLSIMPLE_H
