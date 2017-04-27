#ifndef __MESH_IMPORTER_H__
#define __MESH_IMPORTER_H__

#include <string>
#include <stack>

struct aiMesh;
struct aiNode;
struct aiScene;
struct Mesh;

class Data;
class GameObject;

namespace MeshImporter
{
	//File is the final path of the imported file with the extension. Path is from the file is converted. Base path is the directory to save all generated files
	bool Import(const char* file, const char* path,const char* base_path, std::vector<unsigned int>& uuids, vector<unsigned int>& uuids_anim, vector<unsigned int>& uuids_bones);
	void ImportNode(aiNode* node, const aiScene* scene, GameObject* parent, std::vector<GameObject*>& created_go, std::vector<const aiMesh*>& boned_meshes, std::vector<const GameObject*>& boned_game_objects,
					std::string mesh_file_directory, std::string folder_path, std::vector<unsigned int>& uuids);
	bool ImportMesh(const aiMesh* mesh, const char* folder_path, std::string& output_name, unsigned int& uuid);
	bool Save(Mesh& mesh, const char* folder_path, std::string& output_name, unsigned int& uuid);

	//Same as above but specifying the uuid
	bool ImportUUID(const char* file, const char* path, const char* base_path, std::stack<unsigned int>& uuids, std::vector<uint>& uuids_anim, std::vector<uint>& uuids_bones);
	void ImportNodeUUID(aiNode* node, const aiScene* scene, GameObject* parent, std::vector<GameObject*>& created_go, std::vector<const aiMesh*>& boned_meshes, std::vector<const GameObject*>& boned_game_objects,
						std::string mesh_file_directory, std::string folder_path, std::stack<unsigned int>& uuids);
	bool ImportMeshUUID(const aiMesh* mesh, const char* folder_path, std::string& output_name, unsigned int uuid);
	bool SaveUUID(Mesh& mesh, const char* folder_path, std::string& output_name, unsigned int uuid);

	Mesh* Load(const char* path);
	void LoadBuffers(Mesh* mesh);
	void DeleteBuffers(Mesh* mesh);

	void CollectGameObjects(GameObject* root, std::vector<GameObject*> vector);
	void SaveInfoFile(std::vector<GameObject*> vector, const char* file);
	void SaveGameObjectInfo(GameObject* gameObject, Data& data);

	void LoadAnimBuffers(float* weights, int weights_size, unsigned int& weights_id, int* bones_ids, int bones_ids_size, unsigned int& bones_ids_id);

	Mesh* LoadBillboardMesh();
	Mesh* LoadQuad();

}

#endif // !__MESH_IMPORTER_H_
