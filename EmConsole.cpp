/*************************************************
**Name     : Emory Wynn							**
**Student# : 100655604							**
**************************************************/

#include "EmConsole.h"
#include <io.h>
#include <fcntl.h>

using namespace std;
/**Variables**/

static DWORD oldInputMode, newInputMode;

//static COORD _cursorPosition;
static HANDLE m_con[2]/*, _input*/;
static INPUT_RECORD m_inputRecord[128];

//static UINT _conWidth, _conHeight;
static bool m_buff, m_resizable, m_full, m_init;

bool MouseInput::doubleClick;
short MouseInput::vertWheel = 0, MouseInput::horiWheel = 0;
COORD MouseInput::position;
std::map <short, bool> MouseInput::buttons;

/***Constructers***/

void EmConsole::init()
{
	if(m_init) return;
	m_init = true;
	_setmode(_fileno(stdout), _O_U16TEXT);

	SetConsoleTitleA("EmGine Console Window");
	SetConsoleOutputCP(CP_UTF8);

	m_con[0] = GetStdHandle(STD_OUTPUT_HANDLE);
	m_con[1] = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	setConsoleSize(0, 0);
}

/***Functions**/

//Sets the window to full screen
void EmConsole::setFullScreen(bool full)
{
	if(m_full == full)
		return;

	static COORD sz;

	if(full)
	{
		sz = getConsoleSize();
		COORD sz2 = GetLargestConsoleWindowSize(GetStdHandle(STD_OUTPUT_HANDLE));
		setConsoleSize(sz2.X, sz2.Y);
		//SetConsoleDisplayMode(_con[_buff], (full ? CONSOLE_FULLSCREEN_MODE : CONSOLE_WINDOWED_MODE), nullptr);
		SetConsoleDisplayMode(m_con[0], (full ? CONSOLE_FULLSCREEN_MODE : CONSOLE_WINDOWED_MODE), nullptr);
		SetConsoleDisplayMode(m_con[1], (full ? CONSOLE_FULLSCREEN_MODE : CONSOLE_WINDOWED_MODE), nullptr);
	} else
	{
		//SetConsoleDisplayMode(_con[_buff], (full ? CONSOLE_FULLSCREEN_MODE : CONSOLE_WINDOWED_MODE), nullptr);
		SetConsoleDisplayMode(m_con[0], (full ? CONSOLE_FULLSCREEN_MODE : CONSOLE_WINDOWED_MODE), nullptr);
		SetConsoleDisplayMode(m_con[1], (full ? CONSOLE_FULLSCREEN_MODE : CONSOLE_WINDOWED_MODE), nullptr);
		this->m_full = full;
		setConsoleSize(sz.X, sz.Y);
	}
	m_full = full;
}

bool EmConsole::getFullScreen()
{
	return m_full;
}

/*
summary:
Creates a vector of vector string (2D vector) from a txt file

Usage:
To use this function effectively make sure your *.txt file has
no empty lines in the beginning or end of it (so no extra
vectors are not accidentaly created). Also, make sure ther is only one
space seperating every sprite from each other (for the same reason as before).
if done correctly you will basically have a sprite sheet that can be usefull for
character movement or switching items or whatever.
*/
void EmConsole::textFileToVector(string file, vector<vector<wstring>>& str)
{

	wfstream ascii(file, ios::in);

	try//if(ascii.is_open())
	{
		wstring line;

		str.push_back(vector<wstring>());

		for(int a = 0; getline(ascii, line); a++)
		{
			if(line.size() == 0)
			{
				str.push_back(vector<wstring>());
				continue;
			}
			else if(line.size() > 0)
				str[a].push_back(line);
			//OutputDebugString((line + "\n").c_str());
		}
	} catch(...) {}

	ascii.close();
}

vector<vector<wstring>> EmConsole::textFileToVector(string file)
{
	vector<vector<wstring>> str;
	textFileToVector(file, str);
	return str;
}

void EmConsole::setTitle(string title)
{
	SetConsoleTitleA(title.c_str());
}

