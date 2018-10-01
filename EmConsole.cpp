/*************************************************
**Name     : Emory Wynn							**
**Student# : 100655604							**
**************************************************/

#include "EmConsole.h"

using namespace std;

bool MouseInput::doubleClick;
short MouseInput::vertWheel=0, MouseInput::horiWheel=0;
COORD MouseInput::position;
std::map <short, bool> MouseInput::buttons;

/***Constructers***/

EmConsole::EmConsole(string& title)
{
	SetConsoleOutputCP(CP_WINUNICODE);
	//int s=CP_INSTALLED;

	SetConsoleTitleA(title.c_str());
	_cursorPosition.X = 0;
	_cursorPosition.Y = 0;
	_con[0] = GetStdHandle(STD_OUTPUT_HANDLE);
	_con[1] = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	setConsoleSize(0, 0);
}

EmConsole::EmConsole(const char * title)
{
	SetConsoleTitleA(title);
	SetConsoleOutputCP(CP_WINUNICODE);
	
	_cursorPosition.X = 0;
	_cursorPosition.Y = 0;
	
	_con[0] = GetStdHandle(STD_OUTPUT_HANDLE);
	_con[1] = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);

	_input = GetStdHandle(STD_INPUT_HANDLE);
	GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &oldInputMode);
	SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), newInputMode = (oldInputMode|ENABLE_MOUSE_INPUT) & ~ENABLE_QUICK_EDIT_MODE);

	//_setmode(_fileno(stdout), _O_U16TEXT); //sets the console to UTF16 mode.

	setConsoleSize(0, 0);
}

EmConsole::EmConsole()
{
	SetConsoleTitleA("New Window");
	_cursorPosition.X = 0;
	_cursorPosition.Y = 0;
	_con[0] = GetStdHandle(STD_OUTPUT_HANDLE);
	_con[1] = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	setConsoleSize(0, 0);
}


/***Functions**/

//Sets the window to full screen
void EmConsole::setFullScreen(bool full)
{
	if(_full == full)
		return;
	
	static COORD sz;
	
	if(full)
	{
		sz = getConsoleSize();
		COORD sz2 = GetLargestConsoleWindowSize(GetStdHandle(STD_OUTPUT_HANDLE));
		setConsoleSize(sz2.X, sz2.Y);
		//SetConsoleDisplayMode(_con[_buff], (full ? CONSOLE_FULLSCREEN_MODE : CONSOLE_WINDOWED_MODE), nullptr);
		SetConsoleDisplayMode(_con[0], (full ? CONSOLE_FULLSCREEN_MODE : CONSOLE_WINDOWED_MODE), nullptr);
		SetConsoleDisplayMode(_con[1], (full ? CONSOLE_FULLSCREEN_MODE : CONSOLE_WINDOWED_MODE), nullptr);
	} else
	{
		//SetConsoleDisplayMode(_con[_buff], (full ? CONSOLE_FULLSCREEN_MODE : CONSOLE_WINDOWED_MODE), nullptr);
		SetConsoleDisplayMode(_con[0], (full ? CONSOLE_FULLSCREEN_MODE : CONSOLE_WINDOWED_MODE), nullptr);
		SetConsoleDisplayMode(_con[1], (full ? CONSOLE_FULLSCREEN_MODE : CONSOLE_WINDOWED_MODE), nullptr);
		this->_full = full;
		setConsoleSize(sz.X, sz.Y);
	}
	this->_full = full;
}

vector<vector<wstring>> EmConsole::textFileToVector(string file)
{
	vector<vector<wstring>> str;
	textFileToVector(file, str);
	return str;
}

//Sets title
void EmConsole::setTitle(string title)
{
	SetConsoleTitleA(title.c_str());
}

//Sets the console size
void EmConsole::setConsoleSize(float width, float height)
{
	if(!_full)
	{
		_sz = GetLargestConsoleWindowSize(GetStdHandle(STD_OUTPUT_HANDLE));//largest buffer size
		static SMALL_RECT _SIZE;

		//window dimentions
		_SIZE.Top = 0;
		_SIZE.Left = 0;
		_SIZE.Bottom = height < _sz.Y ? height - 1 : _sz.Y - 1;
		_SIZE.Right = width < _sz.X ? width - 1 : _sz.X - 1;


		SetConsoleScreenBufferSize(_con[!_buff], _sz);
		SetConsoleWindowInfo(_con[!_buff], true, &_SIZE);

		SetConsoleScreenBufferSize(_con[_buff], _sz);
		SetConsoleWindowInfo(_con[_buff], true, &_SIZE);

		_conWidth = _SIZE.Right;
		_conHeight = _SIZE.Bottom;
	}
}

