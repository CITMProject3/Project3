#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#include "Window.h"

class Console : public Window
{
public:
	Console();
	~Console();

	void Draw();

	void Write(const char* text);

private:
	ImGuiTextBuffer buffer;
	bool                ScrollToBottom;
};

extern Console* console;

#endif
