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

namespace Checkpoint
{
	//Public
	int Checkpoint_N = 1;

	void Checkpoint_GetPublics(map<const char*, string>* public_chars, map<const char*, int>* public_ints, map<const char*, float>* public_float, map<const char*, bool>* public_bools, map<const char*, GameObject*>* public_gos)
	{
		public_ints->insert(pair<const char*, int>("CheckpointN", Checkpoint_N));
	}

	void Checkpoint_UpdatePublics(GameObject* game_object)
	{
		ComponentScript* Checkpoint_script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);
		Checkpoint_N = Checkpoint_script->public_ints.at("CheckpointN");
	}

	void Checkpoint_ActualizePublics(GameObject* game_object)
	{
		ComponentScript* Checkpoint_script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);
		Checkpoint_script->public_ints.at("CheckpointN") = Checkpoint_N;
	}

	void Checkpoint_Start(GameObject* game_object)
	{}

	void Checkpoint_Update(GameObject* game_object)
	{}

	void Checkpoint_OnCollision(GameObject* game_object, PhysBody3D* col)
	{
		ComponentCar* car = col->GetCar();
		ComponentTransform* trs = (ComponentTransform*)game_object->GetComponent(C_TRANSFORM);
		if (car && trs)
		{
			car->WentThroughCheckpoint(Checkpoint_N, trs->GetPosition(), trs->GetRotation());
		}
	}
}