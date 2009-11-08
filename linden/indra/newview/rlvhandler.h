#ifndef RLV_HANDLER_H
#define RLV_HANDLER_H

#include "llagentconstants.h"
#include "llappviewer.h"
#include "llformat.h"
#include "llversionviewer.h"
#include "llviewerjointattachment.h"
#include "llviewerobject.h"
#include "llwearable.h"

#include "rlvhelper.h"
#include "rlvevent.h"
#include "rlvmultistringsearch.h"

// ============================================================================

typedef std::map<LLUUID, RlvObject> rlv_object_map_t;
typedef std::multimap<ERlvBehaviour, RlvException> rlv_exception_map_t;
typedef std::map<S32, LLUUID> rlv_attachlock_map_t;

class RlvHandler
{
public:
	RlvHandler();
	~RlvHandler();

	// --------------------------------

	/*
	 * Attachment point helper functions
	 */
public:
	// Returns a pointer to the attachment point for a supplied parameter
	LLViewerJointAttachment* getAttachPoint(const std::string& strText, bool fExact) const;
	LLViewerJointAttachment* getAttachPoint(const LLInventoryItem* pItem, bool fStrict) const;
	LLViewerJointAttachment* getAttachPoint(const LLInventoryCategory* pFolder, bool fStrict) const;
	LLViewerJointAttachment* getAttachPointLegacy(const LLInventoryCategory* pFolder) const;
	S32                      getAttachPointIndex(std::string strText, bool fExact) const;
	S32                      getAttachPointIndex(LLViewerObject* pObj) const;
	S32                      getAttachPointIndex(const LLViewerJointAttachment* pObj) const;
	bool                     hasAttachPointName(const LLInventoryItem* pItem, bool fStrict) const;

	// --------------------------------

	/*
	 * Rule checking functions
	 */
	// NOTE: - to check @detach=n    -> hasLockedAttachment(RLV_LOCK_REMOVE) / hasLockedHUD() / isLockedAttachment(.., RLV_LOCK_REMOVE)
	//       - to check @addattach=n -> hasLockedAttachment(RLV_LOCK_ADD) / isLockedAttachment(.., RLV_LOCK_ADD)
	//       - to check @remattach=n -> (see @detach=n)
	//       - to check exceptions   -> isException()
	//       - to check @addoutfit=n -> isWearable()
	//       - to check @remoutfit=n -> isRemovable()
public:
	// Returns TRUE is at least one object contains the specified behaviour (and optional option)
	bool hasBehaviour(ERlvBehaviour eBehaviour) const { return (eBehaviour < RLV_BHVR_COUNT) ? (0 != m_Behaviours[eBehaviour]) : false; }
	bool hasBehaviour(ERlvBehaviour eBehaviour, const std::string& strOption) const;
	// Returns TRUE if at least one object (except the specified one) contains the specified behaviour (and optional option)
	bool hasBehaviourExcept(ERlvBehaviour eBehaviour, const LLUUID& idObj) const;
	bool hasBehaviourExcept(ERlvBehaviour eBehaviour, const std::string& strOption, const LLUUID& idObj) const;

	// Adds an eLock type lock (held by idRlvObj) for the specified attachment point
	void addAttachmentLock(S32 idxAttachPt, const LLUUID& idRlvObj, ERlvLockMask eLock);
	// Returns TRUE if there is at least 1 eLock type locked attachment (RLV_LOCK_ANY = RLV_LOCK_ADD *or* RLV_LOCK_REMOVE)
	bool hasLockedAttachment(ERlvLockMask eLock) const;
	// Returns TRUE if there is at least 1 non-detachable HUD attachment
	bool hasLockedHUD() const;
	// Returns TRUE if the specified attachment point is eLock type locked (RLV_LOCK_ANY = RLV_LOCK_ADD *or* RLV_LOCK_REMOVE)
	bool isLockedAttachment(S32 idxAttachPt, ERlvLockMask eLock) const;
	bool isLockedAttachment(const LLInventoryItem* pItem, ERlvLockMask eLock) const;
	bool isLockedAttachment(LLViewerJointAttachment* pAttachPt, ERlvLockMask eLock) const;
	bool isLockedAttachment(LLViewerObject* pObj, ERlvLockMask eLock) const;
	// Returns TRUE if the specified attachment point is eLock type locked by anything other than pObj (or one of its children)
	bool isLockedAttachmentExcept(S32 idxAttachPt, ERlvLockMask eLock, LLViewerObject* pObj) const;
	// Adds an eLock type lock (held by idRlvObj) for the specified attachment point
	void removeAttachmentLock(S32 idxAttachPt, const LLUUID& idRlovObj, ERlvLockMask eLock);

