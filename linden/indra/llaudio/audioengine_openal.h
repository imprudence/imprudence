/**
 * @file audioengine_openal.cpp
 * @brief implementation of audio engine using OpenAL
 * support as a OpenAL 3D implementation
 *
 * Copyright (c) 2002-2008, Linden Research, Inc.
 *
 * $LicenseInfo:firstyear=2002&license=viewergpl$
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
 * $/LicenseInfo$
 */


#ifndef LL_AUDIOENGINE_OpenAL_H
#define LL_AUDIOENGINE_OpenAL_H

#include <string>

#include "audioengine.h"
#include "listener_openal.h"
#include "llmediamanager.h"
#include "llmediaimplgstreamer.h"
#include "llrand.h"

class LLAudioEngine_OpenAL : public LLAudioEngine
{
	public:
		LLAudioEngine_OpenAL();
		virtual ~LLAudioEngine_OpenAL();

		virtual BOOL init(const S32 num_channels);
		virtual std::string getDriverName(bool verbose);
		virtual void allocateListener();

		virtual void shutdown();

		virtual void idle(F32 max_decode_time = 0.f);

		void setInternalGain(F32 gain);

		LLAudioBuffer* createBuffer();
		LLAudioChannel* createChannel();

		// Internet stream methods
		virtual void initInternetStream();
		virtual void startInternetStream(const std::string& url_cstr);
		virtual void stopInternetStream();
		virtual void updateInternetStream();
		virtual void pauseInternetStream(int pause);
		virtual int isInternetStreamPlaying();
		virtual void getInternetStreamInfo(char* artist, char* title);
		virtual void setInternetStreamGain(F32 vol);
		virtual const std::string& getInternetStreamURL();
		virtual void InitStreamer();

		void checkALError();

		void initWind();
		void cleanupWind();
		void updateWind(LLVector3 direction, F32 camera_altitude);

	protected: 
		static const S32 mNumWindBuffers=20;
		static const S32 mSampleRate=44100;
		static const S32 mBytesPerSample=4;
		static const S32 mWindDataSize=8820; //44100 * 0.200 * 2 channels * 2 bytes per sample

		BOOL mFirstWind;
		ALuint mWindBuffers[mNumWindBuffers];
		ALuint mWindSource;

		F32 mTargetGain;
		F32 mTargetFreq;
		F32 mTargetPanGainR;
		S16 mWindData[mWindDataSize];

		std::string mInternetStreamURL;
		void * windDSP(void *newbuffer, int length);
#if LL_GSTREAMER_ENABLED
		LLMediaManagerData * mMedia_data;
		LLMediaImplGStreamer * m_streamer;
#endif
};

class LLAudioChannelOpenAL : public LLAudioChannel
{
	public:
		LLAudioChannelOpenAL();
		virtual ~LLAudioChannelOpenAL();
	protected:
		void play();
		void playSynced(LLAudioChannel *channelp);
		void cleanup();
		bool isPlaying();

		bool updateBuffer();
		void update3DPosition();
		void updateLoop(){};

		ALuint ALSource;
};

class LLAudioBufferOpenAL : public LLAudioBuffer{
	public:
		LLAudioBufferOpenAL();
		virtual ~LLAudioBufferOpenAL();

		bool loadWAV(const std::string& filename);
		U32 getLength();

		friend class LLAudioChannelOpenAL;
	protected:
		void cleanup();
		ALuint getBuffer(){return ALBuffer;}
		ALuint ALBuffer;
};

#endif
