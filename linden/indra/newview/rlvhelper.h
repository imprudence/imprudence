#ifndef RLV_HELPER_H
#define RLV_HELPER_H

#include "llagent.h"
#include "llboost.h"
#include "llgesturemgr.h"
#include "llinventorymodel.h"
#include "llviewerinventory.h"
#include "llvoavatar.h"
#include "llwlparamset.h"
#include "rlvdefines.h"
#include "rlvcommon.h"

#ifdef LL_WINDOWS
	#pragma warning (push)
	#pragma warning (disable : 4702) // warning C4702: unreachable code
#endif
#include <boost/variant.hpp>
#ifdef LL_WINDOWS
	#pragma warning (pop)
#endif

// ============================================================================
// RlvCommand
//

class RlvCommand
{
public:
	explicit RlvCommand(const std::string& strCommand);

	/*
	 * Member functions
	 */
public:
	std::string        asString() const;
	const std::string& getBehaviour() const     { return m_strBehaviour; }
	ERlvBehaviour      getBehaviourType() const { return m_eBehaviour; }
	const std::string& getOption() const        { return m_strOption; }
	const std::string& getParam() const         { return m_strParam; }
	ERlvParamType      getParamType() const     { return m_eParamType; }
	bool               isStrict() const			{ return m_fStrict; }
	bool               isValid() const          { return m_fValid; }

	static ERlvBehaviour      getBehaviourFromString(const std::string& strBhvr, bool* pfStrict = NULL);
	static const std::string& getStringFromBehaviour(ERlvBehaviour eBhvr);
	static bool               hasStrictVariant(ERlvBehaviour eBhvr);

	static void initLookupTable();
protected:
	static bool parseCommand(const std::string& strCommand, std::string& strBehaviour, std::string& strOption,  std::string& strParam);

	/*
	 * Operators
	 */
public:
	bool operator ==(const RlvCommand&) const;

	/*
	 * Member variables
	 */
protected:
	bool         	m_fValid;
	std::string  	m_strBehaviour;
	ERlvBehaviour	m_eBehaviour;
	bool            m_fStrict;
	std::string  	m_strOption;
	std::string  	m_strParam;
	ERlvParamType	m_eParamType;

	typedef std::map<std::string, ERlvBehaviour> RlvBhvrTable;
	static RlvBhvrTable m_BhvrMap;

	friend class RlvHandler;
};
typedef std::list<RlvCommand> rlv_command_list_t;

// ============================================================================
// RlvObject
//

class RlvObject
{
public:
	RlvObject(const LLUUID& idObj);

	/*
	 * Member functions
	 */
public:
	bool addCommand(const RlvCommand& rlvCmd);
	bool removeCommand(const RlvCommand& rlvCmd);

	std::string getStatusString(const std::string& strMatch) const;
	bool        hasBehaviour(ERlvBehaviour eBehaviour, bool fStrictOnly) const;
	bool        hasBehaviour(ERlvBehaviour eBehaviour, const std::string& strOption, bool fStrictOnly) const;

	const rlv_command_list_t* getCommandList() const { return &m_Commands; }

	/*
	 * Member variables
	 */
protected:
	LLUUID             m_UUID;				// The object's UUID
	S32                m_idxAttachPt;		// The object's attachment point (or 0 if it's not an attachment)
	bool               m_fLookup;			// TRUE if the object existed in gObjectList at one point in time
	S16                m_nLookupMisses;		// Count of unsuccessful lookups in gObjectList by the GC
	rlv_command_list_t m_Commands;			// List of behaviours held by this object (in the order they were received)

	friend class RlvHandler;
};

// ============================================================================
// RlvAttachmentManager - Self contained helper class that automagically takes care of the nitty gritty of force detaching/reattaching
//

class RlvAttachmentManager
{
public:
	RlvAttachmentManager() : m_pTimer(NULL) {}
	~RlvAttachmentManager() { delete m_pTimer; }