	// Adds or removes an exception for the specified behaviour
	void addException(const LLUUID& idObj, ERlvBehaviour eBhvr, const RlvExceptionOption& varOption);
	void removeException(const LLUUID& idObj, ERlvBehaviour eBhvr, const RlvExceptionOption& varOption);
	// Returns TRUE if the specified option was added as an exception for the specified behaviour
	bool isException(ERlvBehaviour eBhvr, const RlvExceptionOption& varOption, ERlvExceptionCheck typeCheck = RLV_CHECK_DEFAULT) const;
	// Returns TRUE if the specified behaviour should behave "permissive" (rather than "strict"/"secure")
	bool isPermissive(ERlvBehaviour eBhvr) const;

	// Returns TRUE if the specified layer is removable (use hasBehaviour(RLV_BHVR_REMOUTFIT) for the general case)
	bool isRemovable(EWearableType type) const	{ return (type < WT_COUNT) ? (0 == m_LayersRem[type]) : true; }
	// Returns TRUE if the specified layer is not remoutfit blocked by any object (except the one specified by UUID)
	bool isRemovableExcept(EWearableType type, const LLUUID& idObj) const;
	// Returns TRUE if the inventory item is strippable by @detach or @remoutfit
	bool isStrippable(const LLUUID& idItem) const;
	// Returns TRUE if the specified layer is wearable (use hasBehaviour(RLV_BHVR_ADDOUTFIT) for the general case)
	bool isWearable(EWearableType type) const	{ return (type < WT_COUNT) ? (0 == m_LayersAdd[type]) : true; }

	// Returns TRUE if the composite folder doesn't contain any "locked" items
	bool canTakeOffComposite(const LLInventoryCategory* pFolder) const;
	// Returns TRUE if the folder is a composite folder and optionally returns the name
	bool getCompositeInfo(const LLInventoryCategory* pFolder, std::string* pstrName) const;
	// Returns TRUE if the inventory item belongs to a composite folder and optionally returns the name and composite folder
	bool getCompositeInfo(const LLUUID& idItem, std::string* pstrName, LLViewerInventoryCategory** ppFolder) const;
	// Returns TRUE if the folder is a composite folder
	bool isCompositeFolder(const LLInventoryCategory* pFolder) const;
	// Returns TRUE if the inventory item belongs to a composite folder
	bool isCompositeDescendent(const LLUUID& idItem) const;
	// Returns TRUE if the inventory item is part of a folded composite folder and should be hidden from @getoufit or @getattach
	bool isHiddenCompositeItem(const LLUUID& idItem, const std::string& strItemType) const;

	// --------------------------------

	/*
	 * Helper functions 
	 */
public:
	// Accessors
	bool getCanCancelTp() const			{ return m_fCanCancelTp; }						// @accepttp and @tpto
	void setCanCancelTp(bool fAllow)	{ m_fCanCancelTp = fAllow; }					// @accepttp and @tpto

