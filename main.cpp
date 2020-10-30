#include <functional>
#include <vector>
#include <string>
#include <filesystem>
#include <thread>
#include "EmConsole.h"
#include "keyinput.h"
#include "EmGineAudioPlayer.h"
#include "XinputManager.h"
#include <midifile/MidiFile.h>

#pragma region Namespace Stuff
using std::vector;
using std::wstring;
using std::string;
using std::pair;
using std::function;
using std::thread;
using std::stoi;
namespace fs = std::filesystem;
#pragma endregion

#pragma region Deffines

typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long long ullong;
typedef EmGineAudioPlayer AudioPlayer;
#define HEALTH_CAP 25

function<bool()>selector = [&]()->bool {
	if(KeyInput::stroke('A'))return true;
	for(int a = 0; a < 4; a++)
		if(XinputManager::controllerConnected(a))
			if(XinputManager::getController(a)->isButtonStroked(GUITAR_INPUT_BUTTONS::GUITAR_FRET_GREEN))return true;
	return false;
};
function<bool()>deselect = [&]()->bool {if(KeyInput::stroke('S'))return true;  for(int a = 0; a < 4; a++)if(XinputManager::controllerConnected(a))if(XinputManager::getController(a)->isButtonStroked(GUITAR_INPUT_BUTTONS::GUITAR_FRET_RED))return true; if(MouseInput::stroke(RIGHT_CLICK))return true; return false; };
function<bool()>guitarstart = [&]()->bool {if(KeyInput::stroke(VK_RETURN))return true; for(int a = 0; a < 4; a++) if(XinputManager::controllerConnected(a))if(XinputManager::getController(a)->isButtonStroked(GUITAR_INPUT_BUTTONS::GUITAR_START))return true; return false; };
function<bool()>guitarback = [&]()->bool {if(KeyInput::stroke(VK_BACK))return true; for(int a = 0; a < 4; a++) if(XinputManager::controllerConnected(a))if(XinputManager::getController(a)->isButtonStroked(GUITAR_INPUT_BUTTONS::GUITAR_BACK))return true; return false; };
function<bool()>goup = [&]()->bool
{
	if(KeyInput::stroke(VK_UP))
		return true;
	bool hitit = false;
	for(int a = 0; a < 4; a++)
		if(XinputManager::controllerConnected(a))
			if(XinputManager::getController(a)->isButtonStroked(GUITAR_INPUT_BUTTONS::GUITAR_STRUM_UP))
				hitit = true;
	return hitit;
};
function<bool()>godown = [&]()->bool
{
	if(KeyInput::stroke(VK_DOWN))
		return true;
	bool hitit = false;
	for(int a = 0; a < 4; a++)
		if(XinputManager::controllerConnected(a))
			if(XinputManager::getController(a)->isButtonStroked(GUITAR_INPUT_BUTTONS::GUITAR_STRUM_DOWN))
				hitit = true;
	return hitit; };

#pragma endregion

#pragma region Global Variables

struct InstrumentInfo
{
	string instrument;
	int player;
};

SpriteSheet* track = new SpriteSheet, * notes = new SpriteSheet, * logo = new SpriteSheet;
Sprite* box = new Sprite, * pauseMenu = new Sprite;

vector<vector<vector<uint>>>* disiNotes = new vector<vector<vector<uint>>>;
vector<ushort>
* fretColour = new vector<ushort>
{FG_GREEN,FG_RED,FG_YELLOW,FG_BLUE,FG_PURPLE},

* padColour = new vector<ushort>
{FG_PURPLE,FG_RED,FG_YELLOW,FG_BLUE,FG_GREEN},

* fretNoteColour = new vector<ushort>
{FG_GREEN | FG_INTENSIFY,FG_RED | FG_INTENSIFY,
FG_YELLOW | FG_INTENSIFY,FG_BLUE | FG_INTENSIFY,FG_PURPLE | FG_INTENSIFY},

* padNoteColour = new vector<ushort>
{FG_PURPLE | FG_INTENSIFY,FG_RED | FG_INTENSIFY,FG_YELLOW | FG_INTENSIFY,
FG_BLUE | FG_INTENSIFY,FG_GREEN | FG_INTENSIFY};

vector <int>* lines = new vector <int>;
vector<vector < vector < pair<long, long>>>>
* theTracks = new vector<vector < vector < pair<long, long>>>>, * theTrackTmps = new vector<vector < vector < pair<long, long>>>>;

vector<InstrumentInfo>* trackInfo = new vector<InstrumentInfo>;
vector<vector<pair<long, long>>>* vocalTrack = new vector<vector<pair<long, long>>>(5), * vocalTrackTmp = new vector<vector<pair<long, long>>>(5);

vector<vector<wstring>>* lyrics = new vector<vector<wstring>>;
vector<vector<pair<uint, uint>>>* lyricTiming = new vector<vector<pair<uint, uint>>>, * lyricTimingTmp = new vector<vector<pair<uint, uint>>>;
int  barCount, centerTrack, countAmountGuitar[5], countAmountBass[5], countAmountDrum[5], notesHit, noteOffset = 0, fretOffset,
fretboardPosition, noteSpeed = 40, lastSpeed, spacingScale = lastSpeed = noteSpeed, create;
uint colliCountGuitar[5], colliCountBass[5], colliCountDrum[5], firstLyric;

long incriment = -1;
float  fps = 0, speedPercent = 1, currentHealth = 0, lastHealth;
vector<vector<bool> >initPressed, pressed;
bool  start, paused(false), selected(false), exitSelect(false);
bool healthCheck;

int songChoice;
string* songName = new string, songDir/*, instrument*/;
wstring* percentstr = new wstring;
ushort difficulty[4], notePadding = 2;
thread* t1 = new thread;
#pragma endregion

#pragma region Deffinitions
void openSong(string);
bool collision(int);
bool noteDelete(int);
wstring cDir(wstring);
void reset();
void missFX(int index, int seed);
void playButtonPress();
void drawLines();
void barLines();
bool invisable(int, int, uint);
void playSongMovement();
float notesInSong(vector<vector<vector<pair<long, long>>>>*);
void percent();
void playTrack();
bool startScreen();
void songChoiceMovement(int);
bool createdSongList();
void calculateFPS();
void controles();
void playPauseMenu();
bool difficultyMenu();
void fpsLimiter(float limit);
#pragma endregion

#pragma region File I/O

wstring cDir(wstring s)
{
	for(auto& a : s)
		if(a == '\\')
			a = '/';
	return s;
}

