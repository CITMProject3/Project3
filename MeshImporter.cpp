#include "Application.h"
#include "ModuleFileSystem.h"
#include "MeshImporter.h"
#include "Globals.h"
#include "Assimp/include/cimport.h"
#include "Assimp/include/scene.h"
#include "Assimp/include/postprocess.h"
#include "Assimp/include/cfileio.h"
#pragma comment (lib, "Assimp/libx86/assimp.lib")

#include "Glew\include\glew.h"
#include <gl\GL.h>
#include <queue>
#include "ComponentTransform.h"
#include "GameObject.h"
#include "ComponentMesh.h"
#include "ComponentMaterial.h"
#include "TextureImporter.h"

bool MeshImporter::Import(const char * file, const char * path, const char* base_path)
{
	bool ret = false;
	char* buff;
	uint size = App->file_system->Load(path, &buff);

	if (size == 0)
	{
		LOG("Error loading %s", path);
		return ret;
	}

	const aiScene* scene = aiImportFileFromMemory(buff, size, aiProcessPreset_TargetRealtime_MaxQuality, NULL);

	if (scene != nullptr && scene->HasMeshes())
	{
		//Save mesh file as scene file
		Data root_node;
		root_node.AppendArray("GameObjects");

		string file_mesh_directory = path;
		file_mesh_directory = file_mesh_directory.substr(0, file_mesh_directory.find_last_of("\//") + 1);

		aiNode* root = scene->mRootNode;
		vector<GameObject*> objects_created;

		aiNode* tmp_node = root;

		MeshImporter::ImportNode(tmp_node, scene, NULL, file_mesh_directory, objects_created, base_path, root_node);

		for (vector<GameObject*>::iterator go = objects_created.begin(); go != objects_created.end(); ++go)
			delete (*go);

		objects_created.clear();

		char* buf;
		size_t size = root_node.Serialize(&buf);

		App->file_system->Save(file, buf, size);

		delete[] buf;

		aiReleaseImport(scene);

		ret = true;
	}
	else
	{
		LOG("Error loading scene %s. ERROR: %s", path, aiGetErrorString());
	}

	delete[] buff;

	return ret;
}

