#include "stdafx.h"

#include <string>
#include <map>
#include <vector>
#include "../Application.h"
#include "../ModuleScripting.h"
#include "../ModuleInput.h"
#include "../ModuleWindow.h"
#include "../ModuleGOManager.h"
#include "../ComponentTransform.h"
#include "../GameObject.h"
#include "../Component.h"
#include "../ComponentScript.h"
#include "../ComponentRectTransform.h"
#include "../SDL/include/SDL_scancode.h"
#include "../Globals.h"
#include "../ComponentGrid.h"
#include "../ComponentUiButton.h"
#include "../ComponentCanvas.h"
#include "../ComponentMaterial.h"
#include "../ComponentUiImage.h"
#include "../ModuleResourceManager.h"

#include "../ComponentAudioSource.h"


namespace Vehicle_Selection_UI
{
	GameObject* blue_arrow_left = nullptr;
	GameObject* blue_arrow_right = nullptr;
	GameObject* blue_car_portrait = nullptr;
	
	GameObject* red_arrow_left = nullptr;
	GameObject* red_arrow_right = nullptr;
	GameObject* red_car_portrait = nullptr;

	ComponentUiButton* but_blue_arrow_left = nullptr;
	ComponentUiButton* but_blue_arrow_right = nullptr;
	ComponentUiButton* but_blue_car_portrait = nullptr;
	
	ComponentUiButton* but_red_arrow_left = nullptr;
	ComponentUiButton* but_red_arrow_right = nullptr;
	ComponentUiButton* but_red_car_portrait = nullptr;

	ComponentMaterial* mat_blue_car = nullptr;
	ComponentMaterial* mat_red_car = nullptr;
	bool team_blue_selected = false;
	bool team_red_selected = false;
	int player_order[4];
	int p_pos[2];
	int blue_counter_left = 30;
	int blue_counter_right = 30;
	int red_counter_right = 30;
	int red_counter_left = 30;
	int time = 30;
	void Vehicle_Selection_UI_GetPublics(map<const char*, string>* public_chars, map<const char*, int>* public_ints, map<const char*, float>* public_float, map<const char*, bool>* public_bools, map<const char*, GameObject*>* public_gos)
	{
		public_gos->insert(std::pair<const char*, GameObject*>("Blue arrow left", blue_arrow_left));
		public_gos->insert(std::pair<const char*, GameObject*>("Blue arrow right", blue_arrow_right));
		public_gos->insert(std::pair<const char*, GameObject*>("Blue car", blue_car_portrait));
		
		public_gos->insert(std::pair<const char*, GameObject*>("Red arrow left", red_arrow_left));
		public_gos->insert(std::pair<const char*, GameObject*>("Red arrow right", red_arrow_right));
		public_gos->insert(std::pair<const char*, GameObject*>("Red car", red_car_portrait));

		public_ints->insert(std::pair<const char*, int>("Button Cooldown", time));
	}

	void Vehicle_Selection_UI_UpdatePublics(GameObject* game_object)
	{
		ComponentScript* test_script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);

		blue_arrow_left = test_script->public_gos.at("Blue arrow left");
		blue_arrow_right = test_script->public_gos.at("Blue arrow right");
		blue_car_portrait = test_script->public_gos.at("Blue car");
	
		red_arrow_left = test_script->public_gos.at("Red arrow left");
		red_arrow_right = test_script->public_gos.at("Red arrow right");
		red_car_portrait = test_script->public_gos.at("Red car");
		time = test_script->public_ints.at("Button Cooldown");

		but_blue_arrow_left = (ComponentUiButton*)blue_arrow_left->GetComponent(C_UI_BUTTON);
		but_blue_arrow_right = (ComponentUiButton*)blue_arrow_right->GetComponent(C_UI_BUTTON);
		but_blue_car_portrait = (ComponentUiButton*)blue_car_portrait->GetComponent(C_UI_BUTTON);

		but_red_arrow_left = (ComponentUiButton*)red_arrow_left->GetComponent(C_UI_BUTTON);
		but_red_arrow_right = (ComponentUiButton*)red_arrow_right->GetComponent(C_UI_BUTTON);
		but_red_car_portrait = (ComponentUiButton*)red_car_portrait->GetComponent(C_UI_BUTTON);

