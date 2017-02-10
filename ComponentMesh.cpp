#include "ComponentMesh.h"
#include "Application.h"
#include "MeshImporter.h"
#include "imgui\imgui.h"
#include "GameObject.h"
#include "ComponentTransform.h"
#include "ComponentMaterial.h"
#include "ComponentCamera.h"
#include "ResourceFileMesh.h"

ComponentMesh::ComponentMesh(ComponentType type, GameObject* game_object) : Component(type, game_object)
{
	aabb.SetNegativeInfinity();
	bounding_box.SetNegativeInfinity();
}

ComponentMesh::~ComponentMesh()
{
	if (rc_mesh)
	{
		rc_mesh->Unload();
		rc_mesh = nullptr;
	}
	mesh = nullptr;
}

void ComponentMesh::Update()
{
	//Component must be active to update
	if (!IsActive())
		return;
	if (mesh)
	{
		game_object->mesh_to_draw = mesh;

		App->renderer3D->AddToDraw(GetGameObject());
	}
}

void ComponentMesh::OnInspector()
{
	if (ImGui::CollapsingHeader("Geometry Mesh"))
	{
		//Active
		bool is_active = IsActive();
		if (ImGui::Checkbox("###activeMesh", &is_active))
		{
			SetActive(is_active);
		}

		if (mesh)
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
		}

		if (ImGui::Button("Remove ##mesh_rem"))
		{
			Remove();
		}
	}
}

void ComponentMesh::OnTransformModified()
{
	RecalculateBoundingBox();
}

bool ComponentMesh::SetMesh(Mesh * mesh)
{
	bool ret = false;
	if (mesh)
	{
		this->mesh = mesh;

		aabb.Enclose((float3*)mesh->vertices, mesh->num_vertices);
		RecalculateBoundingBox();
		game_object->mesh_to_draw = this->mesh;
		ret = true;
	}
		
	return ret;
}

void ComponentMesh::RecalculateBoundingBox()
{
	math::OBB ob = aabb.Transform(game_object->GetGlobalMatrix());
	bounding_box = ob.MinimalEnclosingAABB();
	game_object->bounding_box = &bounding_box;
}

void ComponentMesh::Save(Data & file)const
{
	Data data;
	data.AppendInt("type", type);
	data.AppendUInt("UUID", uuid);
	data.AppendBool("active", active);
	data.AppendString("path", mesh->file_path.data());

	file.AppendArrayValue(data);
}

void ComponentMesh::Load(Data & conf)
{
	uuid = conf.GetUInt("UUID");
	active = conf.GetBool("active");

	const char* path = conf.GetString("path");

	rc_mesh = (ResourceFileMesh*)App->resource_manager->LoadResource(path, ResourceFileType::RES_MESH);
	Mesh* mesh = rc_mesh->GetMesh(); 
	if(mesh)
		mesh->file_path = path;
	SetMesh(mesh);

	OnTransformModified();
}

const Mesh * ComponentMesh::GetMesh() const
{
	return mesh;
}
