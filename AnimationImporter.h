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
struct aiMesh;

class GameObject;
class ResourceFileAnimation;
class ResourceFileBone;

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

	void CollectGameObjectNames(GameObject* game_object, std::map<std::string, GameObject*>& map);
	void ImportSceneBones(const std::vector<const aiMesh*>& boned_meshes, const std::vector<const GameObject*>& boned_game_objects, GameObject* root, const char* base_path);
	bool ImportBone(const aiBone* bone, const char* base_path, const char* mesh_path, std::string& output_name);
	bool SaveBone(const ResourceFileBone& bone, const char* folder_path, std::string& output_name);
}

#endif // !__MESH_IMPORTER_H_