//Sets the console size
void EmConsole::setConsoleSize(ushort width, ushort height)
{
	if(!m_full)
	{
		m_sz = GetLargestConsoleWindowSize(GetStdHandle(STD_OUTPUT_HANDLE));//largest buffer size
		static SMALL_RECT _SIZE;

		//window dimentions
		_SIZE.Top = 0;
		_SIZE.Left = 0;
		_SIZE.Bottom = height < m_sz.Y ? (SHORT)height - 1 : (SHORT)m_sz.Y - 1;
		_SIZE.Right = width < m_sz.X ? (SHORT)width - 1 : (SHORT)m_sz.X - 1;

		if(!m_resizable)
			sz = {SIZE.Right + 1,SIZE.Bottom + 1};

		SetConsoleScreenBufferSize(m_con[!m_buff], m_sz);
		SetConsoleWindowInfo(m_con[!m_buff], true, &_SIZE);

		SetConsoleScreenBufferSize(m_con[m_buff], m_sz);
		SetConsoleWindowInfo(m_con[m_buff], true, &_SIZE);

		m_conWidth = _SIZE.Right;
		m_conHeight = _SIZE.Bottom;
	}
}

//Gets console's width and height in character blocks
COORD EmConsole::getConsoleSize()
{

	CONSOLE_SCREEN_BUFFER_INFO csbi;
	//COORD size;
	//int columns, rows;

	if(!m_full)
	{
		GetConsoleScreenBufferInfo(m_con[!m_buff], &csbi);

		//columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
		//rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
		return {csbi.srWindow.Right - csbi.srWindow.Left + 1, csbi.srWindow.Bottom - csbi.srWindow.Top + 1};
	}

	return GetLargestConsoleWindowSize(GetStdHandle(STD_OUTPUT_HANDLE));

	//return size/*COORD {(SHORT)columns,(SHORT)rows}*/;

}

int EmConsole::getWidth()
{
	return getConsoleSize().X;
}

int EmConsole::getHeight()
{
	return getConsoleSize().Y;
}

void EmConsole::printf(const char* print ...)
{

	static int printNum = 0;
	wchar_t* buff = new wchar_t[128];
	va_list arg;
	va_start(arg, print);
	vswprintf(buff, 128, L"%p", arg);
	va_end(arg);


	toConsoleBuffer(buff, printNum++, 0);
	delete[] buff;
}

//Sets weather or not the window can be resized
void EmConsole::setResizeable(bool resz)
{
	m_resizable = resz;
}

bool EmConsole::getResizeable()
{
	return m_resizable;
}

//Sets console position
void EmConsole::setConsolePosition(int x, int y)
{
	SetWindowPos(GetConsoleWindow(), NULL, (int)x, (int)y, NULL, NULL, SWP_NOSIZE | SWP_NOZORDER);
}

//Sets the console to the center of the screen
void EmConsole::placeConsoleCenter()
{
	RECT dw, cw;
	GetWindowRect(GetDesktopWindow(), &dw);
	GetWindowRect(GetConsoleWindow(), &cw);
	if(cw.left != 0)
		cw.right -= cw.left,
		cw.left -= cw.left;
	if(cw.top != 0)
		cw.bottom -= cw.top,
		cw.top -= cw.top;
	SetWindowPos(GetConsoleWindow(), NULL, (int)((dw.right / 2) - (cw.right / 2)), (int)(floor((float)dw.bottom / 2) - floor((float)cw.bottom / 2)), NULL, NULL, SWP_NOSIZE | SWP_NOZORDER);
}

//Sets console cursor position
void EmConsole::consoleCursorPosition(int x, int y)
{
	static COORD _cursorPosition;
	_cursorPosition.X = (SHORT)x;
	_cursorPosition.Y = (SHORT)y;
	CONSOLE_CURSOR_INFO info;
	info.bVisible = false;
	info.dwSize = 1;
	SetConsoleCursorInfo(m_con[m_buff], &info);
	SetConsoleCursorPosition(m_con[m_buff], _cursorPosition);
	//SetConsoleCursorInfo(con[1], &infoex);
	//SetConsoleCursorPosition(con[1], cursorPosition);
}


/*special stuff for tetris (don't even try to understand)*/
vector<int> EmConsole::readConsoleLineAtributes(int x, int y, float width)
{
	SMALL_RECT strSize;
	strSize.Top = (SHORT)y;
	strSize.Bottom = (SHORT)y + 1;
	strSize.Left = (SHORT)x;
	strSize.Right = (SHORT)x + (SHORT)width;

	//LPWORD a = new WORD[width];
	m_ci = new CHAR_INFO[(UINT)width];
	ReadConsoleOutputA(m_con[m_buff], m_ci, {(SHORT) width,1}, {(SHORT) 0,(SHORT) 0}, &strSize);

	vector<int>c;
	for(int d = 0; d < width; c.push_back(m_ci[d++].Attributes));


	//for(int d = 0; d < width; c.push_back(a[d++]))
	//	OutputDebugStringA((to_string(a[d])+", ").c_str());
	//OutputDebugStringA("\nend\n");
	delete[] m_ci;
	return c;

}

