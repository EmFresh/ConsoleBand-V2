#pragma once
#include <Windows.h>
#include <vector>
#include<string>

#pragma comment(lib,"winmm.lib")

/***Structs***/
struct AudioInfo
{
	bool playing;
	bool paused;
	bool stopped;
	unsigned int inst;
	const char* tag;
};

/***Classes***/
class AudioPlayer
{
public:
	/****Constructors & Destructor****/

	AudioPlayer();

	//see setAudio();
	AudioPlayer(std::string audio);

	void setInst(int);

	~AudioPlayer();

	/****Member Functions****/

	/*
	Sets the _audio player's _audio file directory for
	the next time play() is called
	*/
	void setAudio(std::string audio);

	/*
	creates a new instance of sound that can run beside
	other instances of the same sound. can be set to
	repeat ONLY IF AUDIO IS NOT A .WAV FILE
	*/
	void play(bool repeat = false);

	/*
	Mutes and un-mutes _audio. _audio is muted
	or unmuted based on it's current state
	*/
	void mute();

	/*
	Pauses the all instances of a running audio file.
	paused audio can be resumed by using resume();
	*/
	void pause();

	/*
	Pauses the latest instance of a running audio files.
	paused audio can be resumed by using resume();
	*/
	void pauseAll();

	/*
	Resumes the latest instance of a paused _audio file.
	Continues to play from the last point it was paused.
	*/
	void resume();

	/*
	Stops the latest instance of the _audio file.
	When _audio is stoped it can not be resumed
	only restarted by play.
	*/
	void stop();

	/*
	Stops all instances of playing _audio.
	When _audio is stoped it can not be resumed
	only restarted by play.
	*/
	void stopAll();

	//Depriciated, will be fixed in a latter build	
	bool isPlaying(int inst = _inst);

	/*
	Prints the amount of open instances of the class,
	the name of the stream and if it is playing,
	has stoped, or is paused
	*/
	static std::vector<AudioInfo> info();

	/*
	Sets the volume of the audio.
	Lowest volume 0.0f
	Highest volume 1.0f
	*/
	void setVolume(float vol = 1);

	unsigned int getPosition();

	void setPosition(long ms);

	/*
	Closes audio instences that have been used
	*/
	static void cleanUp();
private:
	/****Utility Functions****/


	static bool printError(int);

	/****Utility Variables****/
	std::string* _audio = new std::string;
	static unsigned int _numInst, _inst;
	static AudioInfo _info;
};

