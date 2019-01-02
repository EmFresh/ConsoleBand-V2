#include <functional>
#include "EmConsole.h"
#include "keyinput.h"
#include "EmGineAudioPlayer.h"
#include "XinputManager.h"
#include <vector>
#include <string>
#include <filesystem>
#include <thread>
#include <midifile/MidiFile.h>

#pragma region Namespace Stuff
using std::vector;
using std::wstring;
using std::string;
using std::pair;
using std::function;
using std::thread;
using std::stoi;
namespace fs = std::experimental::filesystem;
#pragma endregion

#pragma region Deffines

typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long long ullong;
typedef EmGineAudioPlayer AudioPlayer;
#define HEALTH_CAP 25

#define select (KeyInput::stroke('A') || XinputManager::getController(0)->isButtonStroked(GUITAR_INPUT_BUTTONS::GUITAR_FRET_GREEN))
#define deselect (KeyInput::stroke('S') || XinputManager::getController(0)->isButtonStroked(GUITAR_INPUT_BUTTONS::GUITAR_FRET_RED)||MouseInput::stroke(RIGHT_CLICK))
#define guitarstart (KeyInput::stroke(VK_RETURN) || XinputManager::getController(0)->isButtonStroked(GUITAR_INPUT_BUTTONS::GUITAR_START))
#define guitarback (KeyInput::stroke(VK_BACK) || XinputManager::getController(0)->isButtonStroked(GUITAR_INPUT_BUTTONS::GUITAR_BACK))
#define goup   (KeyInput::stroke(VK_UP) || XinputManager::getController(0)->isButtonStroked(GUITAR_INPUT_BUTTONS::GUITAR_STRUM_UP))
#define godown (KeyInput::stroke(VK_DOWN) || XinputManager::getController(0)->isButtonStroked(GUITAR_INPUT_BUTTONS::GUITAR_STRUM_DOWN))

#pragma endregion

#pragma region Global Variables

EmConsole *con = new EmConsole("RockBand in Console");

SpriteSheet *track = new SpriteSheet, *notes = new SpriteSheet, *logo = new SpriteSheet;
Sprite *box = new Sprite, *pauseMenu = new Sprite;

vector<vector<uint>>*disiNotes = new vector<vector<uint>>(5, vector<uint>());
vector<ushort>
*fretColour = new vector<ushort>
{FG_GREEN,FG_RED,FG_YELLOW,FG_BLUE,FG_PURPLE},

*padColour = new vector<ushort>
{FG_PURPLE,FG_RED,FG_YELLOW,FG_BLUE,FG_GREEN},

*fretNoteColour = new vector<ushort>
{FG_GREEN | FG_INTENSIFY,FG_RED | FG_INTENSIFY,
FG_YELLOW | FG_INTENSIFY,FG_BLUE | FG_INTENSIFY,FG_PURPLE | FG_INTENSIFY},

*padNoteColour = new vector<ushort>
{FG_PURPLE | FG_INTENSIFY,FG_RED | FG_INTENSIFY,FG_YELLOW | FG_INTENSIFY,
FG_BLUE | FG_INTENSIFY,FG_GREEN | FG_INTENSIFY};

vector <int> *lines = new vector <int>;
vector<vector<long>>
*guitarTrack = new vector<vector<long>>(5), *guitarTrackTmp = new vector<vector<long>>(5),
*bassTrack = new vector<vector<long>>(5), *bassTrackTmp = new vector<vector<long>>(5),
*drumTrack = new vector<vector<long>>(5), *drumTrackTmp = new vector<vector<long>>(5);
vector<vector<pair<long, long>>>*vocalTrack = new vector<vector<pair<long, long>>>(5), *vocalTrackTmp = new vector<vector<pair<long, long>>>(5);

vector<vector<wstring>>* lyrics = new vector<vector<wstring>>;
vector<vector<pair<uint, uint>>>* lyricTiming = new vector<vector<pair<uint, uint>>>, *lyricTimingTmp = new vector<vector<pair<uint, uint>>>;

int  barCount, centerTrack, countAmountGuitar[5], countAmountBass[5], countAmountDrum[5], notesHit, noteOffset = 0, fretOffset,
fretboardPosition, noteSpeed = 40, lastSpeed, spacingScale = lastSpeed = noteSpeed, create;
uint colliCountGuitar[5], colliCountBass[5], colliCountDrum[5], firstLyric;

long incriment = -1;
float  fps = 0, speedPercent = 1, currentHealth = 0, lastHealth;
bool initPressed[5], pressed[5], start, paused(false), selected(false), exitSelect(false);
bool healthCheck;


int songChoice;
string* songName = new string, songDir, instrument;
wstring *percentstr = new wstring;
ushort difficulty, notePadding = 2;
thread  *t1 = new thread;
#pragma endregion

