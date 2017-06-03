#include "ModuleWindow.h"

#include "SDL/include/SDL.h"

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
		flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;

		//Use OpenGL 2.1
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

		if (config.GetBool("win_fullscreen")) flags |= SDL_WINDOW_FULLSCREEN;
		if (config.GetBool("win_maximized")) flags |= SDL_WINDOW_MAXIMIZED;
		if (config.GetBool("win_resizable")) flags |= SDL_WINDOW_RESIZABLE;
		if (config.GetBool("win_borderless")) flags |= SDL_WINDOW_BORDERLESS;
		if (config.GetBool("win_fullscreen_desktop")) flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
		
		sprintf_s(window_title, 128, config.GetString("window_title"));
		width = config.GetInt("window_width");
		height = config.GetInt("window_height");

		window = SDL_CreateWindow(window_title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, flags);

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

void ModuleWindow::SaveBeforeClosing(Data& data) const
{
	data.AppendString("window_title", window_title);

	data.AppendInt("window_width", width);
	data.AppendInt("window_height", height);

	data.AppendBool("win_fullscreen", (flags & SDL_WINDOW_FULLSCREEN) ? true : false);
	data.AppendBool("win_maximized", (flags & SDL_WINDOW_MAXIMIZED) ? true : false);
	data.AppendBool("win_resizable", (flags & SDL_WINDOW_RESIZABLE) ? true : false);
	data.AppendBool("win_borderless", (flags & SDL_WINDOW_BORDERLESS) ? true : false);
	data.AppendBool("win_fullscreen_desktop", (flags & SDL_WINDOW_FULLSCREEN_DESKTOP) ? true : false);
}