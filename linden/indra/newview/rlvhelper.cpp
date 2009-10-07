#include "llviewerprecompiledheaders.h"
#include "llagent.h"
#include "llviewerobject.h"
#include "llviewerstats.h"
#include "llviewerwindow.h"
#include "llvoavatar.h"
#include "llwlparammanager.h"

#include "rlvhelper.h"
#include "rlvevent.h"
#include "rlvhandler.h"

// ============================================================================
// Static variable initialization
//

RlvMultiStringSearch RlvCommand::m_BhvrLookup;

// ============================================================================

// Checked:
RlvCommand::RlvCommand(const std::string& strCommand)
	: m_eBehaviour(RLV_BHVR_UNKNOWN), m_eParamType(RLV_TYPE_UNKNOWN)
{
	if ((m_fValid = parseCommand(strCommand, m_strBehaviour, m_strOption, m_strParam)))
	{
		if ( ("n" == m_strParam) || ("add" == m_strParam) )
			m_eParamType = RLV_TYPE_ADD;
		else if ( ("y" == m_strParam) || ("rem" == m_strParam) )
			m_eParamType = RLV_TYPE_REMOVE;
		else if ("force" == m_strParam)
			m_eParamType = RLV_TYPE_FORCE;
		else
		{
			m_eParamType = RLV_TYPE_REPLY; // Assume it's a reply command until we encounter a non-digit

			if ( (m_strParam.empty()) || (-1 != m_strParam.find_first_not_of("0123456789")) )
			{
				m_eParamType = RLV_TYPE_UNKNOWN;
				m_fValid = ("clear" == m_strBehaviour);
			}
		}
	}

	if (!m_fValid)
	{
		m_strBehaviour = m_strOption = m_strParam = "";
		return;
	}

	U16 nBehaviour;
	if (m_BhvrLookup.getExactMatchParam(m_strBehaviour, nBehaviour))
	{
		m_eBehaviour = (ERlvBehaviour)nBehaviour;
	}
}

RlvCommand::RlvCommand(const RlvCommand& rlvCmd)
	: m_fValid(rlvCmd.m_fValid),
	  m_strBehaviour(rlvCmd.m_strBehaviour), m_eBehaviour(rlvCmd.m_eBehaviour),	
	  m_strOption(rlvCmd.m_strOption),
	  m_strParam(rlvCmd.m_strParam), m_eParamType(rlvCmd.m_eParamType)
{
}

// ============================================================================

/*
 * ------------------------------
 * Command           | RLV | RLVa
 * ------------------------------
 * :                 |  F  |  F  (missing behaviour)
 * :option           |  F  |  F  (missing behaviour)
 * :=                |  T  |  F  (missing behaviour)
 * :option=          |  T  |  F  (missing behaviour)
 * :option=param     |  T  |  F  (missing behaviour)
 * =                 |  T  |  F  (missing behaviour)
 * =param            |  T  |  F  (missing behaviour)
 * cmd               |  F  |  F  (missing param) [T if <behaviour> == "clear"]
 * cmd:              |  F  |  F  (missing param)
 * cmd:option        |  F  |  F  (missing param)
 * cmd:=             |  T  |  F  (missing param) [1]
 * cmd:option=       |  T  |  F  (missing param) [1]
 * cmd=              |  T  |  F  (missing param) [1]
 * cmd:option=param  |  T  |  T
 * cmd=param         |  T  |  T
 * cmd:=param        |  T  |  T
 *
 * [1] 'clear:=', 'clear:option=' and 'clear=' are "valid" variations of 'clear'
 */

BOOL RlvCommand::parseCommand(/*[in]*/ const std::string& strCommand,  
	/*[out]*/ std::string& strBehaviour, /*[out]*/ std::string& strOption, /*[out]*/ std::string& strParam)
{
	// Format: <behaviour>[:<option>]=<param>
	int idxParam  = strCommand.find('=');
	int idxOption = (idxParam > 0) ? strCommand.find(':') : -1;
	if (idxOption > idxParam - 1)
		idxOption = -1;

	// If <behaviour> is missing it's always an improperly formatted command
	if ( (0 == idxOption) || (0 == idxParam) )
		return FALSE;

	strBehaviour = strCommand.substr(0, (-1 != idxOption) ? idxOption : idxParam);
	strOption = strParam = "";

	// If <param> is missing it's an improperly formatted command
	if ( (-1 == idxParam) || ((int)strCommand.length() - 1 == idxParam) )
	{
		// Unless "<behaviour> == "clear" AND (idxOption == 0)" 
		// OR <behaviour> == "clear" AND (idxParam != 0) [see table above]
		if ( ("clear" == strBehaviour) && ( (!idxOption) || (idxParam) ) )
			return TRUE;
		return FALSE;
	}

	if ( (-1 != idxOption) && (idxOption + 1 != idxParam) )
		strOption = strCommand.substr(idxOption + 1, idxParam - idxOption - 1);
	strParam = strCommand.substr(idxParam + 1);

	return TRUE;
}

