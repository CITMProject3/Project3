#include "stdafx.h"

#include <string>
#include <map>
#include <vector>
#include "../Application.h"
#include "../ModuleScripting.h"
#include "../ModuleInput.h"
#include "../ModuleWindow.h"
#include "../ComponentTransform.h"
#include "../ModuleGOManager.h"
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

#include "../ComponentAudioSource.h"

namespace MapSelectUI
{

	GameObject* map_fields = nullptr;
	GameObject* map_umi = nullptr;

	GameObject* players_vote[4];
	GameObject* right_arrow = nullptr;
	GameObject* left_arrow = nullptr;

	ComponentUiButton* c_players_vote[4];
	// 0 - P1 Red, 1 - P2 Red, 2 - P1 Blue, 2 - P2 Blue,
	ComponentUiButton* c_right_arrow = nullptr;
	ComponentUiButton* c_left_arrow = nullptr;

	// On Assets
	/*string path_map1 = "/Assets/Scene_Map_1/Scene_Map_1.ezx";
	string path_map2 = "/Assets/Scene_Map_2/Scene_Map_2.ezx";*/
	// On Library
	string path_map1 = "/Library/1441726200/2271634307/2271634307.ezx";
	string path_map2 = "/Library/3645291025/3212315065/3212315065.ezx";

	bool players_ready[4] = { false, false, false, false };

	bool a_pressed = false;
	bool b_pressed = false;
	bool dpad_left_pressed = false;
	bool dpad_right_pressed = false;

	bool current_level = false;
	bool current_level_lastFrame = true;
	int current_map = 0; // 1 -   , 2 -  
	int votes[4] = { 0, 0, 0, 0 };

	bool left_pressed[4];
	bool right_pressed[4];

	int arrow_counter_left = 30;
	int arrow_counter_right = 30;
	int time = 30;
	int player_order[4];
	void MapSelectUI_GetPublics(map<const char*, string>* public_chars, map<const char*, int>* public_ints, map<const char*, float>* public_float, map<const char*, bool>* public_bools, map<const char*, GameObject*>* public_gos)
	{
		public_gos->insert(std::pair<const char*, GameObject*>("Map Fields", map_fields));
		public_gos->insert(std::pair<const char*, GameObject*>("Map Umi", map_umi));
		public_gos->insert(std::pair<const char*, GameObject*>("P1-Red Vote", players_vote[2]));
		public_gos->insert(std::pair<const char*, GameObject*>("P2-Red Vote", players_vote[3]));
		public_gos->insert(std::pair<const char*, GameObject*>("P1-Blue Vote", players_vote[0]));
		public_gos->insert(std::pair<const char*, GameObject*>("P2-Blue Vote", players_vote[1]));
		public_gos->insert(std::pair<const char*, GameObject*>("R-Arrow", right_arrow));
		public_gos->insert(std::pair<const char*, GameObject*>("L-Arrow", left_arrow));

		public_ints->insert(std::pair<const char*, int>("Button Cooldown", time));
	}

	void MapSelectUI_UpdatePublics(GameObject* game_object)
	{
		ComponentScript* test_script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);

		map_fields = test_script->public_gos.at("Map Fields");
		map_umi = test_script->public_gos.at("Map Umi");
		players_vote[0] = test_script->public_gos.at("P1-Blue Vote");
		players_vote[1] = test_script->public_gos.at("P2-Blue Vote");
		players_vote[2] = test_script->public_gos.at("P1-Red Vote");
		players_vote[3] = test_script->public_gos.at("P2-Red Vote");

		right_arrow = test_script->public_gos.at("R-Arrow");
		left_arrow = test_script->public_gos.at("L-Arrow");
		time = test_script->public_ints.at("Button Cooldown");

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

		this_script->public_gos.at("Map Fields") = map_fields;
		this_script->public_gos.at("Map Umi") = map_umi;
		this_script->public_gos.at("P1-Blue Vote") = players_vote[0];
		this_script->public_gos.at("P2-Blue Vote") = players_vote[1];
		this_script->public_gos.at("P1-Red Vote") = players_vote[2];
		this_script->public_gos.at("P2-Red Vote") = players_vote[3];

		this_script->public_gos.at("R-Arrow") = right_arrow;
		this_script->public_gos.at("L-Arrow") = left_arrow;
		this_script->public_ints.at("Button Cooldown") = time;
		
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
		for (int i = 0; i <= 3; ++i)
		{
			right_pressed[i] = false;
			left_pressed[i] = false;
		}

		current_level = false;
		current_level_lastFrame = !current_level;