vector<vector<int>> EmConsole::readConsoleAreaAtributes(int x, int y, float width, float height)
{
	vector<vector<int>>ans;
	for(int a = 0; a < height; a++)
		ans.push_back(readConsoleLineAtributes(x, y++, width));
	//	OutputDebugStringA("\nend Area\n\n\n");
	return ans;
}

wstring EmConsole::readConsoleLine(int x, int y, float width)
{
	SMALL_RECT strSize;
	strSize.Top = (SHORT)y;
	strSize.Bottom = (SHORT)y + 1;
	strSize.Left = (SHORT)x;
	strSize.Right = (SHORT)x + (SHORT)width;

	m_ci = new CHAR_INFO[(UINT)width];
	ReadConsoleOutputA(m_con[m_buff], m_ci, {(SHORT) width,1}, {(SHORT) 0,(SHORT) 0}, &strSize);

	//vector<int>e;
	//for(int d = 0; d < width; e.push_back(ci[d++].Attributes))
	//	OutputDebugStringA((to_string(ci[d].Attributes) + ", ").c_str());
//	OutputDebugStringA("\nend\n");
	wstring c;

	for(int a = 0; a < width; c += m_ci[a++].Char.UnicodeChar);
	delete[]m_ci;
	return c;

}

vector<wstring> EmConsole::readConsoleArea(int x, int y, float width, float height)
{
	vector<wstring>ans;
	for(int a = 0; a < height; a++)
		ans.push_back(readConsoleLine(x, y++, width));
	return ans;
}

char EmConsole::readConsoleCharacter(int x, int y)
{
	return (char)readConsoleLine(x, y, 1)[0];
}

char EmConsole::readActiveConsoleCharacter(int x, int y)
{
	SMALL_RECT strSize;
	strSize.Top = (SHORT)y;
	strSize.Bottom = (SHORT)y + 1;
	strSize.Left = (SHORT)x;
	strSize.Right = (SHORT)x + 1;

	m_ci = new CHAR_INFO;
	ReadConsoleOutputA(m_con[!m_buff], m_ci, {(SHORT) 1,1}, {(SHORT) 0,(SHORT) 0}, &strSize);

	//vector<int>e;
	//for(int d = 0; d < width; e.push_back(ci[d++].Attributes))
	//	OutputDebugStringA((to_string(ci[d].Attributes) + ", ").c_str());
	//	OutputDebugStringA("\nend\n");
	char c;

	c = m_ci->Char.AsciiChar;
	delete m_ci;
	return c;
}


/*Console buffer overloads (overkill)*/

/*
Note: NS means No Spaces, wrighting only characters excluding spaces.
Extrymely inefficient
*/

/*
toConsoleBuffer(string &str, float poX, float poY, int x, int y, int colour);
* str    - string to be drawn to buffer
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
void EmConsole::toConsoleBufferNS(wstring& str, float& poX, float& poY, int x, int y, int colour)
{
	consoleCursorPosition(0, 0);
	SMALL_RECT strSize;
	strSize.Top = (SHORT)y;
	strSize.Bottom = (SHORT)y + 1;
	strSize.Left = (SHORT)x;
	strSize.Right = (SHORT)x + 1;

	poX = poX < 0 ? 0 : poX;
	poY = poY < 0 ? 0 : poY;

	m_ci = new CHAR_INFO;

	for(UINT a = 0; a < str.size(); a++)
	{
		if(str[a] != L' ')
		{
			m_ci->Char.UnicodeChar = str[a];
			m_ci->Attributes = colour;
			WriteConsoleOutputW(m_con[m_buff], m_ci, {1,1}, COORD{(SHORT) poX , (SHORT) poY}, &strSize);
		}
		strSize.Left = (SHORT)++x;
		strSize.Right = (SHORT)x + 1;
	}

	delete m_ci;
}

/*
toConsoleBuffer(string &str, float poX, float poY, int x, int y);
* str    - string to be drawn to buffer
* poX    - the x coord of origin position of the console (where you consoder (0,0) to be)
* poY    - the y coord of origin position of the console (where you consoder (0,0) to be)
* x      - x position from the origin poX coordinate on the screen
* y      - y position from the origin poY coordinate on the screen

summary:
Adds single line to the console to the
specified (x,y) position from the origin (poX, poY)
to be drawn after drawConsole(); is called
*/
void EmConsole::toConsoleBufferNS(wstring& str, float& poX, float& poY, int x, int y)
{
	toConsoleBufferNS(str, poX, poY, x, y, FG_WHITE);
}

