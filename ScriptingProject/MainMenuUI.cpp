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
	GameObject* title_menu = nullptr;
	GameObject* select_parent = nullptr;
	GameObject* select_team_menu = nullptr;
	GameObject* select_menu = nullptr;
	GameObject* select_vehicle = nullptr;
	GameObject* select_level = nullptr;
	ComponentCanvas* canvas = nullptr;
	int current_scene = 0;
	int current_canvas_scene = 0;
	int player_order[4];
	void Main_Menu_UI_GetPublics(map<const char*, string>* public_chars, map<const char*, int>* public_ints, map<const char*, float>* public_float, map<const char*, bool>* public_bools, map<const char*, GameObject*>* public_gos)
	{
		public_gos->insert(std::pair<const char*, GameObject*>("Title Menu", title_menu));
		public_gos->insert(std::pair<const char*, GameObject*>("Select parent", select_parent));
		public_gos->insert(std::pair<const char*, GameObject*>("Select Team Menu", select_team_menu));
		public_gos->insert(std::pair<const char*, GameObject*>("Select character Menu", select_menu));
		public_gos->insert(std::pair<const char*, GameObject*>("Select vehicle Menu", select_vehicle));
		public_gos->insert(std::pair<const char*, GameObject*>("Select level Menu", select_level));
		public_ints->insert(std::pair<const char*, int>("current_menu", current_scene));

		public_ints->insert(std::pair<const char*, int>("Player1", player_order[0]));
		public_ints->insert(std::pair<const char*, int>("Player2", player_order[1]));
		public_ints->insert(std::pair<const char*, int>("Player3", player_order[2]));
		public_ints->insert(std::pair<const char*, int>("Player4", player_order[3]));
	}

	void Main_Menu_UI_UpdatePublics(GameObject* game_object)
	{
		ComponentScript* test_script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);
		current_scene = test_script->public_ints.at("current_menu");
		select_parent = test_script->public_gos.at("Select parent");
		title_menu = test_script->public_gos.at("Title Menu");
		select_team_menu = test_script->public_gos.at("Select Team Menu");
		select_menu = test_script->public_gos.at("Select character Menu");
		select_vehicle = test_script->public_gos.at("Select vehicle Menu");
		select_level = test_script->public_gos.at("Select level Menu");
		canvas = (ComponentCanvas*)game_object->GetComponent(ComponentType::C_CANVAS);

		player_order[0] = test_script->public_ints.at("Player1");
		player_order[1] = test_script->public_ints.at("Player2");
		player_order[2] = test_script->public_ints.at("Player3");
		player_order[3] = test_script->public_ints.at("Player4");
	}

	void Main_Menu_UI_ActualizePublics(GameObject* game_object)
	{
		ComponentScript* test_script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);
		test_script->public_ints.at("current_menu") = current_scene;
		test_script->public_gos.at("Select parent") = select_parent;
		test_script->public_gos.at("Title Menu") = title_menu;
		test_script->public_gos.at("Select Team Menu") = select_team_menu;
		test_script->public_gos.at("Select character Menu") = select_menu;
		test_script->public_gos.at("Select vehicle Menu") = select_vehicle;
		test_script->public_gos.at("Select level Menu") = select_level;

		test_script->public_ints.at("Player1") = player_order[0];
		test_script->public_ints.at("Player2") = player_order[1];
		test_script->public_ints.at("Player3") = player_order[2];
		test_script->public_ints.at("Player4") = player_order[3];

		canvas = (ComponentCanvas*)game_object->GetComponent(ComponentType::C_CANVAS);
	}

	void Main_Menu_UI_UpdatePublics(GameObject* game_object);

	void Main_Menu_UI_Start(GameObject* game_object)
	{
		current_scene = 0;
		Main_Menu_UI_ActualizePublics(game_object);
		if (title_menu != nullptr)
		{
			canvas->AddGoFocus(title_menu);
			title_menu->SetActive(true);
			select_team_menu->SetActive(false);
			select_level->SetActive(false);
			select_menu->SetActive(false);
		}
			
		//canvas->go_focus = title_menu;
	}

	void Main_Menu_UI_Update(GameObject* game_object)
	{
		if (current_canvas_scene != current_scene)
		{
			switch (current_scene)
			{
			case 0:
				title_menu->SetActive(true);
				select_parent->SetActive(false);
				select_team_menu->SetActive(false);
				select_menu->SetActive(false);
				select_vehicle->SetActive(false);
				select_level->SetActive(false);
				break;
			case 1:
				title_menu->SetActive(false);
				select_parent->SetActive(true);
				select_team_menu->SetActive(true);
				select_menu->SetActive(false);
				select_vehicle->SetActive(false);
				select_level->SetActive(false);
				break;
			case 2:
				title_menu->SetActive(false);
				select_parent->SetActive(true);
				select_team_menu->SetActive(false);
				select_menu->SetActive(true);
				select_vehicle->SetActive(false);
				select_level->SetActive(false);
				break;
			case 3:
				title_menu->SetActive(false);
				select_parent->SetActive(true);
				select_team_menu->SetActive(false);
				select_menu->SetActive(false);
				select_vehicle->SetActive(true);
				select_level->SetActive(false);
				break;
			case 4:
				title_menu->SetActive(false);
				select_parent->SetActive(true);
				select_team_menu->SetActive(false);
				select_menu->SetActive(false);
				select_vehicle->SetActive(false);
				select_level->SetActive(true);
				break;
			}
			current_canvas_scene = current_scene;
		}
		
	}
}