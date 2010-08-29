#ifndef __emeraldboobutils_h
#define __emeraldboobutils_h

#include <iostream>
#include <list>

#include "stdtypes.h"
#include "v3math.h"
#include "llquaternion.h"

struct EmeraldGlobalBoobConfig
{
	bool enabled;
	F32 mass;
	F32 hardness;
	F32 zMax;
	F32 velMin;
	F32 velMax;
	F32 zInfluence;
	F32 friction;
	F32 XYInfluence;

	EmeraldGlobalBoobConfig()
		: enabled(false),
			mass(6.4f),
			hardness(0.67f),
			zMax(1.29f),
			velMin(0.0027f*0.017f),
			velMax(0.0027f),
			zInfluence(0.0f),
			friction(0.35f),
			XYInfluence(0.3f)
	{
	}

	bool operator==(const EmeraldGlobalBoobConfig &other) const
	{
		return
			enabled == other.enabled &&
			mass == other.mass &&
			zMax == other.zMax &&
			velMax == other.velMax &&
			velMin == other.velMin &&
			zInfluence == other.zInfluence &&
			XYInfluence == other.XYInfluence &&
			friction == other.friction;
	}
};

std::ostream &operator<<(std::ostream &os, const EmeraldGlobalBoobConfig &v);

struct EmeraldAvatarLocalBoobConfig
{
	F32 actualBoobGrav;
	F32 actualButtGrav;
	F32 actualFatGrav;
	F32 boobSize;

	EmeraldAvatarLocalBoobConfig()
		: actualBoobGrav(0.0f),
		  actualButtGrav(0.0f),
		  actualFatGrav(0.0f),
			boobSize(0.0f)
	{
	}

	bool operator==(const EmeraldAvatarLocalBoobConfig &other) const
	{
		return
			actualBoobGrav == other.actualBoobGrav &&
			actualButtGrav == other.actualButtGrav &&
			actualFatGrav == other.actualFatGrav &&
			boobSize == other.boobSize;
	}

};

std::ostream &operator<<(std::ostream &os, const EmeraldAvatarLocalBoobConfig &v);

struct EmeraldBoobBounceState;

struct EmeraldBoobState
{
	F32 boobGrav;
	LLVector3 chestPosition;
	LLQuaternion chestRotation;
	F32 elapsedTime;
	F32 frameDuration;
	LLVector3 chestDisplacement;
	LLVector3 localChestDisplacement;
	LLVector3 displacementForce;
	F32 mysteryValue;
	std::list<EmeraldBoobBounceState> bounceStates;

	EmeraldBoobState()
		: boobGrav(0.0f),
			chestPosition(0.0f,0.0f,0.0f),
			chestRotation(0.0f,0.0f,0.0f,1.0f),
			elapsedTime(0.0f),
			frameDuration(0.0f),
			chestDisplacement(0.0f,0.0f,0.0f),
			localChestDisplacement(0.0f,0.0f,0.0f),
			displacementForce(0.0f,0.0f,0.0f),
			mysteryValue(0.0f)
	{
	}

	bool operator==(const EmeraldBoobState &other) const
	{
		return
			boobGrav == other.boobGrav &&
			chestPosition == other.chestPosition &&
			chestRotation == other.chestRotation &&
			elapsedTime == other.elapsedTime &&
			frameDuration == other.frameDuration &&
			chestDisplacement == other.chestDisplacement &&
			localChestDisplacement == other.localChestDisplacement &&
			displacementForce == other.displacementForce &&
			mysteryValue == other.mysteryValue &&
			bounceStates == other.bounceStates;
	}
};

std::ostream &operator<<(std::ostream &os, const EmeraldBoobState &v);

struct EmeraldBoobInputs
{
	LLVector3 chestPosition;
	LLQuaternion chestRotation;
	F32 elapsedTime;
	bool appearanceFlag;
	bool appearanceAnimating;
	S32 type;

	EmeraldBoobInputs()
		: chestPosition(0.0f,0.0f,0.0f),
			chestRotation(0.0f,0.0f,0.0f,1.0f),
			elapsedTime(0.0f),
			appearanceFlag(false),
			appearanceAnimating(false),
			type(0)
	{
	}

	bool operator==(const EmeraldBoobInputs &other) const
	{
		return
			chestPosition == other.chestPosition &&
			chestRotation == other.chestRotation &&
			elapsedTime == other.elapsedTime &&
			appearanceFlag == other.appearanceFlag &&
			appearanceAnimating == other.appearanceAnimating &&
			type == other.type;
	}
};

std::ostream &operator<<(std::ostream &os, const EmeraldBoobInputs &v);

struct EmeraldBoobBounceState
{
	F32 bounceStart;
	F32 bounceStartAmplitude;
	F32 bounceStartFrameDuration;

	EmeraldBoobBounceState()
		: bounceStart(0.0f),
			bounceStartAmplitude(0.0f),
			bounceStartFrameDuration(0.0f)
	{
	};

	bool operator==(const EmeraldBoobBounceState &other) const
	{
		return
			bounceStart == other.bounceStart &&
			bounceStartAmplitude == other.bounceStartAmplitude &&
			bounceStartFrameDuration == other.bounceStartFrameDuration;
	}
};

std::ostream &operator<<(std::ostream &os, const EmeraldBoobBounceState &v);


struct EmeraldBoobUtils
{
public:
	static EmeraldBoobState idleUpdate(const EmeraldGlobalBoobConfig &config, const EmeraldAvatarLocalBoobConfig &localConfig, const EmeraldBoobState &oldState, const EmeraldBoobInputs &inputs);

	static F32 convertMass(F32 displayMass);
	static F32 convertHardness(F32 displayHardness);
	static F32 convertVelMax(F32 displayVelMax);
	static F32 convertFriction(F32 displayFriction);
	static F32 convertVelMin(F32 displayVelMin);
};


#endif
