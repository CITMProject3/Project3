#include "Application.h"
#include "ModuleFileSystem.h"
#include "ModuleEditor.h"
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
#include "AnimationImporter.h"
#include "ComponentAnimation.h"
#include "ComponentBone.h"
#include "ModuleResourceManager.h"
#include "ModuleRenderer3D.h"
#include "Random.h"

bool MeshImporter::Import(const char * file, const char * path, const char* base_path, vector<unsigned int>& uuids, vector<unsigned int>& uuids_anim, vector<unsigned int>& uuids_bones)
{
	bool ret = false;
	char* buff;
	uint size = App->file_system->Load(path, &buff);

	if (size == 0)
	{
		LOG("[ERROR] Loading mesh %s has failed", path);
		App->editor->DisplayWarning(WarningType::W_ERROR, "Loading mesh %s has failed", path);
		return ret;
	}

	const aiScene* scene = aiImportFileFromMemory(buff, size, aiProcessPreset_TargetRealtime_MaxQuality, nullptr);

	if (scene != nullptr && scene->HasMeshes())
	{
		string file_mesh_directory = path;
		file_mesh_directory = file_mesh_directory.substr(0, file_mesh_directory.find_last_of("\//") + 1);

		aiNode* root = scene->mRootNode;
		vector<GameObject*> objects_created;
		vector<const aiMesh*> boned_meshes;
		vector<const GameObject*> boned_game_objects;

 		MeshImporter::ImportNode(root, scene, nullptr, objects_created, boned_meshes, boned_game_objects, file_mesh_directory, base_path, uuids);

		//Renaming RootNode
		if (objects_created[0]->name == "RootNode")
		{
			std::string name = path;
			uint slashPos;
			if ((slashPos = name.find_last_of("/")) != std::string::npos)
				name = name.substr(slashPos + 1, name.size() - slashPos);

			uint pointPos;
			if ((pointPos = name.find_first_of(".")) != std::string::npos)
				name = name.substr(0, name.size() - (name.size() - pointPos));
			objects_created[0]->name = name;
		}

		//Importing animations ----------
		std::string output_animation;
		while (uuids_anim.size() < scene->mNumAnimations)
			uuids_anim.push_back(0);

		if (AnimationImporter::ImportSceneAnimations(scene, objects_created[0], base_path, output_animation, uuids_anim))
		{
			ComponentAnimation* animation = (ComponentAnimation*)objects_created[0]->AddComponent(C_ANIMATION);
			animation->SetResource((ResourceFileAnimation*)App->resource_manager->LoadResource(output_animation, RES_ANIMATION));
		}
		//-------------------------------
		//Importing bones ---------------
		AnimationImporter::ImportSceneBones(boned_meshes, boned_game_objects, objects_created[0], base_path, uuids_bones);
		//-------------------------------

		SaveInfoFile(objects_created, file);

		for (vector<GameObject*>::iterator go = objects_created.begin(); go != objects_created.end(); ++go)
			delete (*go);
		objects_created.clear();
		aiReleaseImport(scene);

		ret = true;
	}
	else
	{
		LOG("[ERROR] Loading scene %s due to %s", path, aiGetErrorString());
		App->editor->DisplayWarning(WarningType::W_ERROR, "Loading scene %s due to %s", path, aiGetErrorString());
	}

	delete[] buff;

	return ret;
}

