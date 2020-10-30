/*************************************************
**Name     : Emory Wynn							**
**Student# : 100655604							**
**************************************************/

#pragma once
#include <Windows.h>
#include <fstream>
#include <thread>
#include <string>
#include <vector>
#include <map>
#include "Utilities.h"

//#define newstr new char
//typedef unsigned short ushort;

/*Enums*/
/*
summary:
These are coulers that can be used for text coulour manipulation.

* FG_COLOUR - change the text colour
* BG_COLOUR - Change the colour behind each character of text

Usage:
to use these colours in toConsoleBuffer();, use eather  FG_COLOUR or BG_COLOUR for colour manipulation.
to combine colours or set a FG_COLOUR and BG_COLOUR at the same time, use the
"|" operator (i.e. FG_COLOUR | BG_COLOUR ) for more combonations.
*/
enum Colour
{

	FG_BLUE = FOREGROUND_BLUE,
	FG_GREEN = FOREGROUND_GREEN,
	FG_CYAN = FOREGROUND_BLUE | FOREGROUND_GREEN,
	FG_RED = FOREGROUND_RED,
	FG_PURPLE = FOREGROUND_BLUE | FOREGROUND_RED,
	FG_YELLOW = FOREGROUND_GREEN | FOREGROUND_RED,
	FG_WHITE = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED,
	FG_INTENSIFY = FOREGROUND_INTENSITY,
	//FG_UNDERLINE = UNDERLINE,

	BG_BLUE = BACKGROUND_BLUE,
	BG_GREEN = BACKGROUND_GREEN,
	BG_CYAN = BACKGROUND_BLUE | BACKGROUND_GREEN,
	BG_RED = BACKGROUND_RED,
	BG_PURPLE = BACKGROUND_BLUE | BACKGROUND_RED,
	BG_YELLOW = BACKGROUND_GREEN | BACKGROUND_RED,
	BG_WHITE = BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED,
	BG_INTENSIFY = BACKGROUND_INTENSITY

};

enum MouseButtons
{
	LEFT_CLICK = VK_LBUTTON,
	RIGHT_CLICK = VK_RBUTTON,
	MIDDLE_CLICK = VK_MBUTTON,
	UNKNOWN_CLICK_1 = VK_XBUTTON1,
	UNKNOWN_CLICK_2 = VK_XBUTTON2
};

/*Structs*/
struct Sprite
{
	Sprite() {}

	Sprite(const Sprite& sprite, const char* tag = nullptr):
		m_sprite(sprite.m_sprite), m_pos(sprite.m_pos), m_colour(sprite.m_colour),
		m_width(sprite.m_width), m_height(sprite.m_height)
	{

		if(tag)
			m_tag = tag;

	}

	Sprite(const char* file, char* tag = nullptr)
	{
		create(file, tag);
	}
	Sprite(std::vector<std::wstring>* sprite, char* tag = nullptr)
	{
		create(sprite, tag);
	}
	Sprite(std::vector<std::wstring> sprite, char* tag = nullptr)
	{
		create(&sprite, tag);
	}

	/*
	Takes a .txt file and outputs into the Sprite.
	The sprite will be overridden once this is called
	*/
	virtual void create(const char* file, char* tag = nullptr)
	{
		setTag(tag);

		m_sprite.clear();

		m_height = m_width = 0;

	#pragma region C-style
		//FILE* f;
		//fopen_s(&f, file, "r, ccs=UNICODE");
		//wchar_t* str = new wchar_t[255];
		//
		//	while(str = fgetws(str, 255, f),
		//		  m_sprite.push_back((str == nullptr ? L"" : (str[wcslen(str) - 1] = (str[wcslen(str) - 1] == '\n' ? '\0' : str[wcslen(str) - 1]), str))), str != nullptr)
		//		m_width = m_width < (ushort)(m_sprite[m_height]).size() ? (ushort)(m_sprite[m_height]).size() : m_width,
		//		m_height++;
		//
		//	m_sprite.pop_back();
		//	if(f)
		//		fclose(f);
		//delete[] str;
	#pragma endregion

	#pragma region modern

		std::wfstream unicodeFile(file, std::ios::in);
		std::wstring line;

		for(int a = 0; getline(unicodeFile, line); a++)
			m_sprite.push_back(line);

		unicodeFile.close();

	#pragma endregion

	}
	virtual void create(const std::vector<std::wstring>* sprite, char* tag = nullptr)
	{
		setTag(tag);

		m_sprite.clear();
		m_height = (ushort)sprite->size();
		m_sprite = *sprite;
		for(int a = 0; a < m_height; a++)
			m_width = m_width < (ushort)m_sprite[a].size() ? (ushort)m_sprite[a].size() : m_width;
	}

