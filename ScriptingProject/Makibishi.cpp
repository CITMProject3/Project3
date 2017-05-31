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
#include "../ComponentAudioSource.h"
#include "../SDL/include/SDL_scancode.h"
#include "../PhysBody3D.h"
#include "../ComponentCollider.h"
#include "../ComponentCar.h"
#include "../Globals.h"
#include "../Time.h"

namespace Makibishi
{
	float time_trowing_makibishi = 90.0f;
	float current_time_throwing_makibishi = 0.0f;
	bool sound_played = false;

	void Makibishi_GetPublics(map<const char*, string>* public_chars, map<const char*, int>* public_ints, map<const char*, float>* public_float, map<const char*, bool>* public_bools, map<const char*, GameObject*>* public_gos)
	{
		public_float->insert(pair<const char*, float>("time_trowing_makibishi", time_trowing_makibishi));
		public_float->insert(pair<const char*, float>("current_time_throwing_makibishi", current_time_throwing_makibishi));
	}

	void Makibishi_UpdatePublics(GameObject* game_object)
	{
		ComponentScript* test_script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);

		time_trowing_makibishi = test_script->public_floats.at("time_trowing_makibishi");
		current_time_throwing_makibishi = test_script->public_floats.at("current_time_throwing_makibishi");

	}

	void Makibishi_ActualizePublics(GameObject* game_object)
	{
		ComponentScript* test_script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);

		test_script->public_floats.at("time_trowing_makibishi") = time_trowing_makibishi;
		test_script->public_floats.at("current_time_throwing_makibishi") = current_time_throwing_makibishi;
	}

	void Makibishi_Start(GameObject* game_object)
	{
		sound_played = false;
	}

	void Makibishi_Update(GameObject* game_object)
	{
		if (!game_object->IsActive())
		{
			current_time_throwing_makibishi = 0.0f;
		}
		else
		{

			if (!sound_played)
			{
				ComponentAudioSource *audio = (ComponentAudioSource*)game_object->GetComponent(ComponentType::C_AUDIO_SOURCE);
				audio->PlayAudio(0);
				sound_played = true;
			}

			game_object->GetComponent(ComponentType::C_COLLIDER)->SetActive(true);
			current_time_throwing_makibishi += time->DeltaTime();
			if (current_time_throwing_makibishi >= time_trowing_makibishi)
			{
				current_time_throwing_makibishi = 0.0f;
				((ComponentCollider*)game_object->GetComponent(ComponentType::C_COLLIDER))->body->SetPos(0, 0, 0);//avoid collision
				game_object->SetActive(false);
				game_object->GetComponent(ComponentType::C_COLLIDER)->SetActive(false);
				sound_played = false;
			}
		}
	}
}