//Gets console's width and height in character blocks
COORD EmConsole::getConsoleSize()
{

	CONSOLE_SCREEN_BUFFER_INFO csbi;
	//COORD size;
	//int columns, rows;

	if(!_full)
	{
		GetConsoleScreenBufferInfo(_con[!_buff], &csbi);

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

void EmConsole::printf(const char *print ...)
{

	static int printNum = 0;
	wchar_t *buff = new wchar_t[128], *buffTmp;
	va_list arg;
	va_start(arg, print);
	vswprintf(buff, 128, L"%p", arg);
	va_end(arg);


	toConsoleBuffer(buff, printNum++, 0);
	delete buff;
}

//Sets weather or not the window can be resized
void EmConsole::setResizable(bool resz)
{
	_resizable = resz;
}

//Sets console position
void EmConsole::setConsolePosition(float x, float y)
{
	SetWindowPos(GetConsoleWindow(), NULL, x, y, NULL, NULL, SWP_NOSIZE | SWP_NOZORDER);
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
	SetWindowPos(GetConsoleWindow(), NULL, (dw.right / 2) - (cw.right / 2), floor((float) dw.bottom / 2) - floor((float)cw.bottom / 2), NULL, NULL, SWP_NOSIZE | SWP_NOZORDER);
}

//Sets console cursor position
void EmConsole::consoleCursorPosition(float x, float y)
{
	_cursorPosition.X = x;
	_cursorPosition.Y = y;
	CONSOLE_CURSOR_INFO info;
	info.bVisible = false;
	info.dwSize = 1;
	SetConsoleCursorInfo(_con[!_buff], &info);
	SetConsoleCursorPosition(_con[!_buff], _cursorPosition);
	//SetConsoleCursorInfo(con[1], &info);
	//SetConsoleCursorPosition(con[1], cursorPosition);
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
void EmConsole::textFileToVector(string file, vector<vector<wstring>> &str)
{

	wfstream ascii(file, wfstream::in);

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
			} else if(line.size() > 0)
				str[a].push_back(line);
			//OutputDebugString((line + "\n").c_str());
		}
	} catch(...)
	{
	}

	ascii.close();
}

/*special stuff for tetris (don't even try to understand)*/
vector<int> EmConsole::readConsoleLineAtributes(float x, float y, float width)
{
	SMALL_RECT strSize;
	strSize.Top = y;
	strSize.Bottom = y + 1;
	strSize.Left = x;
	strSize.Right = x + width;

	//LPWORD a = new WORD[width];
	_ci = new CHAR_INFO[width];
	ReadConsoleOutputA(_con[_buff], _ci, {(SHORT) width,1}, {(SHORT) 0,(SHORT) 0}, &strSize);

	vector<int>c;
	for(int d = 0; d < width; c.push_back(_ci[d++].Attributes));


	//for(int d = 0; d < width; c.push_back(a[d++]))
	//	OutputDebugStringA((to_string(a[d])+", ").c_str());
	//OutputDebugStringA("\nend\n");
	delete[] _ci;
	return c;

}

vector<vector<int>> EmConsole::readConsoleAreaAtributes(float x, float y, float width, float height)
{
	vector<vector<int>>ans;
	for(int a = 0; a < height; a++)
		ans.push_back(readConsoleLineAtributes(x, y++, width));
	//	OutputDebugStringA("\nend Area\n\n\n");
	return ans;
}

