/**
 *
 * Copyright (c) 2009-2010, Kitty Barnett
 *
 * The source code in this file is provided to you under the terms of the
 * GNU General Public License, version 2.0, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE. Terms of the GPL can be found in doc/GPL-license.txt
 * in this distribution, or online at http://www.gnu.org/licenses/gpl-2.0.txt
 *
 * By copying, modifying or distributing this software, you acknowledge that
 * you have read and understood your obligations described above, and agree to
 * abide by those obligations.
 *
 */

#include "llviewerprecompiledheaders.h"
#include "llcallbacklist.h"
#include "lldrawpoolalpha.h"
#include "llfloaterbeacons.h"
#include "llfloaterchat.h"
#include "llfloaterdaycycle.h"
#include "llfloaterenvsettings.h"
#include "llfloaterland.h"
#include "llfloatermap.h"
#include "llfloaterregioninfo.h"
#include "llfloatertools.h"
#include "llfloaterwater.h"
#include "llfloaterwindlight.h"
#include "llfloaterworldmap.h"
#include "llinventoryview.h"
#include "llstartup.h"
#include "llviewermenu.h"
#include "llviewermessage.h"
#include "llviewerobjectlist.h"
#include "llviewerparcelmgr.h"
#include "llviewerregion.h"
#include "llviewerwindow.h"
#include "llworld.h"
#include "pipeline.h"

#include "rlvhandler.h"
#include "rlvextensions.h"

// Only defined in llinventorybridge.cpp
#if RLV_TARGET < RLV_MAKE_TARGET(1, 23, 0)			// Version: 1.22.11
	void confirm_replace_attachment_rez(S32 option, void* user_data);
#endif
// Only defined in llinventorymodel.cpp
extern const char* NEW_CATEGORY_NAME;

// ============================================================================
// Static variable initialization
//

BOOL RlvHandler::m_fEnabled = FALSE;
bool RlvHandler::m_fFetchStarted = false;
bool RlvHandler::m_fFetchComplete = false;
RlvMultiStringSearch RlvHandler::m_AttachLookup;
const std::string RlvHandler::cstrSharedRoot = RLV_ROOT_FOLDER;

rlv_handler_t gRlvHandler;

// ============================================================================
// Attachment group helper functions
//

// Has to match the order of ERlvAttachGroupType
const std::string cstrAttachGroups[RLV_ATTACHGROUP_COUNT] = { "head", "torso", "arms", "legs", "hud" };

// Checked: 2009-10-19 (RLVa-1.1.0e) | Added: RLVa-1.1.0e
inline ERlvAttachGroupType rlvGetAttachGroupTypeFromIndex(S32 idxGroup)
{
	switch (idxGroup)
	{
		case 0: // Right Hand
		case 1: // Right Arm
		case 3: // Left Arm
		case 4: // Left Hand
			return RLV_ATTACHGROUP_ARMS;
		case 2: // Head
			return RLV_ATTACHGROUP_HEAD;
		case 5: // Left Leg
		case 7: // Right Leg
			return RLV_ATTACHGROUP_LEGS;
		case 6: // Torso
			return RLV_ATTACHGROUP_TORSO;
		case 8: // HUD
			return RLV_ATTACHGROUP_HUD;
		default:
			return RLV_ATTACHGROUP_INVALID;
	}
}

// Checked: 2009-10-19 (RLVa-1.1.0e) | Added: RLVa-1.1.0e
inline ERlvAttachGroupType rlvGetAttachGroupTypeFromString(const std::string& strGroup)
{
	for (int idx = 0; idx < RLV_ATTACHGROUP_COUNT; idx++)
		if (cstrAttachGroups[idx] == strGroup)
			return (ERlvAttachGroupType)idx;
	return RLV_ATTACHGROUP_INVALID;
}

// ============================================================================
// Command specific helper functions
//

// Checked: 2009-08-04 (RLVa-1.0.1d) | Added: RLVa-1.0.1d
static bool rlvParseNotifyOption(const std::string& strOption, S32& nChannel, std::string& strFilter)
{
	boost_tokenizer tokens(strOption, boost::char_separator<char>(";", "", boost::keep_empty_tokens));
	boost_tokenizer::const_iterator itTok = tokens.begin();

	// Extract and sanity check the first token (required) which is the channel
	if ( (itTok == tokens.end()) || (!LLStringUtil::convertToS32(*itTok, nChannel)) || (!rlvIsValidReplyChannel(nChannel)) )
		return false;

	// Second token (optional) is the filter
	strFilter.clear();
	if (++itTok != tokens.end())
	{
		strFilter = *itTok;
		++itTok;
	}

	return (itTok == tokens.end());
}

// ============================================================================
// Constructor/destructor
//

// Checked: 2009-08-04 (RLVa-1.0.1d) | Modified: RLVa-1.0.1d
RlvHandler::RlvHandler() 
	: m_fCanCancelTp(true), m_pGCTimer(NULL), m_pWLSnapshot(NULL), m_pBhvrNotify(NULL)
{
	// Array auto-initialization to 0 is non-standard? (Compiler warning in VC-8.0)
	memset(m_LayersAdd, 0, sizeof(S16) * WT_COUNT);
	memset(m_LayersRem, 0, sizeof(S16) * WT_COUNT);
	memset(m_Behaviours, 0, sizeof(S16) * RLV_BHVR_COUNT);
}

// Checked: 2009-08-04 (RLVa-1.0.1d) | Modified: RLVa-1.0.1d
RlvHandler::~RlvHandler()
{
	//delete m_pGCTimer;	// <- deletes itself
	delete m_pWLSnapshot;	// <- delete on NULL is harmless
	delete m_pBhvrNotify;
	delete m_pAttachMgr;
}

// ============================================================================
// Attachment related functions 
//

// Checked: 2009-07-29 (RLVa-1.0.1b) | Modified: RLVa-1.0.1b
S32 RlvHandler::getAttachPointIndex(const LLInventoryCategory* pFolder, bool /*fStrict*/) const
{
	if (!pFolder)
		return 0;

	// RLVa-1.0.1 added support for legacy matching (See http://rlva.catznip.com/blog/2009/07/attachment-point-naming-convention/)
	if (RlvSettings::getEnableLegacyNaming())
		return getAttachPointIndexLegacy(pFolder);

	// Otherwise the only valid way to specify an attachment point in a folder name is: ^\.\(\s+attachpt\s+\)
	std::string::size_type idxMatch;
	std::string strAttachPt = rlvGetFirstParenthesisedText(pFolder->getName(), &idxMatch);
	LLStringUtil::trim(strAttachPt);

	return ( (1 == idxMatch) && (RLV_FOLDER_PREFIX_HIDDEN == pFolder->getName().at(0)) ) ? getAttachPointIndex(strAttachPt, true) : 0;
}

// Checked: 2009-07-29 (RLVa-1.0.1b) | Modified: RLVa-1.0.1b
S32 RlvHandler::getAttachPointIndex(const LLInventoryItem* pItem, bool fStrict) const
{
	// Sanity check - if it's not an object then it can't have an attachment point
	if ( (!pItem) || (LLAssetType::AT_OBJECT != pItem->getType()) )
		return 0;

	// The attachment point should be placed at the end of the item's name, surrounded by parenthesis
	// (if there is no such text then strAttachPt will be an empty string which is fine since it means we'll look at the item's parent)
	std::string strAttachPt = rlvGetLastParenthesisedText(pItem->getName());
	LLStringUtil::trim(strAttachPt);

	// If the item is modify   : we look at the item's name first and only then at the containing folder
	// If the item is no modify: we look at the containing folder's name first and only then at the item itself
	S32 idxAttachPt = 0;
	if (pItem->getPermissions().allowModifyBy(gAgent.getID()))
	{
		idxAttachPt = (!strAttachPt.empty()) ? getAttachPointIndex(strAttachPt, true) : 0;
		if (!idxAttachPt)
			idxAttachPt = getAttachPointIndex(gInventory.getCategory(pItem->getParentUUID()), fStrict);
	}
	else
	{
		idxAttachPt = getAttachPointIndex(gInventory.getCategory(pItem->getParentUUID()), fStrict);
		if ( (!idxAttachPt) && (!strAttachPt.empty()) )
			idxAttachPt = getAttachPointIndex(strAttachPt, true);
	}
	return idxAttachPt;
}

// Checked: 2009-07-12 (RLVa-1.0.0h) | Added: RLVa-0.2.2a
S32 RlvHandler::getAttachPointIndex(const LLViewerJointAttachment* pAttachPt) const
{
	LLVOAvatar* pAvatar = gAgent.getAvatarObject();
	if (pAvatar)
	{
		for (LLVOAvatar::attachment_map_t::const_iterator itAttach = pAvatar->mAttachmentPoints.begin(); 
				itAttach != pAvatar->mAttachmentPoints.end(); ++itAttach)
		{
			if (itAttach->second == pAttachPt)
				return itAttach->first;
		}
	}
	return 0;
}

// Checked: 2009-07-29 (RLVa-1.0.1b) | Added: RLVa-1.0.1b
S32 RlvHandler::getAttachPointIndexLegacy(const LLInventoryCategory* pFolder) const
{
	// Hopefully some day this can just be deprecated (see http://rlva.catznip.com/blog/2009/07/attachment-point-naming-convention/)
	if ( (!pFolder) || (pFolder->getName().empty()) )
		return 0;

	// Check for a (...) block *somewhere* in the name
	std::string::size_type idxMatch;
	std::string strAttachPt = rlvGetFirstParenthesisedText(pFolder->getName(), &idxMatch);
	if (!strAttachPt.empty())
	{
		// Could be "(attachpt)", ".(attachpt)" or "Folder name (attachpt)"
		if ( (0 != idxMatch) && ((1 != idxMatch) || (RLV_FOLDER_PREFIX_HIDDEN == pFolder->getName().at(0)) ) &&	// No '(' or '.(' start
			 (idxMatch + strAttachPt.length() + 1 != pFolder->getName().length()) )								// or there's extra text
		{
			// It's definitely not one of the first two so assume it's the last form (in which case we need the last paranthesised block)
			strAttachPt = rlvGetLastParenthesisedText(pFolder->getName());
		}
	}
	else
	{
		// There's no paranthesised block, but it could still be "attachpt" or ".attachpt" (just strip away the '.' from the last one)
		strAttachPt = pFolder->getName();
		if (RLV_FOLDER_PREFIX_HIDDEN == strAttachPt[0])
			strAttachPt.erase(0, 1);
	}
	return getAttachPointIndex(strAttachPt, true);
}

bool RlvHandler::hasLockedHUD() const
{
	LLVOAvatar* pAvatar = gAgent.getAvatarObject();
	if (!pAvatar)
		return false;
	
	LLViewerJointAttachment* pAttachPt;
	for (rlv_attachlock_map_t::const_iterator itAttachPt = m_AttachRem.begin(); itAttachPt != m_AttachRem.end(); ++itAttachPt)
	{
		pAttachPt = get_if_there(pAvatar->mAttachmentPoints, (S32)itAttachPt->first, (LLViewerJointAttachment*)NULL);
		if ( (pAttachPt) && (pAttachPt->getIsHUDAttachment()) )
			return true;	// At least one of our locked attachments is a HUD
	}
	return false;			// None of our locked attachments is a HUD
}

// Checked: 2009-10-13 (RLVa-1.0.5b) | Added: RLVa-1.0.5b
bool RlvHandler::isLockedAttachmentExcept(S32 idxAttachPt, ERlvLockMask eLock, LLViewerObject *pExceptObj) const
{
	if (!pExceptObj)
		return isLockedAttachment(idxAttachPt, eLock);

	// Loop over every object that marked the specific attachment point eLock type locked (but ignore pObj and any of its children)
	LLViewerObject* pTempObj;
	if (eLock & RLV_LOCK_REMOVE)
	{
		for (rlv_attachlock_map_t::const_iterator itAttach = m_AttachRem.lower_bound(idxAttachPt), 
			endAttach = m_AttachRem.upper_bound(idxAttachPt); itAttach != endAttach; ++itAttach)
		{
			if ( ((pTempObj = gObjectList.findObject(itAttach->second)) == NULL) || (pTempObj->getRootEdit()->getID() != pExceptObj->getID()) )
				return true;
		}
	}
	if (eLock & RLV_LOCK_ADD)
	{
		for (rlv_attachlock_map_t::const_iterator itAttach = m_AttachAdd.lower_bound(idxAttachPt), 
			endAttach = m_AttachAdd.upper_bound(idxAttachPt); itAttach != endAttach; ++itAttach)
		{
			if ( ((pTempObj = gObjectList.findObject(itAttach->second)) == NULL) || (pTempObj->getRootEdit()->getID() != pExceptObj->getID()) )
				return true;
		}
	}
	return false;
}

// Checked: 2009-10-10 (RLVa-1.0.5a) | Added: RLVa-1.0.5a
void RlvHandler::addAttachmentLock(S32 idxAttachPt, const LLUUID &idRlvObj, ERlvLockMask eLock)
{
	// Sanity check - make sure it's an object we know about
	if ( (m_Objects.find(idRlvObj) == m_Objects.end()) || (!idxAttachPt) )
		return;	// If (idxAttachPt) == 0 then: (pObj == NULL) || (pObj->isAttachment() == FALSE)

	// NOTE: m_AttachXXX can contain duplicate <idxAttachPt, idRlvObj> pairs (ie @detach:spine=n,detach=n from an attachment on spine)
	if (eLock & RLV_LOCK_REMOVE)
	{
		#ifdef RLV_EXPERIMENTAL_FIRSTUSE
			//LLFirstUse::useRlvDetach();
		#endif // RLV_EXPERIMENTAL_FIRSTUSE

		m_AttachRem.insert(std::pair<S32, LLUUID>(idxAttachPt, idRlvObj));
	}
	if (eLock & RLV_LOCK_ADD)
	{
		m_AttachAdd.insert(std::pair<S32, LLUUID>(idxAttachPt, idRlvObj));
	}
}

// Checked: 2010-07-18 (RLVa-1.1.2b) | Added: RLVa-1.1.2a
void RlvHandler::dumpAttachmentLocks(void*)
{
	LLVOAvatar* pAvatar = gAgent.getAvatarObject();
	if (!pAvatar)
	{
		RLV_INFOS << "No avatar object to dump attachments for" << RLV_ENDL;
		return;
	}

	RLV_INFOS << "Dumping 'remove' locks:" << RLV_ENDL;
	for (rlv_attachlock_map_t::iterator itAttachPt = gRlvHandler.m_AttachRem.begin();
			itAttachPt != gRlvHandler.m_AttachRem.end(); ++itAttachPt)
	{
		// Grab the attachment on the attachment point that's locked (if there is one)
		/*const*/ LLViewerJointAttachment* pAttachPt =
			get_if_there(pAvatar->mAttachmentPoints, (S32)itAttachPt->first, (LLViewerJointAttachment*)NULL);
		/*const*/ LLViewerObject* pAttachObj = (pAttachPt) ? pAttachPt->getObject() : NULL;
		const LLViewerInventoryItem* pAttachItem = (pAttachPt) ? gInventory.getItem(pAttachPt->getItemID()) : NULL;

		// Grab the locking attachment (if we can)
		/*const*/ LLViewerObject* pRlvObj = gObjectList.findObject(itAttachPt->second);
		/*const*/ LLViewerJointAttachment* pRlvAttachPt = (pRlvObj) ? pAvatar->getTargetAttachmentPoint(pRlvObj) : NULL;
		const LLViewerInventoryItem* pRlvItem = (pRlvAttachPt) ? gInventory.getItem(pRlvAttachPt->getItemID()) : NULL;

		std::string strMsg = llformat("'%s' on %s held by '%s' on %s",
			((pAttachItem) ? pAttachItem->getName().c_str() : ((pAttachObj) ? pAttachObj->getID().asString().c_str() : "(empty)")),
			(pAttachPt) ? pAttachPt->getName().c_str() : "(unknown)",
			((pRlvItem) ? pRlvItem->getName().c_str() : ((pRlvObj) ? pRlvObj->getID().asString().c_str() : "(empty)")),
			(pRlvAttachPt) ? pRlvAttachPt->getName().c_str() : "(unknown)");
		RLV_INFOS << strMsg << RLV_ENDL;
	}
}

// Checked: 2009-10-10 (RLVa-1.0.5a) | Added: RLVa-1.0.5a
void RlvHandler::removeAttachmentLock(S32 idxAttachPt, const LLUUID &idRlvObj, ERlvLockMask eLock)
{
	// Sanity check - make sure it's an object we know about
	if ( (m_Objects.find(idRlvObj) == m_Objects.end()) || (!idxAttachPt) )
		return;	// If (idxAttachPt) == 0 then: (pObj == NULL) || (pObj->isAttachment() == FALSE)

	if (eLock & RLV_LOCK_REMOVE)
	{
		for (rlv_attachlock_map_t::iterator itAttach = m_AttachRem.lower_bound(idxAttachPt), 
				endAttach = m_AttachRem.upper_bound(idxAttachPt); itAttach != endAttach; ++itAttach)
		{
			if (idRlvObj == itAttach->second)
			{
				m_AttachRem.erase(itAttach);
				break;
			}
		}
	}
	if (eLock & RLV_LOCK_ADD)
	{
		for (rlv_attachlock_map_t::iterator itAttach = m_AttachAdd.lower_bound(idxAttachPt), 
				endAttach = m_AttachAdd.upper_bound(idxAttachPt); itAttach != endAttach; ++itAttach)
		{
			if (idRlvObj == itAttach->second)
			{
				m_AttachAdd.erase(itAttach);
				break;
			}
		}
	}
}

