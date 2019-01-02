#pragma once
#include <FMOD/fmod.hpp>
#include <vector>

typedef FMOD::System AudioSystem;
typedef FMOD::Sound Audio;
typedef FMOD::Channel AudioChannel;
typedef FMOD::ChannelGroup AudioChannelGroup;

typedef unsigned int uint;

class EmGineAudioPlayer
{
public:
	//MUST be called before using any other function 
	static void init(int channels = 36);

	/*
	disables the audio system, closing and clearing all open audio instances.
	NOTE:
	init must be called before any other calls can be made  
	*/
	static void disable();

	/*
	creates an audio instance that is stored in memory
	*/
	static void createAudio(const char* file);

	/*
	creates an audio instance that is read from disk (recomended for large audio files)
	*/
	static void createAudioStream(const char* file);

	/*
	plays a single audio channel creaded by createAudio/AudioStream() 
	*/
	static void play(bool loop = false, bool newInstance = false, uint index = (m_channels->size() - 1),
			  uint from = 0, uint to = 0, FMOD_TIMEUNIT unit = FMOD_TIMEUNIT_MS);
	/*
	plays all existing audio channels creaded by createAudio/AudioStream()
	*/
	static void playAll(bool loop = false, uint from = 0, uint to = 0, FMOD_TIMEUNIT unit = FMOD_TIMEUNIT_MS);
	
	/*
	pauses an audio channel at specified index.
	Note:
	any stoped audio deleated by cleanup may change original audio index
	*/
	static void pause(uint index = (m_channels->size() - 1));

	/*
	pauses all audio channels
	*/
	static void pauseAll();


	static void stop(uint index = (m_channels->size() - 1));

	static void stopAll();

	static void mute(uint index = (m_channels->size() - 1));

	static void muteAll();
	
	static bool isStoped(uint index = (m_channels->size() - 1));

	static bool isPaused(uint index = (m_channels->size() - 1));
	
	static uint getPosition(uint index = (m_channels->size() - 1));

	static uint size();
	/*
	**normal volume levels from 0 -> 1.
	**below 0 will invert sound.
	**increasing level above the normal level may resault in distortion.
	*/
	static void setVolume(float vol, uint index = (m_channels->size() - 1));

	static void setMasterVolume(float vol);

	static AudioSystem* getAudioSystem();

	static AudioChannelGroup* getMasterChannelGroup();

	static std::vector<AudioChannel*>* getChannels();

	static std::vector<Audio*>* getAudio();

	//not needed but recomended in certin platforms within their update loop
	static void update();

private:

	static void cleanup();

	static FMOD_RESULT __stdcall cleanUpCallback(FMOD_CHANNELCONTROL *chanCtrl, FMOD_CHANNELCONTROL_TYPE ctrlType, FMOD_CHANNELCONTROL_CALLBACK_TYPE callbackType, void *commandData1, void *commandData2);

	static uint stopIndex;
	static AudioSystem* m_system;
	static AudioChannelGroup* m_mainChannelGroup;
	static std::vector<AudioChannel*>* m_channels;
	static std::vector<Audio*>* m_sounds;
};

