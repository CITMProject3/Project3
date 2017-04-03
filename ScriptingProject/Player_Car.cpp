#include "stdafx.h"

#include <string>
#include <map>
#include <vector>
#include "../Application.h"
#include "../ModuleScripting.h"
#include "../ModuleInput.h"
#include "../ModuleWindow.h"
#include "../ModuleGOManager.h"
#include "../GameObject.h"
#include "../ComponentScript.h"
#include "../ComponentTransform.h"
#include "../SDL/include/SDL_scancode.h"
#include "../PhysBody3D.h"
#include "../ComponentCollider.h"
#include "../ComponentCar.h"
#include "../Globals.h"
#include "../PhysVehicle3D.h"
#include "../Random.h"

namespace Player_Car
{
	bool have_item = false;
	bool have_koma = false;
	bool have_triple_koma = false;
	bool have_firecracker = false;
	bool using_firecracker = false;
	GameObject* firecracker = nullptr;

	void Player_Car_GetPublics(map<const char*, string>* public_chars, map<const char*, int>* public_ints, map<const char*, float>* public_float, map<const char*, bool>* public_bools, map<const char*, GameObject*>* public_gos)
	{
		public_bools->insert(pair<const char*, bool>("have_item", have_item));
		public_bools->insert(pair<const char*, bool>("have_koma", have_koma));
		public_bools->insert(pair<const char*, bool>("have_triple_koma", have_triple_koma));
		public_bools->insert(pair<const char*, bool>("have_firecracker", have_firecracker));

		public_gos->insert(pair<const char*, GameObject*>("firecracker", nullptr));
	}

	void Player_Car_UpdatePublics(GameObject* game_object)
	{
		ComponentScript* test_script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);

		have_item = test_script->public_bools.at("have_item");
		have_koma = test_script->public_bools.at("have_koma");
		have_triple_koma = test_script->public_bools.at("have_triple_koma");
		have_firecracker = test_script->public_bools.at("have_firecracker");

		firecracker = test_script->public_gos.at("firecracker");
	}

	void Player_Car_ActualizePublics(GameObject* game_object)
	{
		ComponentScript* test_script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);

		test_script->public_chars.at("have_item") = have_item;
		test_script->public_chars.at("have_koma") = have_koma;
		test_script->public_chars.at("have_triple_koma") = have_triple_koma;
		test_script->public_chars.at("have_firecracker") = have_firecracker;

		test_script->public_gos.at("firecracker") = firecracker;
	}

	void Player_Car_Start(GameObject* game_object)
	{
	}

	void Player_Car_Update(GameObject* game_object)
	{
		ComponentCar* Player_car = (ComponentCar*)game_object->GetComponent(ComponentType::C_CAR);
		if (Player_car)
		{
			if (have_item)
			{
				switch (App->rnd->RandomInt(0, 2))
				{
				case 0:
					have_item = false;
					have_firecracker = true;
					break;
				case 1:
					have_item = false;
					have_koma = true;
					break;
				case 2:
					have_item = false;
					have_triple_koma = true;
					break;
				default:
					break;
				}
			}

			if (have_koma)
			{

			}
			if (have_triple_koma)
			{

			}
			if (have_firecracker)
			{
				if (firecracker)
				{
					firecracker->SetActive(true);
					firecracker->GetComponent(ComponentType::C_COLLIDER)->SetActive(false);
				}
				if (Player_car->GetCurrentTurbo() == T_ROCKET)
				{
					if (Player_car->GetAppliedTurbo()->timer >= Player_car->GetAppliedTurbo()->time)
					{
						have_firecracker = false;
						firecracker->SetActive(false);
						Player_car->ReleaseItem();
						Player_car->GetVehicle()->SetLinearSpeed(0.0f, 0.0f, 0.0f);
					}
				}
				if (!using_firecracker && App->input->GetJoystickButton(Player_car->GetBackPlayer(), JOY_BUTTON::B) == KEY_REPEAT)
				{
					Player_car->UseItem();
					using_firecracker = true;
				}

				if (using_firecracker && App->input->GetJoystickButton(Player_car->GetBackPlayer(), JOY_BUTTON::B) == KEY_UP)
				{
					have_firecracker = false;
					using_firecracker = false;
					firecracker->SetActive(false);
					GameObject* new_firecracker = App->go_manager->CreateGameObject(App->go_manager->root);
					new_firecracker = firecracker;
					new_firecracker->SetActive(true);
					new_firecracker->transform->SetPosition(new_firecracker->transform->GetPosition() + float3(0.0, 0.0, 2.0));
					new_firecracker->GetComponent(ComponentType::C_COLLIDER)->SetActive(true);
					Player_car->ReleaseItem();
				}

				if (!using_firecracker && App->input->GetKey(SDL_SCANCODE_Q) == KEY_REPEAT)
				{
					Player_car->UseItem();
					using_firecracker = true;
				}
				if (using_firecracker && App->input->GetKey(SDL_SCANCODE_Q) == KEY_UP)
				{
					have_firecracker = false;
					using_firecracker = false;
					firecracker->SetActive(false);
					GameObject* new_firecracker = App->go_manager->CreateGameObject(App->go_manager->root);
					new_firecracker = firecracker;
					new_firecracker->SetActive(true);
					new_firecracker->transform->SetPosition(new_firecracker->transform->GetPosition() + float3(0.0, 0.0, 2.0));
					new_firecracker->GetComponent(ComponentType::C_COLLIDER)->SetActive(true);
					Player_car->ReleaseItem();
				}
			}
		}
	}
	void Player_Car_OnCollision(GameObject* game_object, PhysBody3D* col)
	{
		ComponentCar* Player_car = (ComponentCar*)game_object->GetComponent(ComponentType::C_CAR);

		if (ReadFlag(col->collisionOptions, col->co_isItem))
		{
			ComponentCollider* item_col = col->GetCollider();
			if (item_col->GetGameObject()->name.compare("Hitodama"))
			{
				//Do something
			}
			else if (item_col->GetGameObject()->name.compare("Firecracker"))
			{
				Player_car->GetVehicle()->SetLinearSpeed(0.0f, 0.0f, 0.0f);
				App->go_manager->RemoveGameObject(item_col->GetGameObject());
			}
			else if (item_col->GetGameObject()->name.compare("Koma"))
			{
				Player_car->GetVehicle()->SetLinearSpeed(0.0f, 0.0f, 0.0f);
				App->go_manager->RemoveGameObject(item_col->GetGameObject());
			}
			else//item box
			{
				have_item = true;
			}
		}
	}
}