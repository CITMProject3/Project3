
#ifndef __WINDOW_OPTIONS_H__
#define __WINDOW_OPTIONS_H__

#include "Window.h"
#include "Application.h"

class WindowOptions : public Window
{
public:
	WindowOptions();
	~WindowOptions();

	void Draw();
private:

	float brightness;
	bool fullscreen = false;
	bool resizable = false;
	bool full_desktop = false;
	bool borderless = false;
};

#endif