void MeshImporter::ImportNode(aiNode * node, const aiScene * scene, GameObject* parent, string mesh_file_directory, vector<GameObject*>& objects_created, string folder_path, Data& root_data_node)
{
	//Transformation ------------------------------------------------------------------------------------------------------------------
	GameObject* go_root = new GameObject(parent);
	objects_created.push_back(go_root);
	ComponentTransform* c_transform = (ComponentTransform*)go_root->GetComponent(C_TRANSFORM);

	aiVector3D translation;
	aiVector3D scaling;
	aiQuaternion rotation;

	node->mTransformation.Decompose(scaling, rotation, translation);

	math::float3 pos;
	pos.x = translation.x; pos.y = translation.y; pos.z = translation.z;

	math::Quat rot;
	rot.x = rotation.x; rot.y = rotation.y; rot.z = rotation.z; rot.w = rotation.w;

	math::float3 scale;
	scale.x = scaling.x; scale.y = scaling.y; scale.z = scaling.z;

	//Don't load fbx dummies as gameobjects. 
	static const char* dummies[5] =
	{
		"$AssimpFbx$_PreRotation",
		"$AssimpFbx$_Rotation",
		"$AssimpFbx$_PostRotation",
		"$AssimpFbx$_Scaling",
		"$AssimpFbx$_Translation"
	};

	for (int i = 0; i < 5; ++i)
	{
		if (((string)(node->mName.C_Str())).find(dummies[i]) != string::npos && node->mNumChildren == 1)
		{
			node = node->mChildren[0];
			node->mTransformation.Decompose(scaling, rotation, translation);
			pos += float3(translation.x, translation.y, translation.z);
			scale = float3(scale.x * scaling.x, scale.y * scaling.y, scale.z * scaling.z);
			rot = rot * Quat(rotation.x, rotation.y, rotation.z, rotation.w);
			i = -1;
		}
	}

	c_transform->SetPosition(pos);
	c_transform->SetRotation(rot);
	c_transform->SetScale(scale);

	c_transform->Update(); //Force it to update the matrix

	for (int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh_to_load = scene->mMeshes[node->mMeshes[i]];

		GameObject* game_object;
		ComponentTransform* go_transform;
		//Transform
		if (node->mNumMeshes > 1)
		{
			game_object = new GameObject(go_root);
			go_transform = (ComponentTransform*)game_object->GetComponent(C_TRANSFORM);
			objects_created.push_back(game_object);
		}
		else
		{
			game_object = go_root;
			go_transform = c_transform;
		}

		if (node->mName.length > 0)
			game_object->name = node->mName.C_Str();

		//Save GameObject basic info + Transform comopnent
		Data data;
		data.AppendString("name", game_object->name.data());
		data.AppendUInt("UUID", game_object->GetUUID());
		if (game_object->GetParent() == nullptr)
			data.AppendUInt("parent", 0);
		else
			data.AppendUInt("parent", game_object->GetParent()->GetUUID());
		data.AppendBool("active", true);
		data.AppendBool("static", false);
		data.AppendArray("components");

		go_transform->Save(data);

		//Mesh --------------------------------------------------------------------------------------------------------------------------------
		string mesh_path;
		bool ret = MeshImporter::ImportMesh(mesh_to_load, folder_path.data(), mesh_path); 
		Data mesh_data;
		mesh_data.AppendInt("type", ComponentType::C_MESH);
		mesh_data.AppendUInt("UUID", (unsigned int)App->rnd->RandomInt());
		mesh_data.AppendBool("active", true);
		mesh_data.AppendString("path", mesh_path.data());
		data.AppendArrayValue(mesh_data);

		//Load Textures --------------------------------------------------------------------------------------------------------------------

		aiMaterial* material = scene->mMaterials[mesh_to_load->mMaterialIndex];
		//uint numTextures = material->GetTextureCount(aiTextureType_DIFFUSE);
		if (material)
		{
			aiString path;
			material->GetTexture(aiTextureType_DIFFUSE, 0, &path);

			if (path.length > 0)
			{
				string complete_path = path.data; 
				
				size_t found = complete_path.find_first_of('\\');
				while (found != string::npos)
				{
					complete_path[found] = '/';
					found = complete_path.find_first_of('\\', found + 1);
				}
				complete_path = mesh_file_directory + complete_path;
				complete_path.erase(0, 1);


				//Normal map
				aiString normal_path;
				material->GetTexture(aiTextureType_HEIGHT, 0, &normal_path);
				string normal_complete_path;
				if (normal_path.length > 0)
				{
					normal_complete_path = normal_path.data;

					size_t found = normal_complete_path.find_first_of('\\');
					while (found != string::npos)
					{
						normal_complete_path[found] = '/';
						found = normal_complete_path.find_first_of('\\', found + 1);
					}
					normal_complete_path = mesh_file_directory + normal_complete_path;
					normal_complete_path.erase(0, 1);
				}



				Data tex_data;
				tex_data.AppendInt("type", ComponentType::C_MATERIAL);
				tex_data.AppendUInt("UUID", (unsigned int)App->rnd->RandomInt());
				tex_data.AppendBool("active", true);
				tex_data.AppendString("path", "");

				tex_data.AppendArray("textures");
				Data diffuse_data;
				diffuse_data.AppendString("path", App->resource_manager->FindFile(complete_path).data());
				tex_data.AppendArrayValue(diffuse_data);
				if (normal_path.length > 0)
				{
					Data normal_data;
					normal_data.AppendString("path", App->resource_manager->FindFile(normal_complete_path).data());
					tex_data.AppendArrayValue(normal_data);
				}

				data.AppendArrayValue(tex_data);
			}
			else
			{
				//Load default material without textures
				Data tex_data;
				tex_data.AppendInt("type", ComponentType::C_MATERIAL);
				tex_data.AppendUInt("UUID", (unsigned int)App->rnd->RandomInt());
				tex_data.AppendBool("active", true);
				tex_data.AppendString("path", "");

				tex_data.AppendArray("textures");
				data.AppendArrayValue(tex_data);
			}
		}
		

		root_data_node.AppendArrayValue(data);
	}

	for (int i = 0; i < node->mNumChildren; i++)
		MeshImporter::ImportNode(node->mChildren[i], scene, go_root, mesh_file_directory, objects_created, folder_path, root_data_node);
}

