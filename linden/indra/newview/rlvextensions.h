#ifndef RLV_EXTENSIONS_H
#define RLV_EXTENSIONS_H

#include "rlvhelper.h"
#include "rlvevent.h"

// ============================================================================
/*
 * RlvExtGetSet
 * ============
 * Implements @get_XXX:<option>=<channel> and @set_XXX:<option>=force
 *
 */

class RlvExtGetSet : public RlvObserver
{
public:
	RlvExtGetSet();
	virtual ~RlvExtGetSet() {}

	virtual BOOL onForceCommand(const RlvEvent& rlvEvent);
	virtual BOOL onReplyCommand(const RlvEvent& rlvEvent);
protected:
	std::string onGetDebug(std::string strSetting);
	std::string onGetPseudoDebug(const std::string& strSetting);
	void        onSetDebug(std::string strSetting, const std::string& strValue);
	void        onSetPseudoDebug(const std::string& strSetting, const std::string& strValue);

	std::string onGetEnv(std::string strSetting);
	void        onSetEnv(std::string strSetting, const std::string& strValue);

	BOOL processCommand(const LLUUID& idObj, const RlvCommand& rlvCmd);

public:
	enum { DBG_READ = 0x01, DBG_WRITE = 0x02, DBG_PERSIST = 0x04, DBG_PSEUDO = 0x08 };
	static std::map<std::string, S16> m_DbgAllowed;
	static std::map<std::string, std::string> m_PseudoDebug;

	static bool findDebugSetting(/*[in,out]*/ std::string& strSetting, /*[out]*/ S16& flags);
	static S16  getDebugSettingFlags(const std::string& strSetting);
};

// ============================================================================

#endif // RLV_EXTENSIONS_H