void RlvCommand::initLookupTable()
{
	static bool fInitialized = false;
	if (!fInitialized)
	{
		// NOTE: keep this match with the enumeration at all times
		std::string arBehaviours[RLV_BHVR_COUNT] =
			{
				"version", "detach", "redirchat", "rediremote", "sendim", "recvchat", "recvemote", "recvim", "tploc", "tplure", 
				"sittp", "edit", "rez", "addoutfit", "remoutfit", "getoutfit", "getattach", "showinv", "unsit", "sit",
				"getstatus", "getstatusall", "getinv", "getinvworn", "findfolder", "findfolders", "attach", "attachall", "detachall",
				"getpath", "attachthis", "attachallthis", "detachthis", "detachallthis", "fartouch", "showworldmap", "showminimap",
				"showloc", "tpto", "accepttp", "shownames", "fly", "getsitid", "setdebug", "setenv", "detachme", 
				"showhovertextall", "showhovertextworld", "showhovertexthud", "showhovertext", "notify"
			};

		for (int idxBvhr = 0; idxBvhr < RLV_BHVR_COUNT; idxBvhr++)
			m_BhvrLookup.addKeyword(arBehaviours[idxBvhr], idxBvhr);

		fInitialized = true;
	}
}

// Checked: 2009-06-07 (RLVa-0.2.1c)
std::string RlvCommand::asString() const
{
	return (!m_strOption.empty()) ? (std::string(m_strBehaviour)).append(":").append(m_strOption) : (std::string(m_strBehaviour));
}

// =========================================================================

BOOL RlvObject::addCommand(const RlvCommand& rlvCmd)
{
	// Sanity checking
	if (RLV_TYPE_ADD != rlvCmd.getParamType())
		return FALSE;

	// Don't add duplicate commands for this object (ie @detach=n followed by another @detach=n later on)
	BOOL fDuplicate = 
		(RLV_BHVR_UNKNOWN != rlvCmd.getBehaviourType())
			? hasBehaviour(rlvCmd.getBehaviourType(), rlvCmd.getOption())
			: hasBehaviour(rlvCmd.getBehaviour(), rlvCmd.getOption());
	if (fDuplicate)
		return FALSE;

	// Now that we know it's not a duplicate, add it to the end of the list
	m_Commands.push_back(rlvCmd);

	return TRUE;
}

BOOL RlvObject::removeCommand(const RlvCommand& rlvCmd)
{
	// Sanity checking
	if (RLV_TYPE_REMOVE != rlvCmd.getParamType())
		return FALSE;

	for (rlv_command_list_t::iterator itCmd = m_Commands.begin(); itCmd != m_Commands.end(); ++itCmd)
	{
		//if (*itCmd == rlvCmd) <- commands will never be equal since one is an add and the other is a remove *rolls eyes*
		if ( (itCmd->getBehaviour() == rlvCmd.getBehaviour()) && (itCmd->getOption() == rlvCmd.getOption()) )
		{
			m_Commands.erase(itCmd);
			return TRUE;
		}
	}
	return FALSE;	// Command was never added so nothing to remove now
}

BOOL RlvObject::hasBehaviour(ERlvBehaviour eBehaviour) const
{
	for (rlv_command_list_t::const_iterator itCmd = m_Commands.begin(); itCmd != m_Commands.end(); ++itCmd)
		if ( (itCmd->getBehaviourType() == eBehaviour) && (itCmd->getOption().empty()) )
			return TRUE;
	return FALSE;
}

