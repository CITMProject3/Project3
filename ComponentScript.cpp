#include "ComponentScript.h"
#include "Application.h"
#include "ModuleScripting.h"

ComponentScript::ComponentScript(ComponentType type, GameObject* game_object, const char* path) : Component(type, game_object)
{
	this->path = path;
	started = false;
}

ComponentScript::ComponentScript(ComponentType type, GameObject* game_object) : Component(type, game_object)
{
	path.clear();
	path.resize(50);
	started = false;
}

ComponentScript::~ComponentScript()
{
}

void ComponentScript::Update()
{
	//Component must be active to update
	if (!IsActive())
		return;

	if (App->scripting->scripts_loaded)
	{
		if (!started)
		{
			string start_path = path.c_str();
			start_path.append("::Start");
			if (f_start start = (f_start)GetProcAddress(App->scripting->script, start_path.c_str()))
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
			string update_path = path.c_str();
			update_path.append("::Update");
			if (f_update update = (f_update)GetProcAddress(App->scripting->script, update_path.c_str()))
			{
				update(App, GetGameObject());
			}
			else
			{
				DWORD error = GetLastError();
			}
		}
	}
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
	this->path.resize(50);
	this->path = path;
	started = false;
}