void openSong(string songFile)
{

	smf::MidiFile file;
	file.read((songDir = songFile) + "notes.mid");
	file.doTimeAnalysis();
	string trackStr;
	theTracks->clear();
	initPressed.clear();
	pressed.clear();
	for(int a = 0; a < (int)trackInfo->size(); a++)
	{

		pressed.push_back(vector<bool>(5));
		initPressed.push_back(vector<bool>(5));
		theTracks->push_back(vector<vector<pair<long, long>>>(5));
		disiNotes->push_back(vector<vector<uint>>(5));
	}



	lyrics->clear();
	lyricTiming->clear();
	vocalTrack->clear();

	Sprite lyricBox("Game Files/Lyrics Box.txt");

	vocalTrack->assign(120, vector<pair<long, long>>());

	for(int a = 0; a < file.getNumTracks(); a++)
		for(int b = 0; b < file[a].size(); b++)
		{
			if(file[a][b].isTrackName())
			{
				trackStr = string(file[a][b].begin() + 3, file[a][b].end());
				if(trackStr == "PART VOCALS")
					continue;
				if(trackStr == "PART GUITAR")
					continue;
				if(trackStr == "PART BASS")
					continue;
				if(trackStr == "PART DRUMS")
					continue;

				break;
			}

			if(trackStr == "PART VOCALS")
			{

				if(file[a][b].isText())
				{
					if(file[a][b][3] == '[')
						continue;

					if(lyricTiming->size())
					{

						uint timeInSec = (uint)(file.getTimeInSeconds(a, b) * 1000);
						if(file[a][b].size() <= 3)
							continue;
						if((file[a][b][3] != '+' && []()->uint
						{
							unsigned b = 0; for(auto& a : lyrics[0].back())b += a.size(); return b;
						}() + wstring(file[a][b].begin() + 3, file[a][b].end()).size() + 2 < (uint)lyricBox.getWidth() - 1) &&
						   (timeInSec - lyricTiming[0].back().back().second < (uint)750))

							lyrics[0].back().push_back(L" " + wstring(file[a][b].begin() + 3, file[a][b].end())),
							lyricTiming[0].back().push_back({file.getTimeInSeconds(a, b) * 1000, file.getTimeInSeconds(a, b) * 1000});
						else
						{
							lyrics->push_back(vector<wstring>());
							lyricTiming->push_back(vector<pair<uint, uint>>());

							if(lyrics[0][lyrics->size() - 2].size())
								if(lyrics[0][lyrics->size() - 2].back().size())
									if(lyrics[0][lyrics->size() - 2].back().back() == '-')
									{
										function<void()> sylableGroup = [&]()
										{
											lyricTiming[0].back().insert(lyricTiming[0].back().begin(), lyricTiming[0][lyricTiming[0].size() - 2].back()),
												lyrics[0].back().insert(lyrics[0].back().begin(), lyrics[0][lyrics[0].size() - 2].back());

											lyricTiming[0][lyricTiming[0].size() - 2].pop_back(),
												lyrics[0][lyrics[0].size() - 2].pop_back();

											if(lyricTiming[0].size() - 2 > -1)
											{
												lyricTiming[0][lyricTiming[0].size() - 2].back().second = (uint)(file.getTimeInSeconds(a, b) * 1000);

												if(lyrics[0][lyrics->size() - 2].back().back() == L'-')
													sylableGroup();
											}
										};

										sylableGroup();

										lyrics->back().push_back(wstring(file[a][b].begin() + 3, file[a][b].end()));
										lyricTiming->back().push_back({file.getTimeInSeconds(a, b) * 1000,
																	  file.getTimeInSeconds(a, b) * 1000});

										//removes the '+'s from the song 
										if(lyrics->back().back().back() == L'+')
											lyrics->back().pop_back();

										continue;
									}

							lyricTiming[0][lyricTiming[0].size() - 2].back().second = (uint)(file.getTimeInSeconds(a, b) * 1000);

							lyrics->back().push_back(wstring(file[a][b].begin() + 3, file[a][b].end()));

							lyricTiming->back().push_back({file.getTimeInSeconds(a, b) * 1000,file.getTimeInSeconds(a, b) * 1000});
						}

					}
					else
					{
						lyrics->push_back(vector<wstring>());
						lyricTiming->push_back(vector<pair<uint, uint>>());

						lyrics->back().push_back(wstring(file[a][b].begin() + 3, file[a][b].end()));
						lyricTiming->back().push_back({file.getTimeInSeconds(a, b) * 1000,
													  file.getTimeInSeconds(a, b) * 1000});
					}

					//removes the '+'s from the song 
					if(lyrics->back().size())
						if(lyrics->back().back().back() == L'+')
							lyrics->back().pop_back();

					//the '^' is used to define spoken words
					if(lyrics->back().size())
						if(lyrics[0].back().back().back() == '^')
							lyrics[0].back().back().pop_back();

					//the '=' I think is just some sorta typo
					if(lyrics->back().size())
						if(lyrics[0].back().back().back() == '=')
							lyrics[0].back().back().back() = '-';

					//the '#' I don't know what it's used for
					if(lyrics->back().size())
						if(lyrics[0].back().back().back() == '#')
							lyrics[0].back().back().pop_back();

					continue;
				}

				if(file[a][b].isLyricText())
				{
					if(lyricTiming->size())
					{

						uint timeInSec = (uint)(file.getTimeInSeconds(a, b) * 1000);
						if(file[a][b].size() <= 3)
							continue;
						if((file[a][b][3] != '+' && []()->uint
						{
							unsigned b = 0; for(auto& a : lyrics[0].back())b += a.size(); return b;
						}() + wstring(file[a][b].begin() + 3, file[a][b].end()).size() + 2 < (uint)lyricBox.getWidth() - 1) &&
						   (timeInSec - lyricTiming[0].back().back().second < (uint)750))

							lyrics[0].back().push_back(L" " + wstring(file[a][b].begin() + 3, file[a][b].end())),
							lyricTiming[0].back().push_back({file.getTimeInSeconds(a, b) * 1000, file.getTimeInSeconds(a, b) * 1000});
						else
						{
							lyrics->push_back(vector<wstring>());
							lyricTiming->push_back(vector<pair<uint, uint>>());

							if(lyrics[0][lyrics->size() - 2].size())
								if(lyrics[0][lyrics->size() - 2].back().size())
									if(lyrics[0][lyrics->size() - 2].back().back() == '-')
									{
										function<void()> sylableGroup = [&]()
										{
											lyricTiming[0].back().insert(lyricTiming[0].back().begin(), lyricTiming[0][lyricTiming[0].size() - 2].back()),
												lyrics[0].back().insert(lyrics[0].back().begin(), lyrics[0][lyrics[0].size() - 2].back());

											lyricTiming[0][lyricTiming[0].size() - 2].pop_back(),
												lyrics[0][lyrics[0].size() - 2].pop_back();

											if(lyricTiming[0].size() - 2 > -1)
											{
												lyricTiming[0][lyricTiming[0].size() - 2].back().second = (uint)(file.getTimeInSeconds(a, b) * 1000);

												if(lyrics[0][lyrics->size() - 2].back().back() == L'-')
													sylableGroup();
											}
										};

										sylableGroup();

										lyrics->back().push_back(wstring(file[a][b].begin() + 3, file[a][b].end()));
										lyricTiming->back().push_back({file.getTimeInSeconds(a, b) * 1000,
																	  file.getTimeInSeconds(a, b) * 1000});

										//removes the '+'s from the song 
										if(lyrics->back().back().back() == L'+')
											lyrics->back().pop_back();

										continue;
									}

							lyricTiming[0][lyricTiming[0].size() - 2].back().second = (uint)(file.getTimeInSeconds(a, b) * 1000);

							lyrics->back().push_back(wstring(file[a][b].begin() + 3, file[a][b].end()));

							lyricTiming->back().push_back({file.getTimeInSeconds(a, b) * 1000,file.getTimeInSeconds(a, b) * 1000});
						}

					}
					else
					{
						lyrics->push_back(vector<wstring>());
						lyricTiming->push_back(vector<pair<uint, uint>>());

						lyrics->back().push_back(wstring(file[a][b].begin() + 3, file[a][b].end()));
						lyricTiming->back().push_back({file.getTimeInSeconds(a, b) * 1000,
													  file.getTimeInSeconds(a, b) * 1000});
					}

					//removes the '+'s from the song 
					if(lyrics->back().size())
						if(lyrics->back().back().back() == L'+')
							lyrics->back().pop_back();

					//the '^' is used to define spoken words
					if(lyrics->back().size())
						if(lyrics[0].back().back().back() == '^')
							lyrics[0].back().back().pop_back();

					//the '=' I think is just some sorta typo
					if(lyrics->back().size())
						if(lyrics[0].back().back().back() == '=')
							lyrics[0].back().back().back() = '-';

					//the '#' I don't know what it's used for
					if(lyrics->back().size())
						if(lyrics[0].back().back().back() == '#')
							lyrics[0].back().back().pop_back();

					continue;
				}

				if(file[a][b].isNoteOn())
				{
					int key = file[a][b].getKeyNumber();
					(*vocalTrack)[key].push_back({file.getTimeInSeconds(a, b) * 1000,0});
					continue;
				}

				if(file[a][b].isNoteOff())
				{
					int key = file[a][b].getKeyNumber();
					(*vocalTrack)[key][vocalTrack->at(key).size() - 1].second = (uint)(file.getTimeInSeconds(a, b) * 1000);
					continue;
				}
				continue;
			}

			for(int c = 0; c < (int)trackInfo->size(); c++)
			{
				if(trackInfo[0][c].instrument == "guitar")
					if(trackStr == "PART GUITAR")
					{
						if(file[a][b].isNoteOn())
						{
							int key = file[a][b].getKeyNumber();

							//gets solo section
							if(key == 12 * 8 + 7)
								NULL;
							if(key / 12 == difficulty[c])
								if(key % 12 < 5)
									(*theTracks)[c][key % 12].push_back({(long)(file.getTimeInSeconds(a, b) * -1000), 0});

							continue;
						}

						if(file[a][b].isNoteOff())
						{
							int key = file[a][b].getKeyNumber();

							//gets solo section
							if(key == 12 * 8 + 7)
								NULL;
							if(key / 12 == difficulty[c])
								if(key % 12 < 5)
									(*theTracks)[c][key % 12].back().second = ((long)(file.getTimeInSeconds(a, b) * -1000));
						}
					}

				if(trackInfo[0][c].instrument == "rhythm")
					if(trackStr == "PART BASS")
					{
						if(file[a][b].isNoteOn())
						{
							int key = file[a][b].getKeyNumber();

							//gets solo section
							if(key == 12 * 8 + 7)
								NULL;
							if(key / 12 == difficulty[c])
								if(key % 12 < 5)
									(*theTracks)[c][key % 12].push_back({(long)(file.getTimeInSeconds(a, b) * -1000), 0});

							continue;
						}

						if(file[a][b].isNoteOff())
						{
							int key = file[a][b].getKeyNumber();

							//gets solo section
							if(key == 12 * 8 + 7)
								NULL;
							if(key / 12 == difficulty[c])
								if(key % 12 < 5)
									(*theTracks)[c][key % 12].back().second = ((long)(file.getTimeInSeconds(a, b) * -1000));
						}
					}

				if(trackInfo[0][c].instrument == "drum")
					if(trackStr == "PART DRUMS")
					{
						if(file[a][b].isNoteOn())
						{
							int key = file[a][b].getKeyNumber();
							if(key / 12 == difficulty[c])
								if(key % 12 < 5)
									(*theTracks)[c][(key % 12)].push_back({(long)(file.getTimeInSeconds(a, b) * -1000),0});

							continue;
						}
					}
			}
		}
	file.clear();


	for(int a = 0; a < (int)lyrics->size(); a++)
	{
		if(!lyrics[0][a].size())//get rid of empty vectors
			lyrics->erase(lyrics->begin() + a),
			lyricTiming->erase(lyricTiming->begin() + a),
			a--;

		if(a > 0)//create blank spaces in long pauses
		{
			if(a < (int)lyrics->size())
				if(!lyrics[0][a].size())
					continue;

			//if(lyricTiming[0][a].front().first - lyricTiming[0][a - 1].back().first > 2000)
			//	lyrics->insert(lyrics->begin() + a, vector<wstring>(1)),
			//	lyricTiming->insert(lyricTiming->begin() + a, vector<pair<uint, uint>>
			//{ { lyricTiming[0][a - 1].back().first + (1500), lyricTiming[0][a].front().first } }),
			//	lyricTiming[0][a - 1].back().second = lyricTiming[0][a].back().first;

			if(lyricTiming[0][a].front().first - lyricTiming[0][a - 1].back().first > 2000)
				lyricTiming[0][a - 1].back().second = lyricTiming[0][a - 1].back().first + 1200;
		}
	}

	if(lyricTiming->size())
		lyricTiming->back().back().second += 750;

#pragma region Vocal Information
	for(uint a = 0; a < vocalTrack->size();)
		if((120 - vocalTrack->size()) / 12 > 1)
			if((*vocalTrack)[a].size() == 0)
				vocalTrack->erase(vocalTrack->begin());
			else
				break;
		else
			vocalTrack->erase(vocalTrack->begin());

	unsigned diff = 120 - vocalTrack->size();

	for(uint a = vocalTrack->size() - 1; a >= 0; a = vocalTrack->size() - 1)
		if((120 - (diff - vocalTrack->size())) / 12 < 8)
			if((*vocalTrack)[a].size() == 0)
				vocalTrack->erase(vocalTrack->end() - 1);
			else
				break;
		else
			vocalTrack->erase(vocalTrack->end() - 1);
#pragma endregion

	lyrics;
	lyricTiming;
	//guitarTrack;
	//AudioPlayer::stopAll();
	//for(auto &a : fs::directory_iterator(songDir))
	//{
	//	wstring ogg = cDir(a.path());
	//
	//	if(ogg.substr(ogg.find_last_of('.') + 1) == L"ogg")
	//		if(ogg.substr(ogg.find_last_of('/')+1, ogg.find_last_of('.') - ogg.find_last_of('/')-1) != L"preview")
	//			AudioPlayer::createAudioStream(string(ogg.begin(), ogg.end()).c_str());
	//}

}
#pragma endregion	

#pragma region Play

#pragma region Other

void percent()
{
	char p2[8];
	//if(instrument == "guitar")
	sprintf_s(p2, "%.2f", abs((notesInSong(theTracks) - notesHit) / notesInSong(theTracks) * 100 - 100));

	//if(instrument == "rhythm")
	//	sprintf_s(p2, "%.2f", abs((notesInSong(bassTrack) - notesHit) / notesInSong(bassTrack) * 100 - 100));
	//
	//if(instrument == "drum")
	//	sprintf_s(p2, "%.2f", abs((notesInSong(drumTrack) - notesHit) / notesInSong(drumTrack) * 100 - 100));

	string tmp = p2;
	tmp += '%';
	*percentstr = wstring(tmp.begin(), tmp.end());
}

float notesInSong(vector<vector<vector<pair<long, long>>>>* song)
{
	float count(0);
	for(auto& a : *song)
		for(auto& b : a)
			count += (float)b.size();
	return count;
}

