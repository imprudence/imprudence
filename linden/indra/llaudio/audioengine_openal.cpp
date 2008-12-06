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

LLAudioEngine_OpenAL::LLAudioEngine_OpenAL()
{

	#if LL_GSTREAMER_ENABLED
	mMedia_data = new LLMediaManagerData;
								 // initialize GStreamer
	LLMediaImplGStreamer::startup( mMedia_data );

	m_streamer=new LLMediaImplGStreamer ();

	if(!m_streamer)
	{
		llwarns << "LLAudioEngine_OpenAL::LLAudioEngine_OpenAL() Failed to create our private gstreamer audio instance" << llendl;
	}
	#endif
}

LLAudioEngine_OpenAL::~LLAudioEngine_OpenAL()
{
}


BOOL LLAudioEngine_OpenAL::init(const S32 num_channels)
{
	LLAudioEngine::init(num_channels);

	if(!alutInit(NULL, NULL))
	{
		llwarns << "LLAudioEngine_OpenAL::init() ALUT initialization failed: " << alutGetErrorString (alutGetError ()) << llendl;
		return false;
	}

	initInternetStream();

	llinfos << "LLAudioEngine_OpenAL::init() OpenAL successfully initialized" << llendl;

	llinfos << "LLAudioEngine_OpenAL::init() Speed of sound is: " << alGetFloat(AL_SPEED_OF_SOUND) << llendl;

	return TRUE;
}

std::string LLAudioEngine_OpenAL::getDriverName(bool verbose)
{
	ALCdevice *device = alcGetContextsDevice(alcGetCurrentContext());
	
	std::ostringstream version;

	ALint major = alutGetMajorVersion ();
	ALint minor = alutGetMinorVersion ();

	alcGetIntegerv(device, ALC_MAJOR_VERSION, 1, &major);
	alcGetIntegerv(device, ALC_MAJOR_VERSION, 1, &minor);

	version <<
		"OpenAL";

	if (verbose)
	{
		version <<
			", version " <<
			ll_safe_string(alGetString(AL_VERSION)) <<
			" / " <<
			ll_safe_string(alGetString(AL_VENDOR)) <<
			" / " <<
			ll_safe_string(alGetString(AL_RENDERER));
		
		if (device)
			version <<
				": " <<
				ll_safe_string(alcGetString(device,
				    ALC_DEFAULT_DEVICE_SPECIFIER));
	}

	return version.str();
}


void LLAudioEngine_OpenAL::idle(F32 max_decode_time)
{
	LLAudioEngine::idle(max_decode_time);
	#if LL_GSTREAMER_ENABLED
	if(m_streamer != NULL)
		m_streamer->updateMedia();
	#endif
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
	else
	{
		llinfos << "LLAudioEngine_OpenAL::shutdown() OpenAL successfully shut down" << llendl;
	}
	
	delete mListenerp;
	mListenerp = NULL;

	#if LL_GSTREAMER_ENABLED
	if(m_streamer)
	{
		delete m_streamer;
		m_streamer = NULL;
	}
	#endif
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
	alGenSources(1, &ALSource);
}

LLAudioChannelOpenAL::~LLAudioChannelOpenAL()
{
	cleanup();
	alDeleteSources(1, &ALSource);
}

void LLAudioChannelOpenAL::cleanup()
{
	alSourceStop(ALSource);
	mCurrentBufferp = NULL;
}

void LLAudioChannelOpenAL::play()
{
	if(!isPlaying())
	{
		alSourcePlay(ALSource);
		getSource()->setPlayedOnce(TRUE);
	}
}

void LLAudioChannelOpenAL::playSynced(LLAudioChannel *channelp)
{
	play();
}

