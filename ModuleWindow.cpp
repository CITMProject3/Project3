#include "Globals.h"
#include "Application.h"
#include "ModuleWindow.h"

ModuleWindow::ModuleWindow(const char* name, bool start_enabled) : Module(name, start_enabled)
{
	window = NULL;
	screen_surface = NULL;
}

// Destructor
ModuleWindow::~ModuleWindow()
{
}

// Called before render is available
bool ModuleWindow::Init(Data& config)
{
	LOG("Init SDL window & surface");
	bool ret = true;

	if(SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		LOG("SDL_VIDEO could not initialize! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}
	else
	{
		//Create window
		Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;

		//Use OpenGL 2.1
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

		if(WIN_FULLSCREEN == true)
		{
			flags |= SDL_WINDOW_FULLSCREEN;
		}

		if(WIN_RESIZABLE == true)
		{
			flags |= SDL_WINDOW_RESIZABLE;
		}

		if(WIN_BORDERLESS == true)
		{
			flags |= SDL_WINDOW_BORDERLESS;
		}

		if(WIN_FULLSCREEN_DESKTOP == true)
		{
			flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
		}

		window = SDL_CreateWindow(TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, flags);

		if(window == NULL)
		{
			LOG("Window could not be created! SDL_Error: %s\n", SDL_GetError());
			ret = false;
		}
		else
		{
			//Get window surface
			screen_surface = SDL_GetWindowSurface(window);
		}
	}

	return ret;
}

// Called before quitting
bool ModuleWindow::CleanUp()
{
	LOG("Destroying SDL window and quitting all SDL systems");

	//Destroy window
	if(window != NULL)
	{
		SDL_DestroyWindow(window);
	}

	//Quit SDL subsystems
	SDL_Quit();
	return true;
}

void ModuleWindow::SetTitle(const char* title)
{
	SDL_SetWindowTitle(window, title);
}

void ModuleWindow::SetFullScreen(bool value)
{
	Uint32 flag;
	if (value)
		flag = SDL_WINDOW_FULLSCREEN;
	else
		flag = 0;

	SDL_SetWindowFullscreen(window, flag);
}

void ModuleWindow::SetBorderless(bool value)
{
	SDL_bool flag;
	if (value)
		flag = SDL_TRUE;
	else
		flag = SDL_FALSE;
	SDL_SetWindowBordered(window, flag);
}

void ModuleWindow::SetFullDesktop(bool value)
{
	Uint32 flag;
	if (value)
		flag = SDL_WINDOW_FULLSCREEN_DESKTOP;
	else
		flag = 0;

	SDL_SetWindowFullscreen(window, flag);
}

int ModuleWindow::GetScreenWidth() const
{
	return width;
}

int ModuleWindow::GetScreenHeight() const
{
	return height;
}

void ModuleWindow::SetScreenSize(int width, int height)
{
	this->width = width;
	this->height = height;
}

void ModuleWindow::SetResizable(bool value)
{
	//TODO save for next time the engine opens
}