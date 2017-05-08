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

	bool have_item = false;
	bool have_makibishi = false;
	bool have_triple_makibishi = false;
	bool have_firecracker = false;
	bool using_item = false;
	bool throwing_firecracker = false;
	int makibishi_quantity = 0;
	float velocity_firecracker = 120.0f;
	float time_trowing_firecracker = 0.0f;
	float explosion_radius_firecracker = 5.0f;
	float velocity_makibishi = 25.0f;
	bool have_evil_spirit = false;
	bool evil_spirit_effect = false;
	float evil_spirit_vel_reduction = 0.4f;

	float spirit_duration = 0.0f;
	float spirit_max_duration = 10.0f;

	string item_box_name = "item_box";
	GameObject* firecracker = nullptr;
	GameObject* makibishi1 = nullptr;
	GameObject* makibishi2 = nullptr;
	GameObject* makibishi3 = nullptr;
	GameObject* other_car = nullptr;
	GameObject* scene_manager = nullptr;
	GameObject* makibishi_manager = nullptr;

	std::vector<GameObject*> makibishis;

	void Player_Car_LoseItem(GameObject* game_object)
	{
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

	void Player_Car_UpdateEvilSpirit(GameObject* game_object, ComponentCar* car)
	{
		if (App->input->GetJoystickButton(car->GetBackPlayer(), JOY_BUTTON::B) == KEY_UP || App->input->GetKey(SDL_SCANCODE_Q) == KEY_UP)
		{
			current_item = -1;

			if (car->place == 2)
			{
				((ComponentScript*)other_car->GetComponent(ComponentType::C_SCRIPT))->public_bools.at("evil_spirit_effect") = true;
				((ComponentScript*)other_car->GetComponent(ComponentType::C_SCRIPT))->public_floats.at("evil_spirit_current_duration") = 0.0f;
			}
		}
	}

	void Player_Car_UpdateMakibishi(GameObject* game_object, ComponentCar* car)
	{
		if (App->input->GetJoystickButton(car->GetBackPlayer(), JOY_BUTTON::B) == KEY_UP || App->input->GetKey(SDL_SCANCODE_Q) == KEY_UP)
		{
			GameObject* makibishi = (*makibishis.begin());

			//Activating everything
			makibishi->SetActive(true);
			makibishi->GetComponent(ComponentType::C_COLLIDER)->SetActive(true);
			ComponentCollider* makibishi_collider = (ComponentCollider*)makibishi->GetComponent(ComponentType::C_COLLIDER);
			((ComponentScript*)makibishi->GetComponent(ComponentType::C_SCRIPT))->public_floats.at("current_time_trowing_makibishi") = 0.0f;
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
					//new_pos -= game_object->transform->GetForward().Normalized() * (car->chasis_size.z + 1);					//TODO: add kart Z size
					//new_pos += game_object->transform->GetGlobalMatrix().WorldY().Normalized() * (car->chasis_size.y + 2);	//TODO: add kart Y size
					makibishi_collider->body->SetTransform(game_object->transform->GetTransformMatrix().Transposed().ptr());
					makibishi_collider->body->SetPos(new_pos.x, new_pos.y, new_pos.z);
				}
				//Throwing makibishi forward
				else
				{
					//Setting position
					float3 new_pos = game_object->transform->GetPosition();
					//new_pos += game_object->transform->GetForward().Normalized() * (car->kart->chasis_size.z + 2);			//TODO: add kart Z size
					//new_pos += game_object->transform->GetGlobalMatrix().WorldY().Normalized() * (car->chasis_size.y + 2);	//TODO: add kart Y size
					makibishi_collider->body->SetTransform(game_object->transform->GetTransformMatrix().Transposed().ptr());
					makibishi_collider->body->SetPos(new_pos.x, new_pos.y, new_pos.z);

					float x_joy_input = App->input->GetJoystickAxis(car->GetBackPlayer(), JOY_AXIS::LEFT_STICK_X);
					float3 new_vel = ((game_object->transform->GetForward().Normalized() * (velocity_makibishi / 2)) + (game_object->GetGlobalMatrix().WorldY().Normalized() * y_joy_input * (velocity_makibishi / 2)));
					new_vel += (game_object->transform->GetGlobalMatrix().WorldX().Normalized() * x_joy_input * (velocity_makibishi / 2));
					makibishi_collider->body->SetLinearSpeed(new_vel.x, new_vel.y, new_vel.z);
				}
			}
			makibishis.erase(makibishis.begin());
		}
	}

	void Player_Car_UpdateFirecracker(GameObject* game_object, ComponentCar* car)
	{
		if ((App->input->GetJoystickButton(car->GetBackPlayer(), JOY_BUTTON::B) == KEY_DOWN || App->input->GetKey(SDL_SCANCODE_Q) == KEY_DOWN))
		{
			car->UseItem();
			using_item = true;
		}
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
			if (/*TODO: check if turbo current time is over max time*/tmp)
			{
				if (other_car != nullptr)
				{
					if (firecracker && firecracker->transform->GetPosition().Distance(other_car->transform->GetPosition()) <= explosion_radius_firecracker)
						//((ComponentCar*)other_car->GetComponent(ComponentType::C_CAR))->GetVehicle()->SetLinearSpeed(0.0f, 0.0f, 0.0f);				//TODO: stun enemy car
						tmp = 0;
				}
				Player_Car_LoseItem(game_object);
				current_item = -1;
				if (firecracker)
				{
					firecracker->SetActive(false);
					firecracker->GetComponent(ComponentType::C_COLLIDER)->SetActive(false);
				}
				//car->GetVehicle()->SetLinearSpeed(0.0f, 0.0f, 0.0f);			//TODO: "stun" ally car
			}
			else if (App->input->GetJoystickButton(car->GetBackPlayer(), JOY_BUTTON::B) == KEY_UP || App->input->GetKey(SDL_SCANCODE_Q) == KEY_UP)
			{
				Player_Car_LoseItem(game_object);
				using_item = false;
				have_firecracker = false;
				current_item = -1;
				if (firecracker != nullptr)
				{
					float3 new_pos = game_object->transform->GetPosition();
					//new_pos += game_object->transform->GetForward().Normalized() * (car->chasis_size.z + 2);			//TODO: add car Z size
					((ComponentCollider*)firecracker->GetComponent(ComponentType::C_COLLIDER))->body->SetPos(new_pos.x, new_pos.y, new_pos.z);
					firecracker->GetComponent(ComponentType::C_COLLIDER)->SetActive(true);
					throwing_firecracker = true;
					//time_trowing_firecracker = car->GetAppliedTurbo()->timer;				//TODO: get time using turbo
				}
			}
		}
	}

	void Player_Car_UpdateLaunchedFirecracker(GameObject* game_object, ComponentCar* car)
	{
		ComponentCollider* firecracker_col = (ComponentCollider*)firecracker->GetComponent(ComponentType::C_COLLIDER);
		float3 new_pos = firecracker_col->body->GetPosition();
		new_pos += firecracker_col->body->GetTransform().WorldZ().Normalized() * velocity_firecracker * time->DeltaTime();
		firecracker_col->body->SetPos(new_pos.x, new_pos.y, new_pos.z);
		time_trowing_firecracker += time->DeltaTime();

		if (/*time_trowing_firecracker >= car->rocket_turbo.time*/true)				//TODO: check if firecracker launch ends
		{
			throwing_firecracker = false;
			time_trowing_firecracker = 0.0f;
			firecracker->SetActive(false);
			firecracker->GetComponent(ComponentType::C_COLLIDER)->SetActive(false);
		}
	}

	void Player_Car_GetPublics(map<const char*, string>* public_chars, map<const char*, int>* public_ints, map<const char*, float>* public_float, map<const char*, bool>* public_bools, map<const char*, GameObject*>* public_gos)
	{
		//New Player_Car
		public_ints->insert(pair<const char*, int>("current_item", current_item));


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
		public_ints->insert(pair<const char*, int>("makibishi_quantity", makibishi_quantity));
		public_bools->insert(pair<const char*, bool>("have_evil_spirit", have_evil_spirit));
		public_bools->insert(pair<const char*, bool>("evil_spirit_effect", evil_spirit_effect));
		public_float->insert(pair<const char*, float>("evil_spirit_vel_reduction", evil_spirit_vel_reduction));
		public_float->insert(pair<const char*, float>("spirit_duration", spirit_duration));
		public_float->insert(pair<const char*, float>("spirit_max_duration", spirit_max_duration));
		public_chars->insert(pair<const char*, string>("item_box_name", item_box_name));

		public_gos->insert(pair<const char*, GameObject*>("firecracker", nullptr));
		public_gos->insert(pair<const char*, GameObject*>("makibishi1", nullptr));
		public_gos->insert(pair<const char*, GameObject*>("makibishi2", nullptr));
		public_gos->insert(pair<const char*, GameObject*>("makibishi3", nullptr));
		public_gos->insert(pair<const char*, GameObject*>("other_car", nullptr));
		public_gos->insert(pair<const char*, GameObject*>("scene_manager", nullptr));
		public_gos->insert(pair<const char*, GameObject*>("makibishi_manager", nullptr));

	}

	void Player_Car_UpdatePublics(GameObject* game_object)
	{
		ComponentScript* script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);

		//New Player_Car
		current_item = script->public_ints["current_item"];

		have_item = script->public_bools.at("have_item");
		have_makibishi = script->public_bools.at("have_makibishi");
		have_triple_makibishi = script->public_bools.at("have_triple_makibishi");
		have_firecracker = script->public_bools.at("have_firecracker");
		using_item = script->public_bools.at("using_item");
		throwing_firecracker = script->public_bools.at("throwing_firecracker");
		velocity_firecracker = script->public_floats.at("velocity_firecracker");
		time_trowing_firecracker = script->public_floats.at("time_trowing_firecracker");
		explosion_radius_firecracker = script->public_floats.at("explosion_radius_firecracker");
		velocity_makibishi = script->public_floats.at("velocity_makibishi");
		makibishi_quantity = script->public_ints.at("makibishi_quantity");
		have_evil_spirit = script->public_bools.at("have_evil_spirit");
		evil_spirit_effect = script->public_bools.at("evil_spirit_effect");
		evil_spirit_vel_reduction = script->public_floats.at("evil_spirit_vel_reduction");
		spirit_duration = script->public_floats.at("spirit_duration");
		spirit_max_duration = script->public_floats.at("spirit_max_duration");
		item_box_name = script->public_chars.at("item_box_name");

		firecracker = script->public_gos.at("firecracker");
		makibishi1 = script->public_gos.at("makibishi1");
		makibishi2 = script->public_gos.at("makibishi2");
		makibishi3 = script->public_gos.at("makibishi3");
		other_car = script->public_gos.at("other_car");
		scene_manager = script->public_gos.at("scene_manager");
		makibishi_manager = script->public_gos.at("makibishi_manager");

	}

	void Player_Car_ActualizePublics(GameObject* game_object)
	{
		ComponentScript* script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);

		script->public_bools.at("have_item") = have_item;
		script->public_bools.at("have_makibishi") = have_makibishi;
		script->public_bools.at("have_triple_makibishi") = have_triple_makibishi;
		script->public_bools.at("have_firecracker") = have_firecracker;
		script->public_bools.at("using_item") = using_item;
		script->public_bools.at("throwing_firecracker") = throwing_firecracker;
		script->public_floats.at("velocity_firecracker") = velocity_firecracker;
		script->public_floats.at("time_trowing_firecracker") = time_trowing_firecracker;
		script->public_floats.at("explosion_radius_firecracker") = explosion_radius_firecracker;
		script->public_floats.at("velocity_makibishi") = velocity_makibishi;
		script->public_ints.at("makibishi_quantity") = makibishi_quantity;
		script->public_bools.at("have_evil_spirit") = have_evil_spirit;
		script->public_bools.at("evil_spirit_effect") = evil_spirit_effect;
		script->public_floats.at("evil_spirit_vel_reduction") = evil_spirit_vel_reduction;
		script->public_floats.at("spirit_duration") = spirit_duration;
		script->public_floats.at("spirit_max_duration") = spirit_max_duration;
		script->public_chars.at("item_box_name") = item_box_name;

		script->public_gos.at("firecracker") = firecracker;
		script->public_gos.at("makibishi1") = makibishi1;
		script->public_gos.at("makibishi2") = makibishi2;
		script->public_gos.at("makibishi3") = makibishi3;
		script->public_gos.at("other_car") = other_car;
		script->public_gos.at("scene_manager") = scene_manager;
		script->public_gos.at("makibishi_manager") = makibishi_manager;
	}

	void Player_Car_Start(GameObject* game_object)
	{

	}

	void Player_Car_LoseItem(GameObject* game_object);
	void Player_Car_OnPickItem(GameObject* game_object);
	void Player_Car_ChooseItem(GameObject* game_object);

	void Player_Car_Update(GameObject* game_object)
	{
		ComponentCar* Player_car = (ComponentCar*)game_object->GetComponent(ComponentType::C_CAR);
		ComponentCar* car = (ComponentCar*)game_object->GetComponent(ComponentType::C_CAR);
		if (Player_car == nullptr)
			return;

		if (current_item != -1)
		{
			switch (current_item)
			{
				case(EVIL_SPIRIT):
				{
					Player_Car_UpdateEvilSpirit(game_object, car);
					break;
				}
				case(MAKIBISHI):
				{
					Player_Car_UpdateMakibishi(game_object, car);
				}
				case(FIRECRACKER):
				{
					Player_Car_UpdateFirecracker(game_object, car);
				}
			}
		}

		if (evil_spirit_effect)
		{
			Player_Car_UpdateSpiritEffect(game_object, car);
		}
		if (using_item)
		{
			Player_Car_UpdateFirecrackerEffect(game_object, car);
		}
		if (throwing_firecracker)
		{
			Player_Car_UpdateLaunchedFirecracker(game_object, car);
		}
	}

	void Player_Car_OnCollision(GameObject* game_object, PhysBody3D* col)
	{
		ComponentCar* Player_car = (ComponentCar*)game_object->GetComponent(ComponentType::C_CAR);

		if (col->IsCar())
		{
			float3 otherCarPos = ((ComponentTransform*)(col->GetCar()->GetGameObject()->GetComponent(C_TRANSFORM)))->GetPosition();
			float3 myPos = ((ComponentTransform*)(Player_car->GetGameObject()->GetComponent(C_TRANSFORM)))->GetPosition();
			float3 norm = myPos - otherCarPos;
			Player_car->WallHit(norm.Normalized());
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
						//Player_car->GetVehicle()->SetLinearSpeed(0.0f, 0.0f, 0.0f);
						item->SetActive(false);
						Player_car->RemoveHitodama();

						if (other_car != nullptr)
						{
							//if (firecracker && firecracker->transform->GetPosition().Distance(other_car->transform->GetPosition()) <= explosion_radius_firecracker)
							//	((ComponentCar*)other_car->GetComponent(ComponentType::C_CAR))->GetVehicle()->SetLinearSpeed(0.0f, 0.0f, 0.0f);
						}
					}
					else if (item->name == "Makibishi")
					{
						//Player_car->GetVehicle()->SetLinearSpeed(0.0f, 0.0f, 0.0f);
						item->SetActive(false);
						Player_car->RemoveHitodama();
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
				have_firecracker = true;
			}
			else if (result <= 40)
			{
				current_item = MAKIBISHI;
				makibishis.resize(3, nullptr);
				have_triple_makibishi = true;
			}
			else
			{
				current_item = MAKIBISHI;
				makibishis.resize(1, nullptr);
				have_makibishi = true;
			}
		}
		else if (car->place == 2)
		{
			//2nd
			if (result <= 15)
			{
				current_item = EVIL_SPIRIT;
				have_evil_spirit = true;
			}
			else if (result <= 35)
			{
				current_item = MAKIBISHI;
				makibishis.resize(3, nullptr);
				have_triple_makibishi = true;
			}
			else if (result <= 65)
			{
				current_item = MAKIBISHI;
				makibishis.resize(1, nullptr);
				have_makibishi = true;
			}
			else
			{
				current_item = FIRECRACKER;
				have_firecracker = true;
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
						((ComponentScript*)makibishis.back()->GetComponent(ComponentType::C_SCRIPT))->public_floats.at("current_time_trowing_makibishi") = 0.0f;
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

		//Updating UI
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


}