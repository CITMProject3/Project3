#ifndef __MODULE_RESOURCE_MANAGER_H__
#define __MODULE_RESOURCE_MANAGER_H__

#include "Module.h"
#include "ResourceFile.h"
#include "Material.h"

#include <list>
#include <string>
#include <vector>

#define CHECK_MOD_TIME 3

enum FileType
{
	NONE,
	IMAGE,
	MESH,
	FOLDER,
	PREFAB,
	SCENE,
	VERTEX,
	FRAGMENT,
	MATERIAL,
	RENDER_TEXTURE,
	SOUNDBANK
};

struct tmp_mesh_file
{
	std::string mesh_path;
	std::string assets_folder;
	std::string library_folder;
};

struct tmp_mesh_file_uuid
{
	std::string mesh_path;
	std::string assets_folder;
	std::string library_folder;
	unsigned int uuid;
	std::string meta_path;
};

struct Directory;
class ResourceFileMaterial;

class ModuleResourceManager : public Module
{
public:
	ModuleResourceManager(const char* name, bool start_enabled = true);
	~ModuleResourceManager();

	bool Init(Data& config);
	bool Start();
	update_status Update();
	bool CleanUp();

	void FileDropped(const char* file_path);
	void LoadFile(const std::string& library_path, const FileType& type);

	ResourceFile* LoadResource(const std::string& path, ResourceFileType type);
	//Deprecated
	void UnloadResource(const std::string& path);
	void RemoveResourceFromList(ResourceFile* file);
	ResourceFile* FindResourceByUUID(unsigned int uuid);
	ResourceFile* FindResourceByLibraryPath(const std::string& library);

	void SaveScene(const char* file_name, std::string base_library_path);
	bool LoadScene(const char* file_name);
	void SavePrefab(GameObject* gameobject);

	void SaveMaterial(const Material& material, const char* path, uint uuid = 0);
	unsigned int GetDefaultShaderId()const;

	//Returns the path of the file in library
	std::string FindFile(const std::string& assets_file_path)const;

	ResourceFileType GetResourceType(const std::string& path)const;
	int GetNumberResources()const;
	int GetNumberTexures()const;
	int GetNumberMeshes()const;
	int GetTotalBytesInMemory()const;
	int GetTextureBytes()const;
	int GetMeshBytes()const;

	void CreateRenderTexture(const std::string& assets_path, const std::string& library_path);
	void SaveRenderTexture(const std::string& assets_path, const std::string& library_path, int width, int height, bool use_depth_as_texture)const;

	void CreateFolder(const char* assets_path, std::string& base_library_path) const;
	void NameFolderUpdate(const std::string &meta_file, const std::string &meta_path, const std::string &old_folder_name, const std::string &new_folder_name, bool is_file = false) const;

	bool ReadMetaFile(const char* path, unsigned int& type, unsigned int& uuid, double& time_mod, std::string& library_path, std::string& assets_path)const;

private:

	FileType GetFileExtension(const char* path)const;
	std::string CopyOutsideFileToAssetsCurrentDir(const char* path, std::string base_dir = std::string())const;

	void GenerateMetaFile(const char* path, FileType type, uint uuid, std::string library_path, bool is_file = true)const;
	void GenerateMetaFileMesh(const char* path, uint uuid, std::string library_path, const std::vector<unsigned int>& meshes_uuids, const std::vector<uint>& animations_uuids, const std::vector<uint>& bones_uuids)const;

	void ImportFolder(const char* path, std::vector<tmp_mesh_file>& list_meshes, std::string base_dir = std::string(), std::string base_library_dir = std::string())const;
	void ImportFile(const char* path, std::string base_dir = std::string(), std::string base_library_dir = std::string(), unsigned int uuid = 0)const;

	void ImageDropped(const char* path, std::string base_dir = std::string(), std::string base_library_dir = std::string(), unsigned int uuid = 0)const;
	void MeshDropped(const char* path, std::string base_dir = std::string(), std::string base_library_dir = std::string(), unsigned int uuid = 0)const;
	void VertexDropped(const char* path, std::string base_dir = std::string(), std::string base_library_dir = std::string(), unsigned int uuid = 0)const;
	void FragmentDropped(const char* path, std::string base_dir = std::string(), std::string base_library_dir = std::string(), unsigned int uuid = 0)const;
	void SoundbankDropped(const char* path, std::string base_dir = std::string(), std::string base_library_dir = std::string(), unsigned int uuid = 0)const;

	void LoadPrefabFile(const std::string& library_path);

	void CheckDirectoryModification(Directory* directory);

	//If vertex program is false it will find the fragment program.
	ResourceFileMaterial* FindMaterialUsing(bool vertex_program, const std::string& path)const;
	void FindAllResourcesByType(ResourceFileType type, std::vector<ResourceFile*>& result)const;

	void UpdateAssetsAuto();
	void UpdateAssetsAutoRecursive(const std::string& assets_dir, const std::string& library_dir, std::vector<tmp_mesh_file>& mesh_files);
	void UpdateFileWithMeta(const std::string& meta_file, const std::string& base_assets_dir, const std::string& base_lib_dir);
	void ImportFileWithMeta(unsigned int type, unsigned int uuid, std::string library_path, std::string assets_path, const std::string& base_assets_dir, const std::string& base_lib_dir, const std::string& meta_path);
	void ImportMeshFileWithMeta(const char* path,const std::string& base_dir,const std::string& base_library_dir, unsigned int uuid, const std::string& meta_path);
	std::string UpdateFolderWithMeta(const std::string& meta_path);

private:
	std::list<ResourceFile*> resource_files;
	float modification_timer = 0.0f;

	unsigned int num_textures = 0;
	unsigned int num_meshes = 0;
	unsigned int bytes_in_memory = 0;
	unsigned int texture_bytes = 0;
	unsigned int mesh_bytes = 0;

	unsigned int default_shader = -1;

	std::vector<tmp_mesh_file_uuid> tmp_mesh_uuid_files;

};
#endif // !__MODULE_RESOURCE_MANAGER_H__