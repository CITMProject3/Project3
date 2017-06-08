#ifndef __MODULEINPUT_H__
#define __MODULEINPUT_H__

#include "Module.h"
#include <vector>

#define MAX_MOUSE_BUTTONS 5
#define TOLERANCE 0.001

typedef struct _SDL_Joystick SDL_Joystick;
typedef int16_t Sint16;

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

	KEY_STATE GetKey(int id) const;
	KEY_STATE GetMouseButton(int id) const;

	void SetMouseX(int x);
	void SetMouseY(int y);

	int GetMouseX() const;
	int GetMouseY() const;
	int GetMouseZ() const;

	int GetMouseXMotion() const;
	int GetMouseYMotion() const;

	KEY_STATE GetJoystickButton(unsigned int joy, JOY_BUTTON id) const;
	float GetJoystickAxis(unsigned int joy, JOY_AXIS id) const; //From -1.0f to 1.0f
	int GetNumberJoysticks() const;

	bool Quit() const;
	void SetQuit();
	void ResetQuit();

	void InfiniteHorizontal();

private:

	void ResetImGuiDrag();
	void CheckJoystick();

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

	int last_mouse_swap = 0;

	bool infiniteHorizontal = false;
};

#endif // !__MODULEINPUT_H__
