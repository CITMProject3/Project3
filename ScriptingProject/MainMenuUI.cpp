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

namespace Main_Menu_UI
{
	GameObject* start_menu = nullptr;
	GameObject* select_menu = nullptr;
	GameObject* select_vehicle = nullptr;
	GameObject* select_level = nullptr;
	ComponentCanvas* canvas = nullptr;
	
	void Main_Menu_UI_GetPublics(map<const char*, string>* public_chars, map<const char*, int>* public_ints, map<const char*, float>* public_float, map<const char*, bool>* public_bools, map<const char*, GameObject*>* public_gos)
	{
		public_gos->insert(std::pair<const char*, GameObject*>("Start Menu", start_menu));
		public_gos->insert(std::pair<const char*, GameObject*>("Select character Menu", select_menu));
		public_gos->insert(std::pair<const char*, GameObject*>("Select vehicle Menu", select_vehicle));
		public_gos->insert(std::pair<const char*, GameObject*>("Select level Menu", select_level));
	}

	void Main_Menu_UI_UpdatePublics(GameObject* game_object)
	{
		ComponentScript* test_script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);

		start_menu = test_script->public_gos.at("Start Menu");
		select_menu = test_script->public_gos.at("Select character Menu");
		select_vehicle = test_script->public_gos.at("Select vehicle Menu");
		select_level = test_script->public_gos.at("Select level Menu");
		canvas = (ComponentCanvas*)game_object->GetComponent(ComponentType::C_CANVAS);
	}

	void Main_Menu_UI_ActualizePublics(GameObject* game_object)
	{
		ComponentScript* test_script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);

		test_script->public_gos.at("Start Menu") = start_menu;
		test_script->public_gos.at("Select character Menu") = select_menu;
		test_script->public_gos.at("Select vehicle Menu") = select_vehicle;
		test_script->public_gos.at("Select level Menu") = select_level;
		canvas = (ComponentCanvas*)game_object->GetComponent(ComponentType::C_CANVAS);
	}

	void Main_Menu_UI_UpdatePublics(GameObject* game_object);

	void Main_Menu_UI_Start(GameObject* game_object)
	{
		if (start_menu != nullptr)
			canvas->SetGoFocus(start_menu);
			//canvas->go_focus = start_menu;
	}

	void Main_Menu_UI_Update(GameObject* game_object)
	{
		
	}
}