BOOL RlvObject::hasBehaviour(const std::string& strBehaviour) const
{
	for (rlv_command_list_t::const_iterator itCmd = m_Commands.begin(); itCmd != m_Commands.end(); ++itCmd)
		if ( (itCmd->getBehaviour() == strBehaviour) && (itCmd->getOption().empty()) )
			return TRUE;
	return FALSE;
}

BOOL RlvObject::hasBehaviour(ERlvBehaviour eBehaviour, const std::string& strOption) const
{
	for (rlv_command_list_t::const_iterator itCmd = m_Commands.begin(); itCmd != m_Commands.end(); ++itCmd)
		if ( (itCmd->getBehaviourType() == eBehaviour) && (itCmd->getOption() == strOption) )
			return TRUE;
	return FALSE;
}

BOOL RlvObject::hasBehaviour(const std::string& strBehaviour, const std::string& strOption) const
{
	for (rlv_command_list_t::const_iterator itCmd = m_Commands.begin(); itCmd != m_Commands.end(); ++itCmd)
		if ( (itCmd->getBehaviour() == strBehaviour) && (itCmd->getOption() == strOption) )
			return TRUE;
	return FALSE;
}

// Checked: 2009-06-07 (RLVa-0.2.1c)
std::string RlvObject::getStatusString(const std::string& strMatch) const
{
	std::string strStatus, strCmd;

	for (rlv_command_list_t::const_iterator itCmd = m_Commands.begin(); itCmd != m_Commands.end(); ++itCmd)
	{
		strCmd = itCmd->asString();
		if ( (strMatch.empty()) || (std::string::npos != strCmd.find(strMatch)) )
		{
			if (!strStatus.empty())
				strStatus.push_back('/');
			strStatus += strCmd;
		}
	}

	return strStatus;
}

// =========================================================================
/*
 * Various helper classes/timers/functors
 *
 */

// Checked: 2009-05-26 (RLVa-0.2.0d) | Modified: RLVa-0.2.0d
S32 rlvGetDirectDescendentsCount(const LLInventoryCategory* pFolder, LLAssetType::EType type)
{
	S32 cntType = 0;
	if (pFolder)
	{
		LLInventoryModel::cat_array_t*  pFolders;
		LLInventoryModel::item_array_t* pItems;
		gInventory.getDirectDescendentsOf(pFolder->getUUID(), pFolders, pItems);

		if (pItems)
		{
			for (S32 idxItem = 0, cntItem = pItems->count(); idxItem < cntItem; idxItem++)
				if (pItems->get(idxItem)->getType() == type)
					cntType++;
		}
	}
	return cntType;
}

// Checked: 2009-05-30 (RLVa-0.2.0e) | Added: RLVa-0.2.0e
const LLUUID& RlvWearableItemCollector::getFoldedParent(const LLUUID& idFolder) const
{
	std::map<LLUUID, LLUUID>::const_iterator itFolder = m_Folding.end(), itCur = m_Folding.find(idFolder);
	while (itCur != m_Folding.end())
	{
		itFolder = itCur;
		itCur = m_Folding.find(itFolder->second);
	}
	return (m_Folding.end() == itFolder) ? idFolder : itFolder->second;
}

// Checked: 2009-07-29 (RLVa-1.0.1b) | Modified: RLVa-1.0.1b
bool RlvWearableItemCollector::onCollectFolder(const LLInventoryCategory* pFolder)
{
	const LLUUID& idParent = pFolder->getParentUUID();
	if (m_Wearable.end() == std::find(m_Wearable.begin(), m_Wearable.end(), idParent))
		return false;															// Not the child of a wearable folder == skip

	const std::string& strFolder = pFolder->getName();
	if (strFolder.empty())														// Shouldn't happen but does... naughty Lindens
		return false;

	#ifdef RLV_EXTENSION_FLAG_NOSTRIP
		if ( (!m_fAttach) && (-1 != strFolder.find(RLV_FOLDER_FLAG_NOSTRIP)) )	// Don't process "nostrip" folders on detach
			return false;
	#endif // RLV_EXTENSION_FLAG_NOSTRIP

	if (gRlvHandler.isFoldedFolder(pFolder, m_fAttach))							// Check for folder that should get folded under its parent
	{
		m_Tentative.push_front(pFolder->getUUID());
		m_Folding.insert(std::pair<LLUUID, LLUUID>(pFolder->getUUID(), idParent));
	}
	else if ( (RLV_FOLDER_PREFIX_HIDDEN != strFolder[0]) && (m_fMatchAll) )		// Collect from any non-hidden child folder for *all
	{
		m_Wearable.push_front(pFolder->getUUID());
		return (idParent == m_idFolder);										// (Convenience for @getinvworn)
	}
	#ifdef RLV_EXPERIMENTAL_COMPOSITES
	else if ( (RLV_FOLDER_PREFIX_HIDDEN == strFolder[0]) &&						// Hidden folder that's a... 
			  (gRlvHandler.isCompositeFolder(pFolder)) &&						// ... composite folder which we...
		      ((m_fAttach) || (gRlvHandler.canTakeOffComposite(pFolder))) )		// ... attach or can detach (see composite locking)
	{
		m_Wearable.push_front(pFolder->getUUID());
		m_Folding.insert(std::pair<LLUUID, LLUUID>(pFolder->getUUID(), idParent));
	}
	#endif // RLV_EXPERIMENTAL_COMPOSITES

	return false;
}

