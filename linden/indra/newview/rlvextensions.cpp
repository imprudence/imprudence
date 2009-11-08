#include "llviewerprecompiledheaders.h"
#include "llagent.h"
#include "llfloaterwindlight.h"
#include "llviewercontrol.h"
#include "llviewerwindow.h"
#include "llvoavatar.h"
#include "llwlparammanager.h"

#include "rlvextensions.h"
#include "rlvhandler.h"

// ============================================================================

std::map<std::string, S16> RlvExtGetSet::m_DbgAllowed;
std::map<std::string, std::string> RlvExtGetSet::m_PseudoDebug;

// Checked: 2009-06-03 (RLVa-0.2.0h) | Modified: RLVa-0.2.0h
RlvExtGetSet::RlvExtGetSet()
{
	if (!m_DbgAllowed.size())	// m_DbgAllowed is static and should only be initialized once
	{
		m_DbgAllowed.insert(std::pair<std::string, S16>("AvatarSex", DBG_READ | DBG_WRITE | DBG_PSEUDO));
		m_DbgAllowed.insert(std::pair<std::string, S16>("RenderResolutionDivisor", DBG_READ | DBG_WRITE));
		#ifdef RLV_EXTENSION_CMD_GETSETDEBUG_EX
			m_DbgAllowed.insert(std::pair<std::string, S16>(RLV_SETTING_FORBIDGIVETORLV, DBG_READ));
			m_DbgAllowed.insert(std::pair<std::string, S16>(RLV_SETTING_NOSETENV, DBG_READ));
			m_DbgAllowed.insert(std::pair<std::string, S16>("WindLightUseAtmosShaders", DBG_READ));
		#endif // RLV_EXTENSION_CMD_GETSETDEBUG_EX

		// Cache persistance of every setting
		LLControlVariable* pSetting;
		for (std::map<std::string, S16>::iterator itDbg = m_DbgAllowed.begin(); itDbg != m_DbgAllowed.end(); ++itDbg)
		{
			if ( ((pSetting = gSavedSettings.getControl(itDbg->first)) != NULL) && (pSetting->isPersisted()) )
				itDbg->second |= DBG_PERSIST;
		}
	}
}

// Checked: 2009-05-17 (RLVa-0.2.0a)
BOOL RlvExtGetSet::onForceCommand(const RlvEvent& rlvEvent)
{
	return processCommand(rlvEvent.getSenderID(), rlvEvent.getCommand());
}

// Checked: 2009-05-17 (RLVa-0.2.0a)
BOOL RlvExtGetSet::onReplyCommand(const EventType& rlvEvent)
{
	return processCommand(rlvEvent.getSenderID(), rlvEvent.getCommand());
}

// Checked: 2009-06-18 (RLVa-0.2.1d) | Modified: RLVa-0.2.1d
BOOL RlvExtGetSet::processCommand(const LLUUID& idObj, const RlvCommand& rlvCmd)
{
	std::string strBehaviour = rlvCmd.getBehaviour(), strGetSet, strSetting;
	int idxSetting = strBehaviour.find('_');
	if ( (strBehaviour.length() >= 6) && (-1 != idxSetting) && ((int)strBehaviour.length() > idxSetting + 1) )
	{
		strSetting = strBehaviour.substr(idxSetting + 1);
		strBehaviour.erase(idxSetting);	// Get rid of "_<setting>"

		strGetSet = strBehaviour.substr(0, 3);
		strBehaviour.erase(0, 3);		// Get rid of get/set

		if ("debug" == strBehaviour)
		{
			if ( ("get" == strGetSet) && (RLV_TYPE_REPLY == rlvCmd.getParamType()) )
			{
				rlvSendChatReply(rlvCmd.getParam(), onGetDebug(strSetting));
				return TRUE;
			}
			else if ( ("set" == strGetSet) && (RLV_TYPE_FORCE == rlvCmd.getParamType()) )
			{
				if (!gRlvHandler.hasBehaviourExcept(RLV_BHVR_SETDEBUG, idObj))
					onSetDebug(strSetting, rlvCmd.getOption());
				return TRUE;
			}
		}
		else if ("env" == strBehaviour)
		{
			if ( ("get" == strGetSet) && (RLV_TYPE_REPLY == rlvCmd.getParamType()) )
			{
				rlvSendChatReply(rlvCmd.getParam(), onGetEnv(strSetting));
				return TRUE;
			}
			else if ( ("set" == strGetSet) && (RLV_TYPE_FORCE == rlvCmd.getParamType()) )
			{
				if (!gRlvHandler.hasBehaviourExcept(RLV_BHVR_SETENV, idObj))
					onSetEnv(strSetting, rlvCmd.getOption());
				return TRUE;
			}
		}
	}
	else if ("setrot" == rlvCmd.getBehaviour())
	{
		// NOTE: if <option> is invalid (or missing) altogether then RLV-1.17 will rotate to 0.0 (which is actually PI / 4)
		F32 nAngle = 0.0f;
		if (LLStringUtil::convertToF32(rlvCmd.getOption(), nAngle))
		{
			nAngle = RLV_SETROT_OFFSET - nAngle;

			gAgent.startCameraAnimation();

			LLVector3 at(LLVector3::x_axis);
			at.rotVec(nAngle, LLVector3::z_axis);
			at.normalize();
			gAgent.resetAxes(at);

			return TRUE;
		}
	}
	return FALSE;
}

