/** 
 * @file listener_fmod.h
 * @brief Description of LISTENER class abstracting the audio support
 * as an FMOD 3D implementation (windows and Linux)
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

#ifndef LL_LISTENER_FMOD_H
#define LL_LISTENER_FMOD_H

#include "listener.h"

class LLListener_FMOD : public LLListener
{
 private:
 protected:
	 F32 mDopplerFactor;
	 F32 mDistanceFactor;
	 F32 mRolloffFactor;
 public:

 private:
 protected:
 public:  
	LLListener_FMOD();
	virtual ~LLListener_FMOD();
	virtual void init();  

	virtual void translate(LLVector3 offset);
	virtual void setPosition(LLVector3 pos);
	virtual void setVelocity(LLVector3 vel);
	virtual void orient(LLVector3 up, LLVector3 at);
	virtual void commitDeferredChanges();

	virtual void setDopplerFactor(F32 factor);
	virtual F32 getDopplerFactor();
	virtual void setDistanceFactor(F32 factor);
	virtual F32 getDistanceFactor();
	virtual void setRolloffFactor(F32 factor);
	virtual F32 getRolloffFactor();
};

#endif