wstring EmConsole::readConsoleLine(float x, float y, float width)
{
	SMALL_RECT strSize;
	strSize.Top = y;
	strSize.Bottom = y + 1;
	strSize.Left = x;
	strSize.Right = x + width;

	_ci = new CHAR_INFO[width];
	ReadConsoleOutputA(_con[_buff], _ci, {(SHORT) width,1}, {(SHORT) 0,(SHORT) 0}, &strSize);

	//vector<int>e;
	//for(int d = 0; d < width; e.push_back(ci[d++].Attributes))
	//	OutputDebugStringA((to_string(ci[d].Attributes) + ", ").c_str());
//	OutputDebugStringA("\nend\n");
	wstring c;

	for(int a = 0; a < width; c += _ci[a++].Char.UnicodeChar);
	delete[]_ci;
	return c;

}

vector<wstring> EmConsole::readConsoleArea(float x, float y, float width, float height)
{
	vector<wstring>ans;
	for(int a = 0; a < height; a++)
		ans.push_back(readConsoleLine(x, y++, width));
	return ans;
}

char EmConsole::readConsoleCharacter(float x, float y)
{
	return readConsoleLine(x, y, 1)[0];
}

char EmConsole::readActiveConsoleCharacter(float x, float y)
{
	SMALL_RECT strSize;
	strSize.Top = y;
	strSize.Bottom = y + 1;
	strSize.Left = x;
	strSize.Right = x + 1;

	_ci = new CHAR_INFO;
	ReadConsoleOutputA(_con[!_buff], _ci, {(SHORT) 1,1}, {(SHORT) 0,(SHORT) 0}, &strSize);

	//vector<int>e;
	//for(int d = 0; d < width; e.push_back(ci[d++].Attributes))
	//	OutputDebugStringA((to_string(ci[d].Attributes) + ", ").c_str());
	//	OutputDebugStringA("\nend\n");
	char c;

	c = _ci->Char.UnicodeChar;
	delete _ci;
	return c;
}


/*Console buffer overloads (overkill)*/

/*
Note: NS means No Spaces, wrighting only characters excluding spaces.
Extrymely inefficient
*/

/*
toConsoleBuffer(string &str, float poX, float poY, float x, float y, int colour);
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
void EmConsole::toConsoleBufferNS(wstring &str, float& poX, float& poY, float x, float y, int colour)
{
	consoleCursorPosition(0, 0);
	SMALL_RECT strSize;
	strSize.Top = y;
	strSize.Bottom = y + 1;
	strSize.Left = x;
	strSize.Right = x + 1;

	poX = poX < 0 ? 0 : poX;
	poY = poY < 0 ? 0 : poY;

	_ci = new CHAR_INFO;

	for(int a = 0; a < str.size(); a++)
	{
		if(str[a] != L' ')
		{
			_ci->Char.UnicodeChar = str[a];
			_ci->Attributes = colour;
			WriteConsoleOutputW(_con[_buff], _ci, {1,1}, COORD{(SHORT) poX , (SHORT) poY}, &strSize);
		}
		strSize.Left = ++x;
		strSize.Right = x + 1;
	}

	delete _ci;
}

/*
toConsoleBuffer(string &str, float poX, float poY, float x, float y);
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
void EmConsole::toConsoleBufferNS(wstring &str, float& poX, float& poY, float x, float y)
{
	toConsoleBufferNS(str, poX, poY, x, y, FG_WHITE);
}

/*
toConsoleBuffer(string &str, float x, float y, int colour);
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
void EmConsole::toConsoleBufferNS(wstring &str, float x, float y, int colour)
{
	float poX = 0.f, poY = 0.f;
	toConsoleBufferNS(str, poX, poY, x, y, colour);
}

/*
toConsoleBuffer(string &str, float x, float y);
* str    - string to be drawn
* x      - x position from the top left corner of the screen
* y      - y position from the top left corner of the screen

summary:
Adds single line to the console to the
specified (x,y) position from the top left corner
to be drawn after drawConsole(); is called
*/
void EmConsole::toConsoleBufferNS(wstring &str, float x, float y)
{
	toConsoleBufferNS(str, x, y, FG_WHITE);
}

