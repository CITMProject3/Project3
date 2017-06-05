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
#include "../Time.h"

#include "../ComponentAudioSource.h"

namespace Main_Menu_UI
{
	GameObject* citm_menu = nullptr;
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
	float timer = 1.5f;
	float current_time = 0;
	void Main_Menu_UI_GetPublics(map<const char*, string>* public_chars, map<const char*, int>* public_ints, map<const char*, float>* public_float, map<const char*, bool>* public_bools, map<const char*, GameObject*>* public_gos)
	{
		public_gos->insert(std::pair<const char*, GameObject*>("Title Menu", title_menu));
		public_gos->insert(std::pair<const char*, GameObject*>("Select parent", select_parent));
		public_gos->insert(std::pair<const char*, GameObject*>("Select Team Menu", select_team_menu));
		public_gos->insert(std::pair<const char*, GameObject*>("Select character Menu", select_menu));
		public_gos->insert(std::pair<const char*, GameObject*>("Select vehicle Menu", select_vehicle));
		public_gos->insert(std::pair<const char*, GameObject*>("Select level Menu", select_level));
		public_gos->insert(std::pair<const char*, GameObject*>("Citm Menu", citm_menu));
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
		citm_menu = test_script->public_gos.at("Citm Menu");
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
		test_script->public_gos.at("Citm Menu") = citm_menu;
		test_script->public_ints.at("Player1") = player_order[0];
		test_script->public_ints.at("Player2") = player_order[1];
		test_script->public_ints.at("Player3") = player_order[2];
		test_script->public_ints.at("Player4") = player_order[3];

		canvas = (ComponentCanvas*)game_object->GetComponent(ComponentType::C_CANVAS);
	}

	void Main_Menu_UI_UpdatePublics(GameObject* game_object);

	void Main_Menu_UI_Start(GameObject* game_object)
	{
		current_scene = -1;
		current_canvas_scene = -1;
		citm_menu->SetActive(true);
		title_menu->SetActive(false);
		select_parent->SetActive(false);
		select_team_menu->SetActive(false);
		select_menu->SetActive(false);
		select_vehicle->SetActive(false);
		select_level->SetActive(false);
		Main_Menu_UI_ActualizePublics(game_object);
		current_time = 0;
		// Play Logo Intro
		ComponentAudioSource *a_comp = (ComponentAudioSource*)game_object->GetComponent(ComponentType::C_AUDIO_SOURCE);
		if (a_comp) a_comp->PlayAudio(0);
			
		//canvas->go_focus = title_menu;
	}

	void Main_Menu_UI_Update(GameObject* game_object)
	{
		if (current_canvas_scene == -1)
		{
			if (current_time > timer)
				current_scene = 0;
			else
				current_time += time->DeltaTime();
		}
		if (current_canvas_scene == 0)
		{
			for (int i = 0; i < 4; i++)
			{
				if (App->input->GetJoystickButton(i, JOY_BUTTON::B) == KEY_DOWN ||
					App->input->GetJoystickButton(i, JOY_BUTTON::A) == KEY_DOWN ||
					App->input->GetJoystickButton(i, JOY_BUTTON::X) == KEY_DOWN ||
					App->input->GetJoystickButton(i, JOY_BUTTON::Y) == KEY_DOWN ||
					App->input->GetJoystickButton(i, JOY_BUTTON::START) == KEY_DOWN ||
					App->input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN)
				{
					// Play Press Start sound
					ComponentAudioSource *a_comp = (ComponentAudioSource*)game_object->GetComponent(ComponentType::C_AUDIO_SOURCE);
					if (a_comp) a_comp->PlayAudio(1);

					current_scene = 1;
				}
			}
			
		}
		if (current_canvas_scene != current_scene)
		{
			switch (current_scene)
			{
			case -1:
				citm_menu->SetActive(true);
				title_menu->SetActive(false);
				select_parent->SetActive(false);
				select_team_menu->SetActive(false);
				select_menu->SetActive(false);
				select_vehicle->SetActive(false);
				select_level->SetActive(false);
				break;
			case 0:
				citm_menu->SetActive(false);
				title_menu->SetActive(true);
				select_parent->SetActive(false);
				select_team_menu->SetActive(false);
				select_menu->SetActive(false);
				select_vehicle->SetActive(false);
				select_level->SetActive(false);
				break;
			case 1:
				citm_menu->SetActive(false);
				title_menu->SetActive(false);
				select_parent->SetActive(false);
				select_team_menu->SetActive(true);
				select_menu->SetActive(false);
				select_vehicle->SetActive(false);
				select_level->SetActive(false);
				break;
			case 2:
				citm_menu->SetActive(false);
				title_menu->SetActive(false);
				select_parent->SetActive(true);
				select_team_menu->SetActive(false);
				select_menu->SetActive(true);
				select_vehicle->SetActive(false);
				select_level->SetActive(false);
				break;
			case 3:
				citm_menu->SetActive(false);
				title_menu->SetActive(false);
				select_parent->SetActive(true);
				select_team_menu->SetActive(false);
				select_menu->SetActive(false);
				select_vehicle->SetActive(true);
				select_level->SetActive(false);
				break;
			case 4:
				citm_menu->SetActive(false);
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