#ifndef __HIPPO_LIMITS_H__
#define __HIPPO_LIMITS_H__


class HippoLimits
{
public:
	HippoLimits();

	int   getMaxAgentGroups() const { return mMaxAgentGroups; }
	float getMaxHeight()      const { return mMaxHeight;      }
	float getMinHoleSize()    const { return mMinHoleSize;    }
	float getMaxHollow()      const { return mMaxHollow;      }
	float getMaxPrimScale()   const { return mMaxPrimScale;   }
	S32   getMaxLinkedPrims() const { return mMaxLinkedPrims; }

	void setLimits();

private:
	int   mMaxAgentGroups;

	float mMaxHeight;
	float mMinHoleSize;
	float mMaxHollow;
	float mMaxPrimScale;
	S32   mMaxLinkedPrims;

	void setOpenSimLimits();
	void setSecondLifeLimits();
};


extern HippoLimits *gHippoLimits;


#endif