/*
toConsoleBuffer(string &str, int x, int y, int colour);
* str    - string to be drawn to buffer
* x      - x position from the top left corner of the screen
* y      - y position from the top left corner of the screen
* colour - colour of text

summary:
Adds single line to the console to the
specified (x,y) position from the top left corner
to be drawn after drawConsole(); is called
(with text colour modification [see: enum colour in EmConsole.h])
*/
void EmConsole::toConsoleBufferNS(wstring& str, int x, int y, int colour)
{
	float poX = 0.f, poY = 0.f;
	toConsoleBufferNS(str, poX, poY, x, y, colour);
}

/*
toConsoleBuffer(string &str, int x, int y);
* str    - string to be drawn
* x      - x position from the top left corner of the screen
* y      - y position from the top left corner of the screen

summary:
Adds single line to the console to the
specified (x,y) position from the top left corner
to be drawn after drawConsole(); is called
*/
void EmConsole::toConsoleBufferNS(wstring& str, int x, int y)
{
	toConsoleBufferNS(str, x, y, FG_WHITE);
}

/*
toConsoleBuffer(Sprite& str, float poX, float poY, int x, int y, int colour);
* str    - vector of string to be drawn to buffer
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
void EmConsole::toConsoleBufferNS(Sprite& str, float& poX, float& poY, int x, int y, int colour)
{
	for(int a = 0; a < str.getHeight(); a++)
		toConsoleBufferNS(str.getSprite()[a], poX, poY, x, y++, colour);
}

/*
toConsoleBuffer(Sprite& str, float poX, float poY, int x, int y);
* str    - vector of string to be drawn to buffer
* poX    - the x coord of origin position of the console (where you consoder (0,0) to be)
* poY    - the y coord of origin position of the console (where you consoder (0,0) to be)
* x      - x position from the origin poX coordinate on the screen
* y      - y position from the origin poY coordinate on the screen

summary:
Adds multiple lines to the console starting at the
specified (x,y) position from the origin (poX, poY)
to be drawn after drawConsole(); is called
*/
void EmConsole::toConsoleBufferNS(Sprite& str, float& poX, float& poY, int x, int y)
{
	for(int a = 0; a < str.getHeight(); a++)
		toConsoleBufferNS(str.getSprite()[a], poX, poY, x, y++);
}

/*
toConsoleBuffer(Sprite& str, int x, int y, int colour);
* str    - vector of string to be drawn to buffer
* x      - x position from the top left corner of the screen
* y      - y position from the top left corner of the screen
* colour - colour of text

summary:
Adds multiple lines to the console starting at the
specified (x,y) position from the top left corner
to be drawn after drawConsole(); is called
(with text colour modification [see: enum colour in EmConsole.h])
*/
void EmConsole::toConsoleBufferNS(Sprite& str, int x, int y, int colour)
{
	for(int a = 0; a < str.getHeight(); a++)
		toConsoleBufferNS(str.getSprite()[a], x, y++, colour);
}

/*
toConsoleBuffer(Sprite& str, int x, int y);
* str    - vector of string to be drawn to buffer
* x      - x position from the top left corner of the screen
* y      - y position from the top left corner of the screen

summary:
Adds multiple lines to the console starting at the
specified (x,y) position from the top left corner
to be drawn after drawConsole(); is called
*/
void EmConsole::toConsoleBufferNS(Sprite& str, int x, int y)
{
	for(int a = 0; a < str.getHeight(); a++)
		toConsoleBufferNS(str.getSprite()[a], x, y++);
}

/*
toConsoleBuffer(const char* str, float poX, float poY, int x, int y, int colour);
* str    - string to be drawn to buffer
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
void EmConsole::toConsoleBufferNS(const wchar_t* str, float& poX, float& poY, int x, int y, int colour)
{
	wstring* str2 = new wstring(str);
	toConsoleBufferNS(*str2, poX, poY, x, y, colour);
	delete str2;
}

/*
toConsoleBuffer(const char* str, float poX, float poY, int x, int y);
* str    - string to be drawn to buffer
* poX    - the x coord of origin position of the console (where you consoder (0,0) to be)
* poY    - the y coord of origin position of the console (where you consoder (0,0) to be)
* x      - x position from the origin poX coordinate on the screen
* y      - y position from the origin poY coordinate on the screen

summary:
Adds single line to the console to the
specified (x,y) position from the origin (poX, poY)
to be drawn after drawConsole(); is called
*/
void EmConsole::toConsoleBufferNS(const wchar_t* str, float& poX, float& poY, int x, int y)
{
	wstring* str2 = new wstring(str);
	toConsoleBufferNS(*str2, poX, poY, x, y, FG_WHITE);
	delete str2;
}

