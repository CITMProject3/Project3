#ifndef __ANIMATION_IMPORTER_H__
#define __ANIMATION_IMPORTER_H__

#include <string>
#include <map>

#include "MathGeoLib\include\MathGeoLib.h"
#include "ResourceFileAnimation.h"

struct aiScene;
struct aiAnimation;
struct aiBone;
struct aiNodeAnim;

class GameObject;
class ResourceFileAnimation;

namespace AnimationImporter
{
	//File is the final path of the imported file with the extension. Path is from the file is converted. Base path is the directory to save all generated files
	bool ImportSceneAnimations(const aiScene* scene, GameObject* root, const char* base_path, std::string& output_name);
	bool ImportAnimation(const aiAnimation* anim, const char* base_path, std::string& output_name);
	void ImportChannel(const aiNodeAnim* node, Channel& channel);

	bool Save(const ResourceFileAnimation& anim, const char* folder_path, std::string& output_name);
	void SaveChannelData(const Channel& channel, char** cursor);
	void SaveKeys(const std::map<double, float3>&, char** cursor);
	void SaveKeys(const std::map<double, Quat>&, char** cursor);

	uint CalcChannelSize(const Channel& channel);
	//void ImportNode(aiNode* node, const aiScene* scene, GameObject* parent, std::string mesh_file_directory, std::vector<GameObject*>& objects_created, std::string folder_path, Data& root_data_node);

//	bool ImportMesh(const aiMesh* mesh, const char* folder_path, std::string& output_name);
//	bool Save(Mesh& mesh, const char* folder_path, std::string& output_name);
//	Mesh* Load(const char* path);

	void CollectGameObjectNames(GameObject* gameObject, std::map<std::string, GameObject*>& map);
}

#endif // !__MESH_IMPORTER_H_