	/*
	 * Member functions
	 */
public:
	// NOTE: the following two do *not* respect attachment locks so use with care
	static void attach(const LLUUID& idItem, S32 idxAttachPt);
	static void detach(LLViewerJointAttachment* pAttachPt);
protected:
	void startTimer() { if (!m_pTimer) m_pTimer = new RlvAttachmentManagerTimer(this); }

	/*
	 * Event handlers
	 */
public:
	void onAttach(LLViewerJointAttachment* pAttachPt);
	void onDetach(LLViewerJointAttachment* pAttachPt);
	void onSavedAssetIntoInventory(const LLUUID& idItem);
	BOOL onTimer();
	void onWearAttachment(const LLUUID& idItem);

	/*
	 * Member variables
	 */
protected:
	typedef std::map<S32, LLUUID> rlv_detach_map_t;
	rlv_detach_map_t m_PendingDetach;

	struct RlvReattachInfo
	{
		RlvReattachInfo(const LLUUID& itemid) : idItem(itemid), fAssetSaved(false), tsAttach(0) 
			{ tsDetach = LLFrameTimer::getElapsedSeconds(); }

		LLUUID idItem;
		bool   fAssetSaved;
		F64    tsDetach;
		F64    tsAttach;
	protected:
		RlvReattachInfo();
	};
	typedef std::map<S32, RlvReattachInfo> rlv_attach_map_t;
	rlv_attach_map_t m_PendingAttach;

	struct RlvWearInfo
	{
		RlvWearInfo(LLUUID itemid) : idItem(itemid) { tsWear = LLFrameTimer::getElapsedSeconds(); }

		LLUUID idItem;
		F64    tsWear;
		std::map<S32, LLUUID> attachPts;
	protected:
		RlvWearInfo();
	};
	typedef std::map<LLUUID, RlvWearInfo> rlv_wear_map_t;
	rlv_wear_map_t   m_PendingWear;

	class RlvAttachmentManagerTimer : public LLEventTimer
	{
	public:
		RlvAttachmentManagerTimer(RlvAttachmentManager* pMgr) : LLEventTimer(10), m_pMgr(pMgr) {}
		virtual ~RlvAttachmentManagerTimer() { m_pMgr->m_pTimer = NULL; }
		virtual BOOL tick() { return m_pMgr->onTimer(); }
		RlvAttachmentManager* m_pMgr;
	} *m_pTimer;
};

// ============================================================================
// RlvCriteriaCategoryCollector - Criteria based folder matching filter used by @findfolder and @findfolders
//

class RlvCriteriaCategoryCollector : public LLInventoryCollectFunctor
{
public:
	RlvCriteriaCategoryCollector(const std::string& strCriteria)
	{
		std::string::size_type idxIt, idxLast = 0;
		while (idxLast < strCriteria.length())
		{
			idxIt = strCriteria.find("&&", idxLast);
			if (std::string::npos == idxIt)
				idxIt = strCriteria.length();
			if (idxIt != idxLast)
				m_Criteria.push_back(strCriteria.substr(idxLast, idxIt - idxLast));
			idxLast = idxIt + 2;
		}
	}
	virtual ~RlvCriteriaCategoryCollector() {}

	virtual bool operator()(LLInventoryCategory* pFolder, LLInventoryItem* pItem)
	{
		if ( (!pFolder) || (m_Criteria.empty()) )	// We're only interested in matching folders, we don't care about items
			return false;							// (if there are no criteria then we don't want to return a match)

		std::string strFolderName = pFolder->getName();
		LLStringUtil::toLower(strFolderName);

		// NOTE: hidden or "give to #RLV" folders can never be a match
		if ( (strFolderName.empty()) ||
			 (RLV_FOLDER_PREFIX_HIDDEN == strFolderName[0]) || (RLV_FOLDER_PREFIX_PUTINV == strFolderName[0]) )
		{
			return false;
		}

		for (std::list<std::string>::const_iterator itCrit = m_Criteria.begin(); itCrit != m_Criteria.end(); ++itCrit)
			if (std::string::npos == strFolderName.find(*itCrit))	// Return false on the first mismatch
				return false;
		return true;
	}

protected:
	std::list<std::string> m_Criteria;
};