bool LLAudioChannelOpenAL::isPlaying()
{
	ALint state;
	alGetSourcei(ALSource, AL_SOURCE_STATE, &state);
	if(state == AL_PLAYING)
	{
		return TRUE;
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
		alSourcei(ALSource, AL_BUFFER, bufferp->getBuffer());
		alSourcef(ALSource, AL_GAIN, mCurrentSourcep->getGain());
		alSourcei(ALSource, AL_LOOPING, mCurrentSourcep->isLoop() ? AL_TRUE : AL_FALSE);
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
		alSource3f(ALSource, AL_POSITION, 0.0, 0.0, 0.0);
		alSource3f(ALSource, AL_VELOCITY, 0.0, 0.0, 0.0);
		//alSource3f(ALSource, AL_DIRECTION, 0.0, 0.0, 0.0);
		alSourcef (ALSource, AL_ROLLOFF_FACTOR, 0.0);
		alSourcei (ALSource, AL_SOURCE_RELATIVE, AL_TRUE);
	}
	else
	{
		LLVector3 float_pos;
		float_pos.setVec(mCurrentSourcep->getPositionGlobal());
		alSourcefv(ALSource, AL_POSITION, float_pos.mV);
		//llinfos << "LLAudioChannelOpenAL::update3DPosition() Velocity: " << mCurrentSourcep->getVelocity() << llendl;
		alSourcefv(ALSource, AL_VELOCITY, mCurrentSourcep->getVelocity().mV);
		//alSource3f(ALSource, AL_DIRECTION, 0.0, 0.0, 0.0);
		alSourcef (ALSource, AL_ROLLOFF_FACTOR, 1.0);
		alSourcei (ALSource, AL_SOURCE_RELATIVE, AL_FALSE);
	}
	//llinfos << "LLAudioChannelOpenAL::update3DPosition() Gain: " << mCurrentSourcep->getGain() << llendl;
	alSourcef(ALSource, AL_GAIN, mCurrentSourcep->getGain());
}

LLAudioBufferOpenAL::LLAudioBufferOpenAL()
{
	ALBuffer = AL_NONE;
}

LLAudioBufferOpenAL::~LLAudioBufferOpenAL()
{
	cleanup();
}

void LLAudioBufferOpenAL::cleanup()
{
	if(ALBuffer != AL_NONE)
	{
		alDeleteBuffers(1, &ALBuffer);
	}
}

bool LLAudioBufferOpenAL::loadWAV(const std::string& filename)
{
	cleanup();
	ALBuffer = alutCreateBufferFromFile(filename.c_str());
	if(ALBuffer == AL_NONE)
	{
		return FALSE;
	}

	return true;
}


U32 LLAudioBufferOpenAL::getLength()
{
	if(ALBuffer == AL_NONE)
	{
		return 0;
	}
	ALint length;
	alGetBufferi(ALBuffer, AL_SIZE, &length);
	return length >> 2;
}

// ------------

void LLAudioEngine_OpenAL::initWind()
{

	if (true)
		return;

	llinfos << "initWind() start" << llendl;

	alGenBuffers(mNumWindBuffers,mWindBuffers);
	alGenSources(1,&mWindSource);
	checkALError();

	// ok lets make a wind buffer now
	for(int counter=0;counter<mNumWindBuffers;counter++)
	{

		alBufferData(mWindBuffers[counter],AL_FORMAT_STEREO16,windDSP((void*)mWindData,mWindDataSize/mBytesPerSample),mWindDataSize,mSampleRate);
		checkALError();
	}

	alSourceQueueBuffers(mWindSource, mNumWindBuffers, mWindBuffers);
	checkALError();

	alSourcePlay(mWindSource);
	checkALError();

	llinfos << "LLAudioEngine_OpenAL::initWind() done" << llendl;

}

void LLAudioEngine_OpenAL::cleanupWind(){
	llinfos << "LLAudioEngine_OpenAL::cleanupWind()" << llendl;

	alDeleteBuffers(mNumWindBuffers,mWindBuffers);
	alDeleteSources(1, &mWindSource);

	checkALError();
}

void LLAudioEngine_OpenAL::checkALError()
{
	ALenum error;
	if((error=alGetError()) != AL_NO_ERROR)
		llwarns << "LLAudioEngine_OpenAL Error: "<<error<<llendl;

}

