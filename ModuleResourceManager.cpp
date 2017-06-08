#include "ModuleResourceManager.h"

#include "Application.h"
#include "ModuleGOManager.h"
#include "ModuleFileSystem.h"
#include "ModulePhysics3D.h"
#include "ModuleEditor.h"

#include "GameObject.h"
#include "Random.h"
#include "Assets.h"
#include "Time.h"

#include "ComponentMesh.h"

#include "ShaderComplier.h"
#include "TextureImporter.h"
#include "MeshImporter.h"

#include "ResourceFileMaterial.h"
#include "ResourceFileRenderTexture.h"
#include "RenderTexEditorWindow.h"
#include "ResourceFileAnimation.h"
#include "ResourceFileBone.h"
#include "ResourceFileAudio.h"
#include "ResourceFileMesh.h"
#include "ResourceFileTexture.h"
#include "ResourceFilePrefab.h"
#include "ResourceScriptsLibrary.h"

#include "Assimp/include/cimport.h"
#include "Assimp/include/scene.h"
#include "Assimp/include/postprocess.h"
#include "Assimp/include/cfileio.h"
#pragma comment (lib, "Assimp/libx86/assimp.lib")

#include "Devil/include/il.h"
#include "Devil/include/ilut.h"
#pragma comment ( lib, "Devil/libx86/DevIL.lib" )
#pragma comment ( lib, "Devil/libx86/ILU.lib" )
#pragma comment ( lib, "Devil/libx86/ILUT.lib" )

ModuleResourceManager::ModuleResourceManager(const char* name, bool start_enabled) : Module(name, start_enabled)
{ }

ModuleResourceManager::~ModuleResourceManager()
{
	for (list<ResourceFile*>::iterator rc_file = resource_files.begin(); rc_file != resource_files.end(); ++rc_file)
		delete *rc_file;
	resource_files.clear();
}

bool ModuleResourceManager::Init(Data & config)
{
	aiLogStream stream;
	stream = aiGetPredefinedLogStream(aiDefaultLogStream_DEBUGGER, nullptr);
	aiAttachLogStream(&stream);

	//Initialize Devil
	ilInit();
	iluInit();
	ilutInit();
	ilutRenderer(ILUT_OPENGL);
	return true;
}

bool ModuleResourceManager::Start()
{
	LoadDefaults();

	if (App->StartInGame() == false)
		UpdateAssetsAuto();
	return true;
}

update_status ModuleResourceManager::Update()
{
	if (App->StartInGame() == false && App->IsGameRunning() == false)
	{
		modification_timer += time->RealDeltaTime();

		if (modification_timer >= CHECK_MOD_TIME)
		{
			//CheckDirectoryModification(App->editor->assets->root);
			modification_timer = 0.0f;
			//App->editor->assets->Refresh();
		}
	}


	return UPDATE_CONTINUE;
}

bool ModuleResourceManager::CleanUp()
{
	delete billboard_mesh;

	ilShutDown();
	aiDetachAllLogStreams();
	return true;
}

void ModuleResourceManager::UpdateAssetsAuto()
{
	vector<tmp_mesh_file> mesh_files;
	UpdateAssetsAutoRecursive(ASSETS_FOLDER, LIBRARY_FOLDER, mesh_files);

	//New meshes
	for (vector<tmp_mesh_file>::iterator tmp = mesh_files.begin(); tmp != mesh_files.end(); tmp++)
	{
		ImportFile(tmp->mesh_path.data(), tmp->assets_folder, tmp->library_folder);
	}

	//Meshes with meta
	for (vector<tmp_mesh_file_uuid>::iterator tmp = tmp_mesh_uuid_files.begin(); tmp != tmp_mesh_uuid_files.end(); tmp++)
	{
		ImportMeshFileWithMeta(tmp->mesh_path.data(), tmp->assets_folder, tmp->library_folder, tmp->uuid, tmp->meta_path);
	}

	tmp_mesh_uuid_files.clear();
	mesh_files.clear();
}

void ModuleResourceManager::UpdateAssetsAutoRecursive(const string& assets_dir, const string& library_dir, vector<tmp_mesh_file>& mesh_files)
{
	//Get All files and folders
	vector<string> files, folders;
	App->file_system->GetFilesAndDirectories(assets_dir.data(), folders, files);

	const char* meta_ext = "meta";
	//Files
	for (vector<string>::const_iterator file = files.begin(); file != files.end(); ++file)
	{
		if (GetFileExtension((*file).c_str()) != FileType::NONE)
		{
			bool meta_found = false;
			//Search for the meta
			for (vector<string>::const_iterator meta = files.begin(); meta != files.end(); ++meta)
			{
				//Check if the file is .meta
				if (strcmp((*meta).substr((*meta).find_last_of(".") + 1, 4).c_str(), meta_ext) == 0)
				{
					string meta_name = (*meta).substr(0, (*meta).find_last_of("."));
					if ((*file).compare(meta_name) == 0)
					{
						meta_found = true;
						string meta_complete_path = assets_dir + *meta;
						UpdateFileWithMeta(meta_complete_path, assets_dir, library_dir);
						break;
					}
				}
			}

			if (!meta_found)
			{
				string path = assets_dir + *file;
				if (GetFileExtension((*file).c_str()) == FileType::MESH)
				{
					tmp_mesh_file tmp_file;
					tmp_file.mesh_path = path;
					tmp_file.assets_folder = assets_dir;
					tmp_file.library_folder = library_dir;
					mesh_files.push_back(tmp_file);
				}
				else
				{
					ImportFile(path.c_str(), assets_dir, library_dir);
				}
			}
		}
	}

	//Folders
	for (vector<string>::const_iterator folder = folders.begin(); folder != folders.end(); ++folder)
	{
		bool meta_found = false;

		string library_path;
		string path = assets_dir + (*folder);
		//Search for the meta
		for (vector<string>::const_iterator meta = files.begin(); meta != files.end(); ++meta)
		{
			//Check if the file is .meta
			if (strcmp((*meta).substr((*meta).find_last_of(".") + 1, 4).c_str(), meta_ext) == 0)
			{
				string meta_name = (*meta).substr(0, (*meta).find_last_of("."));
				if ((*folder).compare(meta_name) == 0)
				{
					meta_found = true;
					library_path = UpdateFolderWithMeta(assets_dir + (*meta));
					library_path += '/';
					break;
				}
			}
		}


		if (!meta_found)
		{
			library_path = library_dir;
			CreateFolder(path.c_str(), library_path); //Library path is updated here
		}
		path += +'/';
		//Search inside the folder
		UpdateAssetsAutoRecursive(path, library_path, mesh_files);
	}

}

void ModuleResourceManager::UpdateFileWithMeta(const string& meta_file, const string& base_assets_dir, const string& base_lib_dir)
{
	unsigned int type, uuid;
	string md5, md5_lib, library_path, assets_path;

	ReadMetaFile(meta_file.data(), type, uuid, md5, md5_lib, library_path, assets_path);

	char* c_assets_md5 = App->file_system->GetMD5(assets_path.data());
	assert(c_assets_md5 != nullptr);
	//MD5 meta is same as assets?
	if (md5.compare(c_assets_md5) != 0)
	{
		//Update md5 with assets
		ChangeAssetsMD5MetaFile(meta_file.data(), c_assets_md5);
		//Import file to library
		ImportFileWithMeta(type, uuid, library_path, assets_path, base_assets_dir, base_lib_dir, meta_file);
		return;
	}

	if (App->file_system->Exists(library_path.data()))
	{
		char* c_lib_md5 = App->file_system->GetMD5(library_path.data());
		assert(c_lib_md5 != nullptr);
		if (md5_lib.compare(c_lib_md5) != 0)
		{
			ImportFileWithMeta(type, uuid, library_path, assets_path, base_assets_dir, base_lib_dir, meta_file); //Is the same method. It will create the folder in library again. NP
		}	
	}
	else
	{
		//Create the file in library
		ImportFileWithMeta(type, uuid, library_path, assets_path, base_assets_dir, base_lib_dir, meta_file);
	}
}

