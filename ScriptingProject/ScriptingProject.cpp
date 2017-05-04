// ScriptingProject.cpp : Defines the exported functions for the DLL application.

#include "stdafx.h"

#include <string>
#include <map>
#include <vector>

#include "../Application.h"
#include "../Time.h"
#include "../ModuleScripting.h"
#include "../ModuleInput.h"
#include "../ModuleWindow.h"
#include "../GameObject.h"
#include "../ComponentScript.h"
#include "../ComponentTransform.h"
#include "../SDL/include/SDL_scancode.h"
#include "../PhysBody3D.h"
#include "../ComponentCollider.h"

extern "C"
{
	namespace ScriptNames
	{
		void GetScriptNames(Application* engine_app, vector<const char*>* script_names, Time* engine_time)
		{
			App = engine_app;
			time = engine_time;

			script_names->push_back("CarPositionController");
			//Game loop related
			script_names->push_back("Checkpoint");
			script_names->push_back("Finish_lane");
			script_names->push_back("OutOfBounds");
			script_names->push_back("TurboPad");

			//Others
			script_names->push_back("Test");
			script_names->push_back("Test2");

			//TODO
			//script_names->push_back("Player_Car");
			//script_names->push_back("Makibishi");
			//script_names->push_back("Player_Camera");

			script_names->push_back("Scene_Manager");

			script_names->push_back("Main_Menu_UI");
			script_names->push_back("Start_Menu_UI");
			script_names->push_back("Character_Selection_UI");
			script_names->push_back("Vehicle_Selection_UI");
			script_names->push_back("MapSelectUI");
			script_names->push_back("PlayerObjectSelector");

			//script_names->push_back("Kmh_Counter");
			script_names->push_back("Item_Timer");
			// Audio related...
			script_names->push_back("Audio_PlaySound");
		}
	}


	namespace Test
	{
		string test_title;
		int test_int;
		int test_int2;
		GameObject* test_go;

		void Test_GetPublics(map<const char*, string>* public_chars, map<const char*, int>* public_ints, map<const char*, float>* public_float, map<const char*, bool>* public_bools, map<const char*, GameObject*>* public_gos)
		{
			test_title = "Hello World from Script";
			test_int = 3;
			test_int2 = 5;


			public_chars->insert(pair<const char*, string>("Title", test_title));

			public_gos->insert(pair<const char*, GameObject*>("Test_go", nullptr));
		}

		void Test_UpdatePublics(GameObject* game_object)
		{
			ComponentScript* test_script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);

			test_title = test_script->public_chars.at("Title");
			test_go = test_script->public_gos.at("Test_go");
		}

		void Test_Start(GameObject* game_object)
		{
		}

		void Test_Update(GameObject* game_object)
		{
			App->window->SetTitle(test_title.c_str());

			if (App->input->GetKey(SDL_SCANCODE_W) == KEY_REPEAT)
			{
				float3 tmp_pos = game_object->transform->GetPosition();
				tmp_pos.z += 1;
				game_object->transform->SetPosition(tmp_pos);
				if (test_go != nullptr)
				{
					float3 tmp_pos = test_go->transform->GetPosition();
					tmp_pos.z += 1;
					test_go->transform->SetPosition(tmp_pos);
				}
			}
		}
		
		void Test_ActualizePublics(GameObject* game_object)
		{
			ComponentScript* test_script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);

			test_script->public_chars.at("Title") = test_title;
			test_script->public_gos.at("Test_go") = test_go;
		}

		void Test_OnFocus(GameObject* game_object)
		{
		}

		void Test_OnCollision(GameObject* game_object, PhysBody3D* col)
		{
			if (!col->GetCollider()->Static)
			{
				float3 tmp_pos = col->GetCollider()->GetGameObject()->transform->GetPosition();
				tmp_pos.x -= 2;
				col->SetPos(tmp_pos.x, tmp_pos.y, tmp_pos.z);//GetCollider()->GetGameObject()->transform->SetPosition(tmp_pos);
			}
		}
	}

	namespace Test2
	{
		void Test2_Start(GameObject* game_object)
		{
		}

		void Test2_Update(GameObject* game_object)
		{
			App->window->SetTitle("Hello World from Script2");
		}
	}
}