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
#include "../Time.h"

namespace Makibishi
{
	float time_trowing_makibishi = 90.0f;
	float current_time_trowing_makibishi = 0.0f;

	void Makibishi_GetPublics(map<const char*, string>* public_chars, map<const char*, int>* public_ints, map<const char*, float>* public_float, map<const char*, bool>* public_bools, map<const char*, GameObject*>* public_gos)
	{
		public_float->insert(pair<const char*, float>("time_trowing_makibishi", time_trowing_makibishi));
		public_float->insert(pair<const char*, float>("current_time_trowing_makibishi", current_time_trowing_makibishi));
	}

	void Makibishi_UpdatePublics(GameObject* game_object)
	{
		ComponentScript* test_script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);

		time_trowing_makibishi = test_script->public_floats.at("time_trowing_makibishi");
		current_time_trowing_makibishi = test_script->public_floats.at("current_time_trowing_makibishi");

	}

	void Makibishi_ActualizePublics(GameObject* game_object)
	{
		ComponentScript* test_script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);

		test_script->public_floats.at("time_trowing_makibishi") = time_trowing_makibishi;
		test_script->public_floats.at("current_time_trowing_makibishi") = current_time_trowing_makibishi;
	}

	void Makibishi_Start(GameObject* game_object)
	{
	}

	void Makibishi_Update(GameObject* game_object)
	{
		if (!game_object->IsActive())
		{
			current_time_trowing_makibishi = 0.0f;
		}
		else
		{
			game_object->GetComponent(ComponentType::C_COLLIDER)->SetActive(true);
			current_time_trowing_makibishi += time->DeltaTime();
			if (current_time_trowing_makibishi >= time_trowing_makibishi)
			{
				current_time_trowing_makibishi = 0.0f;
				game_object->SetActive(false);
				game_object->GetComponent(ComponentType::C_COLLIDER)->SetActive(false);
			}
		}
	}
}