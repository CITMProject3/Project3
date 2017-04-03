#include "stdafx.h"

#include "../Application.h"

#include "../PhysBody3D.h"

#include "../GameObject.h"
#include "../ComponentCollider.h"
#include "../ComponentAudioSource.h"

#include "../EventQueue.h"
#include "../EventPlaySound.h"

namespace Audio_PlaySound
{
	/*void Audio_GetPublics(map<const char*, string>* public_chars, map<const char*, int>* public_ints, map<const char*, float>* public_float, map<const char*, bool>* public_bools, map<const char*, GameObject*>* public_gos)
	{ }

	void Audio_UpdatePublics(GameObject* game_object)
	{ }*/

	void Audio_PlaySound_Start(GameObject* game_object)
	{ }

	void Audio_PlaySound_Update(GameObject* game_object)
	{ }

	void Audio_PlaySound_OnCollision(PhysBody3D* col)
	{
		ComponentAudioSource *audio = (ComponentAudioSource*)col->GetCollider()->GetGameObject()->GetComponent(ComponentType::C_AUDIO_SOURCE);
		App->event_queue->PostEvent(new EventPlaySound(audio));
	}
}