// Checked: 2009-06-03 (RLVa-0.2.0h) | Modified: RLVa-0.2.0h
bool RlvExtGetSet::findDebugSetting(std::string& strSetting, S16& flags)
{
	LLStringUtil::toLower(strSetting);	// Convenience for non-RLV calls

	std::string strTemp;
	for (std::map<std::string, S16>::const_iterator itSetting = m_DbgAllowed.begin(); itSetting != m_DbgAllowed.end(); ++itSetting)
	{
		strTemp = itSetting->first;
		LLStringUtil::toLower(strTemp);
		
		if (strSetting == strTemp)
		{
			strSetting = itSetting->first;
			flags = itSetting->second;
			return true;
		}
	}
	return false;
}

// Checked: 2009-06-03 (RLVa-0.2.0h) | Added: RLVa-0.2.0h
S16 RlvExtGetSet::getDebugSettingFlags(const std::string& strSetting)
{
	std::map<std::string, S16>::const_iterator itSetting = m_DbgAllowed.find(strSetting);
	return (itSetting != m_DbgAllowed.end()) ? itSetting->second : 0;
}

// Checked: 2009-06-03 (RLVa-0.2.0h) | Modified: RLVa-0.2.0h
std::string RlvExtGetSet::onGetDebug(std::string strSetting)
{
	S16 dbgFlags;
	if ( (findDebugSetting(strSetting, dbgFlags)) && ((dbgFlags & DBG_READ) == DBG_READ) )
	{
		if ((dbgFlags & DBG_PSEUDO) == 0)
		{
			LLControlVariable* pSetting = gSavedSettings.getControl(strSetting);
			if (pSetting)
			{
				switch (pSetting->type())
				{
					case TYPE_U32:
						return llformat("%u", gSavedSettings.getU32(strSetting));
					case TYPE_S32:
						return llformat("%d", gSavedSettings.getS32(strSetting));
					case TYPE_BOOLEAN:
						return llformat("%d", gSavedSettings.getBOOL(strSetting));
					default:
						RLV_ERRS << "Unexpected debug setting type" << LL_ENDL;
						break;
				}
			}
		}
		else
		{
			return onGetPseudoDebug(strSetting);
		}
	}
	return std::string();
}

// Checked: 2009-10-03 (RLVa-1.0.4e) | Added: RLVa-1.0.4e
std::string RlvExtGetSet::onGetPseudoDebug(const std::string& strSetting)
{
	// Skip sanity checking because it's all done in RlvExtGetSet::onGetDebug() already
	if ("AvatarSex" == strSetting)
	{
		std::map<std::string, std::string>::const_iterator itPseudo = m_PseudoDebug.find(strSetting);
		if (itPseudo != m_PseudoDebug.end())
		{
			return itPseudo->second;
		}
		else
		{
			if (gAgent.getAvatarObject())
				return llformat("%d", (gAgent.getAvatarObject()->getSex() == SEX_MALE)); // [See LLFloaterCustomize::LLFloaterCustomize()]
		}
	}
	return std::string();
}

