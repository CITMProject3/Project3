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
#include "../ComponentAudioSource.h"
#include "../SDL/include/SDL_scancode.h"
#include "../PhysBody3D.h"
#include "../ComponentCollider.h"
#include "../ComponentCar.h"
#include "../Globals.h"
#include "../Random.h"
#include "../Time.h"

//To call functions from another script
typedef void(*PSHit_CarCollision)(const math::float3& point);
typedef void(*PSHit_WallCollision)(int car_id, const math::float3& point);

enum Item_Type
{
	MAKIBISHI = 0,
	FIRECRACKER,
	EVIL_SPIRIT,
};

namespace Player_Car
{
	//Sorry everyone :(
	int car_id = 0;

	//New Player_Car
	int current_item = -1;

	float launched_firecracker_lifetime = -1.0f;
	bool using_firecracker = false;
	bool throwing_firecracker = false;
	float velocity_firecracker = 120.0f;
	float explosion_radius_firecracker = 5.0f;

	float velocity_makibishi = 25.0f;
	float makibishi_collision_velocity_reduction = 0.6f;
	bool evil_spirit_effect = false;
	float evil_spirit_vel_reduction = 0.4f;
	GameObject* evil_spirit_object[2];
	float3 evil_spirit_start_pos;
	bool triggers_pressed[2];
	int last_trigger = 0;
	float target_defense_value = 0.0f;
	float current_defense_value = 0.0f;

	float spirit_duration = 0.0f;
	float spirit_max_duration = 10.0f;

	//Turbo
	float turbo_max_acc_time = 3.0f; //Time to reach max acceleration with the turbo
	float turbo_acc_bonus_over_time = 0.1f;
	float turbo_speed_bonus = 0.5f;
	float turbo_dec_time = 0.5f;
	float turbo_lifetime = 5.0f;
	float turbo_timer = 0.0f;

	string item_box_name = "item_box";
	GameObject* firecracker = nullptr;
	GameObject* other_car = nullptr;
	GameObject* scene_manager = nullptr;
	GameObject* makibishi_manager = nullptr;
	GameObject* ps_hit_manager = nullptr;
	GameObject* drift_go_left = nullptr;
	GameObject* drift_go_right = nullptr;

	//Particles
	PSHit_CarCollision ps_hit_func = nullptr;
	PSHit_WallCollision ps_hit_wall_func = nullptr;


	std::vector<GameObject*> makibishis_1;
	std::vector<GameObject*> makibishis_2;
	int item_size = 1;

	void Player_Car_GetPublics(map<const char*, string>* public_chars, map<const char*, int>* public_ints, map<const char*, float>* public_float, map<const char*, bool>* public_bools, map<const char*, GameObject*>* public_gos)
	{
		//New Player_Car
		//(*public_ints)["current_item"] = current_item;
		public_ints->insert(pair<const char*, int>("current_item", current_item));
		public_float->insert(pair<const char*, float>("launched_firecracker_lifetime", launched_firecracker_lifetime));
		(*public_ints)["car_id"] = car_id;
		(*public_ints)["item_size"] = item_size;

		public_bools->insert(pair<const char*, bool>("using_firecracker", using_firecracker));
		public_float->insert(pair<const char*, float>("velocity_firecracker", velocity_firecracker));
		public_float->insert(pair<const char*, float>("explosion_radius_firecracker", explosion_radius_firecracker));
		public_float->insert(pair<const char*, float>("velocity_makibishi", velocity_makibishi));
		public_float->insert(pair<const char*, float>("makibishi_collision_velocity_reduction", makibishi_collision_velocity_reduction));
		public_bools->insert(pair<const char*, bool>("evil_spirit_effect", evil_spirit_effect));
		public_float->insert(pair<const char*, float>("evil_spirit_vel_reduction", evil_spirit_vel_reduction));
		public_float->insert(pair<const char*, float>("spirit_duration", spirit_duration));
		public_float->insert(pair<const char*, float>("spirit_max_duration", spirit_max_duration));
		public_float->insert(pair<const char*, float>("target_defense_value", target_defense_value));
		public_float->insert(pair<const char*, float>("current_defense_value", current_defense_value));
		public_chars->insert(pair<const char*, string>("item_box_name", item_box_name));

		public_float->insert(pair<const char*, float>("turbo_max_acc_time", turbo_max_acc_time));
		public_float->insert(pair<const char*, float>("turbo_acc_bonus_over_time", turbo_acc_bonus_over_time));
		public_float->insert(pair<const char*, float>("turbo_speed_bonus", turbo_speed_bonus));	
		public_float->insert(pair<const char*, float>("turbo_dec_time", turbo_dec_time));

		public_gos->insert(pair<const char*, GameObject*>("firecracker", nullptr));
		public_gos->insert(pair<const char*, GameObject*>("other_car", nullptr));
		public_gos->insert(pair<const char*, GameObject*>("scene_manager", nullptr));
		public_gos->insert(pair<const char*, GameObject*>("makibishi_manager", nullptr));
		public_gos->insert(pair<const char*, GameObject*>("ps_hit_manager", nullptr));
		public_gos->insert(pair<const char*, GameObject*>("ps_drift_left", nullptr));
		public_gos->insert(pair<const char*, GameObject*>("ps_drift_right", nullptr));

	}

