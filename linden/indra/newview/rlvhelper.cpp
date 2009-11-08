#include "llviewerprecompiledheaders.h"
#include "llagent.h"
#include "llfloaterwindlight.h"
#include "llinventoryview.h"
#include "llviewerobject.h"
#include "llviewerstats.h"
#include "llviewerwindow.h"
#include "llvoavatar.h"
#include "llwlparammanager.h"

#include "rlvhelper.h"
#include "rlvevent.h"
#include "rlvhandler.h"

// Only defined in llinventorybridge.cpp
#if RLV_TARGET < RLV_MAKE_TARGET(1, 23, 0)			// Version: 1.22.11
	#include "llinventorybridge.h"
	void confirm_replace_attachment_rez(S32 option, void* user_data);
#endif

// ============================================================================
// Static variable initialization
//

RlvCommand::RlvBhvrTable RlvCommand::m_BhvrMap;

// ============================================================================
// RlvCommmand
//

// Checked: 2009-09-10 (RLVa-1.0.3a) | Modified: RLVa-1.0.3a
RlvCommand::RlvCommand(const std::string& strCommand)
	: m_eBehaviour(RLV_BHVR_UNKNOWN), m_fStrict(false), m_eParamType(RLV_TYPE_UNKNOWN)
{
	if ((m_fValid = parseCommand(strCommand, m_strBehaviour, m_strOption, m_strParam)))
	{
		S32 nTemp = 0;
		if ( ("n" == m_strParam) || ("add" == m_strParam) )
			m_eParamType = RLV_TYPE_ADD;
		else if ( ("y" == m_strParam) || ("rem" == m_strParam) )
			m_eParamType = RLV_TYPE_REMOVE;
		else if ("force" == m_strParam)
			m_eParamType = RLV_TYPE_FORCE;
		else if (LLStringUtil::convertToS32(m_strParam, nTemp))	// Assume it's a reply command if we can convert <param> to an S32
			m_eParamType = RLV_TYPE_REPLY;
		else if (m_strBehaviour == "clear")						// clear is the odd one out so just make it its own type
			m_eParamType = RLV_TYPE_CLEAR;
		else
		{
			m_eParamType = RLV_TYPE_UNKNOWN;
			m_fValid = false;
		}
	}

	if (!m_fValid)
	{
		m_strBehaviour = m_strOption = m_strParam = "";
		return;
	}

	// Check if this is the "strict" (aka "secure") variation of a behaviour
	std::string::size_type idxStrict = m_strBehaviour.find("_sec");
	m_fStrict = (std::string::npos != idxStrict) && (idxStrict + 4 == m_strBehaviour.length());

	RlvBhvrTable::const_iterator itBhvr = m_BhvrMap.find( (!m_fStrict) ? m_strBehaviour : m_strBehaviour.substr(0, idxStrict));
	if ( (itBhvr != m_BhvrMap.end()) && ((!m_fStrict) || (hasStrictVariant(itBhvr->second))) )
		m_eBehaviour = itBhvr->second;
}


bool RlvCommand::parseCommand(const std::string& strCommand, std::string& strBehaviour, std::string& strOption, std::string& strParam)
{
	// (See behaviour notes for the command parsing truth table)

	// Format: <behaviour>[:<option>]=<param>
	int idxParam  = strCommand.find('=');
	int idxOption = (idxParam > 0) ? strCommand.find(':') : -1;
	if (idxOption > idxParam - 1)
		idxOption = -1;

	// If <behaviour> is missing it's always an improperly formatted command
	if ( (0 == idxOption) || (0 == idxParam) )
		return false;

	strBehaviour = strCommand.substr(0, (-1 != idxOption) ? idxOption : idxParam);
	strOption = strParam = "";

	// If <param> is missing it's an improperly formatted command
	if ( (-1 == idxParam) || ((int)strCommand.length() - 1 == idxParam) )
	{
		// Unless "<behaviour> == "clear" AND (idxOption == 0)" 
		// OR <behaviour> == "clear" AND (idxParam != 0) [see table above]
		if ( ("clear" == strBehaviour) && ( (!idxOption) || (idxParam) ) )
			return true;
		return false;
	}

	if ( (-1 != idxOption) && (idxOption + 1 != idxParam) )
		strOption = strCommand.substr(idxOption + 1, idxParam - idxOption - 1);
	strParam = strCommand.substr(idxParam + 1);

	return true;
}

