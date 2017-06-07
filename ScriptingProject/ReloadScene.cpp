#include "stdafx.h"

#include <string>
#include <map>
#include <vector>
#include "../Application.h"
#include "../ModuleScripting.h"
#include "../ModuleGOManager.h"
#include "../ModuleWindow.h"
#include "../ComponentTransform.h"
#include "../GameObject.h"
#include "../Component.h"
#include "../ComponentScript.h"
#include "../ComponentRectTransform.h"
#include "../SDL/include/SDL_scancode.h"
#include "../Globals.h"
#include "../ComponentCanvas.h"
#include "../Time.h"

#include "../ComponentAudioSource.h"

namespace ReloadScene
{

	// On Assets
	string assets_path_map1 = "/Assets/Scene_Map_1/Scene_Map_1.ezx";
	string assets_path_map2 = "/Assets/Scene_Map_2_NEW/Scene_Map_2NEW.ezx";
	// On Library
	string library_path_map1 = "/Library/1441726200/2271634307/2271634307.ezx";
	string library_path_map2 = "/Library/2159977326/1290996728/1290996728.ezx";

	void ReloadScene_GetPublics(map<const char*, string>* public_chars, map<const char*, int>* public_ints, map<const char*, float>* public_float, map<const char*, bool>* public_bools, map<const char*, GameObject*>* public_gos)
	{
	}

	void ReloadScene_Start(GameObject* game_object)
	{
		
		if (App->go_manager->current_scene == 0)
		{
			// Selecting Assets or Library version depending on Game mode
			if (App->StartInGame())
				App->LoadScene(library_path_map1.data());	// Using Library Scene files
			else
				App->LoadScene(assets_path_map1.data());		// Using Assets Scene files
		}
		else
		{
			// Selecting Assets or Library version depending on Game mode
			if (App->StartInGame())
				App->LoadScene(library_path_map2.data());	// Using Library Scene files
			else
				App->LoadScene(assets_path_map2.data());		// Using Assets Scene files
		}
	}

}