void ModuleResourceManager::ImportFileWithMeta(unsigned int type, unsigned int uuid, string library_path, string assets_path, const string& base_assets_dir, const string& base_lib_dir, const string& meta_path)
{
	//Create the folder in library
	string lib_folder_path = library_path.data();
	lib_folder_path = lib_folder_path.substr(0, lib_folder_path.find_last_of("/\\") + 1);
	App->file_system->GenerateDirectory(lib_folder_path.data());

	switch (type)
	{
	case IMAGE:
		TextureImporter::Import(library_path.data(), assets_path.data());
		break;
	case MESH:
	{
		tmp_mesh_file_uuid tmp;
		tmp.mesh_path = assets_path.data();
		tmp.assets_folder = base_assets_dir.data();
		tmp.library_folder = base_lib_dir.data();
		tmp.uuid = uuid;
		tmp.meta_path = meta_path.data();

		tmp_mesh_uuid_files.push_back(tmp);
	}
	break;
	case TERRAIN:
	{
		App->file_system->Delete(library_path.data());
		App->file_system->DuplicateFile(assets_path.data(), library_path.data());
		break;
	}

	case PREFAB:
		App->file_system->DuplicateFile(assets_path.data(), library_path.data());
		break;
	case SCENE:
		App->file_system->DuplicateFile(assets_path.data(), library_path.data());
		break;
	case VERTEX:
		App->file_system->DuplicateFile(assets_path.data(), library_path.data());
		break;
	case FRAGMENT:
		App->file_system->DuplicateFile(assets_path.data(), library_path.data());
		break;
	case MATERIAL:
		App->file_system->DuplicateFile(assets_path.data(), library_path.data());
		break;
	case SOUNDBANK:
	{
		App->file_system->DuplicateFile(assets_path.data(), library_path.data()); // Soundbank

		string json_file_path = assets_path.substr(0, assets_path.find_last_of('.')) + ".json";
		string lib_json_path = library_path.substr(0, library_path.find_last_of('/') + 1);
		lib_json_path += std::to_string(uuid) + ".json";
		App->file_system->DuplicateFile(json_file_path.data(), lib_json_path.data()); // JSON
		break;
	}
	case RENDER_TEXTURE:
		App->file_system->DuplicateFile(assets_path.data(), library_path.data());
		break;
	case SCRIPTS_LIBRARY:
		App->file_system->DuplicateFile(assets_path.data(), library_path.data());
		break;
	}
}

void ModuleResourceManager::ImportMeshFileWithMeta(const char* path, const string& base_dir, const string& base_library_dir, unsigned int id, const string& meta_path)
{
	uint uuid = id;

	//Create the link to Library
	string final_mesh_path;
	if (base_library_dir.size() == 0)
		final_mesh_path = LIBRARY_FOLDER;
	else
		final_mesh_path = base_library_dir;

	final_mesh_path += std::to_string(uuid) + "/";
	string library_dir = final_mesh_path;
	final_mesh_path += std::to_string(uuid) + ".inf";

	//Read the meta uuids
	stack<unsigned int> meshes_uuids;
	std::vector<uint> anim_uuids;
	std::vector<uint> bone_uuids;

	char* buffer = nullptr;
	if (App->file_system->Load(meta_path.data(), &buffer) > 0)
	{
		Data meta(buffer);

		int size_meshes = meta.GetArraySize("meshes");
		for (int i = size_meshes - 1; i >= 0; i--)
		{
			meshes_uuids.push(meta.GetArray("meshes", i).GetUInt("uuid"));
		}

		int size_anim = meta.GetArraySize("animations");
		for (int i = 0; i < size_anim; i++)
		{
			anim_uuids.push_back(meta.GetArray("animations", i).GetUInt("uuid"));
		}

		int size_bones = meta.GetArraySize("bones");
		for (int i = 0; i < size_bones; i++)
		{
			bone_uuids.push_back(meta.GetArray("bones", i).GetUInt("uuid"));
		}
	}

	if (buffer)
		delete[] buffer;

	MeshImporter::ImportUUID(final_mesh_path.data(), path, library_dir.data(), meshes_uuids, anim_uuids, bone_uuids);

}

string ModuleResourceManager::UpdateFolderWithMeta(const string& meta_path)
{
	unsigned int type, uuid;
	string assets_path, library_path, md5, md5_lib;
	ReadMetaFile(meta_path.data(), type, uuid, md5, md5_lib, library_path, assets_path);

	if (App->file_system->Exists(library_path.data()) == false)
	{
		App->file_system->GenerateDirectory(library_path.data());
	}

	return library_path;
}

void ModuleResourceManager::InputFileDropped(list<string>& files)
{
	//Classify not mesh files and mesh files
	vector<string> non_mesh_files, mesh_files;
	for (list<string>::iterator it = files.begin(); it != files.end(); ++it)
	{
		bool is_mesh = false;
		if (App->file_system->IsDirectoryOutside((*it).data()) == false)
			if (GetFileExtension((*it).data()) == FileType::MESH)
				is_mesh = true;
		if (is_mesh)
			mesh_files.push_back(*it);
		else
			non_mesh_files.push_back(*it);
	}

	for (vector<string>::iterator it = non_mesh_files.begin(); it != non_mesh_files.end(); ++it)
		FileDropped((*it).data());

	for (vector<string>::iterator it = mesh_files.begin(); it != mesh_files.end(); ++it)
		FileDropped((*it).data());
}

void ModuleResourceManager::FileDropped(const char * file_path)
{

	if (App->file_system->IsDirectoryOutside(file_path))
	{
		vector<tmp_mesh_file> mesh_files; // Mesh files are imported the last to find all the textures linked.
		ImportFolder(file_path, mesh_files);

		for (vector<tmp_mesh_file>::iterator tmp = mesh_files.begin(); tmp != mesh_files.end(); tmp++)
		{
			ImportFile(tmp->mesh_path.data(), tmp->assets_folder, tmp->library_folder);
		}
		mesh_files.clear();
	}
	else
	{
		ImportFile(file_path, App->editor->assets->CurrentDirectory(), App->editor->assets->CurrentLibraryDirectory());
	}

	App->editor->RefreshAssets();
}

GameObject* ModuleResourceManager::LoadFile(const string & library_path, const FileType & type)
{
	GameObject* ret = nullptr;
	switch (type)
	{
	case MESH:
		ret = LoadPrefabFile(library_path);
		App->go_manager->LinkAnimation(App->go_manager->root);
		break;
	case PREFAB:
		ResourceFilePrefab* r_prefab = (ResourceFilePrefab*)LoadResource(library_path, ResourceFileType::RES_PREFAB);
		if (r_prefab)
		{
			ret = r_prefab->LoadPrefabAsCopy();
		}
		App->go_manager->LinkAnimation(App->go_manager->root);
		break;
	}
	return ret;
}