	virtual void toBuffer(ushort x, ushort y);
	virtual void toBufferNS(ushort x, ushort y);

	void render();

	/*

	************
	  ********     *******
		****
		 **

	*/
	bool boxCollision(Sprite s2, Coord2D<> p1, Coord2D<> p2)
	{
		p1 = {p1.x + short(m_width * .5f) ,p1.y + short(m_height * .5f)};
		p2 = {p2.x + short(s2.m_width * .5f), p2.y + short(s2.m_height * .5f)};

		if(abs(p1.x - p2.x) < (m_width + s2.m_width) * .5f)
			if(abs(p1.y - p2.y) < (m_height + s2.m_height) * .5f)
				return true;
		return false;
	}

	bool boxCollision(Sprite s2)
	{
		Coord2D<> p1 = {m_pos.x  ,m_pos.y};
		Coord2D<> p2 = {s2.m_pos.x  ,s2.m_pos.y};
		return boxCollision(s2, p1, p2);
	}

	bool mouseCollision(Coord2D<> p1, Coord2D<> mouse)
	{
		p1 = {p1.x + short(m_width * .5f), p1.y + short(m_height * .5f)};

		if(abs(p1.x - mouse.x) <= m_width * .5f)
			if(abs(p1.y - mouse.y) <= m_height * .5f)
				return true;
		return false;
	}
	bool mouseCollision(Coord2D<> mouse)
	{
		Coord2D<> p1 = {m_pos.x + m_width, m_pos.y + m_height};

		return mouseCollision(p1, mouse);
	}

	void setTag(const char* tag)
	{
		m_tag = tag;
	}

	const char* getTag()
	{
		return m_tag;
	}

	void setColour(int colour)
	{
		m_colour = colour;
	}
	void setPosition(Coord2D<> pos) { m_pos = pos; }
	void setX(float posx) { m_pos.x = posx; }
	void setY(float posy) { m_pos.y = posy; }


	int getColour() { return m_colour; }

	std::wstring* getSprite()
	{
		return m_sprite.data();
	}

	ushort getWidth()
	{
		return m_width;
	}

	ushort getHeight()
	{
		return m_height;
	}

	Coord2D<> getPosition() { return m_pos; }
	Coord2D<> getCenter() { return m_pos + Coord2D<>{(float)m_width, (float)m_height} *.5f; }

	float getX() { return m_pos.x; }
	float getY() { return m_pos.y; }

	void operator=(Sprite sprite)
	{
		m_sprite = (sprite.m_sprite);
		m_tag = (sprite.m_tag);
		m_width = (sprite.m_width);
		m_height = (sprite.m_height);
		m_colour = (sprite.m_colour);
	}

protected:
	std::vector<std::wstring> m_sprite;
	Coord2D<> m_pos{0,0};
	const char* m_tag = nullptr;
	ushort m_width = 0, m_height = 0;
	int m_colour = FG_WHITE;
};

struct SpriteSheet
{
	SpriteSheet() {}
	/*
	Creates SpriteSheet from another spritesheet. Creates a new
	Sprite when it finds a line in file with a lenght of zero
	*/
	SpriteSheet(const SpriteSheet& cpy)
	{
		m_sheet->clear();
		for(auto& a : *cpy.m_sheet)
			m_sheet->push_back(new Sprite(*a));
	}
	/*
	Creates SpriteSheet from a .txt file. Creates a new
	Sprite when it finds a line in file with specified split character
	*/
	SpriteSheet(const char* file, const wchar_t* split = L"")
	{
		create(file, split);
	}

	~SpriteSheet()
	{
		clear();
		delete m_sheet;
	}