void MeshImporter::ImportNode(aiNode * node, const aiScene * scene, GameObject* parent, std::vector<GameObject*>& created_go, std::vector<const aiMesh*>& boned_meshes,
	std::vector<const GameObject*>& boned_game_objects, string mesh_file_directory, string folder_path, vector<unsigned int>& uuids)
{
	//Transformation ------------------------------------------------------------------------------------------------------------------
	aiVector3D translation;
	aiVector3D scaling;
	aiQuaternion rotation;

	node->mTransformation.Decompose(scaling, rotation, translation);

	float3 pos(translation.x, translation.y, translation.z);
	Quat rot(rotation.x, rotation.y, rotation.z, rotation.w);
	float3 scale(scaling.x, scaling.y, scaling.z);

	//Don't load fbx dummies as gameobjects. 
	static const char* dummies[5] =
	{
		"$AssimpFbx$_PreRotation",
		"$AssimpFbx$_Rotation",
		"$AssimpFbx$_PostRotation",
		"$AssimpFbx$_Scaling",
		"$AssimpFbx$_Translation"
	};

	std::string name = node->mName.C_Str();
	for (int i = 0; i < 5; ++i)
	{
		if (name.find(dummies[i]) != string::npos && node->mNumChildren == 1)
		{
			node = node->mChildren[0];
			node->mTransformation.Decompose(scaling, rotation, translation);
			pos += float3(translation.x, translation.y, translation.z);
			scale = float3(scale.x * scaling.x, scale.y * scaling.y, scale.z * scaling.z);
			rot = rot * Quat(rotation.x, rotation.y, rotation.z, rotation.w);
			name = node->mName.C_Str();
			i = -1;
		}
	}

	GameObject* go_root = new GameObject(parent);
	if (parent)
		parent->AddChild(go_root);

	created_go.push_back(go_root);

	go_root->name = name;

	go_root->transform->SetPosition(pos);
	go_root->transform->SetRotation(rot);
	go_root->transform->SetScale(scale);

	go_root->transform->Update(); //Force it to update the matrix

	if (node->mName.length > 0)
		go_root->name = node->mName.C_Str();

	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		GameObject* child = nullptr;

		if (node->mNumMeshes > 1)
		{
			child = new GameObject(go_root);
			go_root->AddChild(child);

			created_go.push_back(child);
		}
		else
		{
			child = go_root;
		}

		if (scene->mMeshes[node->mMeshes[i]]->HasBones())
		{
			boned_meshes.push_back(scene->mMeshes[node->mMeshes[i]]);
			boned_game_objects.push_back(child);
		}

		if (node->mName.length > 0)
			child->name = node->mName.C_Str();

		aiMesh* mesh_to_load = scene->mMeshes[node->mMeshes[i]];

		//Mesh --------------------------------------------------------------------------------------------------------------------------------
		string mesh_path;

		unsigned int msh_uuid = 0;
		bool ret = MeshImporter::ImportMesh(mesh_to_load, folder_path.data(), mesh_path, msh_uuid); 
		ComponentMesh* mesh = (ComponentMesh*)child->AddComponent(C_MESH);
		mesh->SetResourceMesh((ResourceFileMesh*)App->resource_manager->LoadResource(mesh_path, RES_MESH));
		uuids.push_back(msh_uuid);


		//Load Textures --------------------------------------------------------------------------------------------------------------------
		aiMaterial* material = scene->mMaterials[mesh_to_load->mMaterialIndex];
		//uint numTextures = material->GetTextureCount(aiTextureType_DIFFUSE);
		if (material)
		{

			aiString path;
			material->GetTexture(aiTextureType_DIFFUSE, 0, &path);
			ComponentMaterial* c_material = (ComponentMaterial*)child->AddComponent(C_MATERIAL);
			
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
				material->GetTexture(aiTextureType_NORMALS, 0, &normal_path);
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

				c_material->list_textures_paths[0] = complete_path;
				if (normal_path.length > 0)
					c_material->list_textures_paths[1] = normal_complete_path;
			}	
		}
	}
	for (unsigned int i = 0; i < node->mNumChildren; i++)
		MeshImporter::ImportNode(node->mChildren[i], scene, go_root, created_go, boned_meshes, boned_game_objects, mesh_file_directory, folder_path, uuids);
}

