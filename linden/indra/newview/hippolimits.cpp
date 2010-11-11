

#include "llviewerprecompiledheaders.h"

#include "hippolimits.h"

#include "hippogridmanager.h"
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
	mMaxLinkedPrims(-1),
	mMaxDragDistance(0.f)
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

	if (mMaxAgentGroups < 0)
		mMaxAgentGroups = 50;

	mMaxPrimScale = 256.0f;
	mMinPrimScale = 0.001f;
	mMinPrimXPos = 0;
	mMinPrimYPos = 0;
	mMinPrimZPos = 0;
	mMaxPrimXPos = F32_MAX;
	mMaxPrimYPos = F32_MAX;
	mMaxPrimZPos = F32_MAX;
	mMaxHeight = 10000.0f;
	mMaxLinkedPrims = -1;
	mMaxPhysLinkedPrims = -1;
	mAllowParcelWindLight = TRUE;
	mAllowMinimap = TRUE;
	mMaxInventoryItemsTransfer = -1;
	mRenderName = 2;
	mAllowPhysicalPrims = TRUE;
	skyUseClassicClouds = TRUE;
	mEnableTeenMode = FALSE;
	mEnforceMaxBuild = FALSE;
	mRenderWater = TRUE;

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
	mMaxAgentGroups = 25;
	mMaxPrimScale = 10.0f;
	mMinPrimScale = 0.01f;
	mMaxHeight = 4096.0f;
	mMinHoleSize = 0.05f;  
	mMaxHollow = 0.95f;
	mMaxLinkedPrims = 255;
	mMaxPhysLinkedPrims = 32;
	mMinPrimXPos = 0;
	mMinPrimYPos = 0;
	mMinPrimZPos = 0;
	mMaxPrimXPos = 256;
	mMaxPrimYPos = 256;
	mMaxPrimZPos = 4096;
	mAllowParcelWindLight = FALSE;
	mAllowMinimap = TRUE;
	mMaxInventoryItemsTransfer = 42;
	mRenderName = 2;
	mAllowPhysicalPrims = TRUE;
	skyUseClassicClouds = TRUE;
	mEnableTeenMode = FALSE;
	mEnforceMaxBuild = FALSE;
	mRenderWater = TRUE;
}

F32 HippoLimits::getMaxPrimScale() const
{
	if (gSavedSettings.getBOOL("DisableMaxBuildConstraints") && !mEnforceMaxBuild)
	{
		return FLT_MAX;
	}
	else
	{
		return mMaxPrimScale;
	}
}

F32 HippoLimits::getMinPrimScale() const
{
	if (gSavedSettings.getBOOL("DisableMaxBuildConstraints") && !mEnforceMaxBuild)
	{
		return 0;
	}
	else
	{
		return mMinPrimScale;
	}
}

F32 HippoLimits::getMaxPrimXPos() const
{
	if (gSavedSettings.getBOOL("DisableMaxBuildConstraints") && !mEnforceMaxBuild)
	{
		return FLT_MAX;
	}
	else
	{
		return mMaxPrimXPos;
	}
}

F32 HippoLimits::getMaxPrimYPos() const
{
	if (gSavedSettings.getBOOL("DisableMaxBuildConstraints") && !mEnforceMaxBuild)
	{
		return FLT_MAX;
	}
	else
	{
		return mMaxPrimYPos;
	}
}

F32 HippoLimits::getMaxPrimZPos() const
{
	if (gSavedSettings.getBOOL("DisableMaxBuildConstraints") && !mEnforceMaxBuild)
	{
		return FLT_MAX;
	}
	else
	{
		return mMaxPrimZPos;
	}
}

F32 HippoLimits::getMinPrimXPos() const
{
	if (gSavedSettings.getBOOL("DisableMaxBuildConstraints") && !mEnforceMaxBuild)
	{
		return FLT_MIN;
	}
	else
	{
		return mMinPrimXPos;
	}
}

F32 HippoLimits::getMinPrimYPos() const
{
	if (gSavedSettings.getBOOL("DisableMaxBuildConstraints") && !mEnforceMaxBuild)
	{
		return FLT_MIN;
	}
	else
	{
		return mMinPrimYPos;
	}
}

F32 HippoLimits::getMinPrimZPos() const
{
	if (gSavedSettings.getBOOL("DisableMaxBuildConstraints") && !mEnforceMaxBuild)
	{
		return FLT_MIN;
	}
	else
	{
		return mMinPrimZPos;
	}
}

F32 HippoLimits::getMaxDragDistance() const
{
	if (mMaxDragDistance == 0.f)
	{
		return FLT_MAX;
	}
	else
	{
		F32 max_drag_distance = gSavedSettings.getBOOL("LimitDragDistance") ? gSavedSettings.getF32("MaxDragDistance") : FLT_MAX;

		if(max_drag_distance > mMaxDragDistance) //Chose the more restrictive
		{
			max_drag_distance = mMaxDragDistance;
		}
		return max_drag_distance;
	}
}