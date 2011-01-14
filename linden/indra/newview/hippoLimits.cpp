

#include "llviewerprecompiledheaders.h"

#include "hippoLimits.h"

#include "hippoGridManager.h"
#include "llviewercontrol.h"

#include <llerror.h>


HippoLimits *gHippoLimits = 0;


HippoLimits::HippoLimits() 
:
	mMaxAgentGroups(100),
	mMaxHeight(10000.0f),
	mMinHoleSize(0.05f),
	mMaxHollow(0.95f),
	mMinPrimScale(0.001f),
	mMaxPrimScale(256.0f),
	mMaxLinkedPrims(-1)
{
	setLimits();
}


void HippoLimits::setLimits()
{
	if (gHippoGridManager->getConnectedGrid()->getPlatform() == HippoGridInfo::PLATFORM_SECONDLIFE) 
	{
		setSecondLifeLimits();
	} 
	else 
	{
		setOpenSimLimits();
	}
}


void HippoLimits::setOpenSimLimits()
{
	mMaxAgentGroups = gHippoGridManager->getConnectedGrid()->getMaxAgentGroups();
	if (mMaxAgentGroups < 0) mMaxAgentGroups = 50;
	mMaxPrimScale = 256.0f;
	mMinPrimScale = 0.001f;
	mMaxHeight = 10000.0f;
	mMaxLinkedPrims = -1;

	if (gHippoGridManager->getConnectedGrid()->isRenderCompat()) {
		llinfos << "Using rendering compatible OpenSim limits" << llendl;
		mMinHoleSize = 0.05f;
		mMaxHollow = 0.95f;
	} 
	else 
	{
		llinfos << "Using Hippo OpenSim limits" << llendl;
		mMinHoleSize = 0.01f;
		mMaxHollow = 0.99f;
	}
}

void HippoLimits::setSecondLifeLimits()
{
	llinfos << "Using Second Life limits" << llendl;
	S32 max_groups = gHippoGridManager->getConnectedGrid()->getMaxAgentGroups();
	mMaxAgentGroups = llmax(max_groups, 25);

	mMaxPrimScale = 10.0f;
	mMinPrimScale = 0.01f;
	mMaxHeight = 4096.0f;
	mMinHoleSize = 0.05f;  
	mMaxHollow = 0.95f;
	mMaxLinkedPrims = 255;
}

F32 HippoLimits::getMaxPrimScale() const
{
	if (gSavedSettings.getBOOL("DisableMaxBuildConstraints"))
	{
		return FLT_MAX;
	}
	else
	{
		return mMaxPrimScale;
	}
}
