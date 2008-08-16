/** 
 * @file lldrawpooltreenew.h
 * @brief LLDrawPoolTreeNew class definition
 *
 * Copyright (c) 2003-2007, Linden Research, Inc.
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

#ifndef LL_LLDRAWPOOLTREENEW_H
#define LL_LLDRAWPOOLTREENEW_H

#include "lldrawpool.h"

class LLDrawPoolTreeNew : public LLDrawPool
{
	LLPointer<LLViewerImage> mTexturep;
public:
	LLDrawPoolTreeNew(LLViewerImage *texturep);

	/*virtual*/ LLDrawPool *instancePool();

	/*virtual*/ void prerender();
	/*virtual*/ void render( S32 pass );
	/*virtual*/ void renderForSelect();
	/*virtual*/ S32 rebuild();
	/*virtual*/ BOOL verify() const;
	/*virtual*/ LLViewerImage *getTexture();
	/*virtual*/ LLViewerImage *getDebugTexture();
	/*virtual*/ LLColor3 getDebugColor() const; // For AGP debug display
	virtual S32	getMaterialAttribIndex() { return 0; }

private:
	void renderTree(BOOL selecting = FALSE);
};

#endif // LL_LLDRAWPOOLTREE_H