	// Command specific helper functions
	bool               canShowHoverText(LLViewerObject* pObj) const;					// @showhovertext* command family
	void               filterChat(std::string& strUTF8Text, bool fFilterEmote) const;	// @sendchat, @recvchat and @redirchat
	void               filterLocation(std::string& strUTF8Text) const;					// @showloc
	void               filterNames(std::string& strUTF8Text) const;						// @shownames
	const std::string& getAnonym(const std::string& strName) const;						// @shownames
	std::string        getVersionString() const;										// @version
	std::string        getVersionNumString() const;										// @versionnum
	BOOL               isAgentNearby(const LLUUID& uuid) const;							// @shownames
	bool               redirectChatOrEmote(const std::string& strUTF8Test) const;		// @redirchat and @rediremote

	// Command processing helper functions
	BOOL processCommand(const LLUUID& idObj, const std::string& strCommand, bool fFromObj);
	void processRetainedCommands();
	void retainCommand(const std::string& strObj, const LLUUID& idObj, const std::string& strCmd);

	// Returns a pointer to the currently executing command (do *not* save this pointer)
	const RlvCommand* getCurrentCommand() const { return m_pCurCommand; }
	// Returns the UUID of the object we're currently executing a command for
	const LLUUID& getCurrentObject() const		{ return m_idCurObject; }

	// Initialization
	static BOOL canDisable();
	static BOOL isEnabled()	{ return m_fEnabled; }
	static void initLookupTables();
	static BOOL setEnabled(BOOL fEnable);
protected:
	void clearState();

	// --------------------------------

	/*
	 * Inventory related functions
	 */
public:
	// Starts a fetch of everything under the shared root (if there is one)
	static void fetchSharedInventory();
	// Returns the path of the supplied folder (relative to the shared root)
	std::string getSharedPath(const LLViewerInventoryCategory* pFolder) const;
	std::string getSharedPath(const LLUUID& idFolder) const;
	// Returns a pointer to the shared root folder (if there is one)
	static LLViewerInventoryCategory* getSharedRoot();
	// A "folded folder" is a folder whose items logically belong to the grandparent rather than the parent
	bool isFoldedFolder(const LLInventoryCategory* pFolder, bool fAttach) const;
	bool isFoldedFolderLegacy(const LLInventoryCategory* pFolder, bool fAttach) const;
protected:
	// Find all folders that match a supplied criteria (clears the supplied array)
	bool findSharedFolders(const std::string& strCriteria, LLInventoryModel::cat_array_t& folders) const;

	// Returns a subfolder of idParent that starts with name (exact match > partial match)
	LLViewerInventoryCategory* getSharedFolder(const LLUUID& idParent, const std::string& strName) const;
	// Looks up a folder from a path (relative to the shared root)
	LLViewerInventoryCategory* getSharedFolder(const std::string& strPath) const;

	bool getWornInfo(const LLInventoryCategory* pFolder, U8& wiFolder, U8& wiChildren) const;

	// --------------------------------

	/*
	 * Event handling (forwards to registered observers if we don't handle the command)
	 */
public:
	BOOL addObserver(RlvObserver* pObserver)	{ return m_Emitter.addObserver(pObserver); }
	BOOL removeObserver(RlvObserver* pObserver)	{ return m_Emitter.remObserver(pObserver); }
	void addBehaviourObserver(RlvBehaviourObserver* pBhvrObserver);
	void removeBehaviourObserver(RlvBehaviourObserver* pBhvrObserver);
	void notifyBehaviourObservers(const RlvCommand& rlvCmd, bool fInternal);

	// Externally invoked event handlers
	void onAttach(LLViewerJointAttachment* pAttachPt);						// LLVOAvatar::attachObject()
	void onDetach(LLViewerJointAttachment* pAttachPt);						// LLVOAvatar::detachObject()
	bool onGC();															// RlvGCTimer::tick()
	void onSavedAssetIntoInventory(const LLUUID& idItem) { if (m_pAttachMgr) m_pAttachMgr->onSavedAssetIntoInventory(idItem); }
	void onWearAttachment(const LLUUID& idItem)			 { if (m_pAttachMgr) m_pAttachMgr->onWearAttachment(idItem); }
protected:
	BOOL processAddCommand(const LLUUID& uuid, const RlvCommand& rlvCmd);
	BOOL processRemoveCommand(const LLUUID& uuid, const RlvCommand& rlvCmd);
	BOOL processClearCommand(const LLUUID idObj, const RlvCommand& rlvCmd);
	BOOL processReplyCommand(const LLUUID& uuid, const RlvCommand& rlvCmd) const;
	BOOL processForceCommand(const LLUUID& uuid, const RlvCommand& rlvCmd) const;

