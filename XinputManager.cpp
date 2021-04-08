#include "XinputManager.h"

XinputDevice XinputManager::controllers[4];

void XinputManager::update()
{

	for(int index = 0; index < 4; index++)
	{
		controllers[index].index = index;
		if(controllerConnected(index))
			controllers[index].update();
	}
}

bool XinputManager::controllerConnected(int index)
{
	XINPUT_STATE connected;
	return XInputGetState(index, &connected) == ERROR_SUCCESS;
}

CONTROLLER_TYPE XinputManager::getControllerType(int index)
{
	XINPUT_CAPABILITIES info;
	XInputGetCapabilities(index, NULL, &info);

	switch(info.SubType)
	{
	case XINPUT_DEVSUBTYPE_GAMEPAD:
		return XINPUT_CONTROLLER;
	case XINPUT_DEVSUBTYPE_GUITAR:
		return XINPUT_GUITAR;
	case XINPUT_DEVSUBTYPE_GUITAR_ALTERNATE:
		return XINPUT_GUITAR;
	case XINPUT_DEVSUBTYPE_GUITAR_BASS:
		return XINPUT_GUITAR;
	case XINPUT_DEVSUBTYPE_DRUM_KIT:
		return XINPUT_DRUM;
	}

	return XINPUT_UNKNOWN;
}

XinputDevice* XinputManager::getController(int index)
{
	return &controllers[index];
}

bool XinputManager::isAnyButtonPressed(int bitmask)
{
	for(auto& a : controllers)
		if(a.isButtonPressed(bitmask))
			return true;
	return false;
}

bool XinputManager::isAnyButtonReleased(int bitmask)
{
	for(auto& a : controllers)
		if(a.isButtonReleased(bitmask))
			return true;
	return false;
}

bool XinputManager::isButtonStroked(int bitmask)
{
	for(auto& a : controllers)
		if(a.isButtonReleased(bitmask))
			return true;
	return false;
}