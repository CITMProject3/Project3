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

ResourceFileMesh* ComponentMesh::GetResource() const
{
	return rc_mesh;
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
	for (uint i = 0; i < bones.size(); i++)
		if (bones[i] == bone)
			return;

	bones.push_back(bone);
}

void ComponentMesh::StartBoneDeformation()
{
	if (deformable == nullptr)
	{
		deformable = new Mesh();

		deformable->num_vertices = mesh->num_vertices;

		deformable->vertices = new float[deformable->num_vertices * 3];
		deformable->normals = new float[deformable->num_vertices * 3];
	}
	memset(deformable->vertices, 0, deformable->num_vertices * sizeof(float) * 3);

	if (mesh->normals != nullptr)
	{
		memset(deformable->normals, 0, deformable->num_vertices * sizeof(float) * 3);
	}
}

void ComponentMesh::DeformAnimMesh()
{
	//Just for security
	if (deformable == nullptr)
		StartBoneDeformation();

	for (uint i = 0; i < bones.size(); i++)
	{
		ResourceFileBone* rBone = bones[i]->GetResource();
		ComponentBone* rootBone = bones[i]->GetRoot();

		float4x4 matrix = bones[i]->GetSystemTransform();
		matrix = ((ComponentTransform*)game_object->GetComponent(C_TRANSFORM))->GetLocalTransformMatrix().Inverted() * matrix;

		float4x4 matrixN = matrix;
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
				toAdd = matrixN.TransformPos(originalVN);
				deformable->normals[index * 3] += toAdd.x * rBone->weights[i];
				deformable->normals[index * 3 + 1] += toAdd.y * rBone->weights[i];
				deformable->normals[index * 3 + 2] += toAdd.z * rBone->weights[i];
			}
		}
	}
}
