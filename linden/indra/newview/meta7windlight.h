/**
 * @file lightshare.cpp
 * @brief Handler for Meta7 Lightshare (region-side Windlight settings).
 *
 * Copyright (c) 2010, Tom Meta / Meta7 Project
 *
 * The source code in this file ("Source Code") is provided to you
 * under the terms of the GNU General Public License, version 2.0
 * ("GPL"). Terms of the GPL can be found in doc/GPL-license.txt in
 * this distribution, or online at
 * http://secondlifegrid.net/programs/open_source/licensing/gplv2
 *
 * There are special exceptions to the terms and conditions of the GPL as
 * it is applied to this Source Code. View the full text of the exception
 * in the file doc/FLOSS-exception.txt in this software distribution, or
 * online at
 * http://secondlifegrid.net/programs/open_source/licensing/flossexception
 *
 * By copying, modifying or distributing this software, you acknowledge
 * that you have read and understood your obligations described above,
 * and agree to abide by those obligations.
 *
 * ALL SOURCE CODE IS PROVIDED "AS IS." THE AUTHOR MAKES NO
 * WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
 * COMPLETENESS OR PERFORMANCE.
 */


#ifndef META7WINDLIGHT_H
#define META7WINDLIGHT_H

#include "linden_common.h"

struct M7Color3{
	M7Color3(){};
	M7Color3(F32 pRed, F32 pGreen, F32 pBlue)
	{
		red=pRed;
		green=pGreen;
		blue=pBlue;
	}
	F32	red;
	F32	green;
	F32	blue;
};

struct M7Vector3 {
	M7Vector3(){}
	M7Vector3(F32 pX, F32 pY, F32 pZ)
	{
		X=pX;
		Y=pY;
		Z=pZ;
	}
	F32	X;
	F32	Y;
	F32	Z;
	
};

struct M7Vector2{
	M7Vector2(){}
	M7Vector2(F32 pX, F32 pY)
	{
		X=pX;
		Y=pY;
	}
	F32	X;
	F32	Y;
	
};

struct M7Color4 {
	M7Color4(){}
	M7Color4(F32 pRed, F32 pGreen, F32 pBlue, F32 pAlpha)
	{
		red=pRed;
		green=pGreen;
		blue=pBlue;
		alpha=pAlpha;
	}
	F32	red;
	F32	green;
	F32	blue;
	F32 alpha;
};

struct Meta7WindlightPacket {
	Meta7WindlightPacket(){}
	M7Color3 waterColor;
	F32 waterFogDensityExponent;
	F32 underwaterFogModifier;
	M7Vector3 reflectionWaveletScale;
	F32 fresnelScale;
	F32 fresnelOffset;
	F32 refractScaleAbove;
	F32 refractScaleBelow;
	F32 blurMultiplier;
	M7Vector2 littleWaveDirection;
	M7Vector2 bigWaveDirection;
	unsigned char normalMapTexture[16];
	M7Color4 horizon;
	F32 hazeHorizon;
	M7Color4 blueDensity;
	F32 hazeDensity;
	F32 densityMultiplier;
	F32 distanceMultiplier;
	M7Color4 sunMoonColor;
	F32 sunMoonPosiiton;
	M7Color4 ambient;
	F32 eastAngle;
	F32 sunGlowFocus;
	F32 sunGlowSize;
	F32 sceneGamma;
	F32 starBrightness;
	M7Color4 cloudColor;
	M7Vector3 cloudXYDensity;
	F32 cloudCoverage;
	F32 cloudScale;
	M7Vector3 cloudDetailXYDensity;
	F32 cloudScrollX;
	F32 cloudScrollY;
	unsigned short maxAltitude;
	char cloudScrollXLock;
	char cloudScrollYLock;
	char drawClassicClouds;
	

};

#endif
