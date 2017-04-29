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

namespace TurboPad
{
	//Public
	//int Checkpoint_N = 1;

	void TurboPad_GetPublics(map<const char*, string>* public_chars, map<const char*, int>* public_ints, map<const char*, float>* public_float, map<const char*, bool>* public_bools, map<const char*, GameObject*>* public_gos)
	{
		//public_ints->insert(pair<const char*, int>("CheckpointN", Checkpoint_N));
	}

	void TurboPad_UpdatePublics(GameObject* game_object)
	{
		//ComponentScript* Checkpoint_script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);
		//Checkpoint_N = Checkpoint_script->public_ints.at("CheckpointN");
	}

	void TurboPad_ActualizePublics(GameObject* game_object)
	{
		//ComponentScript* Checkpoint_script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);
		//Checkpoint_script->public_ints.at("CheckpointN") = Checkpoint_N;
	}

	void TurboPad_Start(GameObject* game_object)
	{}

	void TurboPad_Update(GameObject* game_object)
	{}

	void TurboPad_OnCollision(GameObject* game_object, PhysBody3D* col)
	{
		ComponentCar* car = col->GetCar();
		if (car)
		{
			car->current_turbo = T_DRIFT_MACH_2;
		}
	}
}