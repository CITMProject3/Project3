#ifndef __HARDWARE_INFO_H__
#define __HARDWARE_INFO_H__

#include "Window.h"

#define TEXT_COLORED ImVec4(0.24,0.49,0.88,1)

class HardwareInfo : public Window
{
public:
	HardwareInfo();
	~HardwareInfo();

	void Draw();
};

#endif
