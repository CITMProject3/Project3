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
#include "../ComponentUiImage.h"
#include "../ComponentMaterial.h"
#include "../ModuleGOManager.h"
#include "../ComponentCanvas.h"

#include "../ComponentAudioSource.h"

namespace Start_Menu_UI
{
	GameObject* rb_button = nullptr;
	GameObject* rt_button = nullptr;
	GameObject* lb_button = nullptr;
	GameObject* lt_button = nullptr;

	GameObject* rb_button_color_selector = nullptr;
	GameObject* rt_button_color_selector = nullptr;
	GameObject* lb_button_color_selector = nullptr;
	GameObject* lt_button_color_selector = nullptr;

	GameObject* p1_indicator = nullptr;
	GameObject* p2_indicator = nullptr;
	GameObject* p3_indicator = nullptr;
	GameObject* p4_indicator = nullptr;

	GameObject* players = nullptr;
	GameObject* start_but = nullptr;
	GameObject* choose_team = nullptr;

	ComponentUiButton* c_rb = nullptr;
	ComponentUiButton* c_rt = nullptr;
	ComponentUiButton* c_lb = nullptr;
	ComponentUiButton* c_lt = nullptr;

	ComponentMaterial* m_rb = nullptr;
	ComponentMaterial* m_rt = nullptr;
	ComponentMaterial* m_lb = nullptr;
	ComponentMaterial* m_lt = nullptr;

	ComponentMaterial* m_p1 = nullptr;
	ComponentMaterial* m_p2 = nullptr;
	ComponentMaterial* m_p3 = nullptr;
	ComponentMaterial* m_p4 = nullptr;

	bool b_pressed[4];
	bool up_pressed[4];
	bool down_pressed[4];
	bool left_pressed[4];
	bool right_pressed[4];

	int player_order[4];
	int player_position[8];
	//  Player_position
	//  [0] [1] [2] [3]
	//  [4]         [6]
	//  [5]         [7]

	void Start_Menu_UI_GetPublics(map<const char*, string>* public_chars, map<const char*, int>* public_ints, map<const char*, float>* public_float, map<const char*, bool>* public_bools, map<const char*, GameObject*>* public_gos)
	{
		public_gos->insert(std::pair<const char*, GameObject*>("LB Button", lb_button));
		public_gos->insert(std::pair<const char*, GameObject*>("LT Button", lt_button));
		public_gos->insert(std::pair<const char*, GameObject*>("RB Button", rb_button));
		public_gos->insert(std::pair<const char*, GameObject*>("RT Button", rt_button));
		public_gos->insert(std::pair<const char*, GameObject*>("LB Button Color Selector", lb_button_color_selector));
		public_gos->insert(std::pair<const char*, GameObject*>("LT Button Color Selector", lt_button_color_selector));
		public_gos->insert(std::pair<const char*, GameObject*>("RB Button Color Selector", rb_button_color_selector));
		public_gos->insert(std::pair<const char*, GameObject*>("RT Button Color Selector", rt_button_color_selector));
		public_gos->insert(std::pair<const char*, GameObject*>("P1 indicator", p1_indicator));
		public_gos->insert(std::pair<const char*, GameObject*>("P2 indicator", p2_indicator));
		public_gos->insert(std::pair<const char*, GameObject*>("P3 indicator", p3_indicator));
		public_gos->insert(std::pair<const char*, GameObject*>("P4 indicator", p4_indicator));
		public_gos->insert(std::pair<const char*, GameObject*>("Start Button", start_but));
		public_gos->insert(std::pair<const char*, GameObject*>("Choose team", choose_team));
		public_gos->insert(std::pair<const char*, GameObject*>("Players", players));
		public_ints->insert(std::pair<const char*, int>("Player1", player_order[0]));
		public_ints->insert(std::pair<const char*, int>("Player2", player_order[1]));
		public_ints->insert(std::pair<const char*, int>("Player3", player_order[2]));
		public_ints->insert(std::pair<const char*, int>("Player4", player_order[3]));
	}

