#include "stdafx.h"

#include "../Application.h"
#include "../ModuleScripting.h"
#include "../ComponentScript.h"
#include "../ComponentTransform.h"
#include "../ComponentCollider.h"
#include "../ModuleGOManager.h"
#include "../ComponentCar.h"
#include "../GameObject.h"
#include "../PhysBody3D.h"

namespace CarPositionController
{
	//Public
	GameObject* car1 = nullptr;
	GameObject* car2 = nullptr;

	ComponentCar* c1 = nullptr;
	ComponentCar* c2 = nullptr;

	void CarPositionController_GetPublics(map<const char*, string>* public_chars, map<const char*, int>* public_ints, map<const char*, float>* public_float, map<const char*, bool>* public_bools, map<const char*, GameObject*>* public_gos)
	{
		public_gos->insert(pair<const char*, GameObject*>("car1", car1));
		public_gos->insert(pair<const char*, GameObject*>("car2", car2));
	}

	void CarPositionController_UpdatePublics(GameObject* game_object)
	{
		ComponentScript* CarPositionController_script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);
		car1 = CarPositionController_script->public_gos.at("car1");
		car2 = CarPositionController_script->public_gos.at("car2");
	}

	void CarPositionController_ActualizePublics(GameObject* game_object)
	{
		ComponentScript* CarPositionController_script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);
		CarPositionController_script->public_gos.at("car1") = car1;
		CarPositionController_script->public_gos.at("car2") = car2;
	}

	void CarPositionController_Start(GameObject* game_object)
	{
		if (car1 && car2)
		{
			c1 = (ComponentCar*)car1->GetComponent(C_CAR);
			c2 = (ComponentCar*)car2->GetComponent(C_CAR);
		}
	}

	void CarPositionController_Update(GameObject* game_object)
	{
		if (c1 && c2)
		{
			if (c1->lap == c2->lap)
			{
				if (c1->n_checkpoints > c2->n_checkpoints)
				{
					c1->place = 1;
					c2->place = 2;
				}
				else if (c2->n_checkpoints > c1->n_checkpoints)
				{
					c2->place = 1;
					c1->place = 2;
				}
				else if (c1->n_checkpoints > 0 && c2->n_checkpoints > 0)
				{
					ComponentTransform* trs1 = (ComponentTransform*)car1->GetComponent(C_TRANSFORM);
					ComponentTransform* trs2 = (ComponentTransform*)car2->GetComponent(C_TRANSFORM);
					if (c1->last_check_pos.DistanceSq(trs1->GetPosition()) > c2->last_check_pos.DistanceSq(trs2->GetPosition()))
					{
						c1->place = 1;
						c2->place = 2;
					}
					else
					{
						c2->place = 1;
						c1->place = 2;
					}
				}
			}
			else
			{
				if (c1->lap > c2->lap)
				{
					c1->place = 1;
					c2->place = 2;
				}
				else
				{
					c2->place = 1;
					c1->place = 2;
				}
			}
		}
	}

	void CarPositionController_OnCollision(GameObject* game_object, PhysBody3D* col)
	{
	}
}