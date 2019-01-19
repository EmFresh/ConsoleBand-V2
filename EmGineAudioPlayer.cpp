#include "EmGineAudioPlayer.h"
//#include <iostream>
#include <string>
#include <Windows.h>

#pragma region Static Variables
uint EmGineAudioPlayer::stopIndex = 0;
AudioSystem *EmGineAudioPlayer::m_system;
AudioChannelGroup* EmGineAudioPlayer::m_mainChannelGroup;
std::vector<AudioControle*>* EmGineAudioPlayer::m_controle;
#pragma endregion

void EmGineAudioPlayer::init(int channels)
{
	if(m_system)
		return;

	m_controle = new std::vector<AudioControle*>;
	//m_controle = new std::vector<Audio*>;

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
	m_controle->clear();
}

void EmGineAudioPlayer::createAudio(const char * file)
{
	Audio* newSound;

	if(m_system->createSound(file, FMOD_CREATECOMPRESSEDSAMPLE | FMOD_ACCURATETIME, nullptr, &newSound))
	{
		OutputDebugStringA("failed to create Audio\n");
		return;
	}
	m_controle->push_back(new AudioControle{newSound,nullptr});
	//m_controle->push_back(nullptr);

	m_system->playSound(m_controle[0][m_controle->size() - 1]->sound, m_mainChannelGroup, false, &m_controle->back()->channel);
	m_controle->back()->channel->setCallback(cleanUpCallback);
}

void EmGineAudioPlayer::createAudioStream(const char * file)
{
	Audio* newSound;
	if(m_system->createStream(file, FMOD_ACCURATETIME, nullptr, &newSound))
	{
		OutputDebugStringA("failed to create Audio Stream\n");
		return;
	}
	m_controle->push_back(new AudioControle{newSound,nullptr});

	m_system->playSound(m_controle[0][m_controle->size() - 1]->sound, m_mainChannelGroup, false, &m_controle->back()->channel);
	m_controle->back()->channel->setCallback(cleanUpCallback);
}

void EmGineAudioPlayer::play(bool loop, bool newInst, uint index, uint from, uint to, FMOD_TIMEUNIT unit)
{
	if(newInst && m_controle[0][index])
		m_controle->push_back(m_controle[0][index]),
		m_controle->push_back(nullptr),
		index = (uint)m_controle->size() - 1;



	if(!m_controle[0][index] || (m_controle[0][index] ? isStoped(index) : false))
		m_system->playSound(m_controle[0][index]->sound, nullptr, true, &m_controle[0][index]->channel);

	if(loop)
		m_controle[0][index]->channel->setMode(FMOD_LOOP_NORMAL),
		m_controle[0][index]->channel->setLoopCount(-1),
		from < to ? m_controle[0][index]->channel->setLoopPoints(from, unit, to, unit) : NULL;
	else
		m_controle[0][index]->channel->setMode(FMOD_LOOP_OFF);

	m_controle[0][index]->channel->setPaused(false);
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
	for(uint index = 0; index < m_controle->size(); index++)
		if(loop)
		{

			m_controle[0][index]->channel->setPosition(from, unit);//fixes the issue for streamed audio


			m_controle[0][index]->channel->setMode(FMOD_LOOP_NORMAL);
			m_controle[0][index]->sound->getLength(&length, unit);

			from < to && from >= 0 ? to < length ?
				m_controle[0][index]->channel->setLoopPoints(from, unit, to, unit)/*true*/ :
				m_controle[0][index]->channel->setLoopPoints(from, unit, length - 1, unit)/*false*/ : NULL/*else*/;

			m_controle[0][index]->channel->setLoopCount(-1);

		}
		else
			m_controle[0][index]->channel->setMode(FMOD_LOOP_OFF);
	OutputDebugStringA("\n\n");

	for(auto &a : *m_controle)
		a->channel->setPaused(false);
	cg->setPaused(false);
}

void EmGineAudioPlayer::pause(uint index)
{
	m_controle[0][index]->channel->setPaused(true);
}

void EmGineAudioPlayer::pauseAll()
{
	AudioChannelGroup* cg;
	m_system->getMasterChannelGroup(&cg);
	cg->setPaused(true);
	for(auto &a : *m_controle)
		a->channel->setPaused(true);
}

void EmGineAudioPlayer::stop(uint index)
{
	stopIndex = index;
	m_controle[0][index]->channel->stop();
}

void EmGineAudioPlayer::stopAll()
{
	AudioChannelGroup* cg;
	m_system->getMasterChannelGroup(&cg);
	bool paused;
	cg->getPaused(&paused);
	cg->setPaused(true);


	stopIndex = 0;
	update();
	while(0 < m_controle->size())
	{
		m_controle[0][0]->channel->stop();
	}

	cg->setPaused(paused);
}

uint EmGineAudioPlayer::getPosition(uint index, FMOD_TIMEUNIT type)
{
	uint pos;
	m_controle[0][index]->channel->getPosition(&pos, type);
	return pos;
}

bool EmGineAudioPlayer::isStoped(uint index)
{
	bool play;
	m_controle[0][index]->channel->isPlaying(&play);
	return !play;
}

bool EmGineAudioPlayer::isPaused(uint index)
{
	bool pause;
	m_controle[0][index]->channel->getPaused(&pause);
	return pause;
}

uint EmGineAudioPlayer::size()
{
	return (uint)m_controle->size();
}

void EmGineAudioPlayer::setVolume(float vol, uint index)
{
	m_controle[0][index]->channel->setVolume(vol);
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
	m_controle[0][index]->channel->getMute(&mute);
	m_controle[0][index]->channel->setMute(!mute);
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

std::vector<AudioControle*>* EmGineAudioPlayer::getAudioControle()
{
	return m_controle;
}

void EmGineAudioPlayer::update()
{
	m_system->update();
}

void EmGineAudioPlayer::cleanup()
{
	bool play;

	for(unsigned a = 0; a < m_controle->size(); a++)
	{
		AudioChannel *channel = m_controle[0][a]->channel;
		channel->isPlaying(&play);
		if(!play)
		{
			if(find(m_controle->begin() + a + 1, m_controle->end(), m_controle[0][a]) == m_controle->end())
				m_controle[0][a]->sound->release();

			m_controle->erase(m_controle->begin() + a);
			a--;
		}
	}
}

FMOD_RESULT F_CALLBACK EmGineAudioPlayer::cleanUpCallback(FMOD_CHANNELCONTROL * chanCtrl, FMOD_CHANNELCONTROL_TYPE ctrlType, FMOD_CHANNELCONTROL_CALLBACK_TYPE callbackType, void * commandData1, void * commandData2)
{
	callbackType, commandData1, commandData2;//referenced but not quite needed


	if(ctrlType == FMOD_CHANNELCONTROL_CHANNEL)
	{
		// Channel specific functions here...
		AudioChannel *channel;
		channel = (AudioChannel *)chanCtrl;
		bool play;
		channel->isPlaying(&play);

		if(!play)
		{
			for(int a = stopIndex; a < (int)m_controle->size(); a++)
				if(m_controle[0][a]->channel == channel)
				{
					if(find(m_controle->begin() + a + 1, m_controle->end(), m_controle[0][a]) == m_controle->end())
						m_controle[0][a]->sound->release();
					m_controle->erase(m_controle->begin() + a);

					break;
				}
			stopIndex = 0;
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