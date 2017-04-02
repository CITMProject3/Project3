#include "stdafx.h"

#include <string>
#include <map>
#include <vector>

#include "../Application.h"
#include "../ModuleScripting.h"
#include "../ModuleInput.h"
#include "../ModuleWindow.h"
#include "../GameObject.h"
#include "../ComponentScript.h"
#include "../ComponentTransform.h"
#include "../SDL/include/SDL_scancode.h"
#include "../PhysBody3D.h"
#include "../ComponentCollider.h"
#include "../Globals.h"

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
		if (ReadFlag(col->collisionOptions, col->co_isItem))
		{
			ComponentCollider* Hitodama_col = col->GetCollider();
			if (Hitodama_col->GetGameObject()->name.compare("Hitodama"))
			{
				//Do something
			}
		}
	}
}