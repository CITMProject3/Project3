#include "stdafx.h"

#include <string>
#include <map>
#include <vector>
#include "../Application.h"
#include "../ModuleScripting.h"
#include "../ModuleInput.h"
#include "../ModuleWindow.h"
#include "../ComponentTransform.h"
#include "../GameObject.h"
#include "../Component.h"
#include "../ComponentScript.h"
#include "../ComponentRectTransform.h"
#include "../SDL/include/SDL_scancode.h"
#include "../Globals.h"
#include "../ComponentCanvas.h"

namespace Character_Selection_UI
{
	GameObject* grid_drivers = nullptr;
	GameObject* grid_sup = nullptr;
	GameObject* driver1 = nullptr;
	GameObject* support1 = nullptr;
	GameObject* driver2 = nullptr;
	GameObject* support2 = nullptr;


	void Character_Selection_UI_GetPublics(map<const char*, string>* public_chars, map<const char*, int>* public_ints, map<const char*, float>* public_float, map<const char*, bool>* public_bools, map<const char*, GameObject*>* public_gos)
	{
		
	}

	void Character_Selection_UI_UpdatePublics(GameObject* game_object)
	{

	}

	void Character_Selection_UI_ActualizePublics(GameObject* game_object)
	{
	
	}

	void Character_Selection_UI_UpdatePublics(GameObject* game_object);

	void Character_Selection_UI_Start(GameObject* game_object)
	{
	
	}

	void Character_Selection_UI_Update(GameObject* game_object)
	{
	}
}