	// Command handlers (exist for no other reason than to keep the length of the processXXX functions down)
	ERlvCmdRet onAddRemAttach(const LLUUID& idObj, const RlvCommand& rlvCmd, bool& fRefCount);
	ERlvCmdRet onAddRemDetach(const LLUUID& idObj, const RlvCommand& rlvCmd, bool& fRefCount);
	ERlvCmdRet onAddRemOutfit(const LLUUID& idObj, const RlvCommand& rlvCmd, bool& fRefCount);
	ERlvCmdRet onForceDetach(const LLUUID& idObj, const RlvCommand& rlvCmd) const;
	ERlvCmdRet onForceRemAttach(const LLUUID& idObj, const RlvCommand& rlvCmd) const;
	ERlvCmdRet onForceRemOutfit(const LLUUID& idObj, const RlvCommand& rlvCmd) const;
	ERlvCmdRet onGetAttach(const LLUUID& idObj, const RlvCommand& rlvCmd, std::string& strReply);
	ERlvCmdRet onGetOutfit(const LLUUID& idObj, const RlvCommand& rlvCmd, std::string& strReply);
	// Old style command handlers (need to be updated to return ERlvCmdRet)
	void onForceRemOutfit(const LLUUID& idObj, const std::string& strOption) const;
	bool onForceSit(const LLUUID& uuid, const std::string& strOption) const;
	void onForceWear(const std::string& strPath, bool fAttach, bool fMatchAll) const;
	bool onGetPath(const LLUUID& uuid, const std::string& strOption, std::string& strReply) const;
	void onGetInvWorn(const std::string& strPath, std::string &strReply) const;

	// --------------------------------

	/*
	 * Member variables
	 */
public:
	static BOOL fNoSetEnv;
	static BOOL fLegacyNaming;

	static const std::string cstrSharedRoot;		// Name of the shared root folder
	static const std::string cstrBlockedRecvIM;		// Stand-in text for incoming IM when recvim restricted
	static const std::string cstrBlockedSendIM;		// Stand-in text for outgoing IM when sendim restricted
	static const std::string cstrHidden;			// General purpose "this was censored" text
	static const std::string cstrHiddenParcel;
	static const std::string cstrHiddenRegion;
	static const std::string cstrMsgRecvIM;			// Message sent to IM sender when sendim restricted
	static const std::string cstrMsgTpLure;			// Message sent to tplure sender when tplure restricted
	static const std::string cstrAnonyms[28];
protected:
	rlv_object_map_t      m_Objects;				// Map of objects that have active restrictions (idObj -> RlvObject)
	rlv_exception_map_t   m_Exceptions;				// Map of currently active restriction exceptions (ERlvBehaviour -> RlvException)
	rlv_attachlock_map_t  m_AttachAdd;				// Map of attachment points that can't be attached (idxAttachPt -> idObj)
	rlv_attachlock_map_t  m_AttachRem;				// Map of attachment points that can't be detached (idxAttachPt -> idObj)
	S16                   m_LayersAdd[WT_COUNT];	// Array of layers that can't be worn (reference counted)
	S16                   m_LayersRem[WT_COUNT];	// Array of layers that can't be removed (reference counted)
	S16                   m_Behaviours[RLV_BHVR_COUNT];

	rlv_retained_list_t   m_Retained;
	RlvGCTimer*           m_pGCTimer;
	RlvWLSnapshot*        m_pWLSnapshot;
	RlvAttachmentManager* m_pAttachMgr;

	RlvCommand*           m_pCurCommand;			// Convenience (see @tpto)
	LLUUID                m_idCurObject;			// Convenience (see @tpto)

