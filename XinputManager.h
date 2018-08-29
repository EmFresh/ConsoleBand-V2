#pragma once
#include <Windows.h>
#include <Xinput.h>

struct XinputDevice
{
	XinputDevice() {};
	virtual ~XinputDevice() = 0;
};

struct XinputGuitar:public XinputDevice
{
	XinputGuitar() {};
	~XinputGuitar() {};


};

struct XinputDrum:public XinputDevice
{
	XinputDrum() {};
	~XinputDrum() {};


};

struct XinputController:public XinputDevice
{
	XinputController() {};
	~XinputController() {};


};

class XinputManager
{
public:
	XinputManager();
	~XinputManager();

private:

};

