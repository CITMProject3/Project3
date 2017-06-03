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


namespace Character_Selection_UI
{
	GameObject* driver1 = nullptr;
	GameObject* support1 = nullptr;
	GameObject* driver2 = nullptr;
	GameObject* support2 = nullptr;


	GameObject* driver1_port = nullptr;
	GameObject* support1_port = nullptr;
	GameObject* driver2_port = nullptr;
	GameObject* support2_port = nullptr;

	GameObject* Player1 = nullptr;
	GameObject* Player2 = nullptr;
	GameObject* Player3 = nullptr;
	GameObject* Player4 = nullptr;

	GameObject* Player1_num = nullptr;
	GameObject* Player2_num = nullptr;
	GameObject* Player3_num = nullptr;
	GameObject* Player4_num = nullptr;

	GameObject* Player1_num_port = nullptr;
	GameObject* Player2_num_port = nullptr;
	GameObject* Player3_num_port = nullptr;
	GameObject* Player4_num_port = nullptr;

	ComponentUiButton* but_driver1 = nullptr;
	ComponentUiButton* but_support1 = nullptr;
	ComponentUiButton* but_driver2 = nullptr;
	ComponentUiButton* but_support2 = nullptr;

	ComponentUiButton* but_Player1 = nullptr;
	ComponentUiButton* but_Player2 = nullptr;
	ComponentUiButton* but_Player3 = nullptr;
	ComponentUiButton* but_Player4 = nullptr;


	ComponentMaterial* mat_driver1 = nullptr;
	ComponentMaterial* mat_support1 = nullptr;
	ComponentMaterial* mat_driver2 = nullptr;
	ComponentMaterial* mat_support2 = nullptr;

	ComponentMaterial* mat_Player1 = nullptr;
	ComponentMaterial* mat_Player2 = nullptr;
	ComponentMaterial* mat_Player3 = nullptr;
	ComponentMaterial* mat_Player4 = nullptr;

	ComponentMaterial* mat_Player1_num = nullptr;
	ComponentMaterial* mat_Player2_num = nullptr;
	ComponentMaterial* mat_Player3_num = nullptr;
	ComponentMaterial* mat_Player4_num = nullptr;

	ComponentMaterial* mat_Player1_num_port = nullptr;
	ComponentMaterial* mat_Player2_num_port = nullptr;
	ComponentMaterial* mat_Player3_num_port = nullptr;
	ComponentMaterial* mat_Player4_num_port = nullptr;

	ComponentRectTransform* rect_driver1 = nullptr;
	ComponentRectTransform* rect_support1 = nullptr;
	ComponentRectTransform* rect_driver2 = nullptr;
	ComponentRectTransform* rect_support2 = nullptr;

	bool player1_select = false;
	bool player2_select = false;
	bool player3_select = false;
	bool player4_select = false;

	bool champ1_select = false;
	bool champ2_select = false;
	bool champ3_select = false;
	bool champ4_select = false;

	int p_pos[4];

	int character_selection[4];
	int player_order[4];
	void Character_Selection_UI_GetPublics(map<const char*, string>* public_chars, map<const char*, int>* public_ints, map<const char*, float>* public_float, map<const char*, bool>* public_bools, map<const char*, GameObject*>* public_gos)
	{
		public_gos->insert(std::pair<const char*, GameObject*>("Drivers 1", driver1));
		public_gos->insert(std::pair<const char*, GameObject*>("Support 1", support1));
		public_gos->insert(std::pair<const char*, GameObject*>("Drivers 2", driver2));
		public_gos->insert(std::pair<const char*, GameObject*>("Support 2", support2));

		public_gos->insert(std::pair<const char*, GameObject*>("Drivers port 1", driver1_port));
		public_gos->insert(std::pair<const char*, GameObject*>("Support port 1", support1_port));
		public_gos->insert(std::pair<const char*, GameObject*>("Drivers port 2", driver2_port));
		public_gos->insert(std::pair<const char*, GameObject*>("Support port 2", support2_port));

		public_gos->insert(std::pair<const char*, GameObject*>("Player 1", Player1));
		public_gos->insert(std::pair<const char*, GameObject*>("Player 2", Player2));
		public_gos->insert(std::pair<const char*, GameObject*>("Player 3", Player3));
		public_gos->insert(std::pair<const char*, GameObject*>("Player 4", Player4));

		public_gos->insert(std::pair<const char*, GameObject*>("Player 1 num", Player1_num));
		public_gos->insert(std::pair<const char*, GameObject*>("Player 2 num", Player2_num));
		public_gos->insert(std::pair<const char*, GameObject*>("Player 3 num", Player3_num));
		public_gos->insert(std::pair<const char*, GameObject*>("Player 4 num", Player4_num));

		public_gos->insert(std::pair<const char*, GameObject*>("Player 1 num port", Player1_num_port));
		public_gos->insert(std::pair<const char*, GameObject*>("Player 2 num port", Player2_num_port));
		public_gos->insert(std::pair<const char*, GameObject*>("Player 3 num port", Player3_num_port));
		public_gos->insert(std::pair<const char*, GameObject*>("Player 4 num port", Player4_num_port));

	}

