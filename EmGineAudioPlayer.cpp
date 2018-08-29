#include "EmGineAudioPlayer.h"

using namespace std;
using namespace FMOD;

#pragma region Static Variables
AudioSystem *EmGineAudioPlayer::m_system;
ChannelGroup* EmGineAudioPlayer::m_mainChannelGroup;
vector<Channel*>* EmGineAudioPlayer::m_channels;
vector<Audio*>* EmGineAudioPlayer::m_sounds;
#pragma endregion

EmGineAudioPlayer::EmGineAudioPlayer(int channels)
{
	init(channels);
}


EmGineAudioPlayer::~EmGineAudioPlayer()
{
	m_system->close();
	m_system->release();
	m_sounds->clear();
	m_channels->clear();
}

void EmGineAudioPlayer::init(int channels)
{
	if(m_system)
		return;

	m_channels = new vector<Channel*>;
	m_sounds = new vector<Audio*>;

	if(FMOD::System_Create(&m_system))
		return;
	int driverCount;
	if(m_system->getNumDrivers(&driverCount))
		return;

	m_system->init(channels, FMOD_INIT_NORMAL, nullptr);
}

void EmGineAudioPlayer::createSound(const char * file)
{
	cleanup();
	Audio* newSound;

	if(m_system->createSound(file, FMOD_DEFAULT | FMOD_ACCURATETIME, nullptr, &newSound))
		printf("failed to create sound");

	m_sounds->push_back(newSound);
	m_channels->push_back(nullptr);
}

void EmGineAudioPlayer::createStream(const char * file)
{
	cleanup();
	Audio* newSound;
	if(m_system->createStream(file, FMOD_DEFAULT | FMOD_ACCURATETIME, nullptr, &newSound))
		printf("failed to create sound");

	m_sounds->push_back(newSound);
	m_channels->push_back(nullptr);

	m_system->playSound(m_sounds[0][m_sounds->size() - 1], m_mainChannelGroup, false, &m_channels[0][m_channels->size() - 1]);
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
	cleanup();
}

void EmGineAudioPlayer::playAll(bool loop, uint from, uint to, FMOD_TIMEUNIT unit)
{
	ChannelGroup* cg;
	m_system->getMasterChannelGroup(&cg);
	uint length;
	for(uint index = 0; index < m_channels->size(); index++)
		if(loop)
		{
			m_channels[0][index]->setPosition(from, unit);//fixes the issue for streemed audio
			m_channels[0][index]->setMode(FMOD_LOOP_NORMAL);
			m_sounds[0][index]->getLength(&length, unit);

			//m_channels[0][index]->setPosition(from, unit);//fixes the issue for streemed audio
			from < to && from >= 0 ? to < length ?
				m_channels[0][index]->setLoopPoints(from, unit, to, unit) :
				m_channels[0][index]->setLoopPoints(from, unit, length - 1, unit) : NULL;

			//m_channels[0][index]->setPosition(from, unit);//fixes the issue for streemed audio
			m_channels[0][index]->setLoopCount(-1);
		} else
			m_channels[0][index]->setMode(FMOD_LOOP_OFF);

		cg->setPaused(false);
		cleanup();
}

void EmGineAudioPlayer::pause(uint index)
{
	m_channels[0][index]->setPaused(true);
	cleanup();
}

void EmGineAudioPlayer::pauseAll()
{
	ChannelGroup* cg;
	m_system->getMasterChannelGroup(&cg);
	cg->setPaused(true);
	cleanup();
}

void EmGineAudioPlayer::stop(uint index)
{
	uint length;

	Audio* cs;
	m_channels[0][index]->getCurrentSound(&cs);
	cs->getLength(&length, FMOD_TIMEUNIT_MS);
	printf("song length: %d\n", length);

	m_channels[0][index]->stop();
	cleanup();
}

void EmGineAudioPlayer::stopAll()
{
	ChannelGroup* cg;
	m_system->getMasterChannelGroup(&cg);
	cg->stop();
	cleanup();
}

uint EmGineAudioPlayer::getPosition(uint index)
{
	uint pos;
	m_channels[0][index]->getPosition(&pos, FMOD_TIMEUNIT_MS);
	return pos;
	cleanup();
}

bool EmGineAudioPlayer::isStoped(uint index)
{
	bool play;
	m_channels[0][index]->isPlaying(&play);
	return !play;
	cleanup();
}

bool EmGineAudioPlayer::isPaused(uint index)
{
	cleanup();
	bool pause;
	m_channels[0][index]->getPaused(&pause);
	return pause;
}

uint EmGineAudioPlayer::size()
{
	cleanup();
	return m_channels->size();
}

void EmGineAudioPlayer::setVolume(float vol, uint index)
{
	m_channels[0][index]->setVolume(vol);
}

void EmGineAudioPlayer::setMasterVolume(float vol)
{
	ChannelGroup* cg;
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
	cleanup();
}

void EmGineAudioPlayer::muteAll()
{
	ChannelGroup* cg;
	m_system->getMasterChannelGroup(&cg);
	bool mute;
	cg->getMute(&mute);
	cg->setMute(!mute);
	cleanup();
}

FMOD::ChannelGroup * EmGineAudioPlayer::getMasterChannelGroup()
{
	ChannelGroup* cg;
	m_system->getMasterChannelGroup(&cg);
	return cg;
}

std::vector<FMOD::Channel*>* EmGineAudioPlayer::getChannels()
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

	for(int a = 0; a < m_channels->size(); a++)
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







// for later reference
FMOD_RESULT mycallback(FMOD_CHANNELCONTROL *chanCtrl, FMOD_CHANNELCONTROL_TYPE ctrlType, FMOD_CHANNELCONTROL_CALLBACK_TYPE callbackType, void *commandData1, void *commandData2)
{
	if(ctrlType == FMOD_CHANNELCONTROL_CHANNEL)
	{
		Channel *channel = (Channel *) chanCtrl;
		// Channel specific functions here...
	} else
	{
		//	ChannelGroup *group = (ChannelGroup *) chanCtrl;
			// ChannelGroup specific functions here...
	}

	// ChannelControl generic functions here...
	return FMOD_OK;
}