bool MeshImporter::ImportMesh(const aiMesh * mesh_to_load, const char* folder_path, string& output_name)
{
	//Mesh --------------------------------------------------------------------------------------------------------------------------------

	Mesh mesh;
	
	//Vertices ------------------------------------------------------------------------------------------------------
	mesh.num_vertices = mesh_to_load->mNumVertices;
	mesh.vertices = new float[mesh.num_vertices * 3];
	memcpy(mesh.vertices, mesh_to_load->mVertices, sizeof(float)*mesh.num_vertices * 3);
	

	//Indices --------------------------------------------------------------------------------------------------------
	if (mesh_to_load->HasFaces())
	{
		mesh.num_indices = mesh_to_load->mNumFaces * 3;
		mesh.indices = new uint[mesh.num_indices];
		for (uint f = 0; f < mesh_to_load->mNumFaces; f++)
		{
			if (mesh_to_load->mFaces[f].mNumIndices != 3)
			{
				LOG("WARNING: geometry with face != 3 indices is trying to be loaded");
			}
			else
			{
				memcpy(&mesh.indices[f * 3], mesh_to_load->mFaces[f].mIndices, 3 * sizeof(uint));
			}
		}
	}

	//Load UVs -----------------------------------------------------------------------------------------------------------------------
	if (mesh_to_load->HasTextureCoords(0))
	{
		mesh.num_uvs = mesh_to_load->mNumVertices; //Same size as vertices
		mesh.uvs = new float[mesh.num_uvs * 2];
		for (int uvs_item = 0; uvs_item < mesh.num_uvs; uvs_item++)
		{
			memcpy(&mesh.uvs[uvs_item * 2], &mesh_to_load->mTextureCoords[0][uvs_item].x, sizeof(float));
			memcpy(&mesh.uvs[(uvs_item * 2) + 1], &mesh_to_load->mTextureCoords[0][uvs_item].y, sizeof(float));
		}
	}

	//Normals ---------------------------------------------------------------------------------------------------------
	if (mesh_to_load->HasNormals())
	{
		mesh.normals = new float[mesh.num_vertices * 3];
		memcpy(mesh.normals, mesh_to_load->mNormals, sizeof(float) * mesh.num_vertices * 3);
	}

	//Colors --------------------------------------------------------------------------------------------------------
	if (mesh_to_load->HasVertexColors(0))
	{
		mesh.colors = new float[mesh.num_vertices * 3];
		memcpy(mesh.colors, mesh_to_load->mColors, sizeof(float) * mesh.num_vertices * 3);
	}

	//Tangents  --------------------------------------------------------------------------------------------------------
	if (mesh_to_load->HasTangentsAndBitangents())
	{
		mesh.tangents = new float[mesh.num_vertices * 3];
		memcpy(mesh.tangents, mesh_to_load->mTangents, sizeof(float)*mesh.num_vertices * 3);
	}
		
	return Save(mesh, folder_path, output_name);
}

bool MeshImporter::Save(Mesh& mesh, const char* folder_path, string& output_name)
{
	bool ret = false;

	//Indices - Vertices - Normals - Colors - UVs
	uint header[5] =
	{
		mesh.num_indices,
		mesh.num_vertices,
		(mesh.normals) ? mesh.num_vertices : 0,
		(mesh.colors) ? mesh.num_vertices : 0,
		mesh.num_uvs
	};

	uint size = sizeof(header) + sizeof(uint) * header[0] + sizeof(float) * header[1] * 3;
	if (header[2] != 0) size += sizeof(float) * header[2] * 3;
	if (header[3] != 0) size += sizeof(float) * header[3] * 3;
	size += sizeof(float) * header[4] * 2;
	size += sizeof(float) * header[1] * 3;

	char* data = new char[size];
	char* cursor = data;

	//Header
	uint bytes = sizeof(header);
	memcpy(cursor, header, bytes);

	cursor += bytes;

	//Indices
	bytes = sizeof(uint) * header[0];
	memcpy(cursor, mesh.indices, bytes);

	cursor += bytes;

	//Vertices
	bytes = sizeof(float) * header[1] * 3;
	memcpy(cursor, mesh.vertices, bytes);

	cursor += bytes;

	//Note: don't update bytes because for normals and colors is the same size as vertices

	//Normals
	if (header[2] != 0)
	{
		memcpy(cursor, mesh.normals, bytes);
		cursor += bytes;
	}

	//Colors
	if (header[3] != 0)
	{
		memcpy(cursor, mesh.colors, bytes);
		cursor += bytes;
	}

	//Uvs
	bytes = sizeof(float) * header[4] * 2;
	memcpy(cursor, mesh.uvs, bytes);
	cursor += bytes;

	//Tangents
	bytes = sizeof(float) * header[1] * 3;
	memcpy(cursor, mesh.tangents, bytes);
	cursor += bytes;

	//Generate random UUID for the name
	ret = App->file_system->SaveUnique(std::to_string((unsigned int)App->rnd->RandomInt()).data(), data, size, folder_path, "msh", output_name);

	delete[] data;
	data = nullptr;

	return ret;
}