void LLAudioEngine_OpenAL::updateWind(LLVector3 wind_vec, F32 camera_altitude)
{

	if (true)
		return;

	LLVector3 wind_pos;
	F32 pitch;
	F32 center_freq;

	if (!mEnableWind)
		return;

	if(!mWindData)
		return;
	
	if (mWindUpdateTimer.checkExpirationAndReset(LL_WIND_UPDATE_INTERVAL))
	{
		
		// wind comes in as Linden coordinate (+X = forward, +Y = left, +Z = up)
		// need to convert this to the conventional orientation DS3D and OpenAL use
		// where +X = right, +Y = up, +Z = backwards
		
		wind_vec.setVec(-wind_vec.mV[1], wind_vec.mV[2], -wind_vec.mV[0]);


		pitch = 1.0f + mapWindVecToPitch(wind_vec);
		center_freq = 80.0f * powf(pitch,2.5f*(mapWindVecToGain(wind_vec)+1.0f));

		//TESTING
		mMaxWindGain=1.0;

		mTargetFreq = center_freq;
		mTargetGain = (F32)mapWindVecToGain(wind_vec) * mMaxWindGain;
		mTargetPanGainR = (F32)mapWindVecToPan(wind_vec);

		ALfloat source0Pos[]={mListenerp->getPosition().mV[0],mListenerp->getPosition().mV[1],mListenerp->getPosition().mV[2]};
		ALfloat source0Vel[]={ 0.0, 0.0, 0.0};

		alSourcef(mWindSource, AL_GAIN, mTargetGain);
		alSourcef(mWindSource, AL_PITCH, pitch);
		alSourcefv(mWindSource, AL_POSITION, source0Pos);
		alSourcefv(mWindSource, AL_VELOCITY, source0Vel);
		alSourcei(mWindSource, AL_LOOPING, AL_FALSE);

	}

	int processed;
	alGetSourcei(mWindSource, AL_BUFFERS_PROCESSED, &processed);

	while(processed--)
	{
		ALuint buffer;
		alSourceUnqueueBuffers(mWindSource, 1, &buffer);
		checkALError();
		alBufferData(buffer,AL_FORMAT_STEREO16,windDSP((void*)mWindData,mWindDataSize/mBytesPerSample),mWindDataSize,mSampleRate);
		checkALError();
		alSourceQueueBuffers(mWindSource, 1, &buffer);
		checkALError();
	}

	int playing;
	alGetSourcei(mWindSource, AL_SOURCE_STATE, &playing);
	if(playing==AL_STOPPED)
		alSourcePlay(mWindSource);

	checkALError();
}


void * LLAudioEngine_OpenAL::windDSP(void *newbuffer, int length)
{
	// *NOTE: This function gets called a *lot*.  
	// Keep performance in mind if you mess with this.
	// newbuffer = the buffer being constructed
	// length = length in samples of the buffer


	//clear the buffer 
	memset(newbuffer, 0, length*mBytesPerSample);

 	// This turns off wind synth if it is muted or very very low volume
 	if (mTargetGain < 0.0005f)
 	{
		llinfos << "Wind off" << llendl;
 		return newbuffer;
 	}
 
 	static const U8  SUBSAMPLES = 2;
 	static const F32 FILTER_SAMPLE_PERIOD = (F32)SUBSAMPLES / float(mSampleRate);
 	static const F32 BANDWIDTH = 50.0f;
 	static const F32 B2 = expf(-F_TWO_PI * BANDWIDTH * FILTER_SAMPLE_PERIOD);
 
 	static F32 pinking_buf0 = 0.0f;
 	static F32 pinking_buf1 = 0.0f;
 	static F32 pinking_buf2 = 0.0f;
 	static F32 Y0 = 0.0f;
 	static F32 Y1 = 0.0f;
 	static F32 last_sample = 0.0f;
 	static F32 current_freq = 0.0f;
 	static F32 current_gain = 0.0f;
 	static F32 current_pan_gain_r = 0.0f;
 
  	F32 a0 = 0.0f, b1 = 0.0f;
  
  	U8 *cursamplep = (U8*)newbuffer;

	//we assume 16-bit samples, because the ALUT specification maxes out there
 	U8 wordsize = 2;
  
 	bool interp_freq = false; 
 
 	//if the frequency isn't changing much, we don't need to interpolate in the inner loop
 	if (llabs(mTargetFreq - current_freq) > 200.0f)
 	{
 		interp_freq = true;
 	}
 	else
 	{
 		// calculate resonant filter coefficients
 		current_freq = mTargetFreq;
 		b1 = (-4.0f * B2) / (1.0f + B2) * cosf(F_TWO_PI * (current_freq * FILTER_SAMPLE_PERIOD));
 		a0 = (1.0f - B2) * sqrtf(1.0f - (b1 * b1) / (4.0f * B2));
 	}
 
 	while (length)
 	{
 		F32 next_sample;
  
 	    // Start with white noise [-16384, 16383]
 		next_sample = (F32)rand() * (1.0f / (F32)(RAND_MAX / (U16_MAX / 4))) + (S16_MIN / 4);
  									 
 		// Apply a pinking filter
 		// Magic numbers taken from PKE method at http://www.firstpr.com.au/dsp/pink-noise/
 		pinking_buf0 = pinking_buf0 * 0.99765f + next_sample * 0.0990460f;
 		pinking_buf1 = pinking_buf1 * 0.96300f + next_sample * 0.2965164f;
 		pinking_buf2 = pinking_buf2 * 0.57000f + next_sample * 1.0526913f;
 		
 		next_sample = pinking_buf0 + pinking_buf1 + pinking_buf2 + next_sample * 0.1848f;
  
 		if (interp_freq)
 		{
 			// calculate resonant filter coefficients
 			current_freq = (0.999f * current_freq) + (0.001f * mTargetFreq);
 			b1 = (-4.0f * B2) / (1.0f + B2) * cosf(F_TWO_PI * (current_freq * FILTER_SAMPLE_PERIOD));
 			a0 = (1.0f - B2) * sqrtf(1.0f - (b1 * b1) / (4.0f * B2));
 		}
 
 		// Apply a resonant low-pass filter on the pink noise
		next_sample = ( a0 * next_sample - b1 * Y0 - B2 * Y1 );
  
		Y1 = Y0;
		Y0 = next_sample;
 
 		current_gain = (0.999f * current_gain) + (0.001f * mTargetGain);
 		current_pan_gain_r = (0.999f * current_pan_gain_r) + (0.001f * mTargetPanGainR);
 		
 	    next_sample *= current_gain;
 		F32 delta = (next_sample - last_sample) / (F32)SUBSAMPLES;
  		
 		S32	sample_left;
 		S32	sample_right;
 
 		// Mix into the audio buffer, clipping if necessary for 16-bit mix buffers.
 		// *TODO: Should do something more intelligent like reducing wind gain to avoid clipping
 		for (int i=SUBSAMPLES; i && length; --i, --length) 
 		{
 			last_sample = last_sample + delta;
 			sample_right = (S32)(last_sample * current_pan_gain_r);
 			sample_left = (S32)(last_sample - sample_right);
 
			*(S16*)cursamplep = llclamp(sample_left, S16_MIN, S16_MAX);
 			cursamplep += wordsize;
 
 			*(S16*)cursamplep = llclamp(sample_right, S16_MIN, S16_MAX);
 			cursamplep += wordsize;
 		}
 	}		  
  	return newbuffer;

}




