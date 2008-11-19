/**
 * @file audioengine_openal.cpp
 * @brief implementation of audio engine using OpenAL
 * support as a OpenAL 3D implementation
 *
 * $LicenseInfo:firstyear=2002&license=viewergpl$
 *
 * Copyright (c) 2002-2008, Linden Research, Inc.
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

#include "linden_common.h"
#include "lldir.h"

#include "audioengine_openal.h"
#include "listener_openal.h"


// Variables and definitions for Wind
#define MAX_NUM_WIND_BUFFERS 40
static int empty_num_wind_buffers = MAX_NUM_WIND_BUFFERS;
static const float wind_buffer_size_sec = 0.05f; // 1/20th sec
static const U32 wind_gen_freq = LLWindGen<S16>::getInputSamplingRate();
static ALuint wind_source;
static S16 *winddata=NULL;


LLAudioEngine_OpenAL::LLAudioEngine_OpenAL()
{
	mWindGen = NULL;
}

LLAudioEngine_OpenAL::~LLAudioEngine_OpenAL()
{
}

bool LLAudioEngine_OpenAL::init(const S32 num_channels, void* userdata)
{
	mWindGen = NULL;
	LLAudioEngine::init(num_channels, userdata);

	if(!alutInit(NULL, NULL))
	{
		llwarns << "LLAudioEngine_OpenAL::init() ALUT initialization failed: " << alutGetErrorString (alutGetError ()) << llendl;
		return false;
	}

	llinfos << "LLAudioEngine_OpenAL::init() OpenAL successfully initialized" << llendl;

	llinfos << "OpenAL version: "
		<< ll_safe_string(alGetString(AL_VERSION)) << llendl;
	llinfos << "OpenAL vendor: "
		<< ll_safe_string(alGetString(AL_VENDOR)) << llendl;
	llinfos << "OpenAL renderer: "
		<< ll_safe_string(alGetString(AL_RENDERER)) << llendl;

	ALint major = alutGetMajorVersion ();
	ALint minor = alutGetMinorVersion ();
	llinfos << "ALUT version: " << major << "." << minor << llendl;

	ALCdevice *device = alcGetContextsDevice(alcGetCurrentContext());

	alcGetIntegerv(device, ALC_MAJOR_VERSION, 1, &major);
	alcGetIntegerv(device, ALC_MAJOR_VERSION, 1, &minor);
	llinfos << "ALC version: " << major << "." << minor << llendl;

	llinfos << "ALC default device: "
		<< ll_safe_string(alcGetString(device,
					       ALC_DEFAULT_DEVICE_SPECIFIER))
		<< llendl;

	llinfos << "LLAudioEngine_OpenAL::init() Speed of sound is: " << alGetFloat(AL_SPEED_OF_SOUND) << llendl;

	return true;
}

void LLAudioEngine_OpenAL::allocateListener()
{
	mListenerp = (LLListener *) new LLListener_OpenAL();
	if(!mListenerp)
	{
		llwarns << "LLAudioEngine_OpenAL::allocateListener() Listener creation failed" << llendl;
	}
}

void LLAudioEngine_OpenAL::shutdown()
{
	llinfos << "About to LLAudioEngine::shutdown()" << llendl;
	LLAudioEngine::shutdown();

	llinfos << "About to alutExit()" << llendl;
	if(!alutExit())
	{
		llwarns << "Nuts." << llendl;
		llwarns << "LLAudioEngine_OpenAL::shutdown() ALUT shutdown failed: " << alutGetErrorString (alutGetError ()) << llendl;
	}

	llinfos << "LLAudioEngine_OpenAL::shutdown() OpenAL successfully shut down" << llendl;

	delete mListenerp;
	mListenerp = NULL;
}

LLAudioBuffer *LLAudioEngine_OpenAL::createBuffer()
{
	return new LLAudioBufferOpenAL();
}

LLAudioChannel *LLAudioEngine_OpenAL::createChannel()
{
	return new LLAudioChannelOpenAL();
}

void LLAudioEngine_OpenAL::setInternalGain(F32 gain)
{
	//llinfos << "LLAudioEngine_OpenAL::setInternalGain() Gain: " << gain << llendl;
	alListenerf(AL_GAIN, gain);
}

LLAudioChannelOpenAL::LLAudioChannelOpenAL()
{
	alGenSources(1, &mALSource);
}

LLAudioChannelOpenAL::~LLAudioChannelOpenAL()
{
	cleanup();
	alDeleteSources(1, &mALSource);
}

void LLAudioChannelOpenAL::cleanup()
{
	alSourceStop(mALSource);
	mCurrentBufferp = NULL;
}

void LLAudioChannelOpenAL::play()
{
	if(!isPlaying()){
		alSourcePlay(mALSource);
		getSource()->setPlayedOnce(true);
	}
}

void LLAudioChannelOpenAL::playSynced(LLAudioChannel *channelp)
{
	play();
}

bool LLAudioChannelOpenAL::isPlaying()
{
	ALint state;
	alGetSourcei(mALSource, AL_SOURCE_STATE, &state);
	if(state == AL_PLAYING){
		return true;
	}
	return false;
}

bool LLAudioChannelOpenAL::updateBuffer()
{
	if (LLAudioChannel::updateBuffer())
	{
		// Base class update returned true, which means that we need to actually
		// set up the source for a different buffer.
		LLAudioBufferOpenAL *bufferp = (LLAudioBufferOpenAL *)mCurrentSourcep->getCurrentBuffer();
		alSourcei(mALSource, AL_BUFFER, bufferp->getBuffer());
		alSourcef(mALSource, AL_GAIN, mCurrentSourcep->getGain());
		alSourcei(mALSource, AL_LOOPING, mCurrentSourcep->isLoop() ? AL_TRUE : AL_FALSE);
	}

	return true;
}

void LLAudioChannelOpenAL::update3DPosition()
{
	if(!mCurrentSourcep)
	{
		return;
	}
	if (mCurrentSourcep->isAmbient())
	{
		alSource3f(mALSource, AL_POSITION, 0.0, 0.0, 0.0);
		alSource3f(mALSource, AL_VELOCITY, 0.0, 0.0, 0.0);
		//alSource3f(mALSource, AL_DIRECTION, 0.0, 0.0, 0.0);
		alSourcef (mALSource, AL_ROLLOFF_FACTOR, 0.0);
		alSourcei (mALSource, AL_SOURCE_RELATIVE, AL_TRUE);
	} else {
		LLVector3 float_pos;
		float_pos.setVec(mCurrentSourcep->getPositionGlobal());
		alSourcefv(mALSource, AL_POSITION, float_pos.mV);
		//llinfos << "LLAudioChannelOpenAL::update3DPosition() Velocity: " << mCurrentSourcep->getVelocity() << llendl;
		alSourcefv(mALSource, AL_VELOCITY, mCurrentSourcep->getVelocity().mV);
		//alSource3f(mALSource, AL_DIRECTION, 0.0, 0.0, 0.0);
		alSourcef (mALSource, AL_ROLLOFF_FACTOR, 1.0);
		alSourcei (mALSource, AL_SOURCE_RELATIVE, AL_FALSE);
	}
	//llinfos << "LLAudioChannelOpenAL::update3DPosition() Gain: " << mCurrentSourcep->getGain() << llendl;
	alSourcef(mALSource, AL_GAIN, mCurrentSourcep->getGain());
}

LLAudioBufferOpenAL::LLAudioBufferOpenAL()
{
	mALBuffer = AL_NONE;
}

LLAudioBufferOpenAL::~LLAudioBufferOpenAL()
{
	cleanup();
}

void LLAudioBufferOpenAL::cleanup()
{
	if(mALBuffer != AL_NONE)
	{
		alDeleteBuffers(1, &mALBuffer);
		mALBuffer = AL_NONE;
	}
}

bool LLAudioBufferOpenAL::loadWAV(const std::string& filename)
{
	cleanup();
	mALBuffer = alutCreateBufferFromFile(filename.c_str());
	if(mALBuffer == AL_NONE){
		ALenum error = alutGetError(); 
		if (gDirUtilp->fileExists(filename)) {
			llwarns <<
				"LLAudioBufferOpenAL::loadWAV() Error loading "
				<< filename
				<< " " << alutGetErrorString(error) << llendl;
		} else {
			// It's common for the file to not actually exist.
			lldebugs <<
				"LLAudioBufferOpenAL::loadWAV() Error loading "
				 << filename
				 << " " << alutGetErrorString(error) << llendl;
		}
		return false;
	}

	return true;
}

U32 LLAudioBufferOpenAL::getLength(){
	if(mALBuffer == AL_NONE){
		return 0;
	}
	ALint length;
	alGetBufferi(mALBuffer, AL_SIZE, &length);
	return length >> 2;
}

// ------------

void LLAudioEngine_OpenAL::initWind(){
	ALenum error;
	llinfos << "LLAudioEngine_OpenAL::initWind() start" << llendl;

	alGetError(); /* clear error */
	
	alGenSources(1,&wind_source);
	
	if((error=alGetError()) != AL_NO_ERROR)
	{
		llwarns << "LLAudioEngine_OpenAL::initWind() Error creating wind sources: "<<error<<llendl;
	}
	
	winddata=(S16*)malloc(sizeof(S16)*llceil(wind_gen_freq*wind_buffer_size_sec*2*2)); //200ms @wind_gen_freqHz Stereo

	if(winddata==NULL)
	{
		llerrs << "LLAudioEngine_OpenAL::initWind() Error creating wind memory buffer" << llendl;
		mEnableWind=false;
	}

	mWindGen = new LLWindGen<S16>;

	llinfos << "LLAudioEngine_OpenAL::initWind() done" << llendl;
}