	void Start_Menu_UI_UpdatePublics(GameObject* game_object)
	{
		ComponentScript* test_script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);

		rb_button = test_script->public_gos.at("RB Button");
		rt_button = test_script->public_gos.at("RT Button");
		lb_button = test_script->public_gos.at("LB Button");
		lt_button = test_script->public_gos.at("LT Button");
		rb_button_color_selector = test_script->public_gos.at("RB Button Color Selector");
		rt_button_color_selector = test_script->public_gos.at("RT Button Color Selector");
		lb_button_color_selector = test_script->public_gos.at("LB Button Color Selector");
		lt_button_color_selector = test_script->public_gos.at("LT Button Color Selector");
		p1_indicator = test_script->public_gos.at("P1 indicator");
		p2_indicator = test_script->public_gos.at("P2 indicator");
		p3_indicator = test_script->public_gos.at("P3 indicator");
		p4_indicator = test_script->public_gos.at("P4 indicator");
		players = test_script->public_gos.at("Players");
		start_but = test_script->public_gos.at("Start Button");
		choose_team = test_script->public_gos.at("Choose team");
		player_order[0] = test_script->public_ints.at("Player1");
		player_order[1] = test_script->public_ints.at("Player2");
		player_order[2] = test_script->public_ints.at("Player3");
		player_order[3] = test_script->public_ints.at("Player4");

		c_rb = (ComponentUiButton*)rb_button->GetComponent(C_UI_BUTTON);
		c_rt = (ComponentUiButton*)rt_button->GetComponent(C_UI_BUTTON);
		c_lb = (ComponentUiButton*)lb_button->GetComponent(C_UI_BUTTON);
		c_lt = (ComponentUiButton*)lt_button->GetComponent(C_UI_BUTTON);

		m_rb = ((ComponentUiImage*)rb_button_color_selector->GetComponent(C_UI_IMAGE))->UImaterial;
		m_rt = ((ComponentUiImage*)rt_button_color_selector->GetComponent(C_UI_IMAGE))->UImaterial;
		m_lb = ((ComponentUiImage*)lb_button_color_selector->GetComponent(C_UI_IMAGE))->UImaterial;
		m_lt = ((ComponentUiImage*)lt_button_color_selector->GetComponent(C_UI_IMAGE))->UImaterial;

