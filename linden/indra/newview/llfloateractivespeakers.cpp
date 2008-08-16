/** 
 * @file llfloateractivespeakers.cpp
 * @brief Management interface for muting and controlling volume of residents currently speaking
 *
 * Copyright (c) 2005-2007, Linden Research, Inc.
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

#include "llviewerprecompiledheaders.h"

#include "llfloateractivespeakers.h"

#include "llagent.h"
#include "llvoavatar.h"
#include "llfloateravatarinfo.h"
#include "llvieweruictrlfactory.h"
#include "llviewercontrol.h"
#include "llscrolllistctrl.h"
#include "llbutton.h"
#include "lltextbox.h"
#include "llmutelist.h"
#include "llviewerobjectlist.h"
#include "llimpanel.h" // LLVoiceChannel
#include "llsdutil.h"

const F32 SPEAKER_TIMEOUT = 10.f; // seconds of not being on voice channel before removed from list of active speakers
const LLColor4 INACTIVE_COLOR(0.3f, 0.3f, 0.3f, 0.5f);
const LLColor4 ACTIVE_COLOR(0.5f, 0.5f, 0.5f, 1.f);
const F32 TYPING_ANIMATION_FPS = 2.5f;

LLLocalSpeakerMgr*	gLocalSpeakerMgr = NULL;
LLActiveSpeakerMgr*		gActiveChannelSpeakerMgr = NULL;

LLSpeaker::speaker_map_t LLSpeaker::sSpeakers;

LLSpeaker::LLSpeaker(const LLUUID& id, const LLString& name, const ESpeakerType type) : 
	mStatus(LLSpeaker::STATUS_TEXT_ONLY),
	mLastSpokeTime(0.f), 
	mSpeechVolume(0.f), 
	mHasSpoken(FALSE),
	mDotColor(LLColor4::white),
	mID(id),
	mTyping(FALSE),
	mSortIndex(0),
	mType(type)
{
	mHandle.init();
	sSpeakers.insert(std::make_pair(mHandle, this));
	if (name.empty() && type == SPEAKER_AGENT)
	{
		lookupName();
	}
	else
	{
		mDisplayName = name;
	}
	mActivityTimer.resetWithExpiry(SPEAKER_TIMEOUT);
}

LLSpeaker::~LLSpeaker()
{
	sSpeakers.erase(mHandle);
}

void LLSpeaker::lookupName()
{
	gCacheName->getName(mID, onAvatarNameLookup, new LLViewHandle(mHandle));
}

//static 
void LLSpeaker::onAvatarNameLookup(const LLUUID& id, const char* first, const char* last, BOOL is_group, void* user_data)
{
	LLViewHandle speaker_handle = *(LLViewHandle*)user_data;
	delete (LLViewHandle*)user_data;

	speaker_map_t::iterator found_it = sSpeakers.find(speaker_handle);
	if (found_it != sSpeakers.end())
	{
		LLSpeaker* speakerp = found_it->second;
		if (speakerp)
		{
			speakerp->mDisplayName = llformat("%s %s", first, last);
		}
	}
}


// helper sort class
struct LLSortRecentSpeakers
{
	bool operator()(const LLPointer<LLSpeaker> lhs, const LLPointer<LLSpeaker> rhs) const;
};

bool LLSortRecentSpeakers::operator()(const LLPointer<LLSpeaker> lhs, const LLPointer<LLSpeaker> rhs) const
{
	// Sort first on status
	if (lhs->mStatus != rhs->mStatus) 
	{
		return (lhs->mStatus < rhs->mStatus);
	}

	// and then on last speaking time
	if(lhs->mLastSpokeTime != rhs->mLastSpokeTime)
	{
		return (lhs->mLastSpokeTime > rhs->mLastSpokeTime);
	}
	
	// and finally (only if those are both equal), on name.
	return(	lhs->mDisplayName.compare(rhs->mDisplayName) < 0 );
}

LLFloaterActiveSpeakers::LLFloaterActiveSpeakers(const LLSD& seed) : mPanel(NULL)
{
	mFactoryMap["active_speakers_panel"] = LLCallbackMap(createSpeakersPanel, NULL);
	// do not automatically open singleton floaters (as result of getInstance())
	BOOL no_open = FALSE;
	gUICtrlFactory->buildFloater(this, "floater_active_speakers.xml", &getFactoryMap(), no_open);	
	//RN: for now, we poll voice client every frame to get voice amplitude feedback
	//gVoiceClient->addObserver(this);
	mPanel->refreshSpeakers();
}

LLFloaterActiveSpeakers::~LLFloaterActiveSpeakers()
{
}

void LLFloaterActiveSpeakers::onClose(bool app_quitting)
{
	setVisible(FALSE);
}

void LLFloaterActiveSpeakers::draw()
{
	// update state every frame to get live amplitude feedback
	mPanel->refreshSpeakers();
	LLFloater::draw();
}

BOOL LLFloaterActiveSpeakers::postBuild()
{
	mPanel = (LLPanelActiveSpeakers*)LLUICtrlFactory::getPanelByName(this, "active_speakers_panel");
	return TRUE;
}

void LLFloaterActiveSpeakers::onChange()
{
	//refresh();
}

//static
void* LLFloaterActiveSpeakers::createSpeakersPanel(void* data)
{
	// don't show text only speakers
	return new LLPanelActiveSpeakers(gActiveChannelSpeakerMgr, FALSE);
}


//
// LLPanelActiveSpeakers
//
LLPanelActiveSpeakers::LLPanelActiveSpeakers(LLSpeakerMgr* data_source, BOOL show_text_chatters) : 
	mSpeakerList(NULL),
	mMuteVoiceCtrl(NULL),
	mMuteTextCtrl(NULL),
	mNameText(NULL),
	mProfileBtn(NULL),
	mShowTextChatters(show_text_chatters),
	mSpeakerMgr(data_source)
{
	setMouseOpaque(FALSE);
}

LLPanelActiveSpeakers::~LLPanelActiveSpeakers()
{
	
}

BOOL LLPanelActiveSpeakers::postBuild()
{
	mSpeakerList = LLUICtrlFactory::getScrollListByName(this, "speakers_list");

	mMuteTextCtrl = (LLUICtrl*)getCtrlByNameAndType("mute_text_btn", WIDGET_TYPE_DONTCARE);
	childSetCommitCallback("mute_text_btn", onClickMuteTextCommit, this);

	mMuteVoiceCtrl = (LLUICtrl*)getCtrlByNameAndType("mute_btn", WIDGET_TYPE_DONTCARE);
	childSetCommitCallback("mute_btn", onClickMuteVoiceCommit, this);
	childSetAction("mute_btn", onClickMuteVoice, this);

	childSetCommitCallback("speaker_volume", onVolumeChange, this);

	mNameText = LLUICtrlFactory::getTextBoxByName(this, "resident_name");
	
	mProfileBtn = LLUICtrlFactory::getButtonByName(this, "profile_btn");
	childSetAction("profile_btn", onClickProfile, this);
	return TRUE;
}

void LLPanelActiveSpeakers::refreshSpeakers()
{
	// store off current selection and scroll state to preserve across list rebuilds
	LLUUID selected_id = mSpeakerList->getSimpleSelectedValue().asUUID();
	S32 scroll_pos = mSpeakerList->getScrollInterface()->getScrollPos();

	BOOL sort_ascending = mSpeakerList->getSortAscending();
	LLString sort_column = mSpeakerList->getSortColumnName();
	// TODO: put this in xml
	// enforces default sort column of speaker status
	if (sort_column.empty())
	{
		sort_column = "speaking_status";
	}

	mSpeakerMgr->update();

	// clear scrolling list widget of names
	mSpeakerList->clearRows();

	LLSpeakerMgr::speaker_list_t speaker_list;
	mSpeakerMgr->getSpeakerList(&speaker_list, mShowTextChatters);
	for (LLSpeakerMgr::speaker_list_t::const_iterator speaker_it = speaker_list.begin(); speaker_it != speaker_list.end(); ++speaker_it)
	{
		LLUUID speaker_id = (*speaker_it)->mID;
		LLPointer<LLSpeaker> speakerp = (*speaker_it);

		// since we are forced to sort by text, encode sort order as string
		LLString speaking_order_sort_string = llformat("%010d", speakerp->mSortIndex);

		LLSD row;
		row["id"] = speaker_id;

		row["columns"][0]["column"] = "icon_speaking_status";
		row["columns"][0]["type"] = "icon";
		row["columns"][0]["color"] = speakerp->mDotColor.getValue();
		LLString icon_image_id;

		S32 icon_image_idx = llmin(2, llfloor((speakerp->mSpeechVolume / LLVoiceClient::OVERDRIVEN_POWER_LEVEL) * 3.f));
		switch(icon_image_idx)
		{
		case 0:
			icon_image_id = gViewerArt.getString("icn_active-speakers-dot-lvl0.tga");
			break;
		case 1:
			icon_image_id = gViewerArt.getString("icn_active-speakers-dot-lvl1.tga");
			break;
		case 2:
			icon_image_id = gViewerArt.getString("icn_active-speakers-dot-lvl2.tga");
			break;
		}
		//if (speakerp->mTyping)
		//{
		//	S32 typing_anim_idx = llround(mIconAnimationTimer.getElapsedTimeF32() * TYPING_ANIMATION_FPS) % 3;
		//	switch(typing_anim_idx)
		//	{
		//	case 0:
		//		row["columns"][0]["overlay"] = LLUUID(gViewerArt.getString("icn_active-speakers-typing1.tga"));
		//		break;
		//	case 1:
		//		row["columns"][0]["overlay"] = LLUUID(gViewerArt.getString("icn_active-speakers-typing2.tga"));
		//		break;
		//	case 2:
		//		row["columns"][0]["overlay"] = LLUUID(gViewerArt.getString("icn_active-speakers-typing3.tga"));
		//		break;
		//	default:
		//		break;
		//	}
		//}

		row["columns"][0]["value"] = speakerp->mStatus == LLSpeaker::STATUS_MUTED ?
			gViewerArt.getString("mute_icon.tga") : icon_image_id;
		if (speakerp->mStatus > LLSpeaker::STATUS_VOICE_ACTIVE) // if voice is disabled for this speaker
		{
			// non voice speakers have hidden icons, render as transparent
			row["columns"][0]["color"] = LLColor4(0.f, 0.f, 0.f, 0.f).getValue();
		}
		row["columns"][1]["column"] = "speaker_name";
		row["columns"][1]["type"] = "text";
		if (speakerp->mStatus == LLSpeaker::STATUS_NOT_IN_CHANNEL)	
		{
			// draw inactive speakers in gray
			row["columns"][1]["color"] = LLColor4::grey4.getValue();
		}

		if (speakerp->mDisplayName.empty())
		{
			row["columns"][1]["value"] = LLCacheName::getDefaultName();
		}
		else
		{
			row["columns"][1]["value"] = speakerp->mDisplayName;
		}

		row["columns"][2]["column"] = "speaking_status";
		row["columns"][2]["type"] = "text";
		
		// print speaking ordinal in a text-sorting friendly manner
		row["columns"][2]["value"] = speaking_order_sort_string;

		mSpeakerList->addElement(row);
	}
	
	//restore sort order, selection, etc
	mSpeakerList->sortByColumn(sort_column, sort_ascending);
	// make sure something is selected
	if (selected_id.isNull())
	{
		mSpeakerList->selectFirstItem();
	}
	else
	{
		mSpeakerList->selectByValue(selected_id);
	}

	LLPointer<LLSpeaker> speakerp = mSpeakerMgr->findSpeaker(selected_id);
	
	if (gMuteListp)
	{
		// update UI for selected participant
		if (mMuteVoiceCtrl)
		{
			mMuteVoiceCtrl->setValue(gMuteListp->isMuted(selected_id, LLMute::flagVoiceChat));
			mMuteVoiceCtrl->setEnabled(selected_id.notNull() 
										&& selected_id != gAgent.getID() 
										&& mSpeakerMgr->isVoiceActive() 
										&& (speakerp.notNull() && speakerp->mType == LLSpeaker::SPEAKER_AGENT));
		}
		if (mMuteTextCtrl)
		{
			mMuteTextCtrl->setValue(gMuteListp->isMuted(selected_id, LLMute::flagTextChat));
			mMuteTextCtrl->setEnabled(selected_id.notNull() && selected_id != gAgent.getID() && speakerp.notNull() && !gMuteListp->isLinden(speakerp->mDisplayName));
		}
		childSetValue("speaker_volume", gVoiceClient->getUserVolume(selected_id));
		childSetEnabled("speaker_volume", selected_id.notNull() 
						&& selected_id != gAgent.getID() 
						&& mSpeakerMgr->isVoiceActive() 
						&& (speakerp.notNull() && speakerp->mType == LLSpeaker::SPEAKER_AGENT));
		if (mProfileBtn)
		{
			mProfileBtn->setEnabled(selected_id.notNull());
		}
	}

	// show selected user name in large font
	if (mNameText)
	{
		if (speakerp)
		{
			mNameText->setValue(speakerp->mDisplayName);
		}
		else
		{
			mNameText->setValue("");
		}
	}

	// keep scroll value stable
	mSpeakerList->getScrollInterface()->setScrollPos(scroll_pos);
}

void LLPanelActiveSpeakers::setSpeaker(const LLUUID& id, const LLString& name, LLSpeaker::ESpeakerStatus status, LLSpeaker::ESpeakerType type)
{
	mSpeakerMgr->setSpeaker(id, name, status, type);
}


//static
void LLPanelActiveSpeakers::onClickMuteTextCommit(LLUICtrl* ctrl, void* user_data)
{
	LLPanelActiveSpeakers* panelp = (LLPanelActiveSpeakers*)user_data;
	LLUUID speaker_id = panelp->mSpeakerList->getValue().asUUID();
	BOOL is_muted = gMuteListp->isMuted(speaker_id, LLMute::flagTextChat);
	std::string name;

	//fill in name using voice client's copy of name cache
	LLPointer<LLSpeaker> speakerp = panelp->mSpeakerMgr->findSpeaker(speaker_id);
	if (speakerp.isNull())
	{
		return;
	}
	
	name = speakerp->mDisplayName;

	LLMute mute(speaker_id, name, speakerp->mType == LLSpeaker::SPEAKER_AGENT ? LLMute::AGENT : LLMute::OBJECT);

	if (!is_muted)
	{
		gMuteListp->add(mute, LLMute::flagTextChat);
	}
	else
	{
		gMuteListp->remove(mute, LLMute::flagTextChat);
	}
}

//static
void LLPanelActiveSpeakers::onClickMuteVoice(void* user_data)
{
	onClickMuteVoiceCommit(NULL, user_data);
}

//static
void LLPanelActiveSpeakers::onClickMuteVoiceCommit(LLUICtrl* ctrl, void* user_data)
{
	LLPanelActiveSpeakers* panelp = (LLPanelActiveSpeakers*)user_data;
	LLUUID speaker_id = panelp->mSpeakerList->getValue().asUUID();
	BOOL is_muted = gMuteListp->isMuted(speaker_id, LLMute::flagVoiceChat);
	std::string name;

	LLPointer<LLSpeaker> speakerp = panelp->mSpeakerMgr->findSpeaker(speaker_id);
	if (speakerp.isNull())
	{
		return;
	}

	name = speakerp->mDisplayName;

	// muting voice means we're dealing with an agent
	LLMute mute(speaker_id, name, LLMute::AGENT);

	if (!is_muted)
	{
		gMuteListp->add(mute, LLMute::flagVoiceChat);
	}
	else
	{
		gMuteListp->remove(mute, LLMute::flagVoiceChat);
	}
}


//static
void LLPanelActiveSpeakers::onVolumeChange(LLUICtrl* source, void* user_data)
{
	LLPanelActiveSpeakers* panelp = (LLPanelActiveSpeakers*)user_data;
	LLUUID speaker_id = panelp->mSpeakerList->getValue().asUUID();

	gVoiceClient->setUserVolume(speaker_id, (F32)panelp->childGetValue("speaker_volume").asReal());
}

//static 
void LLPanelActiveSpeakers::onClickProfile(void* user_data)
{
	LLPanelActiveSpeakers* panelp = (LLPanelActiveSpeakers*)user_data;
	LLUUID speaker_id = panelp->mSpeakerList->getValue().asUUID();

	LLFloaterAvatarInfo::showFromDirectory(speaker_id);
}
 
//
// LLSpeakerMgr
//

LLSpeakerMgr::LLSpeakerMgr(LLVoiceChannel* channelp) : 
	mVoiceChannel(channelp)
{
}

LLSpeakerMgr::~LLSpeakerMgr()
{
}

LLPointer<LLSpeaker> LLSpeakerMgr::setSpeaker(const LLUUID& id, const LLString& name, LLSpeaker::ESpeakerStatus status, LLSpeaker::ESpeakerType type)
{
	if (id.isNull()) return NULL;

	LLPointer<LLSpeaker> speakerp;
	if (mSpeakers.find(id) == mSpeakers.end())
	{
		speakerp = new LLSpeaker(id, name, type);
		speakerp->mStatus = status;
		mSpeakers.insert(std::make_pair(speakerp->mID, speakerp));
		mSpeakersSorted.push_back(speakerp);
	}
	else
	{
		speakerp = findSpeaker(id);
		if (speakerp.notNull())
		{
			// keep highest priority status (lowest value) instead of overriding current value
			speakerp->mStatus = llmin(speakerp->mStatus, status);
			speakerp->mActivityTimer.resetWithExpiry(SPEAKER_TIMEOUT);
			// RN: due to a weird behavior where IMs from attached objects come from the wearer's agent_id
			// we need to override speakers that we think are objects when we find out they are really
			// residents
			if (type == LLSpeaker::SPEAKER_AGENT)
			{
				speakerp->mType = LLSpeaker::SPEAKER_AGENT;
				speakerp->lookupName();
			}
		}
	}

	return speakerp;
}

void LLSpeakerMgr::update()
{
	if (!gVoiceClient)
	{
		return;
	}
	
	LLColor4 speaking_color = gSavedSettings.getColor4("SpeakingColor");
	LLColor4 overdriven_color = gSavedSettings.getColor4("OverdrivenColor");

	updateSpeakerList();

	// update status of all current speakers
	BOOL voice_channel_active = (!mVoiceChannel && gVoiceClient->inProximalChannel()) || (mVoiceChannel && mVoiceChannel->isActive());
	for (speaker_map_t::iterator speaker_it = mSpeakers.begin(); speaker_it != mSpeakers.end();)
	{
		LLUUID speaker_id = speaker_it->first;
		LLSpeaker* speakerp = speaker_it->second;
		
		speaker_map_t::iterator  cur_speaker_it = speaker_it++;

		if (voice_channel_active && gVoiceClient->getVoiceEnabled(speaker_id))
		{
			speakerp->mSpeechVolume = gVoiceClient->getCurrentPower(speaker_id);

			if (gVoiceClient->getOnMuteList(speaker_id))
			{
				speakerp->mStatus = LLSpeaker::STATUS_MUTED;
				speakerp->mDotColor = LLColor4::white;
			}
			else if (gVoiceClient->getIsSpeaking(speaker_id))
			{
				// reset inactivity expiration
				if (speakerp->mStatus != LLSpeaker::STATUS_SPEAKING)
				{
					speakerp->mLastSpokeTime = mSpeechTimer.getElapsedTimeF32();
					speakerp->mHasSpoken = TRUE;
				}
				speakerp->mStatus = LLSpeaker::STATUS_SPEAKING;
				// interpolate between active color and full speaking color based on power of speech output
				speakerp->mDotColor = speaking_color;
				if (speakerp->mSpeechVolume > LLVoiceClient::OVERDRIVEN_POWER_LEVEL)
				{
					speakerp->mDotColor = overdriven_color;
				}
			}
			else
			{
				speakerp->mSpeechVolume = 0.f;
				speakerp->mDotColor = ACTIVE_COLOR;

				if (speakerp->mHasSpoken)
				{
					// have spoken once, not currently speaking
					speakerp->mStatus = LLSpeaker::STATUS_HAS_SPOKEN;
				}
				else
				{
					// default state for being in voice channel
					speakerp->mStatus = LLSpeaker::STATUS_VOICE_ACTIVE;
				}
			}
		}
		// speaker no longer registered in voice channel, demote to text only
		else if (speakerp->mStatus != LLSpeaker::STATUS_NOT_IN_CHANNEL)
		{
			speakerp->mStatus = LLSpeaker::STATUS_TEXT_ONLY;
			speakerp->mSpeechVolume = 0.f;
			speakerp->mDotColor = ACTIVE_COLOR;
		}
	}

	// sort by status then time last spoken
	std::sort(mSpeakersSorted.begin(), mSpeakersSorted.end(), LLSortRecentSpeakers());

	// for recent speakers who are not currently speaking, show "recent" color dot for most recent
	// fading to "active" color

	S32 recent_speaker_count = 0;
	S32 sort_index = 0;
	speaker_list_t::iterator sorted_speaker_it;
	for(sorted_speaker_it = mSpeakersSorted.begin(); 
		sorted_speaker_it != mSpeakersSorted.end(); )
	{
		LLPointer<LLSpeaker> speakerp = *sorted_speaker_it;
		
		// color code recent speakers who are not currently speaking
		if (speakerp->mStatus == LLSpeaker::STATUS_HAS_SPOKEN)
		{
			speakerp->mDotColor = lerp(speaking_color, ACTIVE_COLOR, clamp_rescale((F32)recent_speaker_count, -2.f, 3.f, 0.f, 1.f));
			recent_speaker_count++;
		}

		// stuff sort ordinal into speaker so the ui can sort by this value
		speakerp->mSortIndex = sort_index++;

		// remove speakers that have been gone too long
		if (speakerp->mStatus == LLSpeaker::STATUS_NOT_IN_CHANNEL && speakerp->mActivityTimer.hasExpired())
		{
			mSpeakers.erase(speakerp->mID);
			sorted_speaker_it = mSpeakersSorted.erase(sorted_speaker_it);
		}
		else
		{
			++sorted_speaker_it;
		}
	}
}

void LLSpeakerMgr::updateSpeakerList()
{
	// are we bound to the currently active voice channel?
	if ((!mVoiceChannel && gVoiceClient->inProximalChannel()) || (mVoiceChannel && mVoiceChannel->isActive()))
	{
		LLVoiceClient::participantMap* participants = gVoiceClient->getParticipantList();
		LLVoiceClient::participantMap::iterator participant_it;

		// add new participants to our list of known speakers
		for (participant_it = participants->begin(); participant_it != participants->end(); ++participant_it)
		{
			LLVoiceClient::participantState* participantp = participant_it->second;
			setSpeaker(participantp->mAvatarID, "", LLSpeaker::STATUS_VOICE_ACTIVE);
		}
	}
}

const LLPointer<LLSpeaker> LLSpeakerMgr::findSpeaker(const LLUUID& speaker_id)
{
	speaker_map_t::iterator found_it = mSpeakers.find(speaker_id);
	if (found_it == mSpeakers.end())
	{
		return NULL;
	}
	return found_it->second;
}

void LLSpeakerMgr::getSpeakerList(speaker_list_t* speaker_list, BOOL include_text)
{
	speaker_list->clear();
	for (speaker_map_t::iterator speaker_it = mSpeakers.begin(); speaker_it != mSpeakers.end(); ++speaker_it)
	{
		LLPointer<LLSpeaker> speakerp = speaker_it->second;
		// what about text only muted or inactive?
		if (include_text || speakerp->mStatus != LLSpeaker::STATUS_TEXT_ONLY)
		{
			speaker_list->push_back(speakerp);
		}
	}
}

void LLSpeakerMgr::setSpeakerTyping(const LLUUID& speaker_id, BOOL typing)
{
	LLPointer<LLSpeaker> speakerp = findSpeaker(speaker_id);
	if (speakerp.notNull())
	{
		speakerp->mTyping = typing;
	}
}

// speaker has chatted via either text or voice
void LLSpeakerMgr::speakerChatted(const LLUUID& speaker_id)
{
	LLPointer<LLSpeaker> speakerp = findSpeaker(speaker_id);
	if (speakerp.notNull())
	{
		speakerp->mLastSpokeTime = mSpeechTimer.getElapsedTimeF32();
		speakerp->mHasSpoken = TRUE;
	}
}

BOOL LLSpeakerMgr::isVoiceActive()
{
	// mVoiceChannel = NULL means current voice channel, whatever it is
	return LLVoiceClient::voiceEnabled() && mVoiceChannel && mVoiceChannel->isActive();
}


//
// LLIMSpeakerMgr
//
LLIMSpeakerMgr::LLIMSpeakerMgr(LLVoiceChannel* channel) : LLSpeakerMgr(channel)
{
}

void LLIMSpeakerMgr::updateSpeakerList()
{
	// don't do normal updates which are pulled from voice channel
	// rely on user list reported by sim
	return;
}

void LLIMSpeakerMgr::processSpeakerList(LLSD list)
{
	for(LLSD::array_iterator list_it = list.beginArray();
		list_it != list.endArray();
		++list_it)
	{
		LLUUID agent_id(list_it->asUUID());

		setSpeaker(agent_id, "", LLSpeaker::STATUS_TEXT_ONLY);
	}
}

void LLIMSpeakerMgr::processSpeakerMap(LLSD map)
{
	for(LLSD::map_iterator map_it = map.beginMap();
		map_it != map.endMap();
		++map_it)
	{
		// add as new speaker
		setSpeaker(LLUUID(map_it->first));
	}
}



void LLIMSpeakerMgr::processSpeakerListUpdate(LLSD update)
{
	for(LLSD::map_iterator update_it = update.beginMap();
		update_it != update.endMap();
		++update_it)
	{
		LLUUID agent_id(update_it->first);
		
		if (update_it->second.asString() == "LEAVE")
		{
			LLPointer<LLSpeaker> speakerp = findSpeaker(agent_id);
			if (speakerp)
			{
				speakerp->mStatus = LLSpeaker::STATUS_NOT_IN_CHANNEL;
				speakerp->mDotColor = INACTIVE_COLOR;
				speakerp->mActivityTimer.resetWithExpiry(SPEAKER_TIMEOUT);
			}
		}
		else if (update_it->second.asString() == "ENTER")
		{
			// add or update speaker
			setSpeaker(agent_id);
		}
		else
		{
			llwarns << "LLIMSpeakerMgr::processSpeakerListUpdate() : bad membership list update " << ll_print_sd(update_it->second) << llendl;
		}
	}
}


//
// LLActiveSpeakerMgr
//

LLActiveSpeakerMgr::LLActiveSpeakerMgr() : LLSpeakerMgr(NULL)
{
}

void LLActiveSpeakerMgr::updateSpeakerList()
{
	// point to whatever the current voice channel is
	mVoiceChannel = LLVoiceChannel::getCurrentVoiceChannel();

	// always populate from active voice channel
	if (LLVoiceChannel::getCurrentVoiceChannel() != mVoiceChannel)
	{
		mSpeakers.clear();
		mSpeakersSorted.clear();
		mVoiceChannel = LLVoiceChannel::getCurrentVoiceChannel();
	}
	LLSpeakerMgr::updateSpeakerList();
}



//
// LLLocalSpeakerMgr
//

LLLocalSpeakerMgr::LLLocalSpeakerMgr() : LLSpeakerMgr(LLVoiceChannelProximal::getInstance())
{
}

LLLocalSpeakerMgr::~LLLocalSpeakerMgr ()
{
}

void LLLocalSpeakerMgr::updateSpeakerList()
{
	// pull speakers from voice channel
	LLSpeakerMgr::updateSpeakerList();

	// add non-voice speakers in chat range
	std::vector< LLCharacter* >::iterator avatar_it;
	for(avatar_it = LLCharacter::sInstances.begin(); avatar_it != LLCharacter::sInstances.end(); ++avatar_it)
	{
		LLVOAvatar* avatarp = (LLVOAvatar*)*avatar_it;
		if (dist_vec(avatarp->getPositionAgent(), gAgent.getPositionAgent()) <= CHAT_NORMAL_RADIUS)
		{
			setSpeaker(avatarp->getID());
		}
	}

	// check if text only speakers have moved out of chat range
	for (speaker_map_t::iterator speaker_it = mSpeakers.begin(); speaker_it != mSpeakers.end(); ++speaker_it)
	{
		LLUUID speaker_id = speaker_it->first;
		LLSpeaker* speakerp = speaker_it->second;
		if (speakerp->mStatus == LLSpeaker::STATUS_TEXT_ONLY)
		{
			LLVOAvatar* avatarp = (LLVOAvatar*)gObjectList.findObject(speaker_id);
			if (!avatarp || dist_vec(avatarp->getPositionAgent(), gAgent.getPositionAgent()) > CHAT_NORMAL_RADIUS)
			{
				speakerp->mStatus = LLSpeaker::STATUS_NOT_IN_CHANNEL;
				speakerp->mDotColor = INACTIVE_COLOR;
				speakerp->mActivityTimer.resetWithExpiry(SPEAKER_TIMEOUT);
			}
		}
	}
}
