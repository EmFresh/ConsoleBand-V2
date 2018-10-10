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
using namespace std;
namespace fs = experimental::filesystem;
#pragma endregion

#pragma region Deffines

typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long long ullong;

#define select (KeyInput::stroke('A') || XinputManager::getController(0)->isButtonStroked(GUITAR_INPUT_BUTTONS::GUITAR_FRET_GREEN))
#define deselect (KeyInput::stroke('S') || XinputManager::getController(0)->isButtonStroked(GUITAR_INPUT_BUTTONS::GUITAR_FRET_RED))
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
vector<ushort>*fretColour = new vector<ushort> {FG_GREEN,FG_RED,FG_YELLOW,
FG_BLUE,FG_PURPLE}, *noteColour = new vector<ushort> {FG_GREEN | FG_INTENSIFY,FG_RED | FG_INTENSIFY,
FG_YELLOW | FG_INTENSIFY,FG_BLUE | FG_INTENSIFY,FG_PURPLE | FG_INTENSIFY};
vector <int> *lines = new vector <int>;
vector<vector<long>>*guitarTrack = new vector<vector<long>>(5), *guitarTrackTmp = new vector<vector<long>>(5);
vector<vector<pair<long, long>>>*vocalTrack = new vector<vector<pair<long, long>>>(5), *vocalTrackTmp = new vector<vector<pair<long, long>>>(5);

vector<vector<wstring>>* lyrics = new vector<vector<wstring>>;
vector<vector<pair<uint, uint>>>* lyricTiming = new vector<vector<pair<uint, uint>>>, *lyricTimingTmp = new vector<vector<pair<uint, uint>>>;

int trackSpeed[5], barCount, centerTrack, countAmount[5], notesHit, noteOffset = 0, fretOffset,
fretboardPosition, noteSpeed = 40, lastSpeed, spacingScale = lastSpeed = noteSpeed, create;
uint colliCount[5], firstLyric;

long incriment = -1;
float  fps = 0, speedPercent = 1;
short currentHealth = 0, totalHealth = 23;
bool pressed[5], start, paused(false), selected(false), exitSelect(false);

int songChoice;
string* songName = new string, songDir;
wstring *percentstr = new wstring;
ushort difficulty;
thread  *t1 = new thread;
EmGineAudioPlayer* sound = new EmGineAudioPlayer;
#pragma endregion

