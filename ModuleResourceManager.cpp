#include "Application.h"
#include "ModuleResourceManager.h"
#include "TextureImporter.h"
#include "MeshImporter.h"
#include "Random.h"
#include "Data.h"
#include "ResourceFileMesh.h"
#include "ResourceFileTexture.h"
#include "ModuleGOManager.h"
#include "GameObject.h"
#include "Assets.h"
#include "ShaderComplier.h"
#include "ResourceFileMaterial.h"
#include "ResourceFileRenderTexture.h"
#include "RenderTexEditorWindow.h"

#include "Glew\include\glew.h"
#include <gl\GL.h>

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
#include "MemLeaks.h"
#include <map>

ModuleResourceManager::ModuleResourceManager(const char* name, bool start_enabled) : Module(name, start_enabled)
{
}

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
	default_shader = ShaderCompiler::LoadDefaultShader();
	return true;
}

update_status ModuleResourceManager::Update()
{
	//TODO:Only do this in editor mode. NOT in game
	modification_timer += time->RealDeltaTime();

	if (modification_timer >= CHECK_MOD_TIME)
	{
		CheckDirectoryModification(App->editor->assets->root);
		modification_timer = 0.0f;
		App->editor->assets->Refresh();
	}
	return UPDATE_CONTINUE;
}

bool ModuleResourceManager::CleanUp()
{
	ilShutDown();
	aiDetachAllLogStreams();
	return true;
}

