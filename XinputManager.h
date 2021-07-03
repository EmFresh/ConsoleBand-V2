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
	CONTROLLER_LSB = XINPUT_GAMEPAD_LEFT_THUMB,
	CONTROLLER_RSB = XINPUT_GAMEPAD_RIGHT_THUMB,
	CONTROLLER_SELECT = XINPUT_GAMEPAD_BACK,
	CONTROLLER_START = XINPUT_GAMEPAD_START
};

//Buttons used for Guitar controllers
enum  GUITAR_INPUT_BUTTONS
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
	GUITAR_BACK = XINPUT_GAMEPAD_BACK,
	GUITAR_START = XINPUT_GAMEPAD_START

};

//Buttons used for Drum controllers
enum DRUM_INPUT_BUTTONS
{
	DRUM_DPAD_UP = XINPUT_GAMEPAD_DPAD_UP,
	DRUM_DPAD_DOWN = XINPUT_GAMEPAD_DPAD_DOWN,
	DRUM_DPAD_LEFT = XINPUT_GAMEPAD_DPAD_LEFT,
	DRUM_DPAD_RIGHT = XINPUT_GAMEPAD_DPAD_RIGHT,
	DRUM_PAD_RED = XINPUT_GAMEPAD_B,
	DRUM_PAD_YELLOW = XINPUT_GAMEPAD_Y,
	DRUM_PAD_BLUE = XINPUT_GAMEPAD_X,
	DRUM_PAD_GREEN = XINPUT_GAMEPAD_A,
	DRUM_KICK_PEDAL = XINPUT_GAMEPAD_LEFT_SHOULDER,
	DRUM_BACK = XINPUT_GAMEPAD_BACK,
	DRUM_START = XINPUT_GAMEPAD_START
};

struct Stick
{
	float x, y;
};

struct Triggers
{
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

	void setVibration(float L, float R)
	{
		XINPUT_VIBRATION vibration;
		ZeroMemory(&vibration, sizeof(XINPUT_VIBRATION));
		vibration.wLeftMotorSpeed = WORD(65535 * L); // use any value between 0-65535 here
		vibration.wRightMotorSpeed = WORD(65535 * R); // use any value between 0-65535 here
		XInputSetState(index, &vibration);
	}

	void setVibrationL(float L)
	{
		XINPUT_VIBRATION vibration;
		ZeroMemory(&vibration, sizeof(XINPUT_VIBRATION));
		setVibration(L, vibration.wRightMotorSpeed);
	}

	void setVibrationR(float R)
	{
		XINPUT_VIBRATION vibration;
		ZeroMemory(&vibration, sizeof(XINPUT_VIBRATION));
		setVibration(vibration.wLeftMotorSpeed, R);
	}

	void resetVibration()
	{
		setVibration(0, 0);
	}

	virtual int getButtonBitmask()
	{
		return info.Gamepad.wButtons > 0 ? info.Gamepad.wButtons : NULL;
	}

	virtual bool isButtonPressed(int bitmask)
	{
		return info.Gamepad.wButtons & bitmask;
	}

	virtual bool isButtonReleased(int bitmask)
	{
		return !isButtonPressed(bitmask);
	}

	virtual bool isButtonStroked(int bitmask)
	{
		if (isButtonPressed(bitmask))
			stroke[bitmask] = true;
		else if (stroke[bitmask] && isButtonReleased(bitmask))
			return ((stroke[bitmask] = false), true);

		return false;
	}

	XINPUT_STATE info = {};
	CONTROLLER_TYPE type = CONTROLLER_TYPE::XINPUT_UNKNOWN;
	int index = 0;
	float deadZoneStick = .03f, deadZoneTrigger = 0;

private:
	std::unordered_map<int, bool> stroke;
};

struct XinputGuitar :public XinputDevice
{
	XinputGuitar() {}

	~XinputGuitar() {}

	int getFrets()
	{
		return info.Gamepad.wButtons;
	}

	bool isSoloButtonPressed(int bitmask) { return bitmask & CONTROLLER_INPUT_BUTTONS::CONTROLLER_LSB; }

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
		return XinputDevice::isButtonStroked(bitmask);
	}

private:
	DWORD frets=0, strumUp=0, strumDown=0;
	float whammyBar=0;
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
	DWORD buttons=0;
	float LT=0, RT=0;
};

class XinputManager
{
public:
	//updates all controllers at once
	static void update();

	//checks if the controller from index 0 -> 3 (inclusive) is connected (up to 4 controllers)
	static bool controllerConnected(int index);

	//gets the type of controller that is connected to the computer 
	static CONTROLLER_TYPE getControllerType(int index);

	//gets the controller from index 0 -> 3 (inclusive)
	static XinputDevice* getController(int index);
private:
	static	XinputDevice controllers[4];
};