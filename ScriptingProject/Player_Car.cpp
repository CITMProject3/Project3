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
#include "../Random.h"
#include "../Time.h"

enum Item_Type
{
	MAKIBISHI = 0,
	FIRECRACKER,
	EVIL_SPIRIT,
};

namespace Player_Car
{
	//New Player_Car
	int current_item = -1;

	float launched_firecracker_lifetime = -1.0f;
	bool using_firecracker = false;
	bool throwing_firecracker = false;
	float velocity_firecracker = 120.0f;
	float explosion_radius_firecracker = 5.0f;

	float velocity_makibishi = 25.0f;
	bool evil_spirit_effect = false;
	float evil_spirit_vel_reduction = 0.4f;

	float spirit_duration = 0.0f;
	float spirit_max_duration = 10.0f;

	string item_box_name = "item_box";
	GameObject* firecracker = nullptr;
	GameObject* other_car = nullptr;
	GameObject* scene_manager = nullptr;
	GameObject* makibishi_manager = nullptr;

	std::vector<GameObject*> makibishis;

	void Player_Car_GetPublics(map<const char*, string>* public_chars, map<const char*, int>* public_ints, map<const char*, float>* public_float, map<const char*, bool>* public_bools, map<const char*, GameObject*>* public_gos)
	{
		//New Player_Car
		//(*public_ints)["current_item"] = current_item;
		public_ints->insert(pair<const char*, int>("current_item", current_item));
		public_float->insert(pair<const char*, float>("launched_firecracker_lifetime", launched_firecracker_lifetime));


		public_bools->insert(pair<const char*, bool>("using_firecracker", using_firecracker));
		public_float->insert(pair<const char*, float>("velocity_firecracker", velocity_firecracker));
		public_float->insert(pair<const char*, float>("explosion_radius_firecracker", explosion_radius_firecracker));
		public_float->insert(pair<const char*, float>("velocity_makibishi", velocity_makibishi));
		public_bools->insert(pair<const char*, bool>("evil_spirit_effect", evil_spirit_effect));
		public_float->insert(pair<const char*, float>("evil_spirit_vel_reduction", evil_spirit_vel_reduction));
		public_float->insert(pair<const char*, float>("spirit_duration", spirit_duration));
		public_float->insert(pair<const char*, float>("spirit_max_duration", spirit_max_duration));
		public_chars->insert(pair<const char*, string>("item_box_name", item_box_name));

		public_gos->insert(pair<const char*, GameObject*>("firecracker", nullptr));
		public_gos->insert(pair<const char*, GameObject*>("other_car", nullptr));
		public_gos->insert(pair<const char*, GameObject*>("scene_manager", nullptr));
		public_gos->insert(pair<const char*, GameObject*>("makibishi_manager", nullptr));

	}

	void Player_Car_UpdatePublics(GameObject* game_object)
	{
		ComponentScript* script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);

		//New Player_Car
		current_item = script->public_ints["current_item"];
		launched_firecracker_lifetime = script->public_floats["launched_firecracker_lifetime"];

		using_firecracker = script->public_bools.at("using_firecracker");
		velocity_firecracker = script->public_floats.at("velocity_firecracker");
		explosion_radius_firecracker = script->public_floats.at("explosion_radius_firecracker");
		velocity_makibishi = script->public_floats.at("velocity_makibishi");
		evil_spirit_effect = script->public_bools.at("evil_spirit_effect");
		evil_spirit_vel_reduction = script->public_floats.at("evil_spirit_vel_reduction");
		spirit_duration = script->public_floats.at("spirit_duration");
		spirit_max_duration = script->public_floats.at("spirit_max_duration");
		item_box_name = script->public_chars.at("item_box_name");

		firecracker = script->public_gos.at("firecracker");
		other_car = script->public_gos.at("other_car");
		scene_manager = script->public_gos.at("scene_manager");
		makibishi_manager = script->public_gos.at("makibishi_manager");

	}

	void Player_Car_ActualizePublics(GameObject* game_object)
	{
		ComponentScript* script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);

		script->public_ints["current_item"] = current_item;
		script->public_floats["launched_firecracker_lifetime"] = launched_firecracker_lifetime;

		script->public_floats.at("velocity_firecracker") = velocity_firecracker;
		script->public_floats.at("explosion_radius_firecracker") = explosion_radius_firecracker;
		script->public_floats.at("velocity_makibishi") = velocity_makibishi;
		script->public_bools.at("evil_spirit_effect") = evil_spirit_effect;
		script->public_floats.at("evil_spirit_vel_reduction") = evil_spirit_vel_reduction;
		script->public_floats.at("spirit_duration") = spirit_duration;
		script->public_floats.at("spirit_max_duration") = spirit_max_duration;
		script->public_chars.at("item_box_name") = item_box_name;

		script->public_gos.at("firecracker") = firecracker;
		script->public_gos.at("other_car") = other_car;
		script->public_gos.at("scene_manager") = scene_manager;
		script->public_gos.at("makibishi_manager") = makibishi_manager;
	}

	void Player_Car_Start(GameObject* game_object)
	{

	}

