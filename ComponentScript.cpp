#include "ComponentScript.h"
#include "Application.h"
#include "ModuleScripting.h"

ComponentScript::ComponentScript(ComponentType type, GameObject* game_object, const char* path) : Component(type, game_object)
{
	this->path = path;
	App->scripting->LoadScriptLibrary(this->path.c_str(), &script);
	script = NULL;
	started = false;
}

ComponentScript::ComponentScript(ComponentType type, GameObject* game_object) : Component(type, game_object)
{
	path.clear();
	path.resize(50);
	script = NULL;
	started = false;
}

ComponentScript::~ComponentScript()
{
	if (script != NULL)
		App->scripting->FreeScriptLibrary(script);
}

void ComponentScript::Update()
{
	//Component must be active to update
	if (!IsActive())
		return;

	//if (App->file_system->Exists(path._Myptr()))
	//{
		if (script == NULL)
		{
			App->scripting->LoadScriptLibrary(path.c_str(), &script);
		}
		else
		{
			if (!started)
			{
				if (f_start start = (f_start)GetProcAddress(script, "Start"))
				{
					start(App, GetGameObject());
					started = true;
				}
				else
				{
					DWORD error = GetLastError();
				}
			}
			else
			{
				if (f_update update = (f_update)GetProcAddress(script, "Update"))
				{
					update(App, GetGameObject());
				}
				else
				{
					DWORD error = GetLastError();
				}
			}
		}
	//}
	//else
	//{
	//	if (script != NULL)
	//	{
	//		App->scripting->FreeScriptLibrary(script);
	//		script = NULL;
	//	}
	//	started = false;
	//}
}

void ComponentScript::OnInspector(bool debug)
{
	string str = (string("Script") + string("##") + std::to_string(uuid));
	if (ImGui::CollapsingHeader(str.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (ImGui::IsItemClicked(1))
		{
			ImGui::OpenPopup("delete##Script");
		}

		if (ImGui::BeginPopup("delete##Script"))
		{
			if (ImGui::MenuItem("Delete"))
			{
				Remove();
			}
			ImGui::EndPopup();
		}

		//Active
		bool is_active = IsActive();
		if (ImGui::Checkbox("###activeScript", &is_active))
		{
			SetActive(is_active);
		}

		const char* last_path = path.c_str();
		ImGui::InputText("Path##PathScript", path._Myptr(), path.capacity());
		//if (last_path != path)
		//	SetPath(path.c_str());
	}
}

void ComponentScript::Save(Data & file) const
{
}

void ComponentScript::Load(Data & conf)
{
}

void ComponentScript::SetPath(const char * path)
{
	if (script != NULL)
	{
		App->scripting->FreeScriptLibrary(script);
		script = NULL;
	}
	this->path.resize(50);
	this->path = path;
	started = false;
}
