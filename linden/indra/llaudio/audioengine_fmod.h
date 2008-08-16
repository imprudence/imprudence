/** 
 * @file audioengine_fmod.h
 * @brief Definition of LLAudioEngine class abstracting the audio
 * support as a FMOD 3D implementation
 *
 * Copyright (c) 2002-2007, Linden Research, Inc.
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

#ifndef LL_AUDIOENGINE_FMOD_H
#define LL_AUDIOENGINE_FMOD_H

#include "audioengine.h"

#if LL_FMOD

#include "listener_fmod.h"

#include "fmod.h"

class LLAudioStreamFMOD;

class LLAudioEngine_FMOD : public LLAudioEngine 
{
public:
	LLAudioEngine_FMOD();
	virtual ~LLAudioEngine_FMOD();

	// initialization/startup/shutdown
	virtual BOOL init(const S32 num_channels, void *user_data);
	virtual void allocateListener();

	virtual void shutdown();

	virtual void idle(F32 max_decode_time = 0.f);

	// Internet stream methods
	virtual void initInternetStream();
	virtual void startInternetStream(const char* url);
	virtual void updateInternetStream();
	virtual void stopInternetStream();
	virtual void pauseInternetStream(int pause);
	virtual int isInternetStreamPlaying();
	virtual void getInternetStreamInfo(char* artist, char* title);
	virtual void setInternetStreamGain(F32 vol);
	virtual const char* getInternetStreamURL();

	/*virtual*/ void initWind();
	/*virtual*/ void cleanupWind();

	/*virtual*/void updateWind(LLVector3 direction, F32 camera_height_above_water);

protected:
	/*virtual*/ LLAudioBuffer *createBuffer(); // Get a free buffer, or flush an existing one if you have to.
	/*virtual*/ LLAudioChannel *createChannel(); // Create a new audio channel.

	/*virtual*/ void setInternalGain(F32 gain);
protected:
	static signed char F_CALLBACKAPI callbackMetaData(char* name, char* value, void* userdata);

	LLAudioStreamFMOD *mCurrentInternetStreamp;
	char mInternetStreamURL[1024];	/*Flawfinder: ignore*/
	int mInternetStreamChannel;

	std::list<LLAudioStreamFMOD *> mDeadStreams;

	//F32 mMinDistance[MAX_BUFFERS];
	//F32 mMaxDistance[MAX_BUFFERS];

	S32 mFadeIn;
	BOOL mInited;

	// On Windows, userdata is the HWND of the application window.
	void* mUserData;

};


class LLAudioChannelFMOD : public LLAudioChannel
{
public:
	LLAudioChannelFMOD();
	virtual ~LLAudioChannelFMOD();

protected:
	/*virtual*/ void play();
	/*virtual*/ void playSynced(LLAudioChannel *channelp);
	/*virtual*/ void cleanup();
	/*virtual*/ BOOL isPlaying();

	/*virtual*/ BOOL updateBuffer();
	/*virtual*/ void update3DPosition();
	/*virtual*/ void updateLoop();

protected:
	int mChannelID;
	S32 mLastSamplePos;
};


class LLAudioBufferFMOD : public LLAudioBuffer
{
public:
	LLAudioBufferFMOD();
	virtual ~LLAudioBufferFMOD();

	/*virtual*/ BOOL loadWAV(const char *filename);
	/*virtual*/ U32 getLength();
	friend class LLAudioChannelFMOD;

	void set3DMode(BOOL use3d);
protected:
	FSOUND_SAMPLE *getSample()	{ return mSamplep; }
protected:
	FSOUND_SAMPLE *mSamplep;
};

class LLAudioStreamFMOD
{
public:
	LLAudioStreamFMOD(const char *url);
	int	startStream();
	BOOL stopStream(); // Returns true if the stream was successfully stopped.
	BOOL ready();

	char *getURL() 	{ return mInternetStreamURL; }

	int getOpenState();
protected:
	FSOUND_STREAM* mInternetStream;
	BOOL mReady;

	char mInternetStreamURL[1024];	/*Flawfinder: ignore*/
};

#endif

#endif

