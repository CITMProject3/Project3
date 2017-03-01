#ifndef __MESH_IMPORTER_H__
#define __MESH_IMPORTER_H__

#include <string>

struct aiMesh;
struct aiNode;
struct aiScene;
struct Mesh;
class Data;

namespace MeshImporter
{
	//File is the final path of the imported file with the extension. Path is from the file is converted. Base path is the directory to save all generated files
	bool Import(const char* file, const char* path,const char* base_path);
	void ImportNode(aiNode* node, const aiScene* scene, GameObject* parent, std::vector<GameObject*>& created_go, std::vector<const aiMesh*>& boned_meshes, std::vector<const GameObject*>& boned_game_objects,
					std::string mesh_file_directory, std::string folder_path);
	bool ImportMesh(const aiMesh* mesh, const char* folder_path, std::string& output_name);
	bool Save(Mesh& mesh, const char* folder_path, std::string& output_name);
	Mesh* Load(const char* path);
	void LoadBuffers(Mesh* mesh);
	void DeleteBuffers(Mesh* mesh);

	void CollectGameObjects(GameObject* root, std::vector<GameObject*> vector);
	void SaveInfoFile(std::vector<GameObject*> vector, const char* file);
	void SaveGameObjectInfo(GameObject* gameObject, Data& data);

}

#endif // !__MESH_IMPORTER_H_