		m_p1 = ((ComponentUiImage*)p1_indicator->GetComponent(C_UI_IMAGE))->UImaterial;
		m_p2 = ((ComponentUiImage*)p2_indicator->GetComponent(C_UI_IMAGE))->UImaterial;
		m_p3 = ((ComponentUiImage*)p3_indicator->GetComponent(C_UI_IMAGE))->UImaterial;
		m_p4 = ((ComponentUiImage*)p4_indicator->GetComponent(C_UI_IMAGE))->UImaterial;
	}

	void Start_Menu_UI_ActualizePublics(GameObject* game_object)
	{
		ComponentScript* test_script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);

		test_script->public_gos.at("RB Button") = rb_button;
		test_script->public_gos.at("RT Button") = rt_button;
		test_script->public_gos.at("LB Button") = lb_button;
		test_script->public_gos.at("LT Button") = lt_button;
		test_script->public_gos.at("RB Button Color Selector") = rb_button_color_selector;
		test_script->public_gos.at("RT Button Color Selector") = rt_button_color_selector;
		test_script->public_gos.at("LB Button Color Selector") = lb_button_color_selector;
		test_script->public_gos.at("LT Button Color Selector") = lt_button_color_selector;
		test_script->public_gos.at("P1 indicator") = p1_indicator;
		test_script->public_gos.at("P2 indicator") = p2_indicator;
		test_script->public_gos.at("P3 indicator") = p3_indicator;
		test_script->public_gos.at("P4 indicator") = p4_indicator;
		test_script->public_gos.at("Players") = players;
		test_script->public_gos.at("Start Button") = start_but;
		test_script->public_gos.at("Choose team") = choose_team;
		test_script->public_ints.at("Player1") = player_order[0];
		test_script->public_ints.at("Player2") = player_order[1];
		test_script->public_ints.at("Player3") = player_order[2];
		test_script->public_ints.at("Player4") = player_order[3];


		c_rb = (ComponentUiButton*)rb_button->GetComponent(C_UI_BUTTON);
		c_rt = (ComponentUiButton*)rt_button->GetComponent(C_UI_BUTTON);
		c_lb = (ComponentUiButton*)lb_button->GetComponent(C_UI_BUTTON);
		c_lt = (ComponentUiButton*)lt_button->GetComponent(C_UI_BUTTON);

		m_rb = ((ComponentUiImage*)rb_button_color_selector->GetComponent(C_UI_IMAGE))->UImaterial;
		m_rt = ((ComponentUiImage*)rt_button_color_selector->GetComponent(C_UI_IMAGE))->UImaterial;
		m_lb = ((ComponentUiImage*)lb_button_color_selector->GetComponent(C_UI_IMAGE))->UImaterial;
		m_lt = ((ComponentUiImage*)lt_button_color_selector->GetComponent(C_UI_IMAGE))->UImaterial;

		m_p1 = ((ComponentUiImage*)p1_indicator->GetComponent(C_UI_IMAGE))->UImaterial;
		m_p2 = ((ComponentUiImage*)p2_indicator->GetComponent(C_UI_IMAGE))->UImaterial;
		m_p3 = ((ComponentUiImage*)p3_indicator->GetComponent(C_UI_IMAGE))->UImaterial;
		m_p4 = ((ComponentUiImage*)p4_indicator->GetComponent(C_UI_IMAGE))->UImaterial;
	}

	void Start_Menu_UI_UpdatePublics(GameObject* game_object);

	void Start_Menu_UI_Start(GameObject* game_object)
	{
		player_order[0] = -1;
		player_order[1] = -1;
		player_order[2] = -1;
		player_order[3] = -1;
		player_position[0] = 0;
		player_position[1] = 1;
		player_position[2] = 2;
		player_position[3] = 3;
		player_position[4] = -1;
		player_position[5] = -1;
		player_position[6] = -1;
		player_position[7] = -1;
		start_but->SetActive(false);
		players->SetActive(true);
		rb_button_color_selector->SetActive(false);
		rt_button_color_selector->SetActive(false);
		lb_button_color_selector->SetActive(false);
		lt_button_color_selector->SetActive(false);
		Start_Menu_UI_ActualizePublics(game_object);
		up_pressed[0] = false;
		up_pressed[1] = false;
		up_pressed[2] = false;
		up_pressed[3] = false;
		down_pressed[0] = false;
		down_pressed[1] = false;
		down_pressed[2] = false;
		down_pressed[3] = false;
		right_pressed[0] = false;
		right_pressed[1] = false;
		right_pressed[2] = false;
		right_pressed[3] = false;
		left_pressed[0] = false;
		left_pressed[1] = false;
		left_pressed[2] = false;
		left_pressed[3] = false;
		b_pressed[0] = false;
		b_pressed[1] = false;
		b_pressed[2] = false;
		b_pressed[3] = false;
	}

	void Start_Menu_UI_Update(GameObject* game_object)
	{
		for (int i = 0; i < 4; i++)
		{
			// DOWN INPUT ------------------------
			if (App->input->GetJoystickAxis(i, JOY_AXIS::LEFT_STICK_Y) > 0.6 || App->input->GetJoystickAxis(i, JOY_AXIS::RIGHT_STICK_Y) > 0.6 || App->input->GetJoystickButton(i, JOY_BUTTON::DPAD_DOWN) == KEY_DOWN || App->input->GetKey(SDL_SCANCODE_DOWN) == KEY_DOWN)
			{
				if (!down_pressed[i])
				{
					down_pressed[i] = true;
					if (player_position[0] == i || player_position[1] == i)
					{
						if (player_position[4] == -1)
						{
							player_position[4] = i + 4;
						}
						else if (player_position[5] == -1)
						{
							player_position[5] = i + 4;
						}
						else if (player_position[6] == -1)
						{
							player_position[6] = i + 4;
						}
						else if (player_position[7] == -1)
						{
							player_position[7] = i + 4;
						}
					}

					else if (player_position[2] == i || player_position[3] == i)
					{
						if (player_position[6] == -1)
						{
							player_position[6] = i + 4;
						}
						else if (player_position[7] == -1)
						{
							player_position[7] = i + 4;
						}
						else if (player_position[4] == -1)
						{
							player_position[4] = i + 4;
						}
						else if (player_position[5] == -1)
						{
							player_position[5] = i + 4;
						}
					}

					else
					{
						if (player_position[4] == i)
						{
							if (player_position[5] == -1)
							{
								player_position[5] = i + 4;
							}
						}
						else if (player_position[6] == i)
						{
							if (player_position[7] == -1)
							{
								player_position[7] = i + 4;
							}
						}
					}
				}
			}
			if (App->input->GetJoystickAxis(i, JOY_AXIS::LEFT_STICK_Y) < 0.1 || App->input->GetJoystickAxis(i, JOY_AXIS::RIGHT_STICK_Y) < 0.1 || App->input->GetJoystickButton(i, JOY_BUTTON::DPAD_DOWN) == KEY_UP || App->input->GetKey(SDL_SCANCODE_DOWN) == KEY_UP)
			{
				down_pressed[i] = false;
			}

			// UP INPUT ------------------------
			if (App->input->GetJoystickAxis(i, JOY_AXIS::LEFT_STICK_Y) <  -0.75 || App->input->GetJoystickAxis(i, JOY_AXIS::RIGHT_STICK_Y) < -0.75 || App->input->GetJoystickButton(i, JOY_BUTTON::DPAD_UP) == KEY_DOWN || App->input->GetKey(SDL_SCANCODE_UP) == KEY_DOWN)
			{
				if (!up_pressed[i])
				{
					up_pressed[i] = true;

					if (player_position[4] == i || player_position[6] == i)
					{
						player_position[i] = i + 4; // Reset
						start_but->SetActive(false);
					}
					else if (player_position[5] == i)
					{
						if (player_position[4] == -1)
						{
							player_position[4] = i + 4;
						}
						else
						{
							player_position[i] = i + 4; // Reset
							start_but->SetActive(false);
						}
					}
					else if (player_position[7] == i)
					{
						if (player_position[6] == -1)
						{
							player_position[6] = i + 4;
						}
						else
						{
							player_position[i] = i + 4; // Reset
							start_but->SetActive(false);
						}
					}
				}
			}
			if (App->input->GetJoystickAxis(i, JOY_AXIS::LEFT_STICK_Y) > -0.1 || App->input->GetJoystickAxis(i, JOY_AXIS::RIGHT_STICK_Y) > -0.1 || App->input->GetJoystickButton(i, JOY_BUTTON::DPAD_UP) == KEY_UP || App->input->GetKey(SDL_SCANCODE_UP) == KEY_UP)
			{
				up_pressed[i] = false;
			}
			// LEFT INPUT ------------------------
			if (App->input->GetJoystickAxis(i, JOY_AXIS::LEFT_STICK_X) < -0.75 || App->input->GetJoystickAxis(i, JOY_AXIS::RIGHT_STICK_X) < -0.75 || App->input->GetJoystickButton(i, JOY_BUTTON::DPAD_LEFT) == KEY_DOWN || App->input->GetKey(SDL_SCANCODE_LEFT) == KEY_DOWN)
			{
				if (!left_pressed[i])
				{
					left_pressed[i] = true;

					if (player_position[0] == i || player_position[1] == i || player_position[2] == i || player_position[3] == i)
					{
						if (player_position[4] == -1)
						{
							player_position[4] = i + 4;
						}
						else if (player_position[5] == -1)
						{
							player_position[5] = i + 4;
						}
					}
					if (player_position[6] == i)
					{
						if (player_position[4] == -1)
						{
							player_position[4] = i + 4;
						}
						else if (player_position[5] == -1)
						{
							player_position[5] = i + 4;
						}
					}
					else if (player_position[7] == i)
					{
						if (player_position[5] == -1)
						{
							player_position[5] = i + 4;
						}
						else if (player_position[4] == -1)
						{
							player_position[4] = i + 4;
						}
					}
				}
			}
			if (App->input->GetJoystickAxis(i, JOY_AXIS::LEFT_STICK_X) >- 0.1 || App->input->GetJoystickAxis(i, JOY_AXIS::RIGHT_STICK_X) > -0.1 || App->input->GetJoystickButton(i, JOY_BUTTON::DPAD_LEFT) == KEY_UP || App->input->GetKey(SDL_SCANCODE_LEFT) == KEY_UP)
			{
				left_pressed[i] = false;
			}
			// RIGHT INPUT ------------------------
			if (App->input->GetJoystickAxis(i, JOY_AXIS::LEFT_STICK_X) > 0.75 || App->input->GetJoystickAxis(i, JOY_AXIS::RIGHT_STICK_X) > 0.75 || App->input->GetJoystickButton(i, JOY_BUTTON::DPAD_RIGHT) == KEY_DOWN || App->input->GetKey(SDL_SCANCODE_RIGHT) == KEY_DOWN)
			{
				//  [0] [1] [2] [3]
				//  [4]         [6]
				//  [5]         [7]

				if (!right_pressed[i])
				{
					right_pressed[i] = true;

					if (player_position[0] == i || player_position[1] == i || player_position[2] == i || player_position[3] == i)
					{
						if (player_position[6] == -1)
						{
							player_position[6] = i + 4;
						}
						else if (player_position[7] == -1)
						{
							player_position[7] = i + 4;
						}
					}

					if (player_position[4] == i)
					{
						if (player_position[6] == -1)
						{
							player_position[6] = i + 4;
						}
						else if (player_position[7] == -1)
						{
							player_position[7] = i + 4;
						}
					}
					else if (player_position[5] == i)
					{
						if (player_position[7] == -1)
						{
							player_position[7] = i + 4;
						}
						else if (player_position[6] == -1)
						{
							player_position[6] = i + 4;
						}
					}
				}
			}
			if (App->input->GetJoystickAxis(i, JOY_AXIS::LEFT_STICK_X) < 0.1 || App->input->GetJoystickAxis(i, JOY_AXIS::RIGHT_STICK_X) < 0.1 || App->input->GetJoystickButton(i, JOY_BUTTON::DPAD_LEFT) == KEY_UP || App->input->GetKey(SDL_SCANCODE_RIGHT) == KEY_UP)
			{
				right_pressed[i] = false;
			}

			// B INPUT ------------------------
			if (App->input->GetJoystickButton(i, JOY_BUTTON::B) == KEY_DOWN || App->input->GetKey(SDL_SCANCODE_B) == KEY_DOWN)
			{
				if (!b_pressed[i])
				{
					b_pressed[i] = true;

					if (player_position[4] == i || player_position[5] == i || player_position[6] == i || player_position[7] == i)
					{
						player_position[i] = i + 4; // Reset
						start_but->SetActive(false);
					}
				}
			}
			if (App->input->GetJoystickButton(i, JOY_BUTTON::B) == KEY_UP || App->input->GetKey(SDL_SCANCODE_B) == KEY_UP)
			{
				b_pressed[i] = false;
			}
		}
		// Resolve
		for (int y = 0; y <= 7; y++)
		{
			if (player_position[y] >= 4)
			{
				int requesting_player = player_position[y] - 4;

				// Delete previous position
				for (int k = 0; k <= 7; k++)
				{
					if (player_position[k] == requesting_player)
					{
						switch (k)
						{
						case 0:
							m_p1->SetIdToRender(1);
							break;
						case 1:
							m_p2->SetIdToRender(1);
							break;
						case 2:
							m_p3->SetIdToRender(1);
							break;
						case 3:
							m_p4->SetIdToRender(1);
							break;
						case 4:
							c_lb->OnPressId(requesting_player);
							m_lb->SetIdToRender(requesting_player);
							lb_button_color_selector->SetActive(false);
							player_order[0] = -1;
							break;
						case 5:
							c_lt->OnPressId(requesting_player);
							m_lt->SetIdToRender(requesting_player);
							lt_button_color_selector->SetActive(false);
							player_order[1] = -1;
							break;
						case 6:
							c_rb->OnPressId(requesting_player);
							m_rb->SetIdToRender(requesting_player);
							rb_button_color_selector->SetActive(false);
							player_order[2] = -1;
							break;
						case 7:
							c_rt->OnPressId(requesting_player);
							m_rt->SetIdToRender(requesting_player);
							rt_button_color_selector->SetActive(false);
							player_order[3] = -1;
							break;
						}
						player_position[k] = -1;
					}
				}

				// Player press AUDIO
				ComponentAudioSource *a_comp = (ComponentAudioSource*)game_object->GetComponent(ComponentType::C_AUDIO_SOURCE);				

				player_position[y] = requesting_player;

				switch (y)
				{
				case 0:
					m_p1->SetIdToRender(0);
					if (a_comp) a_comp->PlayAudio(1);
					break;
				case 1:
					m_p2->SetIdToRender(0);
					if (a_comp) a_comp->PlayAudio(1);
					break;
				case 2:
					m_p3->SetIdToRender(0);
					if (a_comp) a_comp->PlayAudio(1);
					break;
				case 3:
					m_p4->SetIdToRender(0);
					if (a_comp) a_comp->PlayAudio(1);
					break;
				case 4:
					c_lb->OnPressId(requesting_player);
					m_lb->SetIdToRender(requesting_player);
					lb_button_color_selector->SetActive(true);
					player_order[0] = requesting_player;

					if (a_comp) a_comp->PlayAudio(0);
					break;
				case 5:
					c_lt->OnPressId(requesting_player);
					m_lt->SetIdToRender(requesting_player);
					lt_button_color_selector->SetActive(true);
					player_order[1] = requesting_player;

					if (a_comp) a_comp->PlayAudio(0);
					break;
				case 6:
					c_rb->OnPressId(requesting_player);
					m_rb->SetIdToRender(requesting_player);
					rb_button_color_selector->SetActive(true);
					player_order[2] = requesting_player;

					if (a_comp) a_comp->PlayAudio(0);
					break; 

				case 7:
					c_rt->OnPressId(requesting_player);
					m_rt->SetIdToRender(requesting_player);
					rt_button_color_selector->SetActive(true);
					player_order[3] = requesting_player;

					if (a_comp) a_comp->PlayAudio(0);
					break;
				}
			}
		}

		// Check All Players Ready
		int total = 0;
		for (int j = 0; j < 4; j++)
		{
			if (player_order[j] >= 0)
			{
				total++;
			}

			if (total == 4)
			{
				start_but->SetActive(true);
				players->SetActive(false);
				ComponentScript* main_canvas_script = (ComponentScript*)App->go_manager->current_scene_canvas->GetGameObject()->GetComponent(C_SCRIPT);
				if ((App->input->GetJoystickButton(j, JOY_BUTTON::START) == KEY_DOWN || App->input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN) && main_canvas_script->public_ints.at("current_menu") == 1)
				{
					main_canvas_script->public_ints.at("current_menu") = 2;
					main_canvas_script->public_ints.at("Player1") = player_order[0];
					main_canvas_script->public_ints.at("Player2") = player_order[1];
					main_canvas_script->public_ints.at("Player3") = player_order[2];
					main_canvas_script->public_ints.at("Player4") = player_order[3];

					App->go_manager->team1_front = (PLAYER)player_order[0];
					App->go_manager->team1_back = (PLAYER)player_order[1];
					App->go_manager->team2_front = (PLAYER)player_order[2];
					App->go_manager->team2_back = (PLAYER)player_order[3];
				}
			}
			else
			{
				start_but->SetActive(false);
				players->SetActive(true);
			}
		}
	}
}
			