/*
toConsoleBuffer(Sprite& str, float poX, float poY, float x, float y, int colour);
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
void EmConsole::toConsoleBufferNS(Sprite& str, float& poX, float& poY, float x, float y, int colour)
{
	for(int a = 0; a < str.getHeight(); a++)
		toConsoleBufferNS(str.getSprite()[a], poX, poY, x, y++, colour);
}

/*
toConsoleBuffer(Sprite& str, float poX, float poY, float x, float y);
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
void EmConsole::toConsoleBufferNS(Sprite& str, float& poX, float& poY, float x, float y)
{
	for(int a = 0; a < str.getHeight(); a++)
		toConsoleBufferNS(str.getSprite()[a], poX, poY, x, y++);
}

/*
toConsoleBuffer(Sprite& str, float x, float y, int colour);
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
void EmConsole::toConsoleBufferNS(Sprite& str, float x, float y, int colour)
{
	for(int a = 0; a < str.getHeight(); a++)
		toConsoleBufferNS(str.getSprite()[a], x, y++, colour);
}

/*
toConsoleBuffer(Sprite& str, float x, float y);
* str    - vector of string to be drawn to buffer
* x      - x position from the top left corner of the screen
* y      - y position from the top left corner of the screen

summary:
Adds multiple lines to the console starting at the
specified (x,y) position from the top left corner
to be drawn after drawConsole(); is called
*/
void EmConsole::toConsoleBufferNS(Sprite& str, float x, float y)
{
	for(int a = 0; a < str.getHeight(); a++)
		toConsoleBufferNS(str.getSprite()[a], x, y++);
}

/*
toConsoleBuffer(const char* str, float poX, float poY, float x, float y, int colour);
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
void EmConsole::toConsoleBufferNS(const wchar_t* str, float& poX, float& poY, float x, float y, int colour)
{
	wstring* str2 = new wstring(str);
	toConsoleBufferNS(*str2, poX, poY, x, y, colour);
	delete str2;
}

/*
toConsoleBuffer(const char* str, float poX, float poY, float x, float y);
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
void EmConsole::toConsoleBufferNS(const wchar_t* str, float& poX, float& poY, float x, float y)
{
	wstring* str2 = new wstring(str);
	toConsoleBufferNS(*str2, poX, poY, x, y, FG_WHITE);
	delete str2;
}

/*
toConsoleBuffer(const wchar_t *str, float x, float y, int colour);
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
void EmConsole::toConsoleBufferNS(const wchar_t *str, float x, float y, int colour)
{
	wstring* str2 = new wstring(str);
	toConsoleBufferNS(*str2, x, y, colour);
	delete str2;
}

/*
toConsoleBuffer(const wchar_t *str, float x, float y);
* str    - string to be drawn to buffer
* x      - x position from the top left corner of the screen
* y      - y position from the top left corner of the screen

summary:
Adds single line to the console to the
specified (x,y) position from the specified top left corner
to be drawn after drawConsole(); is called
*/
void EmConsole::toConsoleBufferNS(const wchar_t *str, float x, float y)
{
	wstring* str2 = new wstring(str);
	toConsoleBufferNS(*str2, x, y);
	delete str2;
}

