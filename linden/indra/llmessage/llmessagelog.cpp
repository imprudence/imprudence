// <edit>
#include "llmessagelog.h"

LLMessageLogEntry::LLMessageLogEntry(EType type, LLHost from_host, LLHost to_host, U8* data, S32 data_size)
:	mType(type),
	mFromHost(from_host),
	mToHost(to_host),
	mDataSize(data_size)
{
	if(data)
	{
		mData.resize(data_size);
		memcpy(&(mData[0]), data, data_size);
	}
}
LLMessageLogEntry::LLMessageLogEntry(EType type, LLHost from_host, LLHost to_host, std::vector<U8> data, S32 data_size)
:	mType(type),
	mFromHost(from_host),
	mToHost(to_host),
	mDataSize(data_size),
	mData(data)
{
}
LLMessageLogEntry::~LLMessageLogEntry()
{
}
U32 LLMessageLog::sMaxSize = 4096; // testzone fixme todo boom
std::deque<LLMessageLogEntry> LLMessageLog::sDeque;
void (*(LLMessageLog::sCallback))(LLMessageLogEntry);
void LLMessageLog::setMaxSize(U32 size)
{
	sMaxSize = size;
	while(sDeque.size() > sMaxSize)
		sDeque.pop_front();
}
void LLMessageLog::setCallback(void (*callback)(LLMessageLogEntry))
{
	sCallback = callback;
}
void LLMessageLog::log(LLHost from_host, LLHost to_host, U8* data, S32 data_size)
{
	LLMessageLogEntry entry = LLMessageLogEntry(LLMessageLogEntry::TEMPLATE, from_host, to_host, data, data_size);
	if(!entry.mDataSize || !entry.mData.size()) return;
	if(sCallback) sCallback(entry);
	if(!sMaxSize) return;
	sDeque.push_back(entry);
	if(sDeque.size() > sMaxSize)
		sDeque.pop_front();
}
std::deque<LLMessageLogEntry> LLMessageLog::getDeque()
{
	return sDeque;
}
// </edit>
