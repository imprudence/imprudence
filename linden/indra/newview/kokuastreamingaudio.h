/** 
 * @file kokuastreamingaudio.h
 * @brief Definition of KOKUAStreamingAudio base class for streaming audio support
 *
 * $LicenseInfo:firstyear=2011&license=viewergpl$
 * Kokua Viewer Source Code
 * Copyright (C) 2011, Armin.Weatherwax (at) googlemail.com
 * for the Kokua Viewer Team.
 * 
 * The source code in this file ("Source Code") is provided by the author
 * to you under the terms of the GNU General Public License, version 2.0
 * ("GPL").  Terms of the GPL can be found in doc/GPL-license.txt in this
 * distribution, or online at 
 * http://secondlifegrid.net/programs/open_source/licensing/gplv2
 * 
 * There are special exceptions to the terms and conditions of the GPL as
 * it is applied to this Source Code. View the full text of the exception
 * in the file doc/FLOSS-exception.txt in this software distribution, or
 * online at
 * http://secondlifegrid.net/programs/open_source/licensing/flossexception
 * 
 * By copying, modifying or distributing this software, you acknowledge
 * that you have read and understood your obligations described above,
 * and agree to abide by those obligations.
 * 
 * THIS SOURCE CODE IS PROVIDED "AS IS." THE AUTHOR MAKES NO
 * WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
 * COMPLETENESS OR PERFORMANCE.
 *
 * $/LicenseInfo$
 */
#ifndef KOKUA_STREAMINGAUDIO_H
#define KOKUA_STREAMINGAUDIO_H

#include "linden_common.h"

class LLStreamingAudioInterface;

class KOKUAStreamingAudio 
{
public:
	enum KOKUAAudioPlayState
	{
		// isInternetStreamPlaying() returns an *int*, with
		// 0 = stopped, 1 = playing, 2 = paused.
		AUDIO_STOPPED = 0,
		AUDIO_PLAYING = 1,
		AUDIO_PAUSED = 2
	};

	KOKUAStreamingAudio(LLStreamingAudioInterface *impl);
	virtual ~KOKUAStreamingAudio();

	LLStreamingAudioInterface *getStreamingAudioImpl();
// 	void setStreamingAudioImpl(LLStreamingAudioInterface *impl);
	void startInternetStream(const std::string& url);
	void stopInternetStream();
	void pauseInternetStream(int pause);
	void updateInternetStream(); 
	KOKUAAudioPlayState isInternetStreamPlaying();
	void setInternetStreamGain(F32 vol);
	F32 getInternetStreamGain();
	std::string getInternetStreamURL();
	std::string getVersion();	
private:
	LLStreamingAudioInterface *mStreamingAudioImpl;
};

extern KOKUAStreamingAudio* gAudioStream;

#endif
