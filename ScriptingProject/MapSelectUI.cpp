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
#include "../ComponentUiButton.h"
#include "../ComponentCanvas.h"
#include "../ModuleResourceManager.h"
#include "../Random.h"
#include "../Time.h"

namespace MapSelectUI
{

	GameObject* map_mainportrait = nullptr;
	GameObject* map_name = nullptr;
	GameObject* players_vote[4];
	GameObject* right_arrow = nullptr;
	GameObject* left_arrow = nullptr;

	ComponentUiButton* c_map_mainportrait = nullptr;
	ComponentUiButton* c_map_name = nullptr;
	ComponentUiButton* c_players_vote[4];
	// 0 - P1 Red, 1 - P2 Red, 2 - P1 Blue, 2 - P2 Blue,
	ComponentUiButton* c_right_arrow = nullptr;
	ComponentUiButton* c_left_arrow = nullptr;

	string path_map1 = "";
	string path_map2 = "";
	string path_map3 = "";

	bool players_ready[4] = { false, false, false, false };

	bool a_pressed = false;
	bool b_pressed = false;
	bool dpad_left_pressed = false;
	bool dpad_right_pressed = false;

	int current_map = 1; // 1 -   , 2 -   , 3 -   ,
	int votes[4] = { 0, 0, 0, 0 };

	void MapSelectUI_GetPublics(map<const char*, string>* public_chars, map<const char*, int>* public_ints, map<const char*, float>* public_float, map<const char*, bool>* public_bools, map<const char*, GameObject*>* public_gos)
	{
		public_gos->insert(std::pair<const char*, GameObject*>("MapPortrait", map_mainportrait));
		public_gos->insert(std::pair<const char*, GameObject*>("MapName", map_mainportrait));
		public_gos->insert(std::pair<const char*, GameObject*>("P1-Red Vote", players_vote[0]));
		public_gos->insert(std::pair<const char*, GameObject*>("P2-Red Vote", players_vote[1]));
		public_gos->insert(std::pair<const char*, GameObject*>("P1-Blue Vote", players_vote[2]));
		public_gos->insert(std::pair<const char*, GameObject*>("P2-Blue Vote", players_vote[3]));
		public_gos->insert(std::pair<const char*, GameObject*>("R-Arrow", right_arrow));
		public_gos->insert(std::pair<const char*, GameObject*>("L-Arrow", left_arrow));
	}

	void MapSelectUI_UpdatePublics(GameObject* game_object)
	{
		ComponentScript* test_script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);

		map_mainportrait = test_script->public_gos.at("MapPortrait");
		map_name = test_script->public_gos.at("MapName");
		players_vote[0] = test_script->public_gos.at("P1-Red Vote");
		players_vote[1] = test_script->public_gos.at("P2-Red Vote");
		players_vote[2] = test_script->public_gos.at("P1-Blue Vote");
		players_vote[3] = test_script->public_gos.at("P2-Blue Vote");
		right_arrow = test_script->public_gos.at("R-Arrow");
		left_arrow = test_script->public_gos.at("L-Arrow");

