#include "stdafx.h"

#include "../Application.h"

#include "../GameObject.h"
#include "../ComponentScript.h"
#include "../ComponentParticleSystem.h"
#include "../ComponentTransform.h"
#include "../Time.h"

//Testing
#include "../ModuleInput.h"
#include "../SDL/include/SDL_scancode.h"

#include <map>

namespace ParticleHit
{
	GameObject* go_car_hit;
	ComponentParticleSystem* ps_car;

	float car_timer = 0.0f;
	bool car_ps_active = false;

	void ParticleHit_GetPublics(map<const char*, string>* public_chars, map<const char*, int>* public_ints, map<const char*, float>* public_float, map<const char*, bool>* public_bools, map<const char*, GameObject*>* public_gos)
	{
		public_gos->insert(pair<const char*, GameObject*>("Car Hit", go_car_hit));
	}

	void ParticleHit_UpdatePublics(GameObject* game_object)
	{
		ComponentScript* script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);

		go_car_hit = script->public_gos.at("Car Hit");
	}

	void ParticleHit_ActualizePublics(GameObject* game_object)
	{
		ComponentScript* script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);

		script->public_gos.at("Car Hit") = go_car_hit;
	}

	void ParticleHit_Start(GameObject* game_object)
	{
		ps_car = (ComponentParticleSystem*)go_car_hit->GetComponent(ComponentType::C_PARTICLE_SYSTEM);
	}

	void ParticleHit_Update(GameObject* game_object)
	{
		if (car_ps_active)
		{
			car_timer += time->DeltaTime();
			if (car_timer >= 1.0f)
			{
				car_timer = 0.0f;
				car_ps_active = false;
			}
		}
	}

	void ParticleHit_CarCollision(const math::float3& point)
	{
		if (car_ps_active == false)
		{
			go_car_hit->transform->SetPosition(point);
			ps_car->Play();
			car_ps_active = true;
		}
		
	}
}