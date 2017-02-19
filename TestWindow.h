#ifndef __TESTWINDOW_H__
#define __TESTWINDOW_H__

#include "Window.h"

class Hierarchy;

class TestWindow : public Window
{
public:
	TestWindow();
	~TestWindow();

	void Draw(ImGuiWindowFlags flags);
	Hierarchy* hierarchy = nullptr;
};

#endif
