/** 
 * @file llhudconnector.h
 * @brief LLHUDConnector class definition
 *
 * Copyright (c) 2002-2007, Linden Research, Inc.
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

#ifndef LL_LLHUDCONNECTOR_H
#define LL_LLHUDCONNECTOR_H

#include "llmemory.h"
#include "lldarrayptr.h"

#include "llhudobject.h"
#include "v4color.h"
#include "v3math.h"
#include "v3dmath.h"
#include "llstring.h"

class LLViewerObject;

class LLHUDConnector : public LLHUDObject
{
public:
	void setColors(const LLColor4 &color, const LLColor4 &first_color, const LLColor4 &second_color);
	void setZCompare(const BOOL zcompare);
	void setDoFade(const BOOL do_fade);

	void setLabel(const LLString &label);
	void setTargets(LLViewerObject *first_object, LLViewerObject *second_object);
	void setEndpoints(const BOOL &first, const BOOL &second);

	friend class LLHUDObject;
protected:
	LLHUDConnector(const U8 type);

	/*virtual*/ void render();

private:
	~LLHUDConnector();

	LLString	mLabel;

	BOOL		mDoFade;
	F32			mFadeRange;
	F32			mFadeDistance;
	BOOL		mZCompare;

	LLColor4	mColor;
	LLColor4	mFirstColor;
	LLColor4	mSecondColor;

	BOOL		mDrawFirst;
	BOOL		mDrawSecond;
};

#endif // LL_LLHUDCONNECTOR_H
