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

namespace MapSelectUI
{

	GameObject* map_mainportrait = nullptr;
	GameObject* map_name = nullptr;
	GameObject* player1r_vote = nullptr;
	GameObject* player2r_vote = nullptr;
	GameObject* player1b_vote = nullptr;
	GameObject* player2b_vote = nullptr;
	GameObject* right_arrow = nullptr;
	GameObject* left_arrow = nullptr;

	ComponentUiButton* c_map_mainportrait = nullptr;
	ComponentUiButton* c_map_name = nullptr;
	ComponentUiButton* c_player1r_vote = nullptr;
	ComponentUiButton* c_player2r_vote = nullptr;
	ComponentUiButton* c_player1b_vote = nullptr;
	ComponentUiButton* c_player2b_vote = nullptr;
	ComponentUiButton* c_right_arrow = nullptr;
	ComponentUiButton* c_left_arrow = nullptr;

	bool a_pressed = false;
	bool b_pressed = false;
	bool dpad_left_pressed = false;
	bool dpad_right_pressed = false;

	int current_map = 1;
	// 1 -   , 2 -   , 3 -   ,

	void MapSelectUI_GetPublics(map<const char*, string>* public_chars, map<const char*, int>* public_ints, map<const char*, float>* public_float, map<const char*, bool>* public_bools, map<const char*, GameObject*>* public_gos)
	{
		public_gos->insert(std::pair<const char*, GameObject*>("MapPortrait", map_mainportrait));
		public_gos->insert(std::pair<const char*, GameObject*>("MapName", map_mainportrait));
		public_gos->insert(std::pair<const char*, GameObject*>("P1-Red Vote", player1r_vote));
		public_gos->insert(std::pair<const char*, GameObject*>("P2-Red Vote", player2r_vote));
		public_gos->insert(std::pair<const char*, GameObject*>("P1-Blue Vote", player1b_vote));
		public_gos->insert(std::pair<const char*, GameObject*>("P2-Blue Vote", player2b_vote));
		public_gos->insert(std::pair<const char*, GameObject*>("R-Arrow", right_arrow));
		public_gos->insert(std::pair<const char*, GameObject*>("L-Arrow", left_arrow));
	}

	void MapSelectUI_UpdatePublics(GameObject* game_object)
	{
		ComponentScript* test_script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);

		map_mainportrait = test_script->public_gos.at("MapPortrait");
		map_name = test_script->public_gos.at("MapName");
		player1r_vote = test_script->public_gos.at("P1-Red Vote");
		player2r_vote = test_script->public_gos.at("P2-Red Vote");
		player1b_vote = test_script->public_gos.at("P1-Blue Vote");
		player2b_vote = test_script->public_gos.at("P2-Blue Vote");
		right_arrow = test_script->public_gos.at("R-Arrow");
		left_arrow = test_script->public_gos.at("L-Arrow");

		c_map_mainportrait = (ComponentUiButton*)map_mainportrait->GetComponent(C_UI_BUTTON);
		c_map_name = (ComponentUiButton*)map_name->GetComponent(C_UI_BUTTON);
		c_player1r_vote = (ComponentUiButton*)player1r_vote->GetComponent(C_UI_BUTTON);
		c_player2r_vote = (ComponentUiButton*)player2r_vote->GetComponent(C_UI_BUTTON);
		c_player1b_vote = (ComponentUiButton*)player1b_vote->GetComponent(C_UI_BUTTON);
		c_player2b_vote = (ComponentUiButton*)player2b_vote->GetComponent(C_UI_BUTTON);
		c_right_arrow = (ComponentUiButton*)right_arrow->GetComponent(C_UI_BUTTON);
		c_left_arrow = (ComponentUiButton*)left_arrow->GetComponent(C_UI_BUTTON);
	}

	void MapSelectUI_ActualizePublics(GameObject* game_object)
	{
		ComponentScript* this_script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);

		this_script->public_gos.at("MapPortrait") = map_mainportrait;
		this_script->public_gos.at("MapName") = map_name;
		this_script->public_gos.at("P1-Red Vote") = player1r_vote;
		this_script->public_gos.at("P2-Red Vote") = player2r_vote;
		this_script->public_gos.at("P1-Blue Vote") = player1b_vote;
		this_script->public_gos.at("P2-Blue Vote") = player2b_vote;
		this_script->public_gos.at("R-Arrow") = right_arrow;
		this_script->public_gos.at("L-Arrow") = left_arrow;

		c_map_mainportrait = (ComponentUiButton*)map_mainportrait->GetComponent(C_UI_BUTTON);
		c_map_name = (ComponentUiButton*)map_name->GetComponent(C_UI_BUTTON);
		c_player1r_vote = (ComponentUiButton*)player1r_vote->GetComponent(C_UI_BUTTON);
		c_player2r_vote = (ComponentUiButton*)player2r_vote->GetComponent(C_UI_BUTTON);
		c_player1b_vote = (ComponentUiButton*)player1b_vote->GetComponent(C_UI_BUTTON);
		c_player2b_vote = (ComponentUiButton*)player2b_vote->GetComponent(C_UI_BUTTON);
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
			if (a_pressed == false)
			{
				if (App->input->GetJoystickButton(playerID, JOY_BUTTON::A) == KEY_DOWN || App->input->GetKey(SDL_SCANCODE_A) == KEY_DOWN)
				{

				}
			}

			if (b_pressed == false)
			{
				if (App->input->GetJoystickButton(playerID, JOY_BUTTON::B) == KEY_DOWN || App->input->GetKey(SDL_SCANCODE_B) == KEY_DOWN)
				{

				}
			}

			if (dpad_left_pressed == false || dpad_right_pressed == false)
			{
				if (App->input->GetJoystickButton(playerID, JOY_BUTTON::DPAD_LEFT) == KEY_DOWN || App->input->GetKey(SDL_SCANCODE_LEFT) == KEY_DOWN)
				{
					if (current_map <= 1)
						current_map = 3;
					else
						current_map--;
					if(!dpad_left_pressed)
						c_map_mainportrait->OnPressId(current_map-2);
					dpad_left_pressed = true;
				}

				if (App->input->GetJoystickButton(playerID, JOY_BUTTON::DPAD_RIGHT) == KEY_DOWN || App->input->GetKey(SDL_SCANCODE_RIGHT) == KEY_DOWN)
				{
					if (current_map >= 3)
						current_map = 1;
					else
						current_map++;
					if (!dpad_right_pressed)
						c_map_mainportrait->OnPressId(current_map-2);
					dpad_right_pressed = true;
				}
			}
		}
		dpad_left_pressed = false;
		dpad_right_pressed = false;
	}
}