/*
toConsoleBuffer(const wchar_t *str, int x, int y, int colour);
* str    - string to be drawn to buffer
* x      - x position from the top left corner of the screen
* y      - y position from the top left corner of the screen
* colour - colour of text

summary:
Adds single line to the console to the
specified (x,y) position from the specified top left corner
to be drawn after drawConsole(); is called
(with text colour modification [see: enum colour in EmConsole.h])
*/
void EmConsole::toConsoleBufferNS(const wchar_t* str, int x, int y, int colour)
{
	wstring* str2 = new wstring(str);
	toConsoleBufferNS(*str2, x, y, colour);
	delete str2;
}

/*
toConsoleBuffer(const wchar_t *str, int x, int y);
* str    - string to be drawn to buffer
* x      - x position from the top left corner of the screen
* y      - y position from the top left corner of the screen

summary:
Adds single line to the console to the
specified (x,y) position from the specified top left corner
to be drawn after drawConsole(); is called
*/
void EmConsole::toConsoleBufferNS(const wchar_t* str, int x, int y)
{
	wstring* str2 = new wstring(str);
	toConsoleBufferNS(*str2, x, y);
	delete str2;
}

/*
toConsoleBuffer(Sprite& str, float poX, float poY, int x, int y, int colour);
* str    - vector of string to be drawn to buffer
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
void EmConsole::toConsoleBuffer(Sprite& str, int x, int y, vector<vector<int>>& colour)
{
	float p1 = 0, p2 = 0;
	wstring str2;
	for(int a = 0; a < str.getHeight(); a++)
		toConsoleBuffer(str2 = str.getSprite()[a], p1, p2, x, y++, colour[a]);
}

/*
toConsoleBuffer(Sprite& str, float poX, float poY, int x, int y, int colour);
* str    - vector of string to be drawn to buffer
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
void EmConsole::toConsoleBuffer(Sprite& str, float& poX, float& poY, int x, int y, int colour)
{
	//for(int a = 0; a < str.getHeight(); a++)
	//	toConsoleBuffer(str.getSprite()[a], poX, poY, x, y++, colour);

	consoleCursorPosition(0, 0);

	////lets try this
	//CONSOLE_SCREEN_BUFFER_INFOEX infoex;
	//PCONSOLE_SCREEN_BUFFER_INFOEX;
	//infoex.cbSize = sizeof(infoex);
	////CONSOLE_SCREEN_BUFFER_INFO info;
	//int tester = 0;
	//if(GetConsoleScreenBufferInfoEx(m_con[m_buff], &infoex))
	//	tester++;
	////GetConsoleScreenBufferInfo(m_con[m_buff], &info);
	////infoex.srWindow = info.srWindow;
	////
	//int test = 7;
	//infoex.ColorTable[test] = RGB(colour.r, colour.g, colour.b);
	//infoex;
	//
	//
	//if(SetConsoleScreenBufferInfoEx(m_con[m_buff], &infoex))
	//	tester++;
	////SetConsoleScreenBufferSize(m_con[!m_buff], GetLargestConsoleWindowSize(GetStdHandle(STD_OUTPUT_HANDLE)));



	SMALL_RECT strSize;
	strSize.Top = (SHORT)y;
	strSize.Bottom = (SHORT)y + str.getHeight();
	strSize.Left = (SHORT)x;
	strSize.Right = (SHORT)x + str.getWidth() - 1;

	poX = poX < 0 ? 0 : poX;
	poY = poY < 0 ? 0 : poY;

	m_ci = new CHAR_INFO[str.getWidth()*str.getHeight()];

	for(UINT a = 0; a < str.getHeight(); a++)
	{
		for(UINT b = 0; b < str.getWidth(); b++)
		{
			if(b < str.getSprite()[a].size())
				m_ci[b + str.getWidth()*a].Char.UnicodeChar = str.getSprite()[a][b];
			else
				m_ci[b + str.getWidth()*a].Char.UnicodeChar = L' ';
			m_ci[b + str.getWidth()*a].Attributes = colour;
		}
		//static DWORD tmp = 0;
		//consoleCursorPosition(x, y++);
		//WriteConsoleW(m_con[m_buff], ci, (SHORT)str.getWidth(), &tmp, NULL);
	}

	WriteConsoleOutputW(m_con[m_buff], m_ci, {(SHORT) str.getWidth() ,(SHORT) str.getHeight()}, COORD{(SHORT) poX , (SHORT) poY}, &strSize);


	delete[] m_ci;
}

/*
toConsoleBuffer(Sprite& str, float poX, float poY, int x, int y);
* str    - vector of string to be drawn to buffer
* poX    - the x coord of origin position of the console (where you consoder (0,0) to be)
* poY    - the y coord of origin position of the console (where you consoder (0,0) to be)
* x      - x position from the origin poX coordinate on the screen
* y      - y position from the origin poY coordinate on the screen

summary:
Adds multiple lines to the console starting at the
specified (x,y) position from the origin (poX, poY)
to be drawn after drawConsole(); is called
*/
void EmConsole::toConsoleBuffer(Sprite& str, float& poX, float& poY, int x, int y)
{
	//for(int a = 0; a < str.getHeight(); a++)
	//	toConsoleBuffer(str.getSprite()[a], poX, poY, x, y++);
	toConsoleBuffer(str, poX, poY, x, y, FG_WHITE);
}

