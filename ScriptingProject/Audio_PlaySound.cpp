#include "stdafx.h"

#include "../PhysBody3D.h"

#include "../GameObject.h"
#include "../ComponentAudio.h"
#include "../ComponentCollider.h"

namespace Audio_PlaySound
{
	/*void Audio_GetPublics(map<const char*, string>* public_chars, map<const char*, int>* public_ints, map<const char*, float>* public_float, map<const char*, bool>* public_bools, map<const char*, GameObject*>* public_gos)
	{
	}

	void Audio_UpdatePublics(GameObject* game_object)
	{
	}*/

	void Audio_PlaySound_Start(GameObject* game_object)
	{ }

	void Audio_PlaySound_Update(GameObject* game_object)
	{ }

	void Audio_PlaySound_OnCollision(PhysBody3D* col)
	{
		ComponentAudio *audio = (ComponentAudio*)col->GetCollider()->GetGameObject()->GetComponent(ComponentType::C_AUDIO);
		if(audio) audio->PlayEvent();
	}
}