// Checked: 2009-10-10 (RLVa-1.0.4e) | Modified: RLVa-1.0.4e
void RlvExtGetSet::onSetDebug(std::string strSetting, const std::string& strValue)
{
	S16 dbgFlags;
	if ( (findDebugSetting(strSetting, dbgFlags)) && ((dbgFlags & DBG_WRITE) == DBG_WRITE) )
	{
		if ((dbgFlags & DBG_PSEUDO) == 0)
		{
			LLControlVariable* pSetting = gSavedSettings.getControl(strSetting);
			if (pSetting)
			{
				U32 u32Value; S32 s32Value; BOOL fValue;
				switch (pSetting->type())
				{
					case TYPE_U32:
						if (LLStringUtil::convertToU32(strValue, u32Value))
							gSavedSettings.setU32(strSetting, u32Value);
						break;
					case TYPE_S32:
						if (LLStringUtil::convertToS32(strValue, s32Value))
							gSavedSettings.setS32(strSetting, s32Value);
						break;
					case TYPE_BOOLEAN:
						if (LLStringUtil::convertToBOOL(strValue, fValue))
							gSavedSettings.setBOOL(strSetting, fValue);
						break;
					default:
						RLV_ERRS << "Unexpected debug setting type" << LL_ENDL;
						break;
				}

				// Default settings should persist if they were marked that way, but non-default settings should never persist
				pSetting->setPersist( (pSetting->isDefault()) ? ((dbgFlags & DBG_PERSIST) == DBG_PERSIST) : false );
			}
		}
		else
		{
			onSetPseudoDebug(strSetting, strValue);
		}
	}
}

// Checked: 2009-10-10 (RLVa-1.0.4e) | Modified: RLVa-1.0.4e
void RlvExtGetSet::onSetPseudoDebug(const std::string& strSetting, const std::string& strValue)
{
	if ("AvatarSex" == strSetting)
	{
		BOOL fValue;
		if (LLStringUtil::convertToBOOL(strValue, fValue))
			m_PseudoDebug[strSetting] = strValue;
	}
}

// Checked: 2009-09-16 (RLVa-1.0.3c) | Modified: RLVa-1.0.3c
std::string RlvExtGetSet::onGetEnv(std::string strSetting)
{
	// HACK: - create a LLFloaterWindLight instance if there isn't one already
	//       - isOpen() is actually instanceExists()
	//       - creating an instance results in showing the floater which is why we need to ->close() it
	if (!LLFloaterWindLight::isOpen())
	{
		LLFloaterWindLight::instance()->close();
		LLFloaterWindLight::instance()->syncMenu();
	}

	LLWLParamManager* pWLParams = LLWLParamManager::instance();

	F32 nValue = 0.0f;
	if ("daytime" == strSetting)
	{
		nValue = (pWLParams->mAnimator.mIsRunning && pWLParams->mAnimator.mUseLindenTime) ? -1.0f : pWLParams->mAnimator.getDayTime();
	}
	else if ("preset" == strSetting)
	{
		return (pWLParams->mAnimator.mIsRunning && pWLParams->mAnimator.mUseLindenTime) ? std::string() : pWLParams->mCurParams.mName;
	}
	else if ("cloudcoverage" == strSetting)			nValue = pWLParams->mCloudCoverage;
	else if ("cloudscale" == strSetting)			nValue = pWLParams->mCloudScale;
	else if ("cloudscrollx" == strSetting)			nValue = pWLParams->mCurParams.getCloudScrollX() - 10.0f;
	else if ("cloudscrolly" == strSetting)			nValue = pWLParams->mCurParams.getCloudScrollY() - 10.0f;
	else if ("densitymultiplier" == strSetting)		nValue = pWLParams->mDensityMult.x * pWLParams->mDensityMult.mult;
	else if ("distancemultiplier" == strSetting)	nValue = pWLParams->mDistanceMult;
	else if ("eastangle" == strSetting)				nValue = pWLParams->mCurParams.getEastAngle() / F_TWO_PI;
	else if ("hazedensity" == strSetting)			nValue = pWLParams->mHazeDensity.r;
	else if ("hazehorizon" == strSetting)			nValue = pWLParams->mHazeHorizon.r;
	else if ("maxaltitude" == strSetting)			nValue = pWLParams->mMaxAlt;
	else if ("scenegamma" == strSetting)			nValue = pWLParams->mWLGamma;
	else if ("starbrightness" == strSetting)		nValue = pWLParams->mCurParams.getStarBrightness();
	else if ("sunglowfocus" == strSetting)			nValue = -pWLParams->mGlow.b / 5.0f;
	else if ("sunglowsize" == strSetting)			nValue = 2 - pWLParams->mGlow.r / 20.0f;
	else if ("sunmoonposition" == strSetting)		nValue = pWLParams->mCurParams.getSunAngle() / F_TWO_PI;
	else
	{
		char ch = strSetting[strSetting.length() - 1];
		// HACK-RLVa: not entirely proper (creates new synonyms)
		if ('x' == ch)		ch = 'r';
		else if ('y' == ch)	ch = 'g';
		else if ('d' == ch)	ch = 'b';

		if ( ('r' == ch) || ('g' == ch) || ('b' == ch) || ('i' == ch) )
		{
			WLColorControl* pColour = NULL;
			strSetting.erase(strSetting.length() - 1, 1);
			
			if ("ambient" == strSetting)			pColour = &pWLParams->mAmbient;
			else if ("bluedensity" == strSetting)	pColour = &pWLParams->mBlueDensity;
			else if ("bluehorizon" == strSetting)	pColour = &pWLParams->mBlueHorizon;
			else if ("sunmooncolor" == strSetting)	pColour = &pWLParams->mSunlight;
			else if ("cloudcolor" == strSetting)	pColour = &pWLParams->mCloudColor;
			else if ("cloud" == strSetting)			pColour = &pWLParams->mCloudMain;
			else if ("clouddetail" == strSetting)	pColour = &pWLParams->mCloudDetail;

			if (pColour)
			{
				if ('r' == ch)		nValue = pColour->r;
				else if ('g' == ch)	nValue = pColour->g;
				else if ('b' == ch)	nValue = pColour->b;
				else if (('i' == ch) && (pColour->hasSliderName)) nValue = llmax(pColour->r, pColour->g, pColour->b);

				if (pColour->isBlueHorizonOrDensity)	nValue /= 2.0f;
				else if (pColour->isSunOrAmbientColor)	nValue /= 3.0f;
			}
		}
	}

	return llformat("%f", nValue);
}