	/*
	Creates SpriteSheet from a .txt file. Creates a new
	Sprite when it finds a line with specified split character
	*/
	void create(const char* file, const wchar_t* split = L"")
	{
		FILE* f;

		wchar_t* str2 = new wchar_t[255];

		std::vector<std::wstring> sprite;
		unsigned short width = 0, height = 0;
		bool seg = 0;
		std::wstring str;

		fopen_s(&f, file, "r, ccs=UNICODE");

		while(str2 = fgetws(str2, 255, f),
			  str = (str2 == nullptr ? L"" :
			  (str2[wcslen(str2) - 1] = (str2[wcslen(str2) - 1] == '\n' ? '\0' : str2[wcslen(str2) - 1]), str2)),
			  str2 != nullptr)
		{
			if(str == split)
			{
				if(!seg)
				{
					add(&sprite);

					width = 0, height = 0;
					sprite.clear();
				}
				seg = true;
				continue;
			}

			width = width < (ushort)str.size() ? (ushort)str.size() : width;
			sprite.push_back(str);
			height++;
			seg = false;
		}
		fclose(f);

		if(height > 0)
			add(&sprite);

		delete[] str2;
	}

	void add(Sprite sprite) { m_sheet->push_back(new Sprite(sprite)); }
	void add(std::vector<std::wstring>* sprite)
	{
		Sprite* tmp = new Sprite;
		tmp->create(sprite);
		m_sheet->push_back(tmp);

	}

	void remove(int index)
	{
		delete m_sheet[0][index];
		m_sheet->erase(m_sheet->begin() + index);
	}
	void remove(const char* tag)
	{
		for(int a = 0; a < size(); a++)
			if(m_sheet[0][a]->getTag() == tag)
			{
				remove(a);
				return;
			}
	}

	void clear()
	{
		for(auto& a : *m_sheet)
			delete a;
		m_sheet->clear();
	}

	int size()
	{
		return m_sheet->size();
	}

	Sprite& at(unsigned int index)
	{
		return operator[](index);
	}
	Sprite& at(const char* tag)
	{
		for(int a = 0; a < size(); a++)
			if(m_sheet[0][a]->getTag() == tag)
				return *m_sheet[0][a];
		return *m_sheet[0][size()];
	}
	Sprite& operator[](unsigned int index)
	{
		return *m_sheet[0][index];
	}

	std::vector<Sprite*>::iterator begin() { return m_sheet->begin(); }
	std::vector<Sprite*>::iterator end() { return m_sheet->end(); }
	std::vector<Sprite*>::reverse_iterator rbegin() { return m_sheet->rbegin(); }
	std::vector<Sprite*>::reverse_iterator rend() { return m_sheet->rend(); }

protected:
	Coord2D<> m_pos;
	int m_colour = FG_WHITE;
	std::vector<Sprite*>* m_sheet = new std::vector<Sprite*>;
};

struct Animation:public SpriteSheet
{
	Animation():SpriteSheet() {}
	Animation(const SpriteSheet& cpy):SpriteSheet(cpy) { setCurrentFrame(0); }

	void update()
	{
		float time = (float)clock() / CLOCKS_PER_SEC;
		m_pos = at(m_currentFrame).getPosition();
		m_colour = at(m_currentFrame).getColour();

		if(!m_init)
		{
			m_lastTime = time;
			m_init = true;
		}

		if(!m_pause && !m_stop)
		{
			if((time = (time - m_lastTime)) >= m_speed)
			{
				if(m_repeat)
					m_currentFrame = int(time / m_speed) % size();
				else
				{
					m_currentFrame = int(time / m_speed);
					m_currentFrame = m_currentFrame >= size() - 1 ? unsigned((size() - 2) % size()) : m_currentFrame;
				}
			}
		}
		else
		{
			m_lastTime = time;

			if(m_stop)
				reset();
		}
		at(m_currentFrame).setPosition(m_pos);
		at(m_currentFrame).setColour(m_colour);
	}

	Sprite& getSprite()
	{
		return at(m_currentFrame);
	}

	void setRepeat(bool repeat)
	{
		m_repeat = repeat;
	}

	void setPause(bool pause)
	{
		m_pause = pause;
	}

	void setStop(bool stop)
	{
		m_stop = stop;
	}

	bool isRepeat()
	{
		return m_repeat;
	}

	bool isPaused()
	{
		return m_pause;
	}

	bool isStop()
	{
		return m_stop;
	}

	void reset()
	{

		m_currentFrame = 0;
	}

	void setSpeed(float fps)
	{
		m_speed = 1 / fps;
	}

	void setCurrentFrame(ushort frame)
	{
		if(!size())return;
		m_currentFrame = frame;
	}

private:
	ushort  m_currentFrame = 0;
	float m_speed = 0, m_lastTime = 0;
	bool m_init = false, m_pause = false, m_stop = false, m_repeat = true;
};

struct MouseInput
{

	static bool doubleClick;
	static short vertWheel, horiWheel;
	static COORD position;

	static bool pressed(MouseButtons button)
	{
		return GetAsyncKeyState(button);
	}

	static bool released(MouseButtons button)
	{
		return	!GetAsyncKeyState(button);
	}

	static bool stroke(MouseButtons button)
	{
		if(GetAsyncKeyState(button))
			buttons[button] = true;
		if(!GetAsyncKeyState(button) && buttons[button])
			return (buttons[button] = false, true);

		return false;
	}

	static void update()
	{

		DWORD numEvents;
		INPUT_RECORD irBuff[128];

		GetNumberOfConsoleInputEvents(GetStdHandle(STD_INPUT_HANDLE), &numEvents);
		if(numEvents)
			ReadConsoleInputA(
			GetStdHandle(STD_INPUT_HANDLE),      // input buffer handle 
			irBuff,      // buffer to read into 
			128,         // size of read buffer 
			&numEvents);

		for(UINT a = 0; a < numEvents; a++)
		{
			switch(irBuff[a].EventType)
			{
			case MOUSE_EVENT: // mouse input 
				MOUSE_EVENT_RECORD mer = irBuff[a].Event.MouseEvent;
				switch(mer.dwEventFlags)
				{
				case DOUBLE_CLICK:
					doubleClick = true;
					break;

				case MOUSE_MOVED:
					position = {mer.dwMousePosition.X, mer.dwMousePosition.Y};
					break;
				case MOUSE_WHEELED://vertical
					vertWheel += (int)mer.dwButtonState / std::abs((int)mer.dwButtonState);
					break;
				case MOUSE_HWHEELED://horizontal
					horiWheel += (int)mer.dwButtonState / std::abs((int)mer.dwButtonState);
					break;
				}

			}
		}



	}

private:
	static std::map <short, bool> buttons;
};


/*Classes*/
class EmConsole
{
public:
	/***Constructers***/

	EmConsole() = delete;

	static void init();

	static void setFullScreen(bool);
	static bool getFullScreen();

	/***Functions***/

	//sends a text file to a vector of vector wstring 
	static void textFileToVector(std::string file, std::vector<std::vector<std::wstring>>& str);

	//sends a text file to a vector of vector wstring
	static std::vector<std::vector<std::wstring>> textFileToVector(std::string file);

	//sets the title
	static void setTitle(std::string title);

	//sets the console size
	static void setConsoleSize(ushort x, ushort y);

	//returns
	static COORD getConsoleSize();

	static int getWidth();

	static int getHeight();

	void printf(const char*...);

	//sets weather or not the window can be resized
	static void setResizeable(bool resz);
	static bool getResizeable();

	//sets position of the console from the top left corner
	static void setConsolePosition(int x, int y);

	static void placeConsoleCenter();

	static void consoleCursorPosition(int x, int y);


#pragma region Console Reading

	/* Start: special stuff for tetris (don't even try to understand)*/
	static std::vector<int> readConsoleLineAtributes(int x, int y, float width);

	static std::vector<std::vector<int>> readConsoleAreaAtributes(int x, int y, float width, float height);

	static std::vector<std::wstring> readConsoleArea(int x, int y, float width, float height);

	static std::wstring readConsoleLine(int x, int y, float width);

	static char readConsoleCharacter(int x, int y);

	static char readActiveConsoleCharacter(int x, int y);
	/* End: special stuff for tetris (don't even try to understand)*/
#pragma endregion

	MouseInput mouseState()
	{
		static MouseInput input;
		input.update();
		return input;
	}


#pragma region Console buffer overloads

	/*Console buffer overloads*/

	/*
	Note: NS means No Spaces, writing only characters excluding spaces.
	Extremely inefficient
	*/

	/*
	toConsoleBuffer(wstring &str, float poX, float poY, int x, int y, Colour colour);
	* str    - wstring to be drawn to buffer
	* poX    - the x coord of origin position of the console (where you consider (0,0) to be)
	* poY    - the y coord of origin position of the console (where you consider (0,0) to be)
	* x      - x position from the origin poX coordinate on the screen
	* y      - y position from the origin poY coordinate on the screen
	* colour - colour of text

	summary:
	Adds single line to the console to the
	specified (x,y) position from the origin (poX, poY)
	to be drawn after drawConsole(); is called
	(with text colour modification [see: enum colour in EmConsole.h])
	*/
	static void toConsoleBufferNS(std::wstring& str, float& poX, float& poY, int x, int y, int colour);

	/*
	toConsoleBuffer(wstring &str, float poX, float poY, int x, int y);
	* str    - wstring to be drawn to buffer
	* poX    - the x coord of origin position of the console (where you consoder (0,0) to be)
	* poY    - the y coord of origin position of the console (where you consoder (0,0) to be)
	* x      - x position from the origin poX coordinate on the screen
	* y      - y position from the origin poY coordinate on the screen

	summary:
	Adds single line to the console to the
	specified (x,y) position from the origin (poX, poY)
	to be drawn after drawConsole(); is called
	*/
	static void toConsoleBufferNS(std::wstring& str, float& poX, float& poY, int x, int y);

	/*
	toConsoleBuffer(wstring &str, int x, int y, int colour);
	* str    - wstring to be drawn to buffer
	* x      - x position from the top left corner of the screen
	* y      - y position from the top left corner of the screen
	* colour - colour of text

	summary:
	Adds single line to the console to the
	specified (x,y) position from the top left corner
	to be drawn after drawConsole(); is called
	(with text colour modification [see: enum colour in EmConsole.h])
	*/
	static void toConsoleBufferNS(std::wstring& str, int x, int y, int colour);

	/*
	toConsoleBuffer(wstring &str, int x, int y);
	* str    - wstring to be drawn
	* x      - x position from the top left corner of the screen
	* y      - y position from the top left corner of the screen

	summary:
	Adds single line to the console to the
	specified (x,y) position from the top left corner
	to be drawn after drawConsole(); is called
	*/
	static void toConsoleBufferNS(std::wstring& str, int x, int y);

	/*
	toConsoleBuffer(vector<wstring>& str, float poX, float poY, int x, int y, int colour);
	* str    - vector of wstring to be drawn to buffer
	* poX    - the x coord of origin position of the console (where you consoder (0,0) to be)
	* poY    - the y coord of origin position of the console (where you consoder (0,0) to be)
	* x      - x position from the origin poX coordinate on the screen
	* y      - y position from the origin poY coordinate on the screen
	* colour - colour of text

	summary:
	Adds multiple lines to the console starting at the
	specified (x,y) position from the specified origin (poX, poY)
	to be drawn after drawConsole(); is called (with text colour modification [see: enum colour in EmConsole.h])
	*/
	static void toConsoleBufferNS(Sprite& str, float& poX, float& poY, int x, int y, int colour);

	/*
	toConsoleBuffer(vector<wstring>& str, float poX, float poY, int x, int y);
	* str    - vector of wstring to be drawn to buffer
	* poX    - the x coord of origin position of the console (where you consoder (0,0) to be)
	* poY    - the y coord of origin position of the console (where you consoder (0,0) to be)
	* x      - x position from the origin poX coordinate on the screen
	* y      - y position from the origin poY coordinate on the screen

	summary:
	Adds multiple lines to the console starting at the
	specified (x,y) position from the origin (poX, poY)
	to be drawn after drawConsole(); is called
	*/
	static void toConsoleBufferNS(Sprite& str, float& poX, float& poY, int x, int y);

	/*
	toConsoleBuffer(vector<wstring>& str, int x, int y, int colour);
	* str    - vector of wstring to be drawn to buffer
	* x      - x position from the top left corner of the screen
	* y      - y position from the top left corner of the screen
	* colour - colour of text

	summary:
	Adds multiple lines to the console starting at the
	specified (x,y) position from the top left corner
	to be drawn after drawConsole(); is called
	(with text colour modification [see: enum colour in EmConsole.h])
	*/
	static void toConsoleBufferNS(Sprite& str, int x, int y, int colour);

	/*
	toConsoleBuffer(vector<wstring>& str, int x, int y);
	* str    - vector of wstring to be drawn to buffer
	* x      - x position from the top left corner of the screen
	* y      - y position from the top left corner of the screen

	summary:
	Adds multiple lines to the console starting at the
	specified (x,y) position from the top left corner
	to be drawn after drawConsole(); is called
	*/
	static void toConsoleBufferNS(Sprite& str, int x, int y);

	/*
	toConsoleBuffer(const char* str, float poX, float poY, int x, int y, int colour);
	* str    - wstring to be drawn to buffer
	* poX    - the x coord of origin position of the console (where you consoder (0,0) to be)
	* poY    - the y coord of origin position of the console (where you consoder (0,0) to be)
	* x      - x position from the origin poX coordinate on the screen
	* y      - y position from the origin poY coordinate on the screen
	* colour - colour of text

	summary:
	Adds single line to the console to the
	specified (x,y) position from the origin (poX, poY)
	to be drawn after drawConsole(); is called
	(with text colour modification [see: enum colour in EmConsole.h])
	*/
	static void toConsoleBufferNS(const wchar_t* str, float& poX, float& poY, int x, int y, int colour);

	/*
	toConsoleBuffer(const wchar_t* str, float poX, float poY, int x, int y);
	* str    - wstring to be drawn to buffer
	* poX    - the x coord of origin position of the console (where you consoder (0,0) to be)
	* poY    - the y coord of origin position of the console (where you consoder (0,0) to be)
	* x      - x position from the origin poX coordinate on the screen
	* y      - y position from the origin poY coordinate on the screen

	summary:
	Adds single line to the console to the
	specified (x,y) position from the origin (poX, poY)
	to be drawn after drawConsole(); is called
	*/
	static void toConsoleBufferNS(const wchar_t* str, float& poX, float& poY, int x, int y);

	/*
	toConsoleBuffer(const wchar_t *str, int x, int y, int colour);
	* str    - wstring to be drawn to buffer
	* x      - x position from the top left corner of the screen
	* y      - y position from the top left corner of the screen
	* colour - colour of text

	summary:
	Adds single line to the console to the
	specified (x,y) position from the specified top left corner
	to be drawn after drawConsole(); is called
	(with text colour modification [see: enum colour in EmConsole.h])
	*/
	static void toConsoleBufferNS(const wchar_t* str, int x, int y, int colour);

	/*
	toConsoleBuffer(const wchar_t *str, int x, int y);
	* str    - wstring to be drawn to buffer
	* x      - x position from the top left corner of the screen
	* y      - y position from the top left corner of the screen

	summary:
	Adds single line to the console to the
	specified (x,y) position from the specified top left corner
	to be drawn after drawConsole(); is called
	*/
	static void toConsoleBufferNS(const wchar_t* str, int x, int y);

	/*
	toConsoleBuffer(wstring &str, float poX, float poY, int x, int y, int colour);
	* str    - wstring to be drawn to buffer
	* poX    - the x coord of origin position of the console (where you consoder (0,0) to be)
	* poY    - the y coord of origin position of the console (where you consoder (0,0) to be)
	* x      - x position from the origin poX coordinate on the screen
	* y      - y position from the origin poY coordinate on the screen
	* colour - colour of text

	summary:
	Adds single line to the console to the
	specified (x,y) position from the origin (poX, poY)
	to be drawn after drawConsole(); is called
	(with text colour modification [see: enum colour in EmConsole.h])
	*/
	static void toConsoleBuffer(std::wstring& str, float& poX, float& poY, int x, int y, std::vector<int>& colour);

	/*
	toConsoleBuffer(wstring &str, float poX, float poY, int x, int y, int colour);
	* str    - wstring to be drawn to buffer
	* poX    - the x coord of origin position of the console (where you consoder (0,0) to be)
	* poY    - the y coord of origin position of the console (where you consoder (0,0) to be)
	* x      - x position from the origin poX coordinate on the screen
	* y      - y position from the origin poY coordinate on the screen
	* colour - colour of text

	summary:
	Adds single line to the console to the
	specified (x,y) position from the origin (poX, poY)
	to be drawn after drawConsole(); is called
	(with text colour modification [see: enum colour in EmConsole.h])
	*/
	static void toConsoleBuffer(std::wstring& str, float& poX, float& poY, int x, int y, int colour);

	/*
	toConsoleBuffer(wstring &str, float poX, float poY, int x, int y);
	* str    - wstring to be drawn to buffer
	* poX    - the x coord of origin position of the console (where you consoder (0,0) to be)
	* poY    - the y coord of origin position of the console (where you consoder (0,0) to be)
	* x      - x position from the origin poX coordinate on the screen
	* y      - y position from the origin poY coordinate on the screen

	summary:
	Adds single line to the console to the
	specified (x,y) position from the origin (poX, poY)
	to be drawn after drawConsole(); is called
	*/
	static void toConsoleBuffer(std::wstring& str, float& poX, float& poY, int x, int y);

	/*
	toConsoleBuffer(wstring &str, int x, int y, int colour);
	* str    - wstring to be drawn to buffer
	* x      - x position from the top left corner of the screen
	* y      - y position from the top left corner of the screen
	* colour - colour of text

	summary:
	Adds single line to the console to the
	specified (x,y) position from the top left corner
	to be drawn after drawConsole(); is called
	(with text colour modification [see: enum colour in EmConsole.h])
	*/
	static void toConsoleBuffer(std::wstring& str, int x, int y, int colour);

	/*
	toConsoleBuffer(wstring &str, int x, int y);
	* str    - wstring to be drawn
	* x      - x position from the top left corner of the screen
	* y      - y position from the top left corner of the screen

	summary:
	Adds single line to the console to the
	specified (x,y) position from the top left corner
	to be drawn after drawConsole(); is called
	*/
	static void toConsoleBuffer(std::wstring& str, int x, int y);

	/*
	toConsoleBuffer(vector<wstring>& str, float poX, float poY, int x, int y, std::vector<std::vector<int>> &colour);
	* str    - vector of wstring to be drawn to buffer
	* poX    - the x coord of origin position of the console (where you consoder (0,0) to be)
	* poY    - the y coord of origin position of the console (where you consoder (0,0) to be)
	* x      - x position from the origin poX coordinate on the screen
	* y      - y position from the origin poY coordinate on the screen
	* colour - colour of text

	summary:
	Adds multiple lines to the console starting at the
	specified (x,y) position from the specified origin (poX, poY)
	to be drawn after drawConsole(); is called (with text colour modification [see: enum colour in EmConsole.h])
	*/
	static void toConsoleBuffer(Sprite& str, int x, int y, std::vector<std::vector<int>>& colour);

	/*
	toConsoleBuffer(vector<wstring>& str, float poX, float poY, int x, int y, int colour);
	* str    - vector of wstring to be drawn to buffer
	* poX    - the x coord of origin position of the console (where you consoder (0,0) to be)
	* poY    - the y coord of origin position of the console (where you consoder (0,0) to be)
	* x      - x position from the origin poX coordinate on the screen
	* y      - y position from the origin poY coordinate on the screen
	* colour - colour of text

	summary:
	Adds multiple lines to the console starting at the
	specified (x,y) position from the specified origin (poX, poY)
	to be drawn after drawConsole(); is called (with text colour modification [see: enum colour in EmConsole.h])
	*/
	static void toConsoleBuffer(Sprite& str, float& poX, float& poY, int x, int y, int colour);

	/*
	toConsoleBuffer(vector<wstring>& str, float poX, float poY, int x, int y);
	* str    - vector of wstring to be drawn to buffer
	* poX    - the x coord of origin position of the console (where you consoder (0,0) to be)
	* poY    - the y coord of origin position of the console (where you consoder (0,0) to be)
	* x      - x position from the origin poX coordinate on the screen
	* y      - y position from the origin poY coordinate on the screen

	summary:
	Adds multiple lines to the console starting at the
	specified (x,y) position from the origin (poX, poY)
	to be drawn after drawConsole(); is called
	*/
	static void toConsoleBuffer(Sprite& str, float& poX, float& poY, int x, int y);

	/*
	toConsoleBuffer(vector<wstring>& str, int x, int y, int colour);
	* str    - vector of wstring to be drawn to buffer
	* x      - x position from the top left corner of the screen
	* y      - y position from the top left corner of the screen
	* colour - colour of text

	summary:
	Adds multiple lines to the console starting at the
	specified (x,y) position from the top left corner
	to be drawn after drawConsole(); is called
	(with text colour modification [see: enum colour in EmConsole.h])
	*/
	static void toConsoleBuffer(Sprite& str, int x, int y, int colour);

	/*
	toConsoleBuffer(vector<wstring>& str, int x, int y);
	* str    - vector of wstring to be drawn to buffer
	* x      - x position from the top left corner of the screen
	* y      - y position from the top left corner of the screen

	summary:
	Adds multiple lines to the console starting at the
	specified (x,y) position from the top left corner
	to be drawn after drawConsole(); is called
	*/
	static void toConsoleBuffer(Sprite& str, int x, int y);

	/*
		toConsoleBuffer(vector<wstring>& str, int x, int y);
		* str    - vector of wstring to be drawn to buffer
		* x      - x position from the top left corner of the screen
		* y      - y position from the top left corner of the screen

		summary:
		Adds multiple lines to the console starting at the
		specified (x,y) position from the top left corner
		to be drawn after drawConsole(); is called
		*/
	static void toConsoleBuffer(Sprite& str);

	/*
			toConsoleBuffer(vector<wstring>& str, int x, int y);
			* str    - vector of wstring to be drawn to buffer
			* x      - x position from the top left corner of the screen
			* y      - y position from the top left corner of the screen

			summary:
			Adds multiple lines to the console starting at the
			specified (x,y) position from the top left corner
			to be drawn after drawConsole(); is called
			*/
	static void toConsoleBuffer(Sprite& str, int colour);

	/*
	toConsoleBuffer(const wchar_t* str, float poX, float poY, int x, int y, int colour);
	* str    - wstring to be drawn to buffer
	* poX    - the x coord of origin position of the console (where you consoder (0,0) to be)
	* poY    - the y coord of origin position of the console (where you consoder (0,0) to be)
	* x      - x position from the origin poX coordinate on the screen
	* y      - y position from the origin poY coordinate on the screen
	* colour - colour of text

	summary:
	Adds single line to the console to the
	specified (x,y) position from the origin (poX, poY)
	to be drawn after drawConsole(); is called
	(with text colour modification [see: enum colour in EmConsole.h])
	*/
	static void toConsoleBuffer(const wchar_t* str, float& poX, float& poY, int x, int y, int colour);

	/*
	toConsoleBuffer(const wchar_t* str, float poX, float poY, int x, int y);
	* str    - wstring to be drawn to buffer
	* poX    - the x coord of origin position of the console (where you consoder (0,0) to be)
	* poY    - the y coord of origin position of the console (where you consoder (0,0) to be)
	* x      - x position from the origin poX coordinate on the screen
	* y      - y position from the origin poY coordinate on the screen

	summary:
	Adds single line to the console to the
	specified (x,y) position from the origin (poX, poY)
	to be drawn after drawConsole(); is called
	*/
	static void toConsoleBuffer(const wchar_t* str, float& poX, float& poY, int x, int y);

	/*
	toConsoleBuffer(const wchar_t *str, int x, int y, int colour);
	* str    - wstring to be drawn to buffer
	* x      - x position from the top left corner of the screen
	* y      - y position from the top left corner of the screen
	* colour - colour of text

	summary:
	Adds single line to the console to the
	specified (x,y) position from the specified top left corner
	to be drawn after drawConsole(); is called
	(with text colour modification [see: enum colour in EmConsole.h])
	*/
	static void toConsoleBuffer(const wchar_t* str, int x, int y, int colour);

	/*
	toConsoleBuffer(const wchar_t *str, int x, int y);
	* str    - wstring to be drawn to buffer
	* x      - x position from the top left corner of the screen
	* y      - y position from the top left corner of the screen

	summary:
	Adds single line to the console to the
	specified (x,y) position from the specified top left corner
	to be drawn after drawConsole(); is called
	*/
	static void toConsoleBuffer(const wchar_t* str, int x, int y);
#pragma endregion

	/*
	drawConsole(bool clear = true);
	* clear = clear each frame

	Summary:
	Draws the console (choose to clear console buffer after)
	*Note:
	Default is true
	*/
	static void drawConsole(bool clear = true);

	//clears the console
	static void clearConsole();

private:

};
