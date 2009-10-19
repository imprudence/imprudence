#ifndef RLV_HELPER_H
#define RLV_HELPER_H

#include "llboost.h"
#include "llinventorymodel.h"
#include "llselectmgr.h"
#include "llviewercontrol.h"
#include "llviewerobjectlist.h"
#include "llwlparamset.h"
#include "rlvdefines.h"

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

	static ERlvBehaviour      getBehaviourFromString(const std::string& strBhvr);
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

		if ( (strFolderName.empty()) ||	(RLV_FOLDER_PREFIX_HIDDEN == strFolderName[0]) )
			return false;

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
// RlvRetainedCommand
//

struct RlvRetainedCommand
{
public:
	std::string strObject;
	LLUUID      idObject;
	std::string strCmd;

	RlvRetainedCommand(const std::string obj, const LLUUID& uuid, const std::string& cmd) : strObject(obj), idObject(uuid), strCmd(cmd) {}
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
// RlvSettings
//

inline BOOL rlvGetSettingBOOL(const std::string& strSetting, BOOL fDefault)
{
	return (gSavedSettings.controlExists(strSetting)) ? gSavedSettings.getBOOL(strSetting) : fDefault;
}
inline BOOL rlvGetPerUserSettingsBOOL(const std::string& strSetting, BOOL fDefault)
{
	return (gSavedPerAccountSettings.controlExists(strSetting)) ? gSavedPerAccountSettings.getBOOL(strSetting) : fDefault;
}

class RlvSettings
{
public:
	static BOOL getDebug()					{ return rlvGetSettingBOOL(RLV_SETTING_DEBUG, FALSE); }
	static BOOL getForbidGiveToRLV()		{ return rlvGetSettingBOOL(RLV_SETTING_FORBIDGIVETORLV, TRUE); }

	static BOOL getEnableWear();
	static BOOL getHideLockedLayers()		{ return rlvGetSettingBOOL(RLV_SETTING_HIDELOCKEDLAYER, FALSE); }		
	static BOOL getHideLockedAttach()		{ return rlvGetSettingBOOL(RLV_SETTING_HIDELOCKEDATTACH, FALSE); }
	static BOOL getHideLockedInventory()	{ return rlvGetSettingBOOL(RLV_SETTING_HIDELOCKEDINVENTORY, FALSE); }
	static BOOL getShowNameTags()			{ return fShowNameTags; }

	#ifdef RLV_EXTENSION_STARTLOCATION
		static BOOL getLoginLastLocation()	{ return rlvGetPerUserSettingsBOOL(RLV_SETTING_LOGINLASTLOCATION, TRUE); }
		static void updateLoginLastLocation();
	#endif // RLV_EXTENSION_STARTLOCATION

	static BOOL fShowNameTags;
};

// ============================================================================
// State keeping classes/structure
//

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

struct RlvSelectHasLockedAttach : public LLSelectedNodeFunctor
{
	virtual bool apply(LLSelectNode* pNode);
};

struct RlvSelectIsOwnedByOrGroupOwned : public LLSelectedNodeFunctor
{
	RlvSelectIsOwnedByOrGroupOwned(const LLUUID& uuid) : m_idAgent(uuid) {}
	virtual bool apply(LLSelectNode* pNode);
	LLUUID m_idAgent;
};

struct RlvSelectIsSittingOn : public LLSelectedNodeFunctor
{
	RlvSelectIsSittingOn(LLXform* pObject) : m_pObject(pObject) {}
	virtual bool apply(LLSelectNode* pNode);
	LLXform* m_pObject;
};

// ============================================================================
// Various helper functions
//

BOOL rlvAttachToEnabler(void* pParam);
bool rlvCanDeleteOrReturn();
BOOL rlvEnableWearEnabler(void* pParam);
S32  rlvGetDirectDescendentsCount(const LLInventoryCategory* pFolder, LLAssetType::EType type);
bool rlvIsEmote(const std::string& strUTF8Text);
bool rlvIsValidReplyChannel(S32 nChannel);
bool rlvIsWearingItem(const LLInventoryItem* pItem);

void rlvForceDetach(LLViewerJointAttachment* pAttachPt);
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

inline void RlvCurrentlyWorn::fetchItem(const LLUUID& idItem)
{ 
	if (idItem.notNull()) 
	{
		LLInventoryFetchObserver::item_ref_t idItems; 
		idItems.push_back(idItem);
		fetchItems(idItems);
	}
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