Mesh * MeshImporter::Load(const char * path)
{
	Mesh* mesh = nullptr;
	char* buffer = nullptr;
	
	if (App->file_system->Load(path, &buffer) != 0)
	{
		mesh = new Mesh();

		mesh->file_path = path;

		char* cursor = buffer;

		uint header[5];
		uint bytes = sizeof(header);
		memcpy(header, cursor, bytes);

		mesh->num_indices = header[0];
		mesh->num_vertices = header[1];
		mesh->num_uvs = header[4];

		//Indices
		cursor += bytes;
		bytes = sizeof(uint) * mesh->num_indices;
		mesh->indices = new uint[mesh->num_indices];
		memcpy(mesh->indices, cursor, bytes);

		//Vertices
		cursor += bytes;
		bytes = sizeof(float) * mesh->num_vertices * 3;
		mesh->vertices = new float[mesh->num_vertices * 3];
		memcpy(mesh->vertices, cursor, bytes);

		//Normals
		cursor += bytes;
		if (header[2] != 0)
		{
			bytes = sizeof(float) * mesh->num_vertices * 3;
			mesh->normals = new float[mesh->num_vertices * 3];
			memcpy(mesh->normals, cursor, bytes);

			cursor += bytes;
		}

		//Colors
		if (header[3] != 0)
		{
			bytes = sizeof(float) * mesh->num_vertices * 3;
			mesh->colors = new float[mesh->num_vertices * 3];
			memcpy(mesh->colors, cursor, bytes);

			cursor += bytes;
		}

		//Uvs
		bytes = sizeof(float) * mesh->num_uvs * 2;
		mesh->uvs = new float[mesh->num_uvs * 2];
		memcpy(mesh->uvs, cursor, bytes);
		cursor += bytes;

		//Tangents
		bytes = sizeof(float) * mesh->num_vertices * 3;
		mesh->tangents = new float[mesh->num_vertices * 3];
		memcpy(mesh->tangents, cursor, bytes);
		cursor += bytes;

		//Vertices ------------------------------------------------------------------------------------------------------
		
		//Load buffer to VRAM
		glGenBuffers(1, (GLuint*)&(mesh->id_vertices));
		glBindBuffer(GL_ARRAY_BUFFER, mesh->id_vertices);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * mesh->num_vertices, mesh->vertices, GL_STATIC_DRAW);

		//Indices --------------------------------------------------------------------------------------------------------

		//Load indices buffer to VRAM
		glGenBuffers(1, (GLuint*) &(mesh->id_indices));
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->id_indices);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * mesh->num_indices, mesh->indices, GL_STATIC_DRAW);

		//Load UVs -----------------------------------------------------------------------------------------------------------------------

		glGenBuffers(1, (GLuint*)&(mesh->id_uvs));
		glBindBuffer(GL_ARRAY_BUFFER, mesh->id_uvs);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * mesh->num_uvs, mesh->uvs, GL_STATIC_DRAW);

		//Load Normals -----------------------------------------------------------------------------------------------------------------------

		if (mesh->normals)
		{
			glGenBuffers(1, (GLuint*)&(mesh->id_normals));
			glBindBuffer(GL_ARRAY_BUFFER, mesh->id_normals);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * mesh->num_vertices, mesh->normals, GL_STATIC_DRAW);
		}

		//Load Tangents-----------------------------------------------------------------------------------------------------------------------
		glGenBuffers(1, (GLuint*)&(mesh->id_tangents));
		glBindBuffer(GL_ARRAY_BUFFER, mesh->id_tangents);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * mesh->num_vertices, mesh->tangents, GL_STATIC_DRAW);
	}
	if(buffer)
		delete[] buffer;
	buffer = nullptr;

	return mesh;
}