bool MeshImporter::ImportMesh(const aiMesh * mesh_to_load, const char* folder_path, string& output_name, unsigned int& msh_uuid)
{
	//Mesh --------------------------------------------------------------------------------------------------------------------------------

	Mesh mesh;
	
	//Vertices ------------------------------------------------------------------------------------------------------
	mesh.num_vertices = mesh_to_load->mNumVertices;
	mesh.vertices = new float[mesh.num_vertices * 3];
	memcpy(mesh.vertices, mesh_to_load->mVertices, sizeof(float3) * mesh.num_vertices);
	

	//Indices --------------------------------------------------------------------------------------------------------
	if (mesh_to_load->HasFaces())
	{
		mesh.num_indices = mesh_to_load->mNumFaces * 3;
		mesh.indices = new uint[mesh.num_indices];
		for (uint f = 0; f < mesh_to_load->mNumFaces; f++)
		{
			if (mesh_to_load->mFaces[f].mNumIndices != 3)
			{
				LOG("[WARNING] Geometry with face != 3 indices is trying to be loaded");
				App->editor->DisplayWarning(WarningType::W_WARNING, "Geometry with face != 3 indices is trying to be loaded");
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
		for (unsigned int uvs_item = 0; uvs_item < mesh.num_uvs; uvs_item++)
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
	//	mesh.colors = new float[mesh.num_vertices * 3];
//		memcpy(mesh.colors, mesh_to_load->mColors, sizeof(float) * mesh.num_vertices * 3);
	}

	//Tangents  --------------------------------------------------------------------------------------------------------

		mesh.tangents = new float[mesh.num_vertices * 3];
		if (mesh_to_load->HasTangentsAndBitangents())
		{
			memcpy(mesh.tangents, mesh_to_load->mTangents, sizeof(float)*mesh.num_vertices * 3);
		}
		else
		{
			memset(mesh.tangents, 0, mesh.num_vertices * 3);
		}
		
	return Save(mesh, folder_path, output_name, msh_uuid);
}

bool MeshImporter::Save(Mesh& mesh, const char* folder_path, string& output_name, unsigned int& msh_uuid)
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
	bytes = sizeof(float3) * header[1];
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
	msh_uuid = (unsigned int)App->rnd->RandomInt();
	ret = App->file_system->Save(std::to_string(msh_uuid).data(), data, size, folder_path, "msh", output_name);

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

		LoadBuffers(mesh);
	}
	if(buffer)
		delete[] buffer;
	buffer = nullptr;

	return mesh;
}

void MeshImporter::LoadBuffers(Mesh* mesh)
{
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

void MeshImporter::DeleteBuffers(Mesh* mesh)
{
	if (mesh != nullptr)
	{
		App->renderer3D->RemoveBuffer(mesh->id_vertices);
		App->renderer3D->RemoveBuffer(mesh->id_indices);
		App->renderer3D->RemoveBuffer(mesh->id_uvs);
		App->renderer3D->RemoveBuffer(mesh->id_normals);
		App->renderer3D->RemoveBuffer(mesh->id_tangents);
	}
	else
	{
		LOG("Warning: Delete Buffers call on a null mesh");
	}
}

void MeshImporter::CollectGameObjects(GameObject* root, std::vector<GameObject*> vector)
{
	vector.push_back(root);
	std::vector<GameObject*>::const_iterator child = root->GetChilds()->begin();
	while (child != root->GetChilds()->end())
	{
		CollectGameObjects(*child, vector);
		child++;
	}
}

void MeshImporter::SaveInfoFile(std::vector<GameObject*> vector, const char* file)
{
	Data data;
	data.AppendArray("GameObjects");
	for (uint i = 0; i < vector.size(); i++)
	{
		SaveGameObjectInfo(vector[i], data);
	}

	char* buf;
	size_t size = data.Serialize(&buf);

	App->file_system->Save(file, buf, size);

	delete[] buf;
}

void MeshImporter::SaveGameObjectInfo(GameObject* gameObject, Data& data)
{
	Data go_data;
	go_data.AppendString("name", gameObject->name.data());
	go_data.AppendUInt("UUID", gameObject->GetUUID());
	go_data.AppendUInt("parent", gameObject->GetParent() == nullptr ? 0 : gameObject->GetParent()->GetUUID());
	go_data.AppendBool("active", true);
	go_data.AppendBool("static", false);
	go_data.AppendUInt("local_UUID", 0);
	go_data.AppendBool("is_prefab", false);
	go_data.AppendUInt("prefab_root_uuid", 0);
	go_data.AppendString("prefab_path", "");
	go_data.AppendInt("layer", 0);
	go_data.AppendArray("components");


	//WARNING: dirty shit because mesh importer is a mess. Any new resource needs to be added in here
	std::vector<Component*>::const_iterator component = gameObject->GetComponents()->begin();
	while (component != gameObject->GetComponents()->end())
	{
		Data component_data;
		if ((*component)->GetType() != C_TRANSFORM)
		{
			component_data.AppendInt("type", (*component)->GetType());
			component_data.AppendUInt("UUID", (unsigned int)App->rnd->RandomInt());
			component_data.AppendBool("active", true);

			switch ((*component)->GetType())
			{
				case (C_MESH):
					component_data.AppendString("path", ((ComponentMesh*)*component)->GetMesh()->file_path.c_str());
					break;
				case(C_MATERIAL):
				{
					ComponentMaterial* material = ((ComponentMaterial*)*component);
					component_data.AppendString("path", "");
					component_data.AppendArray("textures");
					if (material->list_textures_paths.size() > 0)
					{
						Data tex_data;
						tex_data.AppendString("path", App->resource_manager->FindFile(material->list_textures_paths[0]).c_str());
						component_data.AppendArrayValue(tex_data);
					}
					if (material->list_textures_paths.size() > 1)
					{
						Data tex_data;
						if (material->list_textures_paths[1] != "")
							tex_data.AppendString("path", App->resource_manager->FindFile(material->list_textures_paths[1]).c_str());
						else
							tex_data.AppendString("path", "");
						component_data.AppendArrayValue(tex_data);
					}
						break;
				}
				case(C_ANIMATION):
					component_data.AppendString("path", ((ComponentAnimation*)*component)->GetResourcePath());
					component_data.AppendInt("current_animation", -1);
					break;
				case(C_BONE):
					component_data.AppendString("path", ((ComponentBone*)*component)->GetResourcePath());
					break;
			}
			go_data.AppendArrayValue(component_data);
		}
		else
			(*component)->Save(go_data);

		component++;
	}

	data.AppendArrayValue(go_data);
}

void MeshImporter::LoadAnimBuffers(float * weights, int weights_size, unsigned int & weights_id, int * bones_ids, int bones_ids_size, unsigned int & bones_ids_id)
{
	glGenBuffers(1, (GLuint*)&weights_id);
	glBindBuffer(GL_ARRAY_BUFFER, weights_id);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * weights_size, weights, GL_STATIC_DRAW);

	glGenBuffers(1, (GLuint*)&bones_ids_id);
	glBindBuffer(GL_ARRAY_BUFFER, bones_ids_id);
	glBufferData(GL_ARRAY_BUFFER, sizeof(int) * bones_ids_size, bones_ids, GL_STATIC_DRAW);
}