ResourceFile * ModuleResourceManager::LoadResource(const string &path, ResourceFileType type)
{
	ResourceFile* rc_file = nullptr;

	string name = path.substr(path.find_last_of("/\\") + 1);
	name = name.substr(0, name.find_last_of('.'));
	if (name.length() == 0)
		return nullptr;

	unsigned int uuid = std::stoul(name);
	rc_file = FindResourceByUUID(uuid);

	if (rc_file == nullptr)
	{
		switch (type)
		{
		case RES_MESH:
			rc_file = new ResourceFileMesh(type, path, uuid);
			rc_file->Load();
			mesh_bytes += rc_file->GetBytes();
			num_meshes++;
			break;
		case RES_TEXTURE:
			rc_file = new ResourceFileTexture(type, path, uuid);
			rc_file->Load();
			texture_bytes += rc_file->GetBytes();
			num_textures++;
			break;
		case RES_MATERIAL:
			rc_file = new ResourceFileMaterial(type, path, uuid);
			rc_file->Load();
			//TODO: save info about the shaders loaded in vram
			break;
		case RES_RENDER_TEX:
			rc_file = new ResourceFileRenderTexture(type, path, uuid);
			rc_file->Load();
			break;
		case RES_ANIMATION:
			rc_file = new ResourceFileAnimation(path, uuid);
			rc_file->Load();
			break;
		case RES_BONE:
			rc_file = new ResourceFileBone(path, uuid);
			rc_file->Load();
			break;
		case RES_SOUNDBANK:
			rc_file = new ResourceFileAudio(type, path, uuid);
			rc_file->Load();
			break;
		case RES_PREFAB:
			rc_file = new ResourceFilePrefab(type, path, uuid);
			rc_file->Load(); //This load doesn't actually do his job. Needs to call another load method after this.
			break;
		case RES_SCRIPTS_LIBRARY:
			rc_file = new ResourceScriptsLibrary(type, path, uuid);
			rc_file->Load();
			break;
		}

		resource_files.push_back(rc_file);
		bytes_in_memory += rc_file->GetBytes();
	}
	else
	{
		rc_file->Load();
	}

	return rc_file;
}

void ModuleResourceManager::UnloadResource(const string & path)
{
	ResourceFile* rc_file = nullptr;

	string name = path.substr(path.find_last_of("/\\") + 1);
	name = name.substr(0, name.find_last_of('.'));
	unsigned int uuid = std::stoul(name);
	rc_file = FindResourceByUUID(uuid);

	if (rc_file != nullptr)
	{
		rc_file->Unload();
	}
}

void ModuleResourceManager::RemoveResourceFromList(ResourceFile *file)
{
	if (file)
	{
		switch (file->GetType())
		{
		case ResourceFileType::RES_MESH:
			--num_meshes;
			mesh_bytes -= file->GetBytes();
			break;
		case ResourceFileType::RES_TEXTURE:
			--num_textures;
			texture_bytes -= file->GetBytes();
			break;
		}
		resource_files.remove(file);
		bytes_in_memory -= file->GetBytes();
		delete file;
	}
}

ResourceFile * ModuleResourceManager::FindResourceByUUID(unsigned int uuid)
{
	for (list<ResourceFile*>::iterator rc_file = resource_files.begin(); rc_file != resource_files.end(); ++rc_file)
		if ((*rc_file)->GetUUID() == uuid)
			return *rc_file;

	return nullptr;
}

ResourceFile * ModuleResourceManager::FindResourceByLibraryPath(const string & library)
{
	string name = library.substr(library.find_last_of("/\\") + 1);
	name = name.substr(0, name.find_last_of('.'));
	unsigned int uuid = std::stoul(name);

	return FindResourceByUUID(uuid);
}

void ModuleResourceManager::SaveScene(const char * file_name, string base_library_path)
{
	string name_to_save = file_name;
	//Add extension if doesn't have it yet
	if (name_to_save.find(".ezx", name_to_save.length() - 4) == string::npos)
		name_to_save += ".ezx";

	// Saving SCENE information
	Data root_node;
	root_node.AppendArray("GameObjects");
	App->go_manager->root->Save(root_node);

	string library_scene_path;
	string meta_file = name_to_save + ".meta";
	unsigned int uuid = 0;
	if (App->file_system->Exists(meta_file.data()))
	{
		char* meta_buf;
		int meta_size = App->file_system->Load(meta_file.data(), &meta_buf);

		if (meta_size > 0)
		{
			Data meta_data(meta_buf);
			library_scene_path = meta_data.GetString("library_path");
			uuid = meta_data.GetUInt("uuid");
		}
		else
		{
			LOG("[ERROR] While opening the meta file(%s) of %s", meta_file.data(), name_to_save.data());
			App->editor->DisplayWarning(WarningType::W_ERROR, "While opening the meta file(%s) of %s", meta_file.data(), name_to_save.data());
		}

		delete[] meta_buf;
	}
	else
	{
		uuid = App->rnd->RandomInt();
		string library_dir = base_library_path + "/" + std::to_string(uuid) + "/";
		App->file_system->GenerateDirectory(library_dir.data());
		library_scene_path = library_dir + std::to_string(uuid) + ".ezx";
	}

	root_node.AppendString("current_assets_scene_path", name_to_save.c_str());
	App->go_manager->SetCurrentAssetsScenePath(name_to_save.c_str());
	root_node.AppendString("current_library_scene_path", library_scene_path.c_str());
	App->go_manager->SetCurrentLibraryScenePath(library_scene_path.c_str());

	char* buf;
	size_t size = root_node.Serialize(&buf);
	App->file_system->Save(name_to_save.data(), buf, size);
	App->file_system->Save(library_scene_path.data(), buf, size); //Duplicate the file in library

	GenerateMetaFile(name_to_save.data(), FileType::SCENE, uuid, library_scene_path.data());

	delete[] buf;

	std::string library_textureMapPath = library_scene_path.substr(0, library_scene_path.length() - 3);
	library_textureMapPath += "txmp";
	App->physics->SaveTextureMap(library_textureMapPath.data());

	std::string textureMapPath = name_to_save.substr(0, name_to_save.length() - 3);
	textureMapPath += "txmp";
	App->physics->SaveTextureMap(textureMapPath.data());
	GenerateMetaFile(textureMapPath.data(), FileType::TERRAIN, 0, library_textureMapPath.data());

	App->editor->RefreshAssets();
}

bool ModuleResourceManager::LoadSceneFromAssets(const char* file_name)
{
	std::string lib_path;
	if (!App->StartInGame())
		lib_path = FindFile(file_name);
	else
		lib_path = file_name;

	if (lib_path != "" && lib_path != " ")
	{
		return LoadScene(lib_path.c_str());
	}
}

