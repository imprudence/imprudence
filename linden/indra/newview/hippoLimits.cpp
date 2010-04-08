

#include "llviewerprecompiledheaders.h"

#include "hippoLimits.h"

#include "hippoGridManager.h"

#include <llerror.h>


HippoLimits *gHippoLimits = 0;


HippoLimits::HippoLimits()
{
	setLimits();
}


void HippoLimits::setLimits()
{
	if (gHippoGridManager->getConnectedGrid()->getPlatform() == HippoGridInfo::PLATFORM_SECONDLIFE) {
		setSecondLifeLimits();
	} else {
		setOpenSimLimits();
	}
}


void HippoLimits::setOpenSimLimits()
{
	mMaxAgentGroups = 100;
	mMaxPrimScale = 256.0f;
	mMaxHeight = 10000.0f;
	mMaxLinkedPrims = -1;

	if (gHippoGridManager->getConnectedGrid()->isRenderCompat()) {
		llinfos << "Using rendering compatible OpenSim limits." << llendl;
		mMinHoleSize = 0.05f;  
		mMaxHollow = 0.95f;
	} else {
		llinfos << "Using Hippo OpenSim limits." << llendl;
		mMinHoleSize = 0.01f;
		mMaxHollow = 0.99f;
	}
}

void HippoLimits::setSecondLifeLimits()
{
	llinfos << "Using Second Life limits." << llendl;
	mMaxAgentGroups = 25;
	mMaxPrimScale = 10.0f;
	mMaxHeight = 4096.0f;
	mMinHoleSize = 0.05f;  
	mMaxHollow = 0.95f;
	mMaxLinkedPrims = 255;
}