	mutable RlvEventEmitter<RlvObserver>     m_Emitter;
	mutable std::list<RlvBehaviourObserver*> m_BhvrObservers;
	RlvBehaviourNotifyObserver*				 m_pBhvrNotify;

	static BOOL			  m_fEnabled;				// Use setEnabled() to toggle this
	static BOOL			  m_fFetchStarted;			// TRUE if we fired off an inventory fetch
	static BOOL			  m_fFetchComplete;			// TRUE if everything was fetched
	static RlvMultiStringSearch m_AttachLookup;		// Lookup table for attachment names (lower case)

	bool m_fCanCancelTp;

	friend class RlvSharedRootFetcher;				// Fetcher needs access to m_fFetchComplete
	friend class RlvGCTimer;						// Timer clear its own point at destruction

	// --------------------------------

	/*
	 * Internal access functions used by unit tests
	 */
public:
	const rlv_object_map_t*    getObjectMap() const		{ return &m_Objects; }
	//const rlv_exception_map_t* getExceptionMap() const	{ return &m_Exceptions; }
	//const rlv_detach_map_t*    getDetachMap() const		{ return &m_Attachments; }
	#ifdef RLV_DEBUG_TESTS
		const S16*                 getAddLayers() const		{ return m_LayersAdd; }
		const S16*                 getRemLayers() const		{ return m_LayersRem; }
		const S16*                 getBehaviours() const	{ return m_Behaviours; }
		const rlv_retained_list_t* getRetainedList() const	{ return &m_Retained; }
	#endif // RLV_DEBUG_TESTS
};

typedef RlvHandler rlv_handler_t;
extern rlv_handler_t gRlvHandler;

// ============================================================================
// Inlined member functions
//

// Checked: 2009-10-04 (RLVa-1.0.4a) | Modified: RLVa-1.0.4a
inline void RlvHandler::addException(const LLUUID& idObj, ERlvBehaviour eBhvr, const RlvExceptionOption& varOption)
{
	m_Exceptions.insert(std::pair<ERlvBehaviour, RlvException>(eBhvr, RlvException(idObj, eBhvr, varOption)));
}

// Checked: 2009-07-09 (RLVa-1.0.0f) | Modified: RLVa-1.0.0f
inline bool RlvHandler::canShowHoverText(LLViewerObject *pObj) const
{
	return ( (!pObj) || (LL_PCODE_VOLUME != pObj->getPCode()) ||
		    !( (hasBehaviour(RLV_BHVR_SHOWHOVERTEXTALL)) ||
			   ( (hasBehaviour(RLV_BHVR_SHOWHOVERTEXTWORLD)) && (!pObj->isHUDAttachment()) ) ||
			   ( (hasBehaviour(RLV_BHVR_SHOWHOVERTEXTHUD)) && (pObj->isHUDAttachment()) ) ||
			   (isException(RLV_BHVR_SHOWHOVERTEXT, pObj->getID(), RLV_CHECK_PERMISSIVE)) ) );
}

// Checked: 2009-05-23 (RLVa-0.2.0d) | Modified: RLVa-0.2.0d
inline S32 RlvHandler::getAttachPointIndex(std::string strText, bool fExact) const
{
	U16 nParam; RlvMultiStringSearchMatch match;
	LLStringUtil::toLower(strText);
	return (fExact) ? ((m_AttachLookup.getExactMatchParam(strText, nParam)) ? nParam : 0)
	                : ((m_AttachLookup.findLast(strText, match)) ? match.nParam : 0);
}

// Checked: 2009-05-23 (RLVa-0.2.0d) | Modified: RLVa-0.2.0d
inline S32 RlvHandler::getAttachPointIndex(LLViewerObject* pObj) const
{
	return (pObj) ? ATTACHMENT_ID_FROM_STATE(pObj->getState()) : 0;
}

