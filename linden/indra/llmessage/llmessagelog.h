// <edit>
#ifndef LL_LLMESSAGELOG_H
#define LL_LLMESSAGELOG_H
#include "stdtypes.h"
#include "llhost.h"
#include <queue>
#include <string.h>

class LLMessageSystem;
class LLMessageLogEntry
{
public:
	enum EType
	{
		TEMPLATE,
		HTTP_REQUEST,
		HTTP_RESPONSE
	};
	LLMessageLogEntry(EType type, LLHost from_host, LLHost to_host, U8* data, S32 data_size);
	LLMessageLogEntry(EType type, LLHost from_host, LLHost to_host, std::vector<U8> data, S32 data_size);
	~LLMessageLogEntry();
	EType mType;
	LLHost mFromHost;
	LLHost mToHost;
	S32 mDataSize;
	std::vector<U8> mData;
};
class LLMessageLog
{
public:
	static void setMaxSize(U32 size);
	static void setCallback(void (*callback)(LLMessageLogEntry));
	static void log(LLHost from_host, LLHost to_host, U8* data, S32 data_size);
	static std::deque<LLMessageLogEntry> getDeque();
private:
	static U32 sMaxSize;
	static void (*sCallback)(LLMessageLogEntry);
	static std::deque<LLMessageLogEntry> sDeque;
};
#endif
// </edit>
