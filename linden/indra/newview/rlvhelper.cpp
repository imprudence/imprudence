#include "llviewerprecompiledheaders.h"
#include "llfloaterwindlight.h"
#include "llgesturemgr.h"
#include "llinventoryview.h"
#include "llinventorybridge.h"
#include "llviewerobject.h"
#include "llviewerobjectlist.h"
#include "llviewerregion.h"
#include "llviewerstats.h"
#include "llviewerwindow.h"
#include "llvoavatar.h"
#include "llwearablelist.h"
#include "llwlparammanager.h"

#include "rlvhelper.h"
#include "rlvhandler.h"

// Defined in llinventorybridge.cpp
void wear_attachments_on_avatar(const LLInventoryModel::item_array_t& items, BOOL remove);
void wear_inventory_category_on_avatar_loop(LLWearable* wearable, void*);

// ============================================================================
// RlvCommmand
//

RlvCommand::RlvBhvrTable RlvCommand::m_BhvrMap;

// Checked: 2009-12-27 (RLVa-1.1.0k) | Modified: RLVa-1.1.0k
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
		else if (m_strBehaviour == "clear")						// clear is the odd one out so just make it its own type
			m_eParamType = RLV_TYPE_CLEAR;
		else if ("force" == m_strParam)
			m_eParamType = RLV_TYPE_FORCE;
		else if (LLStringUtil::convertToS32(m_strParam, nTemp))	// Assume it's a reply command if we can convert <param> to an S32
			m_eParamType = RLV_TYPE_REPLY;
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

	m_eBehaviour = getBehaviourFromString(m_strBehaviour, &m_fStrict);
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

// Checked: 2009-12-05 (RLVa-1.1.0h) | Added: RLVa-1.1.0h
ERlvBehaviour RlvCommand::getBehaviourFromString(const std::string& strBhvr, bool* pfStrict /*=NULL*/)
{
	std::string::size_type idxStrict = strBhvr.find("_sec");
	bool fStrict = (std::string::npos != idxStrict) && (idxStrict + 4 == strBhvr.length());
	if (pfStrict)
		*pfStrict = fStrict;

	RlvBhvrTable::const_iterator itBhvr = m_BhvrMap.find( (!fStrict) ? strBhvr : strBhvr.substr(0, idxStrict));
	if ( (itBhvr != m_BhvrMap.end()) && ((!fStrict) || (hasStrictVariant(itBhvr->second))) )
		return itBhvr->second;
	return RLV_BHVR_UNKNOWN;
}

