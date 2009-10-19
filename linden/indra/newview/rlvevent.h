#ifndef RLV_EVENTEMITTER_H
#define RLV_EVENTEMITTER_H

#include <algorithm>
#include <typeinfo>
#include <list>

#include "lluuid.h"

#include "rlvhelper.h"

// ============================================================================
// RlvEvent - Passed to observer event handlers (contains the same paramaters as RlvHandler::processXXXCommand)
//

class RlvEvent
{
public:
	RlvEvent(const LLUUID& uuid, const RlvCommand& rlvCmd) : m_UUID(uuid), m_rlvCmd(rlvCmd) {}
	virtual ~RlvEvent() {}

	const LLUUID&		getSenderID() const	{ return m_UUID; };
	const RlvCommand&	getCommand() const	{ return m_rlvCmd; };

protected:
	LLUUID     m_UUID;
	RlvCommand m_rlvCmd;
};

// ============================================================================
/*
 * RlvObserver
 * ===========
 * Provides a way to extend the existing command set without changing the main RlvHandler class
 *
 * Steps:
 *   - derive your class from RlvObserver
 *   - override any of the event functions you need
 *   - add it as an observer: gRlvHandler.addObserver(new RlbObserverDerivedClass());
 *   - done
 *
 * Notes:
 *   - as long as you don't call gRlvHandler.remObserver() your class will be cleaned up from
 *     RlvEventEmitter destructor (see below)
 *   - event handlers are called only if RlvHandler didn't handle it so while you can
 *     add a new command @foobar=n, you won't get called for @detach=n
 *   - event handlers are called *after* the command is added so a call to 
 *     RlvHandler::hasBehaviour("foobar") would return TRUE
 *   - return TRUE if you handled the command (won't get passed on to the next observer)
 *     return FALSE if you didn't handle the command (gets passed on to the next observer)
 *
 */

class RlvObserver
{
public:
	virtual ~RlvObserver() {}

	typedef RlvEvent EventType;

	virtual BOOL onAddCommand(const EventType& rlvEvent)    { return FALSE; }
	virtual BOOL onRemoveCommand(const EventType& rlvEvent) { return FALSE; }
	virtual BOOL onClearCommand(const EventType& rlvEvent)  { return FALSE; }
	virtual BOOL onReplyCommand(const EventType& rlvEvent)  { return FALSE; }
	virtual BOOL onForceCommand(const EventType& rlvEvent)  { return FALSE; }
};

// ============================================================================
/*
 * RlvEventEmitter
 * ===============
 * Essentially a slightly modified eventEmitter (see lleventemitter.h)
 *
 * Changes:
 *   - if an event handler returns TRUE then no further observers are notified
 *   - cleans up the (remaining) observers in the destructor
 */

template < class T >
class RlvEventEmitter
{
	public:
		typedef typename T::EventType EventType;
		typedef std::list< T* > ObserverContainer;
		typedef BOOL ( T::*observerMethod )( const EventType& );

	protected:
		ObserverContainer observers;

	public:
		RlvEventEmitter() { };

		~RlvEventEmitter()
		{
			clearObservers();
		}

		BOOL addObserver ( T* observerIn )
		{
			if ( ! observerIn )
				return FALSE;

			// check if observer already exists
			if ( std::find ( observers.begin (), observers.end (), observerIn ) != observers.end () )
				return FALSE;

			// save it
			observers.push_back ( observerIn );

			return true;
		}

		BOOL remObserver ( T* observerIn )
		{
			if ( ! observerIn )
				return FALSE;

			observers.remove ( observerIn );

			return TRUE;
		}

		void clearObservers()
		{
			typename std::list< T* >::iterator iter = observers.begin ();

			while (iter != observers.end ())
			{
				delete *iter;
				++iter;
			};

			observers.clear();
		}

		BOOL update ( observerMethod method, const EventType& msgIn )
		{
			typename std::list< T* >::iterator iter = observers.begin ();

			BOOL fContinue = TRUE;
			while ( (iter != observers.end ()) && (fContinue) )
			{
				fContinue = !( ( ( *iter )->*method ) ( msgIn ) );
				++iter;
			};

			return !fContinue;
		}
};

// ============================================================================

class RlvBehaviourObserver
{
public:
	virtual ~RlvBehaviourObserver() {}
	virtual void changed(const RlvCommand& rlvCmd, bool fInternal) = 0;
};

// ============================================================================

class RlvBehaviourNotifyObserver : public RlvBehaviourObserver
{
public:
	virtual ~RlvBehaviourNotifyObserver() { }

	void changed(const RlvCommand& rlvCmd, bool fInternal)
	{
		if (fInternal)
			return;

		std::string strCmd = rlvCmd.asString(), strNotify; ERlvParamType eCmdType = rlvCmd.getParamType();
		if ( (RLV_TYPE_ADD == eCmdType) || (RLV_TYPE_REMOVE == eCmdType) )
			strNotify = llformat("/%s=%s", strCmd.c_str(), rlvCmd.getParam().c_str());
		else if (RLV_TYPE_CLEAR == eCmdType)
			strNotify = llformat("/%s", strCmd.c_str());
		else
			return;

		for (std::multimap<LLUUID, notifyData>::const_iterator itNotify = m_Notifications.begin(); 
				itNotify != m_Notifications.end(); ++itNotify)
		{
			if ( (itNotify->second.strFilter.empty()) || (std::string::npos != strCmd.find(itNotify->second.strFilter)) )
				rlvSendChatReply(itNotify->second.nChannel, strNotify);
		}
	}

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

#endif // RLV_EVENTEMITTER_H
