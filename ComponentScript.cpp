#include "ComponentScript.h"
#include "Application.h"
#include "ModuleScripting.h"
#include "Color.h"

ComponentScript::ComponentScript(ComponentType type, GameObject* game_object, const char* path) : Component(type, game_object)
{
	this->path = path;
	started = false;
	finded_start = false;
	finded_update = false;
	script_num = 0;
}

ComponentScript::ComponentScript(ComponentType type, GameObject* game_object) : Component(type, game_object)
{
	path.clear();
	path.resize(50);
	started = false;
	finded_start = false;
	finded_update = false;
	script_num = 0;
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
			if (f_Start start = (f_Start)GetProcAddress(App->scripting->script, start_path.c_str()))
			{
				finded_start = true;
				if (App->IsGameRunning() && !App->IsGamePaused())
				{
					start(App, GetGameObject());
					started = true;
				}
			}
			else
			{
				finded_start = false;
				error = GetLastError();
			}
		}
		else
		{
			string update_path = path.c_str();
			update_path.append("::Update");
			if (f_Update update = (f_Update)GetProcAddress(App->scripting->script, update_path.c_str()))
			{
				finded_update = true;
				if (App->IsGameRunning() && !App->IsGamePaused())
				{
					update(App, GetGameObject());
				}
			}
			else
			{
				finded_update = false;
				error = GetLastError();
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

		ImGui::InputText("Path##PathScript", path._Myptr(), path.capacity());

		
		if (ImGui::Combo("Path##PathScript", &script_num, App->scripting->GetScriptNames().c_str(), 10))
		{
			path = App->scripting->GetScriptNamesList()[script_num];
		}
		
		if (App->scripting->scripts_loaded)
		{
			if (App->IsGameRunning() && !App->IsGamePaused())
			{
				if (!finded_start)
				{
					if (App->scripting->GetError() == 126)
						ImGui::TextColored(ImVec4(Yellow.r, Yellow.g, Yellow.b, Yellow.a), "Can't find Start");
					else
						ImGui::TextColored(ImVec4(Yellow.r, Yellow.g, Yellow.b, Yellow.a), "Unknown error loading Start");
				}
				else if (!finded_update)
				{
					if (App->scripting->GetError() == 126)
						ImGui::TextColored(ImVec4(Yellow.r, Yellow.g, Yellow.b, Yellow.a), "Can't find Update");
					else
						ImGui::TextColored(ImVec4(Yellow.r, Yellow.g, Yellow.b, Yellow.a), "Unknown error loading Update");
				}
			}
		}
		else
		{
			if(App->scripting->GetError() == 127)
				ImGui::TextColored(ImVec4(Yellow.r, Yellow.g, Yellow.b, Yellow.a), "Can't find Game.dll");
			else
				ImGui::TextColored(ImVec4(Yellow.r, Yellow.g, Yellow.b, Yellow.a), "Unknown error loading Game.dll");
		}
	}
}

void ComponentScript::Save(Data & file) const
{
	Data data;
	data.AppendInt("type", type);
	data.AppendUInt("UUID", uuid);
	data.AppendBool("active", active);
	data.AppendString("script_path", path.c_str());
	file.AppendArrayValue(data);
}

void ComponentScript::Load(Data & conf)
{
	uuid = conf.GetUInt("UUID");
	active = conf.GetBool("active");
	path = conf.GetString("path");
}

void ComponentScript::SetPath(const char * path)
{
	this->path.resize(50);
	this->path = path;
	started = false;
}