// ============================================================================
// RlvWearableItemCollector - Inventory item filter used by attach/detach/attachall/detachall/getinvworn
//

class RlvWearableItemCollector : public LLInventoryCollectFunctor
{
public:
	RlvWearableItemCollector(const LLUUID& idFolder, bool fAttach, bool fMatchAll)
		: m_idFolder(idFolder), m_fAttach(fAttach), m_fMatchAll(fMatchAll)
	{
		m_Wearable.push_back(idFolder);
	}
	virtual ~RlvWearableItemCollector() {}

	virtual bool operator()(LLInventoryCategory* pFolder, LLInventoryItem* pItem);

	const LLUUID& getFoldedParent(const LLUUID& idFolder) const;
protected:
	bool         m_fAttach;
	bool         m_fMatchAll;
	const LLUUID m_idFolder;

	bool onCollectFolder(const LLInventoryCategory* pFolder);
	bool onCollectItem(const LLInventoryItem* pItem);

	std::list<LLUUID> m_Tentative;
	std::list<LLUUID> m_Wearable;

	std::map<LLUUID, LLUUID> m_Folding;
};

// ============================================================================
// RlvForceWear
//

class RlvForceWear
{
public:
	// Folders
	enum eWearAction { ACTION_ATTACH, ACTION_DETACH };
	enum eWearFlags { FLAG_NONE = 0x00, FLAG_MATCHALL = 0x01, FLAG_DEFAULT = FLAG_NONE };
	void forceFolder(const LLViewerInventoryCategory* pFolder, eWearAction eAction, eWearFlags eFlags);

	// Attachments
	static bool isForceDetachable(LLViewerJointAttachment* pAttachPt, bool fCheckComposite = true, LLViewerObject* pExceptObj = NULL);
	void forceDetach(LLViewerJointAttachment* ptAttachPt);

	// Wearables
	static bool isForceRemovable(EWearableType wtType, bool fCheckComposite = true, const LLUUID& idExcept = LLUUID::null);
	void forceRemove(EWearableType wtType);

	// General purpose
	static bool isWearableItem(const LLInventoryItem* pItem);
	static bool isWearingItem(const LLInventoryItem* pItem);

public:
	void done()	{ processRem(); processAdd(); }
protected:
	void processAdd();
	void processRem();
	static void	onWearableArrived(LLWearable* pWearable, void* pParam);

protected:
	LLInventoryModel::item_array_t m_addAttachments, m_addWearables, m_addGestures, m_remGestures;
	std::list<LLViewerJointAttachment*> m_remAttachments;
	std::list<EWearableType> m_remWearables;
};

// ============================================================================
// RlvBehaviourNotifyObserver
//

class RlvBehaviourNotifyObserver : public RlvBehaviourObserver
{
public:
	virtual ~RlvBehaviourNotifyObserver() { }
	virtual void changed(const RlvCommand& rlvCmd, bool fInternal);

	void addNotify(const LLUUID& idObj, S32 nChannel, const std::string& strFilter)
	{
		m_Notifications.insert(std::pair<LLUUID, notifyData>(idObj, notifyData(nChannel, strFilter)));
	}
	void clearNotify(const LLUUID& idObj)
	{
		m_Notifications.erase(idObj);
	}
	bool hasNotify()
	{
		return (m_Notifications.size() != 0);
	}
	void removeNotify(const LLUUID& idObj, S32 nChannel, const std::string& strFilter)
	{
		for (std::multimap<LLUUID, notifyData>::iterator itNotify = m_Notifications.lower_bound(idObj),
				endNotify = m_Notifications.upper_bound(idObj); itNotify != endNotify; ++itNotify)
		{
			if ( (itNotify->second.nChannel == nChannel) && (itNotify->second.strFilter == strFilter) )
			{
				m_Notifications.erase(itNotify);
				break;
			}
		}
	}
protected:
	struct notifyData
	{
		S32         nChannel;
		std::string strFilter;
		notifyData(S32 channel, const std::string& filter) : nChannel(channel), strFilter(filter) {}
	};
	std::multimap<LLUUID, notifyData> m_Notifications;
};