#ifdef RLV_EXTENSION_FLAG_NOSTRIP
	// Checked: 2009-12-25 (RLVa-1.1.0k) | Modified: RLVa-1.1.0k
	bool RlvHandler::isStrippable(const LLUUID& idItem) const
	{
		// An item is exempt from @detach or @remoutfit if:
		//   - its name contains "nostrip" (anywhere in the name)
		//   - its parent folder contains "nostrip" (anywhere in the name)
		if (idItem.notNull())
		{
			LLViewerInventoryItem* pItem = gInventory.getItem(idItem);
			if (pItem)
			{
				if (std::string::npos != pItem->getName().find(RLV_FOLDER_FLAG_NOSTRIP))
					return false;

				LLViewerInventoryCategory* pFolder = gInventory.getCategory(pItem->getParentUUID());
				while ( (pFolder) && gAgent.getInventoryRootID() != (pFolder->getUUID()) )
				{
					if (std::string::npos != pFolder->getName().find(RLV_FOLDER_FLAG_NOSTRIP))
						return false;
					// If the item's parent is a folded folder then we need to check its parent as well
					pFolder = (isFoldedFolder(pFolder, false, true)) ? gInventory.getCategory(pFolder->getParentUUID()) : NULL;
				}
			}
		}
		return true;
	}
#endif // RLV_EXTENSION_FLAG_NOSTRIP

// ============================================================================
// Behaviour related functions
//

bool RlvHandler::hasBehaviourExcept(ERlvBehaviour eBehaviour, const std::string& strOption, const LLUUID& idObj) const
{
	for (rlv_object_map_t::const_iterator itObj = m_Objects.begin(); itObj != m_Objects.end(); ++itObj)
		if ( (idObj != itObj->second.m_UUID) && (itObj->second.hasBehaviour(eBehaviour, strOption, false)) )
			return true;
	return false;
}

// Checked: 2009-10-04 (RLVa-1.0.4c) | Modified: RLVa-1.0.4c
bool RlvHandler::isException(ERlvBehaviour eBhvr, const RlvExceptionOption& varOption, ERlvExceptionCheck typeCheck) const
{
	// We need to "strict check" exceptions only if: the restriction is actually in place *and* (isPermissive(eBhvr) == FALSE)
	if (RLV_CHECK_DEFAULT == typeCheck)
		typeCheck = ( (hasBehaviour(eBhvr)) && (!isPermissive(eBhvr)) ) ? RLV_CHECK_STRICT : RLV_CHECK_PERMISSIVE;

	std::list<LLUUID> objList;
	if (RLV_CHECK_STRICT == typeCheck)
	{
		// If we're "strict checking" then we need the UUID of every object that currently has 'eBhvr' restricted
		for (rlv_object_map_t::const_iterator itObj = m_Objects.begin(); itObj != m_Objects.end(); ++itObj)
			if (itObj->second.hasBehaviour(eBhvr, !hasBehaviour(RLV_BHVR_PERMISSIVE)))
				objList.push_back(itObj->first);
	}

	for (rlv_exception_map_t::const_iterator itException = m_Exceptions.lower_bound(eBhvr), 
			endException = m_Exceptions.upper_bound(eBhvr); itException != endException; ++itException)
	{
		if (itException->second.varOption == varOption)
		{
			// For permissive checks we just return on the very first match
			if (RLV_CHECK_PERMISSIVE == typeCheck)
				return true;

			// For strict checks we don't return until the list is empty (every object with 'eBhvr' restricted also contains the exception)
			std::list<LLUUID>::iterator itList = std::find(objList.begin(), objList.end(), itException->second.idObject);
			if (itList != objList.end())
				objList.erase(itList);
			if (objList.empty())
				return true;
		}
	}
	return false;
}

// ============================================================================
// Command processing functions
//

// Checked: 2009-11-27 (RLVa-1.1.0f) | Modified: RLVa-1.1.0f
void RlvHandler::addBehaviourObserver(RlvBehaviourObserver* pBhvrObserver)
{
	if ( (pBhvrObserver) && (std::find(m_BhvrObservers.begin(), m_BhvrObservers.end(), pBhvrObserver) == m_BhvrObservers.end()) )
		m_BhvrObservers.push_back(pBhvrObserver);
}

// Checked: 2009-11-27 (RLVa-1.1.0f) | Modified: RLVa-1.1.0f
void RlvHandler::addCommandHandler(RlvCommandHandler* pCmdHandler)
{
	if ( (pCmdHandler) && (std::find(m_CommandHandlers.begin(), m_CommandHandlers.end(), pCmdHandler) == m_CommandHandlers.end()) )
		m_CommandHandlers.push_back(pCmdHandler);
}

// Checked: 2009-11-27 (RLVa-1.1.0f) | Modified: RLVa-1.1.0f
void RlvHandler::removeBehaviourObserver(RlvBehaviourObserver* pBhvrObserver)
{
	if (pBhvrObserver)
		m_BhvrObservers.remove(pBhvrObserver);
}

// Checked: 2009-11-27 (RLVa-1.1.0f) | Modified: RLVa-1.1.0f
void RlvHandler::removeCommandHandler(RlvCommandHandler* pCmdHandler)
{
	if (pCmdHandler)
		m_CommandHandlers.remove(pCmdHandler);
}

// Checked: 2009-10-26 (RLVa-1.1.0a) | Modified: RLVa-1.1.0a
void RlvHandler::clearCommandHandlers()
{
	std::list<RlvCommandHandler*>::const_iterator itHandler = m_CommandHandlers.begin();
	while (itHandler != m_CommandHandlers.end())
	{
		delete *itHandler;
		++itHandler;
	}
	m_CommandHandlers.clear();
}

// Checked: 2009-06-03 (RLVa-0.2.0h)
void RlvHandler::notifyBehaviourObservers(const RlvCommand& rlvCmd, bool fInternal)
{
	for (std::list<RlvBehaviourObserver*>::const_iterator itBhvrObserver = m_BhvrObservers.begin();
			itBhvrObserver != m_BhvrObservers.end(); ++itBhvrObserver)
	{
		(*itBhvrObserver)->changed(rlvCmd, fInternal);
	}
}

// Checked: 2009-11-26 (RLVa-1.1.0f) | Modified: RLVa-1.1.0f
bool RlvHandler::notifyCommandHandlers(rlvCommandHandler f, const LLUUID& idObj, const RlvCommand& rlvCmd, ERlvCmdRet& eRet, bool fNotifyAll) const
{
	std::list<RlvCommandHandler*>::const_iterator itHandler = m_CommandHandlers.begin(); bool fContinue = true; eRet = RLV_RET_UNKNOWN;
	while ( (itHandler != m_CommandHandlers.end()) && ((fContinue) || (fNotifyAll)) )
	{
		ERlvCmdRet eCmdRet = RLV_RET_UNKNOWN;
		if ((fContinue = !((*itHandler)->*f)(idObj, rlvCmd, eCmdRet)) == false)
			eRet = eCmdRet;
		++itHandler;
	}
	RLV_ASSERT( (fContinue) || (eRet != RLV_RET_UNKNOWN) );
	return !fContinue;
}

// Checked: 2009-11-25 (RLVa-1.1.0f) | Modified: RLVa-1.1.0f
ERlvCmdRet RlvHandler::processCommand(const LLUUID& idObj, const RlvCommand& rlvCmd, bool fFromObj)
{
	#ifdef RLV_DEBUG
		RLV_INFOS << "[" << idObj << "]: " << rlvCmd.asString() << RLV_ENDL;
	#endif // RLV_DEBUG

	if (!rlvCmd.isValid())
	{
		#ifdef RLV_DEBUG
			RLV_INFOS << "\t-> invalid syntax" << RLV_ENDL;
		#endif // RLV_DEBUG
		return RLV_RET_FAILED_SYNTAX;
	}

	// Using a stack for executing commands solves a few problems:
	//   - if we passed RlvObject::m_UUID for idObj somewhere and process a @clear then idObj points to invalid/cleared memory at the end
	//   - if command X triggers command Y along the way then getCurrentCommand()/getCurrentObject() still return Y even when finished
	m_CurCommandStack.push(&rlvCmd); m_CurObjectStack.push(idObj);
	const LLUUID& idCurObj = m_CurObjectStack.top();

	ERlvCmdRet eRet = RLV_RET_UNKNOWN;
	switch (rlvCmd.getParamType())
	{
		case RLV_TYPE_ADD:		// Checked: 2009-11-26 (RLVa-1.1.0f) | Modified: RLVa-1.1.0f
			{
				if ( (m_Behaviours[rlvCmd.getBehaviourType()]) && 
					 ( (RLV_BHVR_SETDEBUG == rlvCmd.getBehaviourType()) || (RLV_BHVR_SETENV == rlvCmd.getBehaviourType()) ) )
				{
					// Some restrictions can only be held by one single object to avoid deadlocks
					#ifdef RLV_DEBUG
						RLV_INFOS << "\t- " << rlvCmd.getBehaviour() << " is already set by another object => discarding" << RLV_ENDL;
					#endif // RLV_DEBUG
					eRet = RLV_RET_FAILED_LOCK;
					break;
				}

				rlv_object_map_t::iterator itObj = m_Objects.find(idCurObj); bool fAdded = false;
				if (itObj != m_Objects.end())
				{
					RlvObject& rlvObj = itObj->second;
					fAdded = rlvObj.addCommand(rlvCmd);
				}
				else
				{
					RlvObject rlvObj(idCurObj);
					fAdded = rlvObj.addCommand(rlvCmd);
					m_Objects.insert(std::pair<LLUUID, RlvObject>(idCurObj, rlvObj));
				}

				#ifdef RLV_DEBUG
					RLV_INFOS << "\t- " << ( (fAdded) ? "adding behaviour" : "skipping duplicate" ) << RLV_ENDL;
				#endif // RLV_DEBUG

				if (fAdded) {	// If FALSE then this was a duplicate, there's no need to handle those
					if (!m_pGCTimer)
						m_pGCTimer = new RlvGCTimer();
					eRet = processAddRemCommand(idCurObj, rlvCmd);
					notifyBehaviourObservers(rlvCmd, !fFromObj);
				}
				else
				{
					eRet = RLV_RET_SUCCESS_DUPLICATE;
				}
			}
			break;
		case RLV_TYPE_REMOVE:		// Checked: 2009-11-26 (RLVa-1.1.0f) | Modified: RLVa-1.1.0f
			{
				rlv_object_map_t::iterator itObj = m_Objects.find(idCurObj); bool fRemoved = false;
				if (itObj != m_Objects.end())
					fRemoved = itObj->second.removeCommand(rlvCmd);

				#ifdef RLV_DEBUG
					RLV_INFOS << "\t- " << ( (fRemoved)	? "removing behaviour"
														: "skipping remove (unset behaviour or unknown object)") << RLV_ENDL;
				#endif // RLV_DEBUG

				if (fRemoved) {	// Don't handle non-sensical removes
					eRet = processAddRemCommand(idCurObj, rlvCmd);
					notifyBehaviourObservers(rlvCmd, !fFromObj);

					if (0 == itObj->second.m_Commands.size())
					{
						#ifdef RLV_DEBUG
							RLV_INFOS << "\t- command list empty => removing " << idCurObj << RLV_ENDL;
						#endif // RLV_DEBUG
						m_Objects.erase(itObj);
					}
				}
				else
				{
					eRet = RLV_RET_SUCCESS_UNSET;
				}
			}
			break;
		case RLV_TYPE_CLEAR:		// Checked: 2009-11-25 (RLVa-1.1.0f) | Modified: RLVa-1.1.0f
			eRet = processClearCommand(idCurObj, rlvCmd);
			notifyBehaviourObservers(rlvCmd, !fFromObj);
			break;
		case RLV_TYPE_FORCE:		// Checked: 2009-11-26 (RLVa-1.1.0f) | Modified: RLVa-1.1.0f
			eRet = processForceCommand(idCurObj, rlvCmd);
			break;
		case RLV_TYPE_REPLY:		// Checked: 2009-11-25 (RLVa-1.1.0f) | Modified: RLVa-1.1.0f
			eRet = processReplyCommand(idCurObj, rlvCmd);
			break;
		case RLV_TYPE_UNKNOWN:		// Checked: 2009-11-25 (RLVa-1.1.0f) | Modified: RLVa-1.1.0f
		default:
			eRet = RLV_RET_FAILED_PARAM;
			break;
	}
	RLV_ASSERT(RLV_RET_UNKNOWN != eRet);

	#ifdef RLV_DEBUG
		RLV_INFOS << "\t--> command " << ((eRet & RLV_RET_SUCCESS) ? "succeeded" : "failed") << RLV_ENDL;
	#endif // RLV_DEBUG

	m_CurCommandStack.pop(); m_CurObjectStack.pop();
	return eRet;
}

// Checked: 2009-11-25 (RLVa-1.1.0f) | Modified: RLVa-1.1.0f
void RlvHandler::processRetainedCommands(ERlvBehaviour eBhvrFilter /*=RLV_BHVR_UNKNOWN*/, ERlvParamType eTypeFilter /*=RLV_TYPE_UNKNOWN*/)
{
	rlv_retained_list_t::iterator itCmd = m_Retained.begin(), itCurCmd;
	while (itCmd != m_Retained.end())
	{
		itCurCmd = itCmd++;  // Point the loop iterator ahead

		const RlvRetainedCommand& cmd = *itCurCmd;
		if ( ((RLV_BHVR_UNKNOWN == eBhvrFilter) || (cmd.rlvCmd.getBehaviourType() == eBhvrFilter)) &&
		     ((RLV_TYPE_UNKNOWN == eTypeFilter) || (cmd.rlvCmd.getParamType() == eTypeFilter)) )
		{
			processCommand(cmd.idObject, cmd.rlvCmd, true);
			m_Retained.erase(itCurCmd);
		}
	}
}

ERlvCmdRet RlvHandler::processClearCommand(const LLUUID& idObj, const RlvCommand& rlvCmd)
{
	const std::string& strFilter = rlvCmd.getParam(); std::string strCmdRem;

	rlv_object_map_t::const_iterator itObj = m_Objects.find(idObj);
	if (itObj != m_Objects.end())	// No sense in clearing if we don't have any commands for this object
	{
		const RlvObject& rlvObj = itObj->second; bool fContinue = true;
		for (rlv_command_list_t::const_iterator itCmd = rlvObj.m_Commands.begin(), itCurCmd;
				((fContinue) && (itCmd != rlvObj.m_Commands.end())); )
		{
			itCurCmd = itCmd++;		// Point itCmd ahead so it won't get invalidated if/when we erase a command

			const RlvCommand& rlvCmdRem = *itCurCmd; strCmdRem = rlvCmdRem.asString();
			if ( (strFilter.empty()) || (std::string::npos != strCmdRem.find(strFilter)) )
			{
				fContinue = (rlvObj.m_Commands.size() > 1); // rlvObj will become invalid once we remove the last command
				processCommand(idObj, strCmdRem.append("=y"), false);
			}
		}
	}

	// Let our observers know about clear commands
	ERlvCmdRet eRet = RLV_RET_SUCCESS;
	notifyCommandHandlers(&RlvCommandHandler::onClearCommand, idObj, rlvCmd, eRet, true);

	return RLV_RET_SUCCESS; // Don't fail clear commands even if the object didn't exist since it confuses people
}

// ============================================================================
// House keeping (see debug notes for test methodology, test script and last run)
//

void RlvHandler::initLookupTables()
{
	static bool fInitialized = false;
	if (!fInitialized)
	{
		// Initialize the attachment name lookup table
		LLVOAvatar* pAvatar = gAgent.getAvatarObject();
		if (pAvatar)
		{
			std::string strAttachPtName;
			for (LLVOAvatar::attachment_map_t::const_iterator itAttach = pAvatar->mAttachmentPoints.begin(); 
					 itAttach != pAvatar->mAttachmentPoints.end(); ++itAttach)
			{
				LLViewerJointAttachment* pAttachPt = itAttach->second;
				if (pAttachPt)
				{
					strAttachPtName = pAttachPt->getName();
					LLStringUtil::toLower(strAttachPtName);
					m_AttachLookup.addKeyword(strAttachPtName, itAttach->first);
				}
			}
			fInitialized = true;
		}
	}
}