/*




	// newbuffer = the buffer passed from the previous DSP unit.
	// length = length in samples at this mix time.

	U8 *cursamplep = (U8*)newbuffer;
	U8   wordsize = 2;

	double bandwidth = 50;
	double inputSamplingRate = 44100;
	double a0,b1,b2;

	// calculate resonant filter coeffs
	b2 = exp(-(F_TWO_PI) * (bandwidth / inputSamplingRate));

	while (length--)
	{
		gCurrentFreq = (float)((0.999 * gCurrentFreq) + (0.001 * gTargetFreq));
		gCurrentGain = (float)((0.999 * gCurrentGain) + (0.001 * gTargetGain));
		gCurrentPanGainR = (float)((0.999 * gCurrentPanGainR) + (0.001 * gTargetPanGainR));
		b1 = (-4.0 * b2) / (1.0 + b2) * cos(F_TWO_PI * (gCurrentFreq / inputSamplingRate));
		a0 = (1.0 - b2) * sqrt(1.0 - (b1 * b1) / (4.0 * b2));
		double nextSample;

		// start with white noise
		nextSample = ll_frand(2.0f) - 1.0f;

		gbuf0 = 0.997f * gbuf0 + 0.0126502f * nextSample;
		gbuf1 = 0.985f * gbuf1 + 0.0139083f * nextSample;
		gbuf2 = 0.950f * gbuf2 + 0.0205439f * nextSample;
		gbuf3 = 0.850f * gbuf3 + 0.0387225f * nextSample;
		gbuf4 = 0.620f * gbuf4 + 0.0465932f * nextSample;
		gbuf5 = 0.250f * gbuf5 + 0.1093477f * nextSample;

		nextSample = gbuf0 + gbuf1 + gbuf2 + gbuf3 + gbuf4 + gbuf5;

		nextSample = (double)( a0 * nextSample - b1 * gY0 - b2 * gY1 );

		gY1 = gY0;
		gY0 = nextSample;
		nextSample *= gCurrentGain;

		S16     sample;

		sample = llfloor(((F32)nextSample*32768.f*(1.0f - gCurrentPanGainR))+0.5f);
		*(S16*)cursamplep = clipSample(sample, -32768, 32767);

		cursamplep += wordsize;

		sample = llfloor(((F32)nextSample*32768.f*gCurrentPanGainR)+0.5f);

		sample = llfloor(((F32)nextSample*32768.f*gCurrentPanGainR)+0.5f);
		*(S16*)cursamplep = clipSample(sample, -32768, 32767);
		cursamplep += wordsize;
	}

	return newbuffer;
}
*/

// ------------

void LLAudioEngine_OpenAL::InitStreamer()
{
	#if LL_GSTREAMER_ENABLED
	m_streamer=new LLMediaImplGStreamer ();

	if(!m_streamer)
	{
		llwarns << "LLAudioEngine_OpenAL::LLAudioEngine_OpenAL() Failed to create our private gstreamer audio instance" << llendl;
	}

	if(m_streamer)
	{
		m_streamer->init ();
	}
	#endif
}


// ------------

void LLAudioEngine_OpenAL::initInternetStream()
{
	if(!mInternetStreamURL.empty())
		mInternetStreamURL.erase();
}


void LLAudioEngine_OpenAL::startInternetStream(const std::string& url_cstr)
{

	std::string url(url_cstr);

	#if LL_GSTREAMER_ENABLED
	if(!m_streamer)
		return;
	// DCF_DEBUG
	llinfos << "entered LLAudioEngine_OpenAL::startInternetStream()" << llendl;

	if (!url.empty())
	{
		llinfos << "LLAudioEngine_OpenAL::startInternetStream() Starting internet stream: " << url << llendl;
		mInternetStreamURL=url;
		m_streamer->navigateTo ( url );
		llinfos << "Playing....." << llendl;
		m_streamer->addCommand(LLMediaBase::COMMAND_START);
		m_streamer->updateMedia();

	}
	else
	{
		llinfos << "LLAudioEngine_OpenAL setting stream to NULL"<< llendl;
		mInternetStreamURL.erase();
		m_streamer->addCommand(LLMediaBase::COMMAND_STOP);
		m_streamer->updateMedia();
	}
	#endif
}


void LLAudioEngine_OpenAL::updateInternetStream()
{
	// DCF_DEBUG
	llinfos << "entered LLAudioEngine_OpenAL::updateInternetStream()" << llendl;

}


void LLAudioEngine_OpenAL::stopInternetStream()
{
	// DCF_DEBUG
	llinfos << "entered LLAudioEngine_OpenAL::stopInternetStream()" << llendl;

	#if LL_GSTREAMER_ENABLED
	if( ! m_streamer->addCommand(LLMediaBase::COMMAND_STOP))
	{
		llinfos << "attempting to stop stream failed!" << llendl;
	}
	m_streamer->updateMedia();
	#endif
	mInternetStreamURL.erase();
}


void LLAudioEngine_OpenAL::pauseInternetStream(int pause)
{
	#if LL_GSTREAMER_ENABLED
	if(!m_streamer)
		return;
	// DCF_DEBUG
	llinfos << "entered LLAudioEngine_OpenAL::pauseInternetStream()" << llendl;

	if(pause)
	{
		if(!m_streamer->addCommand(LLMediaBase::COMMAND_PAUSE))
		{
			llinfos << "attempting to pause stream failed!" << llendl;
		}
		m_streamer->updateMedia();
	}
	else
	{
		if( ! m_streamer->addCommand(LLMediaBase::COMMAND_START))
		{
			llinfos << "attempting to pause stream failed!" << llendl;
		}
		m_streamer->updateMedia();
	}
	#endif
}


int LLAudioEngine_OpenAL::isInternetStreamPlaying()
{

	#if LL_GSTREAMER_ENABLED
	if(!m_streamer)
		return 0;

	if(m_streamer->getStatus() == LLMediaBase::STATUS_STARTED)
	{
		return 1;				 // Active and playing
	}

	if(m_streamer->getStatus() == LLMediaBase::STATUS_PAUSED)
	{
		return 2;				 // paused
	}

	#endif
	return 0;					 // Stopped
}


void LLAudioEngine_OpenAL::getInternetStreamInfo(char* artist_out, char* title_out)
{
}


void LLAudioEngine_OpenAL::setInternetStreamGain(F32 vol)
{
	#if LL_GSTREAMER_ENABLED
	// Set the gstreamer volume here
	if(!m_streamer)
		return;

	vol = llclamp(vol, 0.f, 1.f);
	m_streamer->setVolume(vol);
	m_streamer->updateMedia();
	#endif
}


const std::string& LLAudioEngine_OpenAL::getInternetStreamURL()
{
	return mInternetStreamURL;
}