	void Player_Car_UpdatePublics(GameObject* game_object)
	{
		ComponentScript* script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);

		//New Player_Car
		current_item = script->public_ints["current_item"];
		launched_firecracker_lifetime = script->public_floats["launched_firecracker_lifetime"];
		car_id = script->public_ints["car_id"];
		item_size = script->public_ints["item_size"];

		using_firecracker = script->public_bools.at("using_firecracker");
		velocity_firecracker = script->public_floats.at("velocity_firecracker");
		explosion_radius_firecracker = script->public_floats.at("explosion_radius_firecracker");
		velocity_makibishi = script->public_floats.at("velocity_makibishi");
		makibishi_collision_velocity_reduction = script->public_floats.at("makibishi_collision_velocity_reduction");
		evil_spirit_effect = script->public_bools.at("evil_spirit_effect");
		evil_spirit_vel_reduction = script->public_floats.at("evil_spirit_vel_reduction");
		spirit_duration = script->public_floats.at("spirit_duration");
		spirit_max_duration = script->public_floats.at("spirit_max_duration");
		target_defense_value = script->public_floats.at("target_defense_value");
		current_defense_value = script->public_floats.at("current_defense_value");
		item_box_name = script->public_chars.at("item_box_name");

		turbo_max_acc_time = script->public_floats.at("turbo_max_acc_time");
		turbo_acc_bonus_over_time = script->public_floats.at("turbo_acc_bonus_over_time");
		turbo_speed_bonus = script->public_floats.at("turbo_speed_bonus");
		turbo_dec_time = script->public_floats.at("turbo_dec_time");
		
