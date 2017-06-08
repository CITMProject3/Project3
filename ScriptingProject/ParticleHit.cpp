#include "stdafx.h"

#include "../Application.h"

#include "../GameObject.h"
#include "../ComponentScript.h"
#include "../ComponentParticleSystem.h"
#include "../ComponentTransform.h"
#include "../ComponentAudioSource.h"
#include "../Time.h"

//Testing
#include "../ModuleInput.h"
#include "../SDL/include/SDL_scancode.h"

#include <map>

namespace ParticleHit
{
	GameObject* go_car_hit = nullptr;
	ComponentParticleSystem* ps_car = nullptr;

	GameObject* go_wall_hit1 = nullptr;
	GameObject* go_wall_hit2 = nullptr;
	ComponentParticleSystem* ps_wall1 = nullptr;
	ComponentParticleSystem* ps_wall2 = nullptr;
	ComponentAudioSource *audio_source = nullptr;

	float car_timer = 0.0f;
	bool car_ps_active = false;

	float wall_timer1 = 0.0f;
	float wall_timer2 = 0.0f;
	bool wall_ps_active1 = false;
	bool wall_ps_active2 = false;

	void ParticleHit_GetPublics(map<const char*, string>* public_chars, map<const char*, int>* public_ints, map<const char*, float>* public_float, map<const char*, bool>* public_bools, map<const char*, GameObject*>* public_gos)
	{
		public_gos->insert(pair<const char*, GameObject*>("Car Hit", go_car_hit));
		public_gos->insert(pair<const char*, GameObject*>("Wall Hit 1", go_wall_hit1));
		public_gos->insert(pair<const char*, GameObject*>("Wall Hit 2", go_wall_hit2));
	}

	void ParticleHit_UpdatePublics(GameObject* game_object)
	{
		ComponentScript* script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);

		go_car_hit = script->public_gos.at("Car Hit");
		go_wall_hit1 = script->public_gos.at("Wall Hit 1");
		go_wall_hit2 = script->public_gos.at("Wall Hit 2");
	}

	void ParticleHit_ActualizePublics(GameObject* game_object)
	{
		ComponentScript* script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);

		script->public_gos.at("Car Hit") = go_car_hit;
		script->public_gos.at("Wall Hit 1") = go_wall_hit1;
		script->public_gos.at("Wall Hit 2") = go_wall_hit2;
	}

	void ParticleHit_Start(GameObject* game_object)
	{
		ps_car = (ComponentParticleSystem*)go_car_hit->GetComponent(ComponentType::C_PARTICLE_SYSTEM);
		ps_wall1 = (ComponentParticleSystem*)go_wall_hit1->GetComponent(ComponentType::C_PARTICLE_SYSTEM);
		ps_wall2 = (ComponentParticleSystem*)go_wall_hit2->GetComponent(ComponentType::C_PARTICLE_SYSTEM);
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

		if (wall_ps_active1)
		{
			wall_timer1 += time->DeltaTime();
			if (wall_timer1 >= 1.0f)
			{
				wall_timer1 = 0.0f;
				wall_ps_active1 = false;
			}
		}

		if (wall_ps_active2)
		{
			wall_timer2 += time->DeltaTime();
			if (wall_timer2 >= 1.0f)
			{
				wall_timer2 = 0.0f;
				wall_ps_active2 = false;
			}
		}
	}

	void ParticleHit_CarCollision(GameObject* game_object, const math::float3& point)
	{
		if (car_ps_active == false)
		{
			go_car_hit->transform->SetPosition(point);
			ps_car->Play();
			car_ps_active = true;

			// Playing impact sound
			if (audio_source) audio_source->PlayAudio(10);
		}
	}

	void ParticleHit_WallCollision(GameObject *game_object, int car_id, const math::float3& point, bool with_makibishi)
	{
		if (car_id == 0)
		{
			if (wall_ps_active1 == false)
			{
				go_wall_hit1->transform->SetPosition(point);
				ps_wall1->Play();
				wall_ps_active1 = true;

				// Playing impact sound
				audio_source = (ComponentAudioSource*)game_object->GetComponent(ComponentType::C_AUDIO_SOURCE);
				if (audio_source)
				{					
					if(with_makibishi)
						audio_source->PlayAudio(9);  // Makibishi
					else
						audio_source->PlayAudio(10); // Terrain
				}
			}	
		}
		else
		{
			if (wall_ps_active2 == false)
			{
				go_wall_hit2->transform->SetPosition(point);
				ps_wall2->Play();
				wall_ps_active2 = true;

				// Playing impact sound
				audio_source = (ComponentAudioSource*)game_object->GetComponent(ComponentType::C_AUDIO_SOURCE);
				if (audio_source)
				{
					if (with_makibishi)
						audio_source->PlayAudio(9);  // Makibishi	
					else
						audio_source->PlayAudio(10); // Terrain
				}
			}
		}
	}
}