	void Character_Selection_UI_UpdatePublics(GameObject* game_object)
	{
		ComponentScript* test_script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);

		driver1 = test_script->public_gos.at("Drivers 1");
		support1 = test_script->public_gos.at("Support 1");
		driver2 = test_script->public_gos.at("Drivers 2");
		support2 = test_script->public_gos.at("Support 2");

		driver1_port = test_script->public_gos.at("Drivers port 1");
		support1_port = test_script->public_gos.at("Support port 1");
		driver2_port = test_script->public_gos.at("Drivers port 2");
		support2_port = test_script->public_gos.at("Support port 2");

		Player1 = test_script->public_gos.at("Player 1");
		Player2 = test_script->public_gos.at("Player 2");
		Player3 = test_script->public_gos.at("Player 3");
		Player4 = test_script->public_gos.at("Player 4");

		Player1_num = test_script->public_gos.at("Player 1 num");
		Player2_num = test_script->public_gos.at("Player 2 num");
		Player3_num = test_script->public_gos.at("Player 3 num");
		Player4_num = test_script->public_gos.at("Player 4 num");

		Player1_num_port = test_script->public_gos.at("Player 1 num port");
		Player2_num_port = test_script->public_gos.at("Player 2 num port");
		Player3_num_port = test_script->public_gos.at("Player 3 num port");
		Player4_num_port = test_script->public_gos.at("Player 4 num port");

		but_driver1 = (ComponentUiButton*)driver1->GetComponent(C_UI_BUTTON);
		but_support1 = (ComponentUiButton*)support1->GetComponent(C_UI_BUTTON);
		but_driver2 = (ComponentUiButton*)driver2->GetComponent(C_UI_BUTTON);
		but_support2 = (ComponentUiButton*)support2->GetComponent(C_UI_BUTTON);

		rect_driver1 = (ComponentRectTransform*)driver1->GetComponent(C_RECT_TRANSFORM);
		rect_support1 = (ComponentRectTransform*)support1->GetComponent(C_RECT_TRANSFORM);
		rect_driver2 = (ComponentRectTransform*)driver2->GetComponent(C_RECT_TRANSFORM);
		rect_support2 = (ComponentRectTransform*)support2->GetComponent(C_RECT_TRANSFORM);

		but_Player1 = (ComponentUiButton*)Player1->GetComponent(C_UI_BUTTON);
		but_Player2 = (ComponentUiButton*)Player2->GetComponent(C_UI_BUTTON);
		but_Player3 = (ComponentUiButton*)Player3->GetComponent(C_UI_BUTTON);
		but_Player4 = (ComponentUiButton*)Player4->GetComponent(C_UI_BUTTON);

		mat_driver1 = ((ComponentUiImage*)driver1->GetComponent(C_UI_BUTTON))->UImaterial;
		mat_support1 = ((ComponentUiImage*)support1->GetComponent(C_UI_BUTTON))->UImaterial;
		mat_driver2 = ((ComponentUiImage*)driver2->GetComponent(C_UI_BUTTON))->UImaterial;
		mat_support2 = ((ComponentUiImage*)support2->GetComponent(C_UI_BUTTON))->UImaterial;