void missFX(int index, int seed)
{
	srand(seed);
	char* name = new char[255];
	for(uint a = 0; a < AudioPlayer::size(); a++)
	{
		AudioPlayer::getAudioControle()->at(a)->sound->getName(name, 255);

		if(strstr(name, trackInfo[0][index].instrument.c_str()))
			AudioPlayer::setVolume(0, a);
	}
	delete name;

	std::unordered_map<string, std::vector<string>>miss
	{
		{"guitar",{"sfx/Guitar_Miss_1.wav","sfx/Guitar_Miss_2.wav","sfx/Guitar_Miss_3.wav","sfx/Guitar_Miss_4.wav"}},
		{"rhythm",{"sfx/Bass_Miss_1.wav","sfx/Bass_Miss_2.wav","sfx/Bass_Miss_3.wav","sfx/Bass_Miss_4.wav"}},
		{"drum",{"sfx/Drum_Miss_1.wav","sfx/Drum_Miss_2.wav","sfx/Drum_Miss_3.wav","sfx/Drum_Miss_4.wav"}}
	};
	AudioPlayer::createAudioStream(miss[trackInfo[0][index].instrument][rand() % miss[trackInfo[0][index].instrument].size()].c_str());
	AudioPlayer::setVolume(2);
	AudioPlayer::play();
}
#pragma endregion

#pragma region Note Alterations
long myMax(vector<vector<vector<pair<long, long>>>>* a_max)
{
	auto maxi = a_max[0][0].size();
	for(int a = 1; a < (int)a_max->size(); a++)
	{
		maxi = max(maxi, a_max[0][a].size());
	}
	return maxi;
}

bool collision(int index)
{
	bool colli(false);
	uint maxi = myMax(theTrackTmps);

	for(uint a = 0; a < maxi; a++)
	{

		if(a < theTrackTmps[0][index].size())
		{
			if(trackInfo[0][index].instrument == "guitar")
			{
				while(colliCountGuitar[a] < (*theTrackTmps)[index][a].size() &&
					  (*theTrackTmps)[index][a][colliCountGuitar[a]].first + noteOffset > fretboardPosition + 3)
					colliCountGuitar[a]++;

				if(pressed[index][a])
					if(colliCountGuitar[a] < (*theTrackTmps)[index][a].size())
						if((*theTrackTmps)[index][a][colliCountGuitar[a]].first + noteOffset > fretboardPosition - 3)
							colli = true;
			}

			if(trackInfo[0][index].instrument == "rhythm")
			{
				while(colliCountBass[a] < (*theTrackTmps)[index][a].size() &&
					  (*theTrackTmps)[index][a][colliCountBass[a]].first + noteOffset > fretboardPosition + 3)
					colliCountBass[a]++;

				if(pressed[index][a])
					if(colliCountBass[a] < (*theTrackTmps)[index][a].size())
						if((*theTrackTmps)[index][a][colliCountBass[a]].first + noteOffset > fretboardPosition - 3)
							colli = true;
			}

			if(trackInfo[0][index].instrument == "drum")
			{
				while(colliCountDrum[a] < (*theTrackTmps)[index][a].size() &&
					  (*theTrackTmps)[index][a][colliCountDrum[a]].first + noteOffset > fretboardPosition + 3)
					colliCountDrum[a]++;

				if(colliCountDrum[a] < (*theTrackTmps)[index][a].size())
					if((*theTrackTmps)[index][a][colliCountDrum[a]].first + noteOffset >= fretboardPosition - 3)
						colli = true;
			}
		}

	}

	return colli;
}

bool noteDelete(int index)
{
	static vector<int> num;
	num.clear();

	if(trackInfo[0][index].instrument == "guitar")
	{
		for(uint a = 0; a < theTrackTmps[0][index].size(); a++)
			if(colliCountGuitar[a] < (*theTrackTmps)[index][a].size())
				if((*theTrackTmps)[index][a][colliCountGuitar[a]].first + noteOffset > fretboardPosition - 3 &&
				   (*theTrackTmps)[index][a][colliCountGuitar[a]].first + noteOffset <= fretboardPosition + 3)
					num.push_back(a);

		uint lastOne = 0;
		bool init = true;
		for(uint a = 0; a < num.size(); a++)
		{
			if(init)
			{
				init = false;
				lastOne = a;
				continue;
			}

			if((*theTrackTmps)[index][num[lastOne]][colliCountGuitar[num[lastOne]]].first != (*theTrackTmps)[index][num[a]][colliCountGuitar[num[a]]].first)
			{
				long check = max((*theTrackTmps)[index][num[lastOne]][colliCountGuitar[num[lastOne]]].first, (*theTrackTmps)[index][num[a]][colliCountGuitar[num[a]]].first);

				if(check == (*theTrackTmps)[index][num[a]][colliCountGuitar[num[a]]].first)
					num.erase(num.begin() + lastOne);
				else
					num.erase(num.begin() + a);
				a--;
				init = true;
			}
		}


		if(num.size() == 1)
		{
			for(int a = 4; a > num[0]; a--)
				if(pressed[index][a])
					return false;

			if(pressed[index][num[0]])
				notesHit++,
				(*disiNotes)[index][num[0]].push_back(colliCountGuitar[num[0]]++);
		}
		else
		{
			int counter = 0, sum = 0;
			for(int a = 0; a < 5; a++)
				if(sum += pressed[index][a], pressed[index][a])
					if((uint)counter < num.size())
					{
						if(num[counter++] != a)
							return false;

					}
					else
						return false;


			if((uint)sum != num.size())
				return false;


			for(auto& a : num)
				notesHit++,
				(*disiNotes)[index][a].push_back(colliCountGuitar[a]++);
		}
	}
	else if(trackInfo[0][index].instrument == "rhythm")
	{
		for(uint a = 0; a < theTrackTmps[0][index].size(); a++)
			if(colliCountBass[a] < (*theTrackTmps)[index][a].size())
				if((*theTrackTmps)[index][a][colliCountBass[a]].first + noteOffset > fretboardPosition - 3 && (*theTrackTmps)[index][a][colliCountBass[a]].first + noteOffset <= fretboardPosition + 3)
					num.push_back(a);

		ushort lastOne = 0;
		bool init = true;
		for(uint a = 0; a < num.size(); a++)
		{
			if(init)
			{
				init = false;
				lastOne = (ushort)a;
				continue;
			}

			if((*theTrackTmps)[index][num[lastOne]][colliCountBass[num[lastOne]]].first != (*theTrackTmps)[index][num[a]][colliCountBass[num[a]]].first)
			{
				long check = max((*theTrackTmps)[index][num[lastOne]][colliCountBass[num[lastOne]]].first, (*theTrackTmps)[index][num[a]][colliCountBass[num[a]]].first);

				if(check == (*theTrackTmps)[index][num[a]][colliCountBass[num[a]]].first)
					num.erase(num.begin() + lastOne);
				else
					num.erase(num.begin() + a);
				a--;
				init = true;
			}
		}


		if(num.size() == 1)
		{
			for(int a = 4; a > num[0]; a--)
				if(pressed[index][a])
					return false;

			if(pressed[index][num[0]])
				notesHit++,
				(*disiNotes)[index][num[0]].push_back(colliCountBass[num[0]]++);
		}
		else
		{
			int counter = 0, sum = 0;
			for(int a = 0; a < 5; a++)
				if(sum += pressed[index][a], pressed[index][a])
					if((uint)counter < num.size())
					{
						if(num[counter++] != a)
							return false;

					}
					else
						return false;


			if((uint)sum != num.size())
				return false;


			for(auto& a : num)
				notesHit++,
				(*disiNotes)[index][a].push_back(colliCountBass[a]++);
		}
	}
	else if(trackInfo[0][index].instrument == "drum")
	{
		bool hitIt = false;
		for(uint a = 0; a < theTrackTmps[0][index].size(); a++)
			if(colliCountDrum[a] < (*theTrackTmps)[index][a].size())
				if((*theTrackTmps)[index][a][colliCountDrum[a]].first + noteOffset >= fretboardPosition - 3 && (*theTrackTmps)[index][a][colliCountDrum[a]].first + noteOffset <= fretboardPosition + 3)
					if(initPressed[index][a] && pressed[index][a])
						hitIt = true,
						notesHit++,
						(*disiNotes)[index][a].push_back(colliCountDrum[a]++);
		if(!hitIt)
			return false;
	}

	for(uint a = 0; a < AudioPlayer::size(); a++)
		AudioPlayer::setVolume(1, a);

	return true;
}

bool invisable(int index, int fret, uint check)
{
	for(uint b = 0; b < (*disiNotes)[index][fret].size(); b++)
		if((*disiNotes)[index][fret][b] == check)
			return true;

	return false;
}
#pragma endregion

#pragma region Bar Lines

void drawLines()
{

	static int spacing = (int)((float)(*notes)[0].getHeight() / ((float)spacingScale / (noteSpeed - (noteSpeed - (spacingScale * 2)) * 2)));

	int barNum = 4;
	int count = 0;
	for(int b = 0; b < (int)theTrackTmps->size(); b++)
	{
		for(uint a = 0; a < lines->size(); a++)
			if(incriment > 0)
				if(!((a + (barCount)) % barNum))
					EmConsole::toConsoleBuffer(trackInfo[0][b].instrument == "drum" ? L"━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" : L"━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━", centerTrack + 1 + count, ((*lines)[a] + incriment / noteSpeed),
											   FG_WHITE | FG_INTENSIFY);
				else
					EmConsole::toConsoleBuffer(trackInfo[0][b].instrument == "drum" ? L"───────────────────────────────────────────────────" : L"────────────────────────────────────────────────────────────────", centerTrack + 1 + count, ((*lines)[a] + incriment / noteSpeed),
											   FG_WHITE);
		count += (*track)[trackInfo[0][b].instrument == "guitar" || trackInfo[0][b].instrument == "rhythm" ? 0 : 1].getWidth();

	}
	if(!paused)
	{
		//create new bar lines
		while(((*lines)[lines->size() - 1] + incriment / noteSpeed) > spacing / 2)
			lines->push_back((*lines)[lines->size() - 1] - spacing);

		//adjust line spacing
		if(noteSpeed != lastSpeed)
		{
			for(uint a = 0; a < lines->size(); a++)
				if(a > 0)
					lines[0][a] = int(lines[0][a - 1] - (float)(*notes)[0].getHeight() / ((float)spacingScale / (noteSpeed - (noteSpeed - (spacingScale * 2)) * 2)));//apply spacing
				else if((incriment / noteSpeed - incriment / lastSpeed) < 0)
					lines[0][a] -= incriment / noteSpeed - incriment / lastSpeed;
				else
					lines[0][a] += incriment / lastSpeed - incriment / noteSpeed;



			lastSpeed = noteSpeed;
			spacing = int((float)(*notes)[0].getHeight() / ((float)spacingScale / (noteSpeed - (noteSpeed - (spacingScale * 2)) * 2)));
		}

		//delete old bar lines
		while(((*lines)[0] + incriment / noteSpeed) > EmConsole::getHeight())
		{
			lines->erase(lines->begin());
			(barCount)++;
			if(barCount > (barNum - 1))
				barCount = 0;
		}
	}

}

