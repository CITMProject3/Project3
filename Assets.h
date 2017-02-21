
#ifndef __ASSETS_H__
#define __ASSETS_H__

#include "Window.h"
#include "ModuleResourceManager.h"
#include <vector>

struct Directory;

struct AssetFile
{
	FileType type;
	std::string name; //To display
	std::string file_path;
	std::string content_path;
	std::string original_file;
	unsigned int uuid;
	int time_mod;
	Directory* directory = nullptr;
};

struct Directory
{
	std::string name; //To display
	std::string path; //Path from Assets to the current directory
	std::vector<Directory*> directories;
	std::vector<AssetFile*> files;
	std::string library_path;
	Directory* parent = nullptr;
};

class Assets : public Window
{
public:
	Assets();
	~Assets();

	void Draw();
	void Refresh();
	string CurrentDirectory()const;
	string CurrentLibraryDirectory()const;
	string FindLibraryDirectory(const string& assets_dir)const;

	void DeleteAssetFile(AssetFile* file);
	void DeleteMetaAndLibraryFile(AssetFile* file);
	
	void GetAllFilesByType(FileType type, vector<string>& result)const;

private:

	void Init();
	void CleanUp();
	void FillDirectoriesRecursive(Directory* root_dir);
	void DeleteDirectoriesRecursive(Directory* root_dir, bool keep_root = false);
	Directory* FindDirectory(const string& dir)const;
	AssetFile* FindAssetFile(const string& file);

	bool IsMeshExtension(const std::string& file_name)const;
	bool IsSceneExtension(const std::string& file_name)const;

	void OpenInExplorer(const std::string* file = NULL)const;

	void GeneralOptions();
	void MeshFileOptions();
	void SceneFileOptions();
	void DirectoryOptions();
	void PrefabFileOptions();
	void MaterialFileOptions();
	void RenderTextureOptions();
	void VertexFragmentOptions();

	void DeleteAssetDirectory(Directory* directory);

public:
	Directory* root = nullptr; //Assets directory
private:
	Directory* current_dir = root;
	Directory* dir_selected = nullptr;
	AssetFile* file_selected = nullptr;
	//Icons
	uint folder_id;
	uint file_id;
	uint mesh_id;
	uint scene_id;
	uint prefab_id;
	uint vertex_id;
	uint fragment_id;
	uint material_id;

};

#endif
