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

namespace Finish_lane
{
	//Public
	int Checkpoint_N = 1;

	void Finish_lane_GetPublics(map<const char*, string>* public_chars, map<const char*, int>* public_ints, map<const char*, float>* public_float, map<const char*, bool>* public_bools, map<const char*, GameObject*>* public_gos)
	{
		public_ints->insert(pair<const char*, int>("CheckpointN", Checkpoint_N));
	}

	void Finish_lane_UpdatePublics(GameObject* game_object)
	{
		ComponentScript* Checkpoint_script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);
		Checkpoint_N = Checkpoint_script->public_ints.at("CheckpointN");
	}

	void Finish_lane_ActualizePublics(GameObject* game_object)
	{
		ComponentScript* Checkpoint_script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);
		Checkpoint_script->public_ints.at("CheckpointN") = Checkpoint_N;
	}

	void Finish_lane_Start(GameObject* game_object)
	{}

	void Finish_lane_Update(GameObject* game_object)
	{}

	void Finish_lane_OnCollision(GameObject* game_object, PhysBody3D* col)
	{
		ComponentCar* car = col->GetCar();
		ComponentTransform* trs = (ComponentTransform*)game_object->GetComponent(C_TRANSFORM);
		if (car && trs)
		{
			Finish_lane_UpdatePublics(game_object);
			car->WentThroughEnd(Checkpoint_N, trs->GetPosition(), trs->GetRotation());
		}
	}
}