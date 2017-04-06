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
#include "../Time.h"

namespace Player_Car
{
	bool have_item = false;
	//bool have_koma = false;
	//bool have_triple_koma = false;
	bool have_firecracker = false;
	bool using_firecracker = false;
	bool throwing_firecracker = false;
	float velocity_firecracker = 40.0f;
	float time_trowing_firecracker = 0.0f;
	float explosion_radius_firecracker = 5.0f;
	//bool have_makibishi = false;
	string item_box_name = "item_box";
	GameObject* firecracker = nullptr;
	GameObject* other_car = nullptr;
	GameObject* scene_manager = nullptr;

	void Player_Car_GetPublics(map<const char*, string>* public_chars, map<const char*, int>* public_ints, map<const char*, float>* public_float, map<const char*, bool>* public_bools, map<const char*, GameObject*>* public_gos)
	{
		public_bools->insert(pair<const char*, bool>("have_item", have_item));
		//public_bools->insert(pair<const char*, bool>("have_koma", have_koma));
		//public_bools->insert(pair<const char*, bool>("have_triple_koma", have_triple_koma));
		public_bools->insert(pair<const char*, bool>("have_firecracker", have_firecracker));
		public_bools->insert(pair<const char*, bool>("using_firecracker", using_firecracker));
		public_bools->insert(pair<const char*, bool>("throwing_firecracker", throwing_firecracker));
		public_float->insert(pair<const char*, float>("velocity_firecracker", velocity_firecracker));
		public_float->insert(pair<const char*, float>("time_trowing_firecracker", time_trowing_firecracker));
		public_float->insert(pair<const char*, float>("explosion_radius_firecracker", explosion_radius_firecracker));
		//public_bools->insert(pair<const char*, bool>("have_makibishi", have_makibishi));
		public_chars->insert(pair<const char*, string>("item_box_name", item_box_name));

		public_gos->insert(pair<const char*, GameObject*>("firecracker", nullptr));
		public_gos->insert(pair<const char*, GameObject*>("other_car", nullptr));
		public_gos->insert(pair<const char*, GameObject*>("scene_manager", nullptr));
	}

	void Player_Car_UpdatePublics(GameObject* game_object)
	{
		ComponentScript* test_script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);

		have_item = test_script->public_bools.at("have_item");
		//have_koma = test_script->public_bools.at("have_koma");
		//have_triple_koma = test_script->public_bools.at("have_triple_koma");
		have_firecracker = test_script->public_bools.at("have_firecracker");
		using_firecracker = test_script->public_bools.at("using_firecracker");
		throwing_firecracker = test_script->public_bools.at("throwing_firecracker");
		velocity_firecracker = test_script->public_floats.at("velocity_firecracker");
		time_trowing_firecracker = test_script->public_floats.at("time_trowing_firecracker");
		explosion_radius_firecracker = test_script->public_floats.at("explosion_radius_firecracker");
		//have_makibishi = test_script->public_bools.at("have_makibishi");
		item_box_name = test_script->public_chars.at("item_box_name");