		arrow_counter_left = time;
		arrow_counter_right = time;
		current_map = 0;
		player_order[0] = App->go_manager->team1_front;
		player_order[1] = App->go_manager->team1_back;
		player_order[2] = App->go_manager->team2_front;
		player_order[3] = App->go_manager->team2_back;

	}

	void MapSelectUI_Update(GameObject* game_object)
	{
		if (current_level != current_level_lastFrame)
		{
			if (!current_level)
			{
				map_fields->SetActive(true);
				map_umi->SetActive(false);
			}
			else
			{
				map_fields->SetActive(false);
				map_umi->SetActive(true);
			}
			current_level_lastFrame = current_level;
		}

		if (App->input->GetKey(SDL_SCANCODE_RIGHT) == KEY_DOWN)
		{
			// Play Move Sound
			ComponentAudioSource *a_comp = (ComponentAudioSource*)game_object->GetComponent(ComponentType::C_AUDIO_SOURCE);
			if (a_comp) a_comp->PlayAudio(0);

			current_level = !current_level;

			if (arrow_counter_right >= time)
			{
				c_right_arrow->OnPress();
			}
			arrow_counter_right = 0;
		}
		else if (App->input->GetKey(SDL_SCANCODE_LEFT) == KEY_DOWN)
		{
			// Play Move Sound
			ComponentAudioSource *a_comp = (ComponentAudioSource*)game_object->GetComponent(ComponentType::C_AUDIO_SOURCE);
			if (a_comp) a_comp->PlayAudio(0);

			current_level = !current_level;

			if (arrow_counter_left >= time)
			{
				c_left_arrow->OnPress();
			}
			arrow_counter_left = 0;
		}

		for (int playerID = 0; playerID < 4; playerID++)
		{
			int id = 0;
			for (int j = 0; j < 4; j++)
			{
				if (player_order[j] == playerID)
				{
					id = j;
				}
			}
			if (App->input->GetJoystickButton(playerID, JOY_BUTTON::A) == KEY_DOWN || App->input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN)
			{
				// Play Loading Game Sound
				ComponentAudioSource *a_comp = (ComponentAudioSource*)game_object->GetComponent(ComponentType::C_AUDIO_SOURCE);
				if (a_comp) a_comp->PlayAudio(3);

				if (!current_level)
				{
					App->LoadScene(path_map1.data());
				}
				else
				{
					App->LoadScene(path_map2.data());
				}
			}

			if (App->input->GetJoystickButton(playerID, JOY_BUTTON::B) == KEY_DOWN || App->input->GetKey(SDL_SCANCODE_B) == KEY_DOWN)
			{
				// Play Deselection Sound
				ComponentAudioSource *a_comp = (ComponentAudioSource*)game_object->GetComponent(ComponentType::C_AUDIO_SOURCE);
				if (a_comp) a_comp->PlayAudio(2);

				if (players_ready[id])
				{
					c_players_vote[id]->OnPressId(votes[id]);

					players_ready[id] = false;
				}
			}

			if (App->input->GetJoystickAxis(playerID, JOY_AXIS::LEFT_STICK_X) < -0.75|| App->input->GetJoystickButton(playerID, JOY_BUTTON::DPAD_LEFT) == KEY_DOWN)
			{
				if (!left_pressed[playerID])
				{
					left_pressed[playerID] = true;

					// Play Move Sound
					ComponentAudioSource *a_comp = (ComponentAudioSource*)game_object->GetComponent(ComponentType::C_AUDIO_SOURCE);
					if (a_comp) a_comp->PlayAudio(0);

					current_level = !current_level;

					if (arrow_counter_left >= time)
					{
						c_left_arrow->OnPress();
					}
					arrow_counter_left = 0;
				}
			}
			if (App->input->GetJoystickAxis(playerID, JOY_AXIS::LEFT_STICK_X) > -0.25 || App->input->GetJoystickButton(playerID, JOY_BUTTON::DPAD_LEFT) == KEY_UP || App->input->GetKey(SDL_SCANCODE_LEFT) == KEY_UP)
			{
				left_pressed[playerID] = false;
			}

			if (App->input->GetJoystickAxis(playerID, JOY_AXIS::LEFT_STICK_X) > 0.75 || App->input->GetJoystickButton(playerID, JOY_BUTTON::DPAD_RIGHT) == KEY_DOWN)
			{
				if (!right_pressed[playerID])
				{
					right_pressed[playerID] = true;

					// Play Move Sound
					ComponentAudioSource *a_comp = (ComponentAudioSource*)game_object->GetComponent(ComponentType::C_AUDIO_SOURCE);
					if (a_comp) a_comp->PlayAudio(0);

					current_level = !current_level;

					if (arrow_counter_right >= time)
					{
						c_right_arrow->OnPress();
					}
					arrow_counter_right = 0;
				}
			}
			if (App->input->GetJoystickAxis(playerID, JOY_AXIS::LEFT_STICK_X) < 0.25 || App->input->GetJoystickButton(playerID, JOY_BUTTON::DPAD_LEFT) == KEY_UP || App->input->GetKey(SDL_SCANCODE_RIGHT) == KEY_UP)
			{
				right_pressed[playerID] = false;
			}

			if (arrow_counter_left < time)
			{
				arrow_counter_left++;

				if (arrow_counter_left == time)
					c_left_arrow->OnPress();
			}

			if (arrow_counter_right < time)
			{
				arrow_counter_right++;

				if (arrow_counter_right == time)
					c_right_arrow->OnPress();
			}

			int total = 0;
			for (int j = 0; j < 4; j++)
			{
				if (players_ready[j])
					total++;

				if (total >= 1)
				{
					unsigned int k = App->rnd->RandomInt(1, 4);

					// Play Loading Game Sound
					ComponentAudioSource *a_comp = (ComponentAudioSource*)game_object->GetComponent(ComponentType::C_AUDIO_SOURCE);
					if (a_comp) a_comp->PlayAudio(3);

					switch (votes[k])
					{
					case 1:
						App->LoadScene(path_map1.data());
						break;
					case 2:
						App->LoadScene(path_map2.data());
						break;
					default:
						App->LoadScene(path_map1.data());
						break;

					}
				}
			}
		}
	}
	void MapSelectUI_OnFocus()
	{

	}
}