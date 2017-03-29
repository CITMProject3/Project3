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

namespace Player_Car
{

	void Player_Car_GetPublics(map<const char*, string>* public_chars, map<const char*, int>* public_ints, map<const char*, float>* public_float, map<const char*, bool>* public_bools, map<const char*, GameObject*>* public_gos)
	{
	}

	void Player_Car_UpdatePublics(GameObject* game_object)
	{
	}

	void Player_Car_Start(GameObject* game_object)
	{
	}

	void Player_Car_Update(GameObject* game_object)
	{
	}
	void Player_Car_OnCollision(PhysBody3D* col)
	{
		if (ReadFlag(col->collisionOptions, col->co_isItem))
		{
			ComponentCollider* Hitodama_Player_Car = col->GetCollider();
			if (Hitodama_Player_Car->GetGameObject()->name.compare("Hitodama"))
			{
				//Do something
			}
		}
	}
}