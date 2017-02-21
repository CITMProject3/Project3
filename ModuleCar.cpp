#include "Globals.h"
#include "Application.h"
#include "ModuleCar.h"

#include "ModuleResourceManager.h"
#include "ModuleGOManager.h"
#include "ComponentTransform.h"
#include "Editor.h"
#include "Assets.h"
#include "GameObject.h"
#include "RaycastHit.h"

#include "ModuleInput.h"

#include "glmath.h"

ModuleCar::ModuleCar(const char* name, bool start_enabled) : Module(name, start_enabled)
{
}

// Destructor
ModuleCar::~ModuleCar()
{
}

// Called before render is available
bool ModuleCar::Init(Data& config)
{
	bool ret = true;
	return ret;
}

bool ModuleCar::Start()
{
	return true;
}

// Called every draw update
update_status ModuleCar::PreUpdate()
{
	if (loaded == false)
	{
#pragma region loading_the_kart
		AssetFile* kartFile = App->editor->assets->FindAssetFile("/Assets/kart.fbx");
		if (kartFile != nullptr)
		{
			App->resource_manager->LoadFile(kartFile->content_path, FileType::MESH);

			const vector<GameObject*>* rootChilds = App->go_manager->root->GetChilds();
			for (vector<GameObject*>::const_iterator it = rootChilds->cbegin(); it != rootChilds->cend(); it++)
			{
				if ((*it)->name == "pivot")
				{
					kart = *it;
				}
			}
		}

		if (kart != nullptr)
		{
			kart_trs = (ComponentTransform*)kart->GetComponent(ComponentType::C_TRANSFORM);

			const vector<GameObject*>* childs = kart->GetChilds();
			for (vector<GameObject*>::const_iterator it = childs->cbegin(); it != childs->cend(); it++)
			{
				if ((*it)->name == "chasis")
				{
					chasis = *it;
				}
				else if ((*it)->name == "camera")
				{
					cam = *it;
				}
			}
		}

		if (chasis != nullptr)
		{
			const vector<GameObject*>* childs = chasis->GetChilds();
			for (vector<GameObject*>::const_iterator it = childs->cbegin(); it != childs->cend(); it++)
			{
				if ((*it)->name == "F_wheel")
				{
					frontWheel = *it;
				}
				else if ((*it)->name == "B_wheel")
				{
					backWheel = *it;
				}
			}
		}
#pragma endregion

#pragma region loading_the_test_track
		AssetFile* trackFile = App->editor->assets->FindAssetFile("/Assets/track_test.fbx");
		if (trackFile != nullptr)
		{
			App->resource_manager->LoadFile(trackFile->content_path, FileType::MESH);

			const vector<GameObject*>* rootChilds = App->go_manager->root->GetChilds();
			for (vector<GameObject*>::const_iterator it = rootChilds->cbegin(); it != rootChilds->cend(); it++)
			{
				if ((*it)->name == "track_test")
				{
					track = *it;
				}
			}
		}
#pragma endregion

		loaded = true;
	}

	return UPDATE_CONTINUE;
}

update_status ModuleCar::Update()
{
	math::Ray ray;
	//ray.dir = -kart_trs->GetGlobalMatrix().WorldY();
	ray.dir = float3(0, -1, 0);
	ray.pos = kart_trs->GetPosition();
	ray.pos -= kart_trs->GetGlobalMatrix().WorldY();

	RaycastHit hit;
	App->go_manager->Raycast(ray, &hit);

	if (hit.object != nullptr && hit.distance < 5)
	{
		LOG("\nHit object: %s\nNormal: %f, %f, %f", hit.object->name.data(), hit.point.x, hit.point.y, hit.point.z);
		Quat normal_rot = Quat::RotateFromTo(kart_trs->GetRotation().WorldY(), hit.normal);
		//kart_trs->SetRotation(/*kart_trs->GetRotation() * */normal_rot);
	}
	else
	{
		if (hit.distance > 5) { LOG("\nToo far for hit"); }
		else { LOG("\nHit no object"); }
		Quat normal_rot = Quat::RotateFromTo(kart_trs->GetRotation().WorldY(), float3(0,1,0));
		//kart_trs->SetRotation(/*kart_trs->GetRotation() * */normal_rot);
	}

	float3 pos = kart_trs->GetPosition();
	float3 newPos = pos;
	if (App->input->GetKey(SDL_SCANCODE_W) == KEY_REPEAT)
	{		
		newPos += kart_trs->GetGlobalMatrix().WorldZ() * speed * time->DeltaTime();
	}

	if (App->input->GetKey(SDL_SCANCODE_S) == KEY_REPEAT)
	{
		newPos -= kart_trs->GetGlobalMatrix().WorldZ() * speed * time->DeltaTime();
	}

	if (App->input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT)
	{
		Quat tmp = kart_trs->GetRotation().RotateAxisAngle(float3(0, 1, 0), -rotateSpeed * DEGTORAD * time->DeltaTime());
		kart_trs->SetRotation(kart_trs->GetRotation() * tmp);
	}

	if (App->input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT)
	{
		Quat tmp = kart_trs->GetRotation().RotateAxisAngle(float3(0, 1, 0), rotateSpeed * DEGTORAD * time->DeltaTime());
		kart_trs->SetRotation(kart_trs->GetRotation() * tmp);
	}

	kart_trs->SetPosition(newPos);

	return UPDATE_CONTINUE;
}

update_status ModuleCar::PostUpdate()
{
	return UPDATE_CONTINUE;
}

// Called before quitting
bool ModuleCar::CleanUp()
{
	return true;
}