// Checked: 2010-07-18 (RLVa-1.1.2b) | Modified: RLVa-1.1.2a
void RlvHandler::onAttach(LLViewerJointAttachment* pAttachPt)
{
	// Sanity check - LLVOAvatar::attachObject() should call us *after* calling LLViewerJointAttachment::addObject()
	LLViewerObject* pObj = pAttachPt->getObject();
	S32 idxAttachPt = getAttachPointIndex(pObj);	 // getAttachPointIndex() has a NULL pointer check so this is safe
	if ( (!pObj) || (!idxAttachPt) )
	{
		RLV_ERRS << "pAttachPt->getObject() == NULL" << LL_ENDL;
		return;
	}

	// Let the attachment manager know
	m_pAttachMgr->onAttach(pAttachPt);

	// Check if we already have an RlvObject instance for this object (rezzed prim attached from in-world, or an attachment that rezzed in)
	rlv_object_map_t::iterator itObj = m_Objects.find(pObj->getID());
	if (itObj != m_Objects.end())
	{
		// Only if we haven't been able to find this object (= attachment that rezzed in) or if it's a rezzed prim attached from in-world
		if ( (!itObj->second.m_fLookup) || (!itObj->second.m_idxAttachPt) )
		{
			// Reset any lookup information we might have for this object
			itObj->second.m_idxAttachPt = idxAttachPt;
			itObj->second.m_fLookup = true;

			// In both cases we should check for "@detach=n" and actually lock down the attachment point it got attached to
			if (itObj->second.hasBehaviour(RLV_BHVR_DETACH, false))
			{
				// (Copy/paste from processAddCommand)
				addAttachmentLock(idxAttachPt, itObj->second.m_UUID, RLV_LOCK_REMOVE);
				if (pObj->isHUDAttachment())
					LLPipeline::sShowHUDAttachments = TRUE;	// Prevents hiding of locked HUD attachments
			}
		}
	}

	// Fetch the inventory item if it isn't already (we need it for a potential reattach-on-detach)
	const LLUUID& idItem = pAttachPt->getItemID();
	LLViewerInventoryItem* pItem = ( (idItem.notNull()) && (gInventory.isInventoryUsable()) ) ? gInventory.getItem(idItem) : NULL;
	if ( (STATE_STARTED == LLStartUp::getStartupState()) && (pItem != NULL) )
	{
		RlvCurrentlyWorn f;
		f.fetchItem(idItem);
	}

	// If what we're wearing is located under the shared root then append the attachment point name as needed
	LLViewerInventoryCategory* pRlvRoot = getSharedRoot();
	if ( (!RlvSettings::getEnableSharedWear()) && (RlvSettings::getSharedInvAutoRename()) &&
	     (STATE_STARTED == LLStartUp::getStartupState()) && (pRlvRoot) && (pItem) && (pItem->isComplete()) &&
		 (gInventory.isObjectDescendentOf(idItem, pRlvRoot->getUUID())) )
	{
		// TODO: find a not too convoluted way to rename the attachment in case it specifies a name different than the current attach point
		S32 idxAttachPtItem = getAttachPointIndex(pItem, true);
		if ( (idxAttachPt != idxAttachPtItem) && (!idxAttachPtItem) )
		{
			std::string strAttachPt = pAttachPt->getName();
			LLStringUtil::toLower(strAttachPt);

			// If we can modify the item then we rename it directly, otherwise we create a new folder and move it
			if (pItem->getPermissions().allowModifyBy(gAgent.getID()))
			{
				std::string strName = pItem->getName();
				LLStringUtil::truncate(strName, DB_INV_ITEM_NAME_STR_LEN - strAttachPt.length() - 3);

				strName += " (" + strAttachPt + ")";

				pItem->rename(strName);
				pItem->updateServer(FALSE);
				gInventory.updateItem(pItem);
				//gInventory.notifyObservers(); <- done further down in LLVOAvatar::attachObject()
			}
			else
			{
				// Don't do anything if the item is a direct descendant of the shared root, or a folded folder
				LLViewerInventoryCategory* pFolder = gInventory.getCategory(pItem->getParentUUID());
				if ( (pFolder) && (pFolder->getUUID() != pRlvRoot->getUUID()) && (!isFoldedFolder(pFolder, true, false)) )
				{
					std::string strFolderName = ".(" + strAttachPt + ")";

					// Rename the item's parent folder if it's called "New Folder", isn't directly under #RLV and contains exactly 1 object
					if ( (NEW_CATEGORY_NAME == pFolder->getName()) && (pFolder->getParentUUID() != pRlvRoot->getUUID()) &&
						 (1 == rlvGetDirectDescendentsCount(pFolder, LLAssetType::AT_OBJECT)) )
					{
						pFolder->rename(strFolderName);
						pFolder->updateServer(FALSE);
						gInventory.updateCategory(pFolder);
						//gInventory.notifyObservers(); <- done further down in LLVOAvatar::attachObject()
					}
					else
					{
						// "No modify" item with a non-renameable parent: create a new folder named and move the item into it
						LLUUID idAttachFolder = gInventory.createNewCategory(pFolder->getUUID(), LLAssetType::AT_NONE, strFolderName);
						move_inventory_item(gAgent.getID(), gAgent.getSessionID(), pItem->getUUID(), idAttachFolder, std::string(), NULL);
						//gInventory.notifyObservers(); <- done further down in LLVOAvatar::attachObject()
					}
				}
			}
		}
	}
}

// Checked: 2009-05-31 (RLVa-0.2.0e) | Modified: RLVa-0.2.0e
void RlvHandler::onDetach(LLViewerJointAttachment* pAttachPt)
{
	LLViewerObject* pObj = pAttachPt->getObject();
	if (!pObj)
	{
		// LLVOAvatar::detachObject() should call us *before* calling LLViewerJointAttachment::removeObject()
		RLV_ERRS << "pAttachPt->getObject() == NULL" << LL_ENDL;
		return;
	}
	S32 idxAttachPt = getAttachPointIndex(pObj);
	if (0 == idxAttachPt)
	{
		// If we ended up here then the user "Drop"'ed this attachment (which we can't recover from)
		return;
	}

	#ifdef RLV_DEBUG
		// TODO-RLV: when we're exiting (for whatever reason) app state won't always reflect it but 
		//           gAgent.getAvatarObject()->mAttachmentPoints will be NULL so anywhere we use
		//			 "get_if_there" will call through a NULL pointer. One case is "idling out" -> test the rest
		//LLViewerJointAttachment* pDbgAttachmentPt = 
		//	get_if_there(gAgent.getAvatarObject()->mAttachmentPoints, (S32)idxAttachPt, (LLViewerJointAttachment*)NULL);
		//RLV_INFOS << "Clean up for '" << pDbgAttachmentPt->getName() << "'" << LL_ENDL;
	#endif // RLV_DEBUG

	// Let the attachment manager know
	m_pAttachMgr->onDetach(pAttachPt);

	// Clean up any restriction this object (or one of its child prims) may have
	rlv_object_map_t::iterator itObj = m_Objects.begin(), itCurrent;
	while (itObj != m_Objects.end())
	{
		itCurrent = itObj++; // @clear will invalidate our iterator so point it ahead now

		// NOTE: ObjectKill seems to happen in reverse (child prims are killed before the root is) so we can't use gObjectList here
		if (itCurrent->second.m_idxAttachPt == idxAttachPt)
			processCommand(itCurrent->second.m_UUID, "clear", true);
	}
}

// Checked: 2009-07-30 (RLVa-1.0.1c) | Modified: RLVa-1.0.1c
bool RlvHandler::onGC()
{
	// We can't issue @clear on an object while we're in the loop below since that would invalidate our iterator
	// (and starting over would mean that some objects might get their "lookup misses" counter updated more than once per GC run)
	std::list<LLUUID> ExpiredObjects;

	for (rlv_object_map_t::iterator itObj = m_Objects.begin(); itObj != m_Objects.end(); ++itObj)
	{
		LLViewerObject* pObj = gObjectList.findObject(itObj->second.m_UUID);
		if (!pObj)
		{
			// If the RlvObject once existed in the gObjectList and now doesn't then expire it right now
			// If the RlvObject never existed in the gObjectList and still doesn't then increase its "lookup misses" counter
			// but if that reaches 20 (we run every 30 seconds so that's about 10 minutes) then we'll expire it too
			if ( (itObj->second.m_fLookup) || (++itObj->second.m_nLookupMisses > 20) )
				ExpiredObjects.push_back(itObj->first);
		}
		else
		{
			// Check if this is an RlvObject instance who's object never existed in gObjectList before (rezzed prim in-world)
			// (it could also be an attachment that only just rezzed in but RlvHandler::onAttach() should be handling those)
			if ( (!itObj->second.m_fLookup) && (!pObj->isAttachment()) )
				itObj->second.m_fLookup = true;
		}
	}

	for (std::list<LLUUID>::const_iterator itExpired = ExpiredObjects.begin(); itExpired != ExpiredObjects.end(); ++itExpired)
	{
		#ifdef RLV_DEBUG
			RLV_INFOS << "Garbage collecting " << *itExpired << LL_ENDL;
		#endif // RLV_DEBUG

		processCommand(*itExpired, "clear", true);
	}

	return (0 != m_Objects.size());	// GC will kill itself if it has nothing to do
}

// Checked: 2009-11-26 (RLVa-1.1.0f) | Added: RLVa-1.1.0f
void RlvHandler::onIdleStartup(void* pParam)
{
	LLTimer* pTimer = (LLTimer*)pParam;
	if (LLStartUp::getStartupState() < STATE_STARTED)
	{
		// We don't want to run this *too* often
		if ( (LLStartUp::getStartupState() >= STATE_MISC) && (pTimer->getElapsedTimeF32() >= 2.0) )
		{
			gRlvHandler.processRetainedCommands(RLV_BHVR_VERSION, RLV_TYPE_REPLY);
			gRlvHandler.processRetainedCommands(RLV_BHVR_VERSIONNUM, RLV_TYPE_REPLY);
			pTimer->reset();
		}
	}
	else
	{
		// Clean-up
		gIdleCallbacks.deleteFunction(onIdleStartup, pParam);
		delete pTimer;
	}
}

// ============================================================================
// String/chat censoring functions
//

// LL must have included an strlen for UTF8 *somewhere* but I can't seem to find it so this one is home grown
size_t utf8str_strlen(const std::string& utf8)
{
	const char* pUTF8 = utf8.c_str(); size_t length = 0;
	for (int idx = 0, cnt = utf8.length(); idx < cnt ;idx++)
	{
		// We're looking for characters that don't start with 10 as their high bits
		if ((pUTF8[idx] & 0xC0) != 0x80)
			length++;
	}
	return length;
}

// TODO-RLV: works, but more testing won't hurt
std::string utf8str_chtruncate(const std::string& utf8, size_t length)
{
	if (0 == length)
		return std::string();
	if (utf8.length() <= length)
		return utf8;

	const char* pUTF8 = utf8.c_str(); int idx = 0;
	while ( (pUTF8[idx]) && (length > 0) )
	{
		// We're looking for characters that don't start with 10 as their high bits
		if ((pUTF8[idx] & 0xC0) != 0x80)
			length--;
		idx++;
	}

	return utf8.substr(0, idx);
}

// Checked: 2009-07-09 (RLVa-1.0.0f) | Modified: RLVa-1.0.0f
void RlvHandler::filterChat(std::string& strUTF8Text, bool fFilterEmote) const
{
	if (strUTF8Text.empty())
		return;

	if (rlvIsEmote(strUTF8Text))					// Check if it's an emote
	{
		if (fFilterEmote)							// Emote filtering depends on fFilterEmote
		{
			if ( (strUTF8Text.find_first_of("\"()*=^_?~") != -1) || 
				 (strUTF8Text.find(" -") != -1) || (strUTF8Text.find("- ") != -1) || (strUTF8Text.find("''") != -1) )
			{
				strUTF8Text = "...";				// Emote contains illegal character (or character sequence)
			}
			else if (!hasBehaviour(RLV_BHVR_EMOTE))
			{
				int idx = strUTF8Text.find('.');	// Truncate at 20 characters or at the dot (whichever is shorter)
				strUTF8Text = utf8str_chtruncate(strUTF8Text, ( (idx > 0) && (idx < 20) ) ? idx + 1 : 20);
			}
		}
	} 
	else if (strUTF8Text[0] == '/')					// Not an emote, but starts with a '/'
	{
		if (utf8str_strlen(strUTF8Text) > 7)		// Allow as long if it's 6 characters or less
			strUTF8Text = "...";
	}
	else if ((strUTF8Text.length() < 4) || (strUTF8Text.compare(0, 2, "((")) || (strUTF8Text.compare(strUTF8Text.length() - 2, 2, "))")))
	{
		strUTF8Text = "...";						// Regular chat (not OOC)
	}
}

// Checked: 2009-07-04 (RLVa-1.0.0a) | Modified: RLVa-1.0.0a
void RlvHandler::filterLocation(std::string& strUTF8Text) const
{
	// TODO-RLVa: if either the region or parcel name is a simple word such as "a" or "the" then confusion will ensue?
	//            -> not sure how you would go about preventing this though :|...

	// Filter any mention of the surrounding region names
	LLWorld::region_list_t regions = LLWorld::getInstance()->getRegionList();
	const std::string& strHiddenRegion = RlvStrings::getString(RLV_STRING_HIDDEN_REGION);
	for (LLWorld::region_list_t::const_iterator itRegion = regions.begin(); itRegion != regions.end(); ++itRegion)
		rlvStringReplace(strUTF8Text, (*itRegion)->getName(), strHiddenRegion);

	// Filter any mention of the parcel name
	LLViewerParcelMgr* pParcelMgr = LLViewerParcelMgr::getInstance();
	if (pParcelMgr)
		rlvStringReplace(strUTF8Text, pParcelMgr->getAgentParcelName(), RlvStrings::getString(RLV_STRING_HIDDEN_PARCEL));
}

void RlvHandler::filterNames(std::string& strUTF8Text) const
{
	std::string strFirstName, strLastName, strName;

	// TODO-RLV: make this a bit more efficient (ie people with a large draw distance will have a load of active regions)
	//   -> the cost of multi string matching them all at once seems to be about the same as calling rlvStringReplace 
	//      twice so that would be a tremendous gain (and we'd get first name and word matching for free)
	#if RLV_TARGET < RLV_MAKE_TARGET(1, 23, 0)			// Version: 1.22.11
		for (LLWorld::region_list_t::const_iterator itRegion = LLWorld::getInstance()->mActiveRegionList.begin();
			 itRegion != LLWorld::getInstance()->mActiveRegionList.end(); ++itRegion)
		{
			LLViewerRegion* pRegion = *itRegion;
			
			for (S32 idxAgent = 0, cntAgent = pRegion->mMapAvatars.count(); idxAgent < cntAgent; idxAgent++)
			{
				// LLCacheName::getName() will add the UUID to the lookup queue if we don't know it yet
				if (gCacheName->getName(pRegion->mMapAvatarIDs.get(idxAgent), strFirstName, strLastName))
				{
					strName = strFirstName + " " + strLastName;

					rlvStringReplace(strUTF8Text, strName, getAnonym(strName));
				}
			}
		}
	#else												// Version: trunk
		// TODO-RLV: should restrict this to a certain radius (probably 1-2 sim range?)
		std::vector<LLUUID> idAgents;
		LLWorld::getInstance()->getAvatars(&idAgents, NULL);

		for (int idxAgent = 0, cntAgent = idAgents.size(); idxAgent < cntAgent; idxAgent++)
		{
			// LLCacheName::getName() will add the UUID to the lookup queue if we don't know it yet
			if (gCacheName->getName(idAgents[idxAgent], strFirstName, strLastName))
			{
				strName = strFirstName + " " + strLastName;

				rlvStringReplace(strUTF8Text, strName, RlvStrings::getAnonym(strName));
			}
		}
	#endif
}

// Checked: 2010-02-27 (RLVa-1.1.1a) | Modified: RLVa-1.2.0a
bool RlvHandler::redirectChatOrEmote(const std::string& strUTF8Text) const
{
	// Sanity check - @redirchat only for chat and @rediremote only for emotes
	ERlvBehaviour eBhvr = (!rlvIsEmote(strUTF8Text)) ? RLV_BHVR_REDIRCHAT : RLV_BHVR_REDIREMOTE;
	if (!hasBehaviour(eBhvr))
		return false;

	if (RLV_BHVR_REDIRCHAT == eBhvr)
	{
		std::string strText = strUTF8Text;
		filterChat(strText, false);
		if (strText != "...")
			return false;	// @sendchat wouldn't filter it so @redirchat won't redirect it either
	}

	for (rlv_exception_map_t::const_iterator itRedir = m_Exceptions.lower_bound(eBhvr),
			endRedir = m_Exceptions.upper_bound(eBhvr); itRedir != endRedir; ++itRedir)
	{
		S32 nChannel = boost::get<S32>(itRedir->second.varOption);
		if ( (!hasBehaviour(RLV_BHVR_SENDCHANNEL)) || (isException(RLV_BHVR_SENDCHANNEL, nChannel)) )
			rlvSendChatReply(nChannel, strUTF8Text);
	}

	return true;
}

// ============================================================================
// Public service functions (called by the outside world or by extension handlers)
//

// Checked: 2009-11-24 (RLVa-1.1.0e)
bool RlvHandler::isAgentNearby(const LLUUID& idAgent) const
{
	#if RLV_TARGET < RLV_MAKE_TARGET(1, 23, 0)			// Version: 1.22.11
		for (LLWorld::region_list_t::const_iterator itRegion = LLWorld::getInstance()->mActiveRegionList.begin();
			 itRegion != LLWorld::getInstance()->mActiveRegionList.end(); ++itRegion)
		{
			LLViewerRegion* pRegion = *itRegion;
			
			for (S32 idxAgent = 0, cntAgent = pRegion->mMapAvatars.count(); idxAgent < cntAgent; idxAgent++)
				if (pRegion->mMapAvatarIDs.get(idxAgent) == uuid)
					return true;
		}
	#else												// Version: 1.23.4
		std::vector<LLUUID> idAgents;
		LLWorld::getInstance()->getAvatars(&idAgents, NULL);

		for (int idxAgent = 0, cntAgent = idAgents.size(); idxAgent < cntAgent; idxAgent++)
		{
			if (idAgents[idxAgent] == idAgent)
				return true;
		}
	#endif
	return false;
}