Mesh * MeshImporter::LoadBillboardMesh()
{
	Mesh* mesh = new Mesh();

	const GLfloat quad_vertex[] = 
	{
		-0.5f, -0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		-0.5f, 0.5f, 0.0f,
		0.5f, 0.5f, 0.0f
	};

	mesh->num_vertices = 4;

	glGenBuffers(1, (GLuint*)&(mesh->id_vertices));
	glBindBuffer(GL_ARRAY_BUFFER, mesh->id_vertices);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * mesh->num_vertices, quad_vertex, GL_STATIC_DRAW);

	const unsigned int indices[] = 
	{
		0, 1, 2,
		1, 3, 2
	};
	mesh->num_indices = 6;

	glGenBuffers(1, (GLuint*)&(mesh->id_indices));
	glBindBuffer(GL_ARRAY_BUFFER, mesh->id_indices);
	glBufferData(GL_ARRAY_BUFFER, sizeof(unsigned int) * mesh->num_indices, indices, GL_STATIC_DRAW);

	const float uvs[] =
	{
		0.0, 0.0,
		1.0, 0.0,
		0.0, 1.0,
		1.0, 1.0
	};

	glGenBuffers(1, (GLuint*)&(mesh->id_uvs));
	glBindBuffer(GL_ARRAY_BUFFER, mesh->id_uvs);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8, uvs, GL_STATIC_DRAW);

	return mesh;
}

Mesh * MeshImporter::LoadQuad()
{
	Mesh* mesh = new Mesh();

	const GLfloat quad_vertex[] =
	{
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f
	};

	mesh->num_vertices = 4;

	glGenBuffers(1, (GLuint*)&(mesh->id_vertices));
	glBindBuffer(GL_ARRAY_BUFFER, mesh->id_vertices);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * mesh->num_vertices, quad_vertex, GL_STATIC_DRAW);

	const unsigned int indices[] =
	{
		0, 1, 3,
		1, 2, 3
	};
	mesh->num_indices = 6;

	glGenBuffers(1, (GLuint*)&(mesh->id_indices));
	glBindBuffer(GL_ARRAY_BUFFER, mesh->id_indices);
	glBufferData(GL_ARRAY_BUFFER, sizeof(unsigned int) * mesh->num_indices, indices, GL_STATIC_DRAW);

	return mesh;
}