		firecracker = script->public_gos.at("firecracker");
		other_car = script->public_gos.at("other_car");
		scene_manager = script->public_gos.at("scene_manager");
		makibishi_manager = script->public_gos.at("makibishi_manager");
		ps_hit_manager = script->public_gos.at("ps_hit_manager");
		drift_go_left = script->public_gos.at("ps_drift_left");
		drift_go_right = script->public_gos.at("ps_drift_right");

	}

	void Player_Car_ActualizePublics(GameObject* game_object)
	{
		ComponentScript* script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);

		script->public_ints["current_item"] = current_item;
		script->public_floats["launched_firecracker_lifetime"] = launched_firecracker_lifetime;
		script->public_ints["car_id"] = car_id;
		script->public_ints["item_size"] = item_size;

		script->public_bools.at("using_firecracker") = using_firecracker;
		script->public_floats.at("velocity_firecracker") = velocity_firecracker;
		script->public_floats.at("explosion_radius_firecracker") = explosion_radius_firecracker;
		script->public_floats.at("velocity_makibishi") = velocity_makibishi;
		script->public_floats.at("makibishi_collision_velocity_reduction") = makibishi_collision_velocity_reduction;
		script->public_bools.at("evil_spirit_effect") = evil_spirit_effect;
		script->public_floats.at("evil_spirit_vel_reduction") = evil_spirit_vel_reduction;
		script->public_floats.at("spirit_duration") = spirit_duration;
		script->public_floats.at("spirit_max_duration") = spirit_max_duration;
		script->public_floats.at("target_defense_value") = target_defense_value;
		script->public_floats.at("current_defense_value") = current_defense_value;
		script->public_chars.at("item_box_name") = item_box_name;

		script->public_floats.at("turbo_max_acc_time") = turbo_max_acc_time;
		script->public_floats.at("turbo_acc_bonus_over_time") = turbo_acc_bonus_over_time;
		script->public_floats.at("turbo_speed_bonus") = turbo_speed_bonus;
		script->public_floats.at("turbo_dec_time") = turbo_dec_time;
		
		script->public_gos.at("firecracker") = firecracker;
		script->public_gos.at("other_car") = other_car;
		script->public_gos.at("scene_manager") = scene_manager;
		script->public_gos.at("makibishi_manager") = makibishi_manager;
		script->public_gos.at("ps_hit_manager") = ps_hit_manager;
		script->public_gos.at("ps_drift_left") = drift_go_left;
		script->public_gos.at("ps_drift_right") = drift_go_right;
	}

	void Player_Car_Start(GameObject* game_object)
	{
		evil_spirit_object[car_id] = nullptr;
		triggers_pressed[0] = 0;
		triggers_pressed[1] = 1;
		last_trigger = -1;
		//Get Evilspirit if there is one
		for (std::vector<GameObject*>::const_iterator it = (*game_object->GetChilds()).begin(); it != (*game_object->GetChilds()).end(); it++)
		{
			if ((*it)->name == "EvilSpirit")
			{
				evil_spirit_object[car_id] = (*it);
				evil_spirit_object[car_id]->SetActive(false);
				evil_spirit_start_pos = evil_spirit_object[car_id]->transform->GetPosition();
				break;
			}
		}	

		// Start Sound Engine
		ComponentAudioSource *audio = (ComponentAudioSource*)game_object->GetComponent(ComponentType::C_AUDIO_SOURCE);
		if (audio) audio->PlayAudio(2);

		//Init particles
		ps_hit_func = (PSHit_CarCollision)GetProcAddress(App->scripting->scripts_lib->lib, "ParticleHit_CarCollision");
		ps_hit_wall_func = (PSHit_WallCollision)GetProcAddress(App->scripting->scripts_lib->lib, "ParticleHit_WallCollision");

		unsigned int kart_model;
		if (car_id == 0) kart_model = App->go_manager->team1_car;
		else kart_model = App->go_manager->team2_car;

		if (kart_model == 0)
		{
			drift_go_left->transform->SetPosition(float3(1.021f, 0.578f, -1.464f));
			drift_go_right->transform->SetPosition(float3(-1.021f, 0.578f, -1.464f));
		}
		else
		{
			drift_go_left->transform->SetPosition(float3(1.8f, 0.623f, -1.1f));
			drift_go_right->transform->SetPosition(float3(-1.8f, 0.623f, -1.1f));
		}

		drift_go_left->SetActive(false);
		drift_go_right->SetActive(false);
	}

#pragma region Forward Declarations
	void Player_Car_OnPickItem(GameObject* game_object);
	void Player_Car_ChooseItem(GameObject* game_object);
	void Player_Car_UseEvilSpirit(GameObject* game_object, ComponentCar* car);
	void Player_Car_UseMakibishi(GameObject* game_object, ComponentCar* car);
	void Player_Car_UseFirecracker(GameObject* game_object, ComponentCar* car);
	void Player_Car_UpdateSpiritEffect(GameObject* game_object, ComponentCar* car);
	void Player_Car_UpdateFirecrackerEffect(GameObject* game_object, ComponentCar* car);
	//void Player_Car_UpdateLaunchedFirecracker(GameObject* game_object, ComponentCar* car);
	void Player_Car_CallUpdateItems();
	void Player_Car_TMP_Use_Makibishi(GameObject* game_object, ComponentCar* car);
	void Player_Car_SpiritEffectDefense(GameObject* game_object, ComponentCar* car);