void LLAudioEngine_OpenAL::cleanupWind(){
	llinfos << "LLAudioEngine_OpenAL::cleanupWind()" << llendl;

	alDeleteSources(1, &wind_source);
	
	if(winddata)
		free(winddata);

	delete mWindGen;
	mWindGen = NULL;
}

void LLAudioEngine_OpenAL::updateWind(LLVector3 wind_vec, F32 camera_altitude)
{
	LLVector3 wind_pos;
	F64 pitch;
	F64 center_freq;
	ALenum error;
	
	mMaxWindGain=1.0;
	
	if (!mEnableWind)
		return;
	
	if(!winddata)
		return;
	
	if (mWindUpdateTimer.checkExpirationAndReset(LL_WIND_UPDATE_INTERVAL))
	{
		
		// wind comes in as Linden coordinate (+X = forward, +Y = left, +Z = up)
		// need to convert this to the conventional orientation DS3D and OpenAL use
		// where +X = right, +Y = up, +Z = backwards
		
		wind_vec.setVec(-wind_vec.mV[1], wind_vec.mV[2], -wind_vec.mV[0]);
		
		pitch = 1.0 + mapWindVecToPitch(wind_vec);
		center_freq = 80.0 * pow(pitch,2.5*(mapWindVecToGain(wind_vec)+1.0));
		
		mWindGen->mTargetFreq = (F32)center_freq;
		mWindGen->mTargetGain = (F32)mapWindVecToGain(wind_vec) * mMaxWindGain;
		mWindGen->mTargetPanGainR = (F32)mapWindVecToPan(wind_vec);
		
		alSourcei(wind_source, AL_LOOPING, AL_FALSE);
		alSource3f(wind_source, AL_POSITION, 0.0, 0.0, 0.0);
		alSource3f(wind_source, AL_VELOCITY, 0.0, 0.0, 0.0);
		alSourcef(wind_source, AL_ROLLOFF_FACTOR, 0.0);
		alSourcei(wind_source, AL_SOURCE_RELATIVE, AL_TRUE);
	}

	// ok lets make a wind buffer now

	int processed, queued, unprocessed;
	alGetSourcei(wind_source, AL_BUFFERS_PROCESSED, &processed);
	alGetSourcei(wind_source, AL_BUFFERS_QUEUED, &queued);
	unprocessed = queued - processed;

	// ensure that there are always at least 3x as many filled buffers
	// queued as we managed to empty since last time.
	empty_num_wind_buffers = llmin(empty_num_wind_buffers + processed * 3 - unprocessed, MAX_NUM_WIND_BUFFERS-unprocessed);
	empty_num_wind_buffers = llmax(empty_num_wind_buffers, 0);

	//llinfos << "empty_num_wind_buffers: " << empty_num_wind_buffers	<<" (" << unprocessed << ":" << processed << ")" << llendl;

	while(processed--) // unqueue old buffers
	{
		ALuint buffer;
		int error;
		alGetError(); /* clear error */
		alSourceUnqueueBuffers(wind_source, 1, &buffer);
		error = alGetError();
		if(error != AL_NO_ERROR)
		{
			llwarns << "LLAudioEngine_OpenAL::updateWind() error swapping (unqueuing) buffers" << llendl;
		}
		else
		{
			alDeleteBuffers(1, &buffer);
		}
	}

	while (empty_num_wind_buffers > 0) // fill+queue new buffers
	{
		ALuint buffer;
		alGetError(); /* clear error */
		alGenBuffers(1,&buffer);
		if((error=alGetError()) != AL_NO_ERROR)
		{
			llwarns << "LLAudioEngine_OpenAL::initWind() Error creating wind buffer: " << error << llendl;
			break;
		}

		alBufferData(buffer,
			     AL_FORMAT_STEREO16,
			     mWindGen->windGenerate(winddata,
				   int(wind_gen_freq*wind_buffer_size_sec), 2),
			     int(2*wind_gen_freq*wind_buffer_size_sec*sizeof(S16)),
			     wind_gen_freq);
		error = alGetError();
		if(error != AL_NO_ERROR)
			llwarns << "LLAudioEngine_OpenAL::updateWind() error swapping (bufferdata) buffers" << llendl;
		
		alSourceQueueBuffers(wind_source, 1, &buffer);
		error = alGetError();
		if(error != AL_NO_ERROR)
			llwarns << "LLAudioEngine_OpenAL::updateWind() error swapping (queuing) buffers" << llendl;

		--empty_num_wind_buffers;
	}

	int playing;
	alGetSourcei(wind_source, AL_SOURCE_STATE, &playing);
	if(playing != AL_PLAYING)
	{
		alSourcePlay(wind_source);

		llinfos << "Wind had stopped - probably ran out of buffers - restarting: " << (unprocessed+empty_num_wind_buffers) << " now queued." << llendl;
	}
}

