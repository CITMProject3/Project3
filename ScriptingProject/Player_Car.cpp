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
	bool have_koma = false;
	bool have_triple_koma = false;
	bool have_firecracker = false;
	bool using_item = false;
	bool throwing_firecracker = false;
	bool throwing_koma = false;
	int koma_quantity = 0;
	float velocity_firecracker = 40.0f;
	float time_trowing_firecracker = 0.0f;
	float explosion_radius_firecracker = 5.0f;
	float time_trowing_koma = 100.0f;
	float current_time_trowing_koma1 = 0.0f;
	float current_time_trowing_koma2 = 0.0f;
	float current_time_trowing_koma3 = 0.0f;
	float velocity_koma = 40.0f;
	//bool have_makibishi = false;
	string item_box_name = "item_box";
	GameObject* firecracker = nullptr;
	GameObject* koma1 = nullptr;
	GameObject* koma2 = nullptr;
	GameObject* koma3 = nullptr;
	GameObject* other_car = nullptr;
	GameObject* scene_manager = nullptr;

	void Player_Car_GetPublics(map<const char*, string>* public_chars, map<const char*, int>* public_ints, map<const char*, float>* public_float, map<const char*, bool>* public_bools, map<const char*, GameObject*>* public_gos)
	{
		public_bools->insert(pair<const char*, bool>("have_item", have_item));
		public_bools->insert(pair<const char*, bool>("have_koma", have_koma));
		public_bools->insert(pair<const char*, bool>("have_triple_koma", have_triple_koma));
		public_bools->insert(pair<const char*, bool>("have_firecracker", have_firecracker));
		public_bools->insert(pair<const char*, bool>("using_item", using_item));
		public_bools->insert(pair<const char*, bool>("throwing_firecracker", throwing_firecracker));
		public_float->insert(pair<const char*, float>("velocity_firecracker", velocity_firecracker));
		public_float->insert(pair<const char*, float>("time_trowing_firecracker", time_trowing_firecracker));
		public_float->insert(pair<const char*, float>("explosion_radius_firecracker", explosion_radius_firecracker));
		public_float->insert(pair<const char*, float>("time_trowing_koma", time_trowing_koma));
		public_float->insert(pair<const char*, float>("current_time_trowing_koma1", current_time_trowing_koma1));
		public_float->insert(pair<const char*, float>("current_time_trowing_koma2", current_time_trowing_koma2));
		public_float->insert(pair<const char*, float>("current_time_trowing_koma3", current_time_trowing_koma3));
		public_float->insert(pair<const char*, float>("velocity_koma", velocity_koma));
		public_bools->insert(pair<const char*, bool>("throwing_koma", throwing_koma));
		public_ints->insert(pair<const char*, int>("koma_quantity", koma_quantity));
		//public_bools->insert(pair<const char*, bool>("have_makibishi", have_makibishi));
		public_chars->insert(pair<const char*, string>("item_box_name", item_box_name));

		public_gos->insert(pair<const char*, GameObject*>("firecracker", nullptr));
		public_gos->insert(pair<const char*, GameObject*>("koma1", nullptr));
		public_gos->insert(pair<const char*, GameObject*>("koma2", nullptr));
		public_gos->insert(pair<const char*, GameObject*>("koma3", nullptr));
		public_gos->insert(pair<const char*, GameObject*>("other_car", nullptr));
		public_gos->insert(pair<const char*, GameObject*>("scene_manager", nullptr));
	}

	void Player_Car_UpdatePublics(GameObject* game_object)
	{
		ComponentScript* test_script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);

		have_item = test_script->public_bools.at("have_item");
		have_koma = test_script->public_bools.at("have_koma");
		have_triple_koma = test_script->public_bools.at("have_triple_koma");
		have_firecracker = test_script->public_bools.at("have_firecracker");
		using_item = test_script->public_bools.at("using_item");
		throwing_firecracker = test_script->public_bools.at("throwing_firecracker");
		velocity_firecracker = test_script->public_floats.at("velocity_firecracker");
		time_trowing_firecracker = test_script->public_floats.at("time_trowing_firecracker");
		explosion_radius_firecracker = test_script->public_floats.at("explosion_radius_firecracker");
		time_trowing_koma = test_script->public_floats.at("time_trowing_koma");
		current_time_trowing_koma1 = test_script->public_floats.at("current_time_trowing_koma1");
		current_time_trowing_koma2 = test_script->public_floats.at("current_time_trowing_koma2");
		current_time_trowing_koma3 = test_script->public_floats.at("current_time_trowing_koma3");
		velocity_koma = test_script->public_floats.at("velocity_koma");
		throwing_koma = test_script->public_bools.at("throwing_koma");
		koma_quantity = test_script->public_ints.at("koma_quantity");
		//have_makibishi = test_script->public_bools.at("have_makibishi");
		item_box_name = test_script->public_chars.at("item_box_name");

		firecracker = test_script->public_gos.at("firecracker");
		koma1 = test_script->public_gos.at("koma1");
		koma2 = test_script->public_gos.at("koma2");
		koma3 = test_script->public_gos.at("koma3");
		other_car = test_script->public_gos.at("other_car");
		scene_manager = test_script->public_gos.at("scene_manager");
	}

	void Player_Car_ActualizePublics(GameObject* game_object)
	{
		ComponentScript* test_script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);

		test_script->public_bools.at("have_item") = have_item;
		test_script->public_bools.at("have_koma") = have_koma;
		test_script->public_bools.at("have_triple_koma") = have_triple_koma;
		test_script->public_bools.at("have_firecracker") = have_firecracker;
		test_script->public_bools.at("using_item") = using_item;
		test_script->public_bools.at("throwing_firecracker") = throwing_firecracker;
		test_script->public_floats.at("velocity_firecracker") = velocity_firecracker;
		test_script->public_floats.at("time_trowing_firecracker") = time_trowing_firecracker;
		test_script->public_floats.at("explosion_radius_firecracker") = explosion_radius_firecracker;
		test_script->public_floats.at("time_trowing_koma") = time_trowing_koma;
		test_script->public_floats.at("current_time_trowing_koma1") = current_time_trowing_koma1;
		test_script->public_floats.at("current_time_trowing_koma2") = current_time_trowing_koma2;
		test_script->public_floats.at("current_time_trowing_koma3") = current_time_trowing_koma3;
		test_script->public_floats.at("velocity_koma") = velocity_koma;
		test_script->public_bools.at("throwing_koma") = throwing_koma;
		test_script->public_ints.at("koma_quantity") = koma_quantity;
		//test_script->public_bools.at("have_makibishi") = have_makibishi;
		test_script->public_chars.at("item_box_name") = item_box_name;

		test_script->public_gos.at("firecracker") = firecracker;
		test_script->public_gos.at("koma1") = koma1;
		test_script->public_gos.at("koma2") = koma2;
		test_script->public_gos.at("koma3") = koma3;
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
				if (have_firecracker || have_koma || have_triple_koma/* || have_makibishi*/)
					have_item = false;
				else
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

			if (have_koma)
			{
				if (throwing_koma)
				{
					if (!koma1->IsActive())
					{
						throwing_koma = false;
						current_time_trowing_koma1 = 0.0f;
						have_koma = false;
					}
					else
					{
						float3 new_pos = koma1->transform->GetPosition();
						new_pos += koma1->transform->GetForward().Normalized() * velocity_koma * time->DeltaTime();
						koma1->transform->SetPosition(new_pos);
						current_time_trowing_koma1 += time->DeltaTime();
						if (current_time_trowing_koma1 >= time_trowing_koma)
						{
							throwing_koma = false;
							current_time_trowing_koma1 = 0.0f;
							koma1->SetActive(false);
							koma1->GetComponent(ComponentType::C_COLLIDER)->SetActive(false);
							have_koma = false;
						}
					}
				}
				else
				{
					if (App->input->GetJoystickButton(Player_car->GetBackPlayer(), JOY_BUTTON::B) == KEY_UP)
					{
						if (koma1 != nullptr)
						{
							float3 new_pos = koma1->transform->GetPosition();
							new_pos += koma1->transform->GetForward().Normalized() * Player_car->chasis_size.z;
							koma1->transform->SetPosition(new_pos);
							koma1->GetComponent(ComponentType::C_COLLIDER)->SetActive(true);
							throwing_koma = true;
							current_time_trowing_koma1 = 0.0f;
						}
					}

					if (using_item && App->input->GetKey(SDL_SCANCODE_Q) == KEY_UP)
					{
						if (koma1 != nullptr)
						{
							float3 new_pos = koma1->transform->GetPosition();
							new_pos += koma1->transform->GetForward().Normalized() * Player_car->chasis_size.z;
							koma1->transform->SetPosition(new_pos);
							koma1->GetComponent(ComponentType::C_COLLIDER)->SetActive(true);
							throwing_koma = true;
							current_time_trowing_koma1 = 0.0f;
						}
					}
				}
			}

			if (have_triple_koma)
			{
				if (throwing_koma)
				{
					if (!koma1->IsActive())
					{
						current_time_trowing_koma1 = 0.0f;
					}
					else
					{
						float3 new_pos = koma1->transform->GetPosition();
						new_pos += koma1->transform->GetForward().Normalized() * velocity_koma * time->DeltaTime();
						koma1->transform->SetPosition(new_pos);
						current_time_trowing_koma1 += time->DeltaTime();
						if (current_time_trowing_koma1 >= time_trowing_koma)
						{
							current_time_trowing_koma1 = 0.0f;
							koma1->SetActive(false);
							koma1->GetComponent(ComponentType::C_COLLIDER)->SetActive(false);
						}
					}
					if(koma_quantity <= 1)
					{
						if (!koma2->IsActive())
						{
							current_time_trowing_koma2 = 0.0f;
						}
						else
						{
							float3 new_pos = koma2->transform->GetPosition();
							new_pos += koma2->transform->GetForward().Normalized() * velocity_koma * time->DeltaTime();
							koma2->transform->SetPosition(new_pos);
							current_time_trowing_koma2 += time->DeltaTime();
							if (current_time_trowing_koma2 >= time_trowing_koma)
							{
								current_time_trowing_koma2 = 0.0f;
								koma2->SetActive(false);
								koma2->GetComponent(ComponentType::C_COLLIDER)->SetActive(false);
							}
						}
					}
					if (koma_quantity <= 0)
					{
						if (!koma3->IsActive())
						{
							throwing_koma = false;
							current_time_trowing_koma3 = 0.0f;
							have_triple_koma = false;
						}
						else
						{
							float3 new_pos = koma3->transform->GetPosition();
							new_pos += koma3->transform->GetForward().Normalized() * velocity_koma * time->DeltaTime();
							koma3->transform->SetPosition(new_pos);
							current_time_trowing_koma3 += time->DeltaTime();
							if (current_time_trowing_koma3 >= time_trowing_koma)
							{
								throwing_koma = false;
								current_time_trowing_koma3 = 0.0f;
								koma3->SetActive(false);
								koma3->GetComponent(ComponentType::C_COLLIDER)->SetActive(false);
								have_triple_koma = false;
							}
						}
					}
				}
				else if(koma_quantity > 0)
				{
					if (App->input->GetJoystickButton(Player_car->GetBackPlayer(), JOY_BUTTON::B) == KEY_UP)
					{
						if (koma_quantity == 3)
						{
							koma_quantity--;
							if (koma1 != nullptr)
							{
								float3 new_pos = koma1->transform->GetPosition();
								new_pos += koma1->transform->GetForward().Normalized() * Player_car->chasis_size.z;
								koma1->transform->SetPosition(new_pos);
								koma1->GetComponent(ComponentType::C_COLLIDER)->SetActive(true);
								throwing_koma = true;
								current_time_trowing_koma1 = 0.0f;
							}
						}
						else if(koma_quantity == 2)
						{
							koma_quantity--;
							if (koma2 != nullptr)
							{
								float3 new_pos = koma2->transform->GetPosition();
								new_pos += koma2->transform->GetForward().Normalized() * Player_car->chasis_size.z;
								koma2->transform->SetPosition(new_pos);
								koma2->GetComponent(ComponentType::C_COLLIDER)->SetActive(true);
								throwing_koma = true;
								current_time_trowing_koma2 = 0.0f;
							}
						}
						else if (koma_quantity == 1)
						{
							koma_quantity--;
							if (koma3 != nullptr)
							{
								float3 new_pos = koma3->transform->GetPosition();
								new_pos += koma3->transform->GetForward().Normalized() * Player_car->chasis_size.z;
								koma3->transform->SetPosition(new_pos);
								koma3->GetComponent(ComponentType::C_COLLIDER)->SetActive(true);
								throwing_koma = true;
								current_time_trowing_koma3 = 0.0f;
							}
						}
					}

					if (using_item && App->input->GetKey(SDL_SCANCODE_Q) == KEY_UP)
					{
						if (koma_quantity == 3)
						{
							if (koma1 != nullptr)
							{
								float3 new_pos = koma1->transform->GetPosition();
								new_pos += koma1->transform->GetForward().Normalized() * Player_car->chasis_size.z;
								koma1->transform->SetPosition(new_pos);
								koma1->GetComponent(ComponentType::C_COLLIDER)->SetActive(true);
								throwing_koma = true;
								current_time_trowing_koma1 = 0.0f;
							}
						}
					}
				}
			}

			if (throwing_firecracker)
			{
				if (!firecracker->IsActive())
				{
					throwing_firecracker = false;
					time_trowing_firecracker = 0.0f;
				}
				else
				{
					float3 new_pos = firecracker->transform->GetPosition();
					new_pos += firecracker->transform->GetForward().Normalized() * velocity_firecracker * time->DeltaTime();
					firecracker->transform->SetPosition(new_pos);
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

						using_item = false;
						if (firecracker) firecracker->SetActive(false);
						Player_car->ReleaseItem();
						Player_car->GetVehicle()->SetLinearSpeed(0.0f, 0.0f, 0.0f);
					}
				}
				if (have_firecracker)
				{
					if (!using_item && App->input->GetJoystickButton(Player_car->GetBackPlayer(), JOY_BUTTON::B) == KEY_REPEAT)
					{
						Player_car->UseItem();
						using_item = true;
					}

					if (using_item && App->input->GetJoystickButton(Player_car->GetBackPlayer(), JOY_BUTTON::B) == KEY_UP)
					{
						Player_Car_LoseItem(game_object);
						using_item = false;
						if (firecracker != nullptr)
						{
							float3 new_pos = firecracker->transform->GetPosition();
							new_pos += firecracker->transform->GetForward().Normalized() * Player_car->chasis_size.z;
							firecracker->transform->SetPosition(new_pos);
							firecracker->GetComponent(ComponentType::C_COLLIDER)->SetActive(true);
							throwing_firecracker = true;
							time_trowing_firecracker = Player_car->GetAppliedTurbo()->timer;
						}
						Player_car->ReleaseItem();
					}

					if (!using_item && App->input->GetKey(SDL_SCANCODE_Q) == KEY_REPEAT)
					{
						Player_car->UseItem();
						using_item = true;
					}
					if (using_item && App->input->GetKey(SDL_SCANCODE_Q) == KEY_UP)
					{
						Player_Car_LoseItem(game_object);
						using_item = false;
						if (firecracker != nullptr)
						{
							float3 new_pos = firecracker->transform->GetPosition();
							new_pos += firecracker->transform->GetForward().Normalized() * Player_car->chasis_size.z;
							firecracker->transform->SetPosition(new_pos);
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
						Player_car->GetVehicle()->SetLinearSpeed(0.0f, 0.0f, 0.0f);
						if (firecracker) firecracker->GetComponent(ComponentType::C_COLLIDER)->SetActive(false);
						item_col->GetGameObject()->SetActive(false);
						item_col->SetActive(false);
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