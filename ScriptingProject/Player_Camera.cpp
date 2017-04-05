#include "stdafx.h"

#include <string>
#include <map>
#include <vector>
#include "../Application.h"
#include "../ModuleScripting.h"
#include "../ModuleInput.h"
#include "../ModuleWindow.h"
#include "../GameObject.h"
#include "../ComponentScript.h"
#include "../ComponentTransform.h"
#include "../SDL/include/SDL_scancode.h"
#include "../PhysBody3D.h"
#include "../ComponentCollider.h"
#include "../ModuleGOManager.h"
#include "../ComponentCamera.h"
#include "../ComponentCar.h"
#include "../Time.h"
#include "../Globals.h"

namespace Player_Camera
{
	//Public
	float Player_Camera_distrance_y = 4.0;
	float Player_Camera_distrance_z = -5.0;
	float Player_Camera_direction_y = 2.0;
	GameObject* Player_Camera_target = nullptr;
	float Player_Camera_inclination_separation_y = 1.0;
	float Player_Camera_inclination_separation_z = 1.0;
	//float Player_Camera_relation_vel_y = 0.5;
	//float Player_Camera_relation_vel_z = 0.5;
	float Player_Camera_relation_vel_fov = 10.0;
	float Player_Camera_fov_increment = 5.0;
	float Player_Camera_fov_decrement = 5.0;
	float Player_Camera_last_fov = 60.0;

	void Player_Camera_GetPublics(map<const char*, string>* public_chars, map<const char*, int>* public_ints, map<const char*, float>* public_float, map<const char*, bool>* public_bools, map<const char*, GameObject*>* public_gos)
	{
		public_float->insert(pair<const char*, float>("dist_y", Player_Camera_distrance_y));
		public_float->insert(pair<const char*, float>("dist_z", Player_Camera_distrance_z));
		public_float->insert(pair<const char*, float>("camera_y", Player_Camera_direction_y));
		public_gos->insert(pair<const char*, GameObject*>("target", nullptr));
		public_float->insert(pair<const char*, float>("inclination y max min", Player_Camera_inclination_separation_y));
		public_float->insert(pair<const char*, float>("inclination z max min", Player_Camera_inclination_separation_z));
		//public_float->insert(pair<const char*, float>("y change boost", Player_Camera_relation_vel_y));
		//public_float->insert(pair<const char*, float>("z change boost", Player_Camera_relation_vel_z));
		public_float->insert(pair<const char*, float>("fov change boost", Player_Camera_relation_vel_fov));
		public_float->insert(pair<const char*, float>("Player_Camera_fov_increment", Player_Camera_fov_increment));
		public_float->insert(pair<const char*, float>("Player_Camera_fov_decrement", Player_Camera_fov_decrement));
		public_float->insert(pair<const char*, float>("Player_Camera_last_fov", Player_Camera_last_fov));
	}

	void Player_Camera_UpdatePublics(GameObject* game_object)
	{
		ComponentScript* Player_Camera_script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);

