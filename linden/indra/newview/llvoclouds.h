/** 
 * @file llvoclouds.h
 * @brief Description of LLVOClouds class
 *
 * Copyright (c) 2001-2007, Linden Research, Inc.
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

#ifndef LL_LLVOCLOUDS_H
#define LL_LLVOCLOUDS_H

#include "llviewerobject.h"
#include "lltable.h"
#include "v4coloru.h"

class LLViewerImage;
class LLViewerCloudGroup;

class LLCloudGroup;


class LLVOClouds : public LLViewerObject
{
public:
	LLVOClouds(const LLUUID &id, const LLPCode pcode, LLViewerRegion *regionp );
	virtual ~LLVOClouds();

	// Initialize data that's only inited once per class.
	static void initClass();

	/*virtual*/ LLDrawable* createDrawable(LLPipeline *pipeline);
	/*virtual*/ BOOL        updateGeometry(LLDrawable *drawable);

	/*virtual*/ BOOL    isActive() const; // Whether this object needs to do an idleUpdate.

	/*virtual*/ void updateTextures(LLAgent &agent);
	/*virtual*/ void setPixelAreaAndAngle(LLAgent &agent); // generate accurate apparent angle and area

	BOOL idleUpdate(LLAgent &agent, LLWorld &world, const F64 &time);

	void setCloudGroup(LLCloudGroup *cgp)		{ mCloudGroupp = cgp; }
protected:
	LLCloudGroup *mCloudGroupp;
};

extern LLUUID gCloudTextureID;

#endif // LL_VO_CLOUDS_H
