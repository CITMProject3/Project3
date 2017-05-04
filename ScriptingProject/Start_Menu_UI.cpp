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
	enum PlayerPosition
	{
		PP_StandBy = 0,
		PP_DriverLeft = 1,
		PP_GunnerLeft = 2,
		PP_DriverRight = 3,
		PP_GunnerRight = 4,
	};

	GameObject* controller_focus[4][4];

	GameObject* roles_portrait[4] = { nullptr, nullptr, nullptr, nullptr };

	GameObject* players = nullptr;
	GameObject* start_but = nullptr;
	GameObject* choose_team = nullptr;

	ComponentMaterial* m_rb = nullptr;
	ComponentMaterial* m_rt = nullptr;
	ComponentMaterial* m_lb = nullptr;
	ComponentMaterial* m_lt = nullptr;

	bool b_pressed[4];
	bool up_pressed[4];
	bool down_pressed[4];
	bool left_pressed[4];
	bool right_pressed[4];

	PlayerPosition controller[4][2];

	int player_order[4][2];
	//  Player_Order
	//  [X] [X] [X] [X]
	//  [0]         [1]
	//  [2]         [3]

	void Start_Menu_UI_GetPublics(map<const char*, string>* public_chars, map<const char*, int>* public_ints, map<const char*, float>* public_float, map<const char*, bool>* public_bools, map<const char*, GameObject*>* public_gos)
	{
		public_gos->insert(std::pair<const char*, GameObject*>("C1 DriverLeft", controller_focus[0][0]));
		public_gos->insert(std::pair<const char*, GameObject*>("C2 DriverLeft", controller_focus[1][0]));
		public_gos->insert(std::pair<const char*, GameObject*>("C3 DriverLeft", controller_focus[2][0]));
		public_gos->insert(std::pair<const char*, GameObject*>("C4 DriverLeft", controller_focus[3][0]));
		public_gos->insert(std::pair<const char*, GameObject*>("C1 GunnerLeft", controller_focus[0][1]));
		public_gos->insert(std::pair<const char*, GameObject*>("C2 GunnerLeft", controller_focus[1][1]));
		public_gos->insert(std::pair<const char*, GameObject*>("C3 GunnerLeft", controller_focus[2][1]));
		public_gos->insert(std::pair<const char*, GameObject*>("C4 GunnerLeft", controller_focus[3][1]));
		public_gos->insert(std::pair<const char*, GameObject*>("C1 DriverRight", controller_focus[0][2]));
		public_gos->insert(std::pair<const char*, GameObject*>("C2 DriverRight", controller_focus[1][2]));
		public_gos->insert(std::pair<const char*, GameObject*>("C3 DriverRight", controller_focus[2][2]));
		public_gos->insert(std::pair<const char*, GameObject*>("C4 DriverRight", controller_focus[3][2]));
		public_gos->insert(std::pair<const char*, GameObject*>("C1 GunnerRight", controller_focus[0][3]));
		public_gos->insert(std::pair<const char*, GameObject*>("C2 GunnerRight", controller_focus[1][3]));
		public_gos->insert(std::pair<const char*, GameObject*>("C3 GunnerRight", controller_focus[2][3]));
		public_gos->insert(std::pair<const char*, GameObject*>("C4 GunnerRight", controller_focus[3][3]));
		public_gos->insert(std::pair<const char*, GameObject*>("DriverL Grid", roles_portrait[0]));
		public_gos->insert(std::pair<const char*, GameObject*>("GunnerL Grid", roles_portrait[1]));
		public_gos->insert(std::pair<const char*, GameObject*>("DriverR Grid", roles_portrait[2]));
		public_gos->insert(std::pair<const char*, GameObject*>("GunnerR Grid", roles_portrait[3]));
		public_gos->insert(std::pair<const char*, GameObject*>("Start Button", start_but));
		public_gos->insert(std::pair<const char*, GameObject*>("Choose team", choose_team));
		public_gos->insert(std::pair<const char*, GameObject*>("Players", players));
		public_ints->insert(std::pair<const char*, int>("Player1", player_order[0][0]));
		public_ints->insert(std::pair<const char*, int>("Player2", player_order[1][0]));
		public_ints->insert(std::pair<const char*, int>("Player3", player_order[2][0]));
		public_ints->insert(std::pair<const char*, int>("Player4", player_order[3][0]));
	}

	void Start_Menu_UI_UpdatePublics(GameObject* game_object)
	{
		ComponentScript* test_script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);

		controller_focus[0][0] = test_script->public_gos.at("C1 DriverLeft");
		controller_focus[1][0] = test_script->public_gos.at("C2 DriverLeft");
		controller_focus[2][0] = test_script->public_gos.at("C3 DriverLeft");
		controller_focus[3][0] = test_script->public_gos.at("C4 DriverLeft");
		controller_focus[0][1] = test_script->public_gos.at("C1 GunnerLeft");
		controller_focus[1][1] = test_script->public_gos.at("C2 GunnerLeft");
		controller_focus[2][1] = test_script->public_gos.at("C3 GunnerLeft");
		controller_focus[3][1] = test_script->public_gos.at("C4 GunnerLeft");
		controller_focus[0][2] = test_script->public_gos.at("C1 DriverRight");
		controller_focus[1][2] = test_script->public_gos.at("C2 DriverRight");
		controller_focus[2][2] = test_script->public_gos.at("C3 DriverRight");
		controller_focus[3][2] = test_script->public_gos.at("C4 DriverRight");
		controller_focus[0][3] = test_script->public_gos.at("C1 GunnerRight");
		controller_focus[1][3] = test_script->public_gos.at("C2 GunnerRight");
		controller_focus[2][3] = test_script->public_gos.at("C3 GunnerRight");
		controller_focus[3][3] = test_script->public_gos.at("C4 GunnerRight");
		roles_portrait[0] = test_script->public_gos.at("DriverL Grid");
		roles_portrait[1] = test_script->public_gos.at("GunnerL Grid");
		roles_portrait[2] = test_script->public_gos.at("DriverR Grid");
		roles_portrait[3] = test_script->public_gos.at("GunnerR Grid");
		players = test_script->public_gos.at("Players");
		start_but = test_script->public_gos.at("Start Button");
		choose_team = test_script->public_gos.at("Choose team");
		player_order[0][0] = test_script->public_ints.at("Player1");
		player_order[1][0] = test_script->public_ints.at("Player2");
		player_order[2][0] = test_script->public_ints.at("Player3");
		player_order[3][0] = test_script->public_ints.at("Player4");
	}

	void Start_Menu_UI_ActualizePublics(GameObject* game_object)
	{
		ComponentScript* test_script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);

		test_script->public_gos.at("C1 DriverLeft") = controller_focus[0][0];
		test_script->public_gos.at("C2 DriverLeft") = controller_focus[1][0];
		test_script->public_gos.at("C3 DriverLeft") = controller_focus[2][0];
		test_script->public_gos.at("C4 DriverLeft") = controller_focus[3][0];
		test_script->public_gos.at("C1 GunnerLeft") = controller_focus[0][1];
		test_script->public_gos.at("C2 GunnerLeft") = controller_focus[1][1];
		test_script->public_gos.at("C3 GunnerLeft") = controller_focus[2][1];
		test_script->public_gos.at("C4 GunnerLeft") = controller_focus[3][1];
		test_script->public_gos.at("C1 DriverRight") = controller_focus[0][2];
		test_script->public_gos.at("C2 DriverRight") = controller_focus[1][2];
		test_script->public_gos.at("C3 DriverRight") = controller_focus[2][2];
		test_script->public_gos.at("C4 DriverRight") = controller_focus[3][2];
		test_script->public_gos.at("C1 GunnerRight") = controller_focus[0][3];
		test_script->public_gos.at("C2 GunnerRight") = controller_focus[1][3];
		test_script->public_gos.at("C3 GunnerRight") = controller_focus[2][3];
		test_script->public_gos.at("C4 GunnerRight") = controller_focus[3][3];
		test_script->public_gos.at("DriverL Grid") = roles_portrait[0];
		test_script->public_gos.at("GunnerL Grid") = roles_portrait[1];
		test_script->public_gos.at("DriverR Grid") = roles_portrait[2];
		test_script->public_gos.at("GunnerR Grid") = roles_portrait[3];
		test_script->public_gos.at("Players") = players;
		test_script->public_gos.at("Start Button") = start_but;
		test_script->public_gos.at("Choose team") = choose_team;
		test_script->public_ints.at("Player1") = player_order[0][0];
		test_script->public_ints.at("Player2") = player_order[1][0];
		test_script->public_ints.at("Player3") = player_order[2][0];
		test_script->public_ints.at("Player4") = player_order[3][0];

		//m_p[0] = ((ComponentUiImage*)p1_indicator->GetComponent(C_UI_IMAGE))->UImaterial;
	}

	void Start_Menu_UI_UpdatePublics(GameObject* game_object);

	void Start_Menu_UI_Start(GameObject* game_object)
	{
		start_but->SetActive(false);
		players->SetActive(false);
		Start_Menu_UI_ActualizePublics(game_object);

		controller[0][1] = PP_DriverLeft;
		controller[1][1] = PP_GunnerLeft;
		controller[2][1] = PP_DriverRight; 
		controller[3][1] = PP_GunnerRight;
		for (int i = 0; i <= 3; ++i)
		{
			controller[i][0] = PP_StandBy;
			//controller[i][1] = PP_DriverLeft;

			controller_focus[0][i]->SetActive(false);
			controller_focus[1][i]->SetActive(false);
			controller_focus[2][i]->SetActive(false);
			controller_focus[3][i]->SetActive(false);

			up_pressed[i] = false;
			down_pressed[i] = false;
			right_pressed[i] = false;
			left_pressed[i] = false;
			b_pressed[i] = false;

			player_order[i][0] = -1;
			player_order[i][1] = -1;
		}
	}

	void Start_Menu_UI_Update(GameObject* game_object)
	{
		for (int i = 0; i < 4; i++)
		{
			// DOWN INPUT ------------------------
			if (App->input->GetJoystickAxis(i, JOY_AXIS::LEFT_STICK_Y) > 0.6 || App->input->GetJoystickAxis(i, JOY_AXIS::RIGHT_STICK_Y) > 0.6 || App->input->GetJoystickButton(i, JOY_BUTTON::DPAD_DOWN) == KEY_DOWN || App->input->GetKey(SDL_SCANCODE_DOWN) )
			{
				if (!down_pressed[i])
				{
					down_pressed[i] = true;

					if (controller[i][0] == PP_StandBy)
					{
						switch (i)
						{
						case 0:
							controller[i][1] = PP_DriverLeft;
							break;
						case 1:
							controller[i][1] = PP_DriverLeft;
							break;
						case 2:
							controller[i][1] = PP_DriverRight;
							break;
						case 3:
							controller[i][1] = PP_DriverRight;
							break;
						}
					}

					else
					{
						if (controller[i][0] == PP_DriverLeft)
						{
							controller[i][1] = PP_GunnerLeft;
						}
						else if (controller[i][1] == PP_DriverRight)
						{
							controller[i][1] = PP_GunnerRight;
						}
					}
				}
			}
			if (App->input->GetJoystickAxis(i, JOY_AXIS::LEFT_STICK_Y) < 0.1 || App->input->GetJoystickAxis(i, JOY_AXIS::RIGHT_STICK_Y) < 0.1 || App->input->GetJoystickButton(i, JOY_BUTTON::DPAD_DOWN) == KEY_UP || App->input->GetKey(SDL_SCANCODE_DOWN) == KEY_UP)
			{
				down_pressed[i] = false;
			}

			// UP INPUT ------------------------
			if (App->input->GetJoystickAxis(i, JOY_AXIS::LEFT_STICK_Y) < -0.75 || App->input->GetJoystickAxis(i, JOY_AXIS::RIGHT_STICK_Y) < -0.75 || App->input->GetJoystickButton(i, JOY_BUTTON::DPAD_UP) == KEY_DOWN || App->input->GetKey(SDL_SCANCODE_UP) == KEY_DOWN)
			{
				if (!up_pressed[i])
				{
					up_pressed[i] = true;

					switch (controller[i][0])
					{
					case PP_GunnerLeft:
						controller[i][1] = PP_DriverLeft;
						break;
					case PP_GunnerRight:
						controller[i][1] = PP_DriverRight;
						break;
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

					switch (controller[i][0])
					{
					case PP_StandBy: // Testing if this works
					case PP_DriverRight:
						controller[i][1] = PP_DriverLeft;
						break;
					case PP_GunnerRight:
						controller[i][1] = PP_GunnerLeft;
						break;
					}
				}
			}
			if (App->input->GetJoystickAxis(i, JOY_AXIS::LEFT_STICK_X) > -0.1 || App->input->GetJoystickAxis(i, JOY_AXIS::RIGHT_STICK_X) > -0.1 || App->input->GetJoystickButton(i, JOY_BUTTON::DPAD_LEFT) == KEY_UP || App->input->GetKey(SDL_SCANCODE_LEFT) == KEY_UP)
			{
				left_pressed[i] = false;
			}

			// RIGHT INPUT ------------------------
			if (App->input->GetJoystickAxis(i, JOY_AXIS::LEFT_STICK_X) > 0.75 || App->input->GetJoystickAxis(i, JOY_AXIS::RIGHT_STICK_X) > 0.75 || App->input->GetJoystickButton(i, JOY_BUTTON::DPAD_RIGHT) == KEY_DOWN || App->input->GetKey(SDL_SCANCODE_RIGHT) == KEY_DOWN)
			{
				if (!right_pressed[i])
				{
					right_pressed[i] = true;

					switch (controller[i][0])
					{
					case PP_StandBy:
					case PP_DriverLeft:
						controller[i][1] = PP_DriverRight;
						break;
					case PP_GunnerLeft:
						controller[i][1] = PP_GunnerRight;
						break;
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

					if (controller[i][0] != PP_StandBy)
					{
						// Playing Deselection Sound
						ComponentAudioSource *a_comp = (ComponentAudioSource*)game_object->GetComponent(ComponentType::C_AUDIO_SOURCE);
						if (a_comp) a_comp->PlayAudio(2);

						controller[i][1] = PP_StandBy;
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
		// Delete previous position
		for (int k = 0; k <= 3; ++k)
		{
			if (controller[k][0] != controller[k][1])
			{
				switch (controller[k][0])
				{
				case PP_StandBy:
					//m_p[k]->SetIdToRender(1);
					break;
				case PP_DriverLeft:
					controller_focus[k][0]->SetActive(false);
					//m_lb->SetIdToRender(k);
					player_order[0][0] = -1;
					break;
				case PP_GunnerLeft:
					controller_focus[k][1]->SetActive(false);
					player_order[1][0] = -1;
					break;
				case PP_DriverRight:
					controller_focus[k][2]->SetActive(false);
					player_order[2][0] = -1;
					break;
				case PP_GunnerRight:
					controller_focus[k][3]->SetActive(false);
					player_order[3][0] = -1;
					break;
				}
				 
				// Player press AUDIO
				ComponentAudioSource *a_comp = (ComponentAudioSource*)game_object->GetComponent(ComponentType::C_AUDIO_SOURCE);

				switch (controller[k][1])
				{
				case PP_StandBy:
					if (a_comp) a_comp->PlayAudio(2);  // Deselection
					break;
				case PP_DriverLeft:
					controller_focus[k][0]->SetActive(true);
					//m_lb->SetIdToRender(k);
					if (a_comp) a_comp->PlayAudio(0);	// Movement
					break;
				case PP_GunnerLeft:
					controller_focus[k][1]->SetActive(true);
					if (a_comp) a_comp->PlayAudio(0);
					break;
				case PP_DriverRight:
					controller_focus[k][2]->SetActive(true);
					if (a_comp) a_comp->PlayAudio(0);
					break;

				case PP_GunnerRight:
					controller_focus[k][3]->SetActive(true);
					if (a_comp) a_comp->PlayAudio(0);
					break;
				}
				controller[k][0] = controller[k][1];
			}

			switch (controller[k][0])
			{
			case PP_DriverLeft:
				if (player_order[0][0] == -1)
					player_order[0][0] = k;
				else if (player_order[0][0] != k)
				{
					if (player_order[0][0] != (k + 4))
						player_order[0][0] = k + 4;
					else
						player_order[0][0] = k;
				}
				break;
			case PP_GunnerLeft:
				if (player_order[1][0] == -1)
					player_order[1][0] = k;
				else if (player_order[1][0] != k)
				{
					if (player_order[1][0] != (k + 4))
						player_order[1][0] = k + 4;
					else
						player_order[1][0] = k;
				}
				break;
			case PP_DriverRight:
				if (player_order[2][0] == -1)
					player_order[2][0] = k;
				else if (player_order[2][0] != k)
				{
					if (player_order[2][0] != (k + 4))
						player_order[2][0] = k + 4;
					else
						player_order[2][0] = k;
				}
				break;
			case PP_GunnerRight:
				if (player_order[3][0] == -1)
					player_order[3][0] = k;
				else if (player_order[3][0] != k)
				{
					if (player_order[3][0] != (k + 4))
						player_order[3][0] = k + 4;
					else
						player_order[3][0] = k;
				}
				break;
			}
		}
		for (int i = 0; i <= 3; ++i)
		{
			if (player_order[i][0] != player_order[i][1])
			{
				if (player_order[i][1] != -1)
				{
					switch (player_order[i][1])
					{
					case -1:
						break;
					case 0:
						((ComponentUiImage*)roles_portrait[i]->GetComponent(C_UI_IMAGE))->UImaterial->SetIdToRender(player_order[i][1]);
						break;
					case 1:
						((ComponentUiImage*)roles_portrait[i]->GetComponent(C_UI_IMAGE))->UImaterial->SetIdToRender(player_order[i][1]);
						break;
					case 2:
						((ComponentUiImage*)roles_portrait[i]->GetComponent(C_UI_IMAGE))->UImaterial->SetIdToRender(player_order[i][1]);
						break;
					case 3:
						((ComponentUiImage*)roles_portrait[i]->GetComponent(C_UI_IMAGE))->UImaterial->SetIdToRender(player_order[i][1]);
						break;
					case 4:
					case 5:
					case 6:
					case 7:
						break;
					}
				}

				if (player_order[i][0] <= 3)
					((ComponentUiImage*)roles_portrait[i]->GetComponent(C_UI_IMAGE))->UImaterial->SetIdToRender(player_order[i][0]+1);
				else 
					((ComponentUiImage*)roles_portrait[i]->GetComponent(C_UI_IMAGE))->UImaterial->SetIdToRender(0);
				player_order[i][1] = player_order[i][0];
				LOG("%i", player_order[i][0]);
			}
		}
		
	

		// Check All Players Ready
		int total = 0;
		for (int j = 0; j < 4; j++)
		{
			if (player_order[j][0] >= 0 && player_order[j][0] <= 3)
			{
				total++;
			}
		}
		
			if (total == 4)
			{
				start_but->SetActive(true);
				players->SetActive(false);
				ComponentScript* main_canvas_script = (ComponentScript*)App->go_manager->current_scene_canvas->GetGameObject()->GetComponent(C_SCRIPT);
				for (int i = 0; i < 4; i++)
				{

					if ((App->input->GetJoystickButton(i, JOY_BUTTON::START) == KEY_DOWN || App->input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN) && main_canvas_script->public_ints.at("current_menu") == 1)
					{
						// Playing Selection Sound
						ComponentAudioSource *a_comp = (ComponentAudioSource*)game_object->GetComponent(ComponentType::C_AUDIO_SOURCE);
						if (a_comp) a_comp->PlayAudio(1);

						main_canvas_script->public_ints.at("current_menu") = 2;
						main_canvas_script->public_ints.at("Player1") = player_order[0][0];
						main_canvas_script->public_ints.at("Player2") = player_order[1][0];
						main_canvas_script->public_ints.at("Player3") = player_order[2][0];
						main_canvas_script->public_ints.at("Player4") = player_order[3][0];

						App->go_manager->team1_front = (PLAYER)player_order[0][0];
						App->go_manager->team1_back = (PLAYER)player_order[1][0];
						App->go_manager->team2_front = (PLAYER)player_order[2][0];
						App->go_manager->team2_back = (PLAYER)player_order[3][0];
					}
				}
			}
			else
			{
				start_but->SetActive(false);
				players->SetActive(false);
			}
		
	}
}
			