bool MeshImporter::ImportUUID(const char * file, const char * path, const char * base_path, std::stack<unsigned int>& uuids, std::vector<uint>& uuids_anim, std::vector<uint>& uuids_bones)
{
	bool ret = false;
	char* buff;
	uint size = App->file_system->Load(path, &buff);

	if (size == 0)
	{
		LOG("[ERROR] Loading mesh %s has failed", path);
		App->editor->DisplayWarning(WarningType::W_ERROR, "Loading mesh %s has failed", path);
		return ret;
	}

	const aiScene* scene = aiImportFileFromMemory(buff, size, aiProcessPreset_TargetRealtime_MaxQuality, NULL);

	if (scene != nullptr && scene->HasMeshes())
	{
		string file_mesh_directory = path;
		file_mesh_directory = file_mesh_directory.substr(0, file_mesh_directory.find_last_of("\//") + 1);

		aiNode* root = scene->mRootNode;
		vector<GameObject*> objects_created;
		vector<const aiMesh*> boned_meshes;
		vector<const GameObject*> boned_game_objects;

		MeshImporter::ImportNodeUUID(root, scene, nullptr, objects_created, boned_meshes, boned_game_objects, file_mesh_directory, base_path, uuids);

		//Renaming RootNode
		if (objects_created[0]->name == "RootNode")
		{
			std::string name = path;
			uint slashPos;
			if ((slashPos = name.find_last_of("/")) != std::string::npos)
				name = name.substr(slashPos + 1, name.size() - slashPos);

			uint pointPos;
			if ((pointPos = name.find_first_of(".")) != std::string::npos)
				name = name.substr(0, name.size() - (name.size() - pointPos));
			objects_created[0]->name = name;
		}

		//Importing animations ----------
		std::string output_animation;
		if (AnimationImporter::ImportSceneAnimations(scene, objects_created[0], base_path, output_animation, uuids_anim))
		{
			ComponentAnimation* animation = (ComponentAnimation*)objects_created[0]->AddComponent(C_ANIMATION);
			animation->SetResource((ResourceFileAnimation*)App->resource_manager->LoadResource(output_animation, RES_ANIMATION));
		}
		//-------------------------------
		//Importing bones ---------------
		AnimationImporter::ImportSceneBones(boned_meshes, boned_game_objects, objects_created[0], base_path, uuids_bones);
		//-------------------------------
		
		SaveInfoFile(objects_created, file);

		for (vector<GameObject*>::iterator go = objects_created.begin(); go != objects_created.end(); ++go)
			delete (*go);

		objects_created.clear();
		aiReleaseImport(scene);

		ret = true;
	}
	else
	{
		LOG("Error loading scene %s. ERROR: %s", path, aiGetErrorString());
		App->editor->DisplayWarning(WarningType::W_ERROR, "Loading scene %s due to %s", path, aiGetErrorString());
	}

	delete[] buff;

	return ret;
}

