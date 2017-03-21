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
struct Channel;

namespace AnimationImporter
{
	//Animation Import --------------------------------------------------
	//File is the final path of the imported file with the extension. Path is from the file is converted. Base path is the directory to save all generated files
	//UUIDs vector MUST be filled with scene animations size!
	bool ImportSceneAnimations(const aiScene* scene, GameObject* root, const char* base_path, std::string& output_name, std::vector<unsigned int>& uuids);
	bool ImportAnimation(const aiAnimation* anim, const char* base_path, std::string& output_name, unsigned int& uuid);
	void ImportChannel(const aiNodeAnim* node, Channel& channel);

	bool Save(const ResourceFileAnimation& anim, const char* folder_path, std::string& output_name, unsigned int& uuid);

	void SaveChannelData(const Channel& channel, char** cursor);
	void SaveKeys(const std::map<double, float3>&, char** cursor);
	void SaveKeys(const std::map<double, Quat>&, char** cursor);
	//-------------------------------------------------------------------

	//Animation Load ----------------------------------------------------
	void LoadAnimation(const char* path, ResourceFileAnimation* animation);
	void LoadChannelData(Channel& channel, char** cursor);	
	void LoadKeys(std::map<double, float3>&, char** cursor, uint size);
	void LoadKeys(std::map<double, Quat>&, char** cursor, uint size);
	//-------------------------------------------------------------------

	uint CalcChannelSize(const Channel& channel);
	void CollectGameObjectNames(GameObject* game_object, std::map<std::string, GameObject*>& map);

	//Bone Import -------------------------------------------------------
	void ImportSceneBones(const std::vector<const aiMesh*>& boned_meshes, const std::vector<const GameObject*>& boned_game_objects, GameObject* root, const char* base_path, std::vector<unsigned int>& uuids);
	bool ImportBone(const aiBone* bone, const char* base_path, const char* mesh_path, std::string& output_name, unsigned int& uuid);
	bool SaveBone(const ResourceFileBone& bone, const char* folder_path, std::string& output_name, unsigned int& uuid);
	//-------------------------------------------------------------------

	//Bone Load ---------------------------------------------------------
	void LoadBone(const char* path, ResourceFileBone* bone);
	//-------------------------------------------------------------------
}

#endif // !__MESH_IMPORTER_H_
