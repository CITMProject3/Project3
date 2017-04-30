#include "stdafx.h"

#include <string>
#include <map>
#include <vector>
#include <list>
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
	bool have_makibishi = false;
	bool have_triple_makibishi = false;
	bool have_firecracker = false;
	bool using_item = false;
	bool throwing_firecracker = false;
	bool throwing_makibishi = false;
	int makibishi_quantity = 0;
	float velocity_firecracker = 40.0f;
	float time_trowing_firecracker = 0.0f;
	float explosion_radius_firecracker = 5.0f;
	float velocity_makibishi = 40.0f;
	//bool have_evil_spirit = false;
	string item_box_name = "item_box";
	GameObject* firecracker = nullptr;
	GameObject* makibishi1 = nullptr;
	GameObject* makibishi2 = nullptr;
	GameObject* makibishi3 = nullptr;
	GameObject* other_car = nullptr;
	GameObject* scene_manager = nullptr;

	void Player_Car_GetPublics(map<const char*, string>* public_chars, map<const char*, int>* public_ints, map<const char*, float>* public_float, map<const char*, bool>* public_bools, map<const char*, GameObject*>* public_gos)
	{
		public_bools->insert(pair<const char*, bool>("have_item", have_item));
		public_bools->insert(pair<const char*, bool>("have_makibishi", have_makibishi));
		public_bools->insert(pair<const char*, bool>("have_triple_makibishi", have_triple_makibishi));
		public_bools->insert(pair<const char*, bool>("have_firecracker", have_firecracker));
		public_bools->insert(pair<const char*, bool>("using_item", using_item));
		public_bools->insert(pair<const char*, bool>("throwing_firecracker", throwing_firecracker));
		public_float->insert(pair<const char*, float>("velocity_firecracker", velocity_firecracker));
		public_float->insert(pair<const char*, float>("time_trowing_firecracker", time_trowing_firecracker));
		public_float->insert(pair<const char*, float>("explosion_radius_firecracker", explosion_radius_firecracker));
		public_float->insert(pair<const char*, float>("velocity_makibishi", velocity_makibishi));
		public_bools->insert(pair<const char*, bool>("throwing_makibishi", throwing_makibishi));
		public_ints->insert(pair<const char*, int>("makibishi_quantity", makibishi_quantity));
		//public_bools->insert(pair<const char*, bool>("have_evil_spirit", have_evil_spirit));
		public_chars->insert(pair<const char*, string>("item_box_name", item_box_name));

		public_gos->insert(pair<const char*, GameObject*>("firecracker", nullptr));
		public_gos->insert(pair<const char*, GameObject*>("makibishi1", nullptr));
		public_gos->insert(pair<const char*, GameObject*>("makibishi2", nullptr));
		public_gos->insert(pair<const char*, GameObject*>("makibishi3", nullptr));
		public_gos->insert(pair<const char*, GameObject*>("other_car", nullptr));
		public_gos->insert(pair<const char*, GameObject*>("scene_manager", nullptr));
	}

	void Player_Car_UpdatePublics(GameObject* game_object)
	{
		ComponentScript* test_script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);

		have_item = test_script->public_bools.at("have_item");
		have_makibishi = test_script->public_bools.at("have_makibishi");
		have_triple_makibishi = test_script->public_bools.at("have_triple_makibishi");
		have_firecracker = test_script->public_bools.at("have_firecracker");
		using_item = test_script->public_bools.at("using_item");
		throwing_firecracker = test_script->public_bools.at("throwing_firecracker");
		velocity_firecracker = test_script->public_floats.at("velocity_firecracker");
		time_trowing_firecracker = test_script->public_floats.at("time_trowing_firecracker");
		explosion_radius_firecracker = test_script->public_floats.at("explosion_radius_firecracker");
		velocity_makibishi = test_script->public_floats.at("velocity_makibishi");
		throwing_makibishi = test_script->public_bools.at("throwing_makibishi");
		makibishi_quantity = test_script->public_ints.at("makibishi_quantity");
		//have_evil_spirit = test_script->public_bools.at("have_evil_spirit");
		item_box_name = test_script->public_chars.at("item_box_name");

		firecracker = test_script->public_gos.at("firecracker");
		makibishi1 = test_script->public_gos.at("makibishi1");
		makibishi2 = test_script->public_gos.at("makibishi2");
		makibishi3 = test_script->public_gos.at("makibishi3");
		other_car = test_script->public_gos.at("other_car");
		scene_manager = test_script->public_gos.at("scene_manager");
	}

	void Player_Car_ActualizePublics(GameObject* game_object)
	{
		ComponentScript* test_script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);

		test_script->public_bools.at("have_item") = have_item;
		test_script->public_bools.at("have_makibishi") = have_makibishi;
		test_script->public_bools.at("have_triple_makibishi") = have_triple_makibishi;
		test_script->public_bools.at("have_firecracker") = have_firecracker;
		test_script->public_bools.at("using_item") = using_item;
		test_script->public_bools.at("throwing_firecracker") = throwing_firecracker;
		test_script->public_floats.at("velocity_firecracker") = velocity_firecracker;
		test_script->public_floats.at("time_trowing_firecracker") = time_trowing_firecracker;
		test_script->public_floats.at("explosion_radius_firecracker") = explosion_radius_firecracker;
		test_script->public_floats.at("velocity_makibishi") = velocity_makibishi;
		test_script->public_bools.at("throwing_makibishi") = throwing_makibishi;
		test_script->public_ints.at("makibishi_quantity") = makibishi_quantity;
		//test_script->public_bools.at("have_evil_spirit") = have_evil_spirit;
		test_script->public_chars.at("item_box_name") = item_box_name;

		test_script->public_gos.at("firecracker") = firecracker;
		test_script->public_gos.at("makibishi1") = makibishi1;
		test_script->public_gos.at("makibishi2") = makibishi2;
		test_script->public_gos.at("makibishi3") = makibishi3;
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
				if (have_firecracker || have_makibishi || have_triple_makibishi/* || have_evil_spirit*/)
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

					if (have_makibishi)
					{
						makibishi1 = makibishi2 = makibishi3 = nullptr;
						for (list<GameObject*>::iterator item = App->go_manager->dynamic_gameobjects.begin(); item != App->go_manager->dynamic_gameobjects.end(); item++)
						{
							if ((*item)->name == "Makibishi" && !(*item)->IsActive())
							{
								makibishi1 = (*item);
								break;
							}
						}
						if (makibishi1 == nullptr)
						{
							for (list<GameObject*>::iterator item = App->go_manager->dynamic_gameobjects.begin(); item != App->go_manager->dynamic_gameobjects.end(); item++)
							{
								if ((*item)->name == "Makibishi")
								{
									if(makibishi1 == nullptr)
										makibishi1 = (*item);
									else
									{
										if (((ComponentScript*)(*item)->GetComponent(ComponentType::C_SCRIPT))->public_floats.at("current_time_trowing_makibishi") > ((ComponentScript*)makibishi1->GetComponent(ComponentType::C_SCRIPT))->public_floats.at("current_time_trowing_makibishi"))
											makibishi1 = (*item);
									}
								}
							}
							((ComponentScript*)makibishi1->GetComponent(ComponentType::C_SCRIPT))->public_floats.at("current_time_trowing_makibishi") = 0.0f;
						}
					}
					else if (have_triple_makibishi)
					{
						makibishi1 = makibishi2 = makibishi3 = nullptr;
						for (list<GameObject*>::iterator item = App->go_manager->dynamic_gameobjects.begin(); item != App->go_manager->dynamic_gameobjects.end(); item++)
						{
							if ((*item)->name == "Makibishi" && !(*item)->IsActive())
							{
								if (makibishi1 == nullptr)
								{
									makibishi1 = (*item);
								}
								else if (makibishi2 == nullptr)
								{
									makibishi2 = (*item);
								}
								else if (makibishi3 == nullptr)
								{
									makibishi3 = (*item);
									break;
								}
							}
						}
						if (makibishi1 == nullptr)
						{
							for (list<GameObject*>::iterator item = App->go_manager->dynamic_gameobjects.begin(); item != App->go_manager->dynamic_gameobjects.end(); item++)
							{
								if ((*item)->name == "Makibishi")
								{
									if (makibishi1 == nullptr)
										makibishi1 = (*item);
									else if (makibishi2 == nullptr)
										makibishi2 = (*item);
									else if (makibishi3 == nullptr)
										makibishi3 = (*item);
									else
									{
										if (((ComponentScript*)(*item)->GetComponent(ComponentType::C_SCRIPT))->public_floats.at("current_time_trowing_makibishi") > ((ComponentScript*)makibishi2->GetComponent(ComponentType::C_SCRIPT))->public_floats.at("current_time_trowing_makibishi"))
										{
											makibishi3 = makibishi2;
											makibishi2 = makibishi1;
											makibishi1 = (*item);
										}
									}
								}
							}
							((ComponentScript*)makibishi1->GetComponent(ComponentType::C_SCRIPT))->public_floats.at("current_time_trowing_makibishi") = 0.0f;
							((ComponentScript*)makibishi2->GetComponent(ComponentType::C_SCRIPT))->public_floats.at("current_time_trowing_makibishi") = 0.0f;
							((ComponentScript*)makibishi3->GetComponent(ComponentType::C_SCRIPT))->public_floats.at("current_time_trowing_makibishi") = 0.0f;
						}
						else if(makibishi2 == nullptr)
						{
							for (list<GameObject*>::iterator item = App->go_manager->dynamic_gameobjects.begin(); item != App->go_manager->dynamic_gameobjects.end(); item++)
							{
								if ((*item)->name == "Makibishi")
								{
									if (makibishi2 == nullptr)
										makibishi2 = (*item);
									else if (makibishi3 == nullptr)
										makibishi3 = (*item);
									else
									{
										if (((ComponentScript*)(*item)->GetComponent(ComponentType::C_SCRIPT))->public_floats.at("current_time_trowing_makibishi") > ((ComponentScript*)makibishi2->GetComponent(ComponentType::C_SCRIPT))->public_floats.at("current_time_trowing_makibishi"))
										{
											makibishi3 = makibishi2;
											makibishi2 = (*item);
										}
									}
								}
							}
							((ComponentScript*)makibishi3->GetComponent(ComponentType::C_SCRIPT))->public_floats.at("current_time_trowing_makibishi") = 0.0f;
							((ComponentScript*)makibishi2->GetComponent(ComponentType::C_SCRIPT))->public_floats.at("current_time_trowing_makibishi") = 0.0f;
						}
						else if (makibishi3 == nullptr)
						{
							for (list<GameObject*>::iterator item = App->go_manager->dynamic_gameobjects.begin(); item != App->go_manager->dynamic_gameobjects.end(); item++)
							{
								if ((*item)->name == "Makibishi")
								{
									if (makibishi3 == nullptr)
										makibishi3 = (*item);
									else
									{
										if (((ComponentScript*)(*item)->GetComponent(ComponentType::C_SCRIPT))->public_floats.at("current_time_trowing_makibishi") > ((ComponentScript*)makibishi3->GetComponent(ComponentType::C_SCRIPT))->public_floats.at("current_time_trowing_makibishi"))
											makibishi3 = (*item);
									}
								}
							}
							((ComponentScript*)makibishi3->GetComponent(ComponentType::C_SCRIPT))->public_floats.at("current_time_trowing_makibishi") = 0.0f;
						}
					}
				}
			}

			if (have_makibishi)
			{
				if (throwing_makibishi)
				{
					if (!makibishi1->IsActive())
					{
						throwing_makibishi = false;
						((ComponentScript*)makibishi1->GetComponent(ComponentType::C_SCRIPT))->public_floats.at("current_time_trowing_makibishi") = 0.0f;
						have_makibishi = false;
					}
				}
				else
				{
					if (App->input->GetJoystickButton(Player_car->GetBackPlayer(), JOY_BUTTON::B) == KEY_UP || using_item && App->input->GetKey(SDL_SCANCODE_Q) == KEY_UP)
					{
						if (makibishi1 != nullptr)
						{
							float3 new_pos = game_object->transform->GetPosition();
							new_pos += game_object->transform->GetForward().Normalized() * Player_car->chasis_size.z * 2;
							((ComponentCollider*)makibishi1->GetComponent(ComponentType::C_COLLIDER))->body->SetTransform(game_object->transform->GetTransformMatrix().Transposed().ptr());
							((ComponentCollider*)makibishi1->GetComponent(ComponentType::C_COLLIDER))->body->SetPos(new_pos.x, new_pos.y, new_pos.z);
							makibishi1->GetComponent(ComponentType::C_COLLIDER)->SetActive(true);
							throwing_makibishi = true;
							((ComponentScript*)makibishi1->GetComponent(ComponentType::C_SCRIPT))->public_floats.at("current_time_trowing_makibishi") = 0.0f;

							if(using_item && App->input->GetKey(SDL_SCANCODE_Q) == KEY_UP)
								((ComponentCollider*)makibishi1->GetComponent(ComponentType::C_COLLIDER))->body->SetLinearSpeed(0.0f, velocity_makibishi/2, velocity_makibishi/2);
							else
							{
								float y_joy_input = App->input->GetJoystickAxis(Player_car->GetBackPlayer(), JOY_AXIS::LEFT_STICK_Y);
								if (y_joy_input < 0)
								{
									float3 new_pos = game_object->transform->GetPosition();
									new_pos += game_object->transform->GetForward().Normalized() * Player_car->chasis_size.z * 2;
									((ComponentCollider*)makibishi1->GetComponent(ComponentType::C_COLLIDER))->body->SetTransform(game_object->transform->GetTransformMatrix().Transposed().ptr());
									((ComponentCollider*)makibishi1->GetComponent(ComponentType::C_COLLIDER))->body->SetPos(new_pos.x, new_pos.y, new_pos.z);
									makibishi1->GetComponent(ComponentType::C_COLLIDER)->SetActive(true);
								}
								else
								{
									float x_joy_input = App->input->GetJoystickAxis(Player_car->GetBackPlayer(), JOY_AXIS::LEFT_STICK_X);
									((ComponentCollider*)makibishi1->GetComponent(ComponentType::C_COLLIDER))->body->SetLinearSpeed(x_joy_input * velocity_makibishi / 2, velocity_makibishi / 2, y_joy_input * velocity_makibishi / 2);
								}
							}
						}
					}
				}
			}

			if (have_triple_makibishi)
			{
				if (throwing_makibishi)
				{
					if (!makibishi1->IsActive())
					{
						((ComponentScript*)makibishi1->GetComponent(ComponentType::C_SCRIPT))->public_floats.at("current_time_trowing_makibishi") = 0.0f;
					}
					if(makibishi_quantity <= 1)
					{
						if (!makibishi2->IsActive())
						{
							((ComponentScript*)makibishi2->GetComponent(ComponentType::C_SCRIPT))->public_floats.at("current_time_trowing_makibishi") = 0.0f;
						}
					}
					if (makibishi_quantity <= 0)
					{
						if (!makibishi3->IsActive())
						{
							throwing_makibishi = false;
							((ComponentScript*)makibishi3->GetComponent(ComponentType::C_SCRIPT))->public_floats.at("current_time_trowing_makibishi") = 0.0f;
							have_triple_makibishi = false;
						}
					}
				}
				if(makibishi_quantity > 0)
				{
					if (App->input->GetJoystickButton(Player_car->GetBackPlayer(), JOY_BUTTON::B) == KEY_UP || using_item && App->input->GetKey(SDL_SCANCODE_Q) == KEY_UP)
					{
						if (makibishi_quantity == 3)
						{
							makibishi_quantity--;
							if (makibishi1 != nullptr)
							{
								float3 new_pos = game_object->transform->GetPosition();
								new_pos += game_object->transform->GetForward().Normalized() * Player_car->chasis_size.z * 2;
								((ComponentCollider*)makibishi1->GetComponent(ComponentType::C_COLLIDER))->body->SetTransform(game_object->transform->GetTransformMatrix().Transposed().ptr());
								((ComponentCollider*)makibishi1->GetComponent(ComponentType::C_COLLIDER))->body->SetPos(new_pos.x, new_pos.y, new_pos.z);
								makibishi1->GetComponent(ComponentType::C_COLLIDER)->SetActive(true);
								throwing_makibishi = true;
								((ComponentScript*)makibishi1->GetComponent(ComponentType::C_SCRIPT))->public_floats.at("current_time_trowing_makibishi") = 0.0f;

								if (using_item && App->input->GetKey(SDL_SCANCODE_Q) == KEY_UP)
									((ComponentCollider*)makibishi1->GetComponent(ComponentType::C_COLLIDER))->body->SetLinearSpeed(0.0f, velocity_makibishi / 2, velocity_makibishi / 2);
								else
								{
									float y_joy_input = App->input->GetJoystickAxis(Player_car->GetBackPlayer(), JOY_AXIS::LEFT_STICK_Y);
									if (y_joy_input < 0)
									{
										float3 new_pos = game_object->transform->GetPosition();
										new_pos += game_object->transform->GetForward().Normalized() * Player_car->chasis_size.z * 2;
										((ComponentCollider*)makibishi1->GetComponent(ComponentType::C_COLLIDER))->body->SetTransform(game_object->transform->GetTransformMatrix().Transposed().ptr());
										((ComponentCollider*)makibishi1->GetComponent(ComponentType::C_COLLIDER))->body->SetPos(new_pos.x, new_pos.y, new_pos.z);
										makibishi1->GetComponent(ComponentType::C_COLLIDER)->SetActive(true);
									}
									else
									{
										float x_joy_input = App->input->GetJoystickAxis(Player_car->GetBackPlayer(), JOY_AXIS::LEFT_STICK_X);
										((ComponentCollider*)makibishi1->GetComponent(ComponentType::C_COLLIDER))->body->SetLinearSpeed(x_joy_input * velocity_makibishi / 2, velocity_makibishi / 2, y_joy_input * velocity_makibishi / 2);
									}
								}
							}
						}
						else if(makibishi_quantity == 2)
						{
							makibishi_quantity--;
							if (makibishi2 != nullptr)
							{
								float3 new_pos = game_object->transform->GetPosition();
								new_pos += game_object->transform->GetForward().Normalized() * Player_car->chasis_size.z * 2;
								((ComponentCollider*)makibishi2->GetComponent(ComponentType::C_COLLIDER))->body->SetTransform(game_object->transform->GetTransformMatrix().Transposed().ptr());
								((ComponentCollider*)makibishi2->GetComponent(ComponentType::C_COLLIDER))->body->SetPos(new_pos.x, new_pos.y, new_pos.z);
								makibishi2->GetComponent(ComponentType::C_COLLIDER)->SetActive(true);
								throwing_makibishi = true;
								((ComponentScript*)makibishi2->GetComponent(ComponentType::C_SCRIPT))->public_floats.at("current_time_trowing_makibishi") = 0.0f;

								if (using_item && App->input->GetKey(SDL_SCANCODE_Q) == KEY_UP)
									((ComponentCollider*)makibishi2->GetComponent(ComponentType::C_COLLIDER))->body->SetLinearSpeed(0.0f, velocity_makibishi / 2, velocity_makibishi / 2);
								else
								{
									float y_joy_input = App->input->GetJoystickAxis(Player_car->GetBackPlayer(), JOY_AXIS::LEFT_STICK_Y);
									if (y_joy_input < 0)
									{
										float3 new_pos = game_object->transform->GetPosition();
										new_pos += game_object->transform->GetForward().Normalized() * Player_car->chasis_size.z * 2;
										((ComponentCollider*)makibishi2->GetComponent(ComponentType::C_COLLIDER))->body->SetTransform(game_object->transform->GetTransformMatrix().Transposed().ptr());
										((ComponentCollider*)makibishi2->GetComponent(ComponentType::C_COLLIDER))->body->SetPos(new_pos.x, new_pos.y, new_pos.z);
										makibishi2->GetComponent(ComponentType::C_COLLIDER)->SetActive(true);
									}
									else
									{
										float x_joy_input = App->input->GetJoystickAxis(Player_car->GetBackPlayer(), JOY_AXIS::LEFT_STICK_X);
										((ComponentCollider*)makibishi2->GetComponent(ComponentType::C_COLLIDER))->body->SetLinearSpeed(x_joy_input * velocity_makibishi / 2, velocity_makibishi / 2, y_joy_input * velocity_makibishi / 2);
									}
								}
							}
						}
						else if (makibishi_quantity == 1)
						{
							makibishi_quantity--;
							if (makibishi3 != nullptr)
							{
								float3 new_pos = game_object->transform->GetPosition();
								new_pos += game_object->transform->GetForward().Normalized() * Player_car->chasis_size.z * 2;
								((ComponentCollider*)makibishi3->GetComponent(ComponentType::C_COLLIDER))->body->SetTransform(game_object->transform->GetTransformMatrix().Transposed().ptr());
								((ComponentCollider*)makibishi3->GetComponent(ComponentType::C_COLLIDER))->body->SetPos(new_pos.x, new_pos.y, new_pos.z);
								makibishi3->GetComponent(ComponentType::C_COLLIDER)->SetActive(true);
								throwing_makibishi = true;
								((ComponentScript*)makibishi3->GetComponent(ComponentType::C_SCRIPT))->public_floats.at("current_time_trowing_makibishi") = 0.0f;

								if (using_item && App->input->GetKey(SDL_SCANCODE_Q) == KEY_UP)
									((ComponentCollider*)makibishi3->GetComponent(ComponentType::C_COLLIDER))->body->SetLinearSpeed(0.0f, velocity_makibishi / 2, velocity_makibishi / 2);
								else
								{
									float y_joy_input = App->input->GetJoystickAxis(Player_car->GetBackPlayer(), JOY_AXIS::LEFT_STICK_Y);
									if (y_joy_input < 0)
									{
										float3 new_pos = game_object->transform->GetPosition();
										new_pos -= game_object->transform->GetForward().Normalized() * Player_car->chasis_size.z * 2;
										((ComponentCollider*)makibishi3->GetComponent(ComponentType::C_COLLIDER))->body->SetTransform(game_object->transform->GetTransformMatrix().Transposed().ptr());
										((ComponentCollider*)makibishi3->GetComponent(ComponentType::C_COLLIDER))->body->SetPos(new_pos.x, new_pos.y, new_pos.z);
										makibishi3->GetComponent(ComponentType::C_COLLIDER)->SetActive(true);
									}
									else
									{
										float x_joy_input = App->input->GetJoystickAxis(Player_car->GetBackPlayer(), JOY_AXIS::LEFT_STICK_X);
										((ComponentCollider*)makibishi3->GetComponent(ComponentType::C_COLLIDER))->body->SetLinearSpeed(x_joy_input * velocity_makibishi / 2, velocity_makibishi / 2, y_joy_input * velocity_makibishi / 2);
									}
								}
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
							float3 new_pos = game_object->transform->GetPosition();
							new_pos += game_object->transform->GetForward().Normalized() * Player_car->chasis_size.z * 2;
							new_pos += game_object->transform->GetGlobalMatrix().WorldY().Normalized() * Player_car->chasis_size.y * 2;
							((ComponentCollider*)firecracker->GetComponent(ComponentType::C_COLLIDER))->body->SetPos(new_pos.x, new_pos.y, new_pos.z);
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
							float3 new_pos = game_object->transform->GetPosition();
							new_pos += game_object->transform->GetForward().Normalized() * Player_car->chasis_size.z * 2;
							new_pos += game_object->transform->GetGlobalMatrix().WorldY().Normalized() * Player_car->chasis_size.y * 2;
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
				
				GameObject* check_go = (GameObject*)item_col->GetGameObject();

				if (item_col->IsActive())
				{
					if (item_col->GetGameObject()->name == "Hitodama")
					{
						//Do something
					}
					else if (item_col->GetGameObject()->name == "Firecracker")
					{
						Player_car->GetVehicle()->SetLinearSpeed(0.0f, 0.0f, 0.0f);
						item_col->GetGameObject()->SetActive(false);
						item_col->SetActive(false);
					}
					else if (item_col->GetGameObject()->name == "Makibishi")
					{
						Player_car->GetVehicle()->SetLinearSpeed(0.0f, 0.0f, 0.0f);
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