// Checked: 2009-05-26 (RLVa-0.2.0d) | Modified: RLVa-0.2.0d
bool RlvWearableItemCollector::onCollectItem(const LLInventoryItem* pItem)
{
	#ifdef RLV_EXTENSION_FLAG_NOSTRIP
		if ( (!m_fAttach) && (-1 != pItem->getName().find(RLV_FOLDER_FLAG_NOSTRIP)) )	// Don't process "nostrip" items on detach
			return false;
	#endif // RLV_EXTENSION_FLAG_NOSTRIP

	const LLUUID& idParent = pItem->getParentUUID(); bool fRet = false;
	switch (pItem->getType())
	{
		case LLAssetType::AT_BODYPART:
			if (!m_fAttach)
				break;																	// Don't process body parts on detach
		case LLAssetType::AT_CLOTHING:
			#ifdef RLV_EXTENSION_FLAG_NOSTRIP
				fRet = ( (m_Wearable.end() != std::find(m_Wearable.begin(), m_Wearable.end(), idParent)) ||
						 ( (m_fAttach) && (m_Tentative.end() != std::find(m_Tentative.begin(), m_Tentative.end(), idParent)) &&
						   (gInventory.getCategory(pItem->getParentUUID())->getName() == ".("RLV_FOLDER_FLAG_NOSTRIP")") ) );
			#else
				fRet = (m_Wearable.end() != std::find(m_Wearable.begin(), m_Wearable.end(), idParent));
			#endif // RLV_EXTENSION_FLAG_NOSTRIP
			break;
		case LLAssetType::AT_OBJECT:
			fRet = ( (m_Wearable.end() != std::find(m_Wearable.begin(), m_Wearable.end(), idParent)) || 
				     (m_Tentative.end() != std::find(m_Tentative.begin(), m_Tentative.end(), idParent)) ) &&
				   ( (!m_fAttach) || (gRlvHandler.hasAttachPointName(pItem, true)) );	// Only care about attach point on attach*
			break;
		#ifdef RLV_EXPERIMENTAL_FORCEWEAR_GESTURES
		case LLAssetType::AT_GESTURE:
			fRet = (m_Wearable.end() != std::find(m_Wearable.begin(), m_Wearable.end(), idParent));
			break;
		#endif // RLV_EXPERIMENTAL_FORCEWEAR_GESTURES
		default:
			break;
	}
	return fRet;
}

// Checked: 2009-05-26 (RLVa-0.2.0d) | Modified: RLVa-0.2.0d
bool RlvWearableItemCollector::operator()(LLInventoryCategory* pFolder, LLInventoryItem* pItem)
{
	// NOTE: this is used for more than was originally intended so only modify if you're sure it won't break something obscure
	return (pFolder) ? onCollectFolder(pFolder) : ( (pItem) ? onCollectItem(pItem) : false );
}

// Checked: 2009-07-06 (RLVa-1.0.0c) | Modified: RLVa-0.2.0f
bool RlvSelectHasLockedAttach::apply(LLSelectNode* pNode)
{
	return (pNode->getObject()) ? !gRlvHandler.isDetachable(pNode->getObject()) : false;
}

