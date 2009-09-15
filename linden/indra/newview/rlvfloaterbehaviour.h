#ifndef RLV_FLOATER_BEHAVIOUR
#define RLV_FLOATER_BEHAVIOUR

#include "llfloater.h"
#include "rlvevent.h"

// ============================================================================

class RlvFloaterBehaviour :
	public LLFloater,
	public LLFloaterSingleton<RlvFloaterBehaviour>,
	public RlvBehaviourObserver
{
	friend class LLUISingleton<RlvFloaterBehaviour, VisibilityPolicy<LLFloater> >;
public:
	virtual ~RlvFloaterBehaviour() {}

	/*
	 * LLFloater overrides
	 */
public:
	/*virtual*/ BOOL canClose();
	/*virtual*/ void onOpen();
	/*virtual*/ void onClose(bool app_quitting);
	/*virtual*/ BOOL postBuild();

	/*
	 * RlvBehaviourObserver overrides
	 */
public:
	/*virtual*/ void changed(const RlvCommand& rlvCmd, bool fInternal);

	/*
	 * Member functions
	 */
public:
	static void show(void*);
	static void onAvatarNameLookup(const LLUUID& uuid, const std::string& strFirst, const std::string& strLast, BOOL fGroup, void* pParam);
protected:
	void refreshAll();
private:
	RlvFloaterBehaviour(const LLSD& key = LLSD());

	std::list<LLUUID> m_PendingLookup;
};

// ============================================================================

#endif // RLV_FLOATER_BEHAVIOUR