/*
toConsoleBuffer(Sprite& str, int x, int y, int colour);
* str    - vector of string to be drawn to buffer
* x      - x position from the top left corner of the screen
* y      - y position from the top left corner of the screen
* colour - colour of text

summary:
Adds multiple lines to the console starting at the
specified (x,y) position from the top left corner
to be drawn after drawConsole(); is called
(with text colour modification [see: enum colour in EmConsole.h])
*/
void EmConsole::toConsoleBuffer(Sprite& str, int x, int y, int colour)
{
	//for(int a = 0; a < str.getHeight(); a++)
	//	toConsoleBuffer(str.getSprite()[a], x, y++, colour);
	float poX = 0, poY = 0;
	toConsoleBuffer(str, poX, poY, x, y, colour);

	//string* str2 = new string(str);
	//toConsoleBuffer(*str2, poX, poY, x, y, colour);
	//delete str2;

	//consoleCursorPosition(0, 0);
	//
	//SMALL_RECT strSize;
	//strSize.Top = y;
	//strSize.Bottom = y + str.getHeight();
	//strSize.Left = x;
	//strSize.Right = x + str.getWidth() - 1;
	//
	////poX = poX < 0 ? 0 : poX;
	////poY = poY < 0 ? 0 : poY;
	//
	//ci = new CHAR_INFO[str.getWidth()*str.getHeight()];
	//
	//for(int a = 0; a < str.getHeight(); a++)
	//{
	//	for(int b = 0; b < str.getWidth(); b++)
	//	{
	//		if(b < str.getSprite()[a].size())
	//			ci[b + str.getWidth()*a].Char.UnicodeChar = str.getSprite()[a][b];
	//		else
	//			ci[b + str.getWidth()*a].Char.UnicodeChar = ' ';
	//		ci[b + str.getWidth()*a].Attributes = colour;
	//	}
	//}
	//
	//WriteConsoleOutputW(_con[_buff], ci, {(SHORT)str.getWidth() ,(SHORT)str.getHeight()}, COORD {(SHORT)0 , (SHORT)0}, &strSize);
	//
	//
	//delete[] ci;
}

/*
toConsoleBuffer(Sprite& str, int x, int y);
* str    - vector of string to be drawn to buffer
* x      - x position from the top left corner of the screen
* y      - y position from the top left corner of the screen

summary:
Adds multiple lines to the console starting at the
specified (x,y) position from the top left corner
to be drawn after drawConsole(); is called
*/
void EmConsole::toConsoleBuffer(Sprite& str, int x, int y)
{
	//for(int a = 0; a < str.getHeight(); a++)
	//	toConsoleBuffer(str.getSprite()[a], x, y++);
	toConsoleBuffer(str, x, y, str.getColour());
}

void EmConsole::toConsoleBuffer(Sprite& str)
{
	toConsoleBuffer(str, str.getX(), str.getY(), str.getColour());
}

void EmConsole::toConsoleBuffer(Sprite& str, int colour)
{
	toConsoleBuffer(str, str.getX(), str.getY(), colour);
}