// ============================================================================
// General purpose inventory functions
//

// Checked: 2009-07-12 (RLVa-1.0.0h)
class RlvSharedRootFetcher : public LLInventoryFetchDescendentsObserver
{
public:
	RlvSharedRootFetcher() {}

	virtual void done()
	{
		RLV_INFOS << "Shared folders fetch completed" << LL_ENDL;
		RlvHandler::m_fFetchComplete = true;

		gInventory.removeObserver(this);
		delete this;
	}
};

// Checked: 2009-07-12 (RLVa-1.0.0h)
void RlvHandler::fetchSharedInventory()
{
	// Sanity check - don't fetch if we're already fetching, or if we don't have a shared root
	LLViewerInventoryCategory* pRlvRoot = getSharedRoot();
	if ( (m_fFetchStarted) || (!pRlvRoot) )
		return;

	// Grab all the folders under the shared root
	LLInventoryModel::cat_array_t folders;
	LLInventoryModel::item_array_t items;
	gInventory.collectDescendents(pRlvRoot->getUUID(), folders, items, FALSE);

	// Add them to the "to fetch" list
	LLInventoryFetchDescendentsObserver::folder_ref_t fetchFolders;
	fetchFolders.push_back(pRlvRoot->getUUID());
	for (S32 idxFolder = 0, cntFolder = folders.count(); idxFolder < cntFolder; idxFolder++)
		fetchFolders.push_back(folders.get(idxFolder)->getUUID());

	// Now fetch them all in one go
	RlvSharedRootFetcher* fetcher = new RlvSharedRootFetcher;

	RLV_INFOS << "Starting fetch of " << fetchFolders.size() << " shared folders" << RLV_ENDL;
	fetcher->fetchDescendents(fetchFolders);
	m_fFetchStarted = true;

	if (fetcher->isEverythingComplete())
		fetcher->done();
	else
		gInventory.addObserver(fetcher);
}

bool RlvHandler::findSharedFolders(const std::string& strCriteria, LLInventoryModel::cat_array_t& folders) const
{
	// Sanity check - can't do anything without a shared root
	LLViewerInventoryCategory* pRlvRoot = getSharedRoot();
	if (!pRlvRoot)
		return false;

	folders.clear();
	LLInventoryModel::item_array_t items;
	RlvCriteriaCategoryCollector functor(strCriteria);
	gInventory.collectDescendentsIf(pRlvRoot->getUUID(), folders, items, FALSE, functor);

	return (folders.count() != 0);
}

// Checked: 2009-07-12 (RLVa-1.0.0h) | Modified: RLVa-0.2.0e
LLViewerInventoryCategory* RlvHandler::getSharedRoot()
{
	if (gInventory.isInventoryUsable())
	{
		LLInventoryModel::cat_array_t* pFolders;
		LLInventoryModel::item_array_t* pItems;
		gInventory.getDirectDescendentsOf(gAgent.getInventoryRootID(), pFolders, pItems);
		if (pFolders)
		{
			// NOTE: we might have multiple #RLV folders so we'll just go with the first one we come across
			LLViewerInventoryCategory* pFolder;
			for (S32 idxFolder = 0, cntFolder = pFolders->count(); idxFolder < cntFolder; idxFolder++)
			{
				if ( ((pFolder = pFolders->get(idxFolder)) != NULL) && (RlvHandler::cstrSharedRoot == pFolder->getName()) )
					return pFolder;
			}
		}
	}
	return NULL;
}

// Checked: 2009-07-28 (RLVa-1.0.1a) | Modified: RLVa-1.0.1a
LLViewerInventoryCategory* RlvHandler::getSharedFolder(const LLUUID& idParent, const std::string& strFolderName) const
{
	LLInventoryModel::cat_array_t*  pFolders;
	LLInventoryModel::item_array_t* pItems;
	gInventory.getDirectDescendentsOf(idParent, pFolders, pItems);
	if ( (!pFolders) || (strFolderName.empty()) )
		return NULL;

	// If we can't find an exact match then we'll settle for a "contains" match
	LLViewerInventoryCategory* pPartial = NULL;

	//LLStringUtil::toLower(strFolderName); <- everything was already converted to lower case before

	std::string strName;
	for (S32 idxFolder = 0, cntFolder = pFolders->count(); idxFolder < cntFolder; idxFolder++)
	{
		LLViewerInventoryCategory* pFolder = pFolders->get(idxFolder);

		strName = pFolder->getName();
		if (strName.empty())
			continue;
		LLStringUtil::toLower(strName);

		if (strFolderName == strName)
			return pFolder;		// Found an exact match, no need to keep on going
		else if ( (!pPartial) && (RLV_FOLDER_PREFIX_HIDDEN != strName[0]) && (strName.find(strFolderName) != std::string::npos) )
			pPartial = pFolder;	// Found a partial (non-hidden) match, but we might still find an exact one (first partial match wins)
	}

	return pPartial;
}

// Checked: 2009-07-12 (RLVa-1.0.0h) | Modified: RLVa-0.2.0e
LLViewerInventoryCategory* RlvHandler::getSharedFolder(const std::string& strPath) const
{
	// Sanity check - no shared root => no shared folder
	LLViewerInventoryCategory* pRlvRoot = getSharedRoot(), *pFolder = pRlvRoot;
	if (!pRlvRoot)
		return NULL;

	// Walk the path (starting at the root)
	boost_tokenizer tokens(strPath, boost::char_separator<char>("/", "", boost::drop_empty_tokens));
	for (boost_tokenizer::const_iterator itToken = tokens.begin(); itToken != tokens.end(); ++itToken)
	{
		pFolder = getSharedFolder(pFolder->getUUID(), *itToken);
		if (!pFolder)
			return NULL;	// No such folder
	}

	return pFolder;			// If strPath was empty or just a bunch of //// then: pFolder == pRlvRoot
}

// Checked: 2009-07-12 (RLVa-1.0.0h) | Modified: RLVa-0.2.0g
std::string RlvHandler::getSharedPath(const LLViewerInventoryCategory* pFolder) const
{
	LLViewerInventoryCategory* pRlvRoot = getSharedRoot();
	// Sanity check - no shared root or no folder => no path
	if ( (!pRlvRoot) || (!pFolder) || (pRlvRoot->getUUID() == pFolder->getUUID()) )
		return std::string();

	const LLUUID& idRLV  = pRlvRoot->getUUID();
	const LLUUID& idRoot = gAgent.getInventoryRootID();
	std::string strPath;

	// Walk up the tree until we reach the top
	while (pFolder)
	{
		strPath = "/" + pFolder->getName() + strPath;

		const LLUUID& idParent = pFolder->getParentUUID();
		if (idRLV == idParent)			// Reached the shared root, we're done
			break;
		else if (idRoot == idParent)	// We reached the agent's inventory root (indicative of a logic error elsewhere)
		{
			RLV_ERRS << "Reached agent's inventory root while building path for shared folder" << LL_ENDL;
			return std::string();
		}
		else
			pFolder = gInventory.getCategory(idParent);
	}

	return strPath.erase(0, 1);
}

// ============================================================================
// Composite folders
//

#ifdef RLV_EXPERIMENTAL_COMPOSITEFOLDERS
	// Checked: 2009-12-18 (RLVa-1.1.0k) | Modified: RLVa-1.1.0i
	bool RlvHandler::getCompositeInfo(const LLInventoryCategory* pFolder, std::string* pstrName) const
	{
		if (pFolder)
		{
			// Composite folder naming: ^\.?[Folder]
			const std::string& cstrFolder = pFolder->getName();
			std::string::size_type idxStart = cstrFolder.find('['), idxEnd = cstrFolder.find(']', idxStart);
			if ( ((0 == idxStart) || (1 == idxStart)) && (idxEnd - idxStart > 1) )
			{
				if (pstrName)
					pstrName->assign(cstrFolder.substr(idxStart + 1, idxEnd - idxStart - 1));
				return true;
			}
		}
		return false;
	}

	// Checked: 2009-12-18 (RLVa-1.1.0k) | Modified: RLVa-1.1.0i
	bool RlvHandler::getCompositeInfo(const LLUUID& idItem, std::string* pstrName, LLViewerInventoryCategory** ppFolder) const
	{
		LLViewerInventoryCategory* pRlvRoot; LLViewerInventoryItem* pItem;
		if ( (idItem.notNull()) && ((pRlvRoot = getSharedRoot()) != NULL) && 
			 (gInventory.isObjectDescendentOf(idItem, pRlvRoot->getUUID())) && ((pItem = gInventory.getItem(idItem)) != NULL) )
		{
			// We know it's an item in a folder under the shared root (we need its parent if it's a folded folder)
			LLViewerInventoryCategory* pFolder = gInventory.getCategory(pItem->getParentUUID());
			if (isFoldedFolder(pFolder, true, false))	// Don't check if the folder is a composite folder
				pFolder = gInventory.getCategory(pFolder->getParentUUID());

			if ( (pFolder) && (getCompositeInfo(pFolder, pstrName)) )
			{
				if (ppFolder)
					*ppFolder = pFolder;
				return true;
			}
		}
		return false;
	}
#endif // RLV_EXPERIMENTAL_COMPOSITEFOLDERS

#ifdef RLV_EXPERIMENTAL_COMPOSITE_FOLDING
	// Checked:
	inline bool RlvHandler::isHiddenCompositeItem(const LLUUID& idItem, const std::string& cstrItemType) const
	{
		// An item that's part of a composite folder will be hidden from @getoutfit and @getattach if:
		//   (1) the composite name specifies either a wearable layer or an attachment point
		//   (2) the specified wearable layer or attachment point is worn and resides in the folder
		//   (3) cstrItemType isn't the specified wearable layer or attach point
		//
		// Example: #RLV/Separates/Shoes/ChiChi Pumps/.[shoes] with items: "Shoe Base", "Shoe (left foot)" and "Shoe (right foot)"
		//   -> as long as "Shoe Base" is worn, @getattach should not reflect "left foot", nor "right foot"
		std::string strComposite; LLViewerInventoryCategory* pFolder;
		EWearableType type; S32 idxAttachPt;
		if ( (getCompositeInfo(idItem, &strComposite, &pFolder)) && (cstrItemType != strComposite) )
		{
			LLUUID idCompositeItem;
			if ((type = LLWearable::typeNameToType(strComposite)) != WT_INVALID)
			{
				idCompositeItem = gAgent.getWearableItem(type);
			}
			else if ((idxAttachPt = getAttachPointIndex(strComposite, true)) != 0)
			{
				LLVOAvatar* pAvatar; LLViewerJointAttachment* pAttachmentPt;
				if ( ((pAvatar = gAgent.getAvatarObject()) != NULL) && 
					 ((pAttachmentPt = get_if_there(pAvatar->mAttachmentPoints, idxAttachPt, (LLViewerJointAttachment*)NULL)) != NULL) )
				{
					idCompositeItem = pAttachmentPt->getItemID();
				}
			}

			if ( (idCompositeItem.notNull()) && (gInventory.isObjectDescendentOf(idCompositeItem, pFolder->getUUID())) )
				return true;
		}
		return false;
	}
#endif // RLV_EXPERIMENTAL_COMPOSITEFOLDING

#ifdef RLV_EXPERIMENTAL_COMPOSITEFOLDERS
	// Checked: 2009-12-18 (RLVa-1.1.0k) | Modified: RLVa-1.1.0i
	bool RlvHandler::canTakeOffComposite(const LLInventoryCategory* pFolder) const
	{
		// Sanity check - if there's no folder or no avatar then there is nothing to take off
		LLVOAvatar* pAvatar = gAgent.getAvatarObject();
		if ( (!pFolder) || (!pAvatar) )
			return false;
		// Sanity check - if nothing is locked then we can definitely take it off
		if ( (!hasBehaviour(RLV_BHVR_REMOUTFIT)) && (!hasLockedAttachment(RLV_LOCK_REMOVE)) )
			return true;

		LLInventoryModel::cat_array_t folders;
		LLInventoryModel::item_array_t items;
		RlvWearableItemCollector functor(pFolder->getUUID(), true, false);
		gInventory.collectDescendentsIf(pFolder->getUUID(), folders, items, FALSE, functor);

		for (S32 idxItem = 0, cntItem = items.count(); idxItem < cntItem; idxItem++)
		{
			const LLViewerInventoryItem* pItem = items.get(idxItem);
			switch (pItem->getType())
			{
				case LLAssetType::AT_BODYPART:
				case LLAssetType::AT_CLOTHING:
					{
						LLWearable* pWearable = gAgent.getWearableFromWearableItem(pItem->getUUID());
						if ( (pWearable) && (!isRemovable(pWearable->getType())) )
							return false;	// If one wearable in the folder is non-removeable then the entire folder should be
					}
					break;
				case LLAssetType::AT_OBJECT:
					{
						LLViewerObject* pObj = pAvatar->getWornAttachment(pItem->getUUID());
						if ( (pObj != NULL) && (isLockedAttachment(pObj, RLV_LOCK_REMOVE)) )
							return false;	// If one attachment in the folder is non-detachable then the entire folder should be
					}
					break;
				default:
					break;
			}
		}
		return true;
	}

	// Checked: 2009-12-18 (RLVa-1.1.0k) | Modified: RLVa-1.1.0i
	bool RlvHandler::canWearComposite(const LLInventoryCategory* pFolder) const
	{
		// Sanity check - if there's no folder or no avatar then there is nothing to wear
		LLVOAvatar* pAvatar = gAgent.getAvatarObject();
		if ( (!pFolder) || (!pAvatar) )
			return false;
		// Sanity check - if nothing is locked then we can definitely wear it
		if ( (!hasBehaviour(RLV_BHVR_ADDOUTFIT)) && (!hasBehaviour(RLV_BHVR_REMOUTFIT)) && (!hasLockedAttachment(RLV_LOCK_ANY)) )
			return true;

		LLInventoryModel::cat_array_t folders;
		LLInventoryModel::item_array_t items;
		RlvWearableItemCollector functor(pFolder->getUUID(), true, false);
		gInventory.collectDescendentsIf(pFolder->getUUID(), folders, items, FALSE, functor);

		for (S32 idxItem = 0, cntItem = items.count(); idxItem < cntItem; idxItem++)
		{
			LLViewerInventoryItem* pItem = items.get(idxItem);

			if (RlvForceWear::isWearingItem(pItem))
				continue; // Don't examine any items we're already wearing

			// A wearable layer or attachment point:
			//   - can't be "add locked"
			//   - can't be worn and "remove locked"
			//   - can't be worn and have its item belong to a *different* composite folder that we can't take off
			switch (pItem->getType())
			{
				case LLAssetType::AT_BODYPART:
				case LLAssetType::AT_CLOTHING:
					{
						// NOTE: without its asset we don't know what type the wearable is so we need to look at the item's flags instead
						EWearableType wtType = (EWearableType)(pItem->getFlags() & LLInventoryItem::II_FLAGS_WEARABLES_MASK);
						LLViewerInventoryCategory* pFolder;
						if ( (!isWearable(wtType)) ||
							 ( (gAgent.getWearable(wtType)) && (!isRemovable(wtType)) ) ||
							 ( (gRlvHandler.getCompositeInfo(gAgent.getWearableItem(wtType), NULL, &pFolder)) &&
							   (pFolder->getUUID() != pItem->getParentUUID()) && (!gRlvHandler.canTakeOffComposite(pFolder)) ) )
						{
							return false;
						}
					}
					break;
				case LLAssetType::AT_OBJECT:
					{
						// If we made it here then *something* is add/remove locked so we absolutely need to know its attachment point
						LLViewerJointAttachment* pAttachPt = getAttachPoint(pItem, true);
						LLViewerInventoryCategory* pFolder;
						if ( (!pAttachPt) || (isLockedAttachment(pAttachPt, RLV_LOCK_ADD)) ||
							 ( (pAttachPt->getObject()) && (isLockedAttachment(pAttachPt, RLV_LOCK_REMOVE)) ) ||
							 ( (gRlvHandler.getCompositeInfo(pAttachPt->getItemID(), NULL, &pFolder)) &&
							   (pFolder->getUUID() != pItem->getParentUUID()) && (!gRlvHandler.canTakeOffComposite(pFolder)) ) )
						{
							return false;
						}
					}
					break;
				default:
					break;
			}
		}
		return true;
	}
#endif // RLV_EXPERIMENTAL_COMPOSITEFOLDERS

// ============================================================================
// Event handlers
//

// (In case anyone cares: this isn't used in public builds)
bool RlvHandler::getWornInfo(const LLInventoryCategory* pFolder, U8& wiFolder, U8& wiChildren) const
{
	// Sanity check - getAvatarObject() can't be NULL [see RlvForceWear::isWearingItem()] and the folder should exist and not be hidden
	if ((!gAgent.getAvatarObject()) || (!pFolder) || (pFolder->getName().empty()) || (RLV_FOLDER_PREFIX_HIDDEN == pFolder->getName()[0]))
		return false;

	LLInventoryModel::cat_array_t  folders;
	LLInventoryModel::item_array_t items;
	RlvWearableItemCollector functor(pFolder->getUUID(), true, true);
	gInventory.collectDescendentsIf(pFolder->getUUID(), folders, items, FALSE, functor);

	LLViewerInventoryItem* pItem;
	U32 cntWorn = 0, cntTotal = 0, cntChildWorn = 0, cntChildTotal = 0;
	for (S32 idxItem = 0, cntItem = items.count(); idxItem < cntItem; idxItem++)
	{
		pItem = items.get(idxItem);

		bool fDirectDescendent = (pFolder->getUUID() == functor.getFoldedParent(pItem->getParentUUID()));
		U32 &refWorn = (fDirectDescendent) ? cntWorn : cntChildWorn, &refTotal = (fDirectDescendent) ? cntTotal : cntChildTotal;

		if (RlvForceWear::isWearingItem(pItem))
			refWorn++;
		refTotal++;
	}

	wiFolder = (0 == cntTotal + cntChildTotal) ? 0 : (0 == cntWorn + cntChildWorn) ? 1 : 
		(cntWorn + cntChildWorn != cntTotal + cntChildTotal) ? 2 : 3;
	wiChildren = (0 == cntChildTotal) ? 0 : (0 == cntChildWorn) ? 1 : (cntChildWorn != cntChildTotal) ? 2 : 3;

	return true;
}