#pragma endregion

	void Player_Car_Update(GameObject* game_object)
	{
		ComponentCar* car = (ComponentCar*)game_object->GetComponent(ComponentType::C_CAR);
		if (car == nullptr)
			return;

		// DEBUG CRZ for Audio Testing
		if (App->input->GetKey(SDL_SCANCODE_1) == KEY_DOWN)
		{
			Player_Car_UseEvilSpirit(game_object, car);
		}

		if (current_item != -1 && evil_spirit_effect == false)
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
						break;
					}
					case(FIRECRACKER):
					{
						Player_Car_UseFirecracker(game_object, car);
					}
				}
			}
		}

		if ((App->input->GetKey(SDL_SCANCODE_Y) == KEY_DOWN || App->input->GetJoystickButton(car->GetFrontPlayer(),JOY_BUTTON::LB) == KEY_DOWN || App->input->GetJoystickButton(car->GetFrontPlayer(),JOY_BUTTON::LB) == KEY_DOWN ) && evil_spirit_effect == false)
		{
			evil_spirit_effect = true;
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
			//Player_Car_UpdateLaunchedFirecracker(game_object, car);
		}

		//TMP Makibishi
		Player_Car_TMP_Use_Makibishi(game_object, car);

		//Drifting
		DRIFT_STATE drift_state = car->GetDriftState();
		if (drift_state != DRIFT_STATE::drift_failed && drift_state != DRIFT_STATE::drift_none)
		{
			drift_go_left->SetActive(true);
			drift_go_right->SetActive(true);
		}
		else
		{
			drift_go_left->SetActive(false);
			drift_go_right->SetActive(false);
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

			float3 col_point = otherCarPos + (norm * 0.5f);
			ps_hit_func(col_point);
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
						car->OnGetHit(makibishi_collision_velocity_reduction);
						item->SetActive(false);
						car->RemoveHitodama();

						if (other_car != nullptr)
						{
							if (firecracker && firecracker->transform->GetPosition().Distance(other_car->transform->GetPosition()) <= explosion_radius_firecracker)
								((ComponentCar*)other_car->GetComponent(C_SCRIPT))->OnGetHit(makibishi_collision_velocity_reduction);
						}
					}
					else if (item->name == "Makibishi")
					{
						car->OnGetHit(makibishi_collision_velocity_reduction);
						col->SetPos(0, 0, 0);//avoid collision
						item->SetActive(false);
						col->GetCollider()->SetActive(false);
						car->RemoveHitodama();
						ps_hit_wall_func(car_id, ((ComponentTransform*)(car->GetGameObject()->GetComponent(C_TRANSFORM)))->GetPosition());
					}
					else if (item->name == item_box_name.c_str())
					{
						Player_Car_ChooseItem(game_object);
						Player_Car_OnPickItem(game_object);
						Player_Car_CallUpdateItems();
					}
				}
			}
		}
		else
		{
			if (col->GetCollider()->GetGameObject()->layer == 2) //Wall
			{
				ps_hit_wall_func(car_id, ((ComponentTransform*)(car->GetGameObject()->GetComponent(C_TRANSFORM)))->GetPosition());
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
			else if (result <= 65)
			{
				current_item = MAKIBISHI;
				item_size = 3;
			}
			else
			{
				current_item = MAKIBISHI;
				item_size = 1;
			}
		}
		else if (car->place == 2)
		{
			//2nd
			if (result <= 25)
			{
				current_item = EVIL_SPIRIT;
			}
			else if (result <= 35)
			{
				current_item = MAKIBISHI;
				item_size = 3;
			}
			else if (result <= 55)
			{
				current_item = MAKIBISHI;
				item_size = 1;
			}
			else
			{
				current_item = FIRECRACKER;
			}
		}
		LOG("Current item: %i", current_item);
		LOG("Item size: %i", item_size);
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
					(car_id == 0 ? makibishis_1 : makibishis_2).clear();
					for (uint i = 0; i < item_size; i++)
					{
						(car_id == 0 ? makibishis_1 : makibishis_2).push_back(get_makibishi());
						((ComponentScript*)(car_id == 0 ? makibishis_1 : makibishis_2).back()->GetComponent(ComponentType::C_SCRIPT))->public_floats.at("current_time_throwing_makibishi") = 0.0f;
					}
				}
				break;
			}
			case(FIRECRACKER):
			{
				if (firecracker != nullptr)
				{
					firecracker->SetActive(true);
					//firecracker->GetComponent(ComponentType::C_COLLIDER)->SetActive(false);
					float3 last_scale = firecracker->transform->GetScale();
					//((ComponentCollider*)firecracker->GetComponent(ComponentType::C_COLLIDER))->body->SetTransform(game_object->transform->GetTransformMatrix().Transposed().ptr());
					firecracker->transform->Set(game_object->GetGlobalMatrix());
					firecracker->transform->SetScale(last_scale);
				}
			}
		}
	}


	void Player_Car_UseEvilSpirit(GameObject* game_object, ComponentCar* car)
	{
		current_item = -1;
		Player_Car_CallUpdateItems();
		if (car->place == 2)
		{
			((ComponentScript*)other_car->GetComponent(ComponentType::C_SCRIPT))->public_bools.at("evil_spirit_effect") = true;
			((ComponentScript*)other_car->GetComponent(ComponentType::C_SCRIPT))->public_floats.at("spirit_duration") = 0.0f;
			((ComponentScript*)other_car->GetComponent(ComponentType::C_SCRIPT))->public_floats.at("current_defense_value") = 0.0f;
		}
		else
		{
			evil_spirit_effect = true;
		}

		// Playing Evil Spirit 
		ComponentAudioSource *audio = (ComponentAudioSource*)game_object->GetComponent(ComponentType::C_AUDIO_SOURCE);
		if (audio) audio->PlayAudio(3);
	}

	void Player_Car_UseMakibishi(GameObject* game_object, ComponentCar* car)
	{
		GameObject* makibishi = (*(car_id == 0 ? makibishis_1 : makibishis_2).begin());

		if (makibishi == nullptr)
		{
			current_item = -1;
			Player_Car_CallUpdateItems();
			return;
		}

		// Play Makibishi Sound
		ComponentAudioSource *audio = (ComponentAudioSource*)game_object->GetComponent(ComponentType::C_AUDIO_SOURCE);
		audio->PlayAudio(5);

		//Activating everything
		makibishi->SetActive(true);
		makibishi->GetComponent(ComponentType::C_COLLIDER)->SetActive(true);
		ComponentCollider* makibishi_collider = (ComponentCollider*)makibishi->GetComponent(ComponentType::C_COLLIDER);
		makibishi_collider->body->Stop();
		((ComponentScript*)makibishi->GetComponent(ComponentType::C_SCRIPT))->public_floats.at("current_time_throwing_makibishi") = 0.0f;
		makibishi_collider->body->SetActivationState(1);

		if (App->input->GetKey(SDL_SCANCODE_Q) == KEY_DOWN)
		{
			//Setting position
			float3 new_pos = game_object->transform->GetPosition();
			new_pos += car->kartY * (car->collShape.size.y + 2);
			makibishi_collider->body->SetTransform(game_object->transform->GetTransformMatrix().Transposed().ptr());
			makibishi_collider->body->SetPos(new_pos.x, new_pos.y, new_pos.z);

			float3 new_vel = ((game_object->transform->GetForward().Normalized() * ((velocity_makibishi * 0.75f) + ((car->GetVelocity() / 3.6f) / 2.0f))) + (game_object->GetGlobalMatrix().WorldY().Normalized() * (velocity_makibishi * 0.25f)));
			makibishi_collider->body->SetLinearSpeed(new_vel.x, new_vel.y, new_vel.z);
			item_size--;
			Player_Car_CallUpdateItems();
		}
		else
		{
			float y_joy_input = -App->input->GetJoystickAxis(car->GetBackPlayer(), JOY_AXIS::LEFT_STICK_Y);
			//Leaving makibishi behind
			if (y_joy_input < 0)
			{
				float3 new_pos = game_object->transform->GetPosition();
				new_pos -= car->kartZ  * car->collShape.size.z;
				new_pos += car->kartY * (car->collShape.size.y + 1);
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
				new_pos += car->kartY * (car->collShape.size.y + 2);
				makibishi_collider->body->SetTransform(game_object->transform->GetTransformMatrix().Transposed().ptr());
				makibishi_collider->body->SetPos(new_pos.x, new_pos.y, new_pos.z);

				float x_joy_input = -App->input->GetJoystickAxis(car->GetBackPlayer(), JOY_AXIS::LEFT_STICK_X);
				float3 new_vel = ((game_object->transform->GetForward().Normalized() * y_joy_input * ((velocity_makibishi * 0.75f) + ((car->GetVelocity() / 3.6f) / 2.0f))) + (game_object->GetGlobalMatrix().WorldY().Normalized() * y_joy_input * (velocity_makibishi * 0.25f)));
				new_vel += (game_object->transform->GetGlobalMatrix().WorldX().Normalized() * x_joy_input * (velocity_makibishi / 2));
				makibishi_collider->body->SetLinearSpeed(new_vel.x, new_vel.y, new_vel.z);
				Player_Car_CallUpdateItems();
			}
			item_size--;
		}
		if (car_id == 0)
		{
			if (makibishis_1.empty() == false)
				makibishis_1.erase(makibishis_1.begin());
			else
				LOG("Tried to erase an empty list of makibishi1");
		}
		else
		{
			if (makibishis_2.empty() == false)
				makibishis_2.erase(makibishis_2.begin());
			else
				LOG("Tried to erase an empty list of makibishi2");
		}
		if (item_size == 0)
		{
			current_item = -1;
			item_size = 1;
			Player_Car_CallUpdateItems();
		}
	}

	void Player_Car_UseFirecracker(GameObject* game_object, ComponentCar* car)
	{
		//car->UseItem();
		using_firecracker = true;
		current_item = -1;
		car->NewTurbo(Turbo(turbo_max_acc_time, turbo_lifetime, 0.0f, turbo_acc_bonus_over_time, 0.0f, turbo_speed_bonus));

		// Playing Firecracker sound
		ComponentAudioSource *audio = (ComponentAudioSource*)game_object->GetComponent(ComponentType::C_AUDIO_SOURCE);
		if (audio) audio->PlayAudio(0);
	}

	void Player_Car_UpdateSpiritEffect(GameObject* game_object, ComponentCar* car)
	{
		if (spirit_duration == 0.0f && car->GetInvertStatus() == false)
		{
			//car->SetMaxVelocity(car->GetMaxVelocity() * (1 - evil_spirit_vel_reduction));
			car->SetInvertStatus(true);
			float vel = (evil_spirit_vel_reduction * (car->GetMaxVelocity()*KMH_TO_UNITS));
			Turbo a = Turbo(0.0f, spirit_max_duration*0.6f, spirit_max_duration*0.4f, 0.0f, -0.5f, -vel);
			car->NewTurbo(a);

			if (evil_spirit_object[car_id] != nullptr)
			{
				evil_spirit_object[car_id]->SetActive(true);
			}
		}
		else
		{
			spirit_duration += time->DeltaTime();

			Player_Car_SpiritEffectDefense(game_object, car);

			if (evil_spirit_object[car_id] != nullptr)
			{
				evil_spirit_object[car_id]->transform->Rotate(Quat::Quat(float3(0, 1, 0), 5.0f*time->DeltaTime()));
				evil_spirit_object[car_id]->transform->SetPosition(evil_spirit_start_pos + float3(0,math::Sin(spirit_duration*10.0f)*0.2f , 0));
				GameObject* evil_child = (*(*evil_spirit_object[car_id]->GetChilds()).begin());
				if(evil_child != nullptr)
					evil_child->transform->Rotate(Quat::Quat(float3(0, 1, 0), 0.5f*time->DeltaTime()));
			}
				
			if (spirit_duration >= spirit_max_duration || current_defense_value >= target_defense_value)
			{
				//car->SetMaxVelocity(car->GetMaxVelocity() / (1 - evil_spirit_vel_reduction));
				car->SetInvertStatus(false);
				evil_spirit_effect = false;
				spirit_duration = 0.0f;
				current_defense_value = 0.0f;
				car->NewTurbo(Turbo(0, 1.0f, turbo_dec_time, 0, 0, 10.0f));
				if (evil_spirit_object[car_id] != nullptr)
				{
					evil_spirit_object[car_id]->SetActive(false);
				}
			}

			if (!evil_spirit_effect)
			{
				// Stopping Evil Spirit Sound
				ComponentAudioSource *audio = (ComponentAudioSource*)game_object->GetComponent(ComponentType::C_AUDIO_SOURCE);
				if (audio) audio->PlayAudio(4);
			}
		}
	}

	void Player_Car_UpdateFirecrackerEffect(GameObject* game_object, ComponentCar* car)
	{

		if (App->input->GetJoystickButton(car->GetBackPlayer(), JOY_BUTTON::B) == KEY_UP || App->input->GetKey(SDL_SCANCODE_Q) == KEY_UP)
		{
			car->NewTurbo(Turbo(0, 0, 0, 0, 0, 0));
			turbo_timer = 0.0f;
			using_firecracker = false;
			if (firecracker)
				firecracker->SetActive(false);
			current_item = -1;
			Player_Car_CallUpdateItems();		
		}
		else
		{
			turbo_timer += time->DeltaTime();
			if (turbo_timer >= turbo_lifetime)
			{
				car->NewTurbo(Turbo(0, 0, 0, 0, 0, 0));
				turbo_timer = 0.0f;
				using_firecracker = false;
				if(firecracker)
					firecracker->SetActive(false);
				current_item = -1;
				Player_Car_CallUpdateItems();							
			}
		}	

		// If current_item is -1 or using_firecracker is false, firecracker has finished
		if (!using_firecracker)
		{
			// Stopping Firecracker sound
			ComponentAudioSource *audio = (ComponentAudioSource*)game_object->GetComponent(ComponentType::C_AUDIO_SOURCE);
			if (audio) audio->PlayAudio(1);
		}
	}

	void Player_Car_CallUpdateItems()
	{
		string path = ((ComponentScript*)scene_manager->GetComponent(C_SCRIPT))->GetPath();
		path.append("_UpdateItems");
		if (f_UpdateItems update_items = (f_UpdateItems)GetProcAddress(App->scripting->scripts_lib->lib, path.c_str()))
		{
    			update_items(car_id, current_item, item_size);
		}
	}

	void Player_Car_TMP_Use_Makibishi(GameObject* game_object, ComponentCar* car)
	{
		if (App->input->GetJoystickButton(car->GetFrontPlayer(), JOY_BUTTON::Y) == KEY_DOWN || App->input->GetJoystickButton(car->GetBackPlayer(), JOY_BUTTON::Y) == KEY_DOWN || App->input->GetKey(SDL_SCANCODE_U) == KEY_DOWN)
		{
			string path = ((ComponentScript*)makibishi_manager->GetComponent(C_SCRIPT))->GetPath();
			path.append("_GetMakibishi");
			if (f_GetMakibishi get_makibishi = (f_GetMakibishi)GetProcAddress(App->scripting->scripts_lib->lib, path.c_str()))
			{
				(car_id == 0 ? makibishis_1 : makibishis_2).push_back(get_makibishi());
				((ComponentScript*)(car_id == 0 ? makibishis_1 : makibishis_2).back()->GetComponent(ComponentType::C_SCRIPT))->public_floats.at("current_time_throwing_makibishi") = 0.0f;

				GameObject* makibishi = (*(car_id == 0 ? makibishis_1 : makibishis_2).begin());

				if (makibishi == nullptr)
				{
					current_item = -1;
					Player_Car_CallUpdateItems();
					return;
				}

				//Activating everything
				makibishi->SetActive(true);
				makibishi->GetComponent(ComponentType::C_COLLIDER)->SetActive(true);
				ComponentCollider* makibishi_collider = (ComponentCollider*)makibishi->GetComponent(ComponentType::C_COLLIDER);
				makibishi_collider->body->Stop();
				((ComponentScript*)makibishi->GetComponent(ComponentType::C_SCRIPT))->public_floats.at("current_time_throwing_makibishi") = 0.0f;
				makibishi_collider->body->SetActivationState(1);

				if (App->input->GetKey(SDL_SCANCODE_U) == KEY_DOWN)
				{
					//Setting position
					float3 new_pos = game_object->transform->GetPosition();
					new_pos += car->kartY * (car->collShape.size.y + 2);
					makibishi_collider->body->SetTransform(game_object->transform->GetTransformMatrix().Transposed().ptr());
					makibishi_collider->body->SetPos(new_pos.x, new_pos.y, new_pos.z);
					float3 new_vel = ((game_object->transform->GetForward().Normalized() * ((velocity_makibishi * 0.75f) + ((car->GetVelocity() / 3.6f) / 2.0f))) + (game_object->GetGlobalMatrix().WorldY().Normalized() * (velocity_makibishi * 0.25f)));
					makibishi_collider->body->SetLinearSpeed(new_vel.x, new_vel.y, new_vel.z);
				}
				else
				{
					float y_joy_input = 0.0f;
					if (App->input->GetJoystickButton(car->GetBackPlayer(), JOY_BUTTON::Y) == KEY_DOWN)
						y_joy_input = -App->input->GetJoystickAxis(car->GetBackPlayer(), JOY_AXIS::LEFT_STICK_Y);
					else
						y_joy_input = -App->input->GetJoystickAxis(car->GetFrontPlayer(), JOY_AXIS::LEFT_STICK_Y);
					//Leaving makibishi behind
					if (y_joy_input < 0)
					{
						float3 new_pos = game_object->transform->GetPosition();
						new_pos -= car->kartZ  * car->collShape.size.z;
						new_pos += car->kartY * (car->collShape.size.y + 1);
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
						new_pos += car->kartY * (car->collShape.size.y + 2);
						makibishi_collider->body->SetTransform(game_object->transform->GetTransformMatrix().Transposed().ptr());
						makibishi_collider->body->SetPos(new_pos.x, new_pos.y, new_pos.z);

						float x_joy_input = 0.0f;
						if (App->input->GetJoystickButton(car->GetBackPlayer(), JOY_BUTTON::Y) == KEY_DOWN)
							x_joy_input = -App->input->GetJoystickAxis(car->GetBackPlayer(), JOY_AXIS::LEFT_STICK_X);
						else
							x_joy_input = -App->input->GetJoystickAxis(car->GetFrontPlayer(), JOY_AXIS::LEFT_STICK_X);
						float3 new_vel = ((game_object->transform->GetForward().Normalized() * y_joy_input * ((velocity_makibishi * 0.75f) + ((car->GetVelocity() / 3.6f) / 2.0f))) + (game_object->GetGlobalMatrix().WorldY().Normalized() * y_joy_input * (velocity_makibishi * 0.25f)));
						new_vel += (game_object->transform->GetGlobalMatrix().WorldX().Normalized() * x_joy_input * (velocity_makibishi / 2));
						makibishi_collider->body->SetLinearSpeed(new_vel.x, new_vel.y, new_vel.z);
					}
				}
				if (car_id == 0)
				{
					if (makibishis_1.empty() == false)
						makibishis_1.erase(makibishis_1.begin());
					else
						LOG("Tried to erase an empty list of makibishi1");
				}
				else
				{
					if (makibishis_2.empty() == false)
						makibishis_2.erase(makibishis_2.begin());
					else
						LOG("Tried to erase an empty list of makibishi2");
				}

				if ((car_id == 0 ? makibishis_1 : makibishis_2).size() == 0)
				{
					current_item = -1;
					Player_Car_CallUpdateItems();
				}
			}
		}
	}

	void Player_Car_SpiritEffectDefense(GameObject* game_object, ComponentCar* car)
	{
		float tmp_defense_value = current_defense_value;

		if (App->input->GetJoystickAxis(car->GetBackPlayer(), JOY_AXIS::LEFT_TRIGGER) < 0.2f && triggers_pressed[0] == true)
		{
			triggers_pressed[0] = false;
		}

		if (App->input->GetJoystickAxis(car->GetBackPlayer(), JOY_AXIS::RIGHT_TRIGGER) < 0.2f && triggers_pressed[1] == true)
		{
			triggers_pressed[1] = false;
		}

		if (App->input->GetJoystickAxis(car->GetBackPlayer(), JOY_AXIS::LEFT_TRIGGER) > 0.75f && triggers_pressed[1] == false && (last_trigger == 1 || last_trigger == -1))
		{
			last_trigger = 0;
			current_defense_value += 10;
			triggers_pressed[0] = true;
		}

		if (App->input->GetJoystickAxis(car->GetBackPlayer(), JOY_AXIS::RIGHT_TRIGGER) > 0.75f && triggers_pressed[1] == false && (last_trigger == 0 || last_trigger == -1))
		{
			last_trigger = 1;
			current_defense_value += 10;
			triggers_pressed[1] = true;
		}

		if (tmp_defense_value == current_defense_value)
			current_defense_value -= 5.0f * time->DeltaTime();


	}
}