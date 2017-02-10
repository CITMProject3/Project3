#ifndef __MODULEINPUT_H__
#define __MODULEINPUT_H__

#include "Module.h"
#include "Globals.h"
#include <vector>

#define MAX_MOUSE_BUTTONS 5
#define TOLERANCE 0.001

using namespace std;

enum JOY_BUTTON		//For XBOX controller!!
{
	DPAD_UP = 0,
	DPAD_DOWN,
	DPAD_LEFT,
	DPAD_RIGHT,
	START,
	SELECT,
	L3,
	R3,
	LB,
	RB,
	A,
	B,
	X,
	Y,
	HOME
};

enum JOY_AXIS
{
	LEFT_STICK_X = 0,
	LEFT_STICK_Y,
	RIGHT_STICK_X,
	RIGHT_STICK_Y,
	LEFT_TRIGGER,
	RIGHT_TRIGGER
};

enum KEY_STATE
{
	KEY_IDLE = 0,
	KEY_DOWN,
	KEY_REPEAT,
	KEY_UP
};

struct JOYSTICK
{
	SDL_Joystick* sdl_joystick;
	KEY_STATE*		button;
	Sint16*		axis;
};

class ModuleInput : public Module
{
public:
	
	ModuleInput(const char* name, bool start_enabled = true);
	~ModuleInput();

	bool Init(Data& config);
	update_status PreUpdate();
	bool CleanUp();

	KEY_STATE GetKey(int id) const
	{
		return keyboard[id];
	}

	KEY_STATE GetMouseButton(int id) const
	{
		return mouse_buttons[id];
	}

	int GetMouseX() const
	{
		return mouse_x;
	}

	int GetMouseY() const
	{
		return mouse_y;
	}

	int GetMouseZ() const
	{
		return mouse_z;
	}

	int GetMouseXMotion() const
	{
		return mouse_x_motion;
	}

	int GetMouseYMotion() const
	{
		return mouse_y_motion;
	}

	KEY_STATE GetJoystickButton(int joy, JOY_BUTTON id) const
	{
		return joysticks[joy]->button[id];
	}

	float GetJoystickAxis(int joy, JOY_AXIS id) const //From -1.0f to 1.0f
	{
		float ret = (float)joysticks[joy]->axis[id] / 32768;
		
		if (ret < TOLERANCE && ret > TOLERANCE) ret = 0;

		return ret;
	}

	int GetNumberJoysticks() const
	{
		return num_joysticks;
	}

	bool Quit() const
	{
		return wants_to_quit;
	}

	void ResetQuit()
	{
		wants_to_quit = false;
	}


private:
	KEY_STATE* keyboard;
	KEY_STATE mouse_buttons[MAX_MOUSE_BUTTONS];
	int mouse_x;
	int mouse_y;
	int mouse_z;
	int mouse_x_motion;
	int mouse_y_motion;
	//int mouse_z_motion;

	int num_joysticks = 0;
	vector<JOYSTICK*>	joysticks;

	bool wants_to_quit = false;
};

#endif // !__MODULEINPUT_H__