		c_map_mainportrait = (ComponentUiButton*)map_mainportrait->GetComponent(C_UI_BUTTON);
		c_map_name = (ComponentUiButton*)map_name->GetComponent(C_UI_BUTTON);
		c_players_vote[0] = (ComponentUiButton*)players_vote[0]->GetComponent(C_UI_BUTTON);
		c_players_vote[1] = (ComponentUiButton*)players_vote[1]->GetComponent(C_UI_BUTTON);
		c_players_vote[2] = (ComponentUiButton*)players_vote[2]->GetComponent(C_UI_BUTTON);
		c_players_vote[3] = (ComponentUiButton*)players_vote[3]->GetComponent(C_UI_BUTTON);
		c_right_arrow = (ComponentUiButton*)right_arrow->GetComponent(C_UI_BUTTON);
		c_left_arrow = (ComponentUiButton*)left_arrow->GetComponent(C_UI_BUTTON);
	}

	void MapSelectUI_ActualizePublics(GameObject* game_object)
	{
		ComponentScript* this_script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);

		this_script->public_gos.at("MapPortrait") = map_mainportrait;
		this_script->public_gos.at("MapName") = map_name;
		this_script->public_gos.at("P1-Red Vote") = players_vote[0];
		this_script->public_gos.at("P2-Red Vote") = players_vote[1];
		this_script->public_gos.at("P1-Blue Vote") = players_vote[2];
		this_script->public_gos.at("P2-Blue Vote") = players_vote[3];
		this_script->public_gos.at("R-Arrow") = right_arrow;
		this_script->public_gos.at("L-Arrow") = left_arrow;

		c_map_mainportrait = (ComponentUiButton*)map_mainportrait->GetComponent(C_UI_BUTTON);
		c_map_name = (ComponentUiButton*)map_name->GetComponent(C_UI_BUTTON);
		c_players_vote[0] = (ComponentUiButton*)players_vote[0]->GetComponent(C_UI_BUTTON);
		c_players_vote[1] = (ComponentUiButton*)players_vote[1]->GetComponent(C_UI_BUTTON);
		c_players_vote[2] = (ComponentUiButton*)players_vote[2]->GetComponent(C_UI_BUTTON);
		c_players_vote[3] = (ComponentUiButton*)players_vote[3]->GetComponent(C_UI_BUTTON);
		c_right_arrow = (ComponentUiButton*)right_arrow->GetComponent(C_UI_BUTTON);
		c_left_arrow = (ComponentUiButton*)left_arrow->GetComponent(C_UI_BUTTON);
	}

	void MapSelectUI_UpdatePublics(GameObject* game_object);

	void MapSelectUI_Start(GameObject* game_object)
	{
	}

	void MapSelectUI_Update(GameObject* game_object)
	{
		for (int playerID = 0; playerID < 4; playerID++)
		{
			if (App->input->GetJoystickButton(playerID, JOY_BUTTON::A) == KEY_DOWN || App->input->GetKey(SDL_SCANCODE_A) == KEY_DOWN)
			{
				c_players_vote[playerID]->OnPressId(current_map - 1); // TO BE TESTED
				

				votes[playerID] = current_map;
				players_ready[playerID] = true;
			}

			if (App->input->GetJoystickButton(playerID, JOY_BUTTON::B) == KEY_DOWN || App->input->GetKey(SDL_SCANCODE_B) == KEY_DOWN)
			{
				c_players_vote[playerID]->OnPressId(0); // TO BE TESTED

				players_ready[playerID] = false;
			}

			if (App->input->GetJoystickButton(playerID, JOY_BUTTON::DPAD_LEFT) == KEY_DOWN || App->input->GetKey(SDL_SCANCODE_LEFT) == KEY_DOWN)
			{
				if (current_map <= 1)
					current_map = 3;
				else
					current_map--;

				c_map_mainportrait->OnPressId(current_map - 2);

				c_left_arrow->OnPressId(0);
				dpad_left_pressed = true;
			}

			if (App->input->GetJoystickButton(playerID, JOY_BUTTON::DPAD_RIGHT) == KEY_DOWN || App->input->GetKey(SDL_SCANCODE_RIGHT) == KEY_DOWN)
			{
				if (current_map >= 3)
					current_map = 1;
				else
					current_map++;

				c_map_mainportrait->OnPressId(current_map - 1); // TO BE TESTED

				c_right_arrow->OnPressId(0);
				dpad_right_pressed = true;
			}
		}





		if (dpad_left_pressed)
		{
			dpad_left_pressed = false;
			for (int playerID = 0; playerID < 4; playerID++)
			{
				if (App->input->GetJoystickButton(playerID, JOY_BUTTON::DPAD_LEFT) == KEY_REPEAT || App->input->GetKey(SDL_SCANCODE_LEFT) == KEY_REPEAT)
				{
					dpad_left_pressed = true;
				}
			}
			if (!dpad_left_pressed)
			{
				c_right_arrow->OnPressId(1); // Return to original state
			}
		}

		if (dpad_right_pressed)
		{
			dpad_right_pressed = false;
			for (int playerID = 0; playerID < 4; playerID++)
			{
				if (App->input->GetJoystickButton(playerID, JOY_BUTTON::DPAD_RIGHT) == KEY_REPEAT || App->input->GetKey(SDL_SCANCODE_RIGHT) == KEY_REPEAT)
				{
					dpad_right_pressed = true;
				}
			}
			if (!dpad_right_pressed)
			{
				c_right_arrow->OnPressId(1);  // Return to original state
			}
		}

		int total = 0;
		for (int j = 0; j < 4; j++)
		{
			if (players_ready[j])
				total++;

			if (total == 4)
			{
				unsigned int k = App->rnd->RandomInt(1, 4);

				switch (votes[k])
				{
				case 1:
					App->resource_manager->LoadSceneFromAssets(path_map1.data());
					break;
				case 2:
					App->resource_manager->LoadSceneFromAssets(path_map2.data());
					break;
				case 3:
					App->resource_manager->LoadSceneFromAssets(path_map3.data());
					break;
				default:
					// Error Reset, but loads map 1 instead (because we need to cover bugs lol lmao pls don't kill me)
					App->resource_manager->LoadSceneFromAssets(path_map1.data());
					break;

				}
			}
			else
				total = 0; // Redundancy
		}
	}

	void MapSelectUIUI_OnFocus()
	{

	}
}
