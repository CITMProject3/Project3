#ifndef __ModuleWindow_H__
#define __ModuleWindow_H__

#include "Module.h"

class Application;

struct SDL_Window;
struct SDL_Surface;

class ModuleWindow : public Module
{
public:

	ModuleWindow(const char* name, bool start_enabled = true);

	// Destructor
	virtual ~ModuleWindow();

	bool Init(Data& config);
	bool CleanUp();

	void SetTitle(const char* title);

	void SetFullScreen(bool value);
	void SetResizable(bool value);
	void SetBorderless(bool value);
	void SetFullDesktop(bool value);

	int GetScreenWidth()const;
	int GetScreenHeight()const;
	void SetScreenSize(int width, int height);

	void SaveBeforeClosing(Data& data) const;

public:

	//The window we'll be rendering to
	SDL_Window* window;

	//The surface contained by the window
	SDL_Surface* screen_surface;

private:

	char window_title[128];
	int width = 0;
	int height = 0;

	unsigned int flags = 0;
};

#endif // __ModuleWindow_H__