void barLines()
{
	if(AudioPlayer::size())
		incriment = AudioPlayer::getPosition(0);

	if(start)
	{
		start = false;
		int firstNote = 0;
		bool firstVal = false;
		for(auto& a : theTrackTmps[0][0])
			if(a.size() > 0)
				if(!firstVal)
					firstNote = a[0].first,
					firstVal = true;
				else if(a[0].first > firstNote)
					firstNote = a[0].first;

		firstNote = firstNote / noteSpeed + incriment / noteSpeed + fretboardPosition;

		lines->clear(),
			lines->push_back(firstNote);
	}

	drawLines();

}
#pragma endregion

#pragma region Game Play

void playPauseMenu()
{
	wstring options[6]{L"Resume", L"Restart", L"Speed", L"Change Difficulty",L" Song Select", L" Start Screen"};
	static uint colours[]{FG_WHITE, FG_GREEN | FG_INTENSIFY, FG_GREEN};
	static uint selection;
	const short optAmount = 6;

	MouseInput::update();

	AudioPlayer::pauseAll();


	if(((KeyInput::press('S') || XinputManager::getController(0)->isButtonPressed(GUITAR_FRET_RED)) || guitarback()) && !selected)
	{
		AudioPlayer::playAll();
		paused = false;
		selection = 0;
		return;
	}
	paused = true;

	if((selector() ||
	   (MouseInput::stroke(LEFT_CLICK) && [&]()->bool
	{
		short tmp = (short)ceil(EmConsole::getHeight() / 4.5f);
		int a = selection;
		if(Sprite({L"",options[a],L""}).mouseCollision({float(EmConsole::getWidth() / 2 - ((options[a]).size() / 2)), float(tmp + a * 2)}, MouseInput::position))
			return true;

		return false;
	}()) ||
	   guitarstart()) || (deselect() && selected))
		selected = !selected,
		std::swap(colours[1], colours[2]);

	if(!selected)
	{
		if(goup())
			selection > 0 ? selection-- : selection = optAmount - 1;
		else if(godown())
			selection + 1 < optAmount ? selection++ : selection = 0;

		for(uint a = 0; a < optAmount; a++)
		{
			short tmp2 = (short)ceil(EmConsole::getHeight() / 4.5f);

			if(Sprite({options[a]}).mouseCollision({float(EmConsole::getWidth() / 2 - options[a].size() / 2), float(tmp2 + a * 2)}, MouseInput::position))
				selection = a;
		}
	}

	if(selected)
		switch(selection)
		{
		case 0://Resume
			selected = !selected;
			selection = 0;
			std::swap(colours[1], colours[2]);
			//AudioPlayer::setPosition(AudioPlayer::getPosition(0) - 1000);
			AudioPlayer::playAll();
			paused = false;
			return;
		case 1://Restart
			selected = !selected;
			selection = 0;
			std::swap(colours[1], colours[2]);
			reset();
			AudioPlayer::playAll();
			paused = false;
			return;
		case 2://speed
			bool newSpeed;
			newSpeed = godown();
			if((newSpeed) || goup())
				if(((spacingScale * 2) * (speedPercent + (newSpeed ? .25 : -.25))) > 0 && ((float)(*notes)[0].getHeight() /
				   ((float)spacingScale / ((spacingScale * 2) * (speedPercent + (newSpeed ? .25 : -.25)) -
				   ((spacingScale * 2) * (speedPercent + (newSpeed ? .25 : -.25)) - spacingScale * 2) * 2))) > 0)

					noteSpeed = spacingScale * 2,
					noteSpeed = int(noteSpeed * (speedPercent += (newSpeed ? .25f : -.25f)));

			wchar_t percent[10];
			swprintf_s(percent, speedPercent != 1 ? L": %.2f%%" : L": normal", abs(2 - speedPercent));
			options[2] += percent;
			break;
		case 3://change difficulty
			EmConsole::clearConsole();
			difficultyMenu();
			openSong(songDir);
			reset();
			selected = !selected;
			paused = false;
			break;
		case 4://song select()
			EmConsole::clearConsole();
			exitSelect = !createdSongList();
			selected = !selected;
			selection = 0;
			std::swap(colours[1], colours[2]);
			reset();
			AudioPlayer::playAll();
			paused = false;
			return;
		case 5://start menu
			selected = !selected;
			selection = 0;
			std::swap(colours[1], colours[2]);
			paused = false;
			exitSelect = true;

			break;
		}

	EmConsole::toConsoleBuffer(*pauseMenu, EmConsole::getWidth() / 2 - (*pauseMenu).getWidth() / 2, EmConsole::getHeight() / 3 - (*pauseMenu).getHeight() / 2);

	for(uint a = 0; a < optAmount; a++)
		EmConsole::toConsoleBuffer(options[a], int(EmConsole::getWidth() / 2 - ((options[a]).size() / 2)), int(ceil(EmConsole::getHeight() / 4.5f) + a * 2), colours[selection == a]);


}

void playSongMovement()
{
	int count = 0;
	for(int c = 0; c < (int)theTrackTmps->size(); c++)
	{//Note Movement
		if(trackInfo[0][c].instrument == "guitar")
			for(short a = 0; a < (short)theTrackTmps[0][c].size(); a++)
			{
				// TODO: THI SHOULDNT BE BASs
				for(int b = countAmountBass[a]; b < (int)(*theTrackTmps)[c][a].size(); b++)
				{

					// moves the song along
					(*theTrackTmps)[c][a][b].first = (*theTracks)[c][a][b].first / noteSpeed + incriment / noteSpeed + fretboardPosition;

					//stop checking if nothing can be seen
					if((*theTrackTmps)[c][a][b].first <= -3)
						break;

					if(b > countAmountBass[a])
					{
						//note drop off
						if((*theTrackTmps)[c][a][b - 1].first + noteOffset > EmConsole::getHeight())
							countAmountBass[a]++;

						if(((*theTrackTmps)[c][a][b - 1].first >= (abs((*theTrackTmps)[c][a][b].first) - abs((*theTrackTmps)[c][a][b - 1].first))))//if the note below is off screen? (actually idk)
							if((*disiNotes)[c][a].size() > 0)
								if((uint)b == (*disiNotes)[c][a][0])
									(*disiNotes)[c][a].erase((*disiNotes)[c][a].begin());

					}
					if(!invisable(c, a, b))
						if((*theTrackTmps)[c][a][b].first + noteOffset > -3 && (*theTrackTmps)[c][a][b].first + noteOffset < EmConsole::getHeight())
						{

							if((*theTrackTmps)[c][a][b].first + noteOffset < fretboardPosition + notes[0][0].getHeight())//draw notes above the fret board
							{
								///Normal
								EmConsole::toConsoleBuffer((*notes)[0], (centerTrack)+(a * 13 + 2) + count, (*theTrackTmps)[c][a][b].first + noteOffset, (*fretNoteColour)[a]);

								///Collision Test
								//EmConsole::toConsoleBuffer((*notes)[0], (centerTrack) +(a * 13 + 2), (*songTemp)[a][b] + noteOffset,
								//					 b != colliCount[a] ? (*noteColour)[a] : FG_INTENSIFY | FG_WHITE);

							}
							else//draw notes below the fret board
								EmConsole::toConsoleBuffer((*notes)[0], (centerTrack)+(a * 13 + 2) + count, (*theTrackTmps)[c][a][b].first + noteOffset, FG_RED);

						}
				}
			}
		else if(trackInfo[0][c].instrument == "rhythm")
			for(short a = 0; a < (short)theTrackTmps[0][c].size(); a++)
			{
				for(int b = countAmountBass[a]; b < (int)(*theTrackTmps)[c][a].size(); b++)
				{

					// moves the song along
					(*theTrackTmps)[c][a][b].first = (*theTracks)[c][a][b].first / noteSpeed + incriment / noteSpeed + fretboardPosition;

					//stop checking if nothing can be seen
					if((*theTrackTmps)[c][a][b].first <= -3)
						break;

					if(b > countAmountBass[a])
					{
						//note drop off
						if((*theTrackTmps)[c][a][b - 1].first + noteOffset > EmConsole::getHeight())
							countAmountBass[a]++;

						if(((*theTrackTmps)[c][a][b - 1].first >= (abs((*theTrackTmps)[c][a][b].first) - abs((*theTrackTmps)[c][a][b - 1].first))))//if the note below is off screen? (actually idk)
							if((*disiNotes)[c][a].size() > 0)
								if((uint)b == (*disiNotes)[c][a][0])
									(*disiNotes)[c][a].erase((*disiNotes)[c][a].begin());

					}
					if(!invisable(c, a, b))
						if((*theTrackTmps)[c][a][b].first + noteOffset > -3 && (*theTrackTmps)[c][a][b].first + noteOffset < EmConsole::getHeight())
							if((*theTrackTmps)[c][a][b].first + noteOffset < fretboardPosition + notes[0][0].getHeight())//draw notes above the fret board
							{
								///Normal
								EmConsole::toConsoleBuffer((*notes)[0], (centerTrack)+(a * 13 + 2) + count, (*theTrackTmps)[c][a][b].first + noteOffset, (*fretNoteColour)[a]);

								///Collision Test
								//EmConsole::toConsoleBuffer((*notes)[0], (centerTrack) +(a * 13 + 2), (*songTemp)[a][b] + noteOffset,
								//					 b != colliCount[a] ? (*noteColour)[a] : FG_INTENSIFY | FG_WHITE);

							}
							else//draw notes below the fret board
								EmConsole::toConsoleBuffer((*notes)[0], (centerTrack)+(a * 13 + 2) + count, (*theTrackTmps)[c][a][b].first + noteOffset, FG_RED);
				}
			}
		else if(trackInfo[0][c].instrument == "drum")
			for(short a = 0; a < (short)theTrackTmps[0][c].size(); a++)
			{
				for(int b = countAmountDrum[a]; b < (int)(*theTrackTmps)[c][a].size(); b++)
				{
					// moves the song along
					(*theTrackTmps)[c][a][b].first = (*theTracks)[c][a][b].first / noteSpeed + incriment / noteSpeed + fretboardPosition;

					//stop checking if nothing can be seen
					if((*theTrackTmps)[c][a][b].first <= -3)
						break;

					if(b > countAmountDrum[a])
					{
						//note drop off
						if((*theTrackTmps)[c][a][b - 1].first + noteOffset > EmConsole::getHeight())
							countAmountDrum[a]++;

						if(((*theTrackTmps)[c][a][b - 1].first >= (abs((*theTrackTmps)[c][a][b].first) - abs((*theTrackTmps)[c][a][b - 1].first))))//if the note below is off screen? (actually idk)
							if((*disiNotes)[c][a].size() > 0)
								if((uint)b == (*disiNotes)[c][a][0])
									(*disiNotes)[c][a].erase((*disiNotes)[c][a].begin());

					}


					if(!invisable(c, a, b))
						if((*theTrackTmps)[c][a][b].first + noteOffset > -3 && (*theTrackTmps)[c][a][b].first + noteOffset < EmConsole::getHeight())
							if((*theTrackTmps)[c][a][b].first + noteOffset < fretboardPosition + notes[0][0].getHeight())//draw notes above the fret board
							{
								///Normal
								if(!a)
									EmConsole::toConsoleBuffer((*notes)[2], (centerTrack)+1 + count, (*theTrackTmps)[c][a][b].first + noteOffset + 1, (*padNoteColour)[0]);
								else
									a--,
									EmConsole::toConsoleBuffer((*notes)[0], (centerTrack)+(a * 13 + 2) + count, (*theTrackTmps)[c][a + 1][b].first + noteOffset, (*padNoteColour)[a + 1]),
									a++;

							}
							else//draw notes below the fret board
							{
								if(!a)
									EmConsole::toConsoleBuffer((*notes)[2], (centerTrack)+1 + count, (*theTrackTmps)[c][a][b].first + noteOffset + 1, FG_RED);
								else
									a--,
									EmConsole::toConsoleBuffer((*notes)[0], (centerTrack)+(a * 13 + 2) + count, (*theTrackTmps)[c][a + 1][b].first + noteOffset, FG_RED),
									a++;
							}
				}
			}
		count += (*track)[trackInfo[0][c].instrument == "guitar" || trackInfo[0][c].instrument == "rhythm" ? 0 : 1].getWidth();
	}
	//Lyrics Movement
	static Sprite lyricBox("Game Files/Lyrics Box.txt");
	EmConsole::toConsoleBuffer(lyricBox, EmConsole::getWidth() / 2 - lyricBox.getWidth() / 2, 0);
	for(uint a = firstLyric; a < lyrics->size(); a++)
	{
		if(incriment > (long)lyricTiming[0][a].back().second)
		{
			firstLyric++;
			continue;
		}

		if(incriment > (long)lyricTiming[0][a].front().first)
		{
			wstring phrase;
			for(auto& b : lyrics[0][a])
				phrase += b;

			1 > lyrics[0][a].size() - 1 ?
				EmConsole::toConsoleBuffer(lyrics[0][a][0], int(EmConsole::getWidth() / 2 - phrase.size() / 2), 2,
										   lyricTiming[0][a][0].first <= (uint)incriment ? FG_GREEN | FG_INTENSIFY : FG_WHITE | FG_INTENSIFY)
				:
				EmConsole::toConsoleBuffer(lyrics[0][a][0], int(EmConsole::getWidth() / 2 - phrase.size() / 2), 2,
										   lyricTiming[0][a][0].first <= (uint)incriment && lyricTiming[0][a][1].first > (uint)incriment ? FG_GREEN | FG_INTENSIFY : FG_WHITE | FG_INTENSIFY)
				;

			uint length = 0;
			for(uint b = 1; b < lyrics[0][a].size(); b++)
				b + 1 > lyrics[0][a].size() - 1 ?
				EmConsole::toConsoleBuffer(lyrics[0][a][b], int(EmConsole::getWidth() / 2 - phrase.size() / 2 + (length += lyrics[0][a][b - 1].size())), 2,
										   lyricTiming[0][a][b].first <= (uint)incriment ? FG_GREEN | FG_INTENSIFY : FG_WHITE | FG_INTENSIFY)
				:
				EmConsole::toConsoleBuffer(lyrics[0][a][b], int(EmConsole::getWidth() / 2 - phrase.size() / 2 + (length += lyrics[0][a][b - 1].size())), 2,
										   lyricTiming[0][a][b].first <= (uint)incriment && lyricTiming[0][a][b + 1].first > (uint)incriment ? FG_GREEN | FG_INTENSIFY : FG_WHITE | FG_INTENSIFY)
				;
		}

		break;
		//	lastLyricPosition = lyricPosition;
	}

}