// Checked: 2009-06-02 (RLVa-0.2.0g)
inline std::string RlvHandler::getSharedPath(const LLUUID& idFolder) const
{
	return getSharedPath(gInventory.getCategory(idFolder)); // getSharedPath() has a NULL pointer check so this is safe
}

// Checked: 2009-06-07 (RLVa-0.2.1c)
inline std::string RlvHandler::getVersionString()  const
{
	return llformat("RestrainedLife viewer v%d.%d.%d (%s %d.%d.%d.%d - RLVa %d.%d.%d)",
		RLV_VERSION_MAJOR, RLV_VERSION_MINOR, RLV_VERSION_PATCH,
		LLAppViewer::instance()->getSecondLifeTitle().c_str(), LL_VERSION_MAJOR, LL_VERSION_MINOR, LL_VERSION_PATCH, LL_VERSION_BUILD,
		RLVa_VERSION_MAJOR, RLVa_VERSION_MINOR, RLVa_VERSION_PATCH);
}

// Checked: 2009-10-04 (RLVa-1.0.4b) | Added: RLVa-1.0.4b
inline std::string RlvHandler::getVersionNumString()  const
{
	return llformat("%d%02d%02d%02d", RLV_VERSION_MAJOR, RLV_VERSION_MINOR, RLV_VERSION_PATCH, RLV_VERSION_BUILD);
}

// Checked: 2009-05-23 (RLVa-0.2.0d) | Modified: RLVa-0.2.0d
inline bool RlvHandler::hasAttachPointName(const LLInventoryItem *pItem, bool fStrict) const
{
	return (getAttachPoint(pItem, fStrict) != NULL); // getAttachPoint() has a NULL pointer check so this is safe
}

// Checked:
inline bool RlvHandler::hasBehaviour(ERlvBehaviour eBehaviour, const std::string& strOption) const
{
	return hasBehaviourExcept(eBehaviour, strOption, LLUUID::null);
}

// Checked:
inline bool RlvHandler::hasBehaviourExcept(ERlvBehaviour eBehaviour, const LLUUID& idObj) const
{
	return hasBehaviourExcept(eBehaviour, std::string(), idObj);
}

// Checked: 2009-10-10 (RLVa-1.0.5a) | Added: RLVa-1.0.5a
inline bool RlvHandler::hasLockedAttachment(ERlvLockMask eLock) const
{
	// Remove locks are more common so check those first
	return ( (eLock & RLV_LOCK_REMOVE) && (!m_AttachRem.empty()) ) || ( (eLock & RLV_LOCK_ADD) && (!m_AttachAdd.empty()) );
}

#ifdef RLV_EXPERIMENTAL_COMPOSITES
	// Checked:
	inline bool RlvHandler::isCompositeFolder(const LLInventoryCategory* pFolder) const
	{
		return getCompositeInfo(pFolder, NULL);
	}

	// Checked:
	inline bool RlvHandler::isCompositeDescendent(const LLUUID& idItem) const
	{
		return getCompositeInfo(idItem, NULL, NULL);
	}
#endif // RLV_EXPERIMENTAL_COMPOSITES

inline bool RlvHandler::isPermissive(ERlvBehaviour eBhvr) const
{
	return (RlvCommand::hasStrictVariant(eBhvr)) 
		? !((hasBehaviour(RLV_BHVR_PERMISSIVE)) || (isException(RLV_BHVR_PERMISSIVE, eBhvr, RLV_CHECK_PERMISSIVE)))
		: true;
}

// Checked: 2009-07-29 (RLVa-1.0.1b) | Added: RLVa-1.0.1b
inline bool RlvHandler::isFoldedFolder(const LLInventoryCategory* pFolder, bool fAttach) const
{
	return
	  (
		// .(<attachpt>) type folder (on detach we don't care about its children, but on attach there can only be 1 attachment)
		( (gRlvHandler.getAttachPoint(pFolder, true)) &&
		  ( (!fAttach) || (1 == rlvGetDirectDescendentsCount(pFolder, LLAssetType::AT_OBJECT))) )
		#ifdef RLV_EXTENSION_FLAG_NOSTRIP
		// .(nostrip) folder
		|| ( (pFolder) && (".("RLV_FOLDER_FLAG_NOSTRIP")" == pFolder->getName()) )
		#endif // RLV_EXTENSION_FLAG_NOSTRIP
	  );
}