/*
toConsoleBuffer(Sprite& str, float poX, float poY, float x, float y, int colour);
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
void EmConsole::toConsoleBuffer(Sprite& str, float x, float y, vector<vector<int>> &colour)
{
	float p1 = 0, p2 = 0;
	wstring str2;
	for(int a = 0; a < str.getHeight(); a++)
		toConsoleBuffer(str2 = str.getSprite()[a], p1, p2, x, y++, colour[a]);
}

/*
toConsoleBuffer(Sprite& str, float poX, float poY, float x, float y, int colour);
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
void EmConsole::toConsoleBuffer(Sprite& str, float& poX, float& poY, float x, float y, int colour)
{
	//for(int a = 0; a < str.getHeight(); a++)
	//	toConsoleBuffer(str.getSprite()[a], poX, poY, x, y++, colour);

	consoleCursorPosition(0, 0);

	SMALL_RECT strSize;
	strSize.Top = y;
	strSize.Bottom = y + str.getHeight();
	strSize.Left = x;
	strSize.Right = x + str.getWidth() - 1;

	poX = poX < 0 ? 0 : poX;
	poY = poY < 0 ? 0 : poY;

	_ci = new CHAR_INFO[str.getWidth()*str.getHeight()];

	for(int a = 0; a < str.getHeight(); a++)
	{
		for(int b = 0; b < str.getWidth(); b++)
		{
			if(b < str.getSprite()[a].size())
				_ci[b + str.getWidth()*a].Char.UnicodeChar = str.getSprite()[a][b];
			else
				_ci[b + str.getWidth()*a].Char.UnicodeChar = L' ';
			_ci[b + str.getWidth()*a].Attributes = colour;
		}
	}

	WriteConsoleOutputW(_con[_buff], _ci, {(SHORT) str.getWidth() ,(SHORT) str.getHeight()}, COORD{(SHORT) poX , (SHORT) poY}, &strSize);


	delete[] _ci;
}

/*
toConsoleBuffer(Sprite& str, float poX, float poY, float x, float y);
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
void EmConsole::toConsoleBuffer(Sprite& str, float& poX, float& poY, float x, float y)
{
	//for(int a = 0; a < str.getHeight(); a++)
	//	toConsoleBuffer(str.getSprite()[a], poX, poY, x, y++);
	toConsoleBuffer(str, poX, poY, x, y, FG_WHITE);
}

/*
toConsoleBuffer(Sprite& str, float x, float y, int colour);
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
void EmConsole::toConsoleBuffer(Sprite& str, float x, float y, int colour)
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
	//_ci = new CHAR_INFO[str.getWidth()*str.getHeight()];
	//
	//for(int a = 0; a < str.getHeight(); a++)
	//{
	//	for(int b = 0; b < str.getWidth(); b++)
	//	{
	//		if(b < str.getSprite()[a].size())
	//			_ci[b + str.getWidth()*a].Char.UnicodeChar = str.getSprite()[a][b];
	//		else
	//			_ci[b + str.getWidth()*a].Char.UnicodeChar = ' ';
	//		_ci[b + str.getWidth()*a].Attributes = colour;
	//	}
	//}
	//
	//WriteConsoleOutputW(_con[_buff], _ci, {(SHORT)str.getWidth() ,(SHORT)str.getHeight()}, COORD {(SHORT)0 , (SHORT)0}, &strSize);
	//
	//
	//delete[] _ci;
}

/*
toConsoleBuffer(Sprite& str, float x, float y);
* str    - vector of string to be drawn to buffer
* x      - x position from the top left corner of the screen
* y      - y position from the top left corner of the screen

summary:
Adds multiple lines to the console starting at the
specified (x,y) position from the top left corner
to be drawn after drawConsole(); is called
*/
void EmConsole::toConsoleBuffer(Sprite& str, float x, float y)
{
	//for(int a = 0; a < str.getHeight(); a++)
	//	toConsoleBuffer(str.getSprite()[a], x, y++);
	toConsoleBuffer(str, x, y, FG_WHITE);
}

/*
toConsoleBuffer(const wchar_t* str, float poX, float poY, float x, float y, int colour);
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
void EmConsole::toConsoleBuffer(const wchar_t* str, float& poX, float& poY, float x, float y, int colour)
{
	//string* str2 = new string(str);
	//toConsoleBuffer(*str2, poX, poY, x, y, colour);
	//delete str2;

	consoleCursorPosition(0, 0);

	SMALL_RECT strSize;
	strSize.Top = y;
	strSize.Bottom = y + 1;
	strSize.Left = x;
	strSize.Right = x + wcslen(str) - 1;

	poX = poX < 0 ? 0 : poX;
	poY = poY < 0 ? 0 : poY;

	_ci = new CHAR_INFO[wcslen(str)];

	for(int a = 0; a < wcslen(str); a++)
	{
		_ci[a].Char.UnicodeChar = str[a];
		_ci[a].Attributes = colour;
	}

	WriteConsoleOutputW(_con[_buff], _ci, {(SHORT) wcslen(str),1}, COORD{(SHORT) poX , (SHORT) poY}, &strSize);
	delete[] _ci;
}

/*
toConsoleBuffer(const wchar_t* str, float poX, float poY, float x, float y);
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
void EmConsole::toConsoleBuffer(const wchar_t* str, float& poX, float& poY, float x, float y)
{
	wstring* str2 = new wstring(str);
	toConsoleBuffer(*str2, poX, poY, x, y, FG_WHITE);
	delete str2;
}

/*
toConsoleBuffer(const wchar_t *str, float x, float y, int colour);
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
void EmConsole::toConsoleBuffer(const wchar_t *str, float x, float y, int colour)
{
	wstring* str2 = new wstring(str);
	toConsoleBuffer(*str2, x, y, colour);
	delete str2;
}

/*
toConsoleBuffer(const wchar_t *str, float x, float y);
* str    - string to be drawn to buffer
* x      - x position from the top left corner of the screen
* y      - y position from the top left corner of the screen

summary:
Adds single line to the console to the
specified (x,y) position from the specified top left corner
to be drawn after drawConsole(); is called
*/
void EmConsole::toConsoleBuffer(const wchar_t *str, float x, float y)
{
	wstring* str2 = new wstring(str);
	toConsoleBuffer(*str2, x, y);
	delete str2;
}

