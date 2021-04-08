#include "EmGineAudioPlayer.h"
//#include <iostream>
#include <string>
#include <Windows.h>

#pragma region Static Variables
uint EmGineAudioPlayer::stopIndex = 0;
AudioSystem* EmGineAudioPlayer::m_system;
AudioChannelGroup* EmGineAudioPlayer::m_mainChannelGroup;
std::vector<AudioControle*>* EmGineAudioPlayer::m_control;
#pragma endregion

void EmGineAudioPlayer::init(int channels)
{
	if(m_system)
		return;

	m_control = new std::vector<AudioControle*>;
	//m_controle = new std::vector<Audio*>;

	if(FMOD::System_Create(&m_system))
		return;

	int driverCount;
	if(m_system->getNumDrivers(&driverCount))
		return;

	printError(m_system->init(channels, FMOD_INIT_NORMAL, nullptr), __LINE__);
}

void EmGineAudioPlayer::disable()
{
	m_system->release();
	m_system->close();
	m_control->clear();
}

void EmGineAudioPlayer::createAudio(const char* file)
{
	Audio* newSound;

	if(m_system->createSound(file, FMOD_CREATECOMPRESSEDSAMPLE | FMOD_ACCURATETIME | FMOD_3D, nullptr, &newSound))
	{
		OutputDebugStringA("failed to create Audio\n");
		return;
	}
	m_control->push_back(new AudioControle{newSound,nullptr});
	//m_controle->push_back(nullptr);

	printError(m_system->playSound(m_control[0][m_control->size() - 1]->sound, m_mainChannelGroup, true, &m_control->back()->channel), __LINE__);
	//m_controle->back()->channel->setCallback(cleanUpCallback);
}

void EmGineAudioPlayer::createAudioStream(const char* file)
{
	Audio* newSound;
	if(m_system->createStream(file, FMOD_ACCURATETIME | FMOD_3D, nullptr, &newSound))
	{
		OutputDebugStringA("failed to create Audio Stream\n");
		return;
	}

	m_control->push_back(new AudioControle{newSound,nullptr});
	printError(m_system->playSound(m_control[0][m_control->size() - 1]->sound, m_mainChannelGroup, true, &m_control->back()->channel), __LINE__);
	//m_controle->back()->channel->setCallback(cleanUpCallback);
}

void EmGineAudioPlayer::play(bool loop, bool newInst, uint index, uint from, uint to, FMOD_TIMEUNIT unit)
{
	if(newInst && m_control[0][index])
		m_control->push_back(m_control[0][index]),
		m_control->push_back(nullptr),
		index = (uint)m_control->size() - 1;



	if(!m_control[0][index] || (m_control[0][index] ? isStoped(index) : false))
		printError(m_system->playSound(m_control[0][index]->sound, nullptr, true, &m_control[0][index]->channel), __LINE__);

	if(loop)
		printError(m_control[0][index]->channel->setMode(FMOD_LOOP_NORMAL), __LINE__),
		printError(m_control[0][index]->channel->setLoopCount(-1), __LINE__),
		from < to ? printError(m_control[0][index]->channel->setLoopPoints(from, unit, to, unit), __LINE__) :
		void();
	else
		printError(m_control[0][index]->channel->setMode(FMOD_LOOP_OFF), __LINE__);


	printError(m_control[0][index]->channel->setPaused(false), __LINE__);
	cleanup();
}

template<class T> T lerp(float t, T p1, T p2)
{
	return (1.f - t) * p1 + t * p2;
}

void EmGineAudioPlayer::playAll(bool loop, uint from, uint to, FMOD_TIMEUNIT unit)
{
	AudioChannelGroup* cg;
	printError(m_system->getMasterChannelGroup(&cg), __LINE__);
	uint length;
	printError(cg->setPaused(true), __LINE__);
	for(uint index = 0; index < m_control->size(); index++)
		if(loop)
		{

			printError(m_control[0][index]->channel->setPosition(from, unit), __LINE__);//fixes the issue for streamed audio


			printError(m_control[0][index]->channel->setMode(FMOD_LOOP_NORMAL), __LINE__);
			printError(m_control[0][index]->sound->getLength(&length, unit), __LINE__);

			from < to&& from >= 0 ? to < length ?
				printError(m_control[0][index]->channel->setLoopPoints(from, unit, to, unit), __LINE__)/*true*/ :
				printError(m_control[0][index]->channel->setLoopPoints(from, unit, length - 1, unit), __LINE__)/*false*/ :
				void()/*else*/;

			printError(m_control[0][index]->channel->setLoopCount(-1), __LINE__);

		}
		else
			printError(m_control[0][index]->channel->setMode(FMOD_LOOP_OFF), __LINE__);
	OutputDebugStringA("\n\n");

	cleanup();

	for(auto& a : *m_control)
		printError(a->channel->setPaused(false), __LINE__);
	printError(cg->setPaused(false), __LINE__);
}

void EmGineAudioPlayer::pause(uint index)
{
	printError(m_control[0][index]->channel->setPaused(true), __LINE__);
}