bool ModuleResourceManager::LoadScene(const char *file_name)
{
	bool ret = false;
	//TODO: Now the current scene is destroyed. Ask the user if wants to save the changes.

	char* buffer = nullptr;
	uint size = App->file_system->Load(file_name, &buffer);
	if (size == 0)
	{
		LOG("[ERROR] While loading Scene %s", file_name);
		App->editor->DisplayWarning(WarningType::W_ERROR, "While loading scene %s", file_name);

		if (buffer)
			delete[] buffer;
		return false;
	}

	Data scene(buffer);
	const char *scene_path = scene.GetString("current_assets_scene_path");
	if (scene_path) App->go_manager->SetCurrentAssetsScenePath(scene_path);
	scene_path = scene.GetString("current_library_scene_path");
	if (scene_path) App->go_manager->SetCurrentLibraryScenePath(scene_path);

	Data root_objects;
	root_objects = scene.GetArray("GameObjects", 0);

	if (root_objects.IsNull() == false)
	{
		//Remove the current scene
		App->go_manager->ClearScene();

		for (size_t i = 0; i < scene.GetArraySize("GameObjects"); i++)
		{
			if (i == 0)
				App->go_manager->root = App->go_manager->LoadGameObject(scene.GetArray("GameObjects", i));
			else
				App->go_manager->LoadGameObject(scene.GetArray("GameObjects", i));
		}

		/*App->go_manager->SetCurrentScenePath(file_name);*/

		while (App->physics->GetNTextures() > 0)
		{
			App->physics->DeleteTexture(0);
		}
		for (size_t i = 0; i < scene.GetArraySize("terrain_textures"); i++)
		{
			Data tex = scene.GetArray("terrain_textures", i);
			string texName("");
			App->physics->LoadTexture(tex.GetString("path"), -1, texName);
		}

		App->physics->SetTerrainMaxHeight(scene.GetFloat("terrain_scaling"));
		App->physics->SetTextureScaling(scene.GetFloat("terrain_tex_scaling"));

		std::string textureMapPath;
		int len = 0;
		bool loadedTerrain = false;

		textureMapPath = file_name;

		len = textureMapPath.find(".ezx");
		if (len == string::npos)
		{
			len = textureMapPath.find(".json");
		}
		len++;
		textureMapPath = textureMapPath.substr(0, len);
		textureMapPath += "txmp";
		loadedTerrain = App->physics->LoadTextureMap(textureMapPath.data());

		if (scene_path != nullptr && loadedTerrain == false)
		{
			textureMapPath = scene_path;

			len = textureMapPath.find(".ezx");
			if (len == string::npos)
			{
				len = textureMapPath.find(".json");
			}
			len++;
			textureMapPath = textureMapPath.substr(0, len);
			textureMapPath += "txmp";
			loadedTerrain = App->physics->LoadTextureMap(textureMapPath.data());
		}

		if (App->IsGameRunning())
		{
			App->OnPlay();
		}
		ret = true;
	}
	else
	{
		LOG("[WARNING] The scene %s is not a valid scene file", file_name);
		App->editor->DisplayWarning(WarningType::W_WARNING, "The scene %s is not a valid scene file", file_name);
	}

	delete[] buffer;

	App->go_manager->LinkAnimation(App->go_manager->root);

	return ret;
}

void ModuleResourceManager::ReloadScene()
{
	string current_scene = App->go_manager->GetCurrentLibraryScenePath();
	if (current_scene.size() > 0)
	{
		LoadScene(current_scene.data());
	}
}

ResourceFilePrefab* ModuleResourceManager::SavePrefab(GameObject * gameobject)
{
	//Create the file
	//Create the meta
	//Duplicate file in assets
	ResourceFilePrefab* ret = nullptr;
	bool unlinked = false;
	for (std::vector<GameObject*>::const_iterator it = gameobject->GetChilds()->begin(); it != gameobject->GetChilds()->end(); it++)
	{
		if (UnlinkChildPrefabs(*it)) unlinked = true;
	}

	if (unlinked == true)
	{
		LOG("Warning: a GameObject child was a prefab, it has lost it's link to it.");
		App->editor->DisplayWarning(WarningType::W_ERROR, "Warning: a GameObject child was a prefab, it has lost it's link to it");
	}

	Data root_node;
	root_node.AppendArray("GameObjects");
	GameObject* parent = gameobject->GetParent();
	gameobject->SetParent(nullptr);
	gameobject->SetAsPrefab(gameobject->GetUUID());
	gameobject->Save(root_node, true);
	char* buf;
	size_t size = root_node.Serialize(&buf);

	string name = App->editor->assets->CurrentDirectory() + gameobject->name.data();
	name += ".pfb";
	App->file_system->Save(name.data(), buf, size);

	string meta_file = name + ".meta";
	string library_path;
	if (App->file_system->Exists(meta_file.data()))
	{
		char* meta_buf;
		int meta_size = App->file_system->Load(meta_file.data(), &meta_buf);

		if (meta_size > 0)
		{
			Data meta_data(meta_buf);
			library_path = meta_data.GetString("library_path");
			unsigned int uuid = meta_data.GetUInt("uuid");
			App->file_system->Save(library_path.data(), buf, size);
			GenerateMetaFile(name.data(), FileType::PREFAB, uuid, library_path.data());
		}
		else
		{
			LOG("Error while opening the meta file(%s) of %s", meta_file.data(), name.data());
			App->editor->DisplayWarning(WarningType::W_ERROR, "While opening the meta file(%s) of %s", meta_file.data(), name.data());
		}

		delete[] meta_buf;
	}
	else
	{
		unsigned int uuid = App->rnd->RandomInt();
		string library_dir = App->editor->assets->CurrentLibraryDirectory() + "/" + std::to_string(uuid) + "/";
		App->file_system->GenerateDirectory(library_dir.data());
		library_path = library_dir + std::to_string(uuid) + ".pfb";
		App->file_system->Save(library_path.data(), buf, size);
		GenerateMetaFile(name.data(), FileType::PREFAB, uuid, library_path.data());
	}

	delete[] buf;

	ret = (ResourceFilePrefab*)LoadResource(library_path, ResourceFileType::RES_PREFAB);
	if (ret)
	{
		gameobject->rc_prefab = ret;
		ret->InsertOriginalInstance(gameobject);
	}

	gameobject->SetParent(parent);
	gameobject->prefab_path = library_path.data();

	return ret;
}

bool ModuleResourceManager::UnlinkChildPrefabs(GameObject* gameObject)
{
	bool ret = false;
	if (gameObject->IsPrefab())
	{
		gameObject->UnlinkPrefab();
		ret = true;
	}
	for (std::vector<GameObject*>::const_iterator it = gameObject->GetChilds()->begin(); it != gameObject->GetChilds()->end(); it++)
	{
		if (UnlinkChildPrefabs(*it))
		{
			ret = true;
		}
	}
	return ret;
}

void ModuleResourceManager::SaveMaterial(const Material & material, const char * path, uint _uuid)
{
	material.Save(path);
	uint uuid = (_uuid == 0) ? App->rnd->RandomInt() : _uuid;
	string assets_folder = path;
	assets_folder = assets_folder.substr(0, assets_folder.find_last_of("/\\") + 1);
	string library_path = App->editor->assets->FindLibraryDirectory(assets_folder);
	library_path = library_path + "/" + std::to_string(uuid) + "/";
	App->file_system->GenerateDirectory(library_path.data());
	library_path = library_path + std::to_string(uuid) + ".mat";
	material.Save(library_path.data());
	GenerateMetaFile(path, FileType::MATERIAL, uuid, library_path);
}

unsigned int ModuleResourceManager::GetDefaultBillboardShaderId() const
{
	return default_billboard_shader;
}

Mesh * ModuleResourceManager::GetDefaultBillboardMesh() const
{
	return billboard_mesh;
}

Mesh * ModuleResourceManager::GetDefaultQuadParticleMesh() const
{
	return quad_particles_mesh;
}

string ModuleResourceManager::FindFile(const string & assets_file_path) const
{
	string ret;

	string meta = assets_file_path;
	meta += ".meta";

	char* buffer = nullptr;
	int size = App->file_system->Load(meta.data(), &buffer);
	if (size > 0)
	{
		Data data(buffer);
		ret = data.GetString("library_path");
	}
	else
	{
		LOG("[ERROR] Could not find file %s", assets_file_path.c_str());
		App->editor->DisplayWarning(WarningType::W_ERROR, "Could not find file %s", assets_file_path.c_str());
	}
	delete[] buffer;

	return ret;
}

