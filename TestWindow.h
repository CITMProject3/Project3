#ifndef __TESTWINDOW_H__
#define __TESTWINDOW_H__

#include "Window.h"

class Hierarchy;

class TestWindow : public Window
{
public:
	TestWindow();
	~TestWindow();

	void Draw();
	void DrawResizeWindows();
	void DrawSecondPanel();
	void ChangePanel(int panel);

	Hierarchy* hierarchy = nullptr;
	int current_panel = 0;
	bool set_dimensions = false;
};

#endif