// ============================================================================
// Initialization helper functions
//

// Checked: 2009-11-25 (RLVa-1.1.0f) | Modified: RLVa-1.1.0f
BOOL RlvHandler::setEnabled(BOOL fEnable)
{
	if (m_fEnabled == fEnable)
		return fEnable;

	if (fEnable)
	{
		// Initialize the command lookup table
		RlvCommand::initLookupTable();

		// Initialize static classes
		RlvSettings::initClass();
		RlvStrings::initClass();

		gRlvHandler.m_pAttachMgr = new RlvAttachmentManager();
		gRlvHandler.addCommandHandler(new RlvExtGetSet());

		// Fetch shared inventory if we're enabled after logon
		if (LLStartUp::getStartupState() >= STATE_CLEANUP)
			fetchSharedInventory();

		m_fEnabled = TRUE;
	}
	else if (canDisable())
	{
		gRlvHandler.clearState();
	}

	#ifdef RLV_ADVANCED_MENU
		// RELEASE-RLVa: LL defines CLIENT_MENU_NAME but we can't get to it from here so we need to keep those two in sync manually
		LLMenuGL* pClientMenu = NULL;
		if ( (gMenuBarView) && ((pClientMenu = gMenuBarView->getChildMenuByName("Advanced", FALSE)) != NULL) )
		{
			pClientMenu->setItemVisible("RLVa", m_fEnabled);
			pClientMenu->setItemEnabled("RLVa", m_fEnabled);
		}
	#endif // RLV_ADVANCED_MENU

	return m_fEnabled;		// Return enabled/disabled state
}

BOOL RlvHandler::canDisable()
{
	return FALSE;
}

void RlvHandler::clearState()
{
	// TODO-RLVa: should restore all RLV controlled debug variables to their defaults

	// Issue @clear on behalf of every object that has a currently active RLV restriction (even if it's just an exception)
	LLUUID idObj; LLViewerObject* pObj; bool fDetachable;
	while (m_Objects.size())
	{
		idObj = m_Objects.begin()->first; // Need a copy since after @clear the data it points to will no longer exist
		fDetachable = ((pObj = gObjectList.findObject(idObj)) != NULL) ? isLockedAttachment(pObj, RLV_LOCK_REMOVE) : true;

		processCommand(idObj, "clear", false);
		if (!fDetachable)
			processCommand(idObj, "detachme=force", false);
	}

	// Sanity check - these should all be empty after we issue @clear on the last object
	if ( (!m_Objects.empty()) || !(m_Exceptions.empty()) || (!m_AttachAdd.empty()) || (!m_AttachRem.empty()) )
	{
		RLV_ERRS << "Object, exception or attachment map not empty after clearing state!" << LL_ENDL;
		m_Objects.clear();
		m_Exceptions.clear();
		m_AttachAdd.clear();
		m_AttachRem.clear();
	}

	// These all need manual clearing
	memset(m_LayersAdd, 0, sizeof(S16) * WT_COUNT);
	memset(m_LayersRem, 0, sizeof(S16) * WT_COUNT);
	memset(m_Behaviours, 0, sizeof(S16) * RLV_BHVR_COUNT);
	m_Retained.clear();
	clearCommandHandlers(); // <- calls delete on all registered command handlers

	// Clear dynamically allocated memory
	delete m_pGCTimer;
	m_pGCTimer = NULL;
	delete m_pWLSnapshot;
	m_pWLSnapshot = NULL;
	delete m_pAttachMgr;
	m_pAttachMgr = NULL;
}

// ============================================================================
// Command handlers (RLV_TYPE_ADD and RLV_TYPE_REMOVE)
//

#define VERIFY_OPTION(x)		{ if (!(x)) { eRet = RLV_RET_FAILED_OPTION; break; } }
#define VERIFY_OPTION_REF(x)	{ if (!(x)) { eRet = RLV_RET_FAILED_OPTION; break; } fRefCount = true; }

// Checked: 2009-12-05 (RLVa-1.1.0h) | Modified: RLVa-1.1.0h
ERlvCmdRet RlvHandler::processAddRemCommand(const LLUUID& idObj, const RlvCommand& rlvCmd)
{
	// NOTE: - at this point the command has already been:
	//            * added to the RlvObject
	//            * removed from the RlvObject (which still exists at this point even if this is the last restriction)
	//       - the object's UUID may or may not exist in gObjectList (see handling of @detach=n|y)
	ERlvBehaviour eBhvr = rlvCmd.getBehaviourType(); ERlvParamType eType = rlvCmd.getParamType();

	ERlvCmdRet eRet = RLV_RET_SUCCESS; bool fRefCount = false, fRefreshHover = false; const std::string& strOption = rlvCmd.getOption();
	switch (eBhvr)
	{
		case RLV_BHVR_DETACH:				// @detach[:<option>]=n|y
			eRet = onAddRemDetach(idObj, rlvCmd, fRefCount);
			break;
		case RLV_BHVR_ADDATTACH:			// @addattach[:<option>]=n|y
		case RLV_BHVR_REMATTACH:			// @addattach[:<option>]=n|y
			eRet = onAddRemAttach(idObj, rlvCmd, fRefCount);
			break;
		case RLV_BHVR_ADDOUTFIT:			// @addoutfit[:<layer>]=n|y			- Checked: 2009-12-05 (RLVa-1.1.0h) | Modified: RLVa-1.1.0h
		case RLV_BHVR_REMOUTFIT:			// @remoutfit[:<layer>]=n|y			- Checked: 2009-12-05 (RLVa-1.1.0h) | Modified: RLVa-1.1.0h
			{
				 // If there's an option it should specify a wearable type name (reference count on no option *and* a valid option)
				EWearableType wtType = LLWearable::typeNameToType(strOption);
				VERIFY_OPTION_REF( (strOption.empty()) || (WT_INVALID != wtType) );

				S16* pLayers = (RLV_BHVR_ADDOUTFIT == eBhvr) ? m_LayersAdd : m_LayersRem;
				for (int idxType = 0; idxType < WT_COUNT; idxType++)
				{
					if ( ((EWearableType)idxType == wtType) || (WT_INVALID == wtType) )
					{
						if (RLV_TYPE_ADD == eType)
							pLayers[idxType]++;
						else
							pLayers[idxType]--;
					}
				}
			}
			break;
		case RLV_BHVR_REDIRCHAT:			// @redirchat:<channel>=n|y			- Checked: 2009-12-05 (RLVa-1.1.0h) | Modified: RLVa-1.1.0h
		case RLV_BHVR_REDIREMOTE:			// @rediremote:<channel>=n|y		- Checked: 2009-12-05 (RLVa-1.1.0h) | Modified: RLVa-1.1.0h
			{
				// There should be an option and it should specify a valid reply channel (if there's an empty option the command is invalid)
				S32 nChannel = 0;
				VERIFY_OPTION_REF( (LLStringUtil::convertToS32(strOption, nChannel)) && (rlvIsValidReplyChannel(nChannel)) );

				if (RLV_TYPE_ADD == eType)
					addException(idObj, eBhvr, nChannel);
				else
					removeException(idObj, eBhvr, nChannel);
			}
			break;
		case RLV_BHVR_SENDCHANNEL:			// @sendchannel[:<channel>]=n|y		- Checked: 2009-12-05 (RLVa-1.1.0h) | Modified: RLVa-1.1.0h
			{
				// If there's an option then it should be a valid (= positive and non-zero) chat channel (only reference count empty option)
				S32 nChannel = 0;
				if ( (LLStringUtil::convertToS32(strOption, nChannel)) && (nChannel > 0) )
				{
					if (RLV_TYPE_ADD == eType)
						addException(idObj, eBhvr, nChannel);
					else
						removeException(idObj, eBhvr, nChannel);
					break;
				}
				VERIFY_OPTION_REF(strOption.empty());
			}
			break;
		case RLV_BHVR_NOTIFY:				// @notify:<params>=add|rem			- Checked: 2009-12-05 (RLVa-1.1.0h) | Modified: RLVa-1.1.0h
			{
				// There should be an option that we can successfully parse (if there's an empty option the command is invalid)
				S32 nChannel; std::string strFilter;
				VERIFY_OPTION_REF( (!strOption.empty()) && (rlvParseNotifyOption(strOption, nChannel, strFilter)) );

				if (RLV_TYPE_ADD == eType)
				{
					if (!m_pBhvrNotify)
						addBehaviourObserver(m_pBhvrNotify = new RlvBehaviourNotifyObserver());
					m_pBhvrNotify->addNotify(idObj, nChannel, strFilter);
				}
				else if (m_pBhvrNotify)
				{
					m_pBhvrNotify->removeNotify(idObj, nChannel, strFilter);
					if (!m_pBhvrNotify->hasNotify())
					{
						removeBehaviourObserver(m_pBhvrNotify);
						delete m_pBhvrNotify;
						m_pBhvrNotify = NULL;
					}
				}
			}
			break;
		case RLV_BHVR_SHOWINV:				// @showinv=n|y						- Checked: 2009-12-05 (RLVa-1.1.0h) | Modified: RLVa-1.1.0h
			{
				VERIFY_OPTION_REF(strOption.empty());

				if (RLV_TYPE_ADD == eType)
				{
					// Close all open inventory windows
					LLInventoryView::closeAll();
				}
			}
			break;
		case RLV_BHVR_SHOWMINIMAP:			// @showminimap=n|y					- Checked: 2009-12-05 (RLVa-1.1.0g) | Modified: RLVa-1.1.0g
			{
				VERIFY_OPTION_REF(strOption.empty());

				if (RLV_TYPE_ADD == eType)
				{
					// Simulate clicking the Minimap button [see LLToolBar::onClickRadar()]
					#if RLV_TARGET < RLV_MAKE_TARGET(1, 23, 0)			// Version: 1.22.11
						if (gFloaterMap->getVisible())
							LLFloaterMap::toggle(NULL);
					#else												// Version: 1.23.4
						if (LLFloaterMap::instanceVisible())
							LLFloaterMap::hideInstance();
					#endif
				}
			}
			break;
		case RLV_BHVR_SHOWWORLDMAP:			// @showworldmap=n|y				- Checked: 2009-12-05 (RLVa-1.1.0g) | Modified: RLVa-1.1.0g
			{
				VERIFY_OPTION_REF(strOption.empty());

				if (RLV_TYPE_ADD == eType)
				{
					// Simulate clicking the Map button [see LLToolBar::onClickMap()]
					if (gFloaterWorldMap->getVisible())
						LLFloaterWorldMap::toggle(NULL);
				}
			}
			break;
		case RLV_BHVR_SHOWLOC:				// @showloc=n|y						- Checked: 2009-12-05 (RLVa-1.1.0h) | Modified: RLVa-1.1.0h
			{
				VERIFY_OPTION_REF(strOption.empty());

				if (RLV_TYPE_ADD == eType)
				{
					// If this is the first @showloc=n restriction refresh all object text so we can filter it if necessary
					fRefreshHover = (0 == m_Behaviours[RLV_BHVR_SHOWLOC]);

					// Close the "About Land" floater if it's currently visible
					if (LLFloaterLand::instanceVisible())
						LLFloaterLand::hideInstance();

					// Close the "Estate Tools" floater is it's currently visible
					if (LLFloaterRegionInfo::instanceVisible())
						LLFloaterRegionInfo::hideInstance();

					// NOTE: we should close the "God Tools" floater as well, but since calling LLFloaterGodTools::instance() always
					//       creates a new instance of the floater and since it's very unlikely to be open it's just better not to
				}
				else
				{
					// If this is the last @showloc=n restriction refresh all object text in case anything needs restoring
					fRefreshHover = (1 == m_Behaviours[RLV_BHVR_SHOWLOC]);
				}
			}
			break;
		case RLV_BHVR_SHOWNAMES:			// @shownames=n|y					- Checked: 2009-12-05 (RLVa-1.1.0h) | Modified: RLVa-1.1.0h
			{
				VERIFY_OPTION_REF(strOption.empty());

				if (RLV_TYPE_ADD == eType)
				{
					// If this is the first @shownames=n restriction refresh all object text so we can filter it if necessary
					fRefreshHover = (0 == m_Behaviours[RLV_BHVR_SHOWNAMES]);

					// Close the "Active Speakers" panel if it's currently visible
					LLFloaterChat::getInstance()->childSetVisible("active_speakers_panel", false);
				}
				else
				{
					// If this is the last @shownames=n restriction refresh all object text in case anything needs restoring
					fRefreshHover = (1 == m_Behaviours[RLV_BHVR_SHOWNAMES]);
				}
			}
			break;
		case RLV_BHVR_SHOWHOVERTEXTALL:		// @showhovertextall=n|y			- Checked: 2009-12-05 (RLVa-1.1.0h) | Modified: RLVa-1.1.0h
		case RLV_BHVR_SHOWHOVERTEXTWORLD:	// @showhovertextworld=n|y			- Checked: 2009-12-05 (RLVa-1.1.0h) | Modified: RLVa-1.1.0h
		case RLV_BHVR_SHOWHOVERTEXTHUD:		// @showhovertexthud=n|y			- Checked: 2009-12-05 (RLVa-1.1.0h) | Modified: RLVa-1.1.0h
			{
				VERIFY_OPTION_REF(strOption.empty());

				// Refresh all object text on the first/last add/rem(LLHUDText::setStringUTF8() decides what needs clearing and what doesn't)
				fRefreshHover = ((RLV_TYPE_ADD == eType) && (0 == m_Behaviours[eBhvr])) ||
					            ((RLV_TYPE_REMOVE == eType) && (1 == m_Behaviours[eBhvr]));
			}
			break;
		case RLV_BHVR_SHOWHOVERTEXT:		// @showhovertext:<uuid>=n|y		- Checked: 2009-12-05 (RLVa-1.1.0h) | Modified: RLVa-1.1.0h
			{
				// There should be an option and it should specify a valid UUID
				LLUUID idException(strOption);
				VERIFY_OPTION_REF(idException.notNull());

				if (RLV_TYPE_ADD == eType)
					addException(idObj, eBhvr, idException);
				else
					removeException(idObj, eBhvr, idException);

				// Clear/restore the object's hover text as needed
				LLViewerObject* pObj = gObjectList.findObject(idException);
				if ( (pObj) && (pObj->mText.notNull()) && (!pObj->mText->getObjectText().empty()) )
					pObj->mText->setStringUTF8( (RLV_TYPE_ADD == eType) ? "" : pObj->mText->getObjectText());
			}
			break;
		case RLV_BHVR_EDIT:					// @edit=n|y						- Checked: 2009-12-05 (RLVa-1.1.0h) | Modified: RLVa-1.1.0h
			{
				VERIFY_OPTION_REF(strOption.empty());

				if (RLV_TYPE_ADD == eType)
				{
					// Turn off "View / Highlight Transparent"
					LLDrawPoolAlpha::sShowDebugAlpha = FALSE;

					// Close the Beacons floater if it's open
					if (LLFloaterBeacons::instanceVisible())
						LLFloaterBeacons::toggleInstance();

					// Get rid of the build floater if it's open [copy/paste from toggle_build_mode()]
					if (gFloaterTools->getVisible())
					{
						gAgent.resetView(FALSE);
						gFloaterTools->close();
						gViewerWindow->showCursor();
					}
				}
			}
			break;
#ifdef RLV_EXTENSION_CMD_TOUCHXXX
		case RLV_BHVR_TOUCH:				// @touch:<uuid>=n					- Checked: 2010-01-01 (RLVa-1.1.0l) | Added: RLVa-1.1.0l
			{
				// There should be an option and it should specify a valid UUID
				LLUUID idException(strOption);
				VERIFY_OPTION_REF(idException.notNull());

				if (RLV_TYPE_ADD == eType)
					addException(idObj, eBhvr, idException);
				else
					removeException(idObj, eBhvr, idException);
			}
			break;
#endif // RLV_EXTENSION_CMD_TOUCHXXX
		case RLV_BHVR_FLY:					// @fly=n|y							- Checked: 2009-12-05 (RLVa-1.1.0h) | Modified: RLVa-1.1.0h
			{
				VERIFY_OPTION_REF(strOption.empty());

				if (RLV_TYPE_ADD == eType)
					gAgent.setFlying(FALSE);
			}
			break;
		case RLV_BHVR_SETENV:				// @setenv=n|y
			eRet = onAddRemSetEnv(idObj, rlvCmd, fRefCount);
			break;
		// The following block is only valid if there's no option
		case RLV_BHVR_EMOTE:				// @emote=n|y						- Checked: 2009-12-05 (RLVa-1.1.0h) | Modified: RLVa-1.1.0h
		case RLV_BHVR_SENDCHAT:				// @sendchat=n|y					- Checked: 2009-12-05 (RLVa-1.1.0h) | Modified: RLVa-1.1.0h
		case RLV_BHVR_CHATWHISPER:			// @chatwhisper=n|y					- Checked: 2009-12-05 (RLVa-1.1.0h) | Modified: RLVa-1.1.0h
		case RLV_BHVR_CHATNORMAL:			// @chatnormal=n|y					- Checked: 2009-12-05 (RLVa-1.1.0h) | Modified: RLVa-1.1.0h
		case RLV_BHVR_CHATSHOUT:			// @chatshout=n|y					- Checked: 2009-12-05 (RLVa-1.1.0h) | Modified: RLVa-1.1.0h
		case RLV_BHVR_PERMISSIVE:			// @permissive=n|y					- Checked: 2009-12-05 (RLVa-1.1.0h) | Modified: RLVa-1.1.0h
		case RLV_BHVR_TPLM:					// @tplm=n|y						- Checked: 2009-12-05 (RLVa-1.1.0h) | Modified: RLVa-1.1.0h
		case RLV_BHVR_TPLOC:				// @tploc=n|y						- Checked: 2009-12-05 (RLVa-1.1.0h) | Modified: RLVa-1.1.0h
		case RLV_BHVR_VIEWNOTE:				// @viewnote=n|y					- Checked: 2009-12-05 (RLVa-1.1.0h) | Modified: RLVa-1.1.0h
		case RLV_BHVR_VIEWSCRIPT:			// @viewscript=n|y					- Checked: 2009-12-05 (RLVa-1.1.0h) | Modified: RLVa-1.1.0h
		case RLV_BHVR_VIEWTEXTURE:			// @viewtexture=n|y					- Checked: 2009-12-05 (RLVa-1.1.0h) | Modified: RLVa-1.1.0h
		case RLV_BHVR_ACCEPTPERMISSION:		// @acceptpermission=n|y			- Checked: 2009-12-05 (RLVa-1.1.0h) | Modified: RLVa-1.1.0h
		case RLV_BHVR_DEFAULTWEAR:			// @defaultwear=n|y					- Checked: 2009-12-05 (RLVa-1.1.0h) | Modified: RLVa-1.1.0h
#ifdef RLV_EXTENSION_CMD_ALLOWIDLE
		case RLV_BHVR_ALLOWIDLE:			// @allowidle=n|y					- Checked: 2009-12-05 (RLVa-1.1.0h) | Modified: RLVa-1.1.0h
#endif // RLV_EXTENSION_CMD_ALLOWIDLE
		case RLV_BHVR_REZ:					// @rez=n|y							- Checked: 2009-12-05 (RLVa-1.1.0h) | Modified: RLVa-1.1.0h
		case RLV_BHVR_FARTOUCH:				// @fartouch=n|y					- Checked: 2009-12-05 (RLVa-1.1.0h) | Modified: RLVa-1.1.0h
#ifdef RLV_EXTENSION_CMD_INTERACT
		case RLV_BHVR_INTERACT:				// @interact=n|y					- Checked: 2010-01-01 (RLVa-1.1.0l) | Added: RLVa-1.1.0l
#endif // RLV_EXTENSION_CMD_INTERACT
		case RLV_BHVR_UNSIT:				// @unsit=n|y						- Checked: 2009-12-05 (RLVa-1.1.0h) | Modified: RLVa-1.1.0h
		case RLV_BHVR_SIT:					// @sit=n|y							- Checked: 2009-12-05 (RLVa-1.1.0h) | Modified: RLVa-1.1.0h
		case RLV_BHVR_SITTP:				// @sittp=n|y						- Checked: 2009-12-05 (RLVa-1.1.0h) | Modified: RLVa-1.1.0h
		case RLV_BHVR_SETDEBUG:				// @setdebug=n|y					- Checked: 2009-12-05 (RLVa-1.1.0h) | Modified: RLVa-1.1.0h
			VERIFY_OPTION_REF(strOption.empty());
			break;
		// The following block is only valid if there's no option (= restriction) or if it specifies a valid UUID (= behaviour exception)
		case RLV_BHVR_RECVCHAT:				// @recvchat[:<uuid>]=n|y			- Checked: 2009-12-05 (RLVa-1.1.0h) | Modified: RLVa-1.1.0h
		case RLV_BHVR_RECVEMOTE:			// @recvemote[:<uuid>]=n|y			- Checked: 2009-12-05 (RLVa-1.1.0h) | Modified: RLVa-1.1.0h
		case RLV_BHVR_SENDIM:				// @sendim[:<uuid>]=n|y				- Checked: 2009-12-05 (RLVa-1.1.0h) | Modified: RLVa-1.1.0h
		case RLV_BHVR_RECVIM:				// @recvim[:<uuid>]=n|y				- Checked: 2009-12-05 (RLVa-1.1.0h) | Modified: RLVa-1.1.0h
		case RLV_BHVR_TPLURE:				// @tplure[:<uuid>]=n|y				- Checked: 2009-12-05 (RLVa-1.1.0h) | Modified: RLVa-1.1.0h
		case RLV_BHVR_ACCEPTTP:				// @accepttp[:<uuid>]=n|y			- Checked: 2009-12-05 (RLVa-1.1.0h) | Modified: RLVa-1.1.0h
#ifdef RLV_EXTENSION_CMD_TOUCHXXX
		case RLV_BHVR_TOUCHWORLD:			// @touchworld[:<uuid>=n|y			- Checked: 2010-01-01 (RLVa-1.1.0l) | Added: RLVa-1.1.0l
		case RLV_BHVR_TOUCHATTACH:			// @touchattach[:<uuid>=n|y			- Checked: 2010-01-01 (RLVa-1.1.0l) | Added: RLVa-1.1.0l
		case RLV_BHVR_TOUCHHUD:				// @touchhud[:<uuid>=n|y			- Checked: 2010-01-01 (RLVa-1.1.0l) | Added: RLVa-1.1.0l
#endif // RLV_EXTENSION_CMD_TOUCHXXX
			{
				LLUUID idException(strOption);
				if (idException.notNull())		// If there's an option then it should specify a valid UUID
				{
					if (RLV_TYPE_ADD == eType)
						addException(idObj, eBhvr, idException);
					else
						removeException(idObj, eBhvr, idException);
					break;
				}
				VERIFY_OPTION_REF(strOption.empty());
			}
			break;
		case RLV_BHVR_UNKNOWN:
			// Pass unknown commands on to registered command handlers
			return (notifyCommandHandlers(&RlvCommandHandler::onAddRemCommand, idObj, rlvCmd, eRet, false)) ? eRet : RLV_RET_FAILED_UNKNOWN;
		default:
			// Fail with "Invalid param" if none of the above handled it
			eRet = RLV_RET_FAILED_PARAM;
			break;
	}

	// If this command represents a behaviour restriction that's been added/removed then we need to do some additional processing
	if ( (RLV_RET_SUCCESS == eRet) && (fRefCount) )
	{
		if (RLV_TYPE_ADD == eType)
		{
			if (rlvCmd.isStrict())
				addException(idObj, RLV_BHVR_PERMISSIVE, eBhvr);
			m_Behaviours[eBhvr]++;
		}
		else
		{
			if (rlvCmd.isStrict())
				removeException(idObj, RLV_BHVR_PERMISSIVE, eBhvr);
			m_Behaviours[eBhvr]--;
		}

		// Since canShowHoverText() uses hasBehaviour() refreshing object text needs to wait until after we've reference counted
		if (fRefreshHover)
			LLHUDText::refreshAllObjectText();

		// Since RlvSettings::updateLoginLastLocation() uses hasBehaviour() it needs to be called after we've reference counted
		#ifdef RLV_EXTENSION_STARTLOCATION
			RlvSettings::updateLoginLastLocation();
		#endif // RLV_EXTENSION_STARTLOCATION

		// Show an - optional - notification on every global behaviour change
		#ifdef RLV_EXTENSION_NOTIFY_BEHAVIOUR
		if ( ((RLV_TYPE_ADD == eType) && (1 == m_Behaviours[eBhvr])) || (0 == m_Behaviours[eBhvr]) )
			RlvNotifications::notifyBehaviour(eBhvr, eType);
		#endif // RLV_EXTENSION_NOTIFY_BEHAVIOUR
	}

	return eRet;
}