		mat_Player1_num = ((ComponentUiImage*)Player1_num->GetComponent(C_UI_IMAGE))->UImaterial;
		mat_Player2_num = ((ComponentUiImage*)Player2_num->GetComponent(C_UI_IMAGE))->UImaterial;
		mat_Player3_num = ((ComponentUiImage*)Player3_num->GetComponent(C_UI_IMAGE))->UImaterial;
		mat_Player4_num = ((ComponentUiImage*)Player4_num->GetComponent(C_UI_IMAGE))->UImaterial;

		mat_Player1_num_port = ((ComponentUiImage*)Player1_num_port->GetComponent(C_UI_IMAGE))->UImaterial;
		mat_Player2_num_port = ((ComponentUiImage*)Player2_num_port->GetComponent(C_UI_IMAGE))->UImaterial;
		mat_Player3_num_port = ((ComponentUiImage*)Player3_num_port->GetComponent(C_UI_IMAGE))->UImaterial;
		mat_Player4_num_port = ((ComponentUiImage*)Player4_num_port->GetComponent(C_UI_IMAGE))->UImaterial;

		mat_Player1 = but_Player1->UImaterial;
		mat_Player2 = but_Player2->UImaterial;
		mat_Player3 = but_Player3->UImaterial;
		mat_Player4 = but_Player4->UImaterial;

	}

	void Character_Selection_UI_ActualizePublics(GameObject* game_object)
	{


		ComponentScript* test_script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);

		test_script->public_gos.at("Drivers 1") = driver1;
		test_script->public_gos.at("Support 1") = support1;
		test_script->public_gos.at("Drivers 2") = driver2;
		test_script->public_gos.at("Support 2") = support2;

		test_script->public_gos.at("Player 1") = Player1;
		test_script->public_gos.at("Player 2") = Player2;
		test_script->public_gos.at("Player 3") = Player3;
		test_script->public_gos.at("Player 4") = Player4;

		test_script->public_gos.at("Drivers port 1") = driver1_port;
		test_script->public_gos.at("Support port 1") = support1_port;
		test_script->public_gos.at("Drivers port 2") = driver2_port;
		test_script->public_gos.at("Support port 2") = support2_port;

		test_script->public_gos.at("Player 1 num") = Player1_num;
		test_script->public_gos.at("Player 2 num") = Player2_num;
		test_script->public_gos.at("Player 3 num") = Player3_num;
		test_script->public_gos.at("Player 4 num") = Player4_num;

		test_script->public_gos.at("Player 1 num port") = Player1_num_port;
		test_script->public_gos.at("Player 2 num port") = Player2_num_port;
		test_script->public_gos.at("Player 3 num port") = Player3_num_port;
		test_script->public_gos.at("Player 4 num port") = Player4_num_port;

		but_driver1 = (ComponentUiButton*)driver1->GetComponent(C_UI_BUTTON);
		but_support1 = (ComponentUiButton*)support1->GetComponent(C_UI_BUTTON);
		but_driver2 = (ComponentUiButton*)driver2->GetComponent(C_UI_BUTTON);
		but_support2 = (ComponentUiButton*)support2->GetComponent(C_UI_BUTTON);

		but_Player1 = (ComponentUiButton*)Player1->GetComponent(C_UI_BUTTON);
		but_Player2 = (ComponentUiButton*)Player2->GetComponent(C_UI_BUTTON);
		but_Player3 = (ComponentUiButton*)Player3->GetComponent(C_UI_BUTTON);
		but_Player4 = (ComponentUiButton*)Player4->GetComponent(C_UI_BUTTON);

		rect_driver1 = (ComponentRectTransform*)driver1->GetComponent(C_RECT_TRANSFORM);
		rect_support1 = (ComponentRectTransform*)support1->GetComponent(C_RECT_TRANSFORM);
		rect_driver2 = (ComponentRectTransform*)driver2->GetComponent(C_RECT_TRANSFORM);
		rect_support2 = (ComponentRectTransform*)support2->GetComponent(C_RECT_TRANSFORM);

		mat_driver1 = ((ComponentUiImage*)driver1->GetComponent(C_UI_BUTTON))->UImaterial;
		mat_support1 = ((ComponentUiImage*)support1->GetComponent(C_UI_BUTTON))->UImaterial;
		mat_driver2 = ((ComponentUiImage*)driver2->GetComponent(C_UI_BUTTON))->UImaterial;
		mat_support2 = ((ComponentUiImage*)support2->GetComponent(C_UI_BUTTON))->UImaterial;

		mat_Player1_num = ((ComponentUiImage*)Player1_num->GetComponent(C_UI_IMAGE))->UImaterial;
		mat_Player2_num = ((ComponentUiImage*)Player2_num->GetComponent(C_UI_IMAGE))->UImaterial;
		mat_Player3_num = ((ComponentUiImage*)Player3_num->GetComponent(C_UI_IMAGE))->UImaterial;
		mat_Player4_num = ((ComponentUiImage*)Player4_num->GetComponent(C_UI_IMAGE))->UImaterial;

		mat_Player1_num_port = ((ComponentUiImage*)Player1_num_port->GetComponent(C_UI_IMAGE))->UImaterial;
		mat_Player2_num_port = ((ComponentUiImage*)Player2_num_port->GetComponent(C_UI_IMAGE))->UImaterial;
		mat_Player3_num_port = ((ComponentUiImage*)Player3_num_port->GetComponent(C_UI_IMAGE))->UImaterial;
		mat_Player4_num_port = ((ComponentUiImage*)Player4_num_port->GetComponent(C_UI_IMAGE))->UImaterial;

		mat_Player1 = but_Player1->UImaterial;
		mat_Player2 = but_Player2->UImaterial;
		mat_Player3 = but_Player3->UImaterial;
		mat_Player4 = but_Player4->UImaterial;
	}

	void Character_Selection_UI_UpdatePublics(GameObject* game_object);

	void Character_Selection_UI_Start(GameObject* game_object)
	{
		character_selection[0] = -1;
		character_selection[1] = -1;
		character_selection[2] = -1;
		character_selection[3] = -1;
		p_pos[0] = 0;
		p_pos[1] = 0;
		p_pos[2] = 0;
		p_pos[3] = 0;
		player_order[0] = App->go_manager->team1_front;
		player_order[1] = App->go_manager->team1_back;
		player_order[2] = App->go_manager->team2_front;
		player_order[3] = App->go_manager->team2_back;
		mat_Player1->color[0] = 0.4f;
		mat_Player1->color[1] = 0.4f;
		mat_Player1->color[2] = 0.4f;

		mat_Player2->color[0] = 0.4f;
		mat_Player2->color[1] = 0.4f;
		mat_Player2->color[2] = 0.4f;

		mat_Player3->color[0] = 0.4f;
		mat_Player3->color[1] = 0.4f;
		mat_Player3->color[2] = 0.4f;

		mat_Player4->color[0] = 0.4f;
		mat_Player4->color[1] = 0.4f;
		mat_Player4->color[2] = 0.4f;

		mat_driver1->color[0] = 1.0f;
		mat_driver1->color[1] = 1.0f;
		mat_driver1->color[2] = 1.0f;

		mat_support1->color[0] = 1.0f;
		mat_support1->color[1] = 1.0f;
		mat_support1->color[2] = 1.0f;

		mat_driver2->color[0] = 1.0f;
		mat_driver2->color[1] = 1.0f;
		mat_driver2->color[2] = 1.0f;

		mat_support2->color[0] = 1.0f;
		mat_support2->color[1] = 1.0f;
		mat_support2->color[2] = 1.0f;


		player1_select = false;
		player2_select = false;
		player3_select = false;
		player4_select = false;

		champ1_select = false;
		champ2_select = false;
		champ3_select = false;
		champ4_select = false;

		mat_Player1_num->SetIdToRender(player_order[0]);
		mat_Player2_num->SetIdToRender(player_order[1]);
		mat_Player3_num->SetIdToRender(player_order[2]);
		mat_Player4_num->SetIdToRender(player_order[3]);

		mat_Player1_num_port->SetIdToRender(player_order[0]);
		mat_Player2_num_port->SetIdToRender(player_order[1]);
		mat_Player3_num_port->SetIdToRender(player_order[2]);
		mat_Player4_num_port->SetIdToRender(player_order[3]);

		but_driver1->UImaterial->SetIdToRender(player_order[0]);
		but_support1->UImaterial->SetIdToRender(player_order[1]);
		but_driver2->UImaterial->SetIdToRender(player_order[2]);
		but_support2->UImaterial->SetIdToRender(player_order[3]);
		Character_Selection_UI_ActualizePublics(game_object);
	}

	void Character_Selection_UI_Update(GameObject* game_object)
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
				// Play Move Selection
				ComponentAudioSource *a_comp = (ComponentAudioSource*)game_object->GetComponent(ComponentType::C_AUDIO_SOURCE);

				switch (id)
				{
				case 0:
					if (p_pos[id] == 0 && player1_select == false)
					{
						rect_driver1->Move(float3(214, 0, 0));
						but_Player1->OnPress();
						p_pos[id] = 1;
						if (a_comp) a_comp->PlayAudio(0);
					}
					break;
				case 1:
					if (p_pos[id] == 0 && player2_select == false)
					{
						rect_support1->Move(float3(214, 0, 0));
						but_Player2->OnPress();
						p_pos[id] = 1;
						if (a_comp) a_comp->PlayAudio(0);
					}
					break;
				case 2:
					if (p_pos[id] == 0 && player3_select == false)
					{
						rect_driver2->Move(float3(214, 0, 0));
						but_Player3->OnPress();
						p_pos[id] = 1;
						if (a_comp) a_comp->PlayAudio(0);
					}
					break;
				case 3:
					if (p_pos[id] == 0 && player4_select == false)
					{
						rect_support2->Move(float3(214, 0, 0));
						but_Player4->OnPress();
						p_pos[id] = 1;
						if (a_comp) a_comp->PlayAudio(0);
					}
					break;
				}
			}

			if (App->input->GetJoystickAxis(i, JOY_AXIS::LEFT_STICK_X) < -0.75 || App->input->GetJoystickButton(i, JOY_BUTTON::DPAD_LEFT) == KEY_DOWN || App->input->GetKey(SDL_SCANCODE_LEFT) == KEY_DOWN)
			{
				// Play Move Selection (index 0)
				ComponentAudioSource *a_comp = (ComponentAudioSource*)game_object->GetComponent(ComponentType::C_AUDIO_SOURCE);

				switch (id)
				{
				case 0:
					if (p_pos[id] == 1 && player1_select == false)
					{
						rect_driver1->Move(float3(-214, 0, 0));
						but_Player1->OnPress();
						p_pos[id] = 0;	
						if (a_comp) a_comp->PlayAudio(0);
					}
					break;
				case 1:
					if (p_pos[id] == 1 && player2_select == false)
					{
						rect_support1->Move(float3(-214, 0, 0));
						but_Player2->OnPress();
						p_pos[id] = 0;
						if (a_comp) a_comp->PlayAudio(0);
					}
					break;
				case 2:
					if (p_pos[id] == 1 && player3_select == false)
					{
						rect_driver2->Move(float3(-214, 0, 0));
						but_Player3->OnPress();
						p_pos[id] =0;
						if (a_comp) a_comp->PlayAudio(0);
					}
					break;
				case 3:
					if (p_pos[id] == 1 && player4_select == false)
					{
						rect_support2->Move(float3(-214, 0, 0));
						but_Player4->OnPress();
						p_pos[id] = 0;
						if (a_comp) a_comp->PlayAudio(0);
					}
					break;
				}
			}

			if (App->input->GetJoystickButton(i, JOY_BUTTON::A) == KEY_DOWN || App->input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN)
			{

				// Play Selection Sound (index 1)
				ComponentAudioSource *a_comp = (ComponentAudioSource*)game_object->GetComponent(ComponentType::C_AUDIO_SOURCE);

				switch (id)
				{
				case 0:
					if (player1_select == false)
					{
						mat_driver1->color[0] = 0.4f;
						mat_driver1->color[1] = 0.4f;
						mat_driver1->color[2] = 0.4f;
						if (p_pos[id] == 0)
						{
							player1_select = true;
							mat_Player1->color[0] = 1.0f;
							mat_Player1->color[1] = 1.0f;
							mat_Player1->color[2] = 1.0f;
							
							champ1_select = true;
							if (a_comp) a_comp->PlayAudio(1);
						}
						else if (p_pos[id] == 1 )
						{
							player1_select = true;
							mat_Player1->color[0] = 1.0f;
							mat_Player1->color[1] = 1.0f;
							mat_Player1->color[2] = 1.0f;
							
							champ3_select = true;
							if (a_comp) a_comp->PlayAudio(1);
						}
						
					}
					break;
				case 1:
					if (player2_select == false)
					{					
							mat_support1->color[0] = 0.4f;
							mat_support1->color[1] = 0.4f;
							mat_support1->color[2] = 0.4f;
						if (p_pos[id] == 0)
						{
							player2_select = true;
							mat_Player2->color[0] = 1.0f;
							mat_Player2->color[1] = 1.0f;
							mat_Player2->color[2] = 1.0f;
							champ2_select = true;
							if (a_comp) a_comp->PlayAudio(1);
						}
						else if (p_pos[id] == 1)
						{
							player2_select = true;
							mat_Player2->color[0] = 1.0f;
							mat_Player2->color[1] = 1.0f;
							mat_Player2->color[2] = 1.0f;
							champ4_select = true;
							if (a_comp) a_comp->PlayAudio(1);
						}
					}
					break;
				case 2:
					if (player3_select == false)
					{
						mat_driver2->color[0] = 0.4f;
						mat_driver2->color[1] = 0.4f;
						mat_driver2->color[2] = 0.4f;
						if (p_pos[id] == 0)
						{
							player3_select = true;
							mat_Player3->color[0] = 1.0f;
							mat_Player3->color[1] = 1.0f;
							mat_Player3->color[2] = 1.0f;
							champ1_select = true;
							if (a_comp) a_comp->PlayAudio(1);
						}
						else if (p_pos[id] == 1)
						{
							player3_select = true;
							mat_Player3->color[0] = 1.0f;
							mat_Player3->color[1] = 1.0f;
							mat_Player3->color[2] = 1.0f;
							champ3_select = true;
							if (a_comp) a_comp->PlayAudio(1);
						}
					}
					break;
				case 3:
					if(player4_select == false)
					{
						mat_support2->color[0] = 0.4f;
						mat_support2->color[1] = 0.4f;
						mat_support2->color[2] = 0.4f;
						if (p_pos[id] == 0)
						{
							player4_select = true;
							mat_Player4->color[0] = 1.0f;
							mat_Player4->color[1] = 1.0f;
							mat_Player4->color[2] = 1.0f;

							champ2_select = true;
							if (a_comp) a_comp->PlayAudio(1);
						}
						else if (p_pos[id] == 1)
						{
							player4_select = true;
							mat_Player4->color[0] = 1.0f;
							mat_Player4->color[1] = 1.0f;
							mat_Player4->color[2] = 1.0f;
							
							champ4_select = true;
							if (a_comp) a_comp->PlayAudio(1);
						}
					}
					break;
				}
			}

			if (App->input->GetJoystickButton(i, JOY_BUTTON::B) == KEY_DOWN || App->input->GetKey(SDL_SCANCODE_B) == KEY_DOWN)
			{
				// Play Deselection Sound (index 2)
				ComponentAudioSource *a_comp = (ComponentAudioSource*)game_object->GetComponent(ComponentType::C_AUDIO_SOURCE);

				switch (id)
				{
				case 0:
					if (player1_select == true)
					{
						player1_select = false;
						mat_Player1->color[0] = 0.4f;
						mat_Player1->color[1] = 0.4f;
						mat_Player1->color[2] = 0.4f;

						mat_driver1->color[0] = 1.0f;
						mat_driver1->color[1] = 1.0f;
						mat_driver1->color[2] = 1.0f;

						if (a_comp) a_comp->PlayAudio(2);
						if (p_pos[id] == 0)
						{
							mat_driver1->color[0] = 1.0f;
							mat_driver1->color[1] = 1.0f;
							mat_driver1->color[2] = 1.0f;
							champ1_select = false;
						}
						else if (p_pos[id] == 1)
						{
							mat_driver2->color[0] = 1.0f;
							mat_driver2->color[1] = 1.0f;
							mat_driver2->color[2] = 1.0f;
							champ3_select = false;
						}
					}
					break;
				case 1:
					if (player2_select == true)
					{
						player2_select = false;
						mat_Player2->color[0] = 0.4f;
						mat_Player2->color[1] = 0.4f;
						mat_Player2->color[2] = 0.4f;
						mat_support1->color[0] = 1.0f;
						mat_support1->color[1] = 1.0f;
						mat_support1->color[2] = 1.0f;
						if (a_comp) a_comp->PlayAudio(2);

						if (p_pos[id] == 0)
						{
							mat_support1->color[0] = 1.0f;
							mat_support1->color[1] = 1.0f;
							mat_support1->color[2] = 1.0f;
							champ2_select = false;
						}
						else if (p_pos[id] == 1)
						{
							mat_support2->color[0] = 1.0f;
							mat_support2->color[1] = 1.0f;
							mat_support2->color[2] = 1.0f;
							champ4_select = false;
						}
					}
					break;
				case 2:
					if (player3_select == true)
					{
						player3_select = false;
						mat_Player3->color[0] = 0.4f;
						mat_Player3->color[1] = 0.4f;
						mat_Player3->color[2] = 0.4f;

						mat_driver2->color[0] = 1.0f;
						mat_driver2->color[1] = 1.0f;
						mat_driver2->color[2] = 1.0f;

						if (a_comp) a_comp->PlayAudio(2);

						if (p_pos[id] == 0)
						{
							mat_driver1->color[0] = 1.0f;
							mat_driver1->color[1] = 1.0f;
							mat_driver1->color[2] = 1.0f;
							champ1_select = false;
						}
						else if (p_pos[id] == 1)
						{
							mat_driver2->color[0] = 1.0f;
							mat_driver2->color[1] = 1.0f;
							mat_driver2->color[2] = 1.0f;
							champ3_select = false;
						}
					}
					break;
				case 3:
					if (player4_select == true)
					{
						player4_select = false;
						mat_Player4->color[0] = 0.4f;
						mat_Player4->color[1] = 0.4f;
						mat_Player4->color[2] = 0.4f;
						mat_support2->color[0] = 1.0f;
						mat_support2->color[1] = 1.0f;
						mat_support2->color[2] = 1.0f;
						if (a_comp) a_comp->PlayAudio(2);

						if (p_pos[id] == 0)
						{
							mat_support1->color[0] = 1.0f;
							mat_support1->color[1] = 1.0f;
							mat_support1->color[2] = 1.0f;
							champ2_select = false;
						}
						else if (p_pos[id] == 1)
						{
							mat_support2->color[0] = 1.0f;
							mat_support2->color[1] = 1.0f;
							mat_support2->color[2] = 1.0f;
							champ4_select = false;
						}
					}
					break;
				}
			}
		}

		if (player1_select && player2_select && player3_select && player4_select)
		{

			if (p_pos[0] == 0)
			{
				App->go_manager->team1_p1_c = 1;
			}
			else
			{
				App->go_manager->team1_p1_c = 0;
			}
			
			if (p_pos[1] == 0)
			{
				App->go_manager->team1_p2_c = 2;
			}
			else
			{
				App->go_manager->team1_p2_c = 3;
			}

			if (p_pos[2] == 0)
			{
				App->go_manager->team2_p1_c = 1;
			}
			else
			{
				App->go_manager->team2_p1_c = 0;
			}

			if (p_pos[3] == 0)
			{
				App->go_manager->team2_p2_c = 2;
			}
			else
			{
				App->go_manager->team2_p2_c = 3;
			}
			//To test
			//App->resource_manager->LoadSceneFromAssets("Assets/test_scene.ezx");
			ComponentScript* main_canvas_script = (ComponentScript*)App->go_manager->current_scene_canvas->GetGameObject()->GetComponent(C_SCRIPT);

			main_canvas_script->public_ints.at("current_menu") = 3;
		}
	}

	void Character_Selection_UI_OnFocus(GameObject* game_object)
	{

	}
}