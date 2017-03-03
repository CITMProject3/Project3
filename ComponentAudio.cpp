#include "ComponentAudio.h"

#include "ModuleAudio.h"
#include "Application.h"

#include "GameObject.h"

#include "imgui\imgui.h"

#include <string>

ComponentAudio::ComponentAudio(ComponentType type, GameObject* game_object) : Component(type, game_object)
{
	App->audio->RegisterGameObject(game_object->GetUUID());
}

ComponentAudio::~ComponentAudio()
{ 
	App->audio->UnregisterGameObject(game_object->GetUUID());
	if(current_event != nullptr) App->audio->UnloadSoundBank(current_event->parent_soundbank->path.c_str());
}

void ComponentAudio::Update()
{
}

void ComponentAudio::OnInspector(bool debug)
{
	std::string str = (std::string("Audio Source") + std::string("##") + std::to_string(uuid));
	if (ImGui::CollapsingHeader(str.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (ImGui::IsItemClicked(1))
		{
			ImGui::OpenPopup("delete##audio");
		}

		if (ImGui::BeginPopup("delete##audio"))
		{
			if (ImGui::MenuItem("Delete"))
			{
				Remove();
			}
			ImGui::EndPopup();
		}

		//Active
		bool is_active = IsActive();
		if (ImGui::Checkbox("###activeAudio", &is_active))
		{
			SetActive(is_active);
		}

		// Event selection
		ImGui::Text("Event: ");
		ImGui::SameLine();

		std::vector<AudioEvent*> events;
		App->audio->ObtainEvents(events);
		static std::string event_selected = current_event != nullptr ? current_event->name : "";
		
		if (ImGui::BeginMenu(event_selected.c_str()))
		{
			for (std::vector<AudioEvent*>::iterator it = events.begin(); it != events.end(); ++it)
			{
				if (ImGui::MenuItem((*it)->name.c_str()))
				{
					if (current_event != nullptr) App->resource_manager->UnloadResource(current_event->parent_soundbank->path);
					App->resource_manager->LoadResource((*it)->parent_soundbank->path, ResourceFileType::RES_SOUNDBANK);

					event_selected = (*it)->name; // Name to show on Inspector
					current_event = *it;		  // Variable that handles the new event
				}				
			}
			ImGui::EndMenu();			
		}
		
		if (ImGui::Button("PLAY") && current_event != nullptr)
			App->audio->PostEvent(current_event, game_object->GetUUID());
	}
}

void ComponentAudio::Save(Data & file)const
{
	/*Data data;
	data.AppendInt("type", type);
	data.AppendUInt("UUID", uuid);
	data.AppendBool("active", active);
	data.AppendString("path", mesh->file_path.data());

	file.AppendArrayValue(data);*/
}

void ComponentAudio::Load(Data & conf)
{
	/*uuid = conf.GetUInt("UUID");
	active = conf.GetBool("active");

	const char* path = conf.GetString("path");

	rc_mesh = (ResourceFileMesh*)App->resource_manager->LoadResource(path, ResourceFileType::RES_MESH);
	Mesh* mesh = rc_mesh->GetMesh();
	if (mesh)
		mesh->file_path = path;
	SetMesh(mesh);

	OnTransformModified();*/
}

void ComponentAudio::Remove()
{
	game_object->RemoveComponent(this);
}