#pragma region Forward Declarations
	void Player_Car_OnPickItem(GameObject* game_object);
	void Player_Car_ChooseItem(GameObject* game_object);
	void Player_Car_UseEvilSpirit(GameObject* game_object, ComponentCar* car);
	void Player_Car_UseMakibishi(GameObject* game_object, ComponentCar* car);
	void Player_Car_UseFirecracker(GameObject* game_object, ComponentCar* car);
	void Player_Car_UpdateSpiritEffect(GameObject* game_object, ComponentCar* car);
	void Player_Car_UpdateFirecrackerEffect(GameObject* game_object, ComponentCar* car);
	void Player_Car_UpdateLaunchedFirecracker(GameObject* game_object, ComponentCar* car);
#pragma endregion

	void Player_Car_Update(GameObject* game_object)
	{
		ComponentCar* Player_car = (ComponentCar*)game_object->GetComponent(ComponentType::C_CAR);
		ComponentCar* car = (ComponentCar*)game_object->GetComponent(ComponentType::C_CAR);
		if (Player_car == nullptr)
			return;

		if (current_item != -1)
		{
			if (App->input->GetJoystickButton(car->GetBackPlayer(), JOY_BUTTON::B) == KEY_DOWN || App->input->GetKey(SDL_SCANCODE_Q) == KEY_DOWN)
			{
				switch (current_item)
				{
					case(EVIL_SPIRIT):
					{
						Player_Car_UseEvilSpirit(game_object, car);
						break;
					}
					case(MAKIBISHI):
					{
						Player_Car_UseMakibishi(game_object, car);
					}
					case(FIRECRACKER):
					{
						Player_Car_UseFirecracker(game_object, car);
					}
				}
			}
		}

		if (evil_spirit_effect)
		{
			Player_Car_UpdateSpiritEffect(game_object, car);
		}
		if (using_firecracker)
		{
			Player_Car_UpdateFirecrackerEffect(game_object, car);
		}
		if (launched_firecracker_lifetime != -1.0f)
		{
			Player_Car_UpdateLaunchedFirecracker(game_object, car);
		}
	}

	void Player_Car_OnCollision(GameObject* game_object, PhysBody3D* col)
	{
		ComponentCar* car = (ComponentCar*)game_object->GetComponent(ComponentType::C_CAR);

		if (col->IsCar())
		{
			float3 otherCarPos = ((ComponentTransform*)(col->GetCar()->GetGameObject()->GetComponent(C_TRANSFORM)))->GetPosition();
			float3 myPos = ((ComponentTransform*)(car->GetGameObject()->GetComponent(C_TRANSFORM)))->GetPosition();
			float3 norm = myPos - otherCarPos;
			car->WallHit(norm.Normalized());
		}

		if (col->IsTrigger())
		{
			if (col->IsCar() == false)
			{
				GameObject* item = col->GetCollider()->GetGameObject();
				if (item->IsActive())
				{
					if (item->name == "Hitodama")
					{
						//Player_car->AddHitodama();
					}
					else if (item->name == "Firecracker")
					{
						car->OnGetHit();
						item->SetActive(false);
						car->RemoveHitodama();

						if (other_car != nullptr)
						{
							if (firecracker && firecracker->transform->GetPosition().Distance(other_car->transform->GetPosition()) <= explosion_radius_firecracker)
								((ComponentCar*)other_car->GetComponent(C_SCRIPT))->OnGetHit();
						}
					}
					else if (item->name == "Makibishi")
					{
						car->OnGetHit();
						item->SetActive(false);
						car->RemoveHitodama();
					}
					else if (current_item == -1 && item->name == item_box_name.c_str())
					{
						Player_Car_ChooseItem(game_object);
						Player_Car_OnPickItem(game_object);
					}
				}
			}
		}
	}

	void Player_Car_ChooseItem(GameObject* game_object)
	{
		ComponentCar* car = (ComponentCar*)game_object->GetComponent(C_CAR);

		//By position
		int result = App->rnd->RandomInt(1, 100);

		if (car->place == 1)
		{
			if (result <= 5)
			{
				current_item = FIRECRACKER;
			}
			else if (result <= 40)
			{
				current_item = MAKIBISHI;
				makibishis.resize(3, nullptr);
			}
			else
			{
				current_item = MAKIBISHI;
				makibishis.resize(1, nullptr);
			}
		}
		else if (car->place == 2)
		{
			//2nd
			if (result <= 15)
			{
				current_item = EVIL_SPIRIT;
			}
			else if (result <= 35)
			{
				current_item = MAKIBISHI;
				makibishis.resize(3, nullptr);
			}
			else if (result <= 65)
			{
				current_item = MAKIBISHI;
				makibishis.resize(1, nullptr);
			}
			else
			{
				current_item = FIRECRACKER;
			}
		}
	}

	void Player_Car_OnPickItem(GameObject* game_object)
	{
		switch (current_item)
		{
			case(MAKIBISHI):
			{
				string path = ((ComponentScript*)makibishi_manager->GetComponent(C_SCRIPT))->GetPath();
				path.append("_GetMakibishi");
				if (f_GetMakibishi get_makibishi = (f_GetMakibishi)GetProcAddress(App->scripting->scripts_lib->lib, path.c_str()))
				{
					//Makibishis are assigned according to the vector size
					for (uint i = 0; i < makibishis.size(); i++)
					{
						makibishis[i] = get_makibishi();
						((ComponentScript*)makibishis.back()->GetComponent(ComponentType::C_SCRIPT))->public_floats.at("current_time_throwing_makibishi") = 0.0f;
					}
				}
				break;
			}
			case(FIRECRACKER):
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
			}
		}
	}


	void Player_Car_UseEvilSpirit(GameObject* game_object, ComponentCar* car)
	{
		current_item = -1;

		if (car->place == 2)
		{
			((ComponentScript*)other_car->GetComponent(ComponentType::C_SCRIPT))->public_bools.at("evil_spirit_effect") = true;
			((ComponentScript*)other_car->GetComponent(ComponentType::C_SCRIPT))->public_floats.at("evil_spirit_current_duration") = 0.0f;
		}
		else
		{
			//TODO: give some kind of feedback when using evil_spirit being in first place?
		}
	}

	void Player_Car_UseMakibishi(GameObject* game_object, ComponentCar* car)
	{
		GameObject* makibishi = (*makibishis.begin());

		//Activating everything
		makibishi->SetActive(true);
		makibishi->GetComponent(ComponentType::C_COLLIDER)->SetActive(true);
		ComponentCollider* makibishi_collider = (ComponentCollider*)makibishi->GetComponent(ComponentType::C_COLLIDER);
		((ComponentScript*)makibishi->GetComponent(ComponentType::C_SCRIPT))->public_floats.at("current_time_throwing_makibishi") = 0.0f;
		makibishi_collider->body->SetActivationState(1);

		if (App->input->GetKey(SDL_SCANCODE_Q) == KEY_UP)
		{
			float3 new_vel = ((game_object->transform->GetForward().Normalized() * (velocity_makibishi / 2)) + (game_object->GetGlobalMatrix().WorldY().Normalized() * (velocity_makibishi / 2)));
			makibishi_collider->body->SetLinearSpeed(new_vel.x, new_vel.y, new_vel.z);
		}
		else
		{
			float y_joy_input = App->input->GetJoystickAxis(car->GetBackPlayer(), JOY_AXIS::LEFT_STICK_Y);
			//Leaving makibishi behind
			if (y_joy_input < 0)
			{
				float3 new_pos = game_object->transform->GetPosition();
				new_pos -= car->kartZ  * car->collShape.size.z;
				new_pos += car->kartY * (car->collShape.size.y + 2);
				makibishi_collider->body->SetTransform(game_object->transform->GetTransformMatrix().Transposed().ptr());
				makibishi_collider->body->SetPos(new_pos.x, new_pos.y, new_pos.z);
				float3 new_vel = (game_object->transform->GetForward().Normalized() * -1);
				((ComponentCollider*)makibishi->GetComponent(ComponentType::C_COLLIDER))->body->SetLinearSpeed(new_vel.x, new_vel.y, new_vel.z);
			}
			//Throwing makibishi forward
			else
			{
				//Setting position
				float3 new_pos = game_object->transform->GetPosition();
				new_pos += car->kartZ * (car->collShape.size.z + 2);
				new_pos += car->kartY * (car->collShape.size.y + 2);
				makibishi_collider->body->SetTransform(game_object->transform->GetTransformMatrix().Transposed().ptr());
				makibishi_collider->body->SetPos(new_pos.x, new_pos.y, new_pos.z);

				float x_joy_input = App->input->GetJoystickAxis(car->GetBackPlayer(), JOY_AXIS::LEFT_STICK_X);
				float3 new_vel = ((game_object->transform->GetForward().Normalized() * (velocity_makibishi / 2)) + (game_object->GetGlobalMatrix().WorldY().Normalized() * y_joy_input * (velocity_makibishi / 2)));
				new_vel += (game_object->transform->GetGlobalMatrix().WorldX().Normalized() * x_joy_input * (velocity_makibishi / 2));
				makibishi_collider->body->SetLinearSpeed(new_vel.x, new_vel.y, new_vel.z);
			}
		}
		makibishis.erase(makibishis.begin());
		if (makibishis.size() == 0)
			current_item = -1;
	}

	void Player_Car_UseFirecracker(GameObject* game_object, ComponentCar* car)
	{
		car->UseItem();
		using_firecracker = true;
	}

	void Player_Car_UpdateSpiritEffect(GameObject* game_object, ComponentCar* car)
	{
		if (spirit_duration == 0.0f && car->inverted_controls == false)
		{
			//car->SetMaxVelocity(car->GetMaxVelocity() * (1 - evil_spirit_vel_reduction));
			car->inverted_controls = true;
		}
		else
		{
			spirit_duration += time->DeltaTime();
			if (spirit_duration >= spirit_max_duration)
			{
				//car->SetMaxVelocity(car->GetMaxVelocity() / (1 - evil_spirit_vel_reduction));
				car->inverted_controls = false;
				evil_spirit_effect = false;
			}
		}
	}

	void Player_Car_UpdateFirecrackerEffect(GameObject* game_object, ComponentCar* car)
	{
		int tmp = 0;
		//When the time gets out
		if (/*TODO: check if turbo or firecracker is in use*/tmp)
		{
			if (car->GetAppliedTurbo().IsActive() == false)
			{
				if (other_car != nullptr)
				{
					if (firecracker && firecracker->transform->GetPosition().Distance(other_car->transform->GetPosition()) <= explosion_radius_firecracker)
						((ComponentCar*)other_car->GetComponent(C_CAR))->OnGetHit();
				}
				current_item = -1;
				if (firecracker)
				{
					firecracker->SetActive(false);
					firecracker->GetComponent(ComponentType::C_COLLIDER)->SetActive(false);
				}
				car->OnGetHit();
			}
			else if (App->input->GetJoystickButton(car->GetBackPlayer(), JOY_BUTTON::B) == KEY_UP || App->input->GetKey(SDL_SCANCODE_Q) == KEY_UP)
			{
				using_firecracker = false;
				current_item = -1;
				if (firecracker != nullptr)
				{
					float3 new_pos = game_object->transform->GetPosition();
					new_pos += car->kartZ * (car->collShape.size.z + 2);
					((ComponentCollider*)firecracker->GetComponent(ComponentType::C_COLLIDER))->body->SetPos(new_pos.x, new_pos.y, new_pos.z);
					firecracker->GetComponent(ComponentType::C_COLLIDER)->SetActive(true);
					launched_firecracker_lifetime = 10.0f - car->GetAppliedTurbo().TimePassed();
				}
			}
		}
	}

	void Player_Car_UpdateLaunchedFirecracker(GameObject* game_object, ComponentCar* car)
	{
		if (firecracker != nullptr)
		{
			ComponentCollider* firecracker_col = (ComponentCollider*)firecracker->GetComponent(ComponentType::C_COLLIDER);
			float3 new_pos = firecracker_col->body->GetPosition();
			new_pos += firecracker_col->body->GetTransform().WorldZ().Normalized() * velocity_firecracker * time->DeltaTime();
			firecracker_col->body->SetPos(new_pos.x, new_pos.y, new_pos.z);
			launched_firecracker_lifetime -= time->DeltaTime();
		
			if (launched_firecracker_lifetime <= 0.0f)
			{
				firecracker->SetActive(false);
				firecracker->GetComponent(ComponentType::C_COLLIDER)->SetActive(false);
				launched_firecracker_lifetime = -1.0f;
			}
		}
		else
		{
			launched_firecracker_lifetime = -1.0f;
		}

	}
}