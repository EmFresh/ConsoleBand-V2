#include "XinputManager.h"

XinputDevice XinputManager::controllers[4];

bool XinputManager::controllerConnected(int index)
{
	XINPUT_STATE connected;
	return XInputGetState(index, &connected) == ERROR_SUCCESS;
}

CONTROLLER_TYPE XinputManager::getControllerType(int index)
{
	XINPUT_CAPABILITIES info;
	XInputGetCapabilities(index, NULL, &info);

	switch (info.SubType)
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