void MeshImporter::ImportNodeUUID(aiNode* node, const aiScene* scene, GameObject* parent, std::vector<GameObject*>& created_go, std::vector<const aiMesh*>& boned_meshes, std::vector<const GameObject*>& boned_game_objects,
	std::string mesh_file_directory, std::string folder_path, std::stack<unsigned int>& uuids)
{
	//Transformation ------------------------------------------------------------------------------------------------------------------
	aiVector3D translation;
	aiVector3D scaling;
	aiQuaternion rotation;

	node->mTransformation.Decompose(scaling, rotation, translation);

	float3 pos(translation.x, translation.y, translation.z);
	Quat rot(rotation.x, rotation.y, rotation.z, rotation.w);
	float3 scale(scaling.x, scaling.y, scaling.z);

	//Don't load fbx dummies as gameobjects. 
	static const char* dummies[5] =
	{
		"$AssimpFbx$_PreRotation",
		"$AssimpFbx$_Rotation",
		"$AssimpFbx$_PostRotation",
		"$AssimpFbx$_Scaling",
		"$AssimpFbx$_Translation"
	};

	std::string name = node->mName.C_Str();
	for (int i = 0; i < 5; ++i)
	{
		if (name.find(dummies[i]) != string::npos && node->mNumChildren == 1)
		{
			node = node->mChildren[0];
			node->mTransformation.Decompose(scaling, rotation, translation);
			pos += float3(translation.x, translation.y, translation.z);
			scale = float3(scale.x * scaling.x, scale.y * scaling.y, scale.z * scaling.z);
			rot = rot * Quat(rotation.x, rotation.y, rotation.z, rotation.w);
			name = node->mName.C_Str();
			i = -1;
		}
	}

	GameObject* go_root = new GameObject(parent);
	if (parent)
		parent->AddChild(go_root);

	created_go.push_back(go_root);

	go_root->name = name;

	go_root->transform->SetPosition(pos);
	go_root->transform->SetRotation(rot);
	go_root->transform->SetScale(scale);

	go_root->transform->Update(); //Force it to update the matrix

	if (node->mName.length > 0)
		go_root->name = node->mName.C_Str();


	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		GameObject* child = nullptr;

		//Transform
		if (node->mNumMeshes > 1)
		{
			child = new GameObject(go_root);
			go_root->AddChild(child);
			created_go.push_back(child);
		}
		else
		{
			child = go_root;
		}

		//MERGE TODO: send id bones
		if (scene->mMeshes[node->mMeshes[i]]->HasBones())
		{
			boned_meshes.push_back(scene->mMeshes[node->mMeshes[i]]);
			boned_game_objects.push_back(child);
		}

		aiMesh* mesh_to_load = scene->mMeshes[node->mMeshes[i]];

		if (node->mName.length > 0)
			child->name = node->mName.C_Str();

		//Mesh --------------------------------------------------------------------------------------------------------------------------------
		string mesh_path;
		unsigned int msh_uuid = uuids.top();
		bool ret = MeshImporter::ImportMeshUUID(mesh_to_load, folder_path.data(), mesh_path, msh_uuid);
		ComponentMesh* mesh = (ComponentMesh*)child->AddComponent(C_MESH);
		mesh->SetResourceMesh((ResourceFileMesh*)App->resource_manager->LoadResource(mesh_path, RES_MESH));
		uuids.pop();

		//Load Textures --------------------------------------------------------------------------------------------------------------------
		aiMaterial* material = scene->mMaterials[mesh_to_load->mMaterialIndex];
		//uint numTextures = material->GetTextureCount(aiTextureType_DIFFUSE);
		if (material)
		{
			aiString path;
			material->GetTexture(aiTextureType_DIFFUSE, 0, &path);
			ComponentMaterial* c_material = (ComponentMaterial*)child->AddComponent(C_MATERIAL);

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

				c_material->list_textures_paths[0] = complete_path;
				if (normal_path.length > 0)
					c_material->list_textures_paths[1] = normal_complete_path;
			}
		}
	}

	for (unsigned int i = 0; i < node->mNumChildren; i++)
		MeshImporter::ImportNodeUUID(node->mChildren[i], scene, go_root, created_go, boned_meshes, boned_game_objects, mesh_file_directory, folder_path, uuids);
}

bool MeshImporter::ImportMeshUUID(const aiMesh * mesh_to_load, const char * folder_path, std::string & output_name, unsigned int uuid)
{
	//Mesh --------------------------------------------------------------------------------------------------------------------------------

	Mesh mesh;

	//Vertices ------------------------------------------------------------------------------------------------------
	mesh.num_vertices = mesh_to_load->mNumVertices;
	mesh.vertices = new float[mesh.num_vertices * 3];
	memcpy(mesh.vertices, mesh_to_load->mVertices, sizeof(float3) * mesh.num_vertices);


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
				App->editor->DisplayWarning(WarningType::W_WARNING, "Geometry with face != 3 indices is trying to be loaded");
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
		for (unsigned int uvs_item = 0; uvs_item < mesh.num_uvs; uvs_item++)
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
		//mesh.colors = new float[mesh.num_vertices * 3];
		//memcpy(mesh.colors, mesh_to_load->mColors, sizeof(float) * mesh.num_vertices * 3);
	}

	//Tangents  --------------------------------------------------------------------------------------------------------

	mesh.tangents = new float[mesh.num_vertices * 3];
	if (mesh_to_load->HasTangentsAndBitangents())
	{
		memcpy(mesh.tangents, mesh_to_load->mTangents, sizeof(float)*mesh.num_vertices * 3);
	}
	else
	{
		memset(mesh.tangents, 0, mesh.num_vertices * 3);
	}

	return SaveUUID(mesh, folder_path, output_name, uuid);
}

bool MeshImporter::SaveUUID(Mesh & mesh, const char * folder_path, std::string & output_name, unsigned int uuid)
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
	bytes = sizeof(float3) * header[1];
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
	ret = App->file_system->Save(std::to_string(uuid).data(), data, size, folder_path, "msh", output_name);

	delete[] data;
	data = nullptr;

	return ret;
}