void playButtonPress()
{
	XinputManager::update();
	static char keyfrets[]{'A','S','D','F','G'};
	static char keypads[]{VK_SPACE,'U','I','O','P'};
	static uint guitarfrets[]{GUITAR_FRET_GREEN, GUITAR_FRET_RED, GUITAR_FRET_YELLOW, GUITAR_FRET_BLUE, GUITAR_FRET_ORANGE};
	static uint drumpads[]{DRUM_KICK_PEDAL,DRUM_PAD_RED,DRUM_PAD_YELLOW,DRUM_PAD_BLUE,DRUM_PAD_GREEN};
	static short lastStrum = -1, currentStrum = -1;
	static bool stroke;

	//Key Stroke
	//if (KeyInput::stroke('R'))
	//	reset();

	//unsigned length;
	//AudioPlayer::getAudio()[0][0]->getLength(&length, FMOD_TIMEUNIT_MS);
	if(paused || guitarstart())
		playPauseMenu();

	bool up = 0, down = 0;
	//Strumming
	if(!paused)
	{
		KeyInput::press('Q');//eat this input;

		bool newSpeed = KeyInput::stroke(VK_NUMPAD4);

		if((newSpeed) || KeyInput::stroke(VK_NUMPAD6))
			if(((spacingScale * 2) * (speedPercent + (newSpeed ? .25f : -.25f))) > 0 && ((float)(*notes)[0].getHeight() /
			   ((float)spacingScale / ((spacingScale * 2) * (speedPercent + (newSpeed ? .25f : -.25f)) -
			   ((spacingScale * 2) * (speedPercent + (newSpeed ? .25f : -.25f)) - spacingScale * 2) * 2))) > 0)

				noteSpeed = spacingScale * 2,
				noteSpeed *= int(speedPercent += (newSpeed ? .25f : -.25f));

		for(int b = 0; b < (int)theTracks->size(); b++)
		{
			if(trackInfo[0][b].instrument == "guitar" || trackInfo[0][b].instrument == "rhythm")
			{
				//Note lights
				for(short a = 0; a < 5; a++)
				{
					if(KeyInput::release(keyfrets[a]) && XinputManager::getController(trackInfo[0][b].player)->isButtonReleased(guitarfrets[a]))
					{
						pressed[b][a] = false;
						if((*fretColour)[a] > 8)
							(*fretColour)[a] -= 8;
					}
					else if(KeyInput::press(keyfrets[a]) || XinputManager::getController(trackInfo[0][b].player)->isButtonPressed(guitarfrets[a]))
					{
						pressed[b][a] = true;
						if((*fretColour)[a] < 8)
							(*fretColour)[a] += 8;
					}
				}


				//Strum logic (if there is any)
				if([&]()->bool
				{
					up = KeyInput::press(VK_UP) || XinputManager::getController(trackInfo[0][b].player)->isButtonPressed(GUITAR_STRUM_UP);
						down = KeyInput::press(VK_DOWN) || XinputManager::getController(trackInfo[0][b].player)->isButtonPressed(GUITAR_STRUM_DOWN);
						currentStrum = up ? (down ? 4 : 2) : (down ? 1 : -1);

						if(currentStrum == -1)
							lastStrum = -1;
						return (lastStrum != currentStrum) && currentStrum != -1;
				} ())
				{
					lastStrum = currentStrum;

					if(collision(b))
					{
						if(!noteDelete(b))
							currentHealth -= currentHealth > 0 ? 1 : 0,
							missFX(b, rand());
						else
							currentHealth += currentHealth < HEALTH_CAP ? .5f : 0;
					}
					else
					{
						currentHealth -= currentHealth > 0 ? 1 : 0;
						missFX(b, rand());
					}
					if(currentHealth < 0)currentHealth = 0;
				}
			}
			else if(trackInfo[0][b].instrument == "drum")
			{
				//Note lights
				for(short a = 0; a < 5; a++)
				{
					if((KeyInput::release(keypads[a]) && XinputManager::getController(trackInfo[0][b].player)->isButtonReleased(drumpads[a])))
					{
						pressed[b][a] = false;
						initPressed[b][a] = true;
						if((*padColour)[a] > 8)
							(*padColour)[a] -= 8;
					}
					else if(KeyInput::press(keypads[a]) || XinputManager::getController(trackInfo[0][b].player)->isButtonPressed(drumpads[a]))
					{
						if(pressed[b][a])
							initPressed[b][a] = false;
						pressed[b][a] = true;

						if((*padColour)[a] < 8)
							(*padColour)[a] += 8;
					}
				}

				//Drum logic (if there is any)
				if([&]()->bool
				{
					int count = 0;
						for(int a = 0; a < 5; a++)
							count += pressed[b][a] && initPressed[b][a];
						return (bool)count;

				} ())
				{

					if(collision(b))
					{
						if(!noteDelete(b))
							currentHealth -= currentHealth > 0 ? 1 : 0,
							missFX(b, rand());
						else
							currentHealth += currentHealth < HEALTH_CAP ? .5f : 0;
					}
					else
					{
						currentHealth -= currentHealth > 0 ? 1 : 0;
						missFX(b, rand());
					}
					if(currentHealth < 0)currentHealth = 0;
				}

			}
		}
		deselect();//eat this input
	}
}