// ============================================================================
// RlvRetainedCommand
//

struct RlvRetainedCommand
{
public:
	LLUUID      idObject;
	RlvCommand  rlvCmd;

	RlvRetainedCommand(const LLUUID& uuid, const RlvCommand& cmd) : idObject(uuid), rlvCmd(cmd) {}
private:
	RlvRetainedCommand();
};
typedef std::list<RlvRetainedCommand> rlv_retained_list_t;

// ============================================================================
// RlvException
//

typedef boost::variant<std::string, LLUUID, S32, ERlvBehaviour> RlvExceptionOption;

struct RlvException
{
public:
	LLUUID				idObject;    // UUID of the object that added the exception
	ERlvBehaviour		eBehaviour;  // Behaviour the exception applies to
	RlvExceptionOption	varOption;   // Exception data (type is dependent on eBehaviour)

	RlvException(const LLUUID& idObj, ERlvBehaviour eBhvr, const RlvExceptionOption& option) : idObject(idObj), eBehaviour(eBhvr), varOption(option) {}
private:
	RlvException();
};

// ============================================================================
// RlvWLSnapshot
//

struct RlvWLSnapshot
{
public:
	static void           restoreSnapshot(const RlvWLSnapshot* pWLSnapshot);
	static RlvWLSnapshot* takeSnapshot();
private:
	RlvWLSnapshot() {}

	bool		 fIsRunning;
	bool		 fUseLindenTime;
	LLWLParamSet WLParams;
};

// ============================================================================
// Various helper classes/timers/functors
//

class RlvGCTimer : public LLEventTimer
{
public:
	RlvGCTimer() : LLEventTimer(30.0) {}
	virtual BOOL tick();
};

class RlvCurrentlyWorn : public LLInventoryFetchObserver
{
public:
	RlvCurrentlyWorn() {}
	~RlvCurrentlyWorn() {}
	virtual void done() {}

	static void fetchWorn();
	void fetchItem(const LLUUID& idItem);
};

class RlvGiveToRLVAgentOffer : public LLInventoryFetchDescendentsObserver
{
public:
	RlvGiveToRLVAgentOffer() {}
	virtual void done();
};

// ============================================================================
// Various helper functions
//

bool rlvCanDeleteOrReturn();
S32  rlvGetDirectDescendentsCount(const LLInventoryCategory* pFolder, LLAssetType::EType type);
bool rlvIsEmote(const std::string& strUTF8Text);
bool rlvIsValidReplyChannel(S32 nChannel);

void rlvSendBusyMessage(const LLUUID& idTo, const std::string& strMsg, const LLUUID& idSession = LLUUID::null);
bool rlvSendChatReply(const std::string& strChannel, const std::string& strReply);
bool rlvSendChatReply(S32 nChannel, const std::string& strReply);

std::string rlvGetFirstParenthesisedText(const std::string& strText, std::string::size_type* pidxMatch = NULL);
std::string rlvGetLastParenthesisedText(const std::string& strText, std::string::size_type* pidxStart = NULL);
void        rlvStringReplace(std::string& strText, std::string strFrom, const std::string& strTo);

#ifdef RLV_ADVANCED_TOGGLE_RLVA
	// "Advanced / RLVa / Enable RLV" menu option
	void rlvToggleEnabled(void*);
	BOOL rlvGetEnabled(void*);
#endif // RLV_ADVANCED_TOGGLE_RLVA

