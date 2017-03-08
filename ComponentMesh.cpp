#include "ComponentMesh.h"
#include "Application.h"
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

void ComponentMesh::Update(float dt)
{
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
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		App->renderer3D->DrawAABB(bounding_box.minPoint, bounding_box.maxPoint, float4(1, 1, 0, 1));
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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
		LOG("The go %s component mesh, can't find the path %s to load", game_object->name.data(), path);
	}
		
}

void ComponentMesh::Remove()
{
	game_object->RemoveComponent(this);

	ComponentMaterial* material = (ComponentMaterial*)game_object->GetComponent(C_MATERIAL);

	if (material != nullptr)
		material->Remove();
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

void ComponentMesh::ResetDeformable()
{
	if (deformable == nullptr)
	{
		deformable = new Mesh();
		deformable->vertices = new float[mesh->num_vertices * 3];
		deformable->normals = new float[mesh->num_vertices * 3];
	}

	memset(deformable->vertices, 0, mesh->num_vertices * sizeof(float) * 3);
	if (mesh->normals != nullptr)
	{
		memset(deformable->normals, 0, mesh->num_vertices * sizeof(float) * 3);
	}
}

void ComponentMesh::DeformAnimMesh()
{
	ResetDeformable();
	UpdateBonesData();

	//Looping through vertices
	for (uint i = 0; i < bones_vertex.size(); i++)
	{
		//Looping through bones afecting current vertex
		for (uint j = 0; j < bones_vertex[i].bone_index.size(); j++)
		{
			uint index = bones_vertex[i].bone_index[j];
			float4x4 matrix = bones_reference[index].transform;

			//Vertex position
			float3 originalV(&mesh->vertices[i * 3]);
			float4x4 m_offset = matrix * bones_reference[index].offset;
			float3 toAdd = m_offset.TransformPos(originalV);

			deformable->vertices[i * 3] += toAdd.x * bones_vertex[i].weights[j];
			deformable->vertices[i * 3 + 1] += toAdd.y * bones_vertex[i].weights[j];
			deformable->vertices[i * 3 + 2] += toAdd.z * bones_vertex[i].weights[j];

			if (mesh->normals != nullptr)
			{
				float3 originalN(&mesh->normals[i * 3]);
				float3 toAddN = matrix.TransformPos(originalN);
				deformable->normals[i * 3] += toAdd.x * bones_vertex[i].weights[j];
				deformable->normals[i * 3 + 1] += toAdd.y * bones_vertex[i].weights[j];
				deformable->normals[i * 3 + 2] += toAdd.z * bones_vertex[i].weights[j];
			}

		}
	}

	for (uint i = 0; i < bones_reference.size(); i++)
	{
		ResourceFileBone* rBone = bones_reference[i].bone->GetResource();
		ComponentBone* rootBone = bones_reference[i].bone->GetRoot();

		float4x4 matrix = bones_reference[i].bone->GetSystemTransform();
		matrix = ((ComponentTransform*)game_object->GetComponent(C_TRANSFORM))->GetLocalTransformMatrix().Inverted() * matrix;

		float4x4 matrix_normals = matrix;
		matrix = matrix * rBone->offset;
		
		for (uint i = 0; i < rBone->numWeights; i++)
		{
			uint index = rBone->weightsIndex[i];
			float3 originalV(&mesh->vertices[index * 3]);
			float3 toAdd = matrix.TransformPos(originalV);

			deformable->vertices[index * 3] += toAdd.x  * rBone->weights[i];
			deformable->vertices[index * 3 + 1] += toAdd.y * rBone->weights[i];
			deformable->vertices[index * 3 + 2] += toAdd.z * rBone->weights[i];

			if (mesh->normals != nullptr)
			{
				float3 originalVN(&mesh->normals[index * 3]);
				toAdd = matrix_normals.TransformPos(originalVN);
				deformable->normals[index * 3] += toAdd.x * rBone->weights[i];
				deformable->normals[index * 3 + 1] += toAdd.y * rBone->weights[i];
				deformable->normals[index * 3 + 2] += toAdd.z * rBone->weights[i];
			}
		}
	}
}

void ComponentMesh::UpdateBonesData()
{
	for (uint i = 0; i < bones_reference.size(); i++)
	{
		float4x4 matrix = bones_reference[i].bone->GetSystemTransform();
		matrix = ((ComponentTransform*)game_object->GetComponent(C_TRANSFORM))->GetLocalTransformMatrix().Inverted() * matrix;
		bones_reference[i].transform = matrix;
	}
	
	/*
	//TODO: optimization, for now we update the same bone more than once
	ComponentTransform* transform = (ComponentTransform*)game_object->GetComponent(C_TRANSFORM);
	for (uint i = 0; i < bones_vertex.size(); i++)
	{
		for (uint j = 0; j = bones_vertex[i].bone_index.size(); i++)
		{
			uint index = bones_vertex[i].bone_index[j];
			float4x4 matrix = bones[index]->GetSystemTransform();
			matrix = transform->GetLocalTransformMatrix().Inverted() * matrix;

			bones_vertex[i].transforms[j] = matrix;
		}
	}
	*/
}