// Checked: 2009-10-10 (RLVa-1.0.5a) | Added: RLVa-1.0.5a
ERlvCmdRet RlvHandler::onAddRemAttach(const LLUUID& idObj, const RlvCommand& rlvCmd, bool& fRefCount)
{
	// Sanity check - if there's an option it should specify a valid attachment point name
	S32 idxAttachPt = getAttachPointIndex(rlvCmd.getOption(), true);
	if ( (!idxAttachPt) && (!rlvCmd.getOption().empty())  )
		return RLV_RET_FAILED_OPTION;

	LLVOAvatar* pAvatar = gAgent.getAvatarObject();
	if (!pAvatar)
		return RLV_RET_FAILED;

	ERlvLockMask eLock = (RLV_BHVR_REMATTACH == rlvCmd.getBehaviourType()) ? RLV_LOCK_REMOVE : RLV_LOCK_ADD;
	for (LLVOAvatar::attachment_map_t::const_iterator itAttach = pAvatar->mAttachmentPoints.begin(); 
			itAttach != pAvatar->mAttachmentPoints.end(); ++itAttach)
	{
		if ( (0 == idxAttachPt) || (itAttach->first == idxAttachPt) )
		{
			if (RLV_TYPE_ADD == rlvCmd.getParamType())
				addAttachmentLock(itAttach->first, idObj, eLock);
			else
				removeAttachmentLock(itAttach->first, idObj, eLock);
		}
	}

	// Refresh HUD visibility if needed
	if ( (RLV_BHVR_REMATTACH == rlvCmd.getBehaviourType()) && (hasLockedHUD()) )
		LLPipeline::sShowHUDAttachments = TRUE;

	fRefCount = rlvCmd.getOption().empty();	// Only reference count global locks
	return RLV_RET_SUCCESS;
}

// Checked: 2010-07-18 (RLVa-1.1.2b) | Modified: RLVa-1.1.2a
ERlvCmdRet RlvHandler::onAddRemDetach(const LLUUID& idObj, const RlvCommand& rlvCmd, bool& fRefCount)
{
	S32 idxAttachPt = 0;
	if (rlvCmd.getOption().empty())	// @detach=n|y
	{
		// The object may or may not exist (it may not have rezzed yet, or it may have already been killed):
		//   * @detach=n: - if it has rezzed then we'll already have its attachment point 
		//                - if it hasn't rezzed yet then it's a @detach=n from a non-attachment and RlvHandler::onAttach() takes care of it
		//   * @detach=y: - if it ever rezzed as an attachment we'll have cached its attach point 
		//                - if it never rezzed as an attachment there won't be a lock to remove
		rlv_object_map_t::const_iterator itObj = m_Objects.find(idObj);
		if ( (itObj != m_Objects.end()) && (itObj->second.m_fLookup) && (itObj->second.m_idxAttachPt) )
			idxAttachPt = itObj->second.m_idxAttachPt;
	}
	else							// @detach:<attachpt>=n|y
	{
		idxAttachPt = getAttachPointIndex(rlvCmd.getOption(), true);
	}

	// The attach point can be zero for @detach=n|y (i.e. non-attachment) but should always be non-zero for @detach:<attachpt>=n|y
	if (0 == idxAttachPt)
		return (rlvCmd.getOption().empty()) ? RLV_RET_SUCCESS : RLV_RET_FAILED_OPTION;

	// Actually lock the attachment point (@detach=n locks remove only; @detach:<attachpt>=n locks both remove and add)
	ERlvLockMask eLock = (rlvCmd.getOption().empty()) ? RLV_LOCK_REMOVE : (ERlvLockMask)(RLV_LOCK_ADD | RLV_LOCK_REMOVE);
	if (RLV_TYPE_ADD == rlvCmd.getParamType())
		addAttachmentLock(idxAttachPt, idObj, eLock);
	else
		removeAttachmentLock(idxAttachPt, idObj, eLock);

	// Refresh HUD visibility if needed
	if ( (RLV_TYPE_ADD == rlvCmd.getParamType()) && (hasLockedHUD()) )
		LLPipeline::sShowHUDAttachments = TRUE;

	fRefCount = false;	// Don't reference count @detach[:<option>]=n
	return RLV_RET_SUCCESS;
}

// Checked: 2009-12-05 (RLVa-1.1.0h) | Added: RLVa-1.1.0h
ERlvCmdRet RlvHandler::onAddRemSetEnv(const LLUUID& idObj, const RlvCommand& rlvCmd, bool& fRefCount)
{
	// Sanity check - there shouldn't be an option
	if (!rlvCmd.getOption().empty())
		return RLV_RET_FAILED_OPTION;
	if (RlvSettings::getNoSetEnv())
		return RLV_RET_FAILED_DISABLED;

	if (RLV_TYPE_ADD == rlvCmd.getParamType())
	{
		// Only close the floaters if their instance exists and they're actually visible
		if ( (LLFloaterEnvSettings::isOpen()) && (LLFloaterEnvSettings::instance()->getVisible()) )
			LLFloaterEnvSettings::instance()->close();
		if ( (LLFloaterWindLight::isOpen()) && (LLFloaterWindLight::instance()->getVisible()) )
			LLFloaterWindLight::instance()->close();
		if ( (LLFloaterWater::isOpen()) && (LLFloaterWater::instance()->getVisible()) )
			LLFloaterWater::instance()->close();
		if ( (LLFloaterDayCycle::isOpen()) && (LLFloaterDayCycle::instance()->getVisible()) )
			LLFloaterDayCycle::instance()->close();

		// Save the current WindLight params so we can restore them on @setenv=y
		if (m_pWLSnapshot)
		{
			RLV_ERRS << "m_pWLSnapshot != NULL" << RLV_ENDL; // Safety net in case we set @setenv=n for more than 1 object
			delete m_pWLSnapshot;
		}
		m_pWLSnapshot = RlvWLSnapshot::takeSnapshot();
	}
	else
	{
		// Restore WindLight parameters to what they were before @setenv=n was issued
		RlvWLSnapshot::restoreSnapshot(m_pWLSnapshot);
		delete m_pWLSnapshot;
		m_pWLSnapshot = NULL;
	}
	fRefCount = true;
	return RLV_RET_SUCCESS;
}

// ============================================================================
// Command handlers (RLV_TYPE_FORCE)
//

// Checked: 2009-12-21 (RLVa-1.1.0j) | Modified: RLVa-1.1.0j
ERlvCmdRet RlvHandler::processForceCommand(const LLUUID& idObj, const RlvCommand& rlvCmd) const
{
	RLV_ASSERT(RLV_TYPE_FORCE == rlvCmd.getParamType());

	ERlvCmdRet eRet = RLV_RET_SUCCESS;
	switch (rlvCmd.getBehaviourType())
	{
		case RLV_BHVR_DETACH:		// @detach[:<option>]=force		- Checked: 2009-12-21 (RLVa-1.1.0k) | Modified: RLVa-1.1.0j
			eRet = onForceRemAttach(idObj, rlvCmd);
			if (RLV_RET_SUCCESS != eRet)
				eRet = onForceWear(rlvCmd.getOption(), false, false);
			break;
		case RLV_BHVR_REMATTACH:	// @remattach[:<option>]=force
			eRet = onForceRemAttach(idObj, rlvCmd);
			break;
		case RLV_BHVR_REMOUTFIT:	// @remoutfit[:<option>]=force
			eRet = onForceRemOutfit(idObj, rlvCmd);
			break;
		case RLV_BHVR_UNSIT:		// @unsit=force					- Checked: 2009-12-21 (RLVa-1.1.0k) | Modified: RLVa-0.2.0g
			{
				VERIFY_OPTION(rlvCmd.getOption().empty());
				LLVOAvatar* pAvatar = gAgent.getAvatarObject();
				if ( (pAvatar) && (pAvatar->mIsSitting) && (!hasBehaviourExcept(RLV_BHVR_UNSIT, idObj)) )
				{
					gAgent.setControlFlags(AGENT_CONTROL_STAND_UP);
					send_agent_update(TRUE, TRUE);	// See behaviour notes on why we have to force an agent update here
				}
			}
			break;
		case RLV_BHVR_SIT:			// @sit:<option>=force
			eRet = onForceSit(idObj, rlvCmd);
			break;
		case RLV_BHVR_TPTO:			// @tpto:<option>=force			- Checked: 2009-07-12 (RLVa-1.0.0k) | Modified: RLVa-1.0.0h
			{
				eRet = RLV_RET_FAILED_OPTION;
				if ( (!rlvCmd.getOption().empty()) && (std::string::npos == rlvCmd.getOption().find_first_not_of("0123456789/.")) )
				{
					LLVector3d posGlobal;

					boost_tokenizer tokens(rlvCmd.getOption(), boost::char_separator<char>("/", "", boost::keep_empty_tokens)); int idx = 0;
					for (boost_tokenizer::const_iterator itToken = tokens.begin(); itToken != tokens.end(); ++itToken)
					{
						if (idx < 3)
							LLStringUtil::convertToF64(*itToken, posGlobal[idx++]);
					}

					if (idx == 3)
					{
						gAgent.teleportViaLocation(posGlobal);
						eRet = RLV_RET_SUCCESS;
					}
				}
			}
			break;
		case RLV_BHVR_ADDOUTFIT:	// @addoutfit:<option>=force <- synonym of @attach:<option>=force
		case RLV_BHVR_ATTACH:		// @attach:<option>=force
			eRet = onForceWear(rlvCmd.getOption(), true, false);	// Force attach single folder
			break;
		case RLV_BHVR_ATTACHALL:	// @attachall:<option>=force
			eRet = onForceWear(rlvCmd.getOption(), true, true);		// Force attach nested folders
			break;
		case RLV_BHVR_DETACHALL:	// @detachall:<option>=force
			eRet = onForceWear(rlvCmd.getOption(), false, true);	// Force detach nested folders
			break;
		case RLV_BHVR_ATTACHTHIS:	// @attachthis[:<option>]=force
		case RLV_BHVR_ATTACHALLTHIS:// @attachallthis[:<option>]=force
		case RLV_BHVR_DETACHTHIS:	// @detachthis[:<option>]=force
		case RLV_BHVR_DETACHALLTHIS:// @detachallthis[:<option>]=force
			{
				ERlvBehaviour eBehaviour = rlvCmd.getBehaviourType(); std::string strReply;
				if ( ((eRet = onGetPath(idObj, rlvCmd, strReply)) == RLV_RET_SUCCESS) && (!strReply.empty()) )
				{
					LLStringUtil::toLower(strReply);
					eRet = onForceWear(strReply,
						(RLV_BHVR_ATTACHTHIS == eBehaviour) || (RLV_BHVR_ATTACHALLTHIS == eBehaviour),
						(RLV_BHVR_ATTACHALLTHIS == eBehaviour) || (RLV_BHVR_DETACHALLTHIS == eBehaviour));
				}
			}
			break;
		case RLV_BHVR_DETACHME:		// @detachme=force				- Checked:
			{
				// NOTE: @detachme=force could be seen as a @detach:<attachpt>=force but RLV implements it as a "detach by UUID"
				VERIFY_OPTION(rlvCmd.getOption().empty());
				LLViewerObject* pObj = NULL; LLVOAvatar* pAvatar = NULL; LLViewerJointAttachment* pAttachPt = NULL;
				if ( ((pObj = gObjectList.findObject(idObj)) != NULL) && (pObj->isAttachment()) &&
					 ((pAvatar = gAgent.getAvatarObject()) != NULL) &&
					 ((pAttachPt = pAvatar->getTargetAttachmentPoint(pObj->getRootEdit())) != NULL) )
				{
					// @detachme should respect locks but shouldn't respect things like nostrip so handle it like a manual user detach
					handle_detach_from_avatar(pAttachPt);
				}
			}
			break;
		case RLV_BHVR_UNKNOWN:
			// Pass unknown commands on to registered command handlers
			return (notifyCommandHandlers(&RlvCommandHandler::onForceCommand, idObj, rlvCmd, eRet, false)) ? eRet : RLV_RET_FAILED_UNKNOWN;
		default:
			// Fail with "Invalid param" if none of the above handled it
			eRet = RLV_RET_FAILED_PARAM;
			break;
	}
	return eRet;
}