// Checked: 2009-07-05 (RLVa-1.0.0b) | Modified: RLVa-0.2.0f
bool RlvSelectIsOwnedByOrGroupOwned::apply(LLSelectNode* pNode)
{
	return (pNode->mPermissions->isGroupOwned()) || (pNode->mPermissions->getOwner() == m_idAgent);
}

// Checked: 2009-05-31 (RLVa-0.2.0f) | Modified: RLVa-0.2.0f
bool RlvSelectIsSittingOn::apply(LLSelectNode* pNode)
{
	return (pNode->getObject()) && (pNode->getObject()->getRootEdit() == m_pObject);
}

// Checked: 2009-07-05 (RLVa-1.0.0b) | Modified: RLVa-0.2.0g
bool rlvCanDeleteOrReturn()
{
	bool fIsAllowed = true;

	if (gRlvHandler.hasBehaviour(RLV_BHVR_REZ))
	{
		// We'll allow if none of the prims are owned by the avie or group owned
		LLObjectSelectionHandle handleSel = LLSelectMgr::getInstance()->getSelection();
		RlvSelectIsOwnedByOrGroupOwned func(gAgent.getID());
		if ( (handleSel.notNull()) && ((0 == handleSel->getRootObjectCount()) || (NULL != handleSel->getFirstRootNode(&func, FALSE))) )
			fIsAllowed = false;
	}
	
	if ( (gRlvHandler.hasBehaviour(RLV_BHVR_UNSIT)) && (gAgent.getAvatarObject()) )
	{
		// We'll allow if the avie isn't sitting on any of the selected objects
		LLObjectSelectionHandle handleSel = LLSelectMgr::getInstance()->getSelection();
		RlvSelectIsSittingOn func(gAgent.getAvatarObject()->getRoot());
		if ( (handleSel.notNull()) && (handleSel->getFirstRootNode(&func, TRUE)) )
			fIsAllowed = false;
	}

	return fIsAllowed;
}

// Checked: 2009-09-08 (RLVa-1.0.2c) | Modified: RLVa-1.0.2c
BOOL rlvAttachToEnabler(void* pParam)
{
	// Enables/disables an option on the "Attach to (HUD)" submenu depending on whether it is (un)detachable
	return gRlvHandler.isDetachable((LLViewerJointAttachment*)pParam);
}

BOOL RlvGCTimer::tick()
{
	bool fContinue = gRlvHandler.onGC();
	if (!fContinue)
		gRlvHandler.m_pGCTimer = NULL;
	return !fContinue;
}

void RlvCurrentlyWorn::fetchWorn()
{
	LLInventoryFetchObserver::item_ref_t idItems;

	// Fetch all currently worn clothing layers and body parts
	for (int type = 0; type < (int)WT_COUNT; type++)
	{
		const LLUUID& idItem = gAgent.getWearableItem((EWearableType)type);
		if (idItem.notNull())
			idItems.push_back(idItem);
	}

	// Fetch all currently worn attachments
	LLVOAvatar* pAvatar = gAgent.getAvatarObject();
	if (pAvatar)
	{
		for (LLVOAvatar::attachment_map_t::const_iterator itAttach = pAvatar->mAttachmentPoints.begin(); 
			 itAttach != pAvatar->mAttachmentPoints.end(); ++itAttach)
		{
			const LLUUID& idItem = itAttach->second->getItemID();
			if (idItem.notNull())
				idItems.push_back(idItem);
		}
	}

	RlvCurrentlyWorn f;
	f.fetchItems(idItems);
}

// =========================================================================

// Checked: 2009-06-03 (RLVa-0.2.0h) | Added: RLVa-0.2.0h
void RlvWLSnapshot::restoreSnapshot(const RlvWLSnapshot* pWLSnapshot)
{
	LLWLParamManager* pWLParams = LLWLParamManager::instance();
	if ( (pWLSnapshot) && (pWLParams) )
	{
		pWLParams->mAnimator.mIsRunning = pWLSnapshot->fIsRunning;
		pWLParams->mAnimator.mUseLindenTime = pWLSnapshot->fUseLindenTime;
		pWLParams->mCurParams = pWLSnapshot->WLParams;
		pWLParams->propagateParameters();
	}
}