		Player_Camera_distrance_y = Player_Camera_script->public_floats.at("dist_y");
		Player_Camera_distrance_z = Player_Camera_script->public_floats.at("dist_z");
		Player_Camera_direction_y = Player_Camera_script->public_floats.at("camera_y");
		Player_Camera_target = Player_Camera_script->public_gos.at("target");
		Player_Camera_inclination_separation_y = Player_Camera_script->public_floats.at("inclination y max min");
		Player_Camera_inclination_separation_z = Player_Camera_script->public_floats.at("inclination z max min");
		//Player_Camera_relation_vel_y = Player_Camera_script->public_floats.at("y change boost");
		//Player_Camera_relation_vel_z = Player_Camera_script->public_floats.at("z change boost");
		Player_Camera_relation_vel_fov = Player_Camera_script->public_floats.at("fov change boost");
		Player_Camera_fov_increment = Player_Camera_script->public_floats.at("Player_Camera_fov_increment");
		Player_Camera_fov_decrement = Player_Camera_script->public_floats.at("Player_Camera_fov_decrement");
		Player_Camera_last_fov = Player_Camera_script->public_floats.at("Player_Camera_last_fov");
	}

	void Player_Camera_ActualizePublics(GameObject* game_object)
	{
		ComponentScript* Player_Camera_script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);

		Player_Camera_script->public_floats.at("dist_y") = Player_Camera_distrance_y;
		Player_Camera_script->public_floats.at("dist_z") = Player_Camera_distrance_z;
		Player_Camera_script->public_floats.at("camera_y") = Player_Camera_direction_y;
		Player_Camera_script->public_gos.at("target") = Player_Camera_target;
		Player_Camera_script->public_floats.at("inclination y max min") = Player_Camera_inclination_separation_y;
		Player_Camera_script->public_floats.at("inclination z max min") = Player_Camera_inclination_separation_z;
		//Player_Camera_script->public_floats.at("y change boost") = Player_Camera_relation_vel_y;
		//Player_Camera_script->public_floats.at("z change boost") = Player_Camera_relation_vel_z;
		Player_Camera_script->public_floats.at("fov change boost") = Player_Camera_relation_vel_fov;
		Player_Camera_script->public_floats.at("Player_Camera_fov_increment") = Player_Camera_fov_increment;
		Player_Camera_script->public_floats.at("Player_Camera_fov_decrement") = Player_Camera_fov_decrement;
		Player_Camera_script->public_floats.at("Player_Camera_last_fov") = Player_Camera_last_fov;
	}

	void Player_Camera_Start(GameObject* game_object)
	{
		ComponentCamera* Player_Camera_cam = (ComponentCamera*)game_object->GetComponent(ComponentType::C_CAMERA);
		if (Player_Camera_cam != nullptr)
		{
			Player_Camera_last_fov = Player_Camera_cam->GetFOV();
		}
	}

	void Player_Camera_Update(GameObject* game_object)
	{
		float3 Player_Camera_euler_rot = game_object->transform->GetForward();
		game_object->transform->SetRotation(float3(0.0, Player_Camera_euler_rot.y, 0.0));

		if (Player_Camera_target != nullptr)
		{
			game_object->transform->Set(Player_Camera_target->GetGlobalMatrix());

			float rotation_x = Player_Camera_target->transform->GetForward().y / (pi/2);
			if (rotation_x > 1)
				rotation_x = 1;
			else if (rotation_x < -1)
				rotation_x = -1;

			float total_dist_z = Player_Camera_distrance_z - rotation_x * Player_Camera_inclination_separation_z;
			float total_dist_y = Player_Camera_distrance_y - rotation_x * Player_Camera_inclination_separation_y;

			ComponentCamera* Player_Camera_cam = (ComponentCamera*)game_object->GetComponent(ComponentType::C_CAMERA);

			float new_fov = Player_Camera_last_fov;

			ComponentCar* Player_Camera_car = (ComponentCar*)Player_Camera_target->GetComponent(ComponentType::C_CAR);
			if (Player_Camera_car != nullptr)
			{
				if (Player_Camera_car->GetCurrentTurbo() != T_IDLE)
				{
					//total_dist_z += Player_Camera_relation_vel_z * (Player_Camera_car->GetVelocity() / Player_Camera_car->GetMaxVelocity());
					//total_dist_y += Player_Camera_relation_vel_y * (Player_Camera_car->GetVelocity() / Player_Camera_car->GetMaxVelocity());
					new_fov += Player_Camera_relation_vel_fov * (Player_Camera_car->GetVelocity() / Player_Camera_car->GetMaxVelocity());
				}
			}

			float3 Player_Camera_target_pos = float3::zero;
			Player_Camera_target_pos += (float3(game_object->transform->GetForward().Normalized().x, game_object->transform->GetForward().Normalized().y, game_object->transform->GetForward().Normalized().z) * total_dist_z);
			Player_Camera_target_pos += (float3(0.0, total_dist_y, 0.0));

			Player_Camera_target_pos += Player_Camera_target->transform->GetPosition();

			game_object->transform->SetPosition(Player_Camera_target_pos);

			if (Player_Camera_cam != nullptr)
			{
				float3 look_at_dir(Player_Camera_target->transform->GetPosition().x, Player_Camera_target->transform->GetPosition().y + Player_Camera_direction_y, Player_Camera_target->transform->GetPosition().z);
				Player_Camera_cam->LookAt(look_at_dir);

				if(new_fov != Player_Camera_cam->GetFOV())
				{
					if (new_fov < Player_Camera_cam->GetFOV())
					{
						float decremented_fov = Player_Camera_cam->GetFOV() - (Player_Camera_fov_decrement * time->DeltaTime());
						if (decremented_fov > new_fov)
							new_fov = decremented_fov;
					}
					else
					{
						float incremented_fov = Player_Camera_cam->GetFOV() + (Player_Camera_fov_increment * time->DeltaTime());
						if (incremented_fov < new_fov)
							new_fov = incremented_fov;
					}
					Player_Camera_cam->SetFOV(new_fov);
				}
			}
		}
	}
}