void playTrack()
{
	//EmConsole::mouseState();
	if(t1->joinable())
		t1->join();
	*t1 = thread(percent);
	t1->detach();
	EmConsole::toConsoleBuffer((*track)[trackInfo[0][0].instrument == "guitar" || trackInfo[0][0].instrument == "rhythm" ? 0 : 1], centerTrack, 0);
	int count = 0;
	for(int a = 1; a < (int)trackInfo->size(); a++)
	{
		count += (*track)[trackInfo[0][a - 1].instrument == "guitar" || trackInfo[0][a - 1].instrument == "rhythm" ? 0 : 1].getWidth();
		EmConsole::toConsoleBuffer((*track)[trackInfo[0][a].instrument == "guitar" || trackInfo[0][a].instrument == "rhythm" ? 0 : 1], centerTrack +
								   count, 0);
	}
	barLines();
	playSongMovement();
	playButtonPress();

	EmConsole::toConsoleBuffer(*box, EmConsole::getWidth() - (*box).getWidth(), 0);
	EmConsole::toConsoleBuffer(*percentstr, EmConsole::getWidth() - (*box).getWidth() / 2 - (percentstr->size() + 1) / 2, 2);

	count = 0;
	for(int a = 0; a < (int)trackInfo->size(); a++)
	{

		// draws frets that light up when key is pressed
		if(trackInfo[0][a].instrument == "guitar" || trackInfo[0][a].instrument == "rhythm")
			for(short b = 0; b < 5; b++)
				EmConsole::toConsoleBuffer((*notes)[0], (centerTrack)+(b * 13 + 2) + count, fretboardPosition, (*fretColour)[b]);

		else if(trackInfo[0][a].instrument == "drum")
		{
			EmConsole::toConsoleBuffer((*notes)[2], (centerTrack)+1 + count, fretboardPosition + 1, (*padColour)[0]);
			for(short b = 0; b < 4; b++)
				EmConsole::toConsoleBuffer((*notes)[0], (centerTrack)+(b * 13 + 2) + count, fretboardPosition, (*padColour)[b + 1]);
		}

		count += (*track)[trackInfo[0][a].instrument == "guitar" || trackInfo[0][a].instrument == "rhythm" ? 0 : 1].getWidth();

	}

	ushort a;
	static int healthX = 0, healthY = EmConsole::getHeight() - 1;
	EmConsole::toConsoleBuffer(L"┏━┓", healthX, healthY - HEALTH_CAP - 1);

	for(a = 1; a <= HEALTH_CAP; a++)
		EmConsole::toConsoleBuffer(L"┃ ┃", healthX, healthY - a);

	int tmp = (int)ceil(currentHealth);
	int bar[]{FG_RED | FG_INTENSIFY,FG_YELLOW | FG_INTENSIFY,FG_GREEN | FG_INTENSIFY};
	for(a = 1; a <= tmp; a++)
		if((a == tmp) && (bool)ceil(fmodf(currentHealth, 1)))
			EmConsole::toConsoleBuffer(L"▄", healthX + 1, healthY - a, bar[int(currentHealth / HEALTH_CAP * 3 > 2 ? 2 : currentHealth / HEALTH_CAP * 3)]);
		else
			EmConsole::toConsoleBuffer(L"█", healthX + 1, healthY - a, bar[int(currentHealth / HEALTH_CAP * 3 > 2 ? 2 : currentHealth / HEALTH_CAP * 3)]);


	EmConsole::toConsoleBuffer(L"┗━┛", healthX, healthY);
}
#pragma endregion
#pragma endregion

#pragma region Song Selection

void songChoiceMovement(int size)
{

	static float delay = .2f;
	static clock_t lastClockT[4], dClock;
	dClock = std::clock();
	for(int a = 0; a < 4; a++)
		if(float(dClock - lastClockT[a]) / CLOCKS_PER_SEC > delay)
		{
			lastClockT[a] = dClock;

			if(KeyInput::press(VK_DOWN) || XinputManager::getController(a)->isButtonPressed(CONTROLLER_DPAD_DOWN) || [&]()->bool {return XinputManager::getControllerType(a) == CONTROLLER_TYPE::XINPUT_DRUM && XinputManager::getController(a)->isButtonPressed(DRUM_PAD_BLUE); }())

			{
				AudioPlayer::setMasterVolume(1);
				if(songChoice < size - 1)
					songChoice++;
				else
					songChoice = 0;
				AudioPlayer::createAudioStream("sfx/Guitar_Miss_2.wav");
				AudioPlayer::setVolume(2);
				AudioPlayer::play();
			}
			else if(KeyInput::press(VK_UP) || XinputManager::getController(a)->isButtonPressed(GUITAR_INPUT_BUTTONS::GUITAR_STRUM_UP) || [&]()->bool {return XinputManager::getControllerType(a) == CONTROLLER_TYPE::XINPUT_DRUM && XinputManager::getController(a)->isButtonPressed(DRUM_PAD_YELLOW); }())
			{
				AudioPlayer::setMasterVolume(1);
				if(songChoice > 0)
					songChoice--;
				else
					songChoice = size - 1;
				AudioPlayer::createAudioStream("sfx/Guitar_Miss_2.wav");
				AudioPlayer::setVolume(2);
				AudioPlayer::play();
			}
		}



	for(int a = 0; a < 4; a++)
		if(XinputManager::getController(a)->isButtonReleased(GUITAR_INPUT_BUTTONS::GUITAR_STRUM_DOWN) && KeyInput::release(VK_DOWN) &&
		   XinputManager::getController(a)->isButtonReleased(GUITAR_INPUT_BUTTONS::GUITAR_STRUM_UP) && KeyInput::release(VK_UP))
		{
			lastClockT[a] = std::clock() - uint(CLOCKS_PER_SEC * delay);
			break;
		}
}

bool createdSongList()
{
	static wstring path = L"songs/";
	wstring p, playing, lastPlayed;
	vector<wstring>songs, songPath;
	wstring noteMsg[]{L"Select",L"Back"}, noteKey[]{L"A",L"S"};
	clock_t lastClockT = std::clock();
	static int count, movement, lastSongChoice;
	int colours[] = {FG_WHITE,FG_GREEN | FG_INTENSIFY}, sample = 0, startHeight = 0, endHeight = 0, startWidth = 0;
	unsigned long long lastClock = 0, clock = 0;

	while(true)
	{
		AudioPlayer::update();
		fpsLimiter(10);

		calculateFPS();
		wchar_t str[20];
		swprintf_s(str, L"%.2f", fps);
		EmConsole::toConsoleBuffer((wstring(L"fps: ") + str).c_str(), 0, 1);

		//guitarstart();//eat this input;
		KeyInput::press('Q');//eat this input;
		XinputManager::update();
		bool pass = selector();



		songChoiceMovement(count);

		startHeight = EmConsole::getHeight() / 3, endHeight = EmConsole::getHeight() - (EmConsole::getHeight() - startHeight) / 3, startWidth = EmConsole::getWidth() / 2;

		if(lastSongChoice != songChoice)
			lastClockT = std::clock();

		songs.clear();
		songPath.clear();
		count = 0;
		for(auto& a : fs::directory_iterator(path))
		{
			if(count < movement || count - movement >(endHeight - startHeight))
			{
				count++;
				continue;
			}
			p = cDir(a.path());

			if(songChoice == count)
			{
				lastSongChoice = songChoice;
				playing = wstring(p.begin(), p.end());
				playing = cDir(playing);
				playing += L'/';

				if(float(std::clock() - lastClockT) / CLOCKS_PER_SEC > 1.5f)
				{
					bool isSong = false;
					for(auto& b : fs::directory_iterator(a.path()))
					{
						wstring tmp = b.path();
						if(tmp.substr(tmp.find_last_of('.') + 1) == L"ini")
							isSong = true;
					}
					if(isSong)
						if(lastPlayed != playing)
						{

							AudioPlayer::stopAll();
							uint started = 5000;
							for(auto& b : fs::directory_iterator(playing))
							{
								wstring ogg = cDir(b.path());
								if(ogg.substr(ogg.find_last_of('.') + 1) == L"ogg")
									if(ogg.substr(ogg.find_last_of('/') + 1, ogg.find_last_of('.') - ogg.find_last_of('/') - 1) != L"preview")
										AudioPlayer::createAudioStream(string(ogg.begin(), ogg.end()).c_str());

								if(ogg.substr(ogg.find_last_of('.') + 1) == L"ini")
								{
									FILE* f;
									fopen_s(&f, string(ogg.begin(), ogg.end()).c_str(), "r");

									char* str2 = new char[255];

									while(str2 = fgets(str2, 255, f))
									{
										if(strstr(str2, "preview_start_time"))
										{
											str2[strlen(str2) - 1] = (str2[strlen(str2) - 1] == '\n' ? '\0' : str2[strlen(str2) - 1]);
											started = stoi(strstr(str2, "=") + 1) + 1;
											fclose(f);
											break;
										}
									}
									delete str2;
									fclose(f);
								}
							}

							EmGineAudioPlayer::getAudioSystem()->getSoftwareFormat(&sample, nullptr, nullptr);
							AudioPlayer::getMasterChannelGroup()->removeFadePoints(clock, clock + (sample * 25));

							AudioPlayer::getMasterChannelGroup()->getDSPClock(nullptr, &clock);
							AudioPlayer::getMasterChannelGroup()->addFadePoint(clock, 0);


							uint
								starting = started,
								ending = started + (25 * 1000);
							AudioPlayer::playAll(true, starting, ending, FMOD_TIMEUNIT_MS);

							AudioPlayer::getMasterChannelGroup()->getDSPClock(nullptr, &clock);
							AudioPlayer::getMasterChannelGroup()->addFadePoint(clock, 0);
							AudioPlayer::getMasterChannelGroup()->addFadePoint(clock + (sample * 2), 1);
							AudioPlayer::getMasterChannelGroup()->addFadePoint(clock + (sample * 23), 1);
							AudioPlayer::getMasterChannelGroup()->addFadePoint(clock + (sample * 25), 0);
							lastClock = clock;
							lastPlayed = playing;


						}
				}
			}

			songPath.push_back(p + L'/');

			bool isSong = false;
			for(auto& b : fs::directory_iterator(a.path()))
			{
				wstring tmp = b.path();
				if(tmp.substr(tmp.find_last_of('.') + 1) == L"ini")
					isSong = true;
			}

			if(isSong)
			{
				for(auto& b : fs::directory_iterator(songPath.back()))
					if(b.path().extension() == L".ini")
					{
						FILE* f;
						wstring s1(b.path());
						fopen_s(&f, string(s1.begin(), s1.end()).c_str(), "r");
						char* str2 = new char[255];

						p = L"";
						while(str2 = fgets(str2, 255, f))
						{
							if(strstr(str2, "name"))
							{
								bool end = p.size();
								str2[strlen(str2) - 1] = '\0';
								string s = strstr(str2, "=") + 1;
								p = wstring(s.begin(), s.end()) + L" -" + p;
								if(end)
									break;
							}
							else
								if(strstr(str2, "artist"))
								{
									bool end = p.size();
									str2[strlen(str2) - 1] = '\0';
									string s = strstr(str2, "=") + 1;
									p += wstring(s.begin(), s.end());
									if(end)
										break;
								}
						}
						delete str2;
						fclose(f);
						songs.push_back(p);
						count++;
						break;
					}
			}
			else
			{
				songs.push_back(p.substr(p.find_last_of('/') + 1));
				count++;
			}
		}

		if(pass || (MouseInput::stroke(LEFT_CLICK) && Sprite(vector<wstring> {songs[songChoice - movement]}).mouseCollision({float(startWidth - (songs[songChoice - movement].size() / 2)), float(startHeight + songChoice - movement)}, MouseInput::position)))
		{

			if(![&]()->bool
			{
				count = 0;
					for(auto& a : fs::directory_iterator(path))
					{
						if(songChoice == count++)
						{
							for(auto& b : fs::directory_iterator(a.path()))
							{
								wstring tmp = b.path();
									if(tmp.substr(tmp.find_last_of('.') + 1) == L"ini")
										return true;
							}
							continue;
						}
					}
				return false;
			}())
			{
				count = 0;
				for(auto& a : fs::directory_iterator(path))
					if(songChoice == count++)
					{
						path = cDir(a.path());
						break;
					}
				songChoice = 0;
			}
			else

				if((lastPlayed = L"", difficultyMenu()))
				{
					AudioPlayer::getMasterChannelGroup()->getDSPClock(nullptr, &clock);
					AudioPlayer::getMasterChannelGroup()->removeFadePoints(0, clock);
					AudioPlayer::getMasterChannelGroup()->removeFadePoints(clock, clock + (sample * 25));
					AudioPlayer::getMasterChannelGroup()->addFadePoint(0, 1);
					AudioPlayer::stopAll();
					openSong(string(playing.begin(), playing.end()));
					return true;
				}

		}
		if(deselect())
		{
			if(path == L"songs/")
			{
				AudioPlayer::getMasterChannelGroup()->getDSPClock(nullptr, &clock);
				AudioPlayer::getMasterChannelGroup()->removeFadePoints(0, clock);
				AudioPlayer::getMasterChannelGroup()->removeFadePoints(clock, clock + (sample * 25));
				AudioPlayer::getMasterChannelGroup()->addFadePoint(0, 1);
				AudioPlayer::stopAll();
				return false;
			}
			else
			{
				songChoice = 0;
				path = path.substr(0, path.substr(0, path.size() - 1).find_last_of('/') + 1);
			}

		}

		AudioPlayer::getMasterChannelGroup()->getDSPClock(nullptr, &clock);
		EmGineAudioPlayer::getAudioSystem()->getSoftwareFormat(&sample, nullptr, nullptr);

		//Loops the song fade in/out
		if(clock >= lastClock + (sample * 25))
		{
			AudioPlayer::getMasterChannelGroup()->removeFadePoints(lastClock, clock);
			AudioPlayer::getMasterChannelGroup()->addFadePoint(clock, 0);
			AudioPlayer::getMasterChannelGroup()->addFadePoint(clock + (sample * 2), 1);
			AudioPlayer::getMasterChannelGroup()->addFadePoint(clock + (sample * 23), 1);
			AudioPlayer::getMasterChannelGroup()->addFadePoint(clock + (sample * 25), 0);
			lastClock = clock;
		}

		if(songChoice >= (endHeight - startHeight) / 3 && songChoice < count - (endHeight - startHeight) / 3)
			movement =
			(
			startHeight + songChoice - movement > startHeight + (endHeight - startHeight) / 3 ?
			(startHeight + songChoice - movement > endHeight - (endHeight - startHeight) / 3 ?
			songChoice - (endHeight - (endHeight - startHeight) / 3 - startHeight) : movement)
			:
			songChoice - (endHeight - startHeight) / 3
			);

		else if(songChoice < (endHeight - startHeight) / 3)
			movement = 0;

		else if(songChoice > count - (endHeight - startHeight) / 3)
			movement = (count - 1) - int((endHeight - startHeight) / 2.f) - int((endHeight - startHeight) / 2.f);

		MouseInput::update();
		uint tmp;
		if((tmp = MouseInput::position.y - startHeight) >= 0 && MouseInput::position.y >= startHeight && MouseInput::position.y <= endHeight && tmp < songs.size())
			if(Sprite(vector<wstring> {songs[tmp]}).mouseCollision({float(startWidth - (songs[tmp].size() / 2)), float(startHeight + tmp)}, MouseInput::position))
				songChoice = MouseInput::position.y - startHeight + movement;


		for(uint a = 0; a < songs.size(); a++)
			EmConsole::toConsoleBuffer(songs[a], startWidth - (songs[a].size() / 2), startHeight + a, ((uint)songChoice - movement == a ? colours[1] : colours[0]));


		EmConsole::toConsoleBuffer(L"Song List", startWidth - 4, startHeight - 2);
		EmConsole::toConsoleBuffer(L"----------------------------", startWidth - 14, startHeight - 1);
		EmConsole::toConsoleBuffer(L"----------------------------", startWidth - 14, endHeight + 1);

		for(int a = 0; a < 2; a++)
			EmConsole::toConsoleBuffer(notes[0][0], int(EmConsole::getWidth() * .1f) + 12 * a, EmConsole::getHeight() - 4, fretNoteColour[0][a]),
			EmConsole::toConsoleBuffer(noteMsg[a], int(EmConsole::getWidth() * .1f) + 12 * a + notes[0][0].getWidth() / 2 - noteMsg[a].size() / 2, EmConsole::getHeight() - 3, fretNoteColour[0][a]),
			EmConsole::toConsoleBuffer(noteKey[a], int(EmConsole::getWidth() * .1f) + 12 * a + notes[0][0].getWidth() / 2 - noteKey[a].size() / 2, EmConsole::getHeight() - 5, fretNoteColour[0][a]);

		if(songChoice - movement >= 0 && (uint)songChoice - movement < songs.size())
			*songName = string(songs[songChoice - movement].begin(), songs[songChoice - movement].end());
		EmConsole::drawConsole();

	}

	return true;
}