// Checked: 2009-10-10 (RLVa-1.0.5a) | Added: RLVa-1.0.5a
inline bool RlvHandler::isLockedAttachment(S32 idxAttachPt, ERlvLockMask eLock) const
{
	return ( (eLock & RLV_LOCK_REMOVE) && (m_AttachRem.find(idxAttachPt) != m_AttachRem.end()) ) ||
		   ( (eLock & RLV_LOCK_ADD) && (m_AttachAdd.find(idxAttachPt) != m_AttachAdd.end()) );
}


// Checked: 2009-10-10 (RLVa-1.0.5a) | Added: RLVa-1.0.5a
inline bool RlvHandler::isLockedAttachment(LLViewerJointAttachment *pAttachPt, ERlvLockMask eLock) const
{
	// If there's an attached object it's faster to just use that; otherwise look up the attachment index because it might be locked empty
	return (pAttachPt != NULL) && 
		   ( ( (pAttachPt->getObject() != NULL) && (isLockedAttachment(pAttachPt->getObject(), eLock)) ) || 
		     (isLockedAttachment(getAttachPointIndex(pAttachPt), eLock)) );
}

// Checked: 2009-10-10 (RLVa-1.0.5a) | Added: RLVa-1.0.5a
inline bool RlvHandler::isLockedAttachment(LLViewerObject* pObj, ERlvLockMask eLock) const
{
	return (pObj != NULL) && (pObj->isAttachment()) && (isLockedAttachment(getAttachPointIndex(pObj), eLock));
}

// Checked: 2009-05-23 (RLVa-0.2.0d) | Added: RLVa-0.2.0d
inline bool RlvHandler::isRemovableExcept(EWearableType type, const LLUUID& idObj) const
{
	// NOTE: mind the bitwise OR rather than the logical OR!!
	return (isRemovable(type)) || !( (hasBehaviourExcept(RLV_BHVR_REMOUTFIT, idObj)) |
		                             (hasBehaviourExcept(RLV_BHVR_REMOUTFIT, LLWearable::typeToTypeName(type), idObj)) );
}

#ifndef RLV_EXTENSION_FLAG_NOSTRIP
	// Checked: 2009-05-23 (RLVa-0.2.0d) | Added: RLVa-0.2.0d
	bool RlvHandler::isStrippable(const LLUUID& idItem) const
	{
		return true;
	}
#endif // RLV_EXTENSION_FLAG_NOSTRIP

// Checked: 2009-10-04 (RLVa-1.0.4a) | Modified: RLVa-1.0.4a
inline void RlvHandler::removeException(const LLUUID& idObj, ERlvBehaviour eBhvr, const RlvExceptionOption& varOption)
{
	for (rlv_exception_map_t::iterator itException = m_Exceptions.lower_bound(eBhvr), 
			endException = m_Exceptions.upper_bound(eBhvr); itException != endException; ++itException)
	{
		if ( (itException->second.idObject == idObj) && (itException->second.varOption == varOption) )
		{
			m_Exceptions.erase(itException);
			break;
		}
	}
}

// Checked: 2009-08-05 (RLVa-1.0.1e) | Modified: RLVa-1.0.1e
inline void RlvHandler::retainCommand(const std::string& strObj, const LLUUID& idObj, const std::string& strCmd)
{
	#ifdef RLV_DEBUG
		RLV_INFOS << "[" << idObj << "]: " << strCmd << " (retaining)" << LL_ENDL;
	#endif // RLV_DEBUG
	m_Retained.push_back(RlvRetainedCommand(strObj, idObj, strCmd));
}

// ============================================================================

#endif // RLV_HANDLER_H
