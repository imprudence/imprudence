 /** 
 * @file kokuastreamingaudio.cpp
 * @brief Definition of KOKUAStreamingAudio base class for streaming audio support
 *
 * $LicenseInfo:firstyear=2011&license=viewergpl$
 * Kokua Viewer Source Code
 * Copyright (C) 2011, Armin.Weatherwax (at) googlemail.com
 * for the Kokua Viewer Team, in special for our Imprudence Viewer.
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

#include "kokuastreamingaudio.h"
#include "llstreamingaudio.h"


KOKUAStreamingAudio* gAudioStream = NULL;

KOKUAStreamingAudio::KOKUAStreamingAudio(LLStreamingAudioInterface *impl) :
mStreamingAudioImpl (impl)
{

}

KOKUAStreamingAudio::~KOKUAStreamingAudio()
{
	if (mStreamingAudioImpl)
	{
		delete mStreamingAudioImpl;
		mStreamingAudioImpl = NULL;
	}
}

LLStreamingAudioInterface* KOKUAStreamingAudio::getStreamingAudioImpl()
{
	return mStreamingAudioImpl;
}

// void KOKUAStreamingAudio::setStreamingAudioImpl(LLStreamingAudioInterface *impl)
// {
// 	mStreamingAudioImpl = impl;
// }

// virtual
void KOKUAStreamingAudio::startInternetStream(const std::string& url)
{
	if (mStreamingAudioImpl)
		mStreamingAudioImpl->start(url);
}

// virtual
void KOKUAStreamingAudio::stopInternetStream()
{
	if (mStreamingAudioImpl)
		mStreamingAudioImpl->stop();
}

// virtual
void KOKUAStreamingAudio::pauseInternetStream(int pause)
{
	if (mStreamingAudioImpl)
		mStreamingAudioImpl->pause(pause);
}

// virtual
void KOKUAStreamingAudio::updateInternetStream()
{
	if (mStreamingAudioImpl)
		mStreamingAudioImpl->update();
}

// virtual
KOKUAStreamingAudio::KOKUAAudioPlayState KOKUAStreamingAudio::isInternetStreamPlaying()
{
	if (mStreamingAudioImpl)
		return (KOKUAStreamingAudio::KOKUAAudioPlayState) mStreamingAudioImpl->isPlaying();

	return KOKUAStreamingAudio::AUDIO_STOPPED; // Stopped
}

// virtual
void KOKUAStreamingAudio::setInternetStreamGain(F32 vol)
{
	if (mStreamingAudioImpl)
		mStreamingAudioImpl->setGain(vol);
}

F32 KOKUAStreamingAudio::getInternetStreamGain()
{
	if (mStreamingAudioImpl)
		return mStreamingAudioImpl->getGain();
	else
		return 1.0f;
}

// virtual
std::string KOKUAStreamingAudio::getInternetStreamURL()
{
	if (mStreamingAudioImpl)
		return mStreamingAudioImpl->getURL();
	else return std::string();
}

std::string KOKUAStreamingAudio::getVersion()
{
	if (mStreamingAudioImpl)
		return mStreamingAudioImpl->getVersion();
	else return std::string();
}