ResourceFileType ModuleResourceManager::GetResourceType(const string & path) const
{
	char* texture_extension = "dds";
	char* mesh_extension = "msh";
	char* render_texture_extension = "rtx";

	string extension = path.substr(path.find_last_of(".") + 1);

	if (extension.compare(texture_extension) == 0)
		return ResourceFileType::RES_TEXTURE;

	if (extension.compare(mesh_extension) == 0)
		return ResourceFileType::RES_MESH;

	if (extension.compare(render_texture_extension) == 0)
		return ResourceFileType::RES_RENDER_TEX;

	return ResourceFileType::RES_MATERIAL; //TODO: CREATE A NULL VALUE FOR THIS
}

int ModuleResourceManager::GetNumberResources() const
{
	return resource_files.size();
}

int ModuleResourceManager::GetNumberTexures() const
{
	return num_textures;
}

int ModuleResourceManager::GetNumberMeshes() const
{
	return num_meshes;
}

int ModuleResourceManager::GetTotalBytesInMemory() const
{
	return bytes_in_memory;
}

int ModuleResourceManager::GetTextureBytes() const
{
	return texture_bytes;
}

int ModuleResourceManager::GetMeshBytes() const
{
	return mesh_bytes;
}

void ModuleResourceManager::CreateRenderTexture(const string & assets_path, const string & library_path)
{
	string assets_name = assets_path + "RenderTexture.rtx";

	uint uuid = App->rnd->RandomInt();

	string library_name = library_path;
	library_name += std::to_string(uuid) + "/";
	App->file_system->GenerateDirectory(library_name.data());

	library_name += std::to_string(uuid) + ".rtx";

	char* buffer;

	Data data;
	data.AppendInt("width", 0);
	data.AppendInt("height", 0);
	data.AppendBool("use_depth_as_texture", false);
	data.AppendString("assets_path", assets_name.data());
	data.AppendString("library_path", library_name.data());

	int size = data.Serialize(&buffer);

	App->file_system->Save(assets_name.data(), buffer, size);
	App->file_system->Save(library_name.data(), buffer, size);
	GenerateMetaFile(assets_name.data(), FileType::RENDER_TEXTURE, uuid, library_name);

	delete[] buffer;

	//Open edit window automatically
	App->editor->rendertex_win->LoadToEdit(library_name.data());
	App->editor->rendertex_win->SetActive(true);
}

void ModuleResourceManager::SaveRenderTexture(const string & assets_path, const string & library_path, int width, int height, bool use_depth_as_texture) const
{
	char* buffer;
	Data data;
	data.AppendInt("width", width);
	data.AppendInt("height", height);
	data.AppendBool("use_depth_as_texture", use_depth_as_texture);
	data.AppendString("assets_path", assets_path.data());
	data.AppendString("library_path", library_path.data());

	int size = data.Serialize(&buffer);

	App->file_system->Save(assets_path.data(), buffer, size);
	App->file_system->Save(library_path.data(), buffer, size);

	delete[] buffer;
}

///Given a path returns if the file is one of the valid extensions to import.
FileType ModuleResourceManager::GetFileExtension(const char * path) const
{
	// Extensions must always contain 3 letters!
	char* mesh_extensions[] = { "fbx", "FBX", "obj", "OBJ", "dae" };
	char* image_extensions[] = { "png", "PNG", "tga", "TGA", "jpg", "JPG" };
	char* scene_extension = "ezx";
	char* vertex_extension = "ver";
	char* fragment_extension = "fra";
	char* render_texture_extension = "rtx";
	char* soundbank_extension = "bnk";
	char* script_library_extension = "dll";
	char* prefab_extension = "pfb";
	char* material_extension = "mat";
	char* terrain_extension = "txmp";

	string name = path;
	string extension = name.substr(name.find_last_of(".") + 1);

	for (int i = 0; i < 5; i++)
	{
		if (extension.compare(mesh_extensions[i]) == 0)
			return FileType::MESH;
	}		

	for (int i = 0; i < 6; i++)
	{
		if (extension.compare(image_extensions[i]) == 0)
			return FileType::IMAGE;
	}		

	if (extension.compare(scene_extension) == 0)
		return FileType::SCENE;

	if (extension.compare(vertex_extension) == 0)
		return FileType::VERTEX;

	if (extension.compare(fragment_extension) == 0)
		return FileType::FRAGMENT;

	if (extension.compare(render_texture_extension) == 0)
		return FileType::RENDER_TEXTURE;

	if (extension.compare(soundbank_extension) == 0)
		return FileType::SOUNDBANK;

	if (extension.compare(script_library_extension) == 0)
		return FileType::SCRIPTS_LIBRARY;

	if (extension.compare(prefab_extension) == 0)
		return FileType::PREFAB;

	if (extension.compare(material_extension) == 0)
		return FileType::MATERIAL;

	if (extension.compare(terrain_extension) == 0)
		return FileType::TERRAIN;

	return NONE;
}

unsigned int ModuleResourceManager::GetUUIDFromLib(const string & library_path)const
{
	string path = library_path;
	string name = path.substr(path.find_last_of("/\\") + 1);
	name = name.substr(0, name.find_last_of('.'));
	if (name.length() == 0)
		return 0;

	return std::stoul(name);
}

void ModuleResourceManager::LoadDefaults()
{
	default_billboard_shader = ShaderCompiler::LoadDefaultBilboardShader();

	billboard_mesh = MeshImporter::LoadBillboardMesh();
	quad_particles_mesh = MeshImporter::LoadQuad();

	unsigned int ps_ps_ver = ShaderCompiler::CompileVertex("Resources/Particles/particleV.ver");
	unsigned int ps_ps_fra = ShaderCompiler::CompileFragment("Resources/Particles/particleF.fra");

}

string ModuleResourceManager::CopyOutsideFileToAssetsCurrentDir(const char * path, string base_dir) const
{
	string current_dir;
	if (base_dir.size() == 0)
		current_dir = App->editor->GetAssetsCurrentDir();
	else
		current_dir = base_dir;
	current_dir += App->file_system->GetNameFromPath(path);
	App->file_system->CopyFromOutsideFile(path, current_dir.data());
	return current_dir;
}

void ModuleResourceManager::GenerateMetaFile(const char *path, FileType type, uint uuid, string library_path, bool is_file) const
{
	Data root;
	root.AppendUInt("Type", static_cast<unsigned int>(type));
	root.AppendUInt("UUID", uuid);
	if (is_file)
	{
		char* md5 = App->file_system->GetMD5(path);
		if (md5)
			root.AppendString("md5", md5);
		char* md5_lib = App->file_system->GetMD5(library_path.data());
		if (md5_lib)
			root.AppendString("md5_lib", md5_lib);
		else
		{
			LOG("[ERROR] While saving meta file. The library file is not created yet. File %s - Library %s", path, library_path.data());
			App->editor->DisplayWarning(WarningType::W_ERROR, "While saving meta file. The library file is not created yet. File %s - Library %s", path, library_path.data());
		}
	}
	root.AppendString("library_path", library_path.data());
	root.AppendString("original_file", path);

	char* buf;
	size_t size = root.Serialize(&buf);

	string final_path = path;

	final_path += ".meta";

	App->file_system->Save(final_path.data(), buf, size);

	delete[] buf;
}

