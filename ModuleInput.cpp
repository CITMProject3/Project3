#include "Globals.h"
#include "Application.h"
#include "ModuleInput.h"

#include "Imgui\imgui.h"
#include "Imgui\imgui_impl_sdl_gl3.h"

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

			mouse_x_motion = e.motion.xrel / SCREEN_SIZE;
			mouse_y_motion = e.motion.yrel / SCREEN_SIZE;
			break;

			case SDL_QUIT:
			quit = true;
			break;

			case SDL_WINDOWEVENT:
				if(e.window.event == SDL_WINDOWEVENT_RESIZED)
					App->renderer3D->OnResize(e.window.data1, e.window.data2, 60.0f);
			break;

			case SDL_DROPFILE:
				char* file_dropped = e.drop.file;
				App->resource_manager->FileDropped(file_dropped);
				SDL_free(file_dropped);
				break;
			
		}
	}

	if ((quit == true || keyboard[SDL_SCANCODE_ESCAPE] == KEY_UP) && !wants_to_quit)
		wants_to_quit = true;

	ImGui_ImplSdlGL3_NewFrame(App->window->window);

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