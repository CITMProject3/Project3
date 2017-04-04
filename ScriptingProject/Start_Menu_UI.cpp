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
#include "../ModuleGOManager.h"
#include "../ComponentCanvas.h"

namespace Start_Menu_UI
{
	GameObject* rb_button = nullptr;
	GameObject* rt_button = nullptr;
	GameObject* lb_button = nullptr;
	GameObject* lt_button = nullptr;

	ComponentUiButton* c_rb = nullptr;
	ComponentUiButton* c_rt = nullptr;
	ComponentUiButton* c_lb = nullptr;
	ComponentUiButton* c_lt = nullptr;

	bool rb_pressed = false;
	bool rt_pressed = false;
	bool lb_pressed = false;
	bool lt_pressed = false;

	int player_order[4];

	void Start_Menu_UI_GetPublics(map<const char*, string>* public_chars, map<const char*, int>* public_ints, map<const char*, float>* public_float, map<const char*, bool>* public_bools, map<const char*, GameObject*>* public_gos)
	{
		public_gos->insert(std::pair<const char*, GameObject*>("LB Button", lb_button));
		public_gos->insert(std::pair<const char*, GameObject*>("LT Button", lt_button));
		public_gos->insert(std::pair<const char*, GameObject*>("RB Button", rb_button));
		public_gos->insert(std::pair<const char*, GameObject*>("RT Button", rt_button));

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

		player_order[0] = test_script->public_ints.at("Player1");
		player_order[1] = test_script->public_ints.at("Player2");
		player_order[2] = test_script->public_ints.at("Player3");
		player_order[3] = test_script->public_ints.at("Player4");

		c_rb = (ComponentUiButton*)rb_button->GetComponent(C_UI_BUTTON);
		c_rt = (ComponentUiButton*)rt_button->GetComponent(C_UI_BUTTON);
		c_lb = (ComponentUiButton*)lb_button->GetComponent(C_UI_BUTTON);
		c_lt = (ComponentUiButton*)lt_button->GetComponent(C_UI_BUTTON);


	}

	void Start_Menu_UI_ActualizePublics(GameObject* game_object)
	{
		ComponentScript* test_script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);

		test_script->public_gos.at("RB Button") = rb_button;
		test_script->public_gos.at("RT Button") = rt_button;
		test_script->public_gos.at("LB Button") = lb_button;
		test_script->public_gos.at("LT Button") = lt_button;

		test_script->public_ints.at("Player1") = player_order[0];
		test_script->public_ints.at("Player2") = player_order[1];
		test_script->public_ints.at("Player3") = player_order[2];
		test_script->public_ints.at("Player4") = player_order[3];

		c_rb = (ComponentUiButton*)rb_button->GetComponent(C_UI_BUTTON);
		c_rt = (ComponentUiButton*)rt_button->GetComponent(C_UI_BUTTON);
		c_lb = (ComponentUiButton*)lb_button->GetComponent(C_UI_BUTTON);
		c_lt = (ComponentUiButton*)lt_button->GetComponent(C_UI_BUTTON);
	}

	void Start_Menu_UI_UpdatePublics(GameObject* game_object);

	void Start_Menu_UI_Start(GameObject* game_object)
	{
		player_order[0] = -1;
		player_order[1] = -1;
		player_order[2] = -1;
		player_order[3] = -1;

		rb_pressed = false;
		rt_pressed = false;
		lb_pressed = false;
		lt_pressed = false;
	}

	void Start_Menu_UI_Update(GameObject* game_object)
	{
		for (int i = 0; i < 4; i++)
		{
			

			if (lb_pressed == false)
			{
				if (App->input->GetJoystickButton(i, JOY_BUTTON::LB) == KEY_DOWN || App->input->GetKey(SDL_SCANCODE_T) == KEY_DOWN)
				{
					bool selectable = true;
					for (int j = 0; j < 4; j++)
					{
						if (player_order[j] == i)
						{
							selectable = false;
						}
					}
					if (selectable)
					{
						player_order[0] = i;
						c_lb->OnPressId(i);
						lb_pressed = true;
					}
				}
			}

			if (lt_pressed == false)
			{
				if (App->input->GetJoystickAxis(i, JOY_AXIS::LEFT_TRIGGER) > 0.2f || App->input->GetKey(SDL_SCANCODE_U) == KEY_DOWN)
				{
					bool selectable = true;
					for (int j = 0; j < 4; j++)
					{
						if (player_order[j] == i)
						{
							selectable = false;
						}
					}
					if (selectable)
					{
						player_order[1] = i;
						c_lt->OnPressId(i);
						lt_pressed = true;
					}
				}
			}

			if (rb_pressed == false)
			{
				if (App->input->GetJoystickButton(i, JOY_BUTTON::RB) == KEY_DOWN || App->input->GetKey(SDL_SCANCODE_R) == KEY_DOWN)
				{
					bool selectable = true;
					for (int j = 0; j < 4; j++)
					{
						if (player_order[j] == i)
						{
							selectable = false;
						}
					}
					if (selectable)
					{
						player_order[2] = i;
						c_rb->OnPressId(i);
						rb_pressed = true;
					}

				}
			}


			if (rt_pressed == false)
			{
				if (App->input->GetJoystickAxis(i, JOY_AXIS::RIGHT_TRIGGER) > 0.2f || App->input->GetKey(SDL_SCANCODE_Y) == KEY_DOWN)
				{
					bool selectable = true;
					for (int j = 0; j < 4; j++)
					{
						if (player_order[j] == i)
						{
							selectable = false;
						}
					}
					if (selectable)
					{
						player_order[3] = i;
						c_rt->OnPressId(i);
						rt_pressed = true;
					}
				}
			}
			
			if (App->input->GetJoystickButton(i, JOY_BUTTON::B) == KEY_DOWN || App->input->GetKey(SDL_SCANCODE_B) == KEY_DOWN)
			{
				for (int j = 0; j < 4; j++)
				{
					if (player_order[j] == i)
					{
						player_order[j] = -1;
						switch (j)
						{
						case 0:
							if (lb_pressed)
							{
								lb_pressed = false;
								c_lb->OnPressId(i);
							}
							break;
						case 1:
							if (lt_pressed)
							{
								lt_pressed = false;
								c_lt->OnPressId(i);
							}
							break;
						case 2:
							if (rb_pressed)
							{
								rb_pressed = false;
								c_rb->OnPressId(i);
							}

							break;
						case 3:
							if (rt_pressed)
							{
								rt_pressed = false;
								c_rt->OnPressId(i);
							}
							break;
						}
						break;
					}
				}
			}

			int total = 0;
			for (int j = 0; j < 4; j++)
			{
				total += player_order[j];

				if (total == 6)
				{
					if (App->input->GetJoystickButton(i, JOY_BUTTON::START) == KEY_DOWN || App->input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN)
					{
						ComponentScript* main_canvas_script = (ComponentScript*)App->go_manager->current_scene_canvas->GetGameObject()->GetComponent(C_SCRIPT);

						main_canvas_script->public_ints.at("current_menu") = 1;
					}
				}
			}

		}
	}
}