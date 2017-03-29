#include "ComponentScript.h"
#include "Application.h"
#include "ModuleScripting.h"
#include "Color.h"
#include "GameObject.h"
#include "ModuleGOManager.h"
#include "PhysBody3D.h"

#include "imgui\imgui.h"

ComponentScript::ComponentScript(ComponentType type, GameObject* game_object, const char* path) : Component(type, game_object)
{
	SetPath(path);
	started = false;
	finded_start = false;
	finded_update = false;
	script_num = 0;
	filter = "";
	public_gos_to_set = false;
}

ComponentScript::ComponentScript(ComponentType type, GameObject* game_object) : Component(type, game_object)
{
	started = false;
	finded_start = false;
	finded_update = false;
	script_num = 0;
	filter = "";
	public_gos_to_set = false;
}

ComponentScript::~ComponentScript()
{
}

void ComponentScript::Update()
{
	if (public_gos_to_set)
	{
		if (!public_gos.empty())
		{
			for (map<const char*, GameObject*>::iterator it = public_gos.begin(); it != public_gos.end(); it++)
			{
				(*it).second = App->go_manager->FindGameObjectByUUID(App->go_manager->root, tmp_public_gos_uuint.find((*it).first)->second);
			}
			public_gos_to_set = false;
			tmp_public_gos_uuint.clear();
		}
	}

	//Component must be active to update
	if (!IsActive())
		return;	

	if (App->scripting->scripts_loaded)
	{
		if (!started)
		{
			string start_path = path.c_str();
			start_path.append("_Start");
			if (f_Start start = (f_Start)GetProcAddress(App->scripting->scripts_lib->lib, start_path.c_str()))
			{
				finded_start = true;
				if (App->IsGameRunning() && !App->IsGamePaused())
				{
					start(GetGameObject());
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
			update_path.append("_Update");
			if (f_Update update = (f_Update)GetProcAddress(App->scripting->scripts_lib->lib, update_path.c_str()))
			{
				string update_publics_path = path.c_str();
				update_publics_path.append("_UpdatePublics");
				if (f_Update update_publics = (f_Update)GetProcAddress(App->scripting->scripts_lib->lib, update_publics_path.c_str()))
				{
					update_publics(GetGameObject());
				}
				finded_update = true;
				if (App->IsGameRunning() && !App->IsGamePaused())
				{
					update(GetGameObject());
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
		
		if (App->scripting->scripts_lib && App->scripting->scripts_lib->finded_script_names)
		{
			ImGui::Text("Script name: %s", path.c_str());
			if (ImGui::Button("Set Script", ImVec2(80, 20)))
			{
				ImGui::OpenPopup("Select Script");
			}

			if (ImGui::BeginPopup("Select Script"))
			{

				ImGui::InputText("Filter", filter._Myptr(), 50);
				string f = filter.data();

				for (int x = 0; x < App->scripting->GetScriptNamesList().size(); x++)
				{
					string name = App->scripting->GetScriptNamesList()[x];
					if (f.empty() || (name.find(f.data()) != std::string::npos))
					{
						if (ImGui::Selectable(App->scripting->GetScriptNamesList()[x]))
						{
							SetPath(App->scripting->GetScriptNamesList()[x]);
						}
					}
				}

				ImGui::EndPopup();
			}


			if (IsActive() && App->IsGameRunning() && !App->IsGamePaused())
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
			ImGui::TextColored(ImVec4(Yellow.r, Yellow.g, Yellow.b, Yellow.a), "Unknown error loading Game.dll");
		}

		if (!public_chars.empty())
		{
			for (map<const char*, string>::iterator it = public_chars.begin(); it != public_chars.end(); it++)
			{
				ImGui::InputText((*it).first, (*it).second._Myptr(), (*it).second.size());
			}
		}
		if (!public_ints.empty())
		{
			for (map<const char*, int>::iterator it = public_ints.begin(); it != public_ints.end(); it++)
			{
				ImGui::InputInt((*it).first, &(*it).second);
			}
		}
		if (!public_floats.empty())
		{
			for (map<const char*, float>::iterator it = public_floats.begin(); it != public_floats.end(); it++)
			{
				ImGui::InputFloat((*it).first, &(*it).second);
			}
		}
		if (!public_bools.empty())
		{
			for (map<const char*, bool>::iterator it = public_bools.begin(); it != public_bools.end(); it++)
			{
				ImGui::Checkbox((*it).first, &(*it).second);
			}
		}
		if (!public_gos.empty())
		{
			for (map<const char*, GameObject*>::iterator it = public_gos.begin(); it != public_gos.end(); it++)
			{
				if (App->scripting->set_go_var_now != nullptr)
				{
					if (App->scripting->setting_go_var_name == (*it).first)
					{
						(*it).second = App->scripting->set_go_var_now;
						App->scripting->set_go_var_now = nullptr;
						App->scripting->setting_go_var = false;
					}
				}
				if ((*it).second != nullptr)
					ImGui::Text((*it).second->name.c_str());
				else
					ImGui::Text("NULL GO");
				ImGui::SameLine();
				if (ImGui::Button("Set GO", ImVec2(80, 20)))
				{
					(*it).second = nullptr;
					App->scripting->setting_go_var = true;
					App->scripting->setting_go_var_name = (*it).first;
				}
			}
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
	data.AppendInt("script_num", script_num);

	if (!public_chars.empty())
	{
		for (map<const char*, string>::const_iterator it = public_chars.begin(); it != public_chars.end(); it++)
		{
			data.AppendString((*it).first, (*it).second.c_str());
		}
	}
	if (!public_ints.empty())
	{
		for (map<const char*, int>::const_iterator it = public_ints.begin(); it != public_ints.end(); it++)
		{
			data.AppendInt((*it).first, (*it).second);
		}
	}
	if (!public_floats.empty())
	{
		for (map<const char*, float>::const_iterator it = public_floats.begin(); it != public_floats.end(); it++)
		{
			data.AppendFloat((*it).first, (*it).second);
		}
	}
	if (!public_bools.empty())
	{
		for (map<const char*, bool>::const_iterator it = public_bools.begin(); it != public_bools.end(); it++)
		{
			data.AppendBool((*it).first, (*it).second);
		}
	}
	if (!public_gos.empty())
	{
		for (map<const char*, GameObject*>::const_iterator it = public_gos.begin(); it != public_gos.end(); it++)
		{
			if((*it).second != nullptr)
				data.AppendUInt((*it).first, (*it).second->GetUUID());
		}
	}

	file.AppendArrayValue(data);
}

void ComponentScript::Load(Data & conf)
{
	uuid = conf.GetUInt("UUID");
	active = conf.GetBool("active");
	SetPath(conf.GetString("script_path"));
	script_num = conf.GetInt("script_num");
	
		if (!public_chars.empty())
		{
			for (map<const char*, string>::iterator it = public_chars.begin(); it != public_chars.end(); it++)
			{
				(*it).second = conf.GetString((*it).first);
			}
		}
		if (!public_ints.empty())
		{
			for (map<const char*, int>::iterator it = public_ints.begin(); it != public_ints.end(); it++)
			{
				(*it).second = conf.GetInt((*it).first);
			}
		}
		if (!public_floats.empty())
		{
			for (map<const char*, float>::iterator it = public_floats.begin(); it != public_floats.end(); it++)
			{
				(*it).second = conf.GetFloat((*it).first);
			}
		}
		if (!public_bools.empty())
		{
			for (map<const char*, bool>::iterator it = public_bools.begin(); it != public_bools.end(); it++)
			{
				(*it).second = conf.GetBool((*it).first);
			}
		}
		if (!public_gos.empty())
		{
			for (map<const char*, GameObject*>::iterator it = public_gos.begin(); it != public_gos.end(); it++)
			{
				tmp_public_gos_uuint.insert(pair<const char*, unsigned int>((*it).first, conf.GetUInt((*it).first)));
				public_gos_to_set = true;
			}
		}

	started = false;
}

void ComponentScript::SetPath(const char * path)
{
	this->path = path;
	started = false;

	if (!public_chars.empty())
		public_chars.clear();
	if (!public_ints.empty())
		public_ints.clear();
	if (!public_floats.empty())
		public_floats.clear();
	if (!public_bools.empty())
		public_bools.clear();
	if (!public_gos.empty())
		public_gos.clear();

	if (App->scripting->scripts_loaded)
	{
		App->scripting->GetPublics(path, &public_chars, &public_ints, &public_floats, &public_bools, &public_gos);
	}
}

void ComponentScript::OnCollision(PhysBody3D* col)
{
	if (App->scripting->scripts_loaded)
	{
		if (started)
		{
			string collision_path = path.c_str();
			collision_path.append("_OnCollision");
			if (f_OnCollision onCollision = (f_OnCollision)GetProcAddress(App->scripting->scripts_lib->lib, collision_path.c_str()))
			{
				if (App->IsGameRunning() && !App->IsGamePaused())
					onCollision(col);
			}
			else
			{
				error = GetLastError();
			}
		}
	}
}
