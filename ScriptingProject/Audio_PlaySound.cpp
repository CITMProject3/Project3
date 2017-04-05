#include "stdafx.h"

#include "../Application.h"

#include "../PhysBody3D.h"

#include "../GameObject.h"
#include "../ComponentCollider.h"
#include "../ComponentAudioSource.h"
#include "../ComponentScript.h"

#include <map>

namespace Audio_PlaySound
{
	unsigned audio_index = 0;

	void Audio_PlaySound_GetPublics(map<const char*, string>* public_chars, map<const char*, int>* public_ints, map<const char*, float>* public_float, map<const char*, bool>* public_bools, map<const char*, GameObject*>* public_gos)
	{
		public_ints->insert(std::pair<const char*, unsigned int>("Audio index", audio_index));
	}

	void Audio_PlaySound_UpdatePublics(GameObject* game_object)
	{
		ComponentScript* script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);
		audio_index = script->public_ints["Audio index"];
	}

	void Audio_PlaySound_ActualizePublics(GameObject* game_object)
	{
		ComponentScript* script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);
		script->public_ints.at("Audio index") = audio_index;
	}

	void Audio_PlaySound_Start(GameObject* game_object)
	{ }

	void Audio_PlaySound_Update(GameObject* game_object)
	{ }

	void Audio_PlaySound_OnCollision(GameObject* game_object, PhysBody3D* col)
	{
		ComponentAudioSource *audio = (ComponentAudioSource*)col->GetCollider()->GetGameObject()->GetComponent(ComponentType::C_AUDIO_SOURCE);
		audio->PlayAudio(audio_index);
	}
}