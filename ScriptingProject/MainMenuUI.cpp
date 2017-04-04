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
	GameObject* select_level = nullptr;
	ComponentCanvas* canvas = nullptr;
	int current_scene = 0;
	int current_canvas_scene = 0;

	void Main_Menu_UI_GetPublics(map<const char*, string>* public_chars, map<const char*, int>* public_ints, map<const char*, float>* public_float, map<const char*, bool>* public_bools, map<const char*, GameObject*>* public_gos)
	{
		public_gos->insert(std::pair<const char*, GameObject*>("Start Menu", start_menu));
		public_gos->insert(std::pair<const char*, GameObject*>("Select character Menu", select_menu));
		public_gos->insert(std::pair<const char*, GameObject*>("Select level Menu", select_level));
		public_ints->insert(std::pair<const char*, int>("current_menu", current_scene));
	}

	void Main_Menu_UI_UpdatePublics(GameObject* game_object)
	{
		ComponentScript* test_script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);
		current_scene = test_script->public_ints.at("current_menu");
		start_menu = test_script->public_gos.at("Start Menu");
		select_menu = test_script->public_gos.at("Select character Menu");
		select_level = test_script->public_gos.at("Select level Menu");
		canvas = (ComponentCanvas*)game_object->GetComponent(ComponentType::C_CANVAS);
	}

	void Main_Menu_UI_ActualizePublics(GameObject* game_object)
	{
		ComponentScript* test_script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);
		test_script->public_ints.at("current_menu") = current_scene;
		test_script->public_gos.at("Start Menu") = start_menu;
		test_script->public_gos.at("Select character Menu") = select_menu;
		test_script->public_gos.at("Select level Menu") = select_level;
		canvas = (ComponentCanvas*)game_object->GetComponent(ComponentType::C_CANVAS);
	}

	void Main_Menu_UI_UpdatePublics(GameObject* game_object);

	void Main_Menu_UI_Start(GameObject* game_object)
	{
		if (start_menu != nullptr)
		{
			canvas->SetGoFocus(start_menu);
			select_level->SetActive(false);
			select_menu->SetActive(false);
			
		}
			
		//canvas->go_focus = start_menu;
	}

	void Main_Menu_UI_Update(GameObject* game_object)
	{
		if (current_canvas_scene != current_scene)
		{
			switch (current_scene)
			{
			case 1:
				start_menu->SetActive(false);
				select_level->SetActive(false);
				select_menu->SetActive(true);
				canvas->SetGoFocus(select_menu);
				break;
			case 2:
				start_menu->SetActive(false);
				select_level->SetActive(true);
				select_menu->SetActive(false);
				canvas->SetGoFocus(select_level);
				break;
			}
			current_canvas_scene = current_scene;
		}
		
	}
}