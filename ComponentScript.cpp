#include "ComponentScript.h"

#include "Application.h"
#include "ModuleScripting.h"

#include <string>
#include "imgui\imgui.h"

#include "MonoScripts.h"
#include "ClassInfo.h"
#include "FieldInfo.h"

typedef struct _MonoString MonoString;

ComponentScript::ComponentScript(ComponentType type, GameObject* game_object) : Component(type, game_object)
{ }

ComponentScript::~ComponentScript()
{
}

void ComponentScript::Update()
{
	//Component must be active to update
	if (!IsActive())
		return;
}

void ComponentScript::OnInspector(bool debug)
{
	std::string str = (std::string("Script") + std::string("##") + std::to_string(uuid));
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


		// Event selection
		ImGui::Text("Script: ");
		ImGui::SameLine();

		std::vector<ClassInfo*> scripts;
		App->scripting->ObtainScripts(scripts);

		if (ImGui::BeginMenu(script_selected.c_str()))
		{
			for (std::vector<ClassInfo*>::iterator it = scripts.begin(); it != scripts.end(); ++it)
			{
				if (ImGui::MenuItem((*it)->GetName()))
				{
					// Loading Script...
					App->scripting->LoadScript((*it));

					script_selected = (*it)->GetName(); // Name to show on Inspector
					current_script = *it;		  // Variable that handles the new event
				}
			}
			ImGui::EndMenu();
		}

		ImGui::Text("Variables: ");

		if (current_script != nullptr)
		{
			for (std::vector<FieldInfo*>::const_iterator it = current_script->fields.begin(); it != current_script->fields.end(); ++it)
			{
				ShowVariable((*it));
			}
		}
	}
}

void ComponentScript::Save(Data & file) const
{
	
}

void ComponentScript::Load(Data & conf)
{ }

void ComponentScript::ShowVariable(FieldInfo *var)
{
	switch (var->GetType())
	{
		case(MonoDataType::MONO_DATA_TYPE_INT16):
		case(MonoDataType::MONO_DATA_TYPE_INT32):
		case(MonoDataType::MONO_DATA_TYPE_INT64):
		{
			int value = 0; var->GetValue(&value);
			if (ImGui::DragInt(var->GetName(), &value))
				var->SetValue(&value);
			break;
		}
		case(MonoDataType::MONO_DATA_TYPE_FLOAT):
		case(MonoDataType::MONO_DATA_TYPE_DOUBLE):
		{
			float value = 0.0f; var->GetValue(&value);
			ImGui::DragFloat(var->GetName(), &value);
			break;
		}
	}
}