// Checked: 2009-06-03 (RLVa-0.2.0h) | Added: RLVa-0.2.0h
RlvWLSnapshot* RlvWLSnapshot::takeSnapshot()
{
	RlvWLSnapshot* pWLSnapshot = NULL;
	LLWLParamManager* pWLParams = LLWLParamManager::instance();
	if (pWLParams)
	{
		pWLSnapshot = new RlvWLSnapshot();
		pWLSnapshot->fIsRunning = pWLParams->mAnimator.mIsRunning;
		pWLSnapshot->fUseLindenTime = pWLParams->mAnimator.mUseLindenTime;
		pWLSnapshot->WLParams = pWLParams->mCurParams;
	}
	return pWLSnapshot;
}

// =========================================================================

BOOL RlvSettings::fShowNameTags = FALSE;

#ifdef RLV_EXTENSION_STARTLOCATION
	// Checked: 2009-07-08 (RLVa-1.0.0e) | Modified: RLVa-0.2.1d
	void RlvSettings::updateLoginLastLocation()
	{
		if (gSavedPerAccountSettings.controlExists(RLV_SETTING_LOGINLASTLOCATION))
		{
			BOOL fValue = (gRlvHandler.hasBehaviour(RLV_BHVR_TPLOC)) || 
						  ( (gRlvHandler.hasBehaviour(RLV_BHVR_UNSIT)) && 
						    (gAgent.getAvatarObject()) && (!gAgent.getAvatarObject()->mIsSitting) );
			if (gSavedPerAccountSettings.getBOOL(RLV_SETTING_LOGINLASTLOCATION) != fValue)
			{
				gSavedPerAccountSettings.setBOOL(RLV_SETTING_LOGINLASTLOCATION, fValue);
				gSavedPerAccountSettings.saveToFile(gSavedSettings.getString("PerAccountSettingsFile"), TRUE);
			}
		}
	}
#endif // RLV_EXTENSION_STARTLOCATION

// =========================================================================

#ifdef RLV_ADVANCED_TOGGLE_RLVA
	// Checked: 2009-07-12 (RLVa-1.0.0h) | Modified: RLVa-1.0.0h
	void rlvDbgToggleEnabled(void*)
	{
		gSavedSettings.setBOOL(RLV_SETTING_MAIN, !rlv_handler_t::isEnabled());

		#if RLV_TARGET < RLV_MAKE_TARGET(1, 23, 0)			// Version: 1.22.11
			LLStringUtil::format_map_t args;
			args["[MESSAGE]"] = llformat("Restrained Life Support will be %s after you restart", 
				(rlv_handler_t::isEnabled()) ? "disabled" : "enabled" );
			gViewerWindow->alertXml("GenericAlert", args);
		#else												// Version: 1.23.4
			LLSD args;
			args["MESSAGE"] = llformat("Restrained Life Support will be %s after you restart", 
				(rlv_handler_t::isEnabled()) ? "disabled" : "enabled" );
			LLNotifications::instance().add("GenericAlert", args);
		#endif
	}
	// Checked: 2009-07-08 (RLVa-1.0.0e)
	BOOL rlvDbgGetEnabled(void*)
	{
		return rlv_handler_t::isEnabled();
	}
#endif // RLV_ADVANCED_TOGGLE_RLVA

// =========================================================================
// Message sending functions
//

// Checked: 2009-08-11 (RLVa-1.0.1h) | Added: RLVa-1.0.1h
void rlvForceDetach(LLViewerJointAttachment* pAttachPt)
{
	// Copy/paste from handle_detach_from_avatar()
	LLViewerObject* attached_object = pAttachPt->getObject();
	if (attached_object)
	{
		gMessageSystem->newMessage("ObjectDetach");
		gMessageSystem->nextBlockFast(_PREHASH_AgentData);
		gMessageSystem->addUUIDFast(_PREHASH_AgentID, gAgent.getID() );
		gMessageSystem->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());

		gMessageSystem->nextBlockFast(_PREHASH_ObjectData);
		gMessageSystem->addU32Fast(_PREHASH_ObjectLocalID, attached_object->getLocalID());
		gMessageSystem->sendReliable( gAgent.getRegionHost() );
	}
}

void rlvSendBusyMessage(const LLUUID& idTo, const std::string& strMsg, const LLUUID& idSession)
{
	// (See process_improved_im)
	std::string strFullName;
	gAgent.buildFullname(strFullName);

	pack_instant_message(gMessageSystem, gAgent.getID(), FALSE, gAgent.getSessionID(), idTo, strFullName,
		strMsg, IM_ONLINE, IM_BUSY_AUTO_RESPONSE, idSession);
	gAgent.sendReliableMessage();
}

