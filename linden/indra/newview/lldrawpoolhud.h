/** 
 * @file lldrawpoolhud.h
 * @brief LLDrawPoolHUD class definition
 *
 * Copyright (c) 2006-2007, Linden Research, Inc.
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

#ifndef LL_LLDRAWPOOLHUD_H
#define LL_LLDRAWPOOLHUD_H

#include "lldrawpool.h"

class LLDrawPoolHUD : public LLDrawPool
{
public:
	LLDrawPoolHUD();
// 	/*virtual*/ ~LLDrawPoolHUD();

	/*virtual*/ LLDrawPool *instancePool();

	/*virtual*/ void prerender();
	/*virtual*/ void beginRenderPass(S32 pass);
	/*virtual*/ void render(S32 pass = 0);
	/*virtual*/ void endRenderPass( S32 pass );
	/*virtual*/ void renderForSelect();
	/*virtual*/ void renderFaceSelected(LLFace* facep, LLImageGL* image, const LLColor4& color,
										const S32 index_offset = 0, const S32 index_count = 0);
// 	/*virtual*/ S32 rebuild();
	
// 	/*virtual*/ LLViewerImage *getTexture();
// 	/*virtual*/ LLViewerImage *getDebugTexture();
	/*virtual*/ void dirtyTexture(const LLViewerImage *texturep);

 	/*virtual*/ void resetDrawOrders();

 	/*virtual*/ void enqueue(LLFace *face);
 	/*virtual*/ BOOL addFace(LLFace *face);
 	/*virtual*/ BOOL removeFace(LLFace *face);

	virtual S32 getMaterialAttribIndex();

// 	/*virtual*/ BOOL verify() const;		// Verify that all data in the draw pool is correct!
	/*virtual*/ LLColor3 getDebugColor() const; // For AGP debug display

public:
	static BOOL sShowHUDAttachments;

protected:
	typedef std::map<LLViewerImage*, face_array_t> bump_face_map_t;
	bump_face_map_t mBumpFaces;
	face_array_t mAlphaFace;
	S32 mRenderMode; // RENDER_TYPE_VOLUME, RENDER_TYPE_BUMP, or RENDER_TYPE_ALPHA
};

#endif // LL_LLDRAWPOOLHUD_H
