#include "Application.h"
#include "ModuleSceneIntro.h"
#include "GameObject.h"
#include "Glew\include\glew.h"
#include <gl\GL.h>
#include "Component.h"
#include "ModuleRenderer3D.h"
#include "ComponentTransform.h"
#include "ComponentMesh.h"
#include "ComponentCamera.h"
#include "Random.h"
#include "GameObject.h"
#include "ModuleGOManager.h"
#include "ModuleInput.h"

#include "SDL/include/SDL_scancode.h"

ModuleSceneIntro::ModuleSceneIntro(const char* name, bool start_enabled) : Module(name, start_enabled)
{
}

ModuleSceneIntro::~ModuleSceneIntro()
{}

bool ModuleSceneIntro::Init(Data& config)
{
	return true;
}

// Load assets
bool ModuleSceneIntro::Start()
{
	LOG("Loading Intro assets");
	bool ret = true;
	

	return ret;
}

// Load assets
bool ModuleSceneIntro::CleanUp()
{
	LOG("Unloading Intro scene");

	return true;
}

GameObject * ModuleSceneIntro::FindGameObject(GameObject * start, const string & name) const
{
	if (start->name.compare(name.data()) == 0)
		return start;

	const vector<GameObject*>* childs = start->GetChilds();
	for (size_t i = 0; i < childs->size(); i++)
	{
		GameObject* ret = FindGameObject(childs->at(i), name);
		if (ret != nullptr)
			return ret;
	}

	return nullptr;
}

// Update
update_status ModuleSceneIntro::Update()
{
	if (water_hack_enabled)
	{
		if (reflection_tra == nullptr)
		{
			GameObject* camref = FindGameObject(App->go_manager->root, "ReflectionCam");
			GameObject* cammain = FindGameObject(App->go_manager->root, "MainCamera");
			if (camref && cammain)
			{
				reflection_tra = camref->transform;
				maincam = cammain->transform;
			}
		}
		else
		{
			float3 position = maincam->GetPosition();
			position.x = 0;
			position.z = 0;
			position.y = -(position.y - (-5.0f)) * 2.0f;
			reflection_tra->SetPosition(position);

			float3 rotation = maincam->GetRotationEuler();
			rotation.x = -rotation.x * 2.0f;
			rotation.y = 0;
			rotation.z = 0;
			reflection_tra->SetRotation(rotation);
		}
	}

	if (App->input->GetKey(SDL_SCANCODE_F2) == KEY_UP)
		water_hack_enabled = !water_hack_enabled;
	

	return UPDATE_CONTINUE;
}

