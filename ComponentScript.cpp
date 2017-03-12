#include "ComponentScript.h"

#include "Application.h"
#include "ModuleScripting.h"

#include <string>
#include "imgui\imgui.h"

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
					//// TODO: Maybe, the new event shares the same Soundbank...
					//// Unloading unused Soundbank.
					//if (current_event != nullptr) App->resource_manager->UnloadResource(current_event->parent_soundbank->path);
					//// Loading new bank: first Init bank if it has been not loaded and then, the other one
					//if (!App->audio->IsInitSoundbankLoaded())
					//	if (App->resource_manager->LoadResource(App->audio->GetInitLibrarySoundbankPath(), ResourceFileType::RES_SOUNDBANK) != nullptr)  // Init SB
					//		App->audio->InitSoundbankLoaded();
					//rc_audio = (ResourceFileAudio*)App->resource_manager->LoadResource((*it)->parent_soundbank->path, ResourceFileType::RES_SOUNDBANK);  // Other one SB

					// Loading Script...
					App->scripting->LoadScript((*it));

					script_selected = (*it)->GetName(); // Name to show on Inspector
					current_script = *it;		  // Variable that handles the new event
				}
			}
			ImGui::EndMenu();
		}
	}
}

void ComponentScript::Save(Data & file) const
{
	
}

void ComponentScript::Load(Data & conf)
{
	
}