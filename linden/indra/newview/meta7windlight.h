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
	LLUUID normalMapTexture;
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