#pragma region Deffinitions
void saveSong();
void openSong(string);
bool collision();
bool noteDelete();
wstring cDir(wstring);
void reset();
void missFX(int seed);
void playButtonPress();
void drawLines();
void barLines();
bool invisable(int, int);
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
//void cleanUpFX();
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
	string track;
	for(auto&a : *guitarTrack)
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
				track = string(file[a][b].begin() + 3, file[a][b].end());
				if(track == "PART VOCALS")
					continue;
				if(track == "PART GUITAR")
					continue;

				break;
			}

			if(track == "PART VOCALS")
			{

				if(file[a][b].isText())
				{
					if(file[a][b][3] == '[')
						continue;

					if(lyricTiming->size())
					{

						uint timeInSec = file.getTimeInSeconds(a, b) * 1000;
						if(file[a][b].size() <= 3)
							continue;
						if((file[a][b][3] != '+' && []()->uint
						{
							int b = 0; for(auto&a : lyrics[0].back())b += a.size(); return b;
						}() + wstring(file[a][b].begin() + 3, file[a][b].end()).size() + 2 < lyricBox.getWidth()-1) &&
						   (timeInSec - lyricTiming[0].back().back().second < 750))

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
												lyricTiming[0][lyricTiming[0].size() - 2].back().second = file.getTimeInSeconds(a, b) * 1000;

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

							lyricTiming[0][lyricTiming[0].size() - 2].back().second = file.getTimeInSeconds(a, b) * 1000;

							lyrics->back().push_back(wstring(file[a][b].begin() + 3, file[a][b].end()));

							lyricTiming->back().push_back({file.getTimeInSeconds(a, b) * 1000,file.getTimeInSeconds(a, b) * 1000});
						}

					} else
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

						uint timeInSec = file.getTimeInSeconds(a, b) * 1000;
						if(file[a][b].size() <= 3)
							continue;
						if((file[a][b][3] != '+' && []()->uint
						{
							int b = 0; for(auto&a : lyrics[0].back())b += a.size(); return b;
						}() + wstring(file[a][b].begin() + 3, file[a][b].end()).size() + 2 < lyricBox.getWidth() - 1) &&
						   (timeInSec - lyricTiming[0].back().back().second < 750))

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
												lyricTiming[0][lyricTiming[0].size() - 2].back().second = file.getTimeInSeconds(a, b) * 1000;

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

							lyricTiming[0][lyricTiming[0].size() - 2].back().second = file.getTimeInSeconds(a, b) * 1000;

							lyrics->back().push_back(wstring(file[a][b].begin() + 3, file[a][b].end()));

							lyricTiming->back().push_back({file.getTimeInSeconds(a, b) * 1000,file.getTimeInSeconds(a, b) * 1000});
						}

					} else
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
					(*vocalTrack)[key][vocalTrack->at(key).size() - 1].second = file.getTimeInSeconds(a, b) * 1000;
					continue;
				}
				continue;
			}

			if(track == "PART GUITAR")
			{
				if(file[a][b].isNoteOn())
				{
					int key = file[a][b].getKeyNumber();
					if(key / 12 == difficulty)
						if(key % 12 < 5)
							(*guitarTrack)[key % 12].push_back(file.getTimeInSeconds(a, b) * -1000);

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
	lyricTiming->back().back().second += 750;

#pragma region Vocal Information
	for(int a = 0; a < vocalTrack->size();)
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

	vector<vector<long>> tmp(10);


	lyrics;
	lyricTiming;
	guitarTrack;
	sound->stopAll();
	for(auto &a : fs::directory_iterator(songDir))
	{
		wstring ogg = cDir(a.path());
		if(ogg.substr(ogg.find_last_of('.') + 1) == L"ogg")
			sound->createStream(string(ogg.begin(), ogg.end()).c_str());
	}

}
#pragma endregion	

#pragma region Play

#pragma region Other

void percent()
{
	char p2[8];
	sprintf_s(p2, "%.2f", abs((notesInSong(guitarTrack) - notesHit) / notesInSong(guitarTrack) * 100 - 100));
	string tmp = p2;
	tmp += '%';
	*percentstr = wstring(tmp.begin(), tmp.end());
}

float notesInSong(vector<vector<long>>*song)
{
	int count(0);
	for(auto &a : *song)
		count += a.size();
	return count;
}

void missFX(int seed)
{
	char* name = new char[255];
	for(int a = 0; a < sound->size(); a++)
	{
		sound->getAudio()->at(a)->getName(name, 255);
		if(strstr(name, "guitar"))
			sound->setVolume(0, a);
	}
	delete name;

	string miss[] {"sfx/Miss_1.wav","sfx/Miss_2.wav","sfx/Miss_3.wav","sfx/Miss_4.wav"};
	[](int, int)
	{}(5, 6);
	sound->createSound(miss[rand() % 4].c_str());
	sound->play();
	sound->setVolume(2);
}
#pragma endregion

#pragma region Note Alterations

bool collision()
{
	bool colli(false);

	for(int a = 0; a < guitarTrackTmp->size(); a++)
	{

		while(colliCount[a] < (*guitarTrackTmp)[a].size() &&
			(*guitarTrackTmp)[a][colliCount[a]] + noteOffset > fretboardPosition + 3)
			colliCount[a]++;

		if(pressed[a])
			if(colliCount[a] < (*guitarTrackTmp)[a].size())
				if((*guitarTrackTmp)[a][colliCount[a]] + noteOffset > fretboardPosition - 3)
					colli = true;
	}
	return colli;
}

bool noteDelete()
{
	short numPressed = 0;
	static vector<int> num;
	num.clear();


	for(int a = 0; a < guitarTrackTmp->size(); a++)
		//if(pressed[a])
		if(colliCount[a] < (*guitarTrackTmp)[a].size())
			if((*guitarTrackTmp)[a][colliCount[a]] + noteOffset > fretboardPosition - 3 && (*guitarTrackTmp)[a][colliCount[a]] + noteOffset <= fretboardPosition + 3)
				num.push_back(a);

	short lastOne = 0, count = 0;
	bool init = true;
	for(int a = 0; a < num.size(); a++)
	{
		if(init)
		{
			init = false;
			lastOne = a;
			continue;
		}

		if((*guitarTrackTmp)[num[lastOne]][colliCount[num[lastOne]]] != (*guitarTrackTmp)[num[a]][colliCount[num[a]]])
		{
			long check = max((*guitarTrackTmp)[num[lastOne]][colliCount[num[lastOne]]], (*guitarTrackTmp)[num[a]][colliCount[num[a]]]);

			if(check == (*guitarTrackTmp)[num[a]][colliCount[num[a]]])
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
			(*disiNotes)[num[0]].push_back(colliCount[num[0]]++);
	} else
	{
		int count = 0, sum = 0;
		for(int a = 0; a < 5; a++)
			if(sum += pressed[a], pressed[a])
				if(count < num.size())
				{
					if(num[count++] != a)
						return false;

				} else
					return false;


				if(sum != num.size())  
					return false;


				for(auto&a : num)
					notesHit++,
					(*disiNotes)[a].push_back(colliCount[a]++);
	}
	for(int a = 0; a < sound->size(); a++)
		sound->setVolume(1, a);
	if(currentHealth < totalHealth)
		currentHealth++;
	return true;
}

bool invisable(int fret, int check)
{
	for(int b = 0; b < (*disiNotes)[fret].size(); b++)
		if((*disiNotes)[fret][b] == check)
			return true;

	return false;
}
#pragma endregion

#pragma region Bar Lines

void drawLines()
{
	static int spacing = (float)(*notes)[0].getHeight() / ((float)spacingScale / (noteSpeed - (noteSpeed - (spacingScale * 2)) * 2));

	int barNum = 4;
	for(int a = 0; a < lines->size(); a++)
		if(incriment > 0)
			if(!((a + (barCount)) % barNum))
				con->toConsoleBuffer(L"━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━", centerTrack + 1, ((*lines)[a] + incriment / noteSpeed),
									 FG_WHITE | FG_INTENSIFY);
			else
				con->toConsoleBuffer(L"────────────────────────────────────────────────────────────────", centerTrack + 1, ((*lines)[a] + incriment / noteSpeed),
									 FG_WHITE);

	if(!paused)
	{
		//create new barlines
		while(((*lines)[lines->size() - 1] + incriment / noteSpeed) > spacing / 2)
			lines->push_back((*lines)[lines->size() - 1] - spacing);

		//adjust line spacing
		if(noteSpeed != lastSpeed)
		{
			for(int a = 0; a < lines->size(); a++)
				if(a > 0)
					lines[0][a] = lines[0][a - 1] - (float)(*notes)[0].getHeight() / ((float)spacingScale / (noteSpeed - (noteSpeed - (spacingScale * 2)) * 2));//apply spacing
				else if((incriment / noteSpeed - incriment / lastSpeed) < 0)
					lines[0][a] -= incriment / noteSpeed - incriment / lastSpeed;
				else
					lines[0][a] += incriment / lastSpeed - incriment / noteSpeed;



			lastSpeed = noteSpeed;
			spacing = (float)(*notes)[0].getHeight() / ((float)spacingScale / (noteSpeed - (noteSpeed - (spacingScale * 2)) * 2));
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
	if(guitarTrack->size() > 0)
	{
		if(sound->size())
			incriment = sound->getPosition(0);
		if(start)
		{
			start = false;
			int firstNote = 0;
			bool firstVal = false;
			for(auto&a : *guitarTrackTmp)
				if(a.size() > 0)
					if(!firstVal)
						firstNote = a[0],
						firstVal = true;
					else if(a[0] > firstNote)
						firstNote = a[0];

			firstNote = firstNote / noteSpeed + incriment / noteSpeed + fretboardPosition - 2;

			lines->clear(),
				lines->push_back(firstNote);
		}

		drawLines();
	} else
	{
		if(sound->size())
			incriment = sound->getPosition(0);
		drawLines();
	}
}
#pragma endregion

#pragma region Game Play

void playPauseMenu()
{
	wstring options[5] {L"Resume", L"Restart", L"Speed", L" Song Select", L" Start Screen"};
	static uint colours[] {FG_WHITE, FG_GREEN | FG_INTENSIFY, FG_GREEN};
	static short selection;

	const short optAmount = 5;

	sound->pauseAll();

	if(((KeyInput::press('S') || XinputManager::getController(0)->isButtonPressed(GUITAR_FRET_RED)) || guitarback) && !selected)
	{
		sound->playAll();
		paused = false;
		selection = 0;
		return;
	}
	paused = true;

	if((select ||
		(MouseInput::stroke(LEFT_CLICK) && [&]()->bool
	{
		for(uint a = 0; a < optAmount; a++)
		{
			if(Sprite({options[a]}).mouseCollision({short(con->getWidth() / 2 - ((options[a]).size() / 2)), short(ceil(con->getHeight() / 4.5f) + a * 2)}, MouseInput::position))
				return true;
		}return false;
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

		MouseInput::update();
		for(uint a = 0; a < optAmount; a++)
		{
			static Sprite tmp = (vector<wstring>{ options[a] });
			if(tmp.mouseCollision({short(con->getWidth() / 2 - ((options[a]).size() / 2)), short(ceil(con->getHeight() / 4.5f) + a * 2)}, MouseInput::position))
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
			//sound->setPosition(sound->getPosition(0) - 1000);
			sound->playAll();
			paused = false;
			return;
		case 1://Restart
			selected = !selected;
			selection = 0;
			std::swap(colours[1], colours[2]);
			reset();
			sound->playAll();
			paused = false;
			return;
		case 2://speed
			bool newSpeed;
			if((newSpeed = goup) || godown)
				if(((spacingScale * 2) * (speedPercent + (newSpeed ? .25 : -.25))) > 0 && ((float)(*notes)[0].getHeight() /
					((float)spacingScale / ((spacingScale * 2) * (speedPercent + (newSpeed ? .25 : -.25)) -
				   ((spacingScale * 2) * (speedPercent + (newSpeed ? .25 : -.25)) - spacingScale * 2) * 2))) > 0)

					noteSpeed = spacingScale * 2,
					noteSpeed *= (speedPercent += (newSpeed ? .25 : -.25));

			wchar_t percent[10];
			swprintf_s(percent, speedPercent != 1 ? L": %.2f%%" : L": normal", abs(2 - speedPercent));
			options[2] += percent;
			break;
		case 3://song select
			con->clearConsole();
			exitSelect = !createdSongList();
			selected = !selected;
			selection = 0;
			std::swap(colours[1], colours[2]);
			reset();
			sound->playAll();
			paused = false;
			return;
		case 4://start menu
			selected = !selected;
			selection = 0;
			std::swap(colours[1], colours[2]);
			paused = false;
			exitSelect = true;

			break;
		}

	con->toConsoleBuffer(*pauseMenu, con->getWidth() / 2 - (*pauseMenu).getWidth() / 2, con->getHeight() / 3 - (*pauseMenu).getHeight() / 2);

	for(uint a = 0; a < optAmount; a++)
		con->toConsoleBuffer(options[a], con->getWidth() / 2 - ((options[a]).size() / 2), ceil(con->getHeight() / 4.5f) + a * 2, colours[selection == a]);
}

void playSongMovement()
{

	//Note Movement
	for(short a = 0; a < guitarTrackTmp->size(); a++)
	{
		for(int b = countAmount[a]; b < (*guitarTrackTmp)[a].size(); b++)
		{

			// moves the song along
			(*guitarTrackTmp)[a][b] = (*guitarTrack)[a][b] / noteSpeed + incriment / noteSpeed + fretboardPosition;

			//stop checcking if nothing can be seen
			if((*guitarTrackTmp)[a][b] <= -3)
				break;

			if(b > countAmount[a])
			{
				//note dropoff
				if((*guitarTrackTmp)[a][b - 1] + noteOffset > con->getHeight())
					countAmount[a]++;

				if(((*guitarTrackTmp)[a][b - 1] >= (abs((*guitarTrackTmp)[a][b]) - abs((*guitarTrackTmp)[a][b - 1]))))//if the note below is off screen? (actually idk)
					if((*disiNotes)[a].size() > 0)
						if(b == (*disiNotes)[a][0])
							(*disiNotes)[a].erase((*disiNotes)[a].begin());

			}
			if(!invisable(a, b))
				if((*guitarTrackTmp)[a][b] + noteOffset > -3 && (*guitarTrackTmp)[a][b] + noteOffset < con->getHeight())
					if((*guitarTrackTmp)[a][b] + noteOffset < fretboardPosition + notes[0][0].getHeight())//draw notes above the fret board
					{
						///Normal
						con->toConsoleBuffer((*notes)[0], (centerTrack)+(a * 13 + 2), (*guitarTrackTmp)[a][b] + noteOffset, (*noteColour)[a]);

						///Colision Test
						//con->toConsoleBuffer((*notes)[0], (centerTrack) +(a * 13 + 2), (*songTemp)[a][b] + noteOffset,
						//					 b != colliCount[a] ? (*noteColour)[a] : FG_INTENSIFY | FG_WHITE);

					} else//draw notes below the fret board
						con->toConsoleBuffer((*notes)[0], (centerTrack)+(a * 13 + 2), (*guitarTrackTmp)[a][b] + noteOffset, FG_RED);
		}
	}

	static Sprite lyricBox("Game Files/Lyrics Box.txt");
	//const uint lyricSpeed = 40;
	//static long lyricPosition, lastLyricPosition;

	con->toConsoleBuffer(lyricBox, centerTrack, 0);

	//Lyrics Movement
	for(int a = firstLyric; a < lyrics->size(); a++)
	{
		if(incriment > lyricTiming[0][a].back().second)
		{
			firstLyric++;
			continue;
		}

		if(incriment > lyricTiming[0][a].front().first)
		{
			wstring phrase;
			for(auto &b : lyrics[0][a])
				phrase += b;

			1 > lyrics[0][a].size() - 1 ?
				con->toConsoleBuffer(lyrics[0][a][0], con->getWidth() / 2 - phrase.size() / 2, 2,
									 lyricTiming[0][a][0].first <= incriment ? FG_GREEN | FG_INTENSIFY : FG_WHITE | FG_INTENSIFY)
				:
				con->toConsoleBuffer(lyrics[0][a][0], con->getWidth() / 2 - phrase.size() / 2, 2,
									 lyricTiming[0][a][0].first <= incriment && lyricTiming[0][a][1].first > incriment ? FG_GREEN | FG_INTENSIFY : FG_WHITE | FG_INTENSIFY)
				;

			uint length = 0;
			for(int b = 1; b < lyrics[0][a].size(); b++)
				b + 1 > lyrics[0][a].size() - 1 ?
				con->toConsoleBuffer(lyrics[0][a][b], con->getWidth() / 2 - phrase.size() / 2 + (length += lyrics[0][a][b - 1].size()), 2,
									 lyricTiming[0][a][b].first <= incriment ? FG_GREEN | FG_INTENSIFY : FG_WHITE | FG_INTENSIFY)
				:
				con->toConsoleBuffer(lyrics[0][a][b], con->getWidth() / 2 - phrase.size() / 2 + (length += lyrics[0][a][b - 1].size()), 2,
									 lyricTiming[0][a][b].first <= incriment && lyricTiming[0][a][b + 1].first > incriment ? FG_GREEN | FG_INTENSIFY : FG_WHITE | FG_INTENSIFY)
				;
		}

		break;
		//	lastLyricPosition = lyricPosition;
	}

}

void playButtonPress()
{
	XinputManager::update();
	static char keyfrets[] {'A','S','D','F','G'};
	static uint guitarfrets[] {GUITAR_FRET_GREEN, GUITAR_FRET_RED, GUITAR_FRET_YELLOW, GUITAR_FRET_BLUE, GUITAR_FRET_ORANGE};
	static short lastStrum = -1, currentStrum = -1;
	static bool stroke;
	//Key Stroke
	if(KeyInput::stroke('R'))
		reset();

	if(paused || guitarstart)
		playPauseMenu();

	bool up = 0, down = 0;
//Strumming
	if(!paused)
	{
		KeyInput::press('Q');//eat this input;

		bool newSpeed;

		if((newSpeed = KeyInput::stroke(VK_NUMPAD4)) || KeyInput::stroke(VK_NUMPAD6))
			if(((spacingScale * 2) * (speedPercent + (newSpeed ? .25 : -.25))) > 0 && ((float)(*notes)[0].getHeight() /
				((float)spacingScale / ((spacingScale * 2) * (speedPercent + (newSpeed ? .25 : -.25)) -
			   ((spacingScale * 2) * (speedPercent + (newSpeed ? .25 : -.25)) - spacingScale * 2) * 2))) > 0)

				noteSpeed = spacingScale * 2,
				noteSpeed *= (speedPercent += (newSpeed ? .25 : -.25));

		//Note lights
		for(short a = 0; a < 5; a++)
		{
			if(KeyInput::release(keyfrets[a]) && XinputManager::getController(0)->isButtonReleased(guitarfrets[a]))
			{
				pressed[a] = false;
				if((*fretColour)[a] > 8)
					(*fretColour)[a] -= 8;
			} else if(KeyInput::press(keyfrets[a]) || XinputManager::getController(0)->isButtonPressed(guitarfrets[a]))
			{
				pressed[a] = true;
				if((*fretColour)[a] < 8)
					(*fretColour)[a] += 8;
			}
		}


		//Strum pressed
		if([&]()->bool
		{
			up = KeyInput::press(VK_UP) || XinputManager::getController(0)->isButtonPressed(GUITAR_STRUM_UP);
				down = KeyInput::press(VK_DOWN) || XinputManager::getController(0)->isButtonPressed(GUITAR_STRUM_DOWN);
				currentStrum = up ? (down ? 4 : 2) : (down ? 1 : -1);
				if (currentStrum==-1)
					lastStrum=-1;
				return (lastStrum != currentStrum) && currentStrum != -1;
		} ())
		{
			lastStrum = currentStrum;

			if(collision())
			{
				if(!noteDelete())
					currentHealth--,
					missFX(rand());
			} else
			{
				currentHealth--;
				missFX(rand());
			}
		}


	}
}

void playTrack()
{
	//con->mouseState();
	con->toConsoleBuffer((*track)[0], centerTrack, 0);
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
	for(short a = 0; a < 5; a++)
		con->toConsoleBuffer((*notes)[0], (centerTrack)+(a * 13 + 2), fretboardPosition, (*fretColour)[a]);

	ushort a;


	con->toConsoleBuffer(L"┏━┓", 0, con->getHeight() - totalHealth - 3);
	for(a = 2; a <= totalHealth + 2; a++)
		con->toConsoleBuffer(L"┃ ┃", 0, con->getHeight() - a);

	for(a = 2; a <= currentHealth + 2; a++)
		if(a == currentHealth && !((totalHealth - currentHealth) % 2))
			con->toConsoleBuffer(L"▄", 1, con->getHeight() - a, FG_GREEN);
		else
			con->toConsoleBuffer(L"█", 1, con->getHeight() - a, FG_GREEN);

	con->toConsoleBuffer(L"┗━┛", 0, con->getHeight() - 1);


}
#pragma endregion
#pragma endregion

#pragma region Song Selection

void songChoiceMovement(int size)
{

	static float delay = .2;
	static clock_t lastClockT;
	if(float(std::clock() - lastClockT) / CLOCKS_PER_SEC > delay)
	{
		lastClockT = std::clock();

		if(KeyInput::press(VK_DOWN) || XinputManager::getController(0)->isButtonPressed(GUITAR_INPUT_BUTTONS::GUITAR_STRUM_DOWN))

		{
			sound->setMasterVolume(1);
			if(songChoice < size - 1)
				songChoice++;
			else
				songChoice = 0;
			sound->createSound("sfx/Miss_2.wav");
			sound->setVolume(2);
			sound->play();
		} else if(KeyInput::press(VK_UP) || XinputManager::getController(0)->isButtonPressed(GUITAR_INPUT_BUTTONS::GUITAR_STRUM_UP))
		{
			sound->setMasterVolume(1);
			if(songChoice > 0)
				songChoice--;
			else
				songChoice = size - 1;
			sound->createSound("sfx/Miss_2.wav");
			sound->setVolume(2);
			sound->play();
		}
	}
	if(XinputManager::getController(0)->isButtonReleased(GUITAR_INPUT_BUTTONS::GUITAR_STRUM_DOWN) && KeyInput::release(VK_DOWN) &&
	   XinputManager::getController(0)->isButtonReleased(GUITAR_INPUT_BUTTONS::GUITAR_STRUM_UP) && KeyInput::release(VK_UP))
		lastClockT = std::clock() - (CLOCKS_PER_SEC*delay);
}

bool createdSongList()
{
	static wstring path = L"songs/";
	wstring p, playing, lastPlayed;
	vector<wstring>songs, songPath;
	wstring noteMsg[] {L"Select",L"Back"}, noteKey[] {L"A",L"S"};
	clock_t lastClockT = std::clock();
	static int count, movement, lastSongChoice;
	int colours[] = {FG_WHITE,FG_GREEN | FG_INTENSIFY}, sample, startHeight, endHeight, startWidth;
	unsigned long long lastClock = 0, clock = 0;

	while(true)
	{
		calculateFPS();
		wchar_t str[20];
		swprintf_s(str, L"%.2f", fps);
		con->toConsoleBuffer((wstring(L"fps: ") + str).c_str(), 0, 1);

		guitarstart;//eat this input;
		KeyInput::press('Q');//eat this input;
		XinputManager::update();
		bool pass;
		if(((pass = select) || MouseInput::stroke(LEFT_CLICK)))
		{
			if(pass || Sprite(vector<wstring> {songs[songChoice - movement]}).mouseCollision({short(startWidth - (songs[songChoice - movement].size() / 2)), short(startHeight + songChoice - movement)}, MouseInput::position))
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
				} else

					if((lastPlayed = L"", difficultyMenu()))
					{
						sound->getMasterChannelGroup()->getDSPClock(nullptr, &clock);
						sound->getMasterChannelGroup()->removeFadePoints(0, clock);
						sound->getMasterChannelGroup()->removeFadePoints(clock, clock + (sample * 25));
						sound->getMasterChannelGroup()->addFadePoint(0, 1);
						sound->stopAll();
						openSong(string(playing.begin(), playing.end()));
						return true;
					}

			}
		}if(deselect)
		{
			if(path == L"songs/")
			{
				sound->getMasterChannelGroup()->getDSPClock(nullptr, &clock);
				sound->getMasterChannelGroup()->removeFadePoints(0, clock);
				sound->getMasterChannelGroup()->removeFadePoints(clock, clock + (sample * 25));
				sound->getMasterChannelGroup()->addFadePoint(0, 1);
				sound->stopAll();
				return false;
			} else
			{
				songChoice = 0;
				path = path.substr(0, path.substr(0, path.size() - 1).find_last_of('/') + 1);
			}

		}

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
							sound->stopAll();
							uint start = 5000;
							for(auto &a : fs::directory_iterator(playing))
							{
								wstring ogg = cDir(a.path());
								if(ogg.substr(ogg.find_last_of('.') + 1) == L"ogg")
									sound->createStream(string(ogg.begin(), ogg.end()).c_str());

								if(ogg.substr(ogg.find_last_of('.') + 1) == L"ini")
								{
									FILE* f;
									fopen_s(&f, string(ogg.begin(), ogg.end()).c_str(), "r");

									char *str = new char[255];

									while(str = fgets(str, 255, f))
									{
										if(strstr(str, "preview_start_time"))
										{
											str[strlen(str) - 1] = (str[strlen(str) - 1] == '\n' ? '\0' : str[strlen(str) - 1]);
											start = stoi(strstr(str, "=") + 1);
											fclose(f);
											break;
										}
									}
									delete str;
									fclose(f);
								}
							}

							EmGineAudioPlayer::getAudioSystem()->getSoftwareFormat(&sample, nullptr, nullptr);
							sound->getMasterChannelGroup()->removeFadePoints(clock, clock + (sample * 25));

							sound->getMasterChannelGroup()->getDSPClock(nullptr, &clock);
							sound->getMasterChannelGroup()->addFadePoint(clock, 0);


							uint
								starting = start,
								ending = start + (25 * 1000),
								diff = ending - starting;
							sound->playAll(true, starting, ending, FMOD_TIMEUNIT_MS);

							sound->getMasterChannelGroup()->getDSPClock(nullptr, &clock);
							sound->getMasterChannelGroup()->addFadePoint(clock, 0);
							sound->getMasterChannelGroup()->addFadePoint(clock + (sample * 2), 1);
							sound->getMasterChannelGroup()->addFadePoint(clock + (sample * 23), 1);
							sound->getMasterChannelGroup()->addFadePoint(clock + (sample * 25), 0);
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
						char *str = new char[255];

						p = L"";
						while(str = fgets(str, 255, f))
						{
							if(strstr(str, "name"))
							{
								bool end = p.size();
								str[strlen(str) - 1] = '\0';
								string s = strstr(str, "=") + 1;
								p = wstring(s.begin(), s.end()) + L" -" + p;
								if(end)
									break;
							} else
								if(strstr(str, "artist"))
								{
									bool end = p.size();
									str[strlen(str) - 1] = '\0';
									string s = strstr(str, "=") + 1;
									p += wstring(s.begin(), s.end());
									if(end)
										break;
								}
						}
						delete str;
						fclose(f);
						songs.push_back(p);
						count++;
						break;
					}
			} else
			{
				songs.push_back(p.substr(p.find_last_of('/') + 1));
				count++;
			}
		}

		sound->getMasterChannelGroup()->getDSPClock(nullptr, &clock);
		EmGineAudioPlayer::getAudioSystem()->getSoftwareFormat(&sample, nullptr, nullptr);

		if(clock >= lastClock + (sample * 25))
		{
			sound->getMasterChannelGroup()->removeFadePoints(lastClock, clock);
			//sound->getMasterChannelGroup()->getDSPClock(nullptr, &clock);
			sound->getMasterChannelGroup()->addFadePoint(clock, 0);
			sound->getMasterChannelGroup()->addFadePoint(clock + (sample * 2), 1);
			sound->getMasterChannelGroup()->addFadePoint(clock + (sample * 23), 1);
			sound->getMasterChannelGroup()->addFadePoint(clock + (sample * 25), 0);
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
			movement = (count - 1) - ((endHeight - startHeight) / 2.f) - ((endHeight - startHeight) / 2.f);

		MouseInput::update();
		int tmp;
		if((tmp = MouseInput::position.Y - startHeight) >= 0 && MouseInput::position.Y >= startHeight && MouseInput::position.Y <= endHeight && tmp < songs.size())
			if(Sprite(vector<wstring> {songs[tmp]}).mouseCollision({short(startWidth - (songs[tmp].size() / 2)), short(startHeight + tmp)}, MouseInput::position))
				songChoice = MouseInput::position.Y - startHeight + movement;


		for(int a = 0; a < songs.size(); a++)
			con->toConsoleBuffer(songs[a], startWidth - (songs[a].size() / 2), startHeight + a, (songChoice - movement == a ? colours[1] : colours[0]));


		con->toConsoleBuffer(L"----------------------------", startWidth - 14, endHeight + 1);
		con->toConsoleBuffer(L"Song List", startWidth - 4, startHeight - 2);
		con->toConsoleBuffer(L"----------------------------", startWidth - 14, startHeight - 1);

		for(int a = 0; a < 2; a++)
			con->toConsoleBuffer(notes[0][0], con->getWidth() * .1f + 12 * a, con->getHeight() - 4, noteColour[0][a]),
			con->toConsoleBuffer(noteMsg[a], con->getWidth() * .1f + 12 * a + notes[0][0].getWidth() / 2 - noteMsg[a].size() / 2, con->getHeight() - 3, noteColour[0][a]),
			con->toConsoleBuffer(noteKey[a], con->getWidth() * .1f + 12 * a + notes[0][0].getWidth() / 2 - noteKey[a].size() / 2, con->getHeight() - 5, noteColour[0][a]);

		if(songChoice - movement >= 0 && songChoice - movement < songs.size())
			*songName = string(songs[songChoice - movement].begin(), songs[songChoice - movement].end());
		con->drawConsole();

	}

	return true;
}
#pragma endregion

#pragma region Other

void reset()
{
	currentHealth = (totalHealth / 2) + 1;
	for(auto &a : colliCount)a = 0;
	for(auto &a : countAmount)a = 0;
	for(auto &a : *disiNotes)a.clear();
	firstLyric = 0;

	sound->stopAll();
	for(auto &a : fs::directory_iterator(songDir))
	{
		wstring ogg = cDir(a.path());
		if(ogg.substr(ogg.find_last_of('.') + 1) == L"ogg")
			sound->createStream(string(ogg.begin(), ogg.end()).c_str());
	}
	sound->playAll();
	guitarTrackTmp->clear(),
		*guitarTrackTmp = *guitarTrack;
	barCount =
		notesHit = 0;
	if(sound->size())
		incriment = sound->getPosition(0);
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
	controles.create("Game Files/Controles.txt");
	while((XinputManager::update(), !select && !deselect))
		con->toConsoleBuffer(controles, con->getWidth() / 2 - controles.getWidth() / 2, con->getHeight() / 2 - controles.getHeight() / 2),
		con->drawConsole();
}

bool difficultyMenu()
{
	static int counter = 0;
	ushort numBoxes = 4, x, y, colours[] {FG_WHITE,FG_GREEN | FG_INTENSIFY};
	wstring options[] {L"Easy",L"Medium",L"Hard",L"Expert"};
	wstring noteMsg[] {L"Select",L"Back"}, noteKey[] {L"A",L"S"};
	ullong clock;
	int sample;
	sound->getMasterChannelGroup()->getDSPClock(nullptr, &clock);
	EmGineAudioPlayer::getAudioSystem()->getSoftwareFormat(&sample, nullptr, nullptr);

	sound->getMasterChannelGroup()->getDSPClock(nullptr, &clock);
	sound->getMasterChannelGroup()->removeFadePoints(0, clock);
	sound->getMasterChannelGroup()->removeFadePoints(clock, clock + (sample * 25));
	sound->getMasterChannelGroup()->addFadePoint(0, 1);
	sound->stopAll();

	sound->createStream("Game Files/A_rock_song_idea.mp3");
	sound->play(true);

	while(true)
	{
		XinputManager::update();

		KeyInput::press(VK_RETURN);//eat this input;
		if(deselect)
			return false;
		bool check;
		if(check = select || MouseInput::stroke(LEFT_CLICK))
			for(int a = 0; a < numBoxes; a++)
				if(check || box->mouseCollision({(short)x, short(y + a * 5 + 1)}, MouseInput::position))
					return true;

		x = con->getWidth() / 2 - ceil((float)box->getWidth() / 2), y = con->getHeight() / 2 - 5;
		if(goup)
			sound->createSound("sfx/Miss_2.wav"),
			sound->play(),
			counter--;
		if(godown)
			sound->createSound("sfx/Miss_2.wav"),
			sound->play(),
			counter++;

		if(counter >= numBoxes)
			counter = 0;
		if(counter < 0)
			counter = numBoxes - 1;

		MouseInput::update();
		for(int a = 0; a < numBoxes; a++)
			if(box->mouseCollision({(short)x, short(y + a * 5 + 1)}, MouseInput::position))
				counter = a;

		con->toConsoleBuffer(L"Difficulty", con->getWidth() / 2 - 5, y - 1);
		con->toConsoleBuffer(L"-----------------------------------", con->getWidth() / 2 - 18, y);

		//draw selection boxes
		for(int a = 0; a < numBoxes; a++)
			con->toConsoleBuffer(*box, x, y + a * 5 + 1, colours[counter == a]),
			con->toConsoleBuffer(options[a], x + (box->getWidth() / 2) - options[a].size() / 2, y + a * 5 + 3, colours[counter == a]);

		for(int a = 0; a < 2; a++)
			con->toConsoleBuffer(notes[0][0], con->getWidth() * .1f + 12 * a, con->getHeight() - 4, noteColour[0][a]),
			con->toConsoleBuffer(noteMsg[a], con->getWidth() * .1f + 12 * a + notes[0][0].getWidth() / 2 - noteMsg[a].size() / 2, con->getHeight() - 3, noteColour[0][a]),
			con->toConsoleBuffer(noteKey[a], con->getWidth() * .1f + 12 * a + notes[0][0].getWidth() / 2 - noteKey[a].size() / 2, con->getHeight() - 5, noteColour[0][a]);

		con->drawConsole();
		switch(counter)
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

bool startScreen()
{
	int x = con->getWidth() / 2 - box->getWidth() / 2, y = con->getHeight() / 2 - (2 * (box->getHeight() / 2)),
		colours[] = {FG_WHITE,FG_GREEN | FG_INTENSIFY}, numBoxes = 2;
	bool exit = false, enter = false;
	wstring options[] {L"Single Play",L"Controles"}, noteMsg[] {L"Select"}, noteKey[] {L"A"};
	Sprite thing;
	sound->createStream("Game Files/A_rock_song_idea.mp3");
	sound->play(true);

	while(!exit)
	{
		MouseInput::update();

		for(int a = 0; a < numBoxes; a++)
			if(box->mouseCollision({(short)x,short(y + a * 5 + 1)}, MouseInput::position))
			{
				create = a;
				if(MouseInput::stroke(LEFT_CLICK))
					enter = true;
			}

		MouseInput::stroke(LEFT_CLICK);//eat this input
		x = con->getWidth() / 2 - (*box).getWidth() / 2, y = con->getHeight() / 2 - (2 * (box->getHeight() / 2));
		XinputManager::update();

		if(goup)
			sound->createSound("sfx/Miss_2.wav"),
			sound->play(),
			create--;
		if(godown)
			sound->createSound("sfx/Miss_2.wav"),
			sound->play(),
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
			con->toConsoleBuffer(notes[0][0], con->getWidth() * .1f + 10 * a, con->getHeight() - 4, noteColour[0][a]),
			con->toConsoleBuffer(noteMsg[a], con->getWidth() * .1f + 10 * a + notes[0][0].getWidth() / 2 - noteMsg[a].size() / 2, con->getHeight() - 3, noteColour[0][a]),
			con->toConsoleBuffer(noteKey[a], con->getWidth() * .1f + 10 * a + notes[0][0].getWidth() / 2 - noteKey[a].size() / 2, con->getHeight() - 5, noteColour[0][a]);


		con->drawConsole();

		deselect;//eat this input

		if(KeyInput::stroke('Q') || XinputManager::getController(0)->isButtonStroked(GUITAR_INPUT_BUTTONS::GUITAR_BACK))
			return false;

		if(select || enter)
			switch(create)
			{
			case 0:
				exit = createdSongList();
				if(!exit)
					sound->createStream("Game Files/A_rock_song_idea.mp3"),
					sound->play(true);
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

void main()
{
	logo->create("Game Files/Logo.txt");
	track->create("Game Files/Track.txt");
	notes->create("Game Files/Notes.txt");
	pauseMenu->create("Game Files/Pause Menu.txt");
	spacingScale /= 2;

	con->setConsoleSize((*track)[0].getWidth() * 2, (*track)[0].getHeight()*.70);
	con->placeConsoleCenter();
	con->setResizable(true);

	std::vector<std::wstring> boxy = {

		L"┌───────────────┐",
		L"│               │",
		L"│               │",
		L"│               │",
		L"└───────────────┘"
	};

	box->create(&boxy);

	while(true)
	{
		if(startScreen())
		{

			while(true)
			{

				centerTrack = con->getWidth() / 2 - (*track)[0].getWidth() / 2;
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
			sound->stopAll();
		} else
			break;
		exitSelect = false;
		noteSpeed = spacingScale * 2,
			speedPercent = 1;
	}
	return;
}