// Checked: 2009-08-05 (RLVa-1.0.1e) | Modified: RLVa-1.0.1e
bool rlvSendChatReply(S32 nChannel, const std::string& strReply)
{
	if (!rlvIsValidChannel(nChannel))
		return false;

	// Copy/paste from send_chat_from_viewer()
	LLMessageSystem* msg = gMessageSystem;
	msg->newMessageFast(_PREHASH_ChatFromViewer);
	msg->nextBlockFast(_PREHASH_AgentData);
	msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
	msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
	msg->nextBlockFast(_PREHASH_ChatData);
	msg->addStringFast(_PREHASH_Message, strReply);
	msg->addU8Fast(_PREHASH_Type, CHAT_TYPE_SHOUT);
	msg->addS32("Channel", nChannel);
	gAgent.sendReliableMessage();
	LLViewerStats::getInstance()->incStat(LLViewerStats::ST_CHAT_COUNT);

	return true;
}

// =========================================================================
// String helper functions
//

// Checked: 2009-07-04 (RLVa-1.0.0a)
void rlvStringReplace(std::string& strText, std::string strFrom, const std::string& strTo)
{
	if (strFrom.empty())
		return;

	size_t lenFrom = strFrom.length();
	size_t lenTo = strTo.length();

	std::string strTemp(strText);
	LLStringUtil::toLower(strTemp);
	LLStringUtil::toLower(strFrom);

	std::string::size_type idxCur, idxStart = 0, idxOffset = 0;
	while ( (idxCur = strTemp.find(strFrom, idxStart)) != std::string::npos)
	{
		strText.replace(idxCur + idxOffset, lenFrom, strTo);
		idxStart = idxCur + lenFrom;
		idxOffset += lenTo - lenFrom;
	}
}

// Checked: 2009-07-29 (RLVa-1.0.1b) | Added: RLVa-1.0.1b
std::string rlvGetFirstParenthesisedText(const std::string& strText, std::string::size_type* pidxMatch /*=NULL*/)
{
	if (pidxMatch)
		*pidxMatch = std::string::npos;	// Assume we won't find anything

	std::string::size_type idxIt, idxStart; int cntLevel = 1;
	if ((idxStart = strText.find_first_of('(')) == std::string::npos)
		return std::string();

	const char* pstrText = strText.c_str(); idxIt = idxStart;
	while ( (cntLevel > 0) && (idxIt < strText.length()) )
	{
		if ('(' == pstrText[++idxIt])
			cntLevel++;
		else if (')' == pstrText[idxIt])
			cntLevel--;
	}

	if (idxIt < strText.length())
	{
		if (pidxMatch)
			*pidxMatch = idxStart;	// Return the character index of the starting '('
		return strText.substr(idxStart + 1, idxIt - idxStart - 1);
	}
	return std::string();
}

// Checked: 2009-07-29 (RLVa-1.0.1b) | Added: RLVa-1.0.1b
std::string rlvGetLastParenthesisedText(const std::string& strText, std::string::size_type* pidxStart /*=NULL*/)
{
	if (pidxStart)
		*pidxStart = std::string::npos;	// Assume we won't find anything

	// Extracts the last - matched - parenthesised text from the input string
	std::string::size_type idxIt, idxEnd; int cntLevel = 1;
	if ((idxEnd = strText.find_last_of(')')) == std::string::npos)
		return std::string();

	const char* pstrText = strText.c_str(); idxIt = idxEnd;
	while ( (cntLevel > 0) && (idxIt >= 0) )
	{
		if (')' == pstrText[--idxIt])
			cntLevel++;
		else if ('(' == pstrText[idxIt])
			cntLevel--;
	}

	if ( (idxIt >= 0) && (idxIt < strText.length()) )	// NOTE: allow for std::string::size_type to be signed or unsigned
	{
		if (pidxStart)
			*pidxStart = idxIt;		// Return the character index of the starting '('
		return strText.substr(idxIt + 1, idxEnd - idxIt - 1);
	}
	return std::string();
}

// =========================================================================
// Debug helper functions
//

// =========================================================================
