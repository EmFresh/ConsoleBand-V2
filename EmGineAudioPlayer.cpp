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

	printError(m_system->init(channels, FMOD_INIT_NORMAL, nullptr), "Line 28");
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

	if(m_system->createSound(file, FMOD_CREATECOMPRESSEDSAMPLE | FMOD_ACCURATETIME | FMOD_3D, nullptr, &newSound))
	{
		OutputDebugStringA("failed to create Audio\n");
		return;
	}
	m_controle->push_back(new AudioControle{newSound,nullptr});
	//m_controle->push_back(nullptr);

	printError(m_system->playSound(m_controle[0][m_controle->size() - 1]->sound, m_mainChannelGroup, false, &m_controle->back()->channel), "Line 50");
	//m_controle->back()->channel->setCallback(cleanUpCallback);
}

void EmGineAudioPlayer::createAudioStream(const char * file)
{
	Audio* newSound;
	if(m_system->createStream(file, FMOD_ACCURATETIME | FMOD_3D, nullptr, &newSound))
	{
		OutputDebugStringA("failed to create Audio Stream\n");
		return;
	}

	m_controle->push_back(new AudioControle{newSound,nullptr});
	printError(m_system->playSound(m_controle[0][m_controle->size() - 1]->sound, m_mainChannelGroup, false, &m_controle->back()->channel), "Line 64");
	//m_controle->back()->channel->setCallback(cleanUpCallback);
}

void EmGineAudioPlayer::play(bool loop, bool newInst, uint index, uint from, uint to, FMOD_TIMEUNIT unit)
{
	if(newInst && m_controle[0][index])
		m_controle->push_back(m_controle[0][index]),
		m_controle->push_back(nullptr),
		index = (uint)m_controle->size() - 1;



	if(!m_controle[0][index] || (m_controle[0][index] ? isStoped(index) : false))
		printError(m_system->playSound(m_controle[0][index]->sound, nullptr, true, &m_controle[0][index]->channel), "Line 78");

	if(loop)
		printError(m_controle[0][index]->channel->setMode(FMOD_LOOP_NORMAL), "Line 81"),
		printError(m_controle[0][index]->channel->setLoopCount(-1), "Line 82"),
		from < to ? printError(m_controle[0][index]->channel->setLoopPoints(from, unit, to, unit), "Line 83") :
		void();
	else
		printError(m_controle[0][index]->channel->setMode(FMOD_LOOP_OFF), "Line 86");


	printError(m_controle[0][index]->channel->setPaused(false), "Line 89");
	cleanup();
}

template<class T> T lerp(float t, T p1, T p2)
{
	return (1.f - t)*p1 + t * p2;
}

void EmGineAudioPlayer::playAll(bool loop, uint from, uint to, FMOD_TIMEUNIT unit)
{
	AudioChannelGroup* cg;
	printError(m_system->getMasterChannelGroup(&cg), "Line 101");
	uint length;
	printError(cg->setPaused(true), "Line 103");
	for(uint index = 0; index < m_controle->size(); index++)
		if(loop)
		{

			printError(m_controle[0][index]->channel->setPosition(from, unit), "Line 108");//fixes the issue for streamed audio


			printError(m_controle[0][index]->channel->setMode(FMOD_LOOP_NORMAL), "Line 111");
			printError(m_controle[0][index]->sound->getLength(&length, unit), "Line 112");

			from < to && from >= 0 ? to < length ?
				printError(m_controle[0][index]->channel->setLoopPoints(from, unit, to, unit), "Line 115")/*true*/ :
				printError(m_controle[0][index]->channel->setLoopPoints(from, unit, length - 1, unit), "Line 116")/*false*/ :
				void()/*else*/;

			printError(m_controle[0][index]->channel->setLoopCount(-1), "Line 119");

		}
		else
			printError(m_controle[0][index]->channel->setMode(FMOD_LOOP_OFF), "Line 123");
	OutputDebugStringA("\n\n");

	cleanup();

	for(auto &a : *m_controle)
		printError(a->channel->setPaused(false), "Line 129");
	printError(cg->setPaused(false), "Line 130");
}

void EmGineAudioPlayer::pause(uint index)
{
	printError(m_controle[0][index]->channel->setPaused(true));
}

void EmGineAudioPlayer::pauseAll()
{
	AudioChannelGroup* cg;
	printError(m_system->getMasterChannelGroup(&cg));
	printError(cg->setPaused(true));
	for(auto &a : *m_controle)
		printError(a->channel->setPaused(true));
}

void EmGineAudioPlayer::stop(uint index)
{
	//stopIndex = index;
	printError(m_controle[0][index]->channel->stop());
	cleanup();
}

void EmGineAudioPlayer::stopAll()
{
	AudioChannelGroup* cg;
	printError(m_system->getMasterChannelGroup(&cg));
	bool paused;
	printError(cg->getPaused(&paused));
	printError(cg->setPaused(true));


	stopIndex = 0;
	for(uint a = 0; a < m_controle->size(); a++)
		printError(m_controle[0][a]->channel->stop());


	cleanup();

	cg->setPaused(paused);
}

uint EmGineAudioPlayer::getPosition(uint index, FMOD_TIMEUNIT type)
{
	uint pos;
	printError(m_controle[0][index]->channel->getPosition(&pos, type));
	return pos;
}

bool EmGineAudioPlayer::isStoped(uint index)
{
	bool play;
	printError(m_controle[0][index]->channel->isPlaying(&play));
	return !play;
}

bool EmGineAudioPlayer::isPaused(uint index)
{
	bool pause;
	printError(m_controle[0][index]->channel->getPaused(&pause));
	return pause;
}

uint EmGineAudioPlayer::size()
{
	return (uint)m_controle->size();
}

void EmGineAudioPlayer::setVolume(float vol, uint index)
{
	printError(m_controle[0][index]->channel->setVolume(vol));
}

void EmGineAudioPlayer::setMasterVolume(float vol)
{
	AudioChannelGroup* cg;
	printError(m_system->getMasterChannelGroup(&cg));
	printError(cg->setVolume(vol));
}

AudioSystem * EmGineAudioPlayer::getAudioSystem()
{
	return m_system;
}

void EmGineAudioPlayer::mute(uint index)
{
	bool mute;
	printError(m_controle[0][index]->channel->getMute(&mute));
	printError(m_controle[0][index]->channel->setMute(!mute));
}

void EmGineAudioPlayer::muteAll()
{
	AudioChannelGroup* cg;
	printError(m_system->getMasterChannelGroup(&cg));
	bool mute;
	cg->getMute(&mute);
	cg->setMute(!mute);
}

FMOD::ChannelGroup * EmGineAudioPlayer::getMasterChannelGroup()
{
	AudioChannelGroup* cg;
	printError(m_system->getMasterChannelGroup(&cg));
	return cg;
}

std::vector<AudioControle*>* EmGineAudioPlayer::getAudioControle()
{
	return m_controle;
}

void EmGineAudioPlayer::update()
{
	printError(m_system->update());
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
				printError(m_controle[0][a]->sound->release());

			m_controle->erase(m_controle->begin() + a);
			a--;
		}
	}
}

void EmGineAudioPlayer::printError(FMOD_RESULT error, const char* where)
{

	if(error)
	{
#if _DEBUG
		std::string str(FMOD_ErrorString(error));
		OutputDebugStringA(("Error:\n" + str + '\n' + where + "\n\n").c_str());
#endif
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