void EmGineAudioPlayer::pauseAll()
{
	AudioChannelGroup* cg;
	printError(m_system->getMasterChannelGroup(&cg), __LINE__);
	printError(cg->setPaused(true), __LINE__);
	for(auto& a : *m_control)
		printError(a->channel->setPaused(true), __LINE__);
}

void EmGineAudioPlayer::stop(uint index)
{
	//stopIndex = index;
	printError(m_control[0][index]->channel->stop(), __LINE__);
	cleanup();
}

void EmGineAudioPlayer::stopAll()
{
	AudioChannelGroup* cg;
	printError(m_system->getMasterChannelGroup(&cg), __LINE__);
	bool paused;
	printError(cg->getPaused(&paused), __LINE__);
	printError(cg->setPaused(true), __LINE__);


	stopIndex = 0;
	for(uint a = 0; a < m_control->size(); a++)
		printError(m_control[0][a]->channel->stop(), __LINE__);


	cleanup();

	cg->setPaused(paused);
}

uint EmGineAudioPlayer::getPosition(uint index, FMOD_TIMEUNIT type)
{
	uint pos;
	printError(m_control[0][index]->channel->getPosition(&pos, type), __LINE__);
	return pos;
}

bool EmGineAudioPlayer::isStoped(uint index)
{
	bool play;
	printError(m_control[0][index]->channel->isPlaying(&play), __LINE__);
	return !play;
}

bool EmGineAudioPlayer::isPaused(uint index)
{
	bool pause;
	printError(m_control[0][index]->channel->getPaused(&pause), __LINE__);
	return pause;
}

uint EmGineAudioPlayer::size()
{
	return (uint)m_control->size();
}

void EmGineAudioPlayer::setVolume(float vol, uint index)
{
	printError(m_control[0][index]->channel->setVolume(vol), __LINE__);
}

void EmGineAudioPlayer::setMasterVolume(float vol)
{
	AudioChannelGroup* cg;
	printError(m_system->getMasterChannelGroup(&cg), __LINE__);
	printError(cg->setVolume(vol), __LINE__);
}

AudioSystem* EmGineAudioPlayer::getAudioSystem()
{
	return m_system;
}

void EmGineAudioPlayer::mute(uint index)
{
	bool mute;
	printError(m_control[0][index]->channel->getMute(&mute), __LINE__);
	printError(m_control[0][index]->channel->setMute(!mute), __LINE__);
}

void EmGineAudioPlayer::muteAll()
{
	AudioChannelGroup* cg;
	printError(m_system->getMasterChannelGroup(&cg), __LINE__);
	bool mute;
	cg->getMute(&mute);
	cg->setMute(!mute);
}

FMOD::ChannelGroup* EmGineAudioPlayer::getMasterChannelGroup()
{
	AudioChannelGroup* cg;
	printError(m_system->getMasterChannelGroup(&cg), __LINE__);
	return cg;
}

std::vector<AudioControle*>* EmGineAudioPlayer::getAudioControl()
{
	return m_control;
}

void EmGineAudioPlayer::update()
{
	printError(m_system->update(), __LINE__);
}

void EmGineAudioPlayer::cleanup()
{
	bool play;

	for(unsigned a = 0; a < m_control->size(); a++)
	{
		AudioChannel* channel = m_control[0][a]->channel;
		channel->isPlaying(&play);
		if(!play)
		{
			if(find(m_control->begin() + a + 1, m_control->end(), m_control[0][a]) == m_control->end())
				printError(m_control[0][a]->sound->release(), __LINE__);

			m_control->erase(m_control->begin() + a);
			a--;
		}
	}
}

#include <FMOD/fmod_errors.h>
void EmGineAudioPlayer::printError(FMOD_RESULT error, int line)
{

	if(error)
	{
	#if _DEBUG
		std::string str(FMOD_ErrorString(error));
		OutputDebugStringA(("Error:\n" + str + "\nline " + std::to_string(line) + "\n\n").c_str());
	#endif
	}
}

FMOD_RESULT F_CALLBACK EmGineAudioPlayer::cleanUpCallback(FMOD_CHANNELCONTROL* chanCtrl, FMOD_CHANNELCONTROL_TYPE ctrlType, FMOD_CHANNELCONTROL_CALLBACK_TYPE callbackType, void* commandData1, void* commandData2)
{
	callbackType, commandData1, commandData2;//referenced but not quite needed


	if(ctrlType == FMOD_CHANNELCONTROL_CHANNEL)
	{
		// Channel specific functions here...
		AudioChannel* channel;
		channel = (AudioChannel*)chanCtrl;
		bool play;
		channel->isPlaying(&play);

		if(!play)
		{
			for(int a = stopIndex; a < (int)m_control->size(); a++)
				if(m_control[0][a]->channel == channel)
				{
					if(find(m_control->begin() + a + 1, m_control->end(), m_control[0][a]) == m_control->end())
						m_control[0][a]->sound->release();
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
FMOD_RESULT F_CALLBACK mycallback(FMOD_CHANNELCONTROL* chanCtrl, FMOD_CHANNELCONTROL_TYPE ctrlType, FMOD_CHANNELCONTROL_CALLBACK_TYPE callbackType, void* commandData1, void* commandData2)
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