bool difficultyMenu()
{

	static int selectx[4], selecty[4];
	int x = 0, y = 0;
	ushort numBoxes = 4 , colours[]{FG_WHITE,FG_GREEN | FG_INTENSIFY};
	string instruments[]{"GUITAR","BASS","DRUM"}, instruments2[]{"guitar","rhythm","drum"};
	wstring options[]{L"Easy",L"Medium",L"Hard",L"Expert"};
	wstring noteMsg[]{L"Select",L"Back"}, noteKey[]{L"A",L"S"};
	ullong clock;
	int sample;
	AudioPlayer::getMasterChannelGroup()->getDSPClock(nullptr, &clock);
	EmGineAudioPlayer::getAudioSystem()->getSoftwareFormat(&sample, nullptr, nullptr);

	AudioPlayer::getMasterChannelGroup()->getDSPClock(nullptr, &clock);
	AudioPlayer::getMasterChannelGroup()->removeFadePoints(0, clock);
	AudioPlayer::getMasterChannelGroup()->removeFadePoints(clock, clock + (sample * 25));
	AudioPlayer::getMasterChannelGroup()->addFadePoint(0, 1);
	AudioPlayer::stopAll();

	AudioPlayer::createAudioStream("Game Files/A_rock_song_idea.mp3");
	AudioPlayer::play(true);
	trackInfo->clear();

	static short movements[4];
	bool inits[4]{0,0,0,0}, ready[4]{0,0,0,0};


	while(true)
	{
		XinputManager::update();
		AudioPlayer::update();

		for(int a = 0; a < 4; a++)
		{
			KeyInput::press(VK_RETURN);//eat this input;
			if(deselect())
				return false;

			//bool clicked = MouseInput::stroke(LEFT_CLICK),
			//	check = selector();


			//if(selector())
			//	for(int a = 0; a < numBoxes; a++)
			//		if(check &&  clicked ? box->mouseCollision({(short)x, short(y + a * 5 + 1)}, MouseInput::position) : true)
			//			return true;

			if(inits[a])
			{
				x = EmConsole::getWidth() / 2 - (ushort)ceil((float)box->getWidth() / 2), y = EmConsole::getHeight() / 2 - 5;
				if(XinputManager::getController(a)->isButtonStroked(CONTROLLER_DPAD_UP))
					AudioPlayer::createAudioStream("sfx/Guitar_Miss_2.wav"),
					AudioPlayer::play(),
					selecty[a]--;
				if(XinputManager::getController(a)->isButtonStroked(CONTROLLER_DPAD_DOWN))
					AudioPlayer::createAudioStream("sfx/Guitar_Miss_2.wav"),
					AudioPlayer::play(),
					selecty[a]++;


				selectx[a] +=
					KeyInput::stroke(VK_LEFT) || XinputManager::getController(a)->isButtonStroked(CONTROLLER_DPAD_LEFT) ? -1 :
					KeyInput::stroke(VK_RIGHT) || XinputManager::getController(a)->isButtonStroked(CONTROLLER_DPAD_RIGHT) ? 1 : 0;

				if(selectx[a] < 0)
					selectx[a] = 2;
				selectx[a] %= 3;


				if(selecty[a] >= numBoxes)
					selecty[a] = 0;
				if(selecty[a] < 0)
					selecty[a] = numBoxes - 1;

				if(XinputManager::getController(a)->isButtonStroked(CONTROLLER_A))
				{
					ready[a] = true;
				}
			}
			//MouseInput::update();
			//for(int b = 0; b < numBoxes; b++)
			//	if(box->mouseCollision({(short)x, short(y + b * 5 + 1)}, MouseInput::position))
			//		selecty[a] = b;

			if(!inits[a])
				if(XinputManager::getController(a)->isButtonStroked(CONTROLLER_A))
				{
					inits[a] = true;
					trackInfo->push_back({instruments2[selectx[a]], a});
				}


		}

		if([&]()->bool {int counter = 0; for(int a = 0; a < 4; a++)counter += ready[a]; return counter == (int)trackInfo->size() && counter > 0; }())
			return true;

		for(int b = 0; b < (int)trackInfo->size(); b++)
		{
			trackInfo[0][b].instrument = instruments2[selectx[trackInfo[0][b].player]];

			int spacing = 35 / 4 * (trackInfo->size() - 1 < 0 ? 0 : trackInfo->size() - 1);

			EmConsole::toConsoleBuffer(L"Difficulty", EmConsole::getWidth() / 2 - 5 - (18 * (trackInfo->size() - b - 1)) + spacing, y - 3);
			EmConsole::toConsoleBuffer(wstring(instruments[selectx[trackInfo[0][b].player]].begin(), instruments[selectx[trackInfo[0][b].player]].end()).c_str(),
									   EmConsole::getWidth() / 2 - instruments[selectx[trackInfo[0][b].player]].size() / 2 - (18 * (trackInfo->size() - b - 1)) + spacing, y - 1, FG_RED | FG_INTENSIFY);
			EmConsole::toConsoleBuffer(L"-----------------------------------", EmConsole::getWidth() / 2 - 18 - (18 * (trackInfo->size() - b - 1)) + spacing, y);

			if(!ready[trackInfo[0][b].player])
				//draw selection boxes
				for(int a = 0; a < numBoxes; a++)
					EmConsole::toConsoleBuffer(*box, x - (18 * (trackInfo->size() - b - 1)) + spacing, y + a * 5 + 1, colours[selecty[trackInfo[0][b].player] == a]),
					EmConsole::toConsoleBuffer(options[a], x + (box->getWidth() / 2) - options[a].size() / 2 - (18 * (trackInfo->size() - b - 1)) + spacing, y + a * 5 + 3, colours[selecty[trackInfo[0][b].player] == a]);
			else
				for(int a = 0; a < numBoxes; a++)
					EmConsole::toConsoleBuffer(*box, x - (18 * (trackInfo->size() - b - 1)) + spacing, y + a * 5 + 1, colours[1]),
					EmConsole::toConsoleBuffer(L"READY", x + (box->getWidth() / 2) - 5 / 2 - (18 * (trackInfo->size() - b - 1)) + spacing, y + a * 5 + 3, colours[1]);

			switch(selecty[trackInfo[0][b].player])
			{
			case 0:
				difficulty[b] = 5;
				break;
			case 1:
				difficulty[b] = 6;
				break;
			case 2:
				difficulty[b] = 7;
				break;
			case 3:
				difficulty[b] = 8;
				break;
			}
		}
		for(int a = 0; a < 2; a++)
			EmConsole::toConsoleBuffer(notes[0][0], int(EmConsole::getWidth() * .1f) + 12 * a, EmConsole::getHeight() - 4, fretNoteColour[0][a]),
			EmConsole::toConsoleBuffer(noteMsg[a], int(EmConsole::getWidth() * .1f) + 12 * a + notes[0][0].getWidth() / 2 - noteMsg[a].size() / 2, EmConsole::getHeight() - 3, fretNoteColour[0][a]),
			EmConsole::toConsoleBuffer(noteKey[a], int(EmConsole::getWidth() * .1f) + 12 * a + notes[0][0].getWidth() / 2 - noteKey[a].size() / 2, EmConsole::getHeight() - 5, fretNoteColour[0][a]);
		EmConsole::drawConsole();
	}
}
#pragma endregion