void ModuleResourceManager::GenerateMetaFileMesh(const char * path, uint uuid, string library_path, const vector<uint>& meshes_uuids, const vector<uint>& animations_uuids, const vector<uint>& bones_uuids) const
{
	Data root;
	root.AppendUInt("Type", static_cast<unsigned int>(FileType::MESH));
	root.AppendUInt("UUID", uuid);
	char* md5 = App->file_system->GetMD5(path);
	if (md5)
		root.AppendString("md5", md5);
	char* md5_lib = App->file_system->GetMD5(library_path.data());
	if (md5_lib)
		root.AppendString("md5_lib", md5_lib);
	else
	{
		LOG("[ERROR] While saving meta file. The library file is not created yet. File %s - Library %s", path, library_path.data());
		App->editor->DisplayWarning(WarningType::W_ERROR, "While saving meta file. The library file is not created yet. File %s - Library %s", path, library_path.data());
	}
	root.AppendString("library_path", library_path.data());
	root.AppendString("original_file", path);

	root.AppendArray("meshes");

	for (vector<unsigned int>::const_iterator it = meshes_uuids.begin(); it != meshes_uuids.end(); ++it)
	{
		Data msh_data;
		msh_data.AppendUInt("uuid", *it);
		root.AppendArrayValue(msh_data);
	}

	root.AppendArray("animations");
	for (vector<unsigned int>::const_iterator it = animations_uuids.begin(); it != animations_uuids.end(); ++it)
	{
		Data anim_data;
		anim_data.AppendUInt("uuid", *it);
		root.AppendArrayValue(anim_data);
	}

	root.AppendArray("bones");
	for (vector<unsigned int>::const_iterator it = bones_uuids.begin(); it != bones_uuids.end(); ++it)
	{
		Data bone_data;
		bone_data.AppendUInt("uuid", *it);
		root.AppendArrayValue(bone_data);
	}

	char* buf;
	size_t size = root.Serialize(&buf);

	string final_path = path;

	final_path += ".meta";

	App->file_system->Save(final_path.data(), buf, size);

	delete[] buf;
}

void ModuleResourceManager::ChangeAssetsMD5MetaFile(const char * meta_path, const string & md5) const
{
	char* buffer = nullptr;
	int size = App->file_system->Load(meta_path, &buffer);
	if (size != -1)
	{
		Data meta(buffer);
		meta.AppendString("md5", md5.data());
		char* m_buffer = nullptr;
		int m_size = meta.Serialize(&m_buffer);
		App->file_system->Save(meta_path, m_buffer, m_size);
		if (m_buffer) delete[] m_buffer;
	}
	if (buffer)
		delete[] buffer;
}

void ModuleResourceManager::ImportFolder(const char * path, vector<tmp_mesh_file>& list_meshes, string base_dir, string base_library_dir) const
{
	vector<string> files, folders;
	App->file_system->GetFilesAndDirectoriesOutside(path, folders, files);
	string directory_path = path;

	//Create Folder metadata
	uint uuid = App->rnd->RandomInt();
	string folder_assets_path;
	if (base_dir.size() == 0)
		folder_assets_path = App->editor->GetAssetsCurrentDir() + App->file_system->GetNameFromPath(path);
	else
		folder_assets_path = base_dir + App->file_system->GetNameFromPath(path);
	App->file_system->GenerateDirectory(folder_assets_path.data());
	string library_path;
	if (base_library_dir.size() == 0)
		library_path = LIBRARY_FOLDER;
	else
		library_path = base_library_dir;

	//Create Folder at Library
	library_path += std::to_string(uuid);
	App->file_system->GenerateDirectory(library_path.data());

	GenerateMetaFile(folder_assets_path.data(), FOLDER, uuid, library_path, false);

	directory_path += "/"; //Add folder ending manually
	folder_assets_path += "/";
	library_path += "/";

	//Import files
	string file_path;
	for (vector<string>::const_iterator file = files.begin(); file != files.end(); ++file)
	{
		file_path = directory_path + (*file);
		if (GetFileExtension(file->data()) == MESH)
		{
			tmp_mesh_file tmp_file;
			tmp_file.mesh_path = file_path;
			tmp_file.assets_folder = folder_assets_path;
			tmp_file.library_folder = library_path;

			list_meshes.push_back(tmp_file);
		}
		else
		{
			ImportFile(file_path.data(), folder_assets_path, library_path);
		}
	}

	//Import folders
	string folder_path;
	for (vector<string>::const_iterator folder = folders.begin(); folder != folders.end(); ++folder)
	{
		folder_path = directory_path + (*folder);
		ImportFolder(folder_path.data(), list_meshes, folder_assets_path, library_path);
	}
}

void ModuleResourceManager::CreateFolder(const char* assets_path, string& base_library_path) const
{
	//An existing folder must be located in Assets.
	uint uuid = App->rnd->RandomInt();

	string library_path;
	if (base_library_path.size() == 0)
		library_path = LIBRARY_FOLDER;
	else
		library_path = base_library_path;

	//Create Folder at Library
	library_path += std::to_string(uuid) + "/";
	App->file_system->GenerateDirectory(library_path.data());

	GenerateMetaFile(assets_path, FOLDER, uuid, library_path, false);

	base_library_path = library_path;
}

void ModuleResourceManager::NameFolderUpdate(const string &meta_file, const string &meta_path, const string &old_folder_name, const string &new_folder_name, bool is_file) const
{
	char *buf;
	if (App->file_system->Load((meta_path + meta_file).c_str(), &buf) > 0)
	{
		// Once loaded, deleting meta file associated by older folder
		App->file_system->Delete((meta_path + meta_file).c_str());

		Data meta(buf);
		unsigned int type = meta.GetUInt("Type");
		unsigned int uuid = meta.GetUInt("UUID");
		double time_mod = meta.GetDouble("time_mod");
		const char *lib_path = meta.GetString("library_path");

		string original_path = meta.GetString("original_file");
		size_t pos = original_path.find(old_folder_name);
		original_path.replace(pos, old_folder_name.length(), new_folder_name);

		// Generating new meta folder file with new path
		if ((FileType)type != FileType::MESH)
			GenerateMetaFile(original_path.c_str(), (FileType)type, uuid, lib_path, is_file);
		else
		{
			vector<unsigned int> meshes_uuids, animations_uuids, bones_uuids;

			if (meta.GetArray("meshes", 0).IsNull() == false)
			{
				for (int i = 0; i < meta.GetArraySize("meshes"); i++)
				{
					Data mesh_info = meta.GetArray("meshes", i);
					meshes_uuids.push_back(mesh_info.GetUInt("uuid"));
				}
			}
			else
			{
				LOG("[WARNING] Couldn't find meshes uuids in the meta file %s when renaming the folder", meta_path.data());
				App->editor->DisplayWarning(WarningType::W_WARNING, "Couldn't find meshes uuids in the meta file %s when renaming the folder", meta_path.data());
			}


			if (meta.GetArray("animations", 0).IsNull() == false)
			{
				for (int i = 0; i < meta.GetArraySize("animations"); i++)
				{
					Data animation_info = meta.GetArray("animations", i);
					animations_uuids.push_back(animation_info.GetUInt("uuid"));
				}
			}
			else
			{
				LOG("[WARNING] Couldn't find animations uuids in the meta file %s when renaming the folder", meta_path.data());
				App->editor->DisplayWarning(WarningType::W_WARNING, "Couldn't find animations uuids in the meta file %s when renaming the folder", meta_path.data());
			}


			if (meta.GetArray("bones", 0).IsNull() == false)
			{
				for (int i = 0; i < meta.GetArraySize("bones"); i++)
				{
					Data bones_info = meta.GetArray("bones", i);
					bones_uuids.push_back(bones_info.GetUInt("uuid"));
				}
			}
			else
			{
				LOG("[WARNING] Couldn't find bones uuids in the meta file %s when renaming the folder", meta_path.data());
				App->editor->DisplayWarning(WarningType::W_WARNING, "Couldn't find bones uuids in the meta file %s when renaming the folder", meta_path.data());
			}

			GenerateMetaFileMesh(original_path.c_str(), uuid, lib_path, meshes_uuids, animations_uuids, bones_uuids);

		}

		delete[] buf;
	}
}

