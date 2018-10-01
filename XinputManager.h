#pragma once
#include <utility>
#include <unordered_map>
#include <Windows.h>
#include <Xinput.h>

enum CONTROLLER_TYPE
{
	XINPUT_CONTROLLER,
	XINPUT_GUITAR,
	XINPUT_DRUM,
	XINPUT_UNKNOWN
};


//Buttons used for Normal controllers
enum CONTROLLER_INPUT_BUTTONS
{
	CONTROLLER_DPAD_UP = XINPUT_GAMEPAD_DPAD_UP,
	CONTROLLER_DPAD_DOWN = XINPUT_GAMEPAD_DPAD_DOWN,
	CONTROLLER_DPAD_LEFT = XINPUT_GAMEPAD_DPAD_LEFT,
	CONTROLLER_DPAD_RIGHT = XINPUT_GAMEPAD_DPAD_RIGHT,
	CONTROLLER_A = XINPUT_GAMEPAD_A,
	CONTROLLER_B = XINPUT_GAMEPAD_B,
	CONTROLLER_X = XINPUT_GAMEPAD_X,
	CONTROLLER_Y = XINPUT_GAMEPAD_Y,
	CONTROLLER_LB = XINPUT_GAMEPAD_LEFT_SHOULDER,
	CONTROLLER_RB = XINPUT_GAMEPAD_RIGHT_SHOULDER,
	CONTROLLER_THUMB_LEFT = XINPUT_GAMEPAD_LEFT_THUMB,
	CONTROLLER_THUMB_RIGHT = XINPUT_GAMEPAD_RIGHT_THUMB,
	CONTROLLER_SELECT = XINPUT_GAMEPAD_BACK,
	CONTROLLER_START = XINPUT_GAMEPAD_START
};

//Buttons used for Guitar controllers
enum GUITAR_INPUT_BUTTONS
{
	GUITAR_DPAD_UP = XINPUT_GAMEPAD_DPAD_UP,
	GUITAR_DPAD_DOWN = XINPUT_GAMEPAD_DPAD_DOWN,
	GUITAR_DPAD_LEFT = XINPUT_GAMEPAD_DPAD_LEFT,
	GUITAR_DPAD_RIGHT = XINPUT_GAMEPAD_DPAD_RIGHT,
	GUITAR_FRET_GREEN = XINPUT_GAMEPAD_A,
	GUITAR_FRET_RED = XINPUT_GAMEPAD_B,
	GUITAR_FRET_YELLOW = XINPUT_GAMEPAD_Y,
	GUITAR_FRET_BLUE = XINPUT_GAMEPAD_X,
	GUITAR_FRET_ORANGE = XINPUT_GAMEPAD_LEFT_SHOULDER,
	GUITAR_STRUM_UP = GUITAR_DPAD_UP,
	GUITAR_STRUM_DOWN = GUITAR_DPAD_DOWN,
	GUITAR_SELECT = XINPUT_GAMEPAD_BACK,
	GUITAR_START = XINPUT_GAMEPAD_START

};

//Buttons used for Drum controllers
enum DRUM_INPUT_BUTTONS
{
	DPAD_UP,
	DPAD_DOWN,
	DPAD_LEFT,
	DPAD_RIGHT,
	PAD_RED,
	PAD_YELLOW,
	PAD_BLUE,
	PAD_GREEN,
	KICK_PEDAL,
	SELECT,
	START
};

struct Stick
{
	float x, y;
};

struct Triggers {
	float L, R;
};

struct XinputDevice
{
	virtual ~XinputDevice() {};

	void setStickDeadZone(float dz) { deadZoneStick = dz; };

	float getStickDeadZone() { return deadZoneStick; };

	void update()
	{
		XInputGetState(index, &info);
	}

	virtual int getButtonBitmask()
	{
		update();
		return info.Gamepad.wButtons > 0 ? info.Gamepad.wButtons : NULL;
	}

	virtual bool isButtonPressed(int bitmask)
	{
		update();
		return info.Gamepad.wButtons == bitmask;
	}

