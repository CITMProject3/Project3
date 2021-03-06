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
	GameObject* blue_car_portrait_selected = nullptr;

	GameObject* red_arrow_left = nullptr;
	GameObject* red_arrow_right = nullptr;
	GameObject* red_car_portrait = nullptr;
	GameObject* red_car_portrait_selected = nullptr;

	ComponentUiButton* but_blue_arrow_left = nullptr;
	ComponentUiButton* but_blue_arrow_right = nullptr;
	ComponentUiButton* but_blue_car_portrait = nullptr;
	ComponentUiButton* but_blue_car_port_select = nullptr;

	ComponentUiButton* but_red_arrow_left = nullptr;
	ComponentUiButton* but_red_arrow_right = nullptr;
	ComponentUiButton* but_red_car_portrait = nullptr;
	ComponentUiButton* but_red_car_port_select = nullptr;

	ComponentMaterial* mat_blue_car = nullptr;
	ComponentMaterial* mat_red_car = nullptr;
	ComponentMaterial* mat_blue_car_select = nullptr;
	ComponentMaterial* mat_red_car_select = nullptr;

	bool left_pressed[4];
	bool right_pressed[4];

	bool team_blue_selected = false;
	bool team_red_selected = false;
	int player_order[4];
	int p_pos[2];
	int blue_counter_left = 30;
	int blue_counter_right = 30;
	int red_counter_right = 30;
	int red_counter_left = 30;
	int total_timer = 30;
	int time = 30;
	void Vehicle_Selection_UI_GetPublics(map<const char*, string>* public_chars, map<const char*, int>* public_ints, map<const char*, float>* public_float, map<const char*, bool>* public_bools, map<const char*, GameObject*>* public_gos)
	{
		public_gos->insert(std::pair<const char*, GameObject*>("Blue arrow left", blue_arrow_left));
		public_gos->insert(std::pair<const char*, GameObject*>("Blue arrow right", blue_arrow_right));
		public_gos->insert(std::pair<const char*, GameObject*>("Blue car", blue_car_portrait));
		public_gos->insert(std::pair<const char*, GameObject*>("blue_car_portrait_selected", blue_car_portrait_selected));

		public_gos->insert(std::pair<const char*, GameObject*>("Red arrow left", red_arrow_left));
		public_gos->insert(std::pair<const char*, GameObject*>("Red arrow right", red_arrow_right));
		public_gos->insert(std::pair<const char*, GameObject*>("Red car", red_car_portrait));
		public_gos->insert(std::pair<const char*, GameObject*>("red_car_portrait_selected", red_car_portrait_selected));

		public_ints->insert(std::pair<const char*, int>("Button Cooldown", time));
	}

	void Vehicle_Selection_UI_UpdatePublics(GameObject* game_object)
	{
		ComponentScript* test_script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);

		blue_arrow_left = test_script->public_gos.at("Blue arrow left");
		blue_arrow_right = test_script->public_gos.at("Blue arrow right");
		blue_car_portrait = test_script->public_gos.at("Blue car");
		blue_car_portrait_selected = test_script->public_gos.at("blue_car_portrait_selected");
	
		red_arrow_left = test_script->public_gos.at("Red arrow left");
		red_arrow_right = test_script->public_gos.at("Red arrow right");
		red_car_portrait = test_script->public_gos.at("Red car");
		red_car_portrait_selected = test_script->public_gos.at("red_car_portrait_selected");
		time = test_script->public_ints.at("Button Cooldown");

		but_blue_arrow_left = (ComponentUiButton*)blue_arrow_left->GetComponent(C_UI_BUTTON);
		but_blue_arrow_right = (ComponentUiButton*)blue_arrow_right->GetComponent(C_UI_BUTTON);
		but_blue_car_portrait = (ComponentUiButton*)blue_car_portrait->GetComponent(C_UI_BUTTON);
		but_blue_car_port_select = (ComponentUiButton*)blue_car_portrait_selected->GetComponent(C_UI_BUTTON);

		but_red_arrow_left = (ComponentUiButton*)red_arrow_left->GetComponent(C_UI_BUTTON);
		but_red_arrow_right = (ComponentUiButton*)red_arrow_right->GetComponent(C_UI_BUTTON);
		but_red_car_portrait = (ComponentUiButton*)red_car_portrait->GetComponent(C_UI_BUTTON);
		but_red_car_port_select = (ComponentUiButton*)red_car_portrait_selected->GetComponent(C_UI_BUTTON);

		mat_blue_car = but_blue_car_portrait->UImaterial;
		mat_red_car = but_red_car_portrait->UImaterial;
		mat_blue_car_select = but_blue_car_port_select->UImaterial;
		mat_red_car_select = but_red_car_port_select->UImaterial;
	}

	void Vehicle_Selection_UI_ActualizePublics(GameObject* game_object)
	{
		ComponentScript* test_script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);

		test_script->public_gos.at("Blue arrow left") = blue_arrow_left;
		test_script->public_gos.at("Blue arrow right") = blue_arrow_right;
		test_script->public_gos.at("Blue car") = blue_car_portrait;
		test_script->public_gos.at("blue_car_portrait_selected") = blue_car_portrait_selected;

		test_script->public_gos.at("Red arrow left") = red_arrow_left;
		test_script->public_gos.at("Red arrow right") = red_arrow_right;
		test_script->public_gos.at("Red car") = red_car_portrait;
		test_script->public_gos.at("red_car_portrait_selected") = red_car_portrait_selected;

		test_script->public_ints.at("Button Cooldown")= time;
		but_blue_arrow_left = (ComponentUiButton*)blue_arrow_left->GetComponent(C_UI_BUTTON);
		but_blue_arrow_right = (ComponentUiButton*)blue_arrow_right->GetComponent(C_UI_BUTTON);
		but_blue_car_portrait = (ComponentUiButton*)blue_car_portrait->GetComponent(C_UI_BUTTON);
		but_blue_car_port_select = (ComponentUiButton*)blue_car_portrait_selected->GetComponent(C_UI_BUTTON);

		but_red_arrow_left = (ComponentUiButton*)red_arrow_left->GetComponent(C_UI_BUTTON);
		but_red_arrow_right = (ComponentUiButton*)red_arrow_right->GetComponent(C_UI_BUTTON);
		but_red_car_portrait = (ComponentUiButton*)red_car_portrait->GetComponent(C_UI_BUTTON);
		but_red_car_port_select = (ComponentUiButton*)red_car_portrait_selected->GetComponent(C_UI_BUTTON);

		mat_blue_car = but_blue_car_portrait->UImaterial;
		mat_red_car = but_red_car_portrait->UImaterial;
		mat_blue_car_select = but_blue_car_port_select->UImaterial;
		mat_red_car_select = but_red_car_port_select->UImaterial;
	}

	void Vehicle_Selection_UI_UpdatePublics(GameObject* game_object);

	void Vehicle_Selection_UI_Start(GameObject* game_object)
	{
		blue_car_portrait_selected->SetActive(false);
		red_car_portrait_selected->SetActive(false);
		player_order[0] = App->go_manager->team1_front;
		player_order[1] = App->go_manager->team1_back;
		player_order[2] = App->go_manager->team2_front;
		player_order[3] = App->go_manager->team2_back;

		mat_blue_car->color[0] = 0.6f;
		mat_blue_car->color[1] = 0.6f;
		mat_blue_car->color[2] = 0.6f;

		for (int i = 0; i <= 3; ++i)
		{
			right_pressed[i] = false;
			left_pressed[i] = false;
		}

		mat_red_car->color[0] = 0.6f;
		mat_red_car->color[1] = 0.6f;
		mat_red_car->color[2] = 0.6f;
		team_blue_selected = false;
		team_red_selected = false;
		blue_counter_left = time;
		blue_counter_right = time;
		red_counter_right = time;
		red_counter_left = time;
		total_timer = time;
		p_pos[0] = 0;
		p_pos[1] = 0;
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
			if (App->input->GetJoystickAxis(i, JOY_AXIS::LEFT_STICK_X) > 0.75 || App->input->GetJoystickButton(i, JOY_BUTTON::DPAD_RIGHT) == KEY_DOWN || App->input->GetKey(SDL_SCANCODE_RIGHT) == KEY_DOWN)
			{
				if (!right_pressed[i])
				{
					right_pressed[i] = true;

					if (team_blue_selected == false && (id == 0 || id == 1))
					{
						// Play Move Sound
						ComponentAudioSource *a_comp = (ComponentAudioSource*)game_object->GetComponent(ComponentType::C_AUDIO_SOURCE);
						if (a_comp) a_comp->PlayAudio(0);
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
						// Play Move Sound
						ComponentAudioSource *a_comp = (ComponentAudioSource*)game_object->GetComponent(ComponentType::C_AUDIO_SOURCE);
						if (a_comp) a_comp->PlayAudio(0);

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
			}
			if (App->input->GetJoystickAxis(i, JOY_AXIS::LEFT_STICK_X) < 0.25 || App->input->GetJoystickButton(i, JOY_BUTTON::DPAD_LEFT) == KEY_UP || App->input->GetKey(SDL_SCANCODE_RIGHT) == KEY_UP)
			{
				right_pressed[i] = false;
			}

			if (App->input->GetJoystickAxis(i, JOY_AXIS::LEFT_STICK_X) < -0.75 || App->input->GetJoystickButton(i, JOY_BUTTON::DPAD_LEFT) == KEY_DOWN || App->input->GetKey(SDL_SCANCODE_LEFT) == KEY_DOWN)
			{
				if (!left_pressed[i])
				{
					left_pressed[i] = true;
					if (team_blue_selected == false && (id == 0 || id == 1))
					{
						// Play Move Sound
						ComponentAudioSource *a_comp = (ComponentAudioSource*)game_object->GetComponent(ComponentType::C_AUDIO_SOURCE);
						if (a_comp) a_comp->PlayAudio(0);

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
						// Play Move Sound
						ComponentAudioSource *a_comp = (ComponentAudioSource*)game_object->GetComponent(ComponentType::C_AUDIO_SOURCE);
						if (a_comp) a_comp->PlayAudio(0);

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
			}
			if (App->input->GetJoystickAxis(i, JOY_AXIS::LEFT_STICK_X) > -0.25 || App->input->GetJoystickButton(i, JOY_BUTTON::DPAD_LEFT) == KEY_UP || App->input->GetKey(SDL_SCANCODE_LEFT) == KEY_UP)
			{
				left_pressed[i] = false;
			}

			if (App->input->GetJoystickButton(i, JOY_BUTTON::A) == KEY_DOWN || App->input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN)
			{
				// Play Selection Sound
				ComponentAudioSource *a_comp = (ComponentAudioSource*)game_object->GetComponent(ComponentType::C_AUDIO_SOURCE);

				if (team_blue_selected == false && (id == 0 || id == 1))
				{
					team_blue_selected = true;
					blue_arrow_left->SetActive(false);
					blue_arrow_right->SetActive(false);
					blue_car_portrait_selected->SetActive(true);
					mat_blue_car_select->SetIdToRender(p_pos[0]);
					blue_car_portrait->SetActive(false);
					if (a_comp) a_comp->PlayAudio(1);
				}

				if (team_red_selected == false && (id == 2 || id == 3))
				{
					team_red_selected = true;
					red_arrow_left->SetActive(false);
					red_arrow_right->SetActive(false);
					red_car_portrait_selected->SetActive(true);
					mat_red_car_select->SetIdToRender(p_pos[1]);
					red_car_portrait->SetActive(false);
					if (a_comp) a_comp->PlayAudio(1);
				}
			}

			if (App->input->GetJoystickButton(i, JOY_BUTTON::B) == KEY_DOWN || App->input->GetKey(SDL_SCANCODE_B) == KEY_DOWN)
			{
				// Play Deselection Sound
				ComponentAudioSource *a_comp = (ComponentAudioSource*)game_object->GetComponent(ComponentType::C_AUDIO_SOURCE);

				if (team_blue_selected == true && (id == 0 || id == 1))
				{
					team_blue_selected = false;
					blue_arrow_left->SetActive(true);
					blue_arrow_right->SetActive(true);
					blue_car_portrait_selected->SetActive(false);
					blue_car_portrait->SetActive(true);
					if (a_comp) a_comp->PlayAudio(2);
				}

				if (team_red_selected == true && (id == 2 || id == 3))
				{
					team_red_selected = false;
					red_arrow_left->SetActive(true);
					red_arrow_right->SetActive(true);
					red_car_portrait_selected->SetActive(false);
					red_car_portrait->SetActive(true);
					if (a_comp) a_comp->PlayAudio(2);
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

		if (total_timer < time)
		{
			total_timer++;

			if (total_timer == time)
			{
				ComponentScript* main_canvas_script = (ComponentScript*)App->go_manager->current_scene_canvas->GetGameObject()->GetComponent(C_SCRIPT);
				main_canvas_script->public_ints.at("current_menu") = 4;
				game_object->SetActive(false);
			}
		}

		if (team_red_selected && team_blue_selected && total_timer == time)
		{
			App->go_manager->team1_car = p_pos[0];
			App->go_manager->team2_car = p_pos[1];
			total_timer = 0;
		}
	}

	void Vehicle_Selection_UI_OnFocus(GameObject* game_object)
	{

	}
}