bool ModuleResourceManager::ReadMetaFile(const char * path, unsigned int & type, unsigned int & uuid, string & md5, string& md5_lib, string & library_path, string & assets_path) const
{
	bool ret = false;

	char* buffer = nullptr;
	if (App->file_system->Load(path, &buffer) > 0)
	{
		ret = true;

		Data meta(buffer);
		type = meta.GetUInt("Type");
		uuid = meta.GetUInt("UUID");
		if ((FileType)type != FileType::FOLDER)
		{
			md5 = meta.GetString("md5");
			md5_lib = meta.GetString("md5_lib");
		}
		library_path = meta.GetString("library_path");
		assets_path = meta.GetString("original_file");
	}

	if (buffer)
		delete[] buffer;

	return ret;
}

void ModuleResourceManager::ImportFile(const char * path, string base_dir, string base_library_dir, unsigned int uuid) const
{
	//1-Make a copy of the file
	//2-Import to own file
	FileType type = GetFileExtension(path);
	switch (type)
	{
	case IMAGE:
		ImageDropped(path, base_dir, base_library_dir, uuid);
		break;
	case MESH:
		MeshDropped(path, base_dir, base_library_dir, uuid);
		break;
	case VERTEX:
		VertexDropped(path, base_dir, base_library_dir, uuid);
		break;
	case FRAGMENT:
		FragmentDropped(path, base_dir, base_library_dir, uuid);
		break;
	case SOUNDBANK:
		SoundbankDropped(path, base_dir, base_library_dir, uuid);
		break;
	case SCRIPTS_LIBRARY:
		ScriptLibraryDropped(path, base_dir, base_library_dir, uuid);
		break; 
	case SCENE:
		SceneDropped(path, base_dir, base_library_dir, uuid);
		break;
	case PREFAB:
		PrefabDropped(path, base_dir, base_library_dir, uuid);
		break;
	}
}

void ModuleResourceManager::ImageDropped(const char* path, string base_dir, string base_library_dir, unsigned int id) const
{
	string file_assets_path;
	if (App->file_system->Exists(path) == false)
		file_assets_path = CopyOutsideFileToAssetsCurrentDir(path, base_dir);
	else
		file_assets_path = path;

	uint uuid = (id == 0) ? App->rnd->RandomInt() : id;

	string final_image_path;
	if (base_library_dir.size() == 0)
		final_image_path = LIBRARY_FOLDER;
	else
		final_image_path = base_library_dir;

	final_image_path += "/" + std::to_string(uuid) + "/";
	App->file_system->GenerateDirectory(final_image_path.data());
	final_image_path += std::to_string(uuid);
	final_image_path += ".dds";

	TextureImporter::Import(final_image_path.data(), file_assets_path.data());
	GenerateMetaFile(file_assets_path.data(), IMAGE, uuid, final_image_path);
}

void ModuleResourceManager::MeshDropped(const char * path, string base_dir, string base_library_dir, unsigned int id) const
{
	string file_assets_path;
	if (App->file_system->Exists(path) == false)
		file_assets_path = CopyOutsideFileToAssetsCurrentDir(path, base_dir);
	else
		file_assets_path = path;

	uint uuid = (id == 0) ? App->rnd->RandomInt() : id;

	//Create the link to Library
	string final_mesh_path;
	if (base_library_dir.size() == 0)
		final_mesh_path = LIBRARY_FOLDER;
	else
		final_mesh_path = base_library_dir;

	final_mesh_path += std::to_string(uuid) + "/";
	App->file_system->GenerateDirectory(final_mesh_path.data());
	string library_dir = final_mesh_path;
	final_mesh_path += std::to_string(uuid) + ".inf";

	vector<unsigned int> meshes_uuids;
	vector<unsigned int> animations;
	vector<unsigned int> bones;
	MeshImporter::Import(final_mesh_path.data(), file_assets_path.data(), library_dir.data(), meshes_uuids, animations, bones);
	GenerateMetaFileMesh(file_assets_path.data(), uuid, final_mesh_path, meshes_uuids, animations, bones);

}

void ModuleResourceManager::VertexDropped(const char * path, string base_dir, string base_library_dir, unsigned int id) const
{
	string file_assets_path;
	if (App->file_system->Exists(path) == false)
		file_assets_path = CopyOutsideFileToAssetsCurrentDir(path, base_dir);
	else
		file_assets_path = path;

	uint uuid = (id == 0) ? App->rnd->RandomInt() : id;
	string final_fragment_path = base_library_dir;
	final_fragment_path += std::to_string(uuid) + "/";
	App->file_system->GenerateDirectory(final_fragment_path.data());

	string library_dir = final_fragment_path;
	final_fragment_path += std::to_string(uuid) + ".ver";

	App->file_system->DuplicateFile(file_assets_path.data(), final_fragment_path.data());
	GenerateMetaFile(file_assets_path.data(), FileType::VERTEX, uuid, final_fragment_path);
	bool success = ShaderCompiler::TryCompileVertex(final_fragment_path.data());
	if (success)
		LOG("Vertex shader %s compiled correctly.", path);
}

void ModuleResourceManager::FragmentDropped(const char * path, string base_dir, string base_library_dir, unsigned int id) const
{
	string file_assets_path;
	if (App->file_system->Exists(path) == false)
		file_assets_path = CopyOutsideFileToAssetsCurrentDir(path, base_dir);
	else
		file_assets_path = path;

	uint uuid = (id == 0) ? App->rnd->RandomInt() : id;
	string final_fragment_path = base_library_dir;
	final_fragment_path += std::to_string(uuid) + "/";
	App->file_system->GenerateDirectory(final_fragment_path.data());

	string library_dir = final_fragment_path;
	final_fragment_path += std::to_string(uuid) + ".fra";

	App->file_system->DuplicateFile(file_assets_path.data(), final_fragment_path.data());
	GenerateMetaFile(file_assets_path.data(), FileType::FRAGMENT, uuid, final_fragment_path);
	bool success = ShaderCompiler::TryCompileVertex(final_fragment_path.data());
	if (success)
		LOG("Fragment shader %s compiled correctly.", path);
}

void ModuleResourceManager::SoundbankDropped(const char * path, string base_dir, string base_library_dir, unsigned int id) const
{
	string file_assets_path;
	if (App->file_system->Exists(path) == false)
		file_assets_path = CopyOutsideFileToAssetsCurrentDir(path, base_dir);
	else
		file_assets_path = path;

	uint uuid = (id == 0) ? App->rnd->RandomInt() : id;

	string lib_soundbank_path = base_library_dir;
	lib_soundbank_path += std::to_string(uuid) + "/";
	App->file_system->GenerateDirectory(lib_soundbank_path.data());

	string lib_json_path = lib_soundbank_path;
	// Soundbank metainfo	
	lib_soundbank_path += std::to_string(uuid) + ".bnk";
	App->file_system->DuplicateFile(file_assets_path.data(), lib_soundbank_path.data());
	GenerateMetaFile(file_assets_path.data(), FileType::SOUNDBANK, uuid, lib_soundbank_path);

	// JSON metainfo
	string json_file_path = file_assets_path.substr(0, file_assets_path.find_last_of('.')) + ".json";
	lib_json_path += std::to_string(uuid) + ".json";
	App->file_system->DuplicateFile(json_file_path.data(), lib_json_path.data());
}