void RlvCommand::initLookupTable()
{
	static bool fInitialized = false;
	if (!fInitialized)
	{
		// NOTE: keep this matched with the enumeration at all times
		std::string arBehaviours[RLV_BHVR_COUNT] =
			{
				"version", "detach", "sendchat", "emote", "chatshout", "chatnormal", "chatwhisper", "redirchat", "rediremote",
				"sendim", "recvchat", "recvemote", "recvim", "tplm", "tploc", "tplure", "sittp", "edit", "rez",
				"addoutfit", "remoutfit", "getoutfit", "addattach", "remattach", "getattach", "showinv", "viewnote", "unsit", "sit", 
				"sendchannel", "getstatus", "getstatusall", "getinv", "getinvworn", "findfolder", "findfolders", 
				"attach", "attachall", "detachall", "getpath", "attachthis", "attachallthis", "detachthis", "detachallthis", 
				"fartouch", "showworldmap", "showminimap", "showloc", "tpto", "accepttp", "acceptpermission", "shownames", "fly", 
				"getsitid", "setdebug", "setenv", "detachme", "showhovertextall",  "showhovertextworld", "showhovertexthud", 
				"showhovertext", "notify", "defaultwear", "versionnum", "permissive", "viewscript", "viewtexture"
			};

		for (int idxBvhr = 0; idxBvhr < RLV_BHVR_COUNT; idxBvhr++)
			m_BhvrMap.insert(std::pair<std::string, ERlvBehaviour>(arBehaviours[idxBvhr], (ERlvBehaviour)idxBvhr));

		fInitialized = true;
	}
}

// =========================================================================
// RlvObject
//

RlvObject::RlvObject(const LLUUID& idObj) : m_UUID(idObj), m_nLookupMisses(0)
{
	LLViewerObject* pObj = gObjectList.findObject(idObj);
	m_fLookup = (NULL != pObj);
	m_idxAttachPt = (pObj) ? ATTACHMENT_ID_FROM_STATE(pObj->getState()) : 0;
}

bool RlvObject::addCommand(const RlvCommand& rlvCmd)
{
	// Sanity checking
	if (RLV_TYPE_ADD != rlvCmd.getParamType())
		return false;

	// Don't add duplicate commands for this object (ie @detach=n followed by another @detach=n later on)
	for (rlv_command_list_t::iterator itCmd = m_Commands.begin(); itCmd != m_Commands.end(); ++itCmd)
	{
		if ( (itCmd->getBehaviour() == rlvCmd.getBehaviour()) && (itCmd->getOption() == rlvCmd.getOption()) && 
			 (itCmd->isStrict() == rlvCmd.isStrict() ) )
		{
			return false;
		}
	}

	// Now that we know it's not a duplicate, add it to the end of the list
	m_Commands.push_back(rlvCmd);

	return true;
}

bool RlvObject::removeCommand(const RlvCommand& rlvCmd)
{
	// Sanity checking
	if (RLV_TYPE_REMOVE != rlvCmd.getParamType())
		return false;

	for (rlv_command_list_t::iterator itCmd = m_Commands.begin(); itCmd != m_Commands.end(); ++itCmd)
	{
		//if (*itCmd == rlvCmd) <- commands will never be equal since one is an add and the other is a remove *rolls eyes*
		if ( (itCmd->getBehaviour() == rlvCmd.getBehaviour()) && (itCmd->getOption() == rlvCmd.getOption()) && 
			 (itCmd->isStrict() == rlvCmd.isStrict() ) )
		{
			m_Commands.erase(itCmd);
			return true;
		}
	}
	return false;	// Command was never added so nothing to remove now
}

bool RlvObject::hasBehaviour(ERlvBehaviour eBehaviour, bool fStrictOnly) const
{
	for (rlv_command_list_t::const_iterator itCmd = m_Commands.begin(); itCmd != m_Commands.end(); ++itCmd)
		if ( (itCmd->getBehaviourType() == eBehaviour) && (itCmd->getOption().empty()) && ((!fStrictOnly) || (itCmd->isStrict())) )
			return true;
	return false;
}