/*
toConsoleBuffer(const wchar_t* str, float poX, float poY, int x, int y, int colour);
* str    - string to be drawn to buffer
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
void EmConsole::toConsoleBuffer(const wchar_t* str, float& poX, float& poY, int x, int y, int colour)
{
	//string* str2 = new string(str);
	//toConsoleBuffer(*str2, poX, poY, x, y, colour);
	//delete str2;

	consoleCursorPosition(0, 0);

	SMALL_RECT strSize;
	strSize.Top = (SHORT)y;
	strSize.Bottom = (SHORT)y + 1;
	strSize.Left = (SHORT)x;
	strSize.Right = (SHORT)x + (SHORT)wcslen(str) - 1;

	poX = poX < 0 ? 0 : poX;
	poY = poY < 0 ? 0 : poY;

	m_ci = new CHAR_INFO[wcslen(str)];

	for(UINT a = 0; a < wcslen(str); a++)
	{
		m_ci[a].Char.UnicodeChar = str[a];
		m_ci[a].Attributes = colour;
	}

	WriteConsoleOutputW(m_con[m_buff], m_ci, {(SHORT) wcslen(str),1}, COORD{(SHORT) poX , (SHORT) poY}, &strSize);
	delete[] m_ci;
}

/*
toConsoleBuffer(const wchar_t* str, float poX, float poY, int x, int y);
* str    - string to be drawn to buffer
* poX    - the x coord of origin position of the console (where you consoder (0,0) to be)
* poY    - the y coord of origin position of the console (where you consoder (0,0) to be)
* x      - x position from the origin poX coordinate on the screen
* y      - y position from the origin poY coordinate on the screen

summary:
Adds single line to the console to the
specified (x,y) position from the origin (poX, poY)
to be drawn after drawConsole(); is called
*/
void EmConsole::toConsoleBuffer(const wchar_t* str, float& poX, float& poY, int x, int y)
{
	wstring* str2 = new wstring(str);
	toConsoleBuffer(*str2, poX, poY, x, y, FG_WHITE);
	delete str2;
}

/*
toConsoleBuffer(const wchar_t *str, int x, int y, int colour);
* str    - string to be drawn to buffer
* x      - x position from the top left corner of the screen
* y      - y position from the top left corner of the screen
* colour - colour of text

summary:
Adds single line to the console to the
specified (x,y) position from the specified top left corner
to be drawn after drawConsole(); is called
(with text colour modification [see: enum colour in EmConsole.h])
*/
void EmConsole::toConsoleBuffer(const wchar_t* str, int x, int y, int colour)
{
	wstring* str2 = new wstring(str);
	toConsoleBuffer(*str2, x, y, colour);
	delete str2;
}

/*
toConsoleBuffer(const wchar_t *str, int x, int y);
* str    - string to be drawn to buffer
* x      - x position from the top left corner of the screen
* y      - y position from the top left corner of the screen

summary:
Adds single line to the console to the
specified (x,y) position from the specified top left corner
to be drawn after drawConsole(); is called
*/
void EmConsole::toConsoleBuffer(const wchar_t* str, int x, int y)
{
	wstring* str2 = new wstring(str);
	toConsoleBuffer(*str2, x, y);
	delete str2;
}

/*
toConsoleBuffer(string &str, float poX, float poY, int x, int y, int colour);
* str    - string to be drawn to buffer
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
void EmConsole::toConsoleBuffer(wstring& str, float& poX, float& poY, int x, int y, vector<int>& colour)
{
	consoleCursorPosition(0, 0);

	SMALL_RECT strSize;
	strSize.Top = (SHORT)y;
	strSize.Bottom = (SHORT)y + 1;
	strSize.Left = (SHORT)x;
	strSize.Right = (SHORT)x + (SHORT)str.size() - 1;

	poX = poX < 0 ? 0 : poX;
	poY = poY < 0 ? 0 : poY;

	m_ci = new CHAR_INFO[str.size()];

	for(UINT a = 0; a < str.size(); a++)
	{
		m_ci[a].Char.UnicodeChar = str[a];
		m_ci[a].Attributes = colour[a];
	}

	WriteConsoleOutputW(m_con[m_buff], m_ci, {(SHORT) str.size(),1}, COORD{(SHORT) poX , (SHORT) poY}, &strSize);
	delete[] m_ci;
}

/*
toConsoleBuffer(string &str, float poX, float poY, int x, int y, int colour);
* str    - string to be drawn to buffer
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
void EmConsole::toConsoleBuffer(wstring& str, float& poX, float& poY, int x, int y, int colour)
{
	//consoleCursorPosition(0, 0);
	//
	//SMALL_RECT strSize;
	//strSize.Top = y;
	//strSize.Bottom = y + 1;
	//strSize.Left = x;
	//strSize.Right = x + str.size() - 1;
	//
	//poX = poX < 0 ? 0 : poX;
	//poY = poY < 0 ? 0 : poY;
	//
	//ci = new CHAR_INFO[str.size()];
	//
	//for(int a = 0; a < str.size(); a++)
	//{
	//	ci[a].Char.UnicodeChar = str[a];
	//	ci[a].Attributes = colour;
	//}
	//
	//WriteConsoleOutputW(con[counter], ci, {(SHORT)str.size(),1}, COORD {(SHORT)poX , (SHORT)poY}, &strSize);
	//delete[] ci;
	toConsoleBuffer(str.c_str(), poX, poY, x, y, colour);
}

/*
toConsoleBuffer(string &str, float poX, float poY, int x, int y);
* str    - string to be drawn to buffer
* poX    - the x coord of origin position of the console (where you consoder (0,0) to be)
* poY    - the y coord of origin position of the console (where you consoder (0,0) to be)
* x      - x position from the origin poX coordinate on the screen
* y      - y position from the origin poY coordinate on the screen

summary:
Adds single line to the console to the
specified (x,y) position from the origin (poX, poY)
to be drawn after drawConsole(); is called
*/
void EmConsole::toConsoleBuffer(wstring& str, float& poX, float& poY, int x, int y)
{
	toConsoleBuffer(str, poX, poY, x, y, FG_WHITE);
}

