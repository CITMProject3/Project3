#include "ComponentAudio.h"

#include "GameObject.h"

#include "imgui\imgui.h"

#include <string>

ComponentAudio::ComponentAudio(ComponentType type, GameObject* game_object) : Component(type, game_object)
{ }

ComponentAudio::~ComponentAudio()
{ }

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

		/*if (mesh)
		{
			ImGui::Text("Number of vertices %d", mesh->num_vertices);
			ImGui::Text("Number of indices %d", mesh->num_indices);

			if (mesh->uvs != nullptr)
				ImGui::Text("Has UVs: yes");
			else
				ImGui::Text("Has UVs: no");

			if (mesh->normals != nullptr)
				ImGui::Text("Has Normals: yes");
			else
				ImGui::Text("Has Normals: no");

			if (mesh->colors != nullptr)
				ImGui::Text("Has Colors: yes");
			else
				ImGui::Text("Has Colors: no");

			ImGui::Text("Vertices id: %i", mesh->id_vertices);
			ImGui::Text("Indices id: %i", mesh->id_indices);
			ImGui::Text("UVs id: %i", mesh->id_uvs);
		}
		else
		{
			ImGui::TextColored(ImVec4(1, 0, 0, 1), "WARNING");
			ImGui::SameLine(); ImGui::Text("No mesh was loaded.");
		}*/
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