// Checked: 2009-11-24 (RLVa-1.1.0k) | Modified: RLVa-1.1.0e
ERlvCmdRet RlvHandler::onForceRemAttach(const LLUUID& idObj, const RlvCommand& rlvCmd) const
{
	RLV_ASSERT(RLV_TYPE_FORCE == rlvCmd.getParamType());
	RLV_ASSERT( (RLV_BHVR_REMATTACH == rlvCmd.getBehaviourType()) || (RLV_BHVR_DETACH == rlvCmd.getBehaviourType()) );

	LLVOAvatar* pAvatar = gAgent.getAvatarObject();
	if (!pAvatar)
		return RLV_RET_FAILED;

	S32 idxAttachPt = 0; ERlvAttachGroupType eAttachGroup = RLV_ATTACHGROUP_INVALID;
	// @remattach:<attachpt>=force - force detach single worn attachment
	if ((idxAttachPt = getAttachPointIndex(rlvCmd.getOption(), true)) != 0)
	{
		LLViewerJointAttachment* pAttachPt = get_if_there(pAvatar->mAttachmentPoints, (S32)idxAttachPt, (LLViewerJointAttachment*)NULL);
		if (pAttachPt)
		{
			RlvForceWear rlvWear;
			rlvWear.forceDetach(pAttachPt);
			rlvWear.done();
		}
		return RLV_RET_SUCCESS;
	}
	// @remattach:<group>=force - force detach worn attachments belonging to <group>
	// @remattach=force         - force detach all worn attachments
	else if ( ((eAttachGroup = rlvGetAttachGroupTypeFromString(rlvCmd.getOption())) != RLV_ATTACHGROUP_INVALID) ||
		      (rlvCmd.getOption().empty()) )
	{
		RlvForceWear rlvWear;
		for (LLVOAvatar::attachment_map_t::const_iterator itAttach = pAvatar->mAttachmentPoints.begin();
				itAttach != pAvatar->mAttachmentPoints.end(); ++itAttach)
		{
			LLViewerJointAttachment* pAttachPt = itAttach->second;
			if ( (pAttachPt) && (pAttachPt->getObject()) &&
				 ((RLV_ATTACHGROUP_INVALID == eAttachGroup) || (rlvGetAttachGroupTypeFromIndex(pAttachPt->getGroup()) == eAttachGroup)) )
			{
				rlvWear.forceDetach(pAttachPt);
			}
		}
		rlvWear.done();
		return RLV_RET_SUCCESS;
	}
	return RLV_RET_FAILED_OPTION;
}

// Checked: 2009-12-18 (RLVa-1.1.0k) | Modified: RLVa-1.1.0i
ERlvCmdRet RlvHandler::onForceRemOutfit(const LLUUID& idObj, const RlvCommand& rlvCmd) const
{
	EWearableType wtOption = LLWearable::typeNameToType(rlvCmd.getOption()), wtType;
	if ( (WT_INVALID == wtOption) && (!rlvCmd.getOption().empty()) )
		return RLV_RET_FAILED_OPTION;

	RlvForceWear rlvWear;
	for (int idxType = 0; idxType < WT_COUNT; idxType++)
	{
		wtType = (EWearableType)idxType;
		if ( (wtType == wtOption) || (WT_INVALID == wtOption) )
			rlvWear.forceRemove(wtType);
	}
	rlvWear.done();
	return RLV_RET_SUCCESS;
}

// Checked: 2009-12-21 (RLVa-1.1.0j) | Modified: RLVa-1.1.0j
ERlvCmdRet RlvHandler::onForceSit(const LLUUID& idObj, const RlvCommand& rlvCmd) const
{
	LLViewerObject* pObj = NULL; LLUUID idTarget(rlvCmd.getOption());
	// Sanity checking - we need to know about the object and it should identify a prim/linkset
	if ( (idTarget.isNull()) || ((pObj = gObjectList.findObject(idTarget)) == NULL) || (LL_PCODE_VOLUME != pObj->getPCode()) )
		return RLV_RET_FAILED_OPTION;

	if (!canSit(pObj))
		return RLV_RET_FAILED_LOCK;

	// Copy/paste from handle_sit_or_stand() [see http://wiki.secondlife.com/wiki/AgentRequestSit]
	gMessageSystem->newMessageFast(_PREHASH_AgentRequestSit);
	gMessageSystem->nextBlockFast(_PREHASH_AgentData);
	gMessageSystem->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
	gMessageSystem->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
	gMessageSystem->nextBlockFast(_PREHASH_TargetObject);
	gMessageSystem->addUUIDFast(_PREHASH_TargetID, pObj->mID);
	// Offset: "a rough position in local coordinates for the edge to sit on"
	// (we might not even be looking at the object so I don't think we can supply the offset to an edge)
	gMessageSystem->addVector3Fast(_PREHASH_Offset, LLVector3::zero);
	pObj->getRegion()->sendReliableMessage();

	return RLV_RET_SUCCESS;
}

// Checked: 2009-12-18 (RLVa-1.1.0k) | Modified: RLVa-1.1.0i
ERlvCmdRet RlvHandler::onForceWear(const std::string& strPath, bool fAttach, bool fMatchAll) const
{
	LLViewerInventoryCategory* pFolder = getSharedFolder(strPath), *pRlvRoot = getSharedRoot();
	if ( (!pFolder)	|| (pFolder->getUUID() == pRlvRoot->getUUID()) )
		return (pRlvRoot != NULL) ? RLV_RET_FAILED_OPTION : RLV_RET_FAILED_NOSHAREDROOT;

	RlvForceWear rlvWear;
	rlvWear.forceFolder(pFolder,
		(fAttach) ? RlvForceWear::ACTION_ATTACH : RlvForceWear::ACTION_DETACH,
		(fMatchAll) ? (RlvForceWear::eWearFlags)(RlvForceWear::FLAG_DEFAULT | RlvForceWear::FLAG_MATCHALL) : (RlvForceWear::FLAG_DEFAULT));
	rlvWear.done();

	return RLV_RET_SUCCESS;
}

// ============================================================================
// Command handlers (RLV_TYPE_REPLY)
//

// Checked: 2009-11-26 (RLVa-1.1.0f) | Modified: RLVa-1.1.0f
ERlvCmdRet RlvHandler::processReplyCommand(const LLUUID& idObj, const RlvCommand& rlvCmd) const
{
	RLV_ASSERT(RLV_TYPE_REPLY == rlvCmd.getParamType());

	// Sanity check - <param> should specify a - valid - reply channel
	S32 nChannel;
	if ( (!LLStringUtil::convertToS32(rlvCmd.getParam(), nChannel)) || (!rlvIsValidReplyChannel(nChannel)) )
		return RLV_RET_FAILED_PARAM;

	ERlvCmdRet eRet = RLV_RET_SUCCESS; std::string strReply;
	switch (rlvCmd.getBehaviourType())
	{
		case RLV_BHVR_VERSION:			// @version=<channel>					- Checked: 2010-03-27 (RLVa-1.2.0b)
		case RLV_BHVR_VERSIONNEW:		// @versionnew=<channel>				- Checked: 2010-03-27 (RLVa-1.2.0b) | Added: RLVa-1.2.0b
			// NOTE: RLV will respond even if there's an option
			strReply = RlvStrings::getVersion(RLV_BHVR_VERSION == rlvCmd.getBehaviourType());
			break;
		case RLV_BHVR_VERSIONNUM:		// @versionnum=<channel>				- Checked: 2009-11-26 (RLVa-1.1.0f) | Added: RLVa-1.0.4b
			// NOTE: RLV will respond even if there's an option
			strReply = RlvStrings::getVersionNum();
			break;
		case RLV_BHVR_GETATTACH:		// @getattach[:<layer>]=<channel>
			eRet = onGetAttach(idObj, rlvCmd, strReply);
			break;
#ifdef RLV_EXTENSION_CMD_GETXXXNAMES
		case RLV_BHVR_GETATTACHNAMES:	// @getattachnames[:<grp>]=<channel>
		case RLV_BHVR_GETADDATTACHNAMES:// @getaddattachnames[:<grp>]=<channel>
		case RLV_BHVR_GETREMATTACHNAMES:// @getremattachnames[:<grp>]=<channel>
			eRet = onGetAttachNames(idObj, rlvCmd, strReply);
			break;
#endif // RLV_EXTENSION_CMD_GETXXXNAMES
		case RLV_BHVR_GETOUTFIT:		// @getoutfit[:<layer>]=<channel>
			eRet = onGetOutfit(idObj, rlvCmd, strReply);
			break;
#ifdef RLV_EXTENSION_CMD_GETXXXNAMES
		case RLV_BHVR_GETOUTFITNAMES:	// @getoutfitnames=<channel>
		case RLV_BHVR_GETADDOUTFITNAMES:// @getaddoutfitnames=<channel>
		case RLV_BHVR_GETREMOUTFITNAMES:// @getremoutfitnames=<channel>
			eRet = onGetOutfitNames(idObj, rlvCmd, strReply);
			break;
#endif // RLV_EXTENSION_CMD_GETXXXNAMES
		case RLV_BHVR_FINDFOLDER:		// @findfolder:<criteria>=<channel>
#ifdef RLV_EXTENSION_CMD_FINDFOLDERS
		case RLV_BHVR_FINDFOLDERS:		// @findfolders:<criteria>=<channel>
#endif // RLV_EXTENSION_CMD_FINDFOLDERS
			eRet = onFindFolder(idObj, rlvCmd, strReply);
			break;
		case RLV_BHVR_GETPATH:			// @getpath[:<option>]=<channel>
			eRet = onGetPath(idObj, rlvCmd, strReply);
			break;
		case RLV_BHVR_GETINV:			// @getinv[:<path>]=<channel>
			eRet = onGetInv(idObj, rlvCmd, strReply);
			break;
		case RLV_BHVR_GETINVWORN:		// @getinvworn[:path]=<channel>
			eRet = onGetInvWorn(idObj, rlvCmd, strReply);
			break;
		case RLV_BHVR_GETSITID:			// @getsitid=<channel>					- Checked: 2009-11-26 (RLVa-1.1.0f)
			{
				// NOTE: RLV 1.16.1 returns a NULL UUID if we're not sitting
				LLVOAvatar* pAvatar = gAgent.getAvatarObject(); LLUUID idSitObj;
				if ( (pAvatar) && (pAvatar->mIsSitting) )
				{
					// LLVOAvatar inherits from 2 classes so make sure we get the right vfptr
					LLViewerObject* pAvatarObj = dynamic_cast<LLViewerObject*>(pAvatar), *pParent;
					// (If there is a parent, we need to upcast it from LLXform to LLViewerObject to get its UUID)
					if ( (pAvatarObj) && ((pParent = static_cast<LLViewerObject*>(pAvatarObj->getRoot())) != pAvatarObj) )
						idSitObj = pParent->getID();
				}
				strReply = idSitObj.asString();
			}
			break;
		case RLV_BHVR_GETSTATUS:		// @getstatus[:<option>]=<channel>		- Checked: 2009-11-26 (RLVa-1.1.0f) | Modified: RLVa-1.1.0f
			{
				// NOTE: specification says response should start with '/' but RLV-1.16.1 returns an empty string when no rules are set
				rlv_object_map_t::const_iterator itObj = m_Objects.find(idObj);
				if (itObj != m_Objects.end())
					strReply = itObj->second.getStatusString(rlvCmd.getOption());
			}
			break;
		case RLV_BHVR_GETSTATUSALL:		// @getstatusall[:<option>]=<channel>	- Checked: 2009-11-26 (RLVa-1.1.0f) | Modified: RLVa-1.1.0f
			{
				// NOTE: specification says response should start with '/' but RLV-1.16.1 returns an empty string when no rules are set
				for (rlv_object_map_t::const_iterator itObj = m_Objects.begin(); itObj != m_Objects.end(); ++itObj)
					strReply += itObj->second.getStatusString(rlvCmd.getOption());
			}
			break;
		case RLV_BHVR_UNKNOWN:
			// Pass unknown commands on to registered command handlers
			return (notifyCommandHandlers(&RlvCommandHandler::onReplyCommand, idObj, rlvCmd, eRet, false)) ? eRet : RLV_RET_FAILED_UNKNOWN;
		default:
			// Fail with "Invalid param" if none of the above handled it
			return RLV_RET_FAILED_PARAM;
	}

	// If we made it this far then:
	//   - the command was handled successfully so we send off the response
	//   - the command failed but we still send off an (empty) response to keep the issuing script from blocking
	rlvSendChatReply(nChannel, strReply);

	return eRet;
}

// Checked: 2009-11-24 (RLVa-1.1.0f) | Modified: RLVa-1.1.0f
ERlvCmdRet RlvHandler::onFindFolder(const LLUUID& idObj, const RlvCommand& rlvCmd, std::string& strReply) const
{
	RLV_ASSERT(RLV_TYPE_REPLY == rlvCmd.getParamType());
	RLV_ASSERT( (RLV_BHVR_FINDFOLDER == rlvCmd.getBehaviourType()) || (RLV_BHVR_FINDFOLDERS == rlvCmd.getBehaviourType()) );

	// (Compatibility: RLV 1.16.1 returns the first random folder it finds while we return a blank on no option)
	if (rlvCmd.getOption().empty())
		return RLV_RET_FAILED_OPTION;

	LLInventoryModel::cat_array_t folders;
	if (findSharedFolders(rlvCmd.getOption(), folders))
	{
		if (RLV_BHVR_FINDFOLDER == rlvCmd.getBehaviourType())
		{
			// We need to return an "in depth" result so whoever has the most '/' is our lucky winner
			// (maxSlashes needs to be initialized to -1 since children of the #RLV folder won't have '/' in their shared path)
			int maxSlashes = -1, curSlashes; std::string strFolderName;
			for (S32 idxFolder = 0, cntFolder = folders.count(); idxFolder < cntFolder; idxFolder++)
			{
				strFolderName = getSharedPath(folders.get(idxFolder));

				curSlashes = std::count(strFolderName.begin(), strFolderName.end(), '/');
				if (curSlashes > maxSlashes)
				{
					maxSlashes = curSlashes;
					strReply = strFolderName;
				}
			}
		}
		else if (RLV_BHVR_FINDFOLDERS == rlvCmd.getBehaviourType())
		{
			for (S32 idxFolder = 0, cntFolder = folders.count(); idxFolder < cntFolder; idxFolder++)
			{
				if (!strReply.empty())
					strReply.push_back(',');
				strReply += getSharedPath(folders.get(idxFolder));
			}
		}
	}
	return RLV_RET_SUCCESS;
}