// Checked: 2009-09-16 (RLVa-1.0.3c) | Modified: RLVa-1.0.3c
void RlvExtGetSet::onSetEnv(std::string strSetting, const std::string& strValue)
{
	// HACK: see RlvExtGetSet::onGetEnv
	if (!LLFloaterWindLight::isOpen())
	{
		LLFloaterWindLight::instance()->close();
		LLFloaterWindLight::instance()->syncMenu();
	}

	LLWLParamManager* pWLParams = LLWLParamManager::instance();
	WLFloatControl* pFloat = NULL;
	WLColorControl* pColour = NULL;

	F32 nValue = 0.0f;
	// Sanity check - make sure strValue specifies a number for all settings except "preset"
	if ( (rlv_handler_t::fNoSetEnv) || ( (!LLStringUtil::convertToF32(strValue, nValue)) && ("preset" != strSetting) ))
		return;

	// Not quite correct, but RLV-1.16.0 will halt the default daytime cycle on invalid commands so we need to as well
	pWLParams->mAnimator.mIsRunning = false;
	pWLParams->mAnimator.mUseLindenTime = false;

	// See LLWorldEnvSettings::handleEvent()
	if ("daytime" == strSetting)
	{
		if (0.0f <= nValue)
		{
			pWLParams->mAnimator.setDayTime(llmin(nValue, 1.0f));
			pWLParams->mAnimator.update(pWLParams->mCurParams);
		}
		else
		{
			pWLParams->mAnimator.mIsRunning = true;
			pWLParams->mAnimator.mUseLindenTime = true;	
		}
		return;
	}
	// See LLFloaterWindLight::onChangePresetName()
	else if ("preset" == strSetting)
	{
		pWLParams->loadPreset(strValue, true);
		return;
	}
	// See LLFloaterWindLight::onStarAlphaMoved
	else if ("starbrightness" == strSetting)
	{
		pWLParams->mCurParams.setStarBrightness(nValue);
		return;
	}
	// See LLFloaterWindLight::onGlowRMoved() / LLFloaterWindLight::onGlowBMoved()
	else if ( ("sunglowfocus" == strSetting) || ("sunglowsize" == strSetting) )
	{
		WLColorControl *pColour = &pWLParams->mGlow;
		if ("sunglowfocus" == strSetting) 
			pColour->b = -nValue * 5;
		else
			pColour->r = (2 - nValue) * 20;

		pColour->update(pWLParams->mCurParams);
		pWLParams->propagateParameters();
		return;
	}
	// See LLFloaterWindLight::onSunMoved()
	else if ( ("eastangle" == strSetting) || ("sunmoonposition" == strSetting) )	
	{
		if ("eastangle" == strSetting)	
			pWLParams->mCurParams.setEastAngle(F_TWO_PI * nValue);
		else
			pWLParams->mCurParams.setSunAngle(F_TWO_PI * nValue);

		// TODO-RLVa: it looks like propagateParameters() will actually take care of this for us, making this redundant?
		WLColorControl* pColour = &pWLParams->mLightnorm;
		pColour->r = -sin(pWLParams->mCurParams.getEastAngle()) * cos(pWLParams->mCurParams.getSunAngle());
		pColour->g = sin(pWLParams->mCurParams.getSunAngle());
		pColour->b = cos(pWLParams->mCurParams.getEastAngle()) * cos(pWLParams->mCurParams.getSunAngle());
		pColour->i = 1.f;

		pColour->update(pWLParams->mCurParams);
		pWLParams->propagateParameters();
		return;
	}
	// See LLFloaterWindLight::onCloudScrollXMoved() / LLFloaterWindLight::onCloudScrollYMoved() 
	else if ("cloudscrollx" == strSetting)
	{
		pWLParams->mCurParams.setCloudScrollX(nValue + 10.0f);
		return;
	}
	else if ("cloudscrolly" == strSetting)
	{
		pWLParams->mCurParams.setCloudScrollY(nValue + 10.0f);
		return;
	}
	// See LLFloaterWindLight::onFloatControlMoved()
	else if ("cloudcoverage" == strSetting)			pFloat = &pWLParams->mCloudCoverage;
	else if ("cloudscale" == strSetting)			pFloat = &pWLParams->mCloudScale;
	else if ("densitymultiplier" == strSetting)		pFloat = &pWLParams->mDensityMult;
	else if ("distancemultiplier" == strSetting)	pFloat = &pWLParams->mDistanceMult;
	else if ("maxaltitude" == strSetting)			pFloat = &pWLParams->mMaxAlt;
	else if ("scenegamma" == strSetting)			pFloat = &pWLParams->mWLGamma;
	// See LLFloaterWindLight::onColorControlRMoved()
	else if ("hazedensity" == strSetting)	pColour = &pWLParams->mHazeDensity;
	else if ("hazehorizon" == strSetting)	pColour = &pWLParams->mHazeHorizon;

	if (pFloat)
	{
		pFloat->x = nValue / pFloat->mult;
		pFloat->update(pWLParams->mCurParams);
		pWLParams->propagateParameters();
		return;
	} 
	else if (pColour)
	{
		pColour->r = nValue;
		pColour->update(pWLParams->mCurParams);
		pWLParams->propagateParameters();
		return;
	}

	// RGBI settings
	char ch = strSetting[strSetting.length() - 1];
	if ('x' == ch)		ch = 'r';
	else if ('y' == ch)	ch = 'g';
	else if ('d' == ch)	ch = 'b';

	if ( ('r' == ch) || ('g' == ch) || ('b' == ch) || ('i' == ch) )
	{
		strSetting.erase(strSetting.length() - 1, 1);
		
		if ("ambient" == strSetting)			pColour = &pWLParams->mAmbient;
		else if ("bluedensity" == strSetting)	pColour = &pWLParams->mBlueDensity;
		else if ("bluehorizon" == strSetting)	pColour = &pWLParams->mBlueHorizon;
		else if ("sunmooncolor" == strSetting)	pColour = &pWLParams->mSunlight;
		else if ("cloudcolor" == strSetting)	pColour = &pWLParams->mCloudColor;
		else if ("cloud" == strSetting)			pColour = &pWLParams->mCloudMain;
		else if ("clouddetail" == strSetting)	pColour = &pWLParams->mCloudDetail;

		if (pColour)
		{
			if (pColour->isBlueHorizonOrDensity)   nValue *= 2.0f;
			else if (pColour->isSunOrAmbientColor) nValue *= 3.0f;

			if ('i' == ch)									// (See: LLFloaterWindLight::onColorControlIMoved)
			{
				if (!pColour->hasSliderName)
					return;

				F32 curMax = llmax(pColour->r, pColour->g, pColour->b);
				if ( (0.0f == nValue) || (0.0f == curMax) )
					pColour->r = pColour->g = pColour->b = pColour->i = nValue;
				else
				{
					F32 nDelta = (nValue - curMax) / curMax;
					pColour->r *= (1.0f + nDelta);
					pColour->g *= (1.0f + nDelta);
					pColour->b *= (1.0f + nDelta);
					pColour->i = nValue;
				}
			}
			else											// (See: LLFloaterWindLight::onColorControlRMoved)
			{
				F32* pnValue = ('r' == ch) ? &pColour->r : ('g' == ch) ? &pColour->g : ('b' == ch) ? &pColour->b : NULL;
				if (pnValue)
					*pnValue = nValue;
				pColour->i = llmax(pColour->r, pColour->g, pColour->b);
			}

			pColour->update(pWLParams->mCurParams);
			pWLParams->propagateParameters();
		}
	}
}

// ============================================================================
