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
	EmGineAudioPlayer(int channels = 36);
	~EmGineAudioPlayer();

	static void init(int channels = 36);

	void createAudio(const char* file);

	void createAudioStream(const char* file);

	void play(bool loop = false, bool newInstance = false, uint index = (m_channels->size() - 1),
			  uint from = 0, uint to = 0, FMOD_TIMEUNIT unit = FMOD_TIMEUNIT_MS);

	void playAll(bool loop = false, uint from = 0, uint to = 0, FMOD_TIMEUNIT unit = FMOD_TIMEUNIT_MS);

	void pause(uint index = (m_channels->size() - 1));

	void pauseAll();

	void stop(uint index = (m_channels->size() - 1));

	void stopAll();

	void mute(uint index = (m_channels->size() - 1));

	void muteAll();

	uint getPosition(uint index = (m_channels->size() - 1));

	bool isStoped(uint index = (m_channels->size() - 1));

	bool isPaused(uint index = (m_channels->size() - 1));

	uint size();
	/*
	**normal volume levels from 0 -> 1.
	**below 0 will invert sound.
	**increasing level above the normal level may resault in distortion.
	*/
	void setVolume(float vol, uint index = (m_channels->size() - 1));

	void setMasterVolume(float vol);

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

