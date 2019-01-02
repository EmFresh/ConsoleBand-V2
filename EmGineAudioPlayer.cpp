#include "EmGineAudioPlayer.h"
#include <string>
#include <Windows.h>

#pragma region Static Variables
uint EmGineAudioPlayer::stopIndex = 0;
AudioSystem *EmGineAudioPlayer::m_system;
AudioChannelGroup* EmGineAudioPlayer::m_mainChannelGroup;
std::vector<AudioChannel*>* EmGineAudioPlayer::m_channels;
std::vector<Audio*>* EmGineAudioPlayer::m_sounds;
#pragma endregion

void EmGineAudioPlayer::init(int channels)
{
	if(m_system)
		return;

	m_channels = new std::vector<AudioChannel*>;
	m_sounds = new std::vector<Audio*>;

	if(FMOD::System_Create(&m_system))
		return;

	int driverCount;
	if(m_system->getNumDrivers(&driverCount))
		return;

	m_system->init(channels, FMOD_INIT_NORMAL, nullptr);
}

void EmGineAudioPlayer::disable()
{
	m_system->release();
	m_system->close();
	m_sounds->clear();
	m_channels->clear();
}


void EmGineAudioPlayer::createAudio(const char * file)
{
	Audio* newSound;

	if(m_system->createSound(file, FMOD_CREATECOMPRESSEDSAMPLE | FMOD_ACCURATETIME, nullptr, &newSound))
	{
		OutputDebugStringA("failed to create Audio\n");
		return;
	}
	m_sounds->push_back(newSound);
	m_channels->push_back(nullptr);

	m_system->playSound(m_sounds[0][m_sounds->size() - 1], m_mainChannelGroup, false, &m_channels->back());
	m_channels->back()->setCallback(cleanUpCallback);
}

void EmGineAudioPlayer::createAudioStream(const char * file)
{
	Audio* newSound;
	if(m_system->createStream(file, FMOD_ACCURATETIME, nullptr, &newSound))
	{
		OutputDebugStringA("failed to create Audio Stream\n");
		return;
	}
	m_sounds->push_back(newSound);
	m_channels->push_back(nullptr);

	m_system->playSound(m_sounds[0][m_sounds->size() - 1], m_mainChannelGroup, false, &m_channels->back());
	m_channels->back()->setCallback(cleanUpCallback);
}

void EmGineAudioPlayer::play(bool loop, bool newInst, uint index, uint from, uint to, FMOD_TIMEUNIT unit)
{
	if(newInst && m_channels[0][index])
		m_sounds->push_back(m_sounds[0][index]),
		m_channels->push_back(nullptr),
		index = m_channels->size() - 1;


	if(!m_channels[0][index] || (m_channels[0][index] ? isStoped(index) : false))
		m_system->playSound(m_sounds[0][index], nullptr, true, &m_channels[0][index]);

	if(loop)
		m_channels[0][index]->setMode(FMOD_LOOP_NORMAL),
		m_channels[0][index]->setLoopCount(-1),
		from < to ? m_channels[0][index]->setLoopPoints(from, unit, to, unit) : NULL;
	else
		m_channels[0][index]->setMode(FMOD_LOOP_OFF);

	m_channels[0][index]->setPaused(false);
	//cleanup();
}

template<class T> T lerp(float t, T p1, T p2)
{
	return (1.f - t)*p1 + t * p2;
}

void EmGineAudioPlayer::playAll(bool loop, uint from, uint to, FMOD_TIMEUNIT unit)
{
	AudioChannelGroup* cg;
	m_system->getMasterChannelGroup(&cg);
	uint length;
	cg->setPaused(true);
	for(uint index = 0; index < m_channels->size(); index++)
		if(loop)
		{

			m_channels[0][index]->setPosition(from, unit);//fixes the issue for streemed audio


			m_channels[0][index]->setMode(FMOD_LOOP_NORMAL);
			m_sounds[0][index]->getLength(&length, unit);

			from < to && from >= 0 ? to < length ?
				m_channels[0][index]->setLoopPoints(from, unit, to, unit)/*true*/ :
				m_channels[0][index]->setLoopPoints(from, unit, length - 1, unit)/*false*/ : NULL/*else*/;

			m_channels[0][index]->setLoopCount(-1);

		}
		else
			m_channels[0][index]->setMode(FMOD_LOOP_OFF);
	OutputDebugStringA("\n\n");

	for(auto &a : m_channels[0])
		a->setPaused(false);
	cg->setPaused(false);
}

void EmGineAudioPlayer::pause(uint index)
{
	m_channels[0][index]->setPaused(true);
}

void EmGineAudioPlayer::pauseAll()
{
	AudioChannelGroup* cg;
	m_system->getMasterChannelGroup(&cg);
	cg->setPaused(true);
	for(auto &a : m_channels[0])
		a->setPaused(true);
}

void EmGineAudioPlayer::stop(uint index)
{
	stopIndex = index;
	m_channels[0][index]->stop();
}