/*
toConsoleBuffer(string &str, float poX, float poY, float x, float y, int colour);
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
void EmConsole::toConsoleBuffer(wstring &str, float& poX, float& poY, float x, float y, vector<int> &colour)
{
	consoleCursorPosition(0, 0);

	SMALL_RECT strSize;
	strSize.Top = y;
	strSize.Bottom = y + 1;
	strSize.Left = x;
	strSize.Right = x + str.size() - 1;

	poX = poX < 0 ? 0 : poX;
	poY = poY < 0 ? 0 : poY;

	_ci = new CHAR_INFO[str.size()];

	for(int a = 0; a < str.size(); a++)
	{
		_ci[a].Char.UnicodeChar = str[a];
		_ci[a].Attributes = colour[a];
	}

	WriteConsoleOutputW(_con[_buff], _ci, {(SHORT) str.size(),1}, COORD{(SHORT) poX , (SHORT) poY}, &strSize);
	delete[] _ci;
}

/*
toConsoleBuffer(string &str, float poX, float poY, float x, float y, int colour);
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
void EmConsole::toConsoleBuffer(wstring &str, float& poX, float& poY, float x, float y, int colour)
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
toConsoleBuffer(string &str, float poX, float poY, float x, float y);
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
void EmConsole::toConsoleBuffer(wstring &str, float& poX, float& poY, float x, float y)
{
	toConsoleBuffer(str, poX, poY, x, y, FG_WHITE);
}

/*
toConsoleBuffer(string &str, float x, float y, int colour);
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
void EmConsole::toConsoleBuffer(wstring &str, float x, float y, int colour)
{
	float poX = 0.f, poY = 0.f;
	toConsoleBuffer(str, poX, poY, x, y, colour);
}

/*
toConsoleBuffer(string &str, float x, float y);
* str    - string to be drawn
* x      - x position from the top left corner of the screen
* y      - y position from the top left corner of the screen

summary:
Adds single line to the console to the
specified (x,y) position from the top left corner
to be drawn after drawConsole(); is called
*/
void EmConsole::toConsoleBuffer(wstring &str, float x, float y)
{
	toConsoleBuffer(str, x, y, FG_WHITE);
}


/*Draws the console (choose to clear console buffer after draw)
*Note:
	Default is true
*/
void EmConsole::drawConsole(bool clear)
{
	if(!_full)
		if(_resizable)
			setConsoleSize(getWidth(), getHeight());
		else
			setConsoleSize(_conWidth + 1, _conHeight + 1);

	if(!clear)
	{
		SetConsoleActiveScreenBuffer(_con[_buff]);

		//have not tested yet
		//swap(_con[_buff], _con[!_buff]);

		//tryed and true
		HANDLE temp = _con[_buff];
		_con[_buff] = _con[!_buff];
		_con[!_buff] = temp;
		_buff = !_buff;
	} else
	{
		SetConsoleActiveScreenBuffer(_con[_buff]);
		_buff = !_buff;
		clearConsole();
	}
}


/*Clears the console*/
void EmConsole::clearConsole()
{
	static DWORD _cCharsWritten;
	COORD L = GetLargestConsoleWindowSize(GetStdHandle(STD_OUTPUT_HANDLE));
	FillConsoleOutputCharacterA(_con[_buff], ' ', L.X * L.Y, COORD{0,0}, &_cCharsWritten);
	FillConsoleOutputAttribute(_con[_buff], NULL, L.X * L.Y, COORD{0,0}, &_cCharsWritten);
}
