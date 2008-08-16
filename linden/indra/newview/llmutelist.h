/** 
 * @file llmutelist.h
 * @brief Management of list of muted players
 *
 * Copyright (c) 2003-2007, Linden Research, Inc.
 * 
 * Second Life Viewer Source Code
 * The source code in this file ("Source Code") is provided by Linden Lab
 * to you under the terms of the GNU General Public License, version 2.0
 * ("GPL"), unless you have obtained a separate licensing agreement
 * ("Other License"), formally executed by you and Linden Lab.  Terms of
 * the GPL can be found in doc/GPL-license.txt in this distribution, or
 * online at http://secondlife.com/developers/opensource/gplv2
 * 
 * There are special exceptions to the terms and conditions of the GPL as
 * it is applied to this Source Code. View the full text of the exception
 * in the file doc/FLOSS-exception.txt in this software distribution, or
 * online at http://secondlife.com/developers/opensource/flossexception
 * 
 * By copying, modifying or distributing this software, you acknowledge
 * that you have read and understood your obligations described above,
 * and agree to abide by those obligations.
 * 
 * ALL LINDEN LAB SOURCE CODE IS PROVIDED "AS IS." LINDEN LAB MAKES NO
 * WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
 * COMPLETENESS OR PERFORMANCE.
 */

#ifndef LL_MUTELIST_H
#define LL_MUTELIST_H

#include "llstring.h"
#include "lluuid.h"

class LLViewerObject;
class LLMessageSystem;
class LLMuteListObserver;

// An entry in the mute list.
class LLMute
{
public:
	// Legacy mutes are BY_NAME and have null UUID.
	enum EType { BY_NAME = 0, AGENT = 1, OBJECT = 2, GROUP = 3, COUNT = 4 };
	
	LLMute(const LLUUID& id, const LLString& name = "", EType type = BY_NAME) 
	: mID(id), mName(name), mType(type) { }

	// Returns name + suffix based on type
	// For example:  "James Tester (resident)"
	LLString getDisplayName() const;
	
	// Converts a UI name into just the agent or object name
	// For example: "James Tester (resident)" sets the name to "James Tester"
	// and the type to AGENT.
	void setFromDisplayName(const LLString& display_name);
	
public:
	LLUUID		mID;	// agent or object id
	LLString	mName;	// agent or object name
	EType		mType;	// needed for UI display of existing mutes
};

class LLMuteList
{
public:
	LLMuteList();
	~LLMuteList();

	void addObserver(LLMuteListObserver* observer);
	void removeObserver(LLMuteListObserver* observer);

	// Add either a normal or a BY_NAME mute.
	BOOL add(const LLMute& mute);

	// Remove both normal and legacy mutes.
	BOOL remove(const LLMute& mute);
	
	// Name is required to test against legacy text-only mutes.
	BOOL isMuted(const LLUUID& id, const LLString& name = LLString::null) const;
	
	BOOL isLinden(const LLString& name) const;
	
	BOOL isLoaded() const { return mIsLoaded; }

	std::vector<LLMute> getMutes() const;
	
	// request the mute list
	void requestFromServer(const LLUUID& agent_id);

	// call this method on logout to save everything.
	void cache(const LLUUID& agent_id);

private:
	BOOL loadFromFile(const LLString& filename);
	BOOL saveToFile(const LLString& filename);

	void setLoaded();
	void notifyObservers();

	void updateAdd(const LLMute& mute);
	void updateRemove(const LLMute& mute);

	// TODO: NULL out mute_id in database
	static void processMuteListUpdate(LLMessageSystem* msg, void**);
	static void processUseCachedMuteList(LLMessageSystem* msg, void**);

	static void onFileMuteList(void** user_data, S32 code);

private:
	struct compare_by_name
	{
		bool operator()(const LLMute& a, const LLMute& b) const
		{
			return a.mName < b.mName;
		}
	};
	struct compare_by_id
	{
		bool operator()(const LLMute& a, const LLMute& b) const
		{
			return a.mID < b.mID;
		}
	};
	typedef std::set<LLMute, compare_by_id> mute_set_t;
	mute_set_t mMutes;
	
	typedef std::set<LLString> string_set_t;
	string_set_t mLegacyMutes;
	
	typedef std::set<LLMuteListObserver*> observer_set_t;
	observer_set_t mObservers;

	BOOL mIsLoaded;

	friend class LLDispatchEmptyMuteList;
};

class LLMuteListObserver
{
public:
	virtual ~LLMuteListObserver() { }
	virtual void onChange() = 0;
};

extern LLMuteList *gMuteListp;

#endif //LL_MUTELIST_H
