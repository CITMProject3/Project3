#include "Application.h"
#include "ModuleInput.h"
#include "ModuleWindow.h"
#include "ModuleResourceManager.h"
#include "ModuleRenderer3D.h"
#include "ModuleEditor.h"
#include "ModuleGOManager.h"
#include "ComponentCanvas.h"
#include "SDL/include/SDL.h"

#include "Imgui\imgui.h"
#include "Imgui\imgui_impl_sdl_gl3.h"
#include "imgui\imgui_internal.h"

#define MAX_KEYS 300

ModuleInput::ModuleInput(const char* name, bool start_enabled) : Module(name, start_enabled)
{
	keyboard = new KEY_STATE[MAX_KEYS];
	memset(keyboard, KEY_IDLE, sizeof(KEY_STATE) * MAX_KEYS);
	memset(mouse_buttons, KEY_IDLE, sizeof(KEY_STATE) * MAX_MOUSE_BUTTONS);
}

// Destructor
ModuleInput::~ModuleInput()
{
	delete[] keyboard;
}

// Called before render is available
bool ModuleInput::Init(Data& config)
{
	LOG("Init SDL input event system");
	bool ret = true;
	SDL_Init(0);

	if(SDL_InitSubSystem(SDL_INIT_EVENTS) < 0)
	{
		LOG("SDL_EVENTS could not initialize! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}

	if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) < 0)
	{
		LOG("SDL_JOYSTICK could not initialize! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}

	num_joysticks = SDL_NumJoysticks();

	LOG("Joysticks connected: %d", num_joysticks);

	if (num_joysticks > 0)
	{
		SDL_JoystickEventState(SDL_ENABLE);
		for (int i = 0; i < num_joysticks; i++)
		{
			SDL_Joystick* joystick;
			joystick = SDL_JoystickOpen(i);

			JOYSTICK* j = new JOYSTICK();
			j->sdl_joystick = joystick;

			int MAX_JOY_BUTTONS = SDL_JoystickNumButtons(joystick);
			j->button = new KEY_STATE[MAX_JOY_BUTTONS];
			memset(j->button, KEY_IDLE, sizeof(KEY_STATE) * MAX_JOY_BUTTONS);

			int MAX_JOY_AXES = SDL_JoystickNumAxes(joystick);
			j->axis = new Sint16[MAX_JOY_AXES];
			memset(j->axis, 0, sizeof(Sint16) * MAX_JOY_AXES);

			joysticks.push_back(j);
		}
	}

	return ret;
}

// Called every draw update
update_status ModuleInput::PreUpdate()
{
	SDL_PumpEvents();
	bool resetDrag = false;

	const Uint8* keys = SDL_GetKeyboardState(NULL);
	
	for(int i = 0; i < MAX_KEYS; ++i)
	{
		if(keys[i] == 1)
		{
			if(keyboard[i] == KEY_IDLE)
				keyboard[i] = KEY_DOWN;
			else
				keyboard[i] = KEY_REPEAT;
		}
		else
		{
			if(keyboard[i] == KEY_REPEAT || keyboard[i] == KEY_DOWN)
				keyboard[i] = KEY_UP;
			else
				keyboard[i] = KEY_IDLE;
		}
	}

	for (int n = 0; n < num_joysticks; n++)
	{
		int MAX_JOY_BUTTONS = SDL_JoystickNumButtons(joysticks[n]->sdl_joystick);
		int MAX_JOY_AXIS = SDL_JoystickNumAxes(joysticks[n]->sdl_joystick);

		for (int i = 0; i < MAX_JOY_BUTTONS; i++)
		{
			Uint8 joy_button = SDL_JoystickGetButton(joysticks[n]->sdl_joystick, i);
			if (joy_button == 1)
			{
				if (joysticks[n]->button[i] == KEY_IDLE)
					joysticks[n]->button[i] = KEY_DOWN;
				else
					joysticks[n]->button[i] = KEY_REPEAT;
			}
			else
			{
				if (joysticks[n]->button[i] == KEY_REPEAT || joysticks[n]->button[i] == KEY_DOWN)
					joysticks[n]->button[i] = KEY_UP;
				else
					joysticks[n]->button[i] = KEY_IDLE;
			}
		}

		for (int i = 0; i < MAX_JOY_AXIS; i++)
		{
			Sint16 axis = SDL_JoystickGetAxis(joysticks[n]->sdl_joystick, i);
			joysticks[n]->axis[i] = axis;
		}
	}

	Uint32 buttons = SDL_GetMouseState(&mouse_x, &mouse_y);

	mouse_x /= SCREEN_SIZE;
	mouse_y /= SCREEN_SIZE;
	mouse_z = 0;

	for(int i = 0; i < 5; ++i)
	{
		if(buttons & SDL_BUTTON(i))
		{
			if(mouse_buttons[i] == KEY_IDLE)
				mouse_buttons[i] = KEY_DOWN;
			else
				mouse_buttons[i] = KEY_REPEAT;
		}
		else
		{
			if(mouse_buttons[i] == KEY_REPEAT || mouse_buttons[i] == KEY_DOWN)
				mouse_buttons[i] = KEY_UP;
			else
				mouse_buttons[i] = KEY_IDLE;
		}
	}

	mouse_x_motion = mouse_y_motion = 0;

	bool quit = false;
	SDL_Event e;
	list<string> files_dropped;
	while(SDL_PollEvent(&e))
	{
		ImGui_ImplSdlGL3_ProcessEvent(&e);
		switch(e.type)
		{
			case SDL_MOUSEWHEEL:
			mouse_z = e.wheel.y;
			break;

			case SDL_MOUSEMOTION:
			mouse_x = e.motion.x / SCREEN_SIZE;
			mouse_y = e.motion.y / SCREEN_SIZE;

			mouse_x_motion = e.motion.xrel / SCREEN_SIZE - last_mouse_swap;
			mouse_y_motion = e.motion.yrel / SCREEN_SIZE;

			if (infiniteHorizontal)
			{
				if (mouse_x > App->window->GetScreenWidth() - 10)
				{
					int last_x = mouse_x;
					App->input->SetMouseX(10);
					last_mouse_swap = mouse_x - last_x;
					resetDrag = true;
				}
				else if (mouse_x < 10)
				{
					int last_x = mouse_x;
					App->input->SetMouseX(App->window->GetScreenWidth() - 10);
					last_mouse_swap = mouse_x - last_x;
					resetDrag = true;
				}
				else
					last_mouse_swap = 0;
			}
			else
			{
				last_mouse_swap = 0;
			}

			break;

			case SDL_QUIT:
			quit = true;
			break;

			case SDL_WINDOWEVENT:
				if (e.window.event == SDL_WINDOWEVENT_RESIZED)
				{
					App->renderer3D->OnResize(e.window.data1, e.window.data2, 60.0f);
					App->editor->OnResize(e.window.data1, e.window.data2);

					if(App->go_manager->current_scene_canvas)
						App->go_manager->current_scene_canvas->ResizeScreen();
				}
			break;

			case SDL_JOYDEVICEADDED:
				CheckJoystick();
				SDL_FlushEvent(SDL_JOYDEVICEADDED);
			break;
			
			case SDL_JOYDEVICEREMOVED:
				CheckJoystick();
				SDL_FlushEvent(SDL_JOYDEVICEREMOVED);
			break;

			case SDL_DROPFILE:
				char* file_dropped = e.drop.file;
				files_dropped.push_back(file_dropped);
				SDL_free(file_dropped);
			break;

		}
	}

	if (files_dropped.size() > 0)
		App->resource_manager->InputFileDropped(files_dropped);

	if ((quit == true || keyboard[SDL_SCANCODE_ESCAPE] == KEY_UP) && !wants_to_quit)
		wants_to_quit = true;

	ImGui_ImplSdlGL3_NewFrame(App->window->window);
	if (resetDrag == true)
		ResetImGuiDrag();
	infiniteHorizontal = false;

	return UPDATE_CONTINUE;
}

// Called before quitting
bool ModuleInput::CleanUp()
{
	LOG("Quitting SDL input event subsystem");

	SDL_QuitSubSystem(SDL_INIT_EVENTS);
	SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
	return true;
}

KEY_STATE ModuleInput::GetKey(int id) const
{
	//Warning: noone can recieve input if editor is using keyboard
	if (App->editor->UsingKeyboard() == false)
		return keyboard[id];
	else
		return KEY_IDLE;
}

KEY_STATE ModuleInput::GetMouseButton(int id) const
{
	//Warning: noone can recieve input if editor is using mouse
	if (App->editor->UsingMouse() == false)
		return mouse_buttons[id];
	else
		return KEY_IDLE;
}

void ModuleInput::SetMouseX(int x)
{
	SDL_WarpMouseInWindow(App->window->window, x, mouse_y);
	mouse_x = x;
}

void ModuleInput::SetMouseY(int y)
{
	SDL_WarpMouseInWindow(App->window->window, mouse_x, y);
	mouse_y = y;
}

int ModuleInput::GetMouseX() const
{
	return mouse_x;
}

int ModuleInput::GetMouseY() const
{
	return mouse_y;
}

int ModuleInput::GetMouseXMotion() const
{
	return mouse_x_motion;
}

int ModuleInput::GetMouseYMotion() const
{
	return mouse_y_motion;
}

int ModuleInput::GetMouseZ() const
{
	if (App->editor->UsingMouse() == false)
		return mouse_z;
	else
		return 0;
}

KEY_STATE ModuleInput::GetJoystickButton(unsigned int joy, JOY_BUTTON id) const
{
	if (joy < joysticks.size() && joy >= 0)
	{
		return joysticks[joy]->button[id];
	}
	return KEY_IDLE;
}

float ModuleInput::GetJoystickAxis(unsigned int joy, JOY_AXIS id) const //From -1.0f to 1.0f
{
	if (joy < joysticks.size() && joy >= 0)
	{
		float ret = (float)joysticks[joy]->axis[id] / 32768;

		if (ret < TOLERANCE && ret > TOLERANCE) ret = 0;

		return ret;
	}
	return 0.0f;
}

int ModuleInput::GetNumberJoysticks() const
{
	return num_joysticks;
}

bool ModuleInput::Quit() const
{
	return wants_to_quit;
}

void ModuleInput::SetQuit()
{
	wants_to_quit = true;
}

void ModuleInput::ResetQuit()
{
	wants_to_quit = false;
}

void ModuleInput::InfiniteHorizontal()
{
	infiniteHorizontal = true;
}

void ModuleInput::ResetImGuiDrag()
{
	ImGui::GetIO().MousePos.x = mouse_x;
	ImGui::GetIO().MousePos.y = mouse_y;

	ImGui::ResetMouseDragDelta(0);

	ImGui::GetCurrentContext()->ActiveIdIsJustActivated = true;
}

void ModuleInput::CheckJoystick()
{
	SDL_QuitSubSystem(SDL_INIT_JOYSTICK);

	joysticks.clear();
	
	if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) < 0)
	{
		LOG("SDL_JOYSTICK could not initialize! SDL_Error: %s\n", SDL_GetError());
	}

	num_joysticks = SDL_NumJoysticks();

	LOG("Joysticks connected: %d", num_joysticks);

	if (num_joysticks > 0)
	{
		SDL_JoystickEventState(SDL_ENABLE);
		for (int i = 0; i < num_joysticks; i++)
		{
			SDL_Joystick* joystick;
			joystick = SDL_JoystickOpen(i);

			JOYSTICK* j = new JOYSTICK();
			j->sdl_joystick = joystick;

			int MAX_JOY_BUTTONS = SDL_JoystickNumButtons(joystick);
			j->button = new KEY_STATE[MAX_JOY_BUTTONS];
			memset(j->button, KEY_IDLE, sizeof(KEY_STATE) * MAX_JOY_BUTTONS);

			int MAX_JOY_AXES = SDL_JoystickNumAxes(joystick);
			j->axis = new Sint16[MAX_JOY_AXES];
			memset(j->axis, 0, sizeof(Sint16) * MAX_JOY_AXES);

			joysticks.push_back(j);
		}
	}
}