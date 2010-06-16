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
	const F32&	getMinPrimScale()   const { return mMinPrimScale;   }
	const S32&	getMaxLinkedPrims() const { return mMaxLinkedPrims; }

	// Returns the max prim size we can use on a grid
	F32		getMaxPrimScale() const;

	void	setLimits();

private:
	S32		mMaxAgentGroups;

	F32		mMaxHeight;
	F32		mMinHoleSize;
	F32		mMaxHollow;
	F32		mMaxPrimScale;
	F32		mMinPrimScale;
	S32		mMaxLinkedPrims;

	void	setOpenSimLimits();
	void	setSecondLifeLimits();
};


extern HippoLimits* gHippoLimits;


#endif
