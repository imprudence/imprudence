// <edit>]
#ifndef LL_LLFLOATERMESSAGEBUILDER_H
#define LL_LLFLOATERMESSAGEBUILDER_H
#include "llfloater.h"
#include "lltemplatemessagereader.h"
#include "llmessagelog.h"

class LLNetListItem
{
public:
	LLNetListItem(LLUUID id);
	LLUUID mID;
	BOOL mAutoName;
	std::string mName;
	std::string mPreviousRegionName;
	LLCircuitData* mCircuitData;
};

class LLFloaterMessageBuilder : public LLFloater, public LLEventTimer
{
public:
	LLFloaterMessageBuilder(std::string initial_text);
	~LLFloaterMessageBuilder();
	static void show(std::string initial_text);
	static std::list<LLNetListItem*> sNetListItems;
	BOOL postBuild();
	BOOL tick();
	static BOOL addField(e_message_variable_type var_type, const char* var_name, std::string input, BOOL hex);
	static void onClickSend(void* user_data);
	static void onCommitPacketCombo(LLUICtrl* ctrl, void* user_data);
	static LLFloaterMessageBuilder* sInstance;
	BOOL handleKeyHere(KEY key, MASK mask);
	std::string mInitialText;
	struct parts_var
	{
		std::string name;
		std::string value;
		BOOL hex;
		e_message_variable_type var_type;
	};
	struct parts_block
	{
		std::string name;
		std::vector<parts_var> vars;
	};
	static LLNetListItem* findNetListItem(LLHost host);
	static LLNetListItem* findNetListItem(LLUUID id);
	void refreshNetList();
	enum ENetInfoMode { NI_NET, NI_LOG };
	ENetInfoMode mNetInfoMode;
	static void onCommitMessageLog(LLUICtrl* ctrl, void* user_data);
	static void onCommitFilter(LLUICtrl* ctrl, void* user_data);
};
#endif
// </edit>