void RlvCommand::initLookupTable()
{
	static bool fInitialized = false;
	if (!fInitialized)
	{
		// NOTE: keep this matched with the enumeration at all times
		std::string arBehaviours[RLV_BHVR_COUNT] =
			{
				"detach", "attach", "addattach", "remattach", "addoutfit", "remoutfit", "emote", "sendchat", "recvchat", "recvemote",
				"redirchat", "rediremote", "chatwhisper", "chatnormal", "chatshout", "sendchannel", "sendim", "recvim", "permissive",
				"notify", "showinv", "showminimap", "showworldmap", "showloc", "shownames", "showhovertext", "showhovertexthud",
				"showhovertextworld", "showhovertextall", "tplm", "tploc", "tplure", "viewnote", "viewscript", "viewtexture",
				"acceptpermission", "accepttp", "defaultwear", "allowidle", "edit", "rez", "fartouch", "interact", "touch",
				"touchattach", "touchhud", "touchworld", "fly", "unsit", "sit", "sittp", "setdebug", "setenv", "detachme", "detachthis",
				"detachall", "detachallthis", "attachthis", "attachall", "attachallthis", "tpto", "version", "versionnew", "versionnum",
				"getattach", "getattachnames", "getaddattachnames", "getremattachnames", "getoutfit", "getoutfitnames",
				"getaddoutfitnames", "getremoutfitnames", "findfolder", "findfolders", "getpath", "getinv", "getinvworn", "getsitid",
				"getstatus", "getstatusall"
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
	RLV_ASSERT(RLV_TYPE_ADD == rlvCmd.getParamType());

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
	RLV_ASSERT(RLV_TYPE_REMOVE == rlvCmd.getParamType());

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

// Checked: 2009-11-27 (RLVa-1.1.0f) | Modified: RLVa-1.1.0f
std::string RlvObject::getStatusString(const std::string& strMatch) const
{
	std::string strStatus, strCmd;

	for (rlv_command_list_t::const_iterator itCmd = m_Commands.begin(); itCmd != m_Commands.end(); ++itCmd)
	{
		strCmd = itCmd->asString();
		if ( (strMatch.empty()) || (std::string::npos != strCmd.find(strMatch)) )
		{
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
void RlvAttachmentManager::attach(const LLUUID& idItem, S32 idxAttachPt)
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
		LLNotifications::instance().forceResponse(LLNotification::Params("ReplaceAttachment").payload(payload), 0/*YES*/);
	#endif
}

// Checked: 2009-11-24 (RLVa-1.1.0k) | Added: RLVa-1.1.0e
void RlvAttachmentManager::detach(LLViewerJointAttachment* pAttachPt)
{
	// [See handle_detach_from_avatar()]
	if ( (pAttachPt) && (pAttachPt->getObject()) )
	{
		gMessageSystem->newMessage("ObjectDetach");
		gMessageSystem->nextBlockFast(_PREHASH_AgentData);
		gMessageSystem->addUUIDFast(_PREHASH_AgentID, gAgent.getID() );
		gMessageSystem->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());

		gMessageSystem->nextBlockFast(_PREHASH_ObjectData);
		gMessageSystem->addU32Fast(_PREHASH_ObjectLocalID, pAttachPt->getObject()->getLocalID());
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
					detach(pAttachPt);
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
			attach(itAttach->second.idItem, itAttach->first);
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
			attach(itAttach->second.idItem, itAttach->first);
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

// Checked: 2009-12-18 (RLVa-1.1.0k) | Modified: RLVa-1.1.0i
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

	if (gRlvHandler.isFoldedFolder(pFolder, m_fAttach, false))					// Check for folder that should get folded under its parent
	{
		m_Tentative.push_front(pFolder->getUUID());
		m_Folding.insert(std::pair<LLUUID, LLUUID>(pFolder->getUUID(), idParent));
	}
	else if ( (RLV_FOLDER_PREFIX_HIDDEN != strFolder[0]) && (m_fMatchAll) )		// Collect from any non-hidden child folder for *all
	{
		#ifdef RLV_EXPERIMENTAL_COMPOSITEFOLDERS
		if ( (!RlvSettings::getEnableComposites()) ||							// ... if we're not checking composite folders
			 (!gRlvHandler.isCompositeFolder(pFolder)) ||						// ... or if it's not a composite folder
		     ((m_fAttach) && (gRlvHandler.canWearComposite(pFolder))) ||		// ... or if we're attaching and can attach it OR
			 (!m_fAttach) && (gRlvHandler.canTakeOffComposite(pFolder)) )		// ... or if we're detaching and can detach it
		#endif // RLV_EXPERIMENTAL_COMPOSITEFOLDERS
		{
			m_Wearable.push_front(pFolder->getUUID());
		}
		return (idParent == m_idFolder);										// (Convenience for @getinvworn)
	}
	#ifdef RLV_EXPERIMENTAL_COMPOSITEFOLDERS
	else if ( (RlvSettings::getEnableComposites()) &&
			  (RLV_FOLDER_PREFIX_HIDDEN == strFolder[0]) &&						// Hidden folder that's a...
			  (gRlvHandler.isCompositeFolder(pFolder)) &&						// ... composite folder which we...
		      ( ((m_fAttach) && (gRlvHandler.canWearComposite(pFolder))) ||		// ... are attaching and can attach OR
			    (!m_fAttach) && (gRlvHandler.canTakeOffComposite(pFolder)) ) )	// ... are detaching and can detach
	{
		m_Wearable.push_front(pFolder->getUUID());
		m_Folding.insert(std::pair<LLUUID, LLUUID>(pFolder->getUUID(), idParent));
	}
	#endif // RLV_EXPERIMENTAL_COMPOSITEFOLDERS

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
				   ( (!m_fAttach) || (gRlvHandler.hasAttachPointName(pItem, true)) || (RlvSettings::getEnableSharedWear()) );
			break;
		#ifdef RLV_EXTENSION_FORCEWEAR_GESTURES
		case LLAssetType::AT_GESTURE:
			fRet = (m_Wearable.end() != std::find(m_Wearable.begin(), m_Wearable.end(), idParent));
			break;
		#endif // RLV_EXTENSION_FORCEWEAR_GESTURES
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
// RlvForceWear
//

// Checked: 2010-02-17 (RLVa-1.1.0o) | Modified: RLVa-1.1.0o
void RlvForceWear::forceFolder(const LLViewerInventoryCategory* pFolder, eWearAction eAction, eWearFlags eFlags)
{
	// [See LLWearableBridge::wearOnAvatar(): don't wear anything until initial wearables are loaded, can destroy clothing items]
	if (!gAgent.areWearablesLoaded())
	{
		LLNotifications::instance().add("CanNotChangeAppearanceUntilLoaded");
		return;
	}
	// Sanity check - getAvatarObject() can't be NULL [see RlvForceWear::isWearingItem()]
	LLVOAvatar* pAvatar = gAgent.getAvatarObject();
	if (!pAvatar)
		return;

	LLInventoryModel::cat_array_t folders;
	LLInventoryModel::item_array_t items;
	RlvWearableItemCollector functor(pFolder->getUUID(), (ACTION_ATTACH == eAction), (FLAG_MATCHALL & eFlags));

	// Grab a list of all the items we'll be wearing/attaching
	gInventory.collectDescendentsIf(pFolder->getUUID(), folders, items, FALSE, functor);

	for (S32 idxItem = 0, cntItem = items.count(); idxItem < cntItem; idxItem++)
	{
		LLViewerInventoryItem* pItem = items.get(idxItem);

		// If it's wearable it should be worn on detach
		if ( (ACTION_DETACH == eAction) && (isWearableItem(pItem)) && (!isWearingItem(pItem)) )
			continue;

		//  NOTES: * if there are composite items then RlvWearableItemCollector made sure they can be worn (or taken off depending)
		//         * some scripts issue @remattach=force,attach:worn-items=force so we need to attach items even if they're currently worn
		switch (pItem->getType())
		{
			case LLAssetType::AT_BODYPART:
				RLV_ASSERT(ACTION_ATTACH == eAction);	// RlvWearableItemCollector shouldn't be supplying us with body parts on detach
			case LLAssetType::AT_CLOTHING:
				if (ACTION_ATTACH == eAction)
				{
					// The check for whether we're replacing a currently worn composite item happens in onWearableArrived()
					if (std::find(m_addWearables.begin(), m_addWearables.end(), pItem) == m_addWearables.end())
						m_addWearables.push_back(pItem);
				}
				else
				{
					LLWearable* pWearable = gAgent.getWearableFromWearableItem(pItem->getUUID());
					if ( (pWearable) && (isForceRemovable(pWearable->getType(), false)) )
					{
						if (std::find(m_remWearables.begin(), m_remWearables.end(), pWearable->getType()) == m_remWearables.end())
							m_remWearables.push_back(pWearable->getType());
					}
				}
				break;

			case LLAssetType::AT_OBJECT:
				if (ACTION_ATTACH == eAction)
				{
					LLViewerJointAttachment* pAttachPt = gRlvHandler.getAttachPoint(pItem, true);
					if ( ( (pAttachPt) &&																 // Need a specific attach pt that
						   (!gRlvHandler.isLockedAttachment(pAttachPt->getObject(), RLV_LOCK_REMOVE)) && // doesn't have a locked object
						   (!gRlvHandler.isLockedAttachment(pAttachPt, RLV_LOCK_ADD)) ) ||               // and that can be attached to
						 (RlvSettings::getEnableSharedWear()) )
					{
						if (std::find(m_addAttachments.begin(), m_addAttachments.end(), pItem) == m_addAttachments.end())
						{
							#ifdef RLV_EXPERIMENTAL_COMPOSITEFOLDERS
							// We still need to check whether we're about to replace a currently worn composite item
							// (which we're not if we're just reattaching an attachment we're already wearing)
							LLViewerInventoryCategory* pCompositeFolder = NULL;
							if ( (pAttachPt->getObject()) && (RlvSettings::getEnableComposites()) &&
								 (pAttachPt->getItemID() != pItem->getUUID()) &&
								 (gRlvHandler.getCompositeInfo(pAttachPt->getItemID(), NULL, &pCompositeFolder)) )
							{
								// If we can't take off the composite folder this item would replace then don't allow it to get attached
								if (gRlvHandler.canTakeOffComposite(pCompositeFolder))
								{
									forceFolder(pCompositeFolder, ACTION_DETACH, FLAG_DEFAULT);
									m_addAttachments.push_back(pItem);
								}
							}
							else
							#endif // RLV_EXPERIMENTAL_COMPOSITEFOLDERS
							{
								m_addAttachments.push_back(pItem);
							}
						}
					}
				}
				else
				{
					LLViewerJointAttachment* pAttachPt = pAvatar->getWornAttachmentPoint(pItem->getUUID());
					if ( (pAttachPt) && (isForceDetachable(pAttachPt, false)) )
					{
						if (std::find(m_remAttachments.begin(), m_remAttachments.end(), pAttachPt) == m_remAttachments.end())
							m_remAttachments.push_back(pAttachPt);
					}
				}
				break;

			#ifdef RLV_EXTENSION_FORCEWEAR_GESTURES
			case LLAssetType::AT_GESTURE:
				if (ACTION_ATTACH == eAction)
				{
					if (std::find(m_addGestures.begin(), m_addGestures.end(), pItem) == m_addGestures.end())
						m_addGestures.push_back(pItem);
				}
				else
				{
					if (std::find(m_remGestures.begin(), m_remGestures.end(), pItem) == m_remGestures.end())
						m_remGestures.push_back(pItem);
				}
				break;
			#endif // RLV_EXTENSION_FORCEWEAR_GESTURES

			default:
				break;
		}
	}
}

// Checked: 2009-12-18 (RLVa-1.1.0k) | Added: RLVa-1.1.0i
bool RlvForceWear::isForceDetachable(LLViewerJointAttachment* pAttachPt, bool fCheckComposite /*=true*/, LLViewerObject* pExceptObj /*=NULL*/)
{
	// Attachment point can be detached by an RLV command if:
	//   - something is worn on the attachment point
	//   - what's worn isn't "remove locked" by anything (or anything except the object specified by pExceptObj)
	//   - what's worn is strippable
	//   - composite folders are disabled *or* what's worn isn't part of a composite folder that has at least one item locked
	#ifdef RLV_EXPERIMENTAL_COMPOSITEFOLDERS
	LLViewerInventoryCategory* pFolder = NULL;
	#endif // RLV_EXPERIMENTAL_COMPOSITEFOLDERS
	return
	  (
		(pAttachPt) && (pAttachPt->getObject()) &&
		( (!pExceptObj) ? (!gRlvHandler.isLockedAttachment(pAttachPt->getObject(), RLV_LOCK_REMOVE))
		                : (!gRlvHandler.isLockedAttachmentExcept(pAttachPt->getObject(), RLV_LOCK_REMOVE, pExceptObj)) )
		#ifdef RLV_EXTENSION_FLAG_NOSTRIP
		&& (gRlvHandler.isStrippable(pAttachPt->getItemID()))
		#endif // RLV_EXTENSION_FLAG_NOSTRIP
		#ifdef RLV_EXPERIMENTAL_COMPOSITEFOLDERS
		&& ( (!fCheckComposite) || (!RlvSettings::getEnableComposites()) ||
		     (!gRlvHandler.getCompositeInfo(pAttachPt->getItemID(), NULL, &pFolder)) || (gRlvHandler.canTakeOffComposite(pFolder)) )
		#endif // RLV_EXPERIMENTAL_COMPOSITEFOLDERS
	  );
}

// Checked: 2009-12-18 (RLVa-1.1.0k) | Added: RLVa-1.1.0i
void RlvForceWear::forceDetach(LLViewerJointAttachment* pAttachPt)
{
	// Sanity check - no need to process duplicate removes
	if ( (!pAttachPt) || (std::find(m_remAttachments.begin(), m_remAttachments.end(), pAttachPt) != m_remAttachments.end()) )
		return;

	if (isForceDetachable(pAttachPt))
	{
		#ifdef RLV_EXPERIMENTAL_COMPOSITEFOLDERS
			LLViewerInventoryCategory* pFolder = NULL;
			if ( (RlvSettings::getEnableComposites()) &&
				 (gRlvHandler.getCompositeInfo(pAttachPt->getItemID(), NULL, &pFolder)) )
			{
				// Attachment belongs to a composite folder so detach the entire folder (if we can take it off)
				if (gRlvHandler.canTakeOffComposite(pFolder))
					forceFolder(pFolder, ACTION_DETACH, FLAG_DEFAULT);
			}
			else
		#endif // RLV_EXPERIMENTAL_COMPOSITEFOLDERS
			{
				m_remAttachments.push_back(pAttachPt);
			}
	}
}

// Checked: 2009-12-18 (RLVa-1.1.0k) | Added: RLVa-1.1.0i
bool RlvForceWear::isForceRemovable(EWearableType wtType, bool fCheckComposite /*=true*/, const LLUUID& idExcept /*=LLUUID::null*/)
{
	// Wearable type can be removed by an RLV command if:
	//   - something is worn on that layer
	//   - its asset type is AT_CLOTHING
	//   - what's worn isn't "remove locked" by anything (or anything except the object specified by idExcept)
	//   - what's worn is strippable
	//   - composite folders are disabled *or* what's worn isn't part of a composite folder that has at least one item locked
	LLWearable* pWearable = gAgent.getWearable(wtType);
	#ifdef RLV_EXPERIMENTAL_COMPOSITEFOLDERS
	LLViewerInventoryCategory* pFolder = NULL;
	#endif // RLV_EXPERIMENTAL_COMPOSITEFOLDERS
	return
	  (
		(pWearable) &&
		(LLAssetType::AT_CLOTHING == LLWearable::typeToAssetType(wtType)) &&
		( (idExcept.notNull()) ? (gRlvHandler.isRemovable(wtType))
		                       : (gRlvHandler.isRemovableExcept(wtType, idExcept)) )
		#ifdef RLV_EXTENSION_FLAG_NOSTRIP
		&& (gRlvHandler.isStrippable(gAgent.getWearableItem(wtType)))
		#endif // RLV_EXTENSION_FLAG_NOSTRIP
		#ifdef RLV_EXPERIMENTAL_COMPOSITEFOLDERS
		&& ( (!fCheckComposite) || (!RlvSettings::getEnableComposites()) ||
		     (!gRlvHandler.getCompositeInfo(gAgent.getWearableItem(wtType), NULL, &pFolder)) || (gRlvHandler.canTakeOffComposite(pFolder)) )
		#endif // RLV_EXPERIMENTAL_COMPOSITEFOLDERS
	  );
}

// Checked: 2009-12-18 (RLVa-1.1.0k) | Added: RLVa-1.1.0i
void RlvForceWear::forceRemove(EWearableType wtType)
{
	// Sanity check - no need to process duplicate removes
	if ( (WT_INVALID == wtType) || (std::find(m_remWearables.begin(), m_remWearables.end(), wtType) != m_remWearables.end()) )
		return;

	if (isForceRemovable(wtType))
	{
		#ifdef RLV_EXPERIMENTAL_COMPOSITEFOLDERS
			LLViewerInventoryCategory* pFolder = NULL;
			if ( (RlvSettings::getEnableComposites()) &&
				 (gRlvHandler.getCompositeInfo(gAgent.getWearableItem(wtType), NULL, &pFolder)) )
			{
				// Wearable belongs to a composite folder so detach the entire folder (if we can take it off)
				if (gRlvHandler.canTakeOffComposite(pFolder))
					forceFolder(pFolder, ACTION_DETACH, FLAG_DEFAULT);
			}
			else
		#endif // RLV_EXPERIMENTAL_COMPOSITEFOLDERS
			{
				m_remWearables.push_back(wtType);
			}
	}
}

// Checked: 2009-12-18 (RLVa-1.1.0k) | Added: RLVa-1.1.0i
void RlvForceWear::processAdd()
{
	// Process attachments
	if (m_addAttachments.size())
	{
		// Workaround for RezMultipleAttachmentsFromInv bug (see http://jira.secondlife.com/browse/SVC-5383)
		#ifndef RLV_WORKAROUND_REZMULTIPLEATTACH
			wear_attachments_on_avatar(m_addAttachments, FALSE);
		#else
			for (S32 idxItem = 0, cntItem = m_addAttachments.count(); idxItem < cntItem; idxItem++)
			{
				LLViewerInventoryItem* pItem = m_addAttachments.get(idxItem);

				S32 idxAttachPt = gRlvHandler.getAttachPointIndex(pItem, true);
				if (0 != idxAttachPt)
				{
					#if RLV_TARGET < RLV_MAKE_TARGET(1, 23, 0)			// Version: 1.22.11
						LLAttachmentRezAction* rez_action = new LLAttachmentRezAction();
						rez_action->mItemID = pItem->getUUID();
						rez_action->mAttachPt = idxAttachPt;

						confirm_replace_attachment_rez(0/*YES*/, (void*)rez_action); // (Will call delete on rez_action)
					#else												// Version: 1.23.4
						LLSD payload;
						payload["item_id"] = pItem->getUUID();
						payload["attachment_point"] = idxAttachPt;

						LLNotifications::instance().forceResponse(LLNotification::Params("ReplaceAttachment").payload(payload), 0/*YES*/);
					#endif
				}
			}
		#endif // RLV_WORKAROUND_REZMULTIPLEATTACH

		m_addAttachments.clear();
	}

	// Process wearables
	if (m_addWearables.size())
	{
		// [See wear_inventory_category_on_avatar_step2()]
		LLWearableHoldingPattern* pWearData = new LLWearableHoldingPattern(TRUE);

		// We need to populate 'pWearData->mFoundList' before doing anything else because (some of) the assets might already be available
		for (S32 idxItem = 0, cntItem = m_addWearables.count(); idxItem < cntItem; idxItem++)
		{
			LLViewerInventoryItem* pItem = m_addWearables.get(idxItem);
			if ( (pItem) && ((LLAssetType::AT_BODYPART == pItem->getType()) || (LLAssetType::AT_CLOTHING == pItem->getType())) )
			{
				LLFoundData* pFound = new LLFoundData(pItem->getUUID(), pItem->getAssetUUID(), pItem->getName(), pItem->getType());
				pWearData->mFoundList.push_front(pFound);
			}
		}

		if (!pWearData->mFoundList.size())
		{
			delete pWearData;
			return;
		}

		// If all the assets are available locally then "pWearData" will be freed *before* the last "gWearableList.getAsset()" call returns
		bool fContinue = true; LLWearableHoldingPattern::found_list_t::const_iterator itWearable = pWearData->mFoundList.begin();
		while ( (fContinue) && (itWearable != pWearData->mFoundList.end()) )
		{
			const LLFoundData* pFound = *itWearable;
			++itWearable;
			fContinue = (itWearable != pWearData->mFoundList.end());
			gWearableList.getAsset(pFound->mAssetID, pFound->mName, pFound->mAssetType, onWearableArrived, (void*)pWearData);
		}

		m_addWearables.clear();
	}

	// Process gestures
	if (m_addGestures.size())
	{
		gGestureManager.activateGestures(m_addGestures);
		for (S32 idxGesture = 0, cntGesture = m_addGestures.count(); idxGesture < cntGesture; idxGesture++)
			gInventory.updateItem(m_addGestures.get(idxGesture));
		gInventory.notifyObservers();

		m_addGestures.clear();
	}
}

// Checked: 2009-12-18 (RLVa-1.1.0k) | Added: RLVa-1.1.0i
void RlvForceWear::processRem()
{
	// Process attachments
	if (m_remAttachments.size())
	{
		// [See LLAgent::userRemoveAllAttachments()]
		gMessageSystem->newMessage("ObjectDetach");
		gMessageSystem->nextBlockFast(_PREHASH_AgentData);
		gMessageSystem->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
		gMessageSystem->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());

		for (std::list<LLViewerJointAttachment*>::const_iterator itAttachPt = m_remAttachments.begin();
				itAttachPt != m_remAttachments.end(); ++itAttachPt)
		{
			LLViewerJointAttachment* pAttachPt = *itAttachPt;
			if (pAttachPt->getObject())
			{
				gMessageSystem->nextBlockFast(_PREHASH_ObjectData);
				gMessageSystem->addU32Fast(_PREHASH_ObjectLocalID, pAttachPt->getObject()->getLocalID());
			}
		}

		gMessageSystem->sendReliable(gAgent.getRegionHost());

		m_remAttachments.clear();
	}

	// Process wearables
	if (m_remWearables.size())
	{
		for (std::list<EWearableType>::const_iterator itWearable = m_remWearables.begin(); itWearable != m_remWearables.end(); ++itWearable)
			gAgent.removeWearable(*itWearable);

		m_remWearables.clear();
	}

	// Process gestures
	if (m_remGestures.size())
	{
		for (S32 idxGesture = 0, cntGesture = m_remGestures.count(); idxGesture < cntGesture; idxGesture++)
		{
			LLViewerInventoryItem* pItem = m_remGestures.get(idxGesture);
			gGestureManager.deactivateGesture(pItem->getUUID());
			gInventory.updateItem(pItem);
			gInventory.notifyObservers();
		}

		m_remGestures.clear();
	}
}

// Checked: 2010-02-17 (RLVa-1.1.0o) | Modified: RLVa-1.1.0o
void RlvForceWear::onWearableArrived(LLWearable* pWearable, void* pParam)
{
	#ifdef RLV_EXPERIMENTAL_COMPOSITEFOLDERS
	// If this wearable will end up replacing a currently worn one that belongs to a composite folder then we need to detach the composite
	LLViewerInventoryCategory* pFolder = NULL;
	if ( (RlvSettings::getEnableComposites()) && (pWearable) && (gAgent.getWearable(pWearable->getType())) )
	{
		// If we're just rewearing the same item we're already wearing then we're not replacing a composite folder
		LLWearableHoldingPattern* pWearData = (LLWearableHoldingPattern*)pParam; LLUUID idItem;
		for (LLWearableHoldingPattern::found_list_t::const_iterator itWearable = pWearData->mFoundList.begin();
				itWearable != pWearData->mFoundList.end(); ++itWearable)
		{
			LLFoundData* pFound = *itWearable;
			if (pWearable->getID() == pFound->mAssetID)
			{
				idItem = pFound->mItemID;
				break;
			}
		}
		if ( (idItem.notNull()) && (idItem != gAgent.getWearableItem(pWearable->getType())) &&
			 (gRlvHandler.getCompositeInfo(gAgent.getWearableItem(pWearable->getType()), NULL, &pFolder)) )
		{
			RlvForceWear rlvWear;
			rlvWear.forceFolder(pFolder, ACTION_DETACH, FLAG_DEFAULT);
			rlvWear.done();
		}
	}
	#endif // RLV_EXPERIMENTAL_COMPOSITEFOLDERS

	wear_inventory_category_on_avatar_loop(pWearable, pParam);
}

// ============================================================================
// RlvBehaviourNotifyObserver
//

void RlvBehaviourNotifyObserver::changed(const RlvCommand& rlvCmd, bool fInternal)
{
	if (fInternal)
		return;

	std::string strCmd = rlvCmd.asString(), strNotify; ERlvParamType eCmdType = rlvCmd.getParamType();
	if ( (RLV_TYPE_ADD == eCmdType) || (RLV_TYPE_REMOVE == eCmdType) )
		strNotify = llformat("/%s=%s", strCmd.c_str(), rlvCmd.getParam().c_str());
	else if (RLV_TYPE_CLEAR == eCmdType)
		strNotify = llformat("/%s", strCmd.c_str());
	else
		return;

	for (std::multimap<LLUUID, notifyData>::const_iterator itNotify = m_Notifications.begin();
			itNotify != m_Notifications.end(); ++itNotify)
	{
		if ( (itNotify->second.strFilter.empty()) || (std::string::npos != strCmd.find(itNotify->second.strFilter)) )
			rlvSendChatReply(itNotify->second.nChannel, strNotify);
	}
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

void RlvGiveToRLVAgentOffer::done()
{
	LLViewerInventoryCategory* pRlvRoot = gRlvHandler.getSharedRoot();
	LLViewerInventoryCategory* pFolder = (mCompleteFolders.size()) ? gInventory.getCategory(mCompleteFolders[0]) : NULL;
	if ( (pRlvRoot) && (pFolder) )
	{
		std::string strName = pFolder->getName();
		if (strName.find(RLV_PUTINV_PREFIX) == 0)
		{
			LLInventoryModel::update_list_t update;
			LLInventoryModel::LLCategoryUpdate updOldParent(pFolder->getParentUUID(), -1);
			update.push_back(updOldParent);
			LLInventoryModel::LLCategoryUpdate updNewParent(pRlvRoot->getUUID(), 1);
			update.push_back(updNewParent);
			gInventory.accountForUpdate(update);

			LLPointer<LLViewerInventoryCategory> pNewFolder = new LLViewerInventoryCategory(pFolder);
			pNewFolder->setParent(pRlvRoot->getUUID());
			pNewFolder->updateParentOnServer(FALSE);
			pNewFolder->rename(strName.erase(0, strName.find(RLV_FOLDER_PREFIX_PUTINV)));
			pNewFolder->updateServer(FALSE);
			gInventory.updateCategory(pNewFolder);
		}
	}

	gInventory.removeObserver(this);
	gInventory.notifyObservers();
	delete this;
}

// ============================================================================
// Various helper functions
//

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
			args["[MESSAGE]"] = llformat("RestrainedLove Support will be %s after you restart",
				(rlv_handler_t::isEnabled()) ? "disabled" : "enabled" );
			gViewerWindow->alertXml("GenericAlert", args);
		#else												// Version: 1.23.4
			LLSD args;
			args["MESSAGE"] = llformat("RestrainedLove Support will be %s after you restart",
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
