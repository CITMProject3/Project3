#include "stdafx.h"

#include <string>
#include <map>
#include <vector>
#include "../Application.h"
#include "../ModuleScripting.h"
#include "../ModuleGOManager.h"
#include "../GameObject.h"
#include "../ComponentScript.h"
#include "../ComponentTransform.h"
#include "../Globals.h"
#include "../Time.h"

namespace MakibishiManager
{
	std::vector<GameObject*> makibishis;
	GameObject* makibishi_parent = nullptr;

	void MakibishiManager_GetPublics(map<const char*, string>* public_chars, map<const char*, int>* public_ints, map<const char*, float>* public_float, map<const char*, bool>* public_bools, map<const char*, GameObject*>* public_gos)
	{
		public_gos->insert(pair<const char*, GameObject*>("makibishi_parent", makibishi_parent));
	}

	void MakibishiManager_UpdatePublics(GameObject* game_object)
	{
		ComponentScript* script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);
		makibishi_parent = script->public_gos.at("makibishi_parent");
	}

	void MakibishiManager_Start(GameObject* game_object)
	{
		makibishis.clear();
		const std::vector<GameObject*>* childs = makibishi_parent->GetChilds();
		for (uint i = 0; i < childs->size(); i++)
		{
			makibishis.push_back((*childs)[i]);
		}
	}

	void MakibishiManager_Update(GameObject* game_object)
	{
	}

	//Gets first inactive makibishi or first thrown in scene
	GameObject* MakibishiManager_GetMakibishi(int num)
	{
		ComponentScript* older = (ComponentScript*)(*makibishis.begin())->GetComponent(C_SCRIPT);
		ComponentScript* older1 = (ComponentScript*)(*makibishis.begin())->GetComponent(C_SCRIPT);
		ComponentScript* older2 = (ComponentScript*)(*makibishis.begin())->GetComponent(C_SCRIPT);
		int finded = 0;

		for (std::vector<GameObject*>::iterator it = makibishis.begin(); it != makibishis.end(); it++)
		{
			ComponentScript* current_script = (ComponentScript*)(*it)->GetComponent(C_SCRIPT);
			assert(current_script);

			if ((*it)->IsActive() == false)
			{
				if (finded == num)
					return (*it);
				else
					finded++;
			}
			else if (current_script->public_floats["current_time_throwing_makibishi"] > older->public_floats["current_time_throwing_makibishi"])
			{
				older2 = older1;
				older1 = older;
				older = current_script;
			}
		}
		if(num == 2)
			return older2->GetGameObject();
		else if (num == 1)
			return older1->GetGameObject();
		else
			return older->GetGameObject();
	}
}