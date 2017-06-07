#include "stdafx.h"

#include "../Application.h"
#include "../ModuleScripting.h"
#include "../ComponentScript.h"
#include "../ComponentTransform.h"
#include "../ComponentCollider.h"
#include "../ComponentAudioSource.h"
#include "../ModuleGOManager.h"
#include "../ComponentCar.h"
#include "../GameObject.h"
#include "../PhysBody3D.h"

namespace OutOfBounds
{
	//Public
	float timer;
	bool sound_played = false;

	void OutOfBounds_GetPublics(map<const char*, string>* public_chars, map<const char*, int>* public_ints, map<const char*, float>* public_float, map<const char*, bool>* public_bools, map<const char*, GameObject*>* public_gos)
	{
	}

	void OutOfBounds_UpdatePublics(GameObject* game_object)
	{
	}

	void OutOfBounds_ActualizePublics(GameObject* game_object)
	{
		
	}

	void OutOfBounds_Start(GameObject* game_object)
	{
		timer = 0.0f;
		sound_played = false;
	}

	void OutOfBounds_Update(GameObject* game_object)
	{
		if (sound_played)
		{
			timer += time->DeltaTime();
			if (timer > 1.0f)
			{
				sound_played = false;
				timer = 0.0f;
			}
		}

	}

	void OutOfBounds_OnCollision(GameObject* game_object, PhysBody3D* col)
	{
		ComponentCar* car = col->GetCar();
		ComponentTransform* trs = (ComponentTransform*)game_object->GetComponent(C_TRANSFORM);
		if (car && trs)
		{
			if (!sound_played)
			{
				sound_played = true;
				// Playing Car impacting water
				ComponentAudioSource *audio = (ComponentAudioSource*)game_object->GetComponent(ComponentType::C_AUDIO_SOURCE);
				if (audio) audio->PlayAudio(0);
			}

			car->Reset();
		}
	}
}