	virtual bool isButtonReleased(int bitmask)
	{
		update();
		return info.Gamepad.wButtons != bitmask;
	}

	virtual bool isButtonStroked(int bitmask)
	{
		update();
		if (isButtonPressed(bitmask))
			stroke[bitmask] = true;

		if (stroke[bitmask] && !isButtonReleased(bitmask))
		{
			stroke[bitmask] = false;
			return true;
		}

		return false;
	}

	XINPUT_STATE info;
	CONTROLLER_TYPE type;
	int index;
	float deadZoneStick = .03f, deadZoneTrigger;

private:
	std::unordered_map<int, bool> stroke;
};

struct XinputGuitar :public XinputDevice
{
	XinputGuitar() {}
	//XinputGuitar(XinputDevice div) :XinputDevice(div) {};
	~XinputGuitar() {}

	int getFrets()
	{
		update();
		return info.Gamepad.wButtons;
	}

	bool isFretPressed(int bitmask)
	{
		return info.Gamepad.wButtons == bitmask;
	}

	bool isFretReleased(int bitmask) 
	{ 
		return info.Gamepad.wButtons != bitmask; 
	}

	bool isFretStroked(int bitmask) 
	{
		return isButtonStroked(bitmask);
	}

private:
	DWORD frets, strumUp, strumDown;
	float whammyBar;
};

struct XinputDrum :public XinputDevice
{
	XinputDrum() {};
	//	XinputDrum(XinputDevice div) :XinputDevice(div) {};
	~XinputDrum() {};

private:
};

struct XinputController :public XinputDevice
{
	XinputController() {};
	~XinputController() {};

	Stick* getSticks(Stick sticks[2])
	{
		update();

		sticks[0].
			x = abs(info.Gamepad.sThumbLX < 0 ? (float)info.Gamepad.sThumbLX / 32768 : (float)info.Gamepad.sThumbLX / 32767) > deadZoneStick ?
			info.Gamepad.sThumbLX < 0 ? (float)info.Gamepad.sThumbLX / 32768 : (float)info.Gamepad.sThumbLX / 32767 : 0;
		sticks[0].
			y = abs(info.Gamepad.sThumbLY < 0 ? (float)info.Gamepad.sThumbLY / 32768 : (float)info.Gamepad.sThumbLY / 32767) > deadZoneStick ?
			info.Gamepad.sThumbLY < 0 ? (float)info.Gamepad.sThumbLY / 32768 : (float)info.Gamepad.sThumbLY / 32767 : 0;

		sticks[1].
			x = abs(info.Gamepad.sThumbRX < 0 ? (float)info.Gamepad.sThumbRX / 32768 : (float)info.Gamepad.sThumbRX / 32767) > deadZoneStick ?
			info.Gamepad.sThumbRX < 0 ? (float)info.Gamepad.sThumbRX / 32768 : (float)info.Gamepad.sThumbRX / 32767 : 0;
		sticks[1].
			y = abs(info.Gamepad.sThumbRY < 0 ? (float)info.Gamepad.sThumbRY / 32768 : (float)info.Gamepad.sThumbRY / 32767) > deadZoneStick ?
			info.Gamepad.sThumbRY < 0 ? (float)info.Gamepad.sThumbRY / 32768 : (float)info.Gamepad.sThumbRY / 32767 : 0;

		
	}

	//
	void getTriggers(Triggers& triggers)
	{
		
		triggers = { (float)info.Gamepad.bLeftTrigger / 255,  (float)info.Gamepad.bRightTrigger / 255 };
	}

private:


	DWORD buttons;
	float LT, RT;
};

class XinputManager
{
public:
	//gets the amount of controllers connected (up to 4) from index 0 -> 3 (inclusive)
	static bool controllerConnected(int index);

	//gets the type of controller that is connected to the computer 
	static CONTROLLER_TYPE getControllerType(int index);

	//gets the controller from index 0 -> 3 (inclusive)
	static XinputDevice* getController(int index);

private:
	static	XinputDevice controllers[4];
};