		mat_blue_car = but_blue_car_portrait->UImaterial;
		mat_red_car = but_red_car_portrait->UImaterial;
	}

	void Vehicle_Selection_UI_ActualizePublics(GameObject* game_object)
	{


		ComponentScript* test_script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);

		test_script->public_gos.at("Blue arrow left") = blue_arrow_left;
		test_script->public_gos.at("Blue arrow right") = blue_arrow_right;
		test_script->public_gos.at("Blue car") = blue_car_portrait;
		
		test_script->public_gos.at("Red arrow left") = red_arrow_left;
		test_script->public_gos.at("Red arrow right") = red_arrow_right;
		test_script->public_gos.at("Red car") = red_car_portrait;
		test_script->public_ints.at("Button Cooldown")= time;
		but_blue_arrow_left = (ComponentUiButton*)blue_arrow_left->GetComponent(C_UI_BUTTON);
		but_blue_arrow_right = (ComponentUiButton*)blue_arrow_right->GetComponent(C_UI_BUTTON);
		but_blue_car_portrait = (ComponentUiButton*)blue_car_portrait->GetComponent(C_UI_BUTTON);

		but_red_arrow_left = (ComponentUiButton*)red_arrow_left->GetComponent(C_UI_BUTTON);
		but_red_arrow_right = (ComponentUiButton*)red_arrow_right->GetComponent(C_UI_BUTTON);
		but_red_car_portrait = (ComponentUiButton*)red_car_portrait->GetComponent(C_UI_BUTTON);

		mat_blue_car = but_blue_car_portrait->UImaterial;
		mat_red_car = but_red_car_portrait->UImaterial;
	}

	void Vehicle_Selection_UI_UpdatePublics(GameObject* game_object);

	void Vehicle_Selection_UI_Start(GameObject* game_object)
	{
		// Play Move Selection
		ComponentAudioSource *a_comp = (ComponentAudioSource*)game_object->GetComponent(ComponentType::C_AUDIO_SOURCE);
		if (a_comp) a_comp->PlayAudio(0);

		player_order[0] = App->go_manager->team1_front;
		player_order[1] = App->go_manager->team1_back;
		player_order[2] = App->go_manager->team2_front;
		player_order[3] = App->go_manager->team2_back;

		mat_blue_car->color[0] = 0.6f;
		mat_blue_car->color[1] = 0.6f;
		mat_blue_car->color[2] = 0.6f;

		mat_red_car->color[0] = 0.6f;
		mat_red_car->color[1] = 0.6f;
		mat_red_car->color[2] = 0.6f;

		blue_counter_left = time;
		blue_counter_right = time;
		red_counter_right = time;
		red_counter_left = time;

		Vehicle_Selection_UI_ActualizePublics(game_object);
	}

	void Vehicle_Selection_UI_Update(GameObject* game_object)
	{
		for (int i = 0; i < 4; i++)
		{
			int id = 0;
			for (int j = 0; j < 4; j++)
			{
				if (player_order[j] == i)
				{
					id = j;
				}
			}
			if (App->input->GetJoystickButton(i, JOY_BUTTON::DPAD_RIGHT) == KEY_DOWN || App->input->GetKey(SDL_SCANCODE_RIGHT) == KEY_DOWN)
			{
				// Play Move Selection
				ComponentAudioSource *a_comp = (ComponentAudioSource*)game_object->GetComponent(ComponentType::C_AUDIO_SOURCE);
				if (a_comp) a_comp->PlayAudio(1);

				if (team_blue_selected == false && (id == 0 || id == 1))
				{
					but_blue_car_portrait->OnPress();

					if (blue_counter_right == time)
					{
						but_blue_arrow_right->OnPress();
					}
					blue_counter_right = 0;
					
					if (p_pos[0] == 1)
						p_pos[0] = 0;
					else
						p_pos[0] = 1;
				}

				if (team_red_selected == false && (id == 2 || id == 3))
				{
					but_red_car_portrait->OnPress();

					if (red_counter_right == time)
					{
						but_red_arrow_right->OnPress();
					}
					red_counter_right = 0;

					if (p_pos[1] == 1)
						p_pos[1] = 0;
					else
						p_pos[1] = 1;
				}
			}

			if (App->input->GetJoystickButton(i, JOY_BUTTON::DPAD_LEFT) == KEY_DOWN || App->input->GetKey(SDL_SCANCODE_LEFT) == KEY_DOWN)
			{
				// Play Move Selection
				ComponentAudioSource *a_comp = (ComponentAudioSource*)game_object->GetComponent(ComponentType::C_AUDIO_SOURCE);
				if (a_comp) a_comp->PlayAudio(1);

				if (team_blue_selected == false && (id == 0 || id == 1))
				{
					but_blue_car_portrait->OnPress();					

					if (blue_counter_left == time)
					{
						but_blue_arrow_left->OnPress();
					}
					blue_counter_left = 0;

					if (p_pos[0] == 1)
						p_pos[0] = 0;
					else
						p_pos[0] = 1;
				}

				if (team_red_selected == false && (id == 2 || id == 3))
				{
					but_red_car_portrait->OnPress();

					if (red_counter_left == time)
					{
						but_red_arrow_left->OnPress();
					}
					red_counter_left = 0;

					if (p_pos[1] == 1)
						p_pos[1] = 0;
					else
						p_pos[1] = 1;
				}
			}


			if (App->input->GetJoystickButton(i, JOY_BUTTON::A) == KEY_DOWN || App->input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN)
			{
				if (team_blue_selected == false && (id == 0 || id == 1))
				{
					team_blue_selected = true;
					blue_arrow_left->SetActive(false);
					blue_arrow_right->SetActive(false);
					mat_blue_car->color[0] = 1.0f;
					mat_blue_car->color[1] = 1.0f;
					mat_blue_car->color[2] = 1.0f;
				}

				if (team_red_selected == false && (id == 2 || id == 3))
				{
					team_red_selected = true;
					red_arrow_left->SetActive(false);
					red_arrow_right->SetActive(false);
					mat_red_car->color[0] = 1.0f;
					mat_red_car->color[1] = 1.0f;
					mat_red_car->color[2] = 1.0f;
				}
			}

			if (App->input->GetJoystickButton(i, JOY_BUTTON::B) == KEY_DOWN || App->input->GetKey(SDL_SCANCODE_B) == KEY_DOWN)
			{
				if (team_blue_selected == true && (id == 0 || id == 1))
				{
					team_blue_selected = false;
					blue_arrow_left->SetActive(true);
					blue_arrow_right->SetActive(true);
					mat_blue_car->color[0] = 0.6f;
					mat_blue_car->color[1] = 0.6f;
					mat_blue_car->color[2] = 0.6f;
				}

				if (team_red_selected == true && (id == 2 || id == 3))
				{
					team_red_selected = false;
					red_arrow_left->SetActive(true);
					red_arrow_right->SetActive(true);
					mat_red_car->color[0] = 0.6f;
					mat_red_car->color[1] = 0.6f;
					mat_red_car->color[2] = 0.6f;
				}
			}
		}

		if (blue_counter_left < time)
		{
			blue_counter_left++;

			if (blue_counter_left == time)
				but_blue_arrow_left->OnPress();
		}

		if (blue_counter_right < time)
		{
			blue_counter_right++;

			if (blue_counter_right == time)
				but_blue_arrow_right->OnPress();
		}

		if (red_counter_left < time)
		{
			red_counter_left++;

			if (red_counter_left == time)
				but_red_arrow_left->OnPress();
		}

		if (red_counter_right < time)
		{
			red_counter_right++;

			if (red_counter_right == time)
				but_red_arrow_right->OnPress();
		}

		if (team_red_selected && team_blue_selected)
		{
			ComponentScript* main_canvas_script = (ComponentScript*)App->go_manager->current_scene_canvas->GetGameObject()->GetComponent(C_SCRIPT);

			main_canvas_script->public_ints.at("current_menu") = 4;

			App->go_manager->team1_car = p_pos[0];
			App->go_manager->team2_car = p_pos[1];
			game_object->SetActive(false);
		}
	}

	void Vehicle_Selection_UI_OnFocus(GameObject* game_object)
	{

	}
}