void ModuleResourceManager::ScriptLibraryDropped(const char * path, string base_dir, string base_library_dir, unsigned int id) const
{
	string file_assets_path;
	if (App->file_system->Exists(path) == false)
		file_assets_path = CopyOutsideFileToAssetsCurrentDir(path, base_dir);
	else
		file_assets_path = path;

	uint uuid = (id == 0) ? App->rnd->RandomInt() : id;
	string script_library_path = base_library_dir;
	script_library_path += std::to_string(uuid) + "/";
	App->file_system->GenerateDirectory(script_library_path.data());

	// ScriptLibrary metainfo	
	script_library_path += std::to_string(uuid) + ".dll";
	App->file_system->DuplicateFile(file_assets_path.data(), script_library_path.data());
	GenerateMetaFile(file_assets_path.data(), FileType::SCRIPTS_LIBRARY, uuid, script_library_path);
}

void ModuleResourceManager::SceneDropped(const char * path, std::string base_dir, std::string base_library_dir, unsigned int id) const
{
	string file_assets_path;
	if (App->file_system->Exists(path) == false)
		file_assets_path = CopyOutsideFileToAssetsCurrentDir(path, base_dir);
	else
		file_assets_path = path;

	uint uuid = (id == 0) ? App->rnd->RandomInt() : id;
	string final_scene_path = base_library_dir;
	final_scene_path += std::to_string(uuid) + "/";
	App->file_system->GenerateDirectory(final_scene_path.data());

	string library_dir = final_scene_path;
	final_scene_path += std::to_string(uuid) + ".ezx";

	App->file_system->DuplicateFile(file_assets_path.data(), final_scene_path.data());
	GenerateMetaFile(file_assets_path.data(), FileType::SCENE, uuid, final_scene_path);
}

void ModuleResourceManager::PrefabDropped(const char * path, std::string base_dir, std::string base_library_dir, unsigned int id) const
{
	string file_assets_path;
	if (App->file_system->Exists(path) == false)
		file_assets_path = CopyOutsideFileToAssetsCurrentDir(path, base_dir);
	else
		file_assets_path = path;

	uint uuid = (id == 0) ? App->rnd->RandomInt() : id;
	string final_prefab_path = base_library_dir;
	final_prefab_path += std::to_string(uuid) + "/";
	App->file_system->GenerateDirectory(final_prefab_path.data());

	string library_dir = final_prefab_path;
	final_prefab_path += std::to_string(uuid) + ".pfb";

	App->file_system->DuplicateFile(file_assets_path.data(), final_prefab_path.data());
	GenerateMetaFile(file_assets_path.data(), FileType::PREFAB, uuid, final_prefab_path);
}

GameObject* ModuleResourceManager::LoadPrefabFile(const string & library_path)
{
	GameObject* ret = nullptr;
	char* buffer = nullptr;
	uint size = App->file_system->Load(library_path.data(), &buffer);
	if (size == 0)
	{
		LOG("[ERROR] While loading prefab file %s", library_path.data());
		App->editor->DisplayWarning(WarningType::W_ERROR, "While loading prefab file %s", library_path.data());
		if (buffer)
			delete[] buffer;
		return ret;
	}

	Data scene(buffer);
	Data root_objects;
	root_objects = scene.GetArray("GameObjects", 0);
	map<unsigned int, unsigned int> uuids;
	if (root_objects.IsNull() == false)
	{
		for (size_t i = 0; i < scene.GetArraySize("GameObjects"); i++)
		{
			if (ret == nullptr)
			{
				ret = App->go_manager->LoadPrefabGameObject(scene.GetArray("GameObjects", i), uuids);
			}
			else
			{
				App->go_manager->LoadPrefabGameObject(scene.GetArray("GameObjects", i), uuids);
			}
		}
	}
	else
	{
		LOG("The %s is not a valid mesh/prefab file", library_path.data());
		App->editor->DisplayWarning(WarningType::W_ERROR, "The %s is not a valid mesh / prefab file", library_path.data());
	}

	delete[] buffer;
	return ret;
}

void ModuleResourceManager::CheckDirectoryModification(Directory * directory)
{
	//Note: only working for textures/vertex programs/fragment programs. Meshes, prefabs and scenes are ignored
	vector<string> files_to_replace;
	vector<AssetFile*> files_to_remove;
	vector<unsigned int>uuids;
	vector<string> vertex_to_replace;
	vector<string> fragment_to_replace;
	//Special case for fragment and vertex. If in use, needs to find the shader using it and recompile it.

	for (vector<AssetFile*>::iterator file = directory->files.begin(); file != directory->files.end(); ++file)
	{
		int mod_time = App->file_system->GetLastModificationTime((*file)->original_file.data());
		if (mod_time != (*file)->time_mod)
		{
			switch ((*file)->type)
			{
			case FileType::IMAGE:
				files_to_replace.push_back((*file)->original_file);
				files_to_remove.push_back(*file);
				uuids.push_back((*file)->uuid);
				break;
			case FileType::VERTEX:
				files_to_replace.push_back((*file)->original_file);
				files_to_remove.push_back(*file);
				uuids.push_back((*file)->uuid);
				vertex_to_replace.push_back((*file)->original_file);
				break;
			case FileType::FRAGMENT:
				files_to_replace.push_back((*file)->original_file);
				files_to_remove.push_back(*file);
				uuids.push_back((*file)->uuid);
				fragment_to_replace.push_back((*file)->original_file);
				break;
			}

		}
	}


	for (vector<AssetFile*>::iterator file = files_to_remove.begin(); file != files_to_remove.end(); ++file)
		App->editor->assets->DeleteMetaAndLibraryFile((*file));

	for (size_t i = 0; i < files_to_replace.size(); i++)
	{
		ImportFile(files_to_replace[i].data(), directory->path, directory->library_path, uuids[i]);
		ResourceFile* rc = FindResourceByUUID(uuids[i]);
		if (rc)
		{
			rc->UnLoadAll();
			rc->Reload();
		}
	}

	for (vector<string>::iterator ver = vertex_to_replace.begin(); ver != vertex_to_replace.end(); ++ver)
	{
		ResourceFileMaterial* mat = FindMaterialUsing(true, *ver);
		if (mat)
		{
			mat->UnLoadAll();
			mat->Reload();
		}
	}

	for (vector<string>::iterator fra = fragment_to_replace.begin(); fra != fragment_to_replace.end(); ++fra)
	{
		ResourceFileMaterial* mat = FindMaterialUsing(false, *fra);
		if (mat)
		{
			mat->UnLoadAll();
			mat->Reload();
		}
	}

	for (vector<Directory*>::iterator dir = directory->directories.begin(); dir != directory->directories.end(); ++dir)
	{
		CheckDirectoryModification(*dir);
	}
}

ResourceFileMaterial * ModuleResourceManager::FindMaterialUsing(bool vertex_program, const string & path) const
{
	vector<ResourceFile*> materials;
	FindAllResourcesByType(ResourceFileType::RES_MATERIAL, materials);
	ResourceFileMaterial* item;
	for (vector<ResourceFile*>::const_iterator it = materials.begin(); it != materials.end(); ++it)
	{
		item = (ResourceFileMaterial*)(*it);
		if (vertex_program)
		{
			if (item->material.vertex_path.compare(path) == 0)
				return item;
		}
		else //Fragment
		{
			if (item->material.fragment_path.compare(path) == 0)
				return item;
		}
	}

	return nullptr;
}

void ModuleResourceManager::FindAllResourcesByType(ResourceFileType type, vector<ResourceFile*>& result)const
{
	for (list<ResourceFile*>::const_iterator it = resource_files.begin(); it != resource_files.end(); ++it)
		if ((*it)->GetType() == type)
			result.push_back(*it);
}