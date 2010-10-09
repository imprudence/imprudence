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

#include "llagent.h"
#include "llcachename.h"
#include "llscrolllistctrl.h"
#include "lluictrlfactory.h"
#include "llviewerinventory.h"
#include "llviewerobjectlist.h"
#include "llvoavatar.h"

#include "rlvfloaterbehaviour.h"
#include "rlvhandler.h"

// ============================================================================

RlvFloaterBehaviour::RlvFloaterBehaviour(const LLSD& key) 
	: LLFloater(std::string("rlvBehaviours"))
{
	LLUICtrlFactory::getInstance()->buildFloater(this, "floater_rlv_behaviour.xml");
}


void RlvFloaterBehaviour::show(void*)
{
	RlvFloaterBehaviour::showInstance();
}

void RlvFloaterBehaviour::refreshAll()
{
	LLVOAvatar* pAvatar = gAgent.getAvatarObject();
	LLCtrlListInterface* pList = childGetListInterface("behaviour_list");
	const rlv_object_map_t* pRlvObjects = gRlvHandler.getObjectMap();
	if ( (!pAvatar) || (!pList) || (!pRlvObjects) )
		return;

	pList->operateOnAll(LLCtrlListInterface::OP_DELETE);

	for (rlv_object_map_t::const_iterator itObj = pRlvObjects->begin(), endObj = pRlvObjects->end(); itObj != endObj; ++itObj)
	{
		std::string strName = itObj->first.asString();

		LLViewerInventoryItem* pItem = NULL;
		LLViewerObject* pObj = gObjectList.findObject(itObj->first);
		if (pObj)
		{
			LLViewerJointAttachment* pAttachPt = 
				get_if_there(pAvatar->mAttachmentPoints, gRlvHandler.getAttachPointIndex(pObj), (LLViewerJointAttachment*)NULL);
			if (pAttachPt)
			{
				pItem = gInventory.getItem(pAttachPt->getItemID());
			}
		}

		if (pItem)
			strName = pItem->getName();

		const rlv_command_list_t* pCommands = itObj->second.getCommandList();
		for (rlv_command_list_t::const_iterator itCmd = pCommands->begin(), endCmd = pCommands->end(); itCmd != endCmd; ++itCmd)
		{
			std::string strBhvr = itCmd->asString(); LLUUID uuid(itCmd->getOption());
			if (uuid.notNull())
			{
				std::string strLookup;
				if ( (gCacheName->getFullName(uuid, strLookup)) || (gCacheName->getGroupName(uuid, strLookup)) )
				{
					if (strLookup.find("???") == std::string::npos)
						strBhvr.assign(itCmd->getBehaviour()).append(":").append(strLookup);
				}
				else if (m_PendingLookup.end() == std::find(m_PendingLookup.begin(), m_PendingLookup.end(), uuid))
				{
					gCacheName->get(uuid, FALSE, onAvatarNameLookup, this);
					m_PendingLookup.push_back(uuid);
				}
			}

			LLSD element;

			// Restriction column
			element["columns"][0]["column"] = "behaviour";
			element["columns"][0]["value"] = strBhvr;
			element["columns"][0]["font"] = "SANSSERIF";
			element["columns"][0]["font-style"] = "NORMAL";

			// Object Name column
			element["columns"][1]["column"] = "name";
			element["columns"][1]["value"] = strName;
			element["columns"][1]["font"] = "SANSSERIF";
			element["columns"][1]["font-style"] = "NORMAL";

			pList->addElement(element, ADD_BOTTOM);
		}
	}
}

// ============================================================================
/*
 * LLFloater overrides
 */

BOOL RlvFloaterBehaviour::canClose()
{
	return !LLApp::isExiting();
}

void RlvFloaterBehaviour::onOpen()
{
	gRlvHandler.addBehaviourObserver(this);

	refreshAll();
}

void RlvFloaterBehaviour::onClose(bool fQuitting)
{
	LLFloater::setVisible(FALSE);

	gRlvHandler.removeBehaviourObserver(this);

	for (std::list<LLUUID>::const_iterator itLookup = m_PendingLookup.begin(); itLookup != m_PendingLookup.end(); ++itLookup)
	{
		gCacheName->cancelCallback(*itLookup, onAvatarNameLookup, this);
	}
	m_PendingLookup.clear();
}

BOOL RlvFloaterBehaviour::postBuild()
{
	return TRUE;
}

// ============================================================================
/*
 * RlvBehaviourObserver overrides
 */

void RlvFloaterBehaviour::changed(const RlvCommand& /*rlvCmd*/, bool /*fInternal*/)
{
	refreshAll();
}

// ============================================================================

void RlvFloaterBehaviour::onAvatarNameLookup(const LLUUID& uuid, const std::string& strFirst, const std::string& strLast, BOOL fGroup, void* pParam)
{
	RlvFloaterBehaviour* pSelf = (RlvFloaterBehaviour*)pParam;

	std::list<LLUUID>::iterator itLookup = std::find(pSelf->m_PendingLookup.begin(), pSelf->m_PendingLookup.end(), uuid);
	if (itLookup != pSelf->m_PendingLookup.end())
		pSelf->m_PendingLookup.erase(itLookup);

	pSelf->refreshAll();
}

// ============================================================================