		firecracker = test_script->public_gos.at("firecracker");
		other_car = test_script->public_gos.at("other_car");
		scene_manager = test_script->public_gos.at("scene_manager");
	}

	void Player_Car_ActualizePublics(GameObject* game_object)
	{
		ComponentScript* test_script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);

		test_script->public_bools.at("have_item") = have_item;
		//test_script->public_bools.at("have_koma") = have_koma;
		//test_script->public_bools.at("have_triple_koma") = have_triple_koma;
		test_script->public_bools.at("have_firecracker") = have_firecracker;
		test_script->public_bools.at("using_firecracker") = using_firecracker;
		test_script->public_bools.at("throwing_firecracker") = throwing_firecracker;
		test_script->public_floats.at("velocity_firecracker") = velocity_firecracker;
		test_script->public_floats.at("time_trowing_firecracker") = time_trowing_firecracker;
		test_script->public_floats.at("explosion_radius_firecracker") = explosion_radius_firecracker;
		//test_script->public_bools.at("have_makibishi") = have_makibishi;
		test_script->public_chars.at("item_box_name") = item_box_name;

		test_script->public_gos.at("firecracker") = firecracker;
		test_script->public_gos.at("other_car") = other_car;
		test_script->public_gos.at("scene_manager") = scene_manager;
	}

	void Player_Car_Start(GameObject* game_object)
	{
	}

	void Player_Car_LoseItem(GameObject* game_object);
	void Player_Car_PickItem(GameObject* game_object);

	void Player_Car_Update(GameObject* game_object)
	{
		ComponentCar* Player_car = (ComponentCar*)game_object->GetComponent(ComponentType::C_CAR);
		if (Player_car)
		{
			if (have_item)
			{
				if (!have_firecracker/* || !have_koma || !have_triple_koma || !have_makibishi*/)
				{
					unsigned int percentage = App->rnd->RandomInt(0, 99);

					if (other_car != nullptr)
					{
						//int Player_car_distance = game_object->GetComponent(ComponentType::C_CAR) check distance between cars
					}
					have_item = false;
					Player_Car_PickItem(game_object);
				}
			}

			/*if (have_koma)
			{

			}
			if (have_triple_koma)
			{

			}*/
			if (throwing_firecracker)
			{
				if (!firecracker->IsActive())
				{
					throwing_firecracker = false;
					time_trowing_firecracker = 0.0f;
				}
				else
				{
					ComponentCollider* firecracker_col = (ComponentCollider*)firecracker->GetComponent(ComponentType::C_COLLIDER);
					float3 new_pos = firecracker_col->body->GetPosition();
					new_pos += firecracker_col->body->GetTransform().WorldZ().Normalized() * velocity_firecracker * time->DeltaTime();
					firecracker_col->body->SetPos(new_pos.x, new_pos.y, new_pos.z);
					time_trowing_firecracker += time->DeltaTime();
					if (time_trowing_firecracker >= Player_car->rocket_turbo.time)
					{
						throwing_firecracker = false;
						time_trowing_firecracker = 0.0f;
						firecracker->SetActive(false);
						firecracker->GetComponent(ComponentType::C_COLLIDER)->SetActive(false);
					}
				}
			}
			else if (have_firecracker)
			{
				if (firecracker != nullptr)
				{
					firecracker->SetActive(true);
					firecracker->GetComponent(ComponentType::C_COLLIDER)->SetActive(false);
					float3 last_scale = firecracker->transform->GetScale();
					((ComponentCollider*)firecracker->GetComponent(ComponentType::C_COLLIDER))->body->SetTransform(game_object->transform->GetTransformMatrix().Transposed().ptr());
					firecracker->transform->Set(game_object->GetGlobalMatrix());
					firecracker->transform->SetScale(last_scale);
				}
				if (Player_car->GetCurrentTurbo() == T_ROCKET)
				{
					if (Player_car->GetAppliedTurbo()->timer >= Player_car->GetAppliedTurbo()->time)
					{
						if (other_car != nullptr)
						{
							if (firecracker && firecracker->transform->GetPosition().Distance(other_car->transform->GetPosition()) <= explosion_radius_firecracker)
								((ComponentCar*)other_car->GetComponent(ComponentType::C_CAR))->GetVehicle()->SetLinearSpeed(0.0f, 0.0f, 0.0f);
						}
						Player_Car_LoseItem(game_object);

						using_firecracker = false;
						if (firecracker) firecracker->SetActive(false);
						Player_car->ReleaseItem();
						Player_car->GetVehicle()->SetLinearSpeed(0.0f, 0.0f, 0.0f);
					}
				}
				if (have_firecracker)
				{
					if (!using_firecracker && App->input->GetJoystickButton(Player_car->GetBackPlayer(), JOY_BUTTON::B) == KEY_REPEAT)
					{
						Player_car->UseItem();
						using_firecracker = true;
					}

					if (using_firecracker && App->input->GetJoystickButton(Player_car->GetBackPlayer(), JOY_BUTTON::B) == KEY_UP)
					{
						Player_Car_LoseItem(game_object);
						using_firecracker = false;
						if (firecracker != nullptr)
						{
							float3 new_pos = game_object->transform->GetPosition();
							new_pos += game_object->transform->GetForward().Normalized() * Player_car->chasis_size.z * 2;
							((ComponentCollider*)firecracker->GetComponent(ComponentType::C_COLLIDER))->body->SetPos(new_pos.x, new_pos.y, new_pos.z);
							firecracker->GetComponent(ComponentType::C_COLLIDER)->SetActive(true);
							throwing_firecracker = true;
							time_trowing_firecracker = Player_car->GetAppliedTurbo()->timer;
						}
						Player_car->ReleaseItem();
					}

					if (!using_firecracker && App->input->GetKey(SDL_SCANCODE_Q) == KEY_REPEAT)
					{
						Player_car->UseItem();
						using_firecracker = true;
					}
					if (using_firecracker && App->input->GetKey(SDL_SCANCODE_Q) == KEY_UP)
					{
						Player_Car_LoseItem(game_object);
						using_firecracker = false;
						if (firecracker != nullptr)
						{
							float3 new_pos = game_object->transform->GetPosition();
							new_pos += game_object->transform->GetForward().Normalized() * Player_car->chasis_size.z * 2;
							((ComponentCollider*)firecracker->GetComponent(ComponentType::C_COLLIDER))->body->SetPos(new_pos.x, new_pos.y, new_pos.z);
							firecracker->GetComponent(ComponentType::C_COLLIDER)->SetActive(true);
							throwing_firecracker = true;
							time_trowing_firecracker = Player_car->GetAppliedTurbo()->timer;
						}
						Player_car->ReleaseItem();
					}
				}
			}
		}
	}

	void Player_Car_OnCollision(GameObject* game_object, PhysBody3D* col)
	{
		ComponentCar* Player_car = (ComponentCar*)game_object->GetComponent(ComponentType::C_CAR);

		if (col->IsTrigger())
		{
			if (!col->IsCar())
			{
				ComponentCollider* item_col = col->GetCollider();
				if (item_col->IsActive())
				{
					if (item_col->GetGameObject()->name == "Hitodama")
					{
						//Do something
					}
					else if (item_col->GetGameObject()->name == "Firecracker")
					{
						if (firecracker != col->GetCollider()->GetGameObject())
						{
							Player_car->GetVehicle()->SetLinearSpeed(0.0f, 0.0f, 0.0f);
							if (firecracker) firecracker->GetComponent(ComponentType::C_COLLIDER)->SetActive(false);
							item_col->GetGameObject()->SetActive(false);
							item_col->SetActive(false);
						}
					}
					else if (item_col->GetGameObject()->name == "Koma")
					{
						Player_car->GetVehicle()->SetLinearSpeed(0.0f, 0.0f, 0.0f);
						if (firecracker) firecracker->GetComponent(ComponentType::C_COLLIDER)->SetActive(false);
						item_col->GetGameObject()->SetActive(false);
						item_col->SetActive(false);
					}
					else if (item_col->GetGameObject()->name == item_box_name.c_str())
					{
						have_item = true;
					}
				}
			}
		}
	}

	void Player_Car_PickItem(GameObject* game_object)
	{
		have_firecracker = true;
		if (scene_manager != nullptr)
		{
			string function_path = ((ComponentScript*)scene_manager->GetComponent(C_SCRIPT))->GetPath();
			function_path.append("_UpdateItems");
			if (f_UpdateItems update_items = (f_UpdateItems)GetProcAddress(App->scripting->scripts_lib->lib, function_path.c_str()))
			{
				update_items(((ComponentCar*)game_object->GetComponent(C_CAR))->team, true);
			}
			else
			{
				LOG("Scripting error: %s", GetLastError());
			}
		}
	}

	void Player_Car_LoseItem(GameObject* game_object)
	{
		have_firecracker = false;
		if (scene_manager != nullptr)
		{
			string function_path = ((ComponentScript*)scene_manager->GetComponent(C_SCRIPT))->GetPath();
			function_path.append("_UpdateItems");
			if (f_UpdateItems update_items = (f_UpdateItems)GetProcAddress(App->scripting->scripts_lib->lib, function_path.c_str()))
			{
				update_items(((ComponentCar*)game_object->GetComponent(C_CAR))->team, false);
			}
			else
			{
				LOG("Scripting error: %s", GetLastError());
			}
		}
	}
}