// ============================================================================
// Debug helper functions
//

// ============================================================================
// Inlined class member functions
//

// Checked: 2009-09-19 (RLVa-1.0.3d)
inline std::string RlvCommand::asString() const
{
	// NOTE: @clear=<param> should be represented as clear:<param>
	return (m_eParamType != RLV_TYPE_CLEAR)
		? (!m_strOption.empty()) ? (std::string(m_strBehaviour)).append(":").append(m_strOption) : (std::string(m_strBehaviour))
	    : (!m_strParam.empty())  ? (std::string(m_strBehaviour)).append(":").append(m_strParam)  : (std::string(m_strBehaviour));
}

inline bool RlvCommand::operator ==(const RlvCommand& rhs) const
{
	// The specification notes that "@detach=n" is semantically identical to "@detach=add" (same for "y" and "rem"
	return (m_strBehaviour == rhs.m_strBehaviour) && (m_strOption == rhs.m_strOption) &&
		( (RLV_TYPE_UNKNOWN != m_eParamType) ? (m_eParamType == rhs.m_eParamType) : (m_strParam == rhs.m_strParam) );
}

inline bool RlvCommand::hasStrictVariant(ERlvBehaviour eBhvr)
{
	switch (eBhvr)
	{
		case RLV_BHVR_RECVCHAT:
		case RLV_BHVR_RECVEMOTE:
		case RLV_BHVR_RECVIM:
		case RLV_BHVR_SENDIM:
		case RLV_BHVR_TPLURE:
		case RLV_BHVR_SENDCHANNEL:
			return true;
		default:
			return false;
	}
}

inline void RlvCurrentlyWorn::fetchItem(const LLUUID& idItem)
{
	if (idItem.notNull())
	{
		LLInventoryFetchObserver::item_ref_t idItems;
		idItems.push_back(idItem);
		fetchItems(idItems);
	}
}

// Checked: 2009-12-18 (RLVa-1.1.0k) | Added: RLVa-1.1.0i
inline bool RlvForceWear::isWearableItem(const LLInventoryItem* pItem)
{
	return (LLAssetType::AT_OBJECT == pItem->getType()) || (LLAssetType::AT_GESTURE == pItem->getType()) ||
		(LLAssetType::AT_BODYPART == pItem->getType()) || (LLAssetType::AT_CLOTHING == pItem->getType());
}

// Checked: 2009-12-18 (RLVa-1.1.0k) | Modified: RLVa-1.1.0i
inline bool RlvForceWear::isWearingItem(const LLInventoryItem* pItem)
{
	return
		((LLAssetType::AT_OBJECT == pItem->getType()) && (gAgent.getAvatarObject()->isWearingAttachment(pItem->getUUID()))) ||
		((LLAssetType::AT_GESTURE == pItem->getType()) && (gGestureManager.isGestureActive(pItem->getUUID()))) ||
		(gAgent.isWearingItem(pItem->getUUID()));
}

// ============================================================================
// Inlined helper functions
//

inline bool rlvIsEmote(const std::string& strUTF8Text)
{
	return (strUTF8Text.length() > 4) && ( (strUTF8Text.compare(0, 4, "/me ") == 0) || (strUTF8Text.compare(0, 4, "/me'") == 0) );
}

// Checked: 2009-09-05 (RLVa-1.0.2a) | Added: RLVa-1.0.2a
inline bool rlvIsValidReplyChannel(S32 nChannel)
{
	return (nChannel > 0) && (CHAT_CHANNEL_DEBUG != nChannel);
}

// Checked: 2009-08-05 (RLVa-1.0.1e) | Added: RLVa-1.0.0e
inline bool rlvSendChatReply(const std::string& strChannel, const std::string& strReply)
{
	S32 nChannel;
	return (LLStringUtil::convertToS32(strChannel, nChannel)) ? rlvSendChatReply(nChannel, strReply) : false;
}

// ============================================================================

#endif // RLV_HELPER_H