bool RlvObject::hasBehaviour(ERlvBehaviour eBehaviour, const std::string& strOption, bool fStrictOnly) const
{
	for (rlv_command_list_t::const_iterator itCmd = m_Commands.begin(); itCmd != m_Commands.end(); ++itCmd)
		if ( (itCmd->getBehaviourType() == eBehaviour) && (itCmd->getOption() == strOption) && ((!fStrictOnly) || (itCmd->isStrict())) )
			return true;
	return false;
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

// ============================================================================
// RlvAttachmentManager
//

// Checked: 2009-10-12 (RLVa-1.0.5b) | Modified: RLVa-1.0.5b
void RlvAttachmentManager::forceAttach(const LLUUID& idItem, S32 idxAttachPt)
{
	#if RLV_TARGET < RLV_MAKE_TARGET(1, 23, 0)			// Version: 1.22.11
		LLAttachmentRezAction* rez_action = new LLAttachmentRezAction();
		rez_action->mItemID = idItem;
		rez_action->mAttachPt = idxAttachPt;

		confirm_replace_attachment_rez(0/*YES*/, (void*)rez_action); // (Will call delete on rez_action)
	#else												// Version: 1.23.4
		LLSD payload;
		payload["item_id"] = idItem;
		payload["attachment_point"] = idxAttachPt;

		LLNotifications::instance().forceResponse(
			LLNotification::Params("ReplaceAttachment").payload(payload), 0/*YES*/);
	#endif
}

// Checked: 2009-10-12 (RLVa-1.0.5b) | Modified: RLVa-1.0.5b
void RlvAttachmentManager::forceDetach(LLViewerJointAttachment* pAttachPt)
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

// Checked: 2009-10-12 (RLVa-1.0.5b) | Modified: RLVa-1.0.5b
void RlvAttachmentManager::onAttach(LLViewerJointAttachment* pAttachPt)
{
	S32 idxAttachPt = gRlvHandler.getAttachPointIndex(pAttachPt->getObject());
	if (!idxAttachPt)
		return;

	// If the attachment point has a pending "reattach" then we don't want to do anything
	rlv_attach_map_t::iterator itAttach = m_PendingAttach.find(idxAttachPt);
	if (itAttach != m_PendingAttach.end())
	{
		if (pAttachPt->getItemID() == itAttach->second.idItem)
			m_PendingAttach.erase(itAttach);
		return;
	}

	// Check if the attach is the result of a user action (="Wear")
	rlv_wear_map_t::iterator itWear = m_PendingWear.find(pAttachPt->getItemID());
	if (itWear != m_PendingWear.end())
	{
		// We need to return the attachment point to its previous state if it's non-attachable
		if (gRlvHandler.isLockedAttachment(idxAttachPt, RLV_LOCK_ADD))
		{
			// Get the state of the attachment point at the time the user picked "Wear" (if we don't have one it wasn't "add locked" then)
			std::map<S32, LLUUID>::iterator itAttachPrev = itWear->second.attachPts.find(idxAttachPt);
			if ( (itAttachPrev != itWear->second.attachPts.end()) && (pAttachPt->getItemID() != itAttachPrev->second) )
			{
				// If it was empty we need to force detach the new attachment; if it wasn't we need to reattach the old one
				if (itAttachPrev->second.isNull())
				{
					forceDetach(pAttachPt);
					m_PendingDetach.insert(std::pair<S32, LLUUID>(idxAttachPt, pAttachPt->getItemID()));
				}
				else if (m_PendingAttach.find(idxAttachPt) == m_PendingAttach.end()) // (only if we're not reattaching something else there)
				{
					m_PendingAttach.insert(std::pair<S32, RlvReattachInfo>(idxAttachPt, RlvReattachInfo(itAttachPrev->second)));
				}
			}
		}
		m_PendingWear.erase(itWear); // No need to start the timer since it should be running already if '!m_PendingWear.empty()'
	}
}

// Checked: 2009-10-12 (RLVa-1.0.5b) | Modified: RLVa-1.0.5b
void RlvAttachmentManager::onDetach(LLViewerJointAttachment* pAttachPt)
{
	S32 idxAttachPt = gRlvHandler.getAttachPointIndex(pAttachPt->getObject());
	if (!idxAttachPt)
		return;

	// If this is an attachment that we force-detached then we don't want to do anything (even if it is "remove locked")
	rlv_detach_map_t::iterator itDetach = m_PendingDetach.find(idxAttachPt);
	if ( (itDetach != m_PendingDetach.end()) && (itDetach->second == pAttachPt->getItemID()) )
	{
		m_PendingDetach.erase(itDetach);
		return;
	}

	// If the attachment is currently "remove locked" and we're not already trying to reattach something there we should reattach it
	if ( (m_PendingAttach.find(idxAttachPt) == m_PendingAttach.end()) && (gRlvHandler.isLockedAttachment(idxAttachPt, RLV_LOCK_REMOVE)) )
	{
		m_PendingAttach.insert(std::pair<S32, RlvReattachInfo>(idxAttachPt, RlvReattachInfo(pAttachPt->getItemID())));
		startTimer();
	}
}

// Checked: 2009-10-12 (RLVa-1.0.5b) | Modified: RLVa-1.0.5b
void RlvAttachmentManager::onSavedAssetIntoInventory(const LLUUID& idItem)
{
	for (rlv_attach_map_t::iterator itAttach = m_PendingAttach.begin(); itAttach != m_PendingAttach.end(); ++itAttach)
	{
		if ( (!itAttach->second.fAssetSaved) && (idItem == itAttach->second.idItem) )
		{
			forceAttach(itAttach->second.idItem, itAttach->first);
			itAttach->second.tsAttach = LLFrameTimer::getElapsedSeconds();
		}
	}
}

// Checked: 2009-10-12 (RLVa-1.0.5b) | Modified: RLVa-1.0.5b
BOOL RlvAttachmentManager::onTimer()
{
	F64 tsCurrent = LLFrameTimer::getElapsedSeconds();

	// Garbage collect (failed) wear requests older than 60 seconds
	rlv_wear_map_t::iterator itWear = m_PendingWear.begin();
	while (itWear != m_PendingWear.end())
	{
		if (itWear->second.tsWear + 60 < tsCurrent)
			m_PendingWear.erase(itWear++);
		else
			++itWear;
	}

	// Walk over the pending reattach list
	rlv_attach_map_t::iterator itAttach = m_PendingAttach.begin();
	while (itAttach != m_PendingAttach.end())
	{
		// Sanity check - make sure the item is still in the user's inventory
		if (gInventory.getItem(itAttach->second.idItem) == NULL)
		{
			m_PendingAttach.erase(itAttach++);
			continue;
		}

		// Force an attach if we haven't gotten an SavedAssetIntoInventory message after 15 seconds
		// (or if it's been 30 seconds since we last tried to reattach the item)
		bool fAttach = false;
		if ( (!itAttach->second.fAssetSaved) && (itAttach->second.tsDetach + 15 < tsCurrent) )
		{
			itAttach->second.fAssetSaved = true;
			fAttach = true;
		}
		else if ( (itAttach->second.fAssetSaved) && (itAttach->second.tsAttach + 30 < tsCurrent) )
		{
			fAttach = true;
		}

		if (fAttach)
		{
			forceAttach(itAttach->second.idItem, itAttach->first);
			itAttach->second.tsAttach = tsCurrent;
		}

		++itAttach;
	}

	return ( (m_PendingAttach.empty()) && (m_PendingDetach.empty()) && (m_PendingWear.empty()) );
}

// Checked: 2009-10-12 (RLVa-1.0.5b) | Modified: RLVa-1.0.5b
void RlvAttachmentManager::onWearAttachment(const LLUUID& idItem)
{
	// We only need to keep track of wears if there are non-attachable attachment points
	if (!gRlvHandler.hasLockedAttachment(RLV_LOCK_ADD))
		return;

	LLVOAvatar* pAvatar = gAgent.getAvatarObject();
	if (!pAvatar)
		return;

	// If the attachment point this will end up being attached to is:
	//   - unlocked    : nothing should happen (from RLVa's point of view)
	//   - RLV_LOCK_ADD: the new attachment should get detached and the current one reattached (unless it's currently empty)
	//   - RLV_LOCK_REM: the current attachment will get reattached on ObjectKill (if there is no current one then nothing should happen)
	RlvWearInfo infoWear(idItem);
	for (LLVOAvatar::attachment_map_t::const_iterator itAttach = pAvatar->mAttachmentPoints.begin(); 
			itAttach != pAvatar->mAttachmentPoints.end(); ++itAttach)
	{
		if (gRlvHandler.isLockedAttachment(itAttach->first, RLV_LOCK_ADD))	// We only need to keep track of these (see above)
			infoWear.attachPts.insert(std::pair<S32, LLUUID>(itAttach->first, itAttach->second->getItemID()));
	}

	m_PendingWear.insert(std::pair<LLUUID, RlvWearInfo>(idItem, infoWear));
	startTimer();
}

// ============================================================================
// RlvWearableItemCollector
//

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

// ============================================================================
// RlvWLSnapshot
//

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

// Checked: 2009-09-16 (RLVa-1.0.3c) | Modified: RLVa-1.0.3c
RlvWLSnapshot* RlvWLSnapshot::takeSnapshot()
{
	// HACK: see RlvExtGetSet::onGetEnv
	if (!LLFloaterWindLight::isOpen())
	{
		LLFloaterWindLight::instance()->close();
		LLFloaterWindLight::instance()->syncMenu();
	}

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
// RlvSettings
//

BOOL RlvSettings::fShowNameTags = FALSE;

BOOL RlvSettings::getEnableWear()
{
	return 
		rlvGetSettingBOOL(RLV_SETTING_ENABLEWEAR, TRUE) &&	 // "Enable Wear" is toggled on and...
		(!gRlvHandler.hasBehaviour(RLV_BHVR_DEFAULTWEAR)) && // not restricted and...
		(!gRlvHandler.hasBehaviour(RLV_BHVR_ADDATTACH));	 // we have attach points we can attach to [see RlvHandler::onAddRemAttach()]
}

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
// Various helper classes/timers/functors
//

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

// Checked: 2009-07-06 (RLVa-1.0.0c) | Modified: RLVa-0.2.0f
bool RlvSelectHasLockedAttach::apply(LLSelectNode* pNode)
{
	return (pNode->getObject()) ? gRlvHandler.isLockedAttachment(pNode->getObject(), m_eLock) : false;
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

// ============================================================================
// Various helper functions
//

// Checked: 2009-10-10 (RLVa-1.0.5a) | Modified: RLVa-1.0.5a
BOOL rlvAttachToEnabler(void* pParam)
{
	// Disable an option on the "Attach to (HUD)" submenu if:
	//   - the attachment point is locked non-detachable with an object attached
	//   - the attachment point is locked non-attachable
	return (pParam != NULL) && 
		(!gRlvHandler.isLockedAttachment(((LLViewerJointAttachment*)pParam)->getObject(), RLV_LOCK_REMOVE)) &&
		(!gRlvHandler.isLockedAttachment((LLViewerJointAttachment*)pParam, RLV_LOCK_ADD));
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

// Checked: 2009-10-04 (RLVa-1.0.4b) | Modified: RLVa-1.0.4b
BOOL rlvEnableWearEnabler(void* pParam)
{
	// Visually disables the "Enable Wear" option when restricted from toggling it
	return (!gRlvHandler.hasBehaviour(RLV_BHVR_DEFAULTWEAR));
}

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

#ifdef RLV_ADVANCED_TOGGLE_RLVA
	// Checked: 2009-07-12 (RLVa-1.0.0h) | Modified: RLVa-1.0.0h
	void rlvToggleEnabled(void*)
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
	BOOL rlvGetEnabled(void*)
	{
		return rlv_handler_t::isEnabled();
	}
#endif // RLV_ADVANCED_TOGGLE_RLVA

// =========================================================================
// Message sending functions
//

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
	if (!rlvIsValidReplyChannel(nChannel))
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
