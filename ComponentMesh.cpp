#include "ComponentMesh.h"
#include "Application.h"
#include "ModuleEditor.h"
#include "MeshImporter.h"
#include "imgui\imgui.h"
#include "GameObject.h"
#include "ComponentTransform.h"
#include "ComponentMaterial.h"
#include "ComponentCamera.h"
#include "ResourceFileMesh.h"
#include "ComponentBone.h"
#include "ResourceFileBone.h"

#include "glut/glut.h"

#include "ModuleRenderer3D.h"
#include "ModuleResourceManager.h"

#include "Brofiler/include/Brofiler.h"

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

	App->renderer3D->RemoveBuffer(weight_id);
	App->renderer3D->RemoveBuffer(bone_id);
}

void ComponentMesh::Update()
{
	BROFILER_CATEGORY("ComponentMesh::Update", Profiler::Color::AliceBlue)
	//Component must be active to update
	if (!IsActive())
		return;
	if (mesh)
	{
		game_object->mesh_to_draw = mesh;

		App->renderer3D->AddToDraw(GetGameObject());
	}
	if (App->renderer3D->renderAABBs)
	{
		App->renderer3D->DrawAABB(bounding_box.minPoint, bounding_box.maxPoint, float4(1, 1, 0, 1));
	}
}

void ComponentMesh::OnInspector(bool debug)
{
	string str = (string("Geometry Mesh") + string("##") + std::to_string(uuid));
	if (ImGui::CollapsingHeader(str.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (ImGui::IsItemClicked(1))
		{
			ImGui::OpenPopup("delete##mesh");
		}

		if (ImGui::BeginPopup("delete##mesh"))
		{
			if (ImGui::MenuItem("Delete"))
			{
				Remove();
			}
			ImGui::EndPopup();
		}

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
	}
}

void ComponentMesh::OnTransformModified()
{
	RecalculateBoundingBox();
}

bool ComponentMesh::SetMesh(Mesh *mesh)
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

void ComponentMesh::SetResourceMesh(ResourceFileMesh* resource)
{
	rc_mesh = resource;
	mesh = rc_mesh->GetMesh();
}

void ComponentMesh::RecalculateBoundingBox()
{
	math::OBB ob = aabb;
	ob.Transform(game_object->GetGlobalMatrix());
	
	bounding_box = ob.MinimalEnclosingAABB();
	game_object->bounding_box = &bounding_box;
}

void ComponentMesh::Save(Data & file)const
{
	Data data;
	data.AppendInt("type", type);
	data.AppendUInt("UUID", uuid);
	data.AppendBool("active", active);
	if (mesh)
		data.AppendString("path", mesh->file_path.data());
	else
		data.AppendString("path", "");

	file.AppendArrayValue(data);
}

void ComponentMesh::Load(Data & conf)
{
	uuid = conf.GetUInt("UUID");
	active = conf.GetBool("active");

	const char* path = conf.GetString("path");

	rc_mesh = (ResourceFileMesh*)App->resource_manager->LoadResource(path, ResourceFileType::RES_MESH);
	if (rc_mesh)
	{
		Mesh* mesh = rc_mesh->GetMesh();
		if (mesh)
		{
			mesh->file_path = path;
			SetMesh(mesh);

			OnTransformModified();
		}
	}
	else
	{
		LOG("[ERROR] Mesh path (%s) for %s cannot be loaded",path, game_object->name.data());
		App->editor->DisplayWarning(WarningType::W_ERROR, "Mesh path (%s) for %s cannot be loaded", path, game_object->name.data());
	}		
}

void ComponentMesh::Remove()
{
	game_object->RemoveComponent(this);

	ComponentMaterial* material = (ComponentMaterial*)game_object->GetComponent(C_MATERIAL);

	if (material != nullptr)
		material->Remove();
}

bool ComponentMesh::HasBones()
{
	return bones_reference.size() > 0;
}

void ComponentMesh::AddBone(ComponentBone* bone)
{
	for (uint i = 0; i < bones_reference.size(); i++)
		if (bones_reference[i].bone == bone)
			return;

	bones_reference.push_back(Bone_Reference(bone, bone->GetResource()->offset));

	if (bones_vertex.empty())
	{
		bones_vertex = std::vector<Bone_Vertex>(mesh->num_vertices);
	}

	ResourceFileBone* rBone = bone->GetResource();
	for (uint i = 0; i < rBone->numWeights; i++)
	{
		uint data_b_index = bones_reference.size() - 1;
		float data_b_float = rBone->weights[i];
		bones_vertex[rBone->weightsIndex[i]].AddBone(data_b_index, data_b_float);
	}
}

void ComponentMesh::DeformAnimMesh()
{
	BROFILER_CATEGORY("ComponentMesh::DeformAnimMesh", Profiler::Color::Maroon)

	bones_trans.clear();

	for (uint i = 0; i < bones_reference.size(); i++)
	{
		float4x4 matrix = bones_reference[i].bone->GetSystemTransform();
		matrix = game_object->transform->GetLocalTransformMatrix().Inverted() * matrix;
		float4x4 bone_trn_mat = matrix * bones_reference[i].offset;
		bones_trans.push_back(bone_trn_mat.Transposed());
	}
}
void ComponentMesh::InitAnimBuffers()
{
	if (mesh != nullptr)
	{
		int size = mesh->num_vertices * 4;
		float* weights = new float[size];
		int* bones_ids = new int[size];

		for (size_t i = 0; i < bones_vertex.size(); ++i)
		{
			int ver_id = i * 4;

			if (bones_vertex[i].weights.size() != bones_vertex[i].bone_index.size())
			{
				LOG("[WARNING] %s has different number of weights and index in the animation", game_object->name); //Just in case
				App->editor->DisplayWarning(WarningType::W_WARNING, "%s has different number of weights and index in the animation", game_object->name);
				return;
			}

			//Reset all to zero
			for (int w = 0; w < 4; ++w)
			{
				weights[ver_id + w] = 0;
				bones_ids[ver_id + w] = 0;
			}

			for (size_t w = 0; w < bones_vertex[i].weights.size(); ++w)
			{
				weights[ver_id + w] = bones_vertex[i].weights[w];
				bones_ids[ver_id + w] = bones_vertex[i].bone_index[w];
			}
		}

		MeshImporter::LoadAnimBuffers(weights, size, weight_id, bones_ids, size, bone_id);

		delete[] weights;
		delete[] bones_ids;

		animated = true;
	}
	else
	{
//		LOG("[WARNING] Trying to init animation buffers from '%s' without a mesh", game_object->name);
//		App->editor->DisplayWarning(WarningType::W_WARNING, "Trying to init animation buffers from '%s' without a mesh", game_object->name);
	}

}
