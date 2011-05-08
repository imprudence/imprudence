#ifndef __HIPPO_LIMITS_H__
#define __HIPPO_LIMITS_H__


class HippoLimits
{
public:
	HippoLimits();

	const S32&	getMaxAgentGroups() const { return mMaxAgentGroups; }
	const F32&	getMaxHeight()      const { return mMaxHeight;      }
	const F32&	getMinHoleSize()    const { return mMinHoleSize;    }
	const F32&	getMaxHollow()      const { return mMaxHollow;      }
	const S32&	getMaxLinkedPrims() const { return mMaxLinkedPrims; }
	const S32&	getMaxPhysLinkedPrims() const { return mMaxPhysLinkedPrims; }
	const F32&	getMaxInventoryItemsTransfer() const { return mMaxInventoryItemsTransfer; }
	const std::string& getVoiceConnector()  const { return mVoiceConnector; }

	// Returns the max prim size we can use on a grid
	F32		getMinPrimScale() const;
	F32		getMaxPrimScale() const;

	F32  getMaxDragDistance() const;

	F32		getMinPrimXPos() const;
	F32		getMinPrimYPos() const;
	F32		getMinPrimZPos() const;
	F32		getMaxPrimXPos() const;
	F32		getMaxPrimYPos() const;
	F32		getMaxPrimZPos() const;

	void	setLimits();

	S32		mMaxAgentGroups;

	F32		mMaxHeight;
	F32		mMinHoleSize;
	F32		mMaxHollow;
	F32		mMaxPrimScale;
	F32		mMinPrimScale;
	S32		mMaxLinkedPrims;
	S32		mMaxPhysLinkedPrims;
	F32		mMaxPrimXPos;
	F32		mMaxPrimYPos;
	F32		mMaxPrimZPos;
	F32		mMinPrimXPos;
	F32		mMinPrimYPos;
	F32		mMinPrimZPos;

	S32     mRenderName;

	F32		mMaxInventoryItemsTransfer;
	F32     mMaxDragDistance;

	BOOL    skyUseClassicClouds;
	BOOL    mAllowParcelWindLight;
	BOOL    mAllowMinimap;
	BOOL    mAllowPhysicalPrims;
	BOOL    mEnableTeenMode;
	BOOL    mEnforceMaxBuild;
	BOOL    mRenderWater;
	std::string    mVoiceConnector;

private:
	void	setOpenSimLimits();
	void	setSecondLifeLimits();
};


extern HippoLimits* gHippoLimits;


#endif