#pragma region Other

void fpsLimiter(float limit)
{
	static bool enter = false;
	static clock_t frameStart;


	//way 1: 
	if(enter)
		if(limit > 0)
			while((CLOCKS_PER_SEC / limit) > (clock() - frameStart));

	////way 2: puts the thread to sleep 
	//if(enter)
	//	if(limit > 0)
	//		sleep((CLOCKS_PER_SEC / limit) - (clock() - frameStart));

	frameStart = clock();

	enter = true;
}

void reset()
{

	AudioPlayer::stopAll();
	lastHealth = currentHealth = (HEALTH_CAP / 2) + 1;
	for(auto& a : colliCountGuitar)a = 0;
	for(auto& a : countAmountGuitar)a = 0;
	for(auto& a : colliCountBass)a = 0;
	for(auto& a : countAmountBass)a = 0;
	for(auto& a : colliCountDrum)a = 0;
	for(auto& a : countAmountDrum)a = 0;
	for(auto& a : *disiNotes)
		for(auto& b : a)
			b.clear();
	firstLyric = 0;

	for(auto& a : fs::directory_iterator(songDir))
	{
		wstring ogg = cDir(a.path());
		if(ogg.substr(ogg.find_last_of('.') + 1) == L"ogg")
			if(ogg.substr(ogg.find_last_of('/') + 1, ogg.find_last_of('.') - ogg.find_last_of('/') - 1) != L"preview")
				AudioPlayer::createAudioStream(string(ogg.begin(), ogg.end()).c_str());
	}

	for(int a = 0; a < (int)theTrackTmps->size(); a++)
		theTrackTmps[0][a].clear();

	theTrackTmps[0] = theTracks[0];
	//bassTrackTmp->clear(),
	//	*bassTrackTmp = *bassTrack;
	//drumTrackTmp->clear(),
	//	*drumTrackTmp = *drumTrack;

	barCount =
		notesHit = 0;

	AudioPlayer::playAll();
	if(AudioPlayer::size())
		incriment = AudioPlayer::getPosition(0);
	start = true;
}

void calculateFPS()
{
	static const int SAMPLE = 15;
	static ushort count;
	static float frameTimes[SAMPLE];

	static uint timer;

	frameTimes[count++] = 1.f / ((float(clock() - timer) / CLOCKS_PER_SEC));
	if(count == SAMPLE)
	{
		count = 0;
		fps = 0;
		for(auto& a : frameTimes)
			fps += a;
		fps /= SAMPLE;
	}

	timer = clock();
}

void controles()
{
	Sprite controles;
	wstring noteMsg[]{L"Back",L"Select"}, noteKey[]{L"S",L"A"};
	controles.create("Game Files/Controles.txt");
	while((XinputManager::update(), !selector() && !deselect()))
	{
		EmConsole::toConsoleBuffer(controles, EmConsole::getWidth() / 2 - controles.getWidth() / 2, EmConsole::getHeight() / 2 - controles.getHeight() / 2);
		int a = 0;
		EmConsole::toConsoleBuffer(notes[0][0], int(EmConsole::getWidth() * .1f) + 12 * a, EmConsole::getHeight() - 4, fretNoteColour[0][a + 1]);
		EmConsole::toConsoleBuffer(noteMsg[a], int(EmConsole::getWidth() * .1f) + 12 * a + notes[0][0].getWidth() / 2 - noteMsg[a].size() / 2, EmConsole::getHeight() - 3, fretNoteColour[0][a + 1]);
		EmConsole::toConsoleBuffer(noteKey[a], int(EmConsole::getWidth() * .1f) + 12 * a + notes[0][0].getWidth() / 2 - noteKey[a].size() / 2, EmConsole::getHeight() - 5, fretNoteColour[0][a + 1]);

		EmConsole::drawConsole();
	}
}

bool startScreen()
{
	int x = EmConsole::getWidth() / 2 - box->getWidth() / 2, y = EmConsole::getHeight() / 2 - (2 * (box->getHeight() / 2)),
		colours[] = {FG_WHITE,FG_GREEN | FG_INTENSIFY}, numBoxes = 2;
	bool exit = false, enter = false;
	wstring options[]{L"Single Play",L"Controls"}, noteMsg[]{L"Select"}, noteKey[]{L"A"};
	Sprite thing;
	AudioPlayer::createAudioStream("Game Files/A_rock_song_idea.mp3");
	AudioPlayer::play(true);

	while(!exit)
	{

		MouseInput::update();
		AudioPlayer::update();

		for(int a = 0; a < numBoxes; a++)
			if(box->mouseCollision({(float)x,float(y + a * 5 + 1)}, MouseInput::position))
			{
				create = a;
				if(MouseInput::stroke(LEFT_CLICK))
					enter = true;
				else
					selector();//eat this input
			}

		//MouseInput::stroke(LEFT_CLICK);//eat this input
		x = EmConsole::getWidth() / 2 - (*box).getWidth() / 2, y = EmConsole::getHeight() / 2 - (2 * (box->getHeight() / 2));
		XinputManager::update();

		if(goup())
			AudioPlayer::createAudioStream("sfx/Guitar_Miss_2.wav"),
			AudioPlayer::play(),
			create--;
		else if(godown())
			AudioPlayer::createAudioStream("sfx/Guitar_Miss_2.wav"),
			AudioPlayer::play(),
			create++;

		if(create >= numBoxes)
			create = 0;
		if(create < 0)
			create = numBoxes - 1;

		EmConsole::toConsoleBuffer((*logo)[0], EmConsole::getWidth() / 2 - (*logo)[0].getWidth() / 2, 10, FG_RED | FG_INTENSIFY);


		//draw selection boxes
		for(int a = 0; a < numBoxes; a++)
			EmConsole::toConsoleBuffer(*box, x, y + a * 5, colours[create == a]),
			EmConsole::toConsoleBuffer(options[a], x + (box->getWidth() / 2) - options[a].size() / 2, y + a * 5 + 2, colours[create == a]);

		for(int a = 0; a < 1; a++)
			EmConsole::toConsoleBuffer(notes[0][0], int(EmConsole::getWidth() * .1f) + 10 * a, EmConsole::getHeight() - 4, fretNoteColour[0][a]),
			EmConsole::toConsoleBuffer(noteMsg[a], int(EmConsole::getWidth() * .1f) + 10 * a + notes[0][0].getWidth() / 2 - noteMsg[a].size() / 2, EmConsole::getHeight() - 3, fretNoteColour[0][a]),
			EmConsole::toConsoleBuffer(noteKey[a], int(EmConsole::getWidth() * .1f) + 10 * a + notes[0][0].getWidth() / 2 - noteKey[a].size() / 2, EmConsole::getHeight() - 5, fretNoteColour[0][a]);


		EmConsole::drawConsole();

		deselect();//eat this input

		if(KeyInput::stroke(VK_ESCAPE) || XinputManager::getController(0)->isButtonStroked(GUITAR_INPUT_BUTTONS::GUITAR_BACK))
			return false;

		if(selector() || enter)
			switch(create)
			{
			case 0:
				exit = createdSongList();
				if(!exit)
					AudioPlayer::createAudioStream("Game Files/A_rock_song_idea.mp3"),
					AudioPlayer::play(true);
				break;
			case 1:
				controles();
				break;
			}
		enter = false;
	}

	reset();
	return true;
}
#pragma endregion

int main()
{
	EmConsole::init("RockBand in Console");

	AudioPlayer::init();
	logo->create("Game Files/Logo.txt");
	track->create("Game Files/Track.txt");
	notes->create("Game Files/Notes.txt");
	pauseMenu->create("Game Files/Pause Menu.txt");
	spacingScale /= 2;

	//XinputManager::getController(0)->setVibrationR(1);

	EmConsole::setConsoleSize((*track)[0].getWidth() * 2, short((*track)[0].getHeight() * .70));
	EmConsole::placeConsoleCenter();
	EmConsole::setResizeable(true);

	std::vector<std::wstring>* boxy = new  std::vector < std::wstring >;
	*boxy =
	{

		L"┌───────────────┐",
		L"│               │",
		L"│               │",
		L"│               │",
		L"└───────────────┘"
	};

	box->create(boxy);
	delete boxy;

	while(true)
	{
		if(startScreen())
		{

			while(true)
			{
				AudioPlayer::update();

				centerTrack = EmConsole::getWidth() / 2;
				for(int a = 0; a < (int)trackInfo->size(); a++)
					centerTrack -= trackInfo[0][a].instrument == "guitar" || trackInfo[0][a].instrument == "rhythm" ? (*track)[0].getWidth() / 2 : (*track)[1].getWidth() / 2;
				fretboardPosition = EmConsole::getHeight() - 7;
				//	fretboardXPosition = (centerTrack) + (a * 13 + 2)

				if(!create)
					playTrack();

				if(exitSelect)
					break;

				//#ifdef _DEBUG
				calculateFPS();
				wchar_t str[20];
				swprintf_s(str, L"%.2f", fps);
				EmConsole::toConsoleBuffer((wstring(L"fps: ") + str).c_str(), 0, 1);

				//#endif // _DEBUG

				EmConsole::drawConsole();
			}
			if(t1->joinable())
				t1->join();
			reset();
			//for(auto&a : *guitarTrack)
			//	a.clear();
			lyrics->clear();
			lyricTiming->clear();
			vocalTrack->clear();
			AudioPlayer::stopAll();
		}
		else
			break;
		exitSelect = false;
		noteSpeed = spacingScale * 2,
			speedPercent = 1;
	}
	return 0;
}