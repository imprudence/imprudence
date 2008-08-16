//--------------------------------------------------------------------
// Fake Voice Sound Signal
// author: JJ Ventrella
// THIS IS A TEMPORARY FILE FOR DEVELOPMENT PURPOSES ONLY
//-----------------------------------------------------------------------------
#ifndef FAKE_VOICE_SOUND_SIGNAL_H
#define FAKE_VOICE_SOUND_SIGNAL_H

//----------------------------------------------------------------------------
// FakeVoiceSoundSignal
//----------------------------------------------------------------------------
class FakeVoiceSoundSignal
{
	public:
		FakeVoiceSoundSignal();	
		~FakeVoiceSoundSignal();		
		void	update();
		F32		getAmplitude();
		bool	getActive();
		
};//-----------------------------------------------------------------
 //   end of class
//------------------------------------------------------------------

#endif //FAKE_VOICE_SOUND_SIGNAL_H