void EmGineAudioPlayer::stopAll()
{
	AudioChannelGroup* cg;
	m_system->getMasterChannelGroup(&cg);
	bool paused;
	cg->getPaused(&paused);
	cg->setPaused(true);

	while(0 < m_channels->size())
	{
		stopIndex = 0;
		if(m_channels[0][0])
			m_channels[0][0]->stop();
		else
			m_channels->erase(m_channels->begin());
	}
	update();
	cg->setPaused(paused);
}

uint EmGineAudioPlayer::getPosition(uint index)
{
	uint pos;
	m_channels[0][index]->getPosition(&pos, FMOD_TIMEUNIT_MS);
	return pos;
}

bool EmGineAudioPlayer::isStoped(uint index)
{
	bool play;
	m_channels[0][index]->isPlaying(&play);
	return !play;
}

bool EmGineAudioPlayer::isPaused(uint index)
{
	bool pause;
	m_channels[0][index]->getPaused(&pause);
	return pause;
}

uint EmGineAudioPlayer::size()
{
	return m_channels->size();
}

void EmGineAudioPlayer::setVolume(float vol, uint index)
{
	m_channels[0][index]->setVolume(vol);
}

void EmGineAudioPlayer::setMasterVolume(float vol)
{
	AudioChannelGroup* cg;
	m_system->getMasterChannelGroup(&cg);
	cg->setVolume(vol);
}

AudioSystem * EmGineAudioPlayer::getAudioSystem()
{
	return m_system;
}

void EmGineAudioPlayer::mute(uint index)
{
	bool mute;
	m_channels[0][index]->getMute(&mute);
	m_channels[0][index]->setMute(!mute);
}

void EmGineAudioPlayer::muteAll()
{
	AudioChannelGroup* cg;
	m_system->getMasterChannelGroup(&cg);
	bool mute;
	cg->getMute(&mute);
	cg->setMute(!mute);
}

FMOD::ChannelGroup * EmGineAudioPlayer::getMasterChannelGroup()
{
	AudioChannelGroup* cg;
	m_system->getMasterChannelGroup(&cg);
	return cg;
}

std::vector<AudioChannel*>* EmGineAudioPlayer::getChannels()
{
	return m_channels;
}

std::vector<Audio*>* EmGineAudioPlayer::getAudio()
{
	return m_sounds;
}

void EmGineAudioPlayer::update()
{
	m_system->update();
}

void EmGineAudioPlayer::cleanup()
{
	bool play;

	for(unsigned a = 0; a < m_channels->size(); a++)
	{
		m_channels[0][a]->isPlaying(&play);
		if(!play)
		{
			m_channels->erase(m_channels->begin() + a);
			if(find(m_sounds->begin() + a + 1, m_sounds->end(), m_sounds[0][a]) == m_sounds->end())
				m_sounds[0][a]->release();
			m_sounds->erase(m_sounds->begin() + a),
				a--;
		}
	}
}

FMOD_RESULT F_CALLBACK EmGineAudioPlayer::cleanUpCallback(FMOD_CHANNELCONTROL * chanCtrl, FMOD_CHANNELCONTROL_TYPE ctrlType, FMOD_CHANNELCONTROL_CALLBACK_TYPE callbackType, void * commandData1, void * commandData2)
{
	callbackType, commandData1, commandData2;//unreferenced but needed


	if(ctrlType == FMOD_CHANNELCONTROL_CHANNEL)
	{// Channel specific functions here...
		AudioChannel *channel;
		channel = (AudioChannel *)chanCtrl;
		bool play;
		channel->isPlaying(&play);

		if(!play)
		{
			unsigned a = stopIndex;
			for(; (a = a > m_channels->size() - 1 ? m_channels->size() - 1 : a) >= 0; a--)
				if(m_channels[0][a] == channel)
				{
					m_channels->erase(m_channels->begin() + a);

					if(find(m_sounds->begin() + a + 1, m_sounds->end(), m_sounds[0][a]) == m_sounds->end())
						m_sounds[0][a]->release();

					m_sounds->erase(m_sounds->begin() + a);

					stopIndex = m_channels->size() - 1;
					break;
				}
		}
	}
	else
	{// ChannelGroup specific functions here...
		//	ChannelGroup *group = (ChannelGroup *) chanCtrl;
	}

	// ChannelControl generic functions here...
	return FMOD_OK;
}

// for later reference
FMOD_RESULT F_CALLBACK mycallback(FMOD_CHANNELCONTROL *chanCtrl, FMOD_CHANNELCONTROL_TYPE ctrlType, FMOD_CHANNELCONTROL_CALLBACK_TYPE callbackType, void *commandData1, void *commandData2)
{
	chanCtrl, callbackType, commandData1, commandData2;//unreferenced

	if(ctrlType == FMOD_CHANNELCONTROL_CHANNEL)
	{
		//AudioChannel *channel = (AudioChannel *)chanCtrl;
		/// Channel specific functions here...
	}
	else
	{
		//	ChannelGroup *group = (ChannelGroup *) chanCtrl;
			/// ChannelGroup specific functions here...
	}

	// ChannelControl generic functions here...
	return FMOD_OK;
}