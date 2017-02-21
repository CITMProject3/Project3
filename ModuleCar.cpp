#include "Globals.h"
#include "Application.h"
#include "ModuleCar.h"

#include "ModuleResourceManager.h"
#include "ComponentTransform.h"
#include "Editor.h"
#include "Assets.h"
#include "GameObject.h"

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
	
	//App->resource_manager->LoadFile(file_selected->content_path, MESH);
	return true;
}

// Called every draw update
update_status ModuleCar::PreUpdate()
{
	if (loaded == false)
	{
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

		loaded = true;
	}

	return UPDATE_CONTINUE;
}

update_status ModuleCar::Update()
{
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