/*
toConsoleBuffer(string &str, int x, int y, int colour);
* str    - string to be drawn to buffer
* x      - x position from the top left corner of the screen
* y      - y position from the top left corner of the screen
* colour - colour of text

summary:
Adds single line to the console to the
specified (x,y) position from the top left corner
to be drawn after drawConsole(); is called
(with text colour modification [see: enum colour in EmConsole.h])
*/
void EmConsole::toConsoleBuffer(wstring& str, int x, int y, int colour)
{
	float poX = 0.f, poY = 0.f;
	toConsoleBuffer(str, poX, poY, x, y, colour);
}

/*
toConsoleBuffer(string &str, int x, int y);
* str    - string to be drawn
* x      - x position from the top left corner of the screen
* y      - y position from the top left corner of the screen

summary:
Adds single line to the console to the
specified (x,y) position from the top left corner
to be drawn after drawConsole(); is called
*/
void EmConsole::toConsoleBuffer(wstring& str, int x, int y)
{
	toConsoleBuffer(str, x, y, FG_WHITE);
}

void PrintLastError(const char* head, int code)
{
	void* lpMsgBuf = nullptr;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		(DWORD)code,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	OutputDebugStringA((std::string(head) + (const char*)lpMsgBuf).c_str());
	OutputDebugStringA("\n");
}

/*Draws the console (choose to clear console buffer after draw)
*Note:
	Default is true
*/
void EmConsole::drawConsole(bool clear)
{
	if(!m_full)
		if(m_resizable)
			setConsoleSize(getWidth(), getHeight());
		else
			setConsoleSize(m_conWidth + 1, m_conHeight + 1);
	CONSOLE_FONT_INFOEX fontEX;
	fontEX.cbSize = sizeof(fontEX);
	if(GetCurrentConsoleFontEx(m_con[m_buff], m_full, &fontEX))
	{
		fontEX.dwFontSize.X = 6;
		fontEX.dwFontSize.Y = 13;
		fontEX.FontFamily = FF_DONTCARE;
		SetCurrentConsoleFontEx(m_con[m_buff], m_full, &fontEX);
	}
	

	//PrintLastError("Console Font: ", GetLastError());

	if(!clear)
	{
		SetConsoleActiveScreenBuffer(m_con[m_buff]);



		//have not tested yet
		//swap(_con[_buff], _con[!_buff]);

		//tryed and true
		HANDLE temp = m_con[m_buff];
		m_con[m_buff] = m_con[!m_buff];
		m_con[!m_buff] = temp;
		m_buff = !m_buff;
	}
	else
	{
		SetConsoleActiveScreenBuffer(m_con[m_buff]);
		m_buff = !m_buff;
		clearConsole();
	}
	

}

/*Clears the console*/
void EmConsole::clearConsole()
{
	static DWORD _cCharsWritten;
	COORD L = GetLargestConsoleWindowSize(GetStdHandle(STD_OUTPUT_HANDLE));
	FillConsoleOutputCharacterA(m_con[m_buff], ' ', L.X * L.Y, COORD{0,0}, &_cCharsWritten);
	FillConsoleOutputAttribute(m_con[m_buff], NULL, L.X * L.Y, COORD{0,0}, &_cCharsWritten);
}

void Sprite::toBuffer(ushort x, ushort y)
{
	EmConsole::toConsoleBuffer(*this, x, y, m_colour);
}

void Sprite::toBufferNS(ushort x, ushort y)
{
	EmConsole::toConsoleBufferNS(*this, x, y, m_colour);
}

void Sprite::render()
{
	EmConsole::drawConsole();
}