// Checked: 2009-11-24 (RLVa-1.1.0f) | Modified: RLVa-1.1.0e
ERlvCmdRet RlvHandler::onGetAttach(const LLUUID& idObj, const RlvCommand& rlvCmd, std::string& strReply) const
{
	RLV_ASSERT(RLV_TYPE_REPLY == rlvCmd.getParamType());
	RLV_ASSERT(RLV_BHVR_GETATTACH == rlvCmd.getBehaviourType());

	LLVOAvatar* pAvatar = gAgent.getAvatarObject();
	if (!pAvatar)
		return RLV_RET_FAILED;

	// Sanity check - <option> should specify an attachment point or be empty
	S32 idxAttachPt = getAttachPointIndex(rlvCmd.getOption(), true);
	if ( (idxAttachPt == 0) && (!rlvCmd.getOption().empty()) )
		return RLV_RET_FAILED_OPTION;

	// If we're fetching all worn attachments then the reply should start with 0
	if (0 == idxAttachPt)
		strReply.push_back('0');

	for (LLVOAvatar::attachment_map_t::const_iterator itAttach = pAvatar->mAttachmentPoints.begin();
			itAttach != pAvatar->mAttachmentPoints.end(); ++itAttach)
	{
		LLViewerJointAttachment* pAttachPt = itAttach->second;
		if ( (0 == idxAttachPt) || (itAttach->first == idxAttachPt) )
		{
			bool fWorn = (pAttachPt->getObject() != NULL) &&
				( (!RlvSettings::getHideLockedAttach()) ||
				  (RlvForceWear::isForceDetachable(pAttachPt, true, gObjectList.findObject(idObj))) );
			strReply.push_back( (fWorn) ? '1' : '0' );
		}
	}
	return RLV_RET_SUCCESS;
}

// Checked: 2009-10-19 (RLVa-1.1.0f) | Added: RLVa-1.1.0e
ERlvCmdRet RlvHandler::onGetAttachNames(const LLUUID& idObj, const RlvCommand& rlvCmd, std::string& strReply) const
{
	RLV_ASSERT(RLV_TYPE_REPLY == rlvCmd.getParamType());
	RLV_ASSERT( (RLV_BHVR_GETATTACHNAMES == rlvCmd.getBehaviourType()) || (RLV_BHVR_GETADDATTACHNAMES == rlvCmd.getBehaviourType()) ||
		        (RLV_BHVR_GETREMATTACHNAMES == rlvCmd.getBehaviourType()) );

	LLVOAvatar* pAvatar = gAgent.getAvatarObject();
	if (!pAvatar)
		return RLV_RET_FAILED;

	ERlvAttachGroupType eAttachGroup = rlvGetAttachGroupTypeFromString(rlvCmd.getOption());
	for (LLVOAvatar::attachment_map_t::const_iterator itAttach = pAvatar->mAttachmentPoints.begin();
			itAttach != pAvatar->mAttachmentPoints.end(); ++itAttach)
	{
		LLViewerJointAttachment* pAttachPt = itAttach->second;
		if ( (RLV_ATTACHGROUP_INVALID == eAttachGroup) || (rlvGetAttachGroupTypeFromIndex(pAttachPt->getGroup()) == eAttachGroup) )
		{
			bool fAdd = false;
			switch (rlvCmd.getBehaviourType())
			{
				case RLV_BHVR_GETATTACHNAMES:		// Every attachment point that has an attached object (locked or unlocked)
					fAdd = (pAttachPt->getObject() != NULL);
					break;
				case RLV_BHVR_GETADDATTACHNAMES:	// Every attachment point that can be worn on (but ignore any locks set by the issuer)
					fAdd = (!isLockedAttachmentExcept(itAttach->first, RLV_LOCK_ADD, gObjectList.findObject(idObj))) &&
						   ( (pAttachPt->getObject() == NULL) ||
						     (!isLockedAttachmentExcept(itAttach->first, RLV_LOCK_REMOVE, gObjectList.findObject(idObj))) );
					break;
				case RLV_BHVR_GETREMATTACHNAMES:	// Every attachment point that can be detached (but ignore any locks set by the issuer)
					fAdd = RlvForceWear::isForceDetachable(pAttachPt, true, gObjectList.findObject(idObj));
					break;
				default:
					break;
			}

			if (fAdd)
			{
				if (!strReply.empty())
					strReply.push_back(',');
				strReply.append(pAttachPt->getName());
			}
		}
	}
	return RLV_RET_SUCCESS;
}

// Checked: 2009-11-21 (RLVa-1.1.0f) | Modified: RLVa-1.1.0f
ERlvCmdRet RlvHandler::onGetInv(const LLUUID& idObj, const RlvCommand& rlvCmd, std::string& strReply) const
{
	RLV_ASSERT(RLV_TYPE_REPLY == rlvCmd.getParamType());
	RLV_ASSERT(RLV_BHVR_GETINV == rlvCmd.getBehaviourType());

	LLViewerInventoryCategory* pFolder = getSharedFolder(rlvCmd.getOption());
	if (!pFolder)
		return (getSharedRoot() != NULL) ? RLV_RET_FAILED_OPTION : RLV_RET_FAILED_NOSHAREDROOT;

	LLInventoryModel::cat_array_t* pFolders;
	LLInventoryModel::item_array_t* pItems;
	gInventory.getDirectDescendentsOf(pFolder->getUUID(), pFolders, pItems);
	if (!pFolders)
		return RLV_RET_FAILED;

	for (S32 idxFolder = 0, cntFolder = pFolders->count(); idxFolder < cntFolder; idxFolder++)
	{
		// Return all folders that:
		//   - aren't hidden
		//   - aren't a folded folder (only really matters when "Enable Legacy Naming" is enabled - see related blog post)
		//     (we can skip checking for .<composite> folders since the ones we'll want to hide start with '.' anyway)
		const std::string& strFolder = pFolders->get(idxFolder)->getName();
		if ( (!strFolder.empty()) && (RLV_FOLDER_PREFIX_HIDDEN != strFolder[0]) &&
			 (!isFoldedFolder(pFolders->get(idxFolder).get(), true, false)) )
		{
			if (!strReply.empty())
				strReply.push_back(',');
			strReply += strFolder;
		}
	}
	return RLV_RET_SUCCESS;
}

struct rlv_wear_info { U32 cntWorn, cntTotal, cntChildWorn, cntChildTotal; };

// Checked: 2009-11-27 (RLVa-1.1.0f) | Modified: RLVa-1.1.0f
ERlvCmdRet RlvHandler::onGetInvWorn(const LLUUID& idObj, const RlvCommand& rlvCmd, std::string& strReply) const
{
	// Sanity check - getAvatarObject() can't be NULL [see RlvForceWear::isWearingItem()]
	if (!gAgent.getAvatarObject())
		return RLV_RET_FAILED;
	// Sanity check - folder should exist and not be hidden
	LLViewerInventoryCategory* pFolder = getSharedFolder(rlvCmd.getOption());
	if ( (!pFolder) || (pFolder->getName().empty()) || (RLV_FOLDER_PREFIX_HIDDEN == pFolder->getName()[0]) )
		return (getSharedRoot() != NULL) ? RLV_RET_FAILED_OPTION : RLV_RET_FAILED_NOSHAREDROOT;

	// Collect everything @attachall would be attaching
	LLInventoryModel::cat_array_t  folders;
	LLInventoryModel::item_array_t items;
	RlvWearableItemCollector functor(pFolder->getUUID(), true, true);
	gInventory.collectDescendentsIf(pFolder->getUUID(), folders, items, FALSE, functor);

	rlv_wear_info wi = {0};

	// Add all the folders to a lookup map
	std::map<LLUUID, rlv_wear_info> mapFolders;
	mapFolders.insert(std::pair<LLUUID, rlv_wear_info>(pFolder->getUUID(), wi));
	for (S32 idxFolder = 0, cntFolder = folders.count(); idxFolder < cntFolder; idxFolder++)
		mapFolders.insert(std::pair<LLUUID, rlv_wear_info>(folders.get(idxFolder)->getUUID(), wi));

	// Iterate over all the found items
	LLViewerInventoryItem* pItem; std::map<LLUUID, rlv_wear_info>::iterator itFolder;
	for (S32 idxItem = 0, cntItem = items.count(); idxItem < cntItem; idxItem++)
	{
		pItem = items.get(idxItem);

		// The "folded parent" is the folder this item should be considered a direct descendent of (may or may not match actual parent)
		const LLUUID& idParent = functor.getFoldedParent(pItem->getParentUUID());

		// Walk up the tree: sooner or later one of the parents will be a folder in the map
		LLViewerInventoryCategory* pParent = gInventory.getCategory(idParent);
		while ( (itFolder = mapFolders.find(pParent->getUUID())) == mapFolders.end() )
			pParent = gInventory.getCategory(pParent->getParentUUID());

		U32 &cntWorn  = (idParent == pParent->getUUID()) ? itFolder->second.cntWorn : itFolder->second.cntChildWorn,
			&cntTotal = (idParent == pParent->getUUID()) ? itFolder->second.cntTotal : itFolder->second.cntChildTotal;

		if (RlvForceWear::isWearingItem(pItem))
			cntWorn++;
		cntTotal++;
	}

	// Extract the result for the main folder
	itFolder = mapFolders.find(pFolder->getUUID());
	wi.cntWorn = itFolder->second.cntWorn;
	wi.cntTotal = itFolder->second.cntTotal;
	mapFolders.erase(itFolder);

	// Build the result for each child folder
	for (itFolder = mapFolders.begin(); itFolder != mapFolders.end(); ++itFolder)
	{
		rlv_wear_info& wiFolder = itFolder->second;

		wi.cntChildWorn += wiFolder.cntWorn + wiFolder.cntChildWorn;
		wi.cntChildTotal += wiFolder.cntTotal + wiFolder.cntChildTotal;

		strReply += llformat(",%s|%d%d", gInventory.getCategory(itFolder->first)->getName().c_str(),
		 (0 == wiFolder.cntTotal) ? 0 : (0 == wiFolder.cntWorn) ? 1 : (wiFolder.cntWorn != wiFolder.cntTotal) ? 2 : 3,
		 (0 == wiFolder.cntChildTotal) ? 0 : (0 == wiFolder.cntChildWorn) ? 1 : (wiFolder.cntChildWorn != wiFolder.cntChildTotal) ? 2 : 3
		);
	}

	// Now just prepend the root and done
	strReply = llformat("|%d%d", (0 == wi.cntTotal) ? 0 : (0 == wi.cntWorn) ? 1 : (wi.cntWorn != wi.cntTotal) ? 2 : 3,
		(0 == wi.cntChildTotal) ? 0 : (0 == wi.cntChildWorn) ? 1 : (wi.cntChildWorn != wi.cntChildTotal) ? 2: 3) + strReply;

	return RLV_RET_SUCCESS;
}

// Checked: 2009-11-24 (RLVa-1.1.0f) | Modified: RLVa-1.1.0e
ERlvCmdRet RlvHandler::onGetOutfit(const LLUUID& idObj, const RlvCommand& rlvCmd, std::string& strReply) const
{
	RLV_ASSERT(RLV_TYPE_REPLY == rlvCmd.getParamType());
	RLV_ASSERT(RLV_BHVR_GETOUTFIT == rlvCmd.getBehaviourType());

	// (Compatibility: RLV 1.16.1 will execute @getoutfit=<channel> if <layer> is invalid while we just return failure)
	EWearableType layerType = LLWearable::typeNameToType(rlvCmd.getOption());
	if ( (WT_INVALID == layerType) && (!rlvCmd.getOption().empty()) )
		return RLV_RET_FAILED_OPTION;

	const EWearableType layerTypes[] =
		{
			WT_GLOVES, WT_JACKET, WT_PANTS, WT_SHIRT, WT_SHOES, WT_SKIRT, WT_SOCKS,
			WT_UNDERPANTS, WT_UNDERSHIRT, WT_SKIN, WT_EYES, WT_HAIR, WT_SHAPE, WT_ALPHA, WT_TATTOO
		};

	for (int idx = 0, cnt = sizeof(layerTypes) / sizeof(EWearableType); idx < cnt; idx++)
		if ( (WT_INVALID == layerType) || (layerTypes[idx] == layerType) )
		{
			// We never hide body parts, even if they're "locked" and we're hiding locked layers
			// (nor do we hide a layer if the issuing object is the only one that has this layer locked)
			bool fWorn = (gAgent.getWearable(layerTypes[idx])) &&
				( (!RlvSettings::getHideLockedLayers()) ||
				  (LLAssetType::AT_BODYPART == LLWearable::typeToAssetType(layerTypes[idx])) ||
				  ( (isRemovableExcept(layerTypes[idx], idObj)) &&
				    (isStrippable(gAgent.getWearableItem(layerTypes[idx]))) ) );
			strReply.push_back( (fWorn) ? '1' : '0' );
		}

	return RLV_RET_SUCCESS;
}

// Checked: 2009-11-21 (RLVa-1.1.0f) | Added: RLVa-1.1.0e
ERlvCmdRet RlvHandler::onGetOutfitNames(const LLUUID& idObj, const RlvCommand& rlvCmd, std::string& strReply) const
{
	RLV_ASSERT(RLV_TYPE_REPLY == rlvCmd.getParamType());
	RLV_ASSERT( (RLV_BHVR_GETOUTFITNAMES == rlvCmd.getBehaviourType()) || (RLV_BHVR_GETADDOUTFITNAMES == rlvCmd.getBehaviourType()) ||
		        (RLV_BHVR_GETREMOUTFITNAMES == rlvCmd.getBehaviourType()) );

	// Sanity check - all these commands are optionless
	if (!rlvCmd.getOption().empty())
		return RLV_RET_FAILED_OPTION;

	for (int idxType = 0; idxType < WT_COUNT; idxType++)
	{
		bool fAdd = false; EWearableType wtType = (EWearableType)idxType;
		switch (rlvCmd.getBehaviourType())
		{
			case RLV_BHVR_GETOUTFITNAMES:		// Every layer that's worn
				fAdd = (gAgent.getWearable(wtType) != NULL);
				break;
			case RLV_BHVR_GETADDOUTFITNAMES:	// Every layer that can be worn on (but ignore any locks set by the issuer)
				fAdd = (isWearable(wtType)) && ( (gAgent.getWearable(wtType) == NULL) || (isRemovableExcept(wtType, idObj)) );
				break;
			case RLV_BHVR_GETREMOUTFITNAMES:	// Every layer that can be removed (but ignore any locks set by the issuer)
				fAdd = (gAgent.getWearable(wtType) != NULL) &&
					   (LLAssetType::AT_BODYPART != LLWearable::typeToAssetType(wtType)) &&
					   (isRemovableExcept(wtType, idObj)) && (isStrippable(gAgent.getWearableItem(wtType)));
				break;
			default:
				break;
		}

		if (fAdd)
		{
			if (!strReply.empty())
				strReply.push_back(',');
			strReply.append(LLWearable::typeToTypeName((EWearableType)idxType));
		}
	}
	return RLV_RET_SUCCESS;
}

// Checked: 2009-11-26 (RLVa-1.1.0f) | Modified: RLVa-1.1.0f
ERlvCmdRet RlvHandler::onGetPath(const LLUUID& idObj, const RlvCommand& rlvCmd, std::string& strReply) const
{
	// NOTE: @attachthis/attachallthis/detachthis/detachallthis call us directly to simulate @attach:getpath[:<option>]=force

	// Sanity check - no need to go through all this trouble if we don't have a shared root
	LLViewerInventoryCategory* pRlvRoot = getSharedRoot();
	if (!pRlvRoot)
		return RLV_RET_FAILED_NOSHAREDROOT;

	EWearableType wtType = LLWearable::typeNameToType(rlvCmd.getOption()); LLUUID idItem;
	if (WT_INVALID != wtType)				// <option> can be a clothing layer
	{
		idItem = gAgent.getWearableItem(wtType);
	}
	else
	{
		LLViewerJointAttachment* pAttachPt = NULL;

		if (rlvCmd.getOption().empty())		// ... or it can be empty (in which case we act on the object that issued the command)
		{
			LLViewerObject* pObj = gObjectList.findObject(idObj);
			if ( (pObj) && (pObj->isAttachment()) && (gAgent.getAvatarObject()) )
				pAttachPt = gAgent.getAvatarObject()->getTargetAttachmentPoint(pObj);
		}
		else								// ... or it can specify an attachment point
		{
			pAttachPt = getAttachPoint(rlvCmd.getOption(), true);
		}

		// If we found something get its inventory item UUID, otherwise return failure
		if (!pAttachPt)
			return RLV_RET_FAILED_OPTION;
		idItem = pAttachPt->getItemID();
	}

	// If what we found is under the shared root then get its path
	if ( (!idItem.isNull()) && (gInventory.isObjectDescendentOf(idItem, pRlvRoot->getUUID())) )
	{
		LLInventoryItem* pItem = gInventory.getItem(idItem);
		if (pItem)
		{
			// (unless the containing folder is a folded folder in which case we need its parent)
			LLViewerInventoryCategory* pFolder = gInventory.getCategory(pItem->getParentUUID());
			strReply = getSharedPath( (!isFoldedFolder(pFolder, true, true)) ? pFolder : gInventory.getCategory(pFolder->getParentUUID()) );
		}
	}
	return RLV_RET_SUCCESS;
}

// ============================================================================