void ModuleResourceManager::FileDropped(const char * file_path)
{
	//Files extensions accepted
	//Images: PNG TGA
	//Meshes: FBX / OBJ
	//Audio: PENDING

	if (App->file_system->IsDirectoryOutside(file_path))
	{
		vector<tmp_mesh_file> mesh_files;
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

void ModuleResourceManager::LoadFile(const string & library_path, const FileType & type)
{
	switch (type)
	{
	case MESH:
		LoadPrefabFile(library_path);
		break;
	case PREFAB:
		LoadPrefabFile(library_path);
		break;
	}
}

ResourceFile * ModuleResourceManager::LoadResource(const string & path, ResourceFileType type)
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

void ModuleResourceManager::RemoveResourceFromList(ResourceFile * file)
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

	Data root_node;
	root_node.AppendArray("GameObjects");

	App->go_manager->root->Save(root_node);
	
	char* buf;
	size_t size = root_node.Serialize(&buf);

	//Add extension if doesn't have it yet
	if (name_to_save.find(".ezx", name_to_save.length() - 4) == string::npos)
		name_to_save += ".ezx";

	App->file_system->Save(name_to_save.data(), buf, size);
	

	string meta_file = name_to_save.substr(0, name_to_save.length() - 4) + ".meta";
	if (App->file_system->Exists(meta_file.data()))
	{
		char* meta_buf;
		int meta_size = App->file_system->Load(meta_file.data(), &meta_buf);

		if (meta_size > 0)
		{
			Data meta_data(meta_buf);
			string library_path = meta_data.GetString("library_path");
			App->file_system->Save(library_path.data(), buf, size);
		}
		else
		{
			LOG("Error while opening the meta file(%s) of %s", meta_file.data(), name_to_save.data());
		}

		delete[] meta_buf;
	}
	else
	{
		unsigned int uuid = App->rnd->RandomInt();
		string library_dir = base_library_path + "/" + std::to_string(uuid) + "/";
		App->file_system->GenerateDirectory(library_dir.data());
		string library_filename = library_dir + std::to_string(uuid) + ".ezx";
		GenerateMetaFile(name_to_save.data(), FileType::SCENE, uuid, library_filename.data());
		App->file_system->Save(library_filename.data(), buf, size); //Duplicate the file in library
	}

	delete[] buf;
	App->editor->RefreshAssets();
}

bool ModuleResourceManager::LoadScene(const char * file_name)
{
	bool ret = false;
	//TODO: Now the current scene is destroyed. Ask the user if wants to save the changes.

	char* buffer = nullptr;
	uint size = App->file_system->Load(file_name, &buffer);
	if (size == 0)
	{
		LOG("Error while loading Scene: %s", file_name);
		if (buffer)
			delete[] buffer;
		return false;
	}

	Data scene(buffer);
	Data root_objects;
	root_objects = scene.GetArray("GameObjects", 0);

	if (root_objects.IsNull() == false)
	{
		//Remove the current scene
		App->go_manager->ClearScene();

		for (int i = 0; i < scene.GetArraySize("GameObjects"); i++)
		{
			if (i == 0)
				App->go_manager->root = App->go_manager->LoadGameObject(scene.GetArray("GameObjects", i));
			else
				App->go_manager->LoadGameObject(scene.GetArray("GameObjects", i));
		}
		App->go_manager->SetCurrentScenePath(file_name);
		ret = true;
	}
	else
	{
		LOG("The scene %s is not a valid scene file", file_name);
	}

	delete[] buffer;

	return ret;
}

void ModuleResourceManager::SavePrefab(GameObject * gameobject)
{
	//Create the file
	//Create the meta
	//Duplicate file in assets

	Data root_node;
	root_node.AppendArray("GameObjects");
	GameObject* parent = gameobject->GetParent();
	gameobject->SetParent(nullptr);
	gameobject->SetAsPrefab();
	gameobject->Save(root_node);
	char* buf;
	size_t size = root_node.Serialize(&buf);

	string name = App->editor->assets->CurrentDirectory() + gameobject->name.data();
	name += ".pfb";
	App->file_system->Save(name.data(), buf, size);

	string meta_file = name.substr(0, name.length() - 4) + ".meta";
	if (App->file_system->Exists(meta_file.data()))
	{
		char* meta_buf;
		int meta_size = App->file_system->Load(meta_file.data(), &meta_buf);

		if (meta_size > 0)
		{
			Data meta_data(meta_buf);
			string library_path = meta_data.GetString("library_path");
			App->file_system->Save(library_path.data(), buf, size);
		}
		else
		{
			LOG("Error while opening the meta file(%s) of %s", meta_file.data(), name.data());
		}

		delete[] meta_buf;
	}
	else
	{
		unsigned int uuid = App->rnd->RandomInt();
		string library_dir = App->editor->assets->CurrentLibraryDirectory() + "/" + std::to_string(uuid) + "/";
		App->file_system->GenerateDirectory(library_dir.data());
		string library_filename = library_dir + std::to_string(uuid) + ".pfb";
		GenerateMetaFile(name.data(), FileType::PREFAB, uuid, library_filename.data());
		App->file_system->Save(library_filename.data(), buf, size); 
	}

	delete[] buf;
	gameobject->SetParent(parent);
}

void ModuleResourceManager::SaveMaterial(const Material & material, const char * path, uint _uuid)
{
	material.Save(path);
	uint uuid = (_uuid == 0) ? App->rnd->RandomInt() : _uuid;
	string assets_folder = path;
	assets_folder = assets_folder.substr(0, assets_folder.find_last_of("/\\")+1);
	string library_path = App->editor->assets->FindLibraryDirectory(assets_folder);
	library_path = library_path + "/" + std::to_string(uuid) + "/";
	App->file_system->GenerateDirectory(library_path.data());
	library_path = library_path + std::to_string(uuid) + ".mat";
	GenerateMetaFile(path, FileType::MATERIAL, uuid, library_path);
	material.Save(library_path.data());
}

unsigned int ModuleResourceManager::GetDefaultShaderId() const
{
	return default_shader;
}

string ModuleResourceManager::FindFile(const string & assets_file_path)
{
	string ret;

	string meta = assets_file_path.substr(0, assets_file_path.length() - 4);
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
		LOG("Could not find file %s", assets_file_path.data());
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
	GenerateMetaFile(assets_name.data(), FileType::RENDER_TEXTURE, uuid, library_name);
	App->file_system->Save(library_name.data(), buffer, size);

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
	char* mesh_extensions[] = { "fbx", "FBX", "obj", "OBJ"};
	char* image_extensions[] = {"png", "PNG", "tga", "TGA"};
	char* scene_extension = "ezx";
	char* vertex_extension = "ver";
	char* fragment_extension = "fra";
	char* render_texture_extension = "rtx";

	string name = path;
	string extension = name.substr(name.find_last_of(".") + 1);

	for (int i = 0; i < 4; i++)
		if (extension.compare(mesh_extensions[i]) == 0)
			return FileType::MESH;

	for (int i = 0; i < 4; i++)
		if (extension.compare(image_extensions[i]) == 0)
			return FileType::IMAGE;

	if (extension.compare(scene_extension) == 0)
		return FileType::SCENE;

	if (extension.compare(vertex_extension) == 0)
		return FileType::VERTEX;

	if (extension.compare(fragment_extension) == 0)
		return FileType::FRAGMENT;

	if (extension.compare(render_texture_extension) == 0)
		return FileType::RENDER_TEXTURE;
	
	return NONE;
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

void ModuleResourceManager::GenerateMetaFile(const char * path, FileType type, uint uuid, string library_path, bool is_file)const
{
	Data root;
	root.AppendUInt("Type", static_cast<unsigned int>(type));
	root.AppendUInt("UUID", uuid);
	root.AppendDouble("time_mod", App->file_system->GetLastModificationTime(path));
	root.AppendString("library_path", library_path.data());
	root.AppendString("original_file", path);

	char* buf;
	size_t size = root.Serialize(&buf);

	string final_path = path;
	if(is_file)
		final_path = final_path.substr(0, final_path.length() - 4); //Substract extension. Note: known is: ".png" ".fbx", etc. (4 char)
	final_path += ".meta";

	App->file_system->Save(final_path.data(), buf, size);

	delete[] buf;
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

	final_image_path += std::to_string(uuid) + "/";
	App->file_system->GenerateDirectory(final_image_path.data());
	final_image_path += std::to_string(uuid);
	final_image_path += ".dds";

	GenerateMetaFile(file_assets_path.data(), IMAGE, uuid, final_image_path);

	TextureImporter::Import(final_image_path.data(), file_assets_path.data());
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

	GenerateMetaFile(file_assets_path.data(), FileType::MESH, uuid, final_mesh_path);
	MeshImporter::Import(final_mesh_path.data(), file_assets_path.data(), library_dir.data());
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

	GenerateMetaFile(file_assets_path.data(), FileType::VERTEX, uuid, final_fragment_path);
	App->file_system->DuplicateFile(file_assets_path.data(), final_fragment_path.data()); 
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

	GenerateMetaFile(file_assets_path.data(), FileType::FRAGMENT, uuid, final_fragment_path);
	App->file_system->DuplicateFile(file_assets_path.data(), final_fragment_path.data()); 
	bool success = ShaderCompiler::TryCompileVertex(final_fragment_path.data());
	if (success)
		LOG("Fragment shader %s compiled correctly.", path);
}

void ModuleResourceManager::LoadPrefabFile(const string & library_path)
{
	char* buffer = nullptr;
	uint size = App->file_system->Load(library_path.data(), &buffer);
	if (size == 0)
	{
		LOG("Error while loading: %s", library_path.data());
		if (buffer)
			delete[] buffer;
		return;
	}

	Data scene(buffer);
	Data root_objects;
	root_objects = scene.GetArray("GameObjects", 0);
	map<unsigned int, unsigned int> uuids;
	if (root_objects.IsNull() == false)
	{
		for (int i = 0; i < scene.GetArraySize("GameObjects"); i++)
		{
			App->go_manager->LoadPrefabGameObject(scene.GetArray("GameObjects", i), uuids);
		}
	}
	else
	{
		LOG("The %s is not a valid mesh/prefab file", library_path.data());
	}

	delete[] buffer;
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

	for (int i = 0; i < files_to_replace.size(); i++)
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
