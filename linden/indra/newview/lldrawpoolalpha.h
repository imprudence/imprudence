/** 
 * @file lldrawpoolalpha.h
 * @brief LLDrawPoolAlpha class definition
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

#ifndef LL_LLDRAWPOOLALPHA_H
#define LL_LLDRAWPOOLALPHA_H

#include "lldrawpool.h"
#include "llviewerimage.h"
#include "llframetimer.h"

class LLFace;
class LLColor4;

class LLDrawPoolAlpha: public LLDrawPool
{
public:
	LLDrawPoolAlpha();
	/*virtual*/ ~LLDrawPoolAlpha();

	/*virtual*/ LLDrawPool *instancePool();

	/*virtual*/ void beginRenderPass(S32 pass = 0);
	/*virtual*/ void render(S32 pass = 0);
	/*virtual*/ void renderFaceSelected(LLFace *facep, LLImageGL *image, const LLColor4 &color,
										const S32 index_offset = 0, const S32 index_count = 0);
	/*virtual*/ void prerender();
	/*virtual*/ void renderForSelect();

	/*virtual*/ void enqueue(LLFace *face);
	/*virtual*/ BOOL removeFace(LLFace *face);
	/*virtual*/ void resetDrawOrders();

	/*virtual*/ void enableShade();
	/*virtual*/ void disableShade();
	/*virtual*/ void setShade(F32 shade);


	virtual S32 getMaterialAttribIndex();

	BOOL mRebuiltLastFrame;
	enum
	{
		NUM_ALPHA_BINS = 1024
	};

	/*virtual*/ BOOL verify() const;
	/*virtual*/ LLViewerImage *getDebugTexture();
	/*virtual*/ LLColor3 getDebugColor() const; // For AGP debug display

	static BOOL sShowDebugAlpha;
protected:
	F32 mMinDistance;
	F32 mMaxDistance;
	F32 mInvBinSize;

	LLDynamicArray<LLFace*> mDistanceBins[NUM_ALPHA_BINS];
};

#endif // LL_LLDRAWPOOLALPHA_H