#pragma region Deffinitions
void openSong(string);
bool collision();
bool noteDelete();
wstring cDir(wstring);
void reset();
void missFX(int seed);
void playButtonPress();
void drawLines();
void barLines();
bool invisable(int, uint);
void playSongMovement();
float notesInSong(vector<vector<long>>*);
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
	for(auto &a : s)
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

	for(auto&a : *guitarTrack)
		a.clear();
	for(auto&a : *bassTrack)
		a.clear();
	for(auto&a : *drumTrack)
		a.clear();


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
								int b = 0; for(auto&a : lyrics[0].back())b += a.size(); return b;
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
								int b = 0; for(auto&a : lyrics[0].back())b += a.size(); return b;
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

			if(instrument == "guitar")
				if(trackStr == "PART GUITAR")
				{
					if(file[a][b].isNoteOn())
					{
						int key = file[a][b].getKeyNumber();

						//gets solo section
						if(key == 12 * 8 + 7)
							NULL;
							if(key / 12 == difficulty)
								if(key % 12 < 5)
									(*guitarTrack)[key % 12].push_back((long)(file.getTimeInSeconds(a, b) * -1000));

						continue;
					}
				}

			if(instrument == "rhythm")
				if(trackStr == "PART BASS")
				{
					if(file[a][b].isNoteOn())
					{
						int key = file[a][b].getKeyNumber();
						if(key / 12 == difficulty)
							if(key % 12 < 5)
								(*bassTrack)[key % 12].push_back((long)(file.getTimeInSeconds(a, b) * -1000));

						continue;
					}
				}

			if(instrument == "drum")
				if(trackStr == "PART DRUMS")
				{
					if(file[a][b].isNoteOn())
					{
						int key = file[a][b].getKeyNumber();
						if(key / 12 == difficulty)
							if(key % 12 < 5)
								(*drumTrack)[(key % 12)].push_back((long)(file.getTimeInSeconds(a, b) * -1000));

						continue;
					}
				}
		}
	file.clear();


	for(uint a = 0; a < lyrics->size(); a++)
	{
		if(!lyrics[0][a].size())//get rid of empty vectors
			lyrics->erase(lyrics->begin() + a),
			lyricTiming->erase(lyricTiming->begin() + a),
			a--;

		if(a)//create blank spaces in long pauses
		{
			if(a < lyrics->size())
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

	int diff = 120 - vocalTrack->size();

	for(int a = vocalTrack->size() - 1; a >= 0; a = vocalTrack->size() - 1)
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
	guitarTrack;
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
	if(instrument == "guitar")
		sprintf_s(p2, "%.2f", abs((notesInSong(guitarTrack) - notesHit) / notesInSong(guitarTrack) * 100 - 100));

	if(instrument == "rhythm")
		sprintf_s(p2, "%.2f", abs((notesInSong(bassTrack) - notesHit) / notesInSong(bassTrack) * 100 - 100));

	if(instrument == "drum")
		sprintf_s(p2, "%.2f", abs((notesInSong(drumTrack) - notesHit) / notesInSong(drumTrack) * 100 - 100));

	string tmp = p2;
	tmp += '%';
	*percentstr = wstring(tmp.begin(), tmp.end());
}

float notesInSong(vector<vector<long>>*song)
{
	float count(0);
	for(auto &a : *song)
		count += (float)a.size();
	return count;
}

void missFX(int seed)
{
	srand(seed);
	char* name = new char[255];
	for(uint a = 0; a < AudioPlayer::size(); a++)
	{
		AudioPlayer::getAudio()->at(a)->getName(name, 255);
		if(strstr(name, instrument.c_str()))
			AudioPlayer::setVolume(0, a);
	}
	delete name;

	std::unordered_map<string, std::vector<string>>miss
	{
		{"guitar",{"sfx/Guitar_Miss_1.wav","sfx/Guitar_Miss_2.wav","sfx/Guitar_Miss_3.wav","sfx/Guitar_Miss_4.wav"}},
		{"rhythm",{"sfx/Bass_Miss_1.wav","sfx/Bass_Miss_2.wav","sfx/Bass_Miss_3.wav","sfx/Bass_Miss_4.wav"}},
		{"drum",{"sfx/Drum_Miss_1.wav","sfx/Drum_Miss_2.wav","sfx/Drum_Miss_3.wav","sfx/Drum_Miss_4.wav"}}
	};
	AudioPlayer::createAudioStream(miss[instrument][rand() % miss[instrument].size()].c_str());
	AudioPlayer::setVolume(2);
	AudioPlayer::play();
}
#pragma endregion

#pragma region Note Alterations

bool collision()
{
	bool colli(false);
	uint maxi = max(max(guitarTrackTmp->size(), bassTrackTmp->size()), drumTrackTmp->size());

	for(uint a = 0; a < maxi; a++)
	{
		if(a < guitarTrackTmp->size())
			if(instrument == "guitar")
			{
				while(colliCountGuitar[a] < (*guitarTrackTmp)[a].size() &&
					(*guitarTrackTmp)[a][colliCountGuitar[a]] + noteOffset > fretboardPosition + 3)
					colliCountGuitar[a]++;

				if(pressed[a])
					if(colliCountGuitar[a] < (*guitarTrackTmp)[a].size())
						if((*guitarTrackTmp)[a][colliCountGuitar[a]] + noteOffset > fretboardPosition - 3)
							colli = true;
			}

		if(a < bassTrackTmp->size())
			if(instrument == "rhythm")
			{
				while(colliCountBass[a] < (*bassTrackTmp)[a].size() &&
					(*bassTrackTmp)[a][colliCountBass[a]] + noteOffset > fretboardPosition + 3)
					colliCountBass[a]++;

				if(pressed[a])
					if(colliCountBass[a] < (*bassTrackTmp)[a].size())
						if((*bassTrackTmp)[a][colliCountBass[a]] + noteOffset > fretboardPosition - 3)
							colli = true;
			}

		if(a < drumTrackTmp->size())
			if(instrument == "drum")
			{
				while(colliCountDrum[a] < (*drumTrackTmp)[a].size() &&
					(*drumTrackTmp)[a][colliCountDrum[a]] + noteOffset > fretboardPosition + 3)
					colliCountDrum[a]++;

				if(colliCountDrum[a] < (*drumTrackTmp)[a].size())
					if((*drumTrackTmp)[a][colliCountDrum[a]] + noteOffset >= fretboardPosition - 3)
						colli = true;
			}
	}

	return colli;
}

bool noteDelete()
{
	static vector<int> num;
	num.clear();

	if(instrument == "guitar")
	{
		for(uint a = 0; a < guitarTrackTmp->size(); a++)
			if(colliCountGuitar[a] < (*guitarTrackTmp)[a].size())
				if((*guitarTrackTmp)[a][colliCountGuitar[a]] + noteOffset > fretboardPosition - 3 && (*guitarTrackTmp)[a][colliCountGuitar[a]] + noteOffset <= fretboardPosition + 3)
					num.push_back(a);

		short lastOne = 0;
		bool init = true;
		for(uint a = 0; a < num.size(); a++)
		{
			if(init)
			{
				init = false;
				lastOne = a;
				continue;
			}

			if((*guitarTrackTmp)[num[lastOne]][colliCountGuitar[num[lastOne]]] != (*guitarTrackTmp)[num[a]][colliCountGuitar[num[a]]])
			{
				long check = max((*guitarTrackTmp)[num[lastOne]][colliCountGuitar[num[lastOne]]], (*guitarTrackTmp)[num[a]][colliCountGuitar[num[a]]]);

				if(check == (*guitarTrackTmp)[num[a]][colliCountGuitar[num[a]]])
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
				if(pressed[a])
					return false;

			if(pressed[num[0]])
				notesHit++,
				(*disiNotes)[num[0]].push_back(colliCountGuitar[num[0]]++);
		}
		else
		{
			int counter = 0, sum = 0;
			for(int a = 0; a < 5; a++)
				if(sum += pressed[a], pressed[a])
					if((uint)counter < num.size())
					{
						if(num[counter++] != a)
							return false;

					}
					else
						return false;


			if((uint)sum != num.size())
				return false;


			for(auto&a : num)
				notesHit++,
				(*disiNotes)[a].push_back(colliCountGuitar[a]++);
		}
	}
	else if(instrument == "rhythm")
	{
		for(uint a = 0; a < bassTrackTmp->size(); a++)
			if(colliCountBass[a] < (*bassTrackTmp)[a].size())
				if((*bassTrackTmp)[a][colliCountBass[a]] + noteOffset > fretboardPosition - 3 && (*bassTrackTmp)[a][colliCountBass[a]] + noteOffset <= fretboardPosition + 3)
					num.push_back(a);

		short lastOne = 0;
		bool init = true;
		for(uint a = 0; a < num.size(); a++)
		{
			if(init)
			{
				init = false;
				lastOne = a;
				continue;
			}

			if((*bassTrackTmp)[num[lastOne]][colliCountBass[num[lastOne]]] != (*bassTrackTmp)[num[a]][colliCountBass[num[a]]])
			{
				long check = max((*bassTrackTmp)[num[lastOne]][colliCountBass[num[lastOne]]], (*bassTrackTmp)[num[a]][colliCountBass[num[a]]]);

				if(check == (*bassTrackTmp)[num[a]][colliCountBass[num[a]]])
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
				if(pressed[a])
					return false;

			if(pressed[num[0]])
				notesHit++,
				(*disiNotes)[num[0]].push_back(colliCountBass[num[0]]++);
		}
		else
		{
			int counter = 0, sum = 0;
			for(int a = 0; a < 5; a++)
				if(sum += pressed[a], pressed[a])
					if((uint)counter < num.size())
					{
						if(num[counter++] != a)
							return false;

					}
					else
						return false;


			if((uint)sum != num.size())
				return false;


			for(auto&a : num)
				notesHit++,
				(*disiNotes)[a].push_back(colliCountBass[a]++);
		}
	}
	else if(instrument == "drum")
	{
		bool hitIt = false;
		for(uint a = 0; a < drumTrackTmp->size(); a++)
			if(colliCountDrum[a] < (*drumTrackTmp)[a].size())
				if((*drumTrackTmp)[a][colliCountDrum[a]] + noteOffset >= fretboardPosition - 3 && (*drumTrackTmp)[a][colliCountDrum[a]] + noteOffset <= fretboardPosition + 3)
					if(initPressed[a] && pressed[a])
						hitIt = true,
						notesHit++,
						(*disiNotes)[a].push_back(colliCountDrum[a]++);
		if(!hitIt)
			return false;
	}

	for(uint a = 0; a < AudioPlayer::size(); a++)
		AudioPlayer::setVolume(1, a);

	return true;
}

bool invisable(int fret, uint check)
{
	for(uint b = 0; b < (*disiNotes)[fret].size(); b++)
		if((*disiNotes)[fret][b] == check)
			return true;

	return false;
}
#pragma endregion

#pragma region Bar Lines

void drawLines()
{
	static int spacing = (int)((float)(*notes)[0].getHeight() / ((float)spacingScale / (noteSpeed - (noteSpeed - (spacingScale * 2)) * 2)));

	int barNum = 4;
	for(uint a = 0; a < lines->size(); a++)
		if(incriment > 0)
			if(!((a + (barCount)) % barNum))
				con->toConsoleBuffer(instrument == "drum" ? L"━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" : L"━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━", centerTrack + 1, ((*lines)[a] + incriment / noteSpeed),
					FG_WHITE | FG_INTENSIFY);
			else
				con->toConsoleBuffer(instrument == "drum" ? L"───────────────────────────────────────────────────" : L"────────────────────────────────────────────────────────────────", centerTrack + 1, ((*lines)[a] + incriment / noteSpeed),
					FG_WHITE);

	if(!paused)
	{
		//create new barlines
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

		//deleat old barlines
		while(((*lines)[0] + incriment / noteSpeed) > con->getHeight())
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
		for(auto&a : instrument == "guitar" ? *guitarTrackTmp : instrument == "rhythm" ? *bassTrackTmp : instrument == "drum" ? *drumTrackTmp : vector<vector<long>>())
			if(a.size() > 0)
				if(!firstVal)
					firstNote = a[0],
					firstVal = true;
				else if(a[0] > firstNote)
					firstNote = a[0];

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

	if(((KeyInput::press('S') || XinputManager::getController(0)->isButtonPressed(GUITAR_FRET_RED)) || guitarback) && !selected)
	{
		AudioPlayer::playAll();
		paused = false;
		selection = 0;
		return;
	}
	paused = true;

	if((select ||
		(MouseInput::stroke(LEFT_CLICK) && [&]()->bool
			{
				short tmp = ceil(con->getHeight() / 4.5f);
				int a = selection;
				if(Sprite({L"",options[a],L""}).mouseCollision({short(con->getWidth() / 2 - ((options[a]).size() / 2)), short(tmp + a * 2)}, MouseInput::position))
					return true;

				return false;
			}()) ||
		guitarstart) || (deselect && selected))
		selected = !selected,
				std::swap(colours[1], colours[2]);

			if(!selected)
			{
				if(goup)
					selection > 0 ? selection-- : selection = optAmount - 1;
				else if(godown)
					selection + 1 < optAmount ? selection++ : selection = 0;

				for(uint a = 0; a < optAmount; a++)
				{
					short tmp2 = ceil(con->getHeight() / 4.5f);

					if(Sprite({options[a]}).mouseCollision({short(con->getWidth() / 2 - options[a].size() / 2), short(tmp2 + a * 2)}, MouseInput::position))
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
					newSpeed = godown;
					if((newSpeed) || goup)
						if(((spacingScale * 2) * (speedPercent + (newSpeed ? .25 : -.25))) > 0 && ((float)(*notes)[0].getHeight() /
							((float)spacingScale / ((spacingScale * 2) * (speedPercent + (newSpeed ? .25 : -.25)) -
							((spacingScale * 2) * (speedPercent + (newSpeed ? .25 : -.25)) - spacingScale * 2) * 2))) > 0)

							noteSpeed = spacingScale * 2,
							noteSpeed = int(noteSpeed * (speedPercent += (newSpeed ? .25f : -.25f)));

					wchar_t percent[10];
					swprintf_s(percent, speedPercent != 1 ? L": %.2f%%" : L": normal", abs(2 - speedPercent));
					options[2] += percent;
					break;
				case 3:
					con->clearConsole();
					difficultyMenu();
					openSong(songDir);
					reset();
					selected = !selected;
					paused = false;
					break;
				case 4://song select
					con->clearConsole();
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

			con->toConsoleBuffer(*pauseMenu, con->getWidth() / 2 - (*pauseMenu).getWidth() / 2, con->getHeight() / 3 - (*pauseMenu).getHeight() / 2);

			for(uint a = 0; a < optAmount; a++)
				con->toConsoleBuffer(options[a], (con->getWidth() / 2 - ((options[a]).size() / 2)), int(ceil(con->getHeight() / 4.5f) + a * 2), colours[selection == a]);


}

void playSongMovement()
{
	//Note Movement
	if(instrument == "guitar")
		for(short a = 0; a < (short)guitarTrackTmp->size(); a++)
		{
			for(int b = countAmountBass[a]; b < (int)(*guitarTrackTmp)[a].size(); b++)
			{

				// moves the song along
				(*guitarTrackTmp)[a][b] = (*guitarTrack)[a][b] / noteSpeed + incriment / noteSpeed + fretboardPosition;

				//stop checcking if nothing can be seen
				if((*guitarTrackTmp)[a][b] <= -3)
					break;

				if(b > countAmountBass[a])
				{
					//note dropoff
					if((*guitarTrackTmp)[a][b - 1] + noteOffset > con->getHeight())
						countAmountBass[a]++;

					if(((*guitarTrackTmp)[a][b - 1] >= (abs((*guitarTrackTmp)[a][b]) - abs((*guitarTrackTmp)[a][b - 1]))))//if the note below is off screen? (actually idk)
						if((*disiNotes)[a].size() > 0)
							if((uint)b == (*disiNotes)[a][0])
								(*disiNotes)[a].erase((*disiNotes)[a].begin());

				}
				if(!invisable(a, b))
					if((*guitarTrackTmp)[a][b] + noteOffset > -3 && (*guitarTrackTmp)[a][b] + noteOffset < con->getHeight())
						if((*guitarTrackTmp)[a][b] + noteOffset < fretboardPosition + notes[0][0].getHeight())//draw notes above the fret board
						{
							///Normal
							con->toConsoleBuffer((*notes)[0], (centerTrack)+(a * 13 + 2), (*guitarTrackTmp)[a][b] + noteOffset, (*fretNoteColour)[a]);

							///Colision Test
							//con->toConsoleBuffer((*notes)[0], (centerTrack) +(a * 13 + 2), (*songTemp)[a][b] + noteOffset,
							//					 b != colliCount[a] ? (*noteColour)[a] : FG_INTENSIFY | FG_WHITE);

						}
						else//draw notes below the fret board
							con->toConsoleBuffer((*notes)[0], (centerTrack)+(a * 13 + 2), (*guitarTrackTmp)[a][b] + noteOffset, FG_RED);
			}
		}
	else if(instrument == "rhythm")
		for(short a = 0; a < (short)bassTrackTmp->size(); a++)
		{
			for(int b = countAmountBass[a]; b < (int)(*bassTrackTmp)[a].size(); b++)
			{

				// moves the song along
				(*bassTrackTmp)[a][b] = (*bassTrack)[a][b] / noteSpeed + incriment / noteSpeed + fretboardPosition;

				//stop checcking if nothing can be seen
				if((*bassTrackTmp)[a][b] <= -3)
					break;

				if(b > countAmountBass[a])
				{
					//note dropoff
					if((*bassTrackTmp)[a][b - 1] + noteOffset > con->getHeight())
						countAmountBass[a]++;

					if(((*bassTrackTmp)[a][b - 1] >= (abs((*bassTrackTmp)[a][b]) - abs((*bassTrackTmp)[a][b - 1]))))//if the note below is off screen? (actually idk)
						if((*disiNotes)[a].size() > 0)
							if((uint)b == (*disiNotes)[a][0])
								(*disiNotes)[a].erase((*disiNotes)[a].begin());

				}
				if(!invisable(a, b))
					if((*bassTrackTmp)[a][b] + noteOffset > -3 && (*bassTrackTmp)[a][b] + noteOffset < con->getHeight())
						if((*bassTrackTmp)[a][b] + noteOffset < fretboardPosition + notes[0][0].getHeight())//draw notes above the fret board
						{
							///Normal
							con->toConsoleBuffer((*notes)[0], (centerTrack)+(a * 13 + 2), (*bassTrackTmp)[a][b] + noteOffset, (*fretNoteColour)[a]);

							///Colision Test
							//con->toConsoleBuffer((*notes)[0], (centerTrack) +(a * 13 + 2), (*songTemp)[a][b] + noteOffset,
							//					 b != colliCount[a] ? (*noteColour)[a] : FG_INTENSIFY | FG_WHITE);

						}
						else//draw notes below the fret board
							con->toConsoleBuffer((*notes)[0], (centerTrack)+(a * 13 + 2), (*bassTrackTmp)[a][b] + noteOffset, FG_RED);
			}
		}
	else if(instrument == "drum")
		for(short a = 0; a < (short)drumTrackTmp->size(); a++)
		{
			for(int b = countAmountDrum[a]; b < (int)(*drumTrackTmp)[a].size(); b++)
			{
				// moves the song along
				(*drumTrackTmp)[a][b] = (*drumTrack)[a][b] / noteSpeed + incriment / noteSpeed + fretboardPosition;

				//stop checcking if nothing can be seen
				if((*drumTrackTmp)[a][b] <= -3)
					break;

				if(b > countAmountDrum[a])
				{
					//note dropoff
					if((*drumTrackTmp)[a][b - 1] + noteOffset > con->getHeight())
						countAmountDrum[a]++;

					if(((*drumTrackTmp)[a][b - 1] >= (abs((*drumTrackTmp)[a][b]) - abs((*drumTrackTmp)[a][b - 1]))))//if the note below is off screen? (actually idk)
						if((*disiNotes)[a].size() > 0)
							if((uint)b == (*disiNotes)[a][0])
								(*disiNotes)[a].erase((*disiNotes)[a].begin());

				}


				if(!invisable(a, b))
					if((*drumTrackTmp)[a][b] + noteOffset > -3 && (*drumTrackTmp)[a][b] + noteOffset < con->getHeight())
						if((*drumTrackTmp)[a][b] + noteOffset < fretboardPosition + notes[0][0].getHeight())//draw notes above the fret board
						{
							///Normal
							if(!a)
								con->toConsoleBuffer((*notes)[2], (centerTrack)+1, (*drumTrackTmp)[a][b] + noteOffset + 1, (*padNoteColour)[0]);
							else
								a--,
								con->toConsoleBuffer((*notes)[0], (centerTrack)+(a * 13 + 2), (*drumTrackTmp)[a + 1][b] + noteOffset, (*padNoteColour)[a + 1]),
								a++;

						}
						else//draw notes below the fret board
						{
							if(!a)
								con->toConsoleBuffer((*notes)[2], (centerTrack)+1, (*drumTrackTmp)[a][b] + noteOffset + 1, FG_RED);
							else
								a--,
								con->toConsoleBuffer((*notes)[0], (centerTrack)+(a * 13 + 2), (*drumTrackTmp)[a + 1][b] + noteOffset, FG_RED),
								a++;
						}
			}
		}

	//Lyrics Movement
	static Sprite lyricBox("Game Files/Lyrics Box.txt");
	con->toConsoleBuffer(lyricBox, con->getWidth() / 2 - lyricBox.getWidth() / 2, 0);
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
			for(auto &b : lyrics[0][a])
				phrase += b;

			1 > lyrics[0][a].size() - 1 ?
				con->toConsoleBuffer(lyrics[0][a][0], (con->getWidth() / 2 - phrase.size() / 2), 2,
					lyricTiming[0][a][0].first <= (uint)incriment ? FG_GREEN | FG_INTENSIFY : FG_WHITE | FG_INTENSIFY)
				:
				con->toConsoleBuffer(lyrics[0][a][0], (con->getWidth() / 2 - phrase.size() / 2), 2,
					lyricTiming[0][a][0].first <= (uint)incriment && lyricTiming[0][a][1].first > (uint)incriment ? FG_GREEN | FG_INTENSIFY : FG_WHITE | FG_INTENSIFY)
				;

			uint length = 0;
			for(uint b = 1; b < lyrics[0][a].size(); b++)
				b + 1 > lyrics[0][a].size() - 1 ?
				con->toConsoleBuffer(lyrics[0][a][b], con->getWidth() / 2 - phrase.size() / 2 + (length += lyrics[0][a][b - 1].size()), 2,
					lyricTiming[0][a][b].first <= (uint)incriment ? FG_GREEN | FG_INTENSIFY : FG_WHITE | FG_INTENSIFY)
				:
				con->toConsoleBuffer(lyrics[0][a][b], con->getWidth() / 2 - phrase.size() / 2 + (length += lyrics[0][a][b - 1].size()), 2,
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
	if(paused || guitarstart)
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
		if(instrument == "guitar" || instrument == "rhythm")
		{
			//Note lights
			for(short a = 0; a < 5; a++)
			{
				if(KeyInput::release(keyfrets[a]) && XinputManager::getController(0)->isButtonReleased(guitarfrets[a]))
				{
					pressed[a] = false;
					if((*fretColour)[a] > 8)
						(*fretColour)[a] -= 8;
				}
				else if(KeyInput::press(keyfrets[a]) || XinputManager::getController(0)->isButtonPressed(guitarfrets[a]))
				{
					pressed[a] = true;
					if((*fretColour)[a] < 8)
						(*fretColour)[a] += 8;
				}
			}


			//Strum logic (if there is any)
			if([&]()->bool
				{
					up = KeyInput::press(VK_UP) || XinputManager::getController(0)->isButtonPressed(GUITAR_STRUM_UP);
						down = KeyInput::press(VK_DOWN) || XinputManager::getController(0)->isButtonPressed(GUITAR_STRUM_DOWN);
						currentStrum = up ? (down ? 4 : 2) : (down ? 1 : -1);

						if(currentStrum == -1)
							lastStrum = -1;
						return (lastStrum != currentStrum) && currentStrum != -1;
				} ())
			{
				lastStrum = currentStrum;

				if(collision())
				{
					if(!noteDelete())
						currentHealth -= currentHealth > 0 ? 1 : 0,
						missFX(rand());
					else
						currentHealth += currentHealth < HEALTH_CAP ? .5f : 0;
				}
				else
				{
					currentHealth -= currentHealth > 0 ? 1 : 0;
					missFX(rand());
				}
				if(currentHealth < 0)currentHealth = 0;
			}
		}
		else if(instrument == "drum")
		{
			//Note lights
			for(short a = 0; a < 5; a++)
			{
				if((KeyInput::release(keypads[a]) && XinputManager::getController(0)->isButtonReleased(drumpads[a])))
				{
					pressed[a] = false;
					initPressed[a] = true;
					if((*padColour)[a] > 8)
						(*padColour)[a] -= 8;
				}
				else if(KeyInput::press(keypads[a]) || XinputManager::getController(0)->isButtonPressed(drumpads[a]))
				{
					if(pressed[a])
						initPressed[a] = false;
					pressed[a] = true;

					if((*padColour)[a] < 8)
						(*padColour)[a] += 8;
				}
			}

			//Drum logic (if there is any)
			if([&]()->bool
				{
					int count = 0;
						for(int a = 0; a < 5; a++)
							count += pressed[a] && initPressed[a];
						return (bool)count;

				} ())
			{

				if(collision())
				{
					if(!noteDelete())
						currentHealth -= currentHealth > 0 ? 1 : 0,
						missFX(rand());
					else
						currentHealth += currentHealth < HEALTH_CAP ? .5f : 0;
				}
				else
				{
					currentHealth -= currentHealth > 0 ? 1 : 0;
					missFX(rand());
				}
				if(currentHealth < 0)currentHealth = 0;
			}

		}
		deselect;//eat this input
	}
}

void playTrack()
{
	//con->mouseState();
	con->toConsoleBuffer((*track)[instrument == "guitar" || instrument == "rhythm" ? 0 : 1], centerTrack, 0);
	if(t1->joinable())
		t1->join();
	*t1 = thread(percent);
	t1->detach();

	barLines();
	playSongMovement();
	playButtonPress();

	con->toConsoleBuffer(*box, con->getWidth() - (*box).getWidth(), 0);
	con->toConsoleBuffer(*percentstr, con->getWidth() - (*box).getWidth() / 2 - (percentstr->size() + 1) / 2, 2);

	// draws frets that lightup when key is pressed
	if(instrument == "guitar" || instrument == "rhythm")
		for(short a = 0; a < 5; a++)
			con->toConsoleBuffer((*notes)[0], (centerTrack)+(a * 13 + 2), fretboardPosition, (*fretColour)[a]);
	else if(instrument == "drum")
	{
		con->toConsoleBuffer((*notes)[2], (centerTrack)+1, fretboardPosition + 1, (*padColour)[0]);
		for(short a = 0; a < 4; a++)
			con->toConsoleBuffer((*notes)[0], (centerTrack)+(a * 13 + 2), fretboardPosition, (*padColour)[a + 1]);
	}

	ushort a;
	static int healthX = 0, healthY = con->getHeight() - 1;
	con->toConsoleBuffer(L"┏━┓", healthX, healthY - HEALTH_CAP - 1);

	for(a = 1; a <= HEALTH_CAP; a++)
		con->toConsoleBuffer(L"┃ ┃", healthX, healthY - a);

	int tmp = (int)ceil(currentHealth);
	int bar[]{FG_RED | FG_INTENSIFY,FG_YELLOW | FG_INTENSIFY,FG_GREEN | FG_INTENSIFY};
	for(a = 1; a <= tmp; a++)
		if((a == tmp) && (bool)ceil(fmodf(currentHealth, 1)))
			con->toConsoleBuffer(L"▄", healthX + 1, healthY - a, bar[int(currentHealth / HEALTH_CAP * 3 > 2 ? 2 : currentHealth / HEALTH_CAP * 3)]);
		else
			con->toConsoleBuffer(L"█", healthX + 1, healthY - a, bar[int(currentHealth / HEALTH_CAP * 3 > 2 ? 2 : currentHealth / HEALTH_CAP * 3)]);


	con->toConsoleBuffer(L"┗━┛", healthX, healthY);
}
#pragma endregion
#pragma endregion

#pragma region Song Selection

void songChoiceMovement(int size)
{

	static float delay = .2f;
	static clock_t lastClockT;
	if(float(std::clock() - lastClockT) / CLOCKS_PER_SEC > delay)
	{
		lastClockT = std::clock();

		if(KeyInput::press(VK_DOWN) || XinputManager::getController(0)->isButtonPressed(CONTROLLER_DPAD_DOWN))

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
		else if(KeyInput::press(VK_UP) || XinputManager::getController(0)->isButtonPressed(GUITAR_INPUT_BUTTONS::GUITAR_STRUM_UP))
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
	if(XinputManager::getController(0)->isButtonReleased(GUITAR_INPUT_BUTTONS::GUITAR_STRUM_DOWN) && KeyInput::release(VK_DOWN) &&
		XinputManager::getController(0)->isButtonReleased(GUITAR_INPUT_BUTTONS::GUITAR_STRUM_UP) && KeyInput::release(VK_UP))
		lastClockT = std::clock() - uint(CLOCKS_PER_SEC*delay);
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
		con->toConsoleBuffer((wstring(L"fps: ") + str).c_str(), 0, 1);

		//guitarstart;//eat this input;
		KeyInput::press('Q');//eat this input;
		XinputManager::update();
		bool pass = select;



		songChoiceMovement(count);

		startHeight = con->getHeight() / 3, endHeight = con->getHeight() - (con->getHeight() - startHeight) / 3, startWidth = con->getWidth() / 2;

		if(lastSongChoice != songChoice)
			lastClockT = std::clock();

		songs.clear();
		songPath.clear();
		count = 0;
		for(auto &a : fs::directory_iterator(path))
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
					for(auto &b : fs::directory_iterator(a.path()))
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
							for(auto &b : fs::directory_iterator(playing))
							{
								wstring ogg = cDir(b.path());
								if(ogg.substr(ogg.find_last_of('.') + 1) == L"ogg")
									if(ogg.substr(ogg.find_last_of('/') + 1, ogg.find_last_of('.') - ogg.find_last_of('/') - 1) != L"preview")
										AudioPlayer::createAudioStream(string(ogg.begin(), ogg.end()).c_str());

								if(ogg.substr(ogg.find_last_of('.') + 1) == L"ini")
								{
									FILE* f;
									fopen_s(&f, string(ogg.begin(), ogg.end()).c_str(), "r");

									char *str2 = new char[255];

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
			for(auto &b : fs::directory_iterator(a.path()))
			{
				wstring tmp = b.path();
				if(tmp.substr(tmp.find_last_of('.') + 1) == L"ini")
					isSong = true;
			}

			if(isSong)
			{
				for(auto &b : fs::directory_iterator(songPath.back()))
					if(b.path().extension() == L".ini")
					{
						FILE* f;
						wstring s1(b.path());
						fopen_s(&f, string(s1.begin(), s1.end()).c_str(), "r");
						char *str2 = new char[255];

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

		if(pass || (MouseInput::stroke(LEFT_CLICK) && Sprite(vector<wstring> {songs[songChoice - movement]}).mouseCollision({short(startWidth - (songs[songChoice - movement].size() / 2)), short(startHeight + songChoice - movement)}, MouseInput::position)))
		{

			if(![&]()->bool
				{
					count = 0;
						for(auto &a : fs::directory_iterator(path))
						{
							if(songChoice == count++)
								for(auto &b : fs::directory_iterator(a.path()))
								{
									wstring tmp = b.path();
										if(tmp.substr(tmp.find_last_of('.') + 1) == L"ini")
											return true;
								}
						}
					return false;
				}())
			{
				count = 0;
				for(auto &a : fs::directory_iterator(path))
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
		if(deselect)
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
		if((tmp = MouseInput::position.Y - startHeight) >= 0 && MouseInput::position.Y >= startHeight && MouseInput::position.Y <= endHeight && tmp < songs.size())
			if(Sprite(vector<wstring> {songs[tmp]}).mouseCollision({short(startWidth - (songs[tmp].size() / 2)), short(startHeight + tmp)}, MouseInput::position))
				songChoice = MouseInput::position.Y - startHeight + movement;


		for(uint a = 0; a < songs.size(); a++)
			con->toConsoleBuffer(songs[a], startWidth - (songs[a].size() / 2), startHeight + a, ((uint)songChoice - movement == a ? colours[1] : colours[0]));


		con->toConsoleBuffer(L"Song List", startWidth - 4, startHeight - 2);
		con->toConsoleBuffer(L"----------------------------", startWidth - 14, startHeight - 1);
		con->toConsoleBuffer(L"----------------------------", startWidth - 14, endHeight + 1);

		for(int a = 0; a < 2; a++)
			con->toConsoleBuffer(notes[0][0], int(con->getWidth() * .1f) + 12 * a, con->getHeight() - 4, fretNoteColour[0][a]),
			con->toConsoleBuffer(noteMsg[a], int(con->getWidth() * .1f) + 12 * a + notes[0][0].getWidth() / 2 - noteMsg[a].size() / 2, con->getHeight() - 3, fretNoteColour[0][a]),
			con->toConsoleBuffer(noteKey[a], int(con->getWidth() * .1f) + 12 * a + notes[0][0].getWidth() / 2 - noteKey[a].size() / 2, con->getHeight() - 5, fretNoteColour[0][a]);

		if(songChoice - movement >= 0 && (uint)songChoice - movement < songs.size())
			*songName = string(songs[songChoice - movement].begin(), songs[songChoice - movement].end());
		con->drawConsole();

	}

	return true;
}

bool difficultyMenu()
{

	static int selectx = 0, selecty = 0;
	ushort numBoxes = 4, x = 0, y = 0, colours[]{FG_WHITE,FG_GREEN | FG_INTENSIFY};
	string instruments[]{"guitar","bass","drum"}, instruments2[]{"guitar","rhythm","drum"};
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

	while(true)
	{
		XinputManager::update();

		instrument = instruments2[selectx];
		KeyInput::press(VK_RETURN);//eat this input;
		if(deselect)
			return false;

		bool clicked = MouseInput::stroke(LEFT_CLICK),
			check = select;


		if(check || clicked)
			for(int a = 0; a < numBoxes; a++)
				if(check &&  clicked ? box->mouseCollision({(short)x, short(y + a * 5 + 1)}, MouseInput::position) : true)
					return true;

		x = con->getWidth() / 2 - (ushort)ceil((float)box->getWidth() / 2), y = con->getHeight() / 2 - 5;
		if(goup)
			AudioPlayer::createAudioStream("sfx/Guitar_Miss_2.wav"),
			AudioPlayer::play(),
			selecty--;
		if(godown)
			AudioPlayer::createAudioStream("sfx/Guitar_Miss_2.wav"),
			AudioPlayer::play(),
			selecty++;


		selectx +=
			KeyInput::stroke(VK_LEFT) || XinputManager::getController(0)->isButtonStroked(CONTROLLER_DPAD_LEFT) ? -1 :
			KeyInput::stroke(VK_RIGHT) || XinputManager::getController(0)->isButtonStroked(CONTROLLER_DPAD_RIGHT) ? 1 : 0;

		if(selectx < 0)
			selectx = 2;
		selectx %= 3;


		if(selecty >= numBoxes)
			selecty = 0;
		if(selecty < 0)
			selecty = numBoxes - 1;

		MouseInput::update();
		for(int a = 0; a < numBoxes; a++)
			if(box->mouseCollision({(short)x, short(y + a * 5 + 1)}, MouseInput::position))
				selecty = a;

		con->toConsoleBuffer(L"Difficulty", con->getWidth() / 2 - 5, y - 3);
		con->toConsoleBuffer(wstring(instruments[selectx].begin(), instruments[selectx].end()).c_str(), con->getWidth() / 2 - instruments[selectx].size() / 2, y - 1, FG_RED | FG_INTENSIFY);
		con->toConsoleBuffer(L"-----------------------------------", con->getWidth() / 2 - 18, y);

		//draw selection boxes
		for(int a = 0; a < numBoxes; a++)
			con->toConsoleBuffer(*box, x, y + a * 5 + 1, colours[selecty == a]),
			con->toConsoleBuffer(options[a], x + (box->getWidth() / 2) - options[a].size() / 2, y + a * 5 + 3, colours[selecty == a]);

		for(int a = 0; a < 2; a++)
			con->toConsoleBuffer(notes[0][0], int(con->getWidth() * .1f) + 12 * a, con->getHeight() - 4, fretNoteColour[0][a]),
			con->toConsoleBuffer(noteMsg[a], int(con->getWidth() * .1f) + 12 * a + notes[0][0].getWidth() / 2 - noteMsg[a].size() / 2, con->getHeight() - 3, fretNoteColour[0][a]),
			con->toConsoleBuffer(noteKey[a], int(con->getWidth() * .1f) + 12 * a + notes[0][0].getWidth() / 2 - noteKey[a].size() / 2, con->getHeight() - 5, fretNoteColour[0][a]);

		con->drawConsole();
		switch(selecty)
		{
		case 0:
			difficulty = 5;
			break;
		case 1:
			difficulty = 6;
			break;
		case 2:
			difficulty = 7;
			break;
		case 3:
			difficulty = 8;
			break;
		}
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
	for(auto &a : colliCountGuitar)a = 0;
	for(auto &a : countAmountGuitar)a = 0;
	for(auto &a : colliCountBass)a = 0;
	for(auto &a : countAmountBass)a = 0;
	for(auto &a : colliCountDrum)a = 0;
	for(auto &a : countAmountDrum)a = 0;
	for(auto &a : *disiNotes)a.clear();
	firstLyric = 0;

	for(auto &a : fs::directory_iterator(songDir))
	{
		wstring ogg = cDir(a.path());
		if(ogg.substr(ogg.find_last_of('.') + 1) == L"ogg")
			if(ogg.substr(ogg.find_last_of('/') + 1, ogg.find_last_of('.') - ogg.find_last_of('/') - 1) != L"preview")
				AudioPlayer::createAudioStream(string(ogg.begin(), ogg.end()).c_str());
	}

	guitarTrackTmp->clear(),
		*guitarTrackTmp = *guitarTrack;
	bassTrackTmp->clear(),
		*bassTrackTmp = *bassTrack;
	drumTrackTmp->clear(),
		*drumTrackTmp = *drumTrack;
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
		for(auto &a : frameTimes)
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
	while((XinputManager::update(), !select && !deselect))
	{
		con->toConsoleBuffer(controles, con->getWidth() / 2 - controles.getWidth() / 2, con->getHeight() / 2 - controles.getHeight() / 2);
		int a = 0;
		con->toConsoleBuffer(notes[0][0], int(con->getWidth() * .1f) + 12 * a, con->getHeight() - 4, fretNoteColour[0][a + 1]);
		con->toConsoleBuffer(noteMsg[a], int(con->getWidth() * .1f) + 12 * a + notes[0][0].getWidth() / 2 - noteMsg[a].size() / 2, con->getHeight() - 3, fretNoteColour[0][a + 1]);
		con->toConsoleBuffer(noteKey[a], int(con->getWidth() * .1f) + 12 * a + notes[0][0].getWidth() / 2 - noteKey[a].size() / 2, con->getHeight() - 5, fretNoteColour[0][a + 1]);

		con->drawConsole();
	}
}

bool startScreen()
{
	int x = con->getWidth() / 2 - box->getWidth() / 2, y = con->getHeight() / 2 - (2 * (box->getHeight() / 2)),
		colours[] = {FG_WHITE,FG_GREEN | FG_INTENSIFY}, numBoxes = 2;
	bool exit = false, enter = false;
	wstring options[]{L"Single Play",L"Controles"}, noteMsg[]{L"Select"}, noteKey[]{L"A"};
	Sprite thing;
	AudioPlayer::createAudioStream("Game Files/A_rock_song_idea.mp3");
	AudioPlayer::play(true);

	while(!exit)
	{

		MouseInput::update();

		for(int a = 0; a < numBoxes; a++)
			if(box->mouseCollision({(short)x,short(y + a * 5 + 1)}, MouseInput::position))
			{
				create = a;
				if(MouseInput::stroke(LEFT_CLICK))
					enter = true;
				else
					select;//eat this input
			}

		//MouseInput::stroke(LEFT_CLICK);//eat this input
		x = con->getWidth() / 2 - (*box).getWidth() / 2, y = con->getHeight() / 2 - (2 * (box->getHeight() / 2));
		XinputManager::update();

		if(goup)
			AudioPlayer::createAudioStream("sfx/Guitar_Miss_2.wav"),
			AudioPlayer::play(),
			create--;
		if(godown)
			AudioPlayer::createAudioStream("sfx/Guitar_Miss_2.wav"),
			AudioPlayer::play(),
			create++;

		if(create >= numBoxes)
			create = 0;
		if(create < 0)
			create = numBoxes - 1;

		con->toConsoleBuffer((*logo)[0], con->getWidth() / 2 - (*logo)[0].getWidth() / 2, 10, FG_RED | FG_INTENSIFY);


		//draw selection boxes
		for(int a = 0; a < numBoxes; a++)
			con->toConsoleBuffer(*box, x, y + a * 5, colours[create == a]),
			con->toConsoleBuffer(options[a], x + (box->getWidth() / 2) - options[a].size() / 2, y + a * 5 + 2, colours[create == a]);

		for(int a = 0; a < 1; a++)
			con->toConsoleBuffer(notes[0][0], int(con->getWidth() * .1f) + 10 * a, con->getHeight() - 4, fretNoteColour[0][a]),
			con->toConsoleBuffer(noteMsg[a], int(con->getWidth() * .1f) + 10 * a + notes[0][0].getWidth() / 2 - noteMsg[a].size() / 2, con->getHeight() - 3, fretNoteColour[0][a]),
			con->toConsoleBuffer(noteKey[a], int(con->getWidth() * .1f) + 10 * a + notes[0][0].getWidth() / 2 - noteKey[a].size() / 2, con->getHeight() - 5, fretNoteColour[0][a]);


		con->drawConsole();

		deselect;//eat this input

		if(KeyInput::stroke(VK_ESCAPE) || XinputManager::getController(0)->isButtonStroked(GUITAR_INPUT_BUTTONS::GUITAR_BACK))
			return false;

		if(select || enter)
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
	AudioPlayer::init();
	logo->create("Game Files/Logo.txt");
	track->create("Game Files/Track.txt");
	notes->create("Game Files/Notes.txt");
	pauseMenu->create("Game Files/Pause Menu.txt");
	spacingScale /= 2;

	con->setConsoleSize((*track)[0].getWidth() * 2, int((*track)[0].getHeight()*.70));
	con->placeConsoleCenter();
	con->setResizable(true);

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

				centerTrack = instrument == "guitar" || instrument == "rhythm" ? con->getWidth() / 2 - (*track)[0].getWidth() / 2 : con->getWidth() / 2 - (*track)[1].getWidth() / 2;
				fretboardPosition = con->getHeight() - 7;
				//	fretboardXPosition = (centerTrack) + (a * 13 + 2)

				if(!create)
					playTrack();

				if(exitSelect)
					break;

				//#ifdef _DEBUG
				calculateFPS();
				wchar_t str[20];
				swprintf_s(str, L"%.2f", fps);
				con->toConsoleBuffer((wstring(L"fps: ") + str).c_str(), 0, 1);

				//#endif // _DEBUG

				con->drawConsole();
			}
			if(t1->joinable())
				t1->join();
			reset();
			for(auto&a : *guitarTrack)
				a.clear();
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