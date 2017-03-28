#include "AnimationImporter.h"
#include "Globals.h"

#include "ResourceFileAnimation.h"
#include "application.h"

#include "ModuleFileSystem.h"
#include "ModuleEditor.h"
#include "GameObject.h"
#include "ComponentMesh.h"
#include "ResourceFileBone.h"
#include "ComponentBone.h"
#include "ModuleResourceManager.h"

#include "Random.h"

//External libraries
#include "Assimp\include\scene.h"
#include "Assimp\include\anim.h"
#pragma comment (lib, "Assimp/libx86/assimp.lib")

bool AnimationImporter::ImportSceneAnimations(const aiScene* scene, GameObject* root, const char* base_path, std::string& output_name, std::vector<unsigned int>& uuids)
{
	//By now only the first animation is going to be imported, not taking into consideration any other animations in the file
	bool ret = false;
	if (scene->HasAnimations() == true)
	{
		if (uuids.empty())
			uuids.push_back(0);

		ret = ImportAnimation(scene->mAnimations[0], base_path, output_name, uuids[0]);
		if (ret == false)
		{
			LOG("[WARNING] Animation hasn't been imported");
			App->editor->DisplayWarning(WarningType::W_WARNING, "Animation has not been imported");
		}
	}
	return ret;
}

bool AnimationImporter::ImportAnimation(const aiAnimation* anim, const char* base_path, std::string& output_name, unsigned int& uuid)
{
	ResourceFileAnimation animation("", 0);

	animation.full_duration = anim->mDuration * 24;
	animation.ticks_per_second = anim->mTicksPerSecond * 24;
	animation.num_channels = anim->mNumChannels;
	animation.channels = new Channel[animation.num_channels];

	for (uint i = 0; i < anim->mNumChannels; i++)
		ImportChannel(anim->mChannels[i], animation.channels[i]);

	bool ret = Save(animation, base_path, output_name, uuid);

	delete[] animation.channels;
	animation.channels = nullptr;
	return ret;
}

void AnimationImporter::ImportChannel(const aiNodeAnim* node, Channel& channel)
{
	channel.name = node->mNodeName.C_Str();
	uint pos = channel.name.find("_$AssimpFbx$_");
	if (pos != std::string::npos)
	{
		channel.name = channel.name.substr(0, pos);
	}

	//Loading position keys
	for (uint i = 0; i < node->mNumPositionKeys; i++)
		channel.positionKeys[node->mPositionKeys[i].mTime * 24] = float3(node->mPositionKeys[i].mValue.x, node->mPositionKeys[i].mValue.y, node->mPositionKeys[i].mValue.z);

	//Loading rotation keys
	for (uint i = 0; i < node->mNumRotationKeys; i++)
		channel.rotationKeys[node->mRotationKeys[i].mTime * 24] = Quat(node->mRotationKeys[i].mValue.x, node->mRotationKeys[i].mValue.y, node->mRotationKeys[i].mValue.z, node->mRotationKeys[i].mValue.w);

	//Loading scale keys
	for (uint i = 0; i < node->mNumScalingKeys; i++)
		channel.scaleKeys[node->mScalingKeys[i].mTime * 24] = float3(node->mScalingKeys[i].mValue.x, node->mScalingKeys[i].mValue.y, node->mScalingKeys[i].mValue.z);
}

bool AnimationImporter::Save(const ResourceFileAnimation& anim, const char* folder_path, std::string& output_name, unsigned int& uuid)
{
	bool ret = false;

	//Duration, ticks per sec, num_channels, channels
	uint size = sizeof(float) + sizeof(uint) + sizeof(uint);
	for (uint i = 0; i < anim.num_channels; i++)
		size += CalcChannelSize(anim.channels[i]);

	//Allocate buffer size
	char* data = new char[size];
	char* cursor = data;

	//Duration
	memcpy(cursor, &anim.full_duration, sizeof(float));
	cursor += sizeof(float);

	//Ticks per sec
	memcpy(cursor, &anim.ticks_per_second, sizeof(uint));
	cursor += sizeof(float);

	//Channels number
	memcpy(cursor, &anim.num_channels, sizeof(uint));
	cursor += sizeof(uint);

	for (uint i = 0; i < anim.num_channels; i++)
		SaveChannelData(anim.channels[i], &cursor);

	//Generate random UUID for the name
	if (uuid == 0)
		uuid = App->rnd->RandomInt();
	
	ret = App->file_system->Save(std::to_string(uuid).data(), data, size, folder_path, "anim", output_name);

	delete[] data;
	data = nullptr;

	return ret;
}

void AnimationImporter::SaveChannelData(const Channel& channel, char** cursor)
{
	//Name (size and string)
	uint nameSize = channel.name.size();
	memcpy(*cursor, &nameSize, sizeof(uint));
	*cursor += sizeof(uint);

	memcpy(*cursor, channel.name.c_str(), channel.name.size());
	*cursor += channel.name.size();

	//Ranges
	uint ranges[3] = { channel.positionKeys.size(), channel.rotationKeys.size(), channel.scaleKeys.size() };
	memcpy(*cursor, ranges, sizeof(uint) * 3);
	*cursor += sizeof(uint) * 3;

	SaveKeys(channel.positionKeys, cursor);
	SaveKeys(channel.rotationKeys, cursor);
	SaveKeys(channel.scaleKeys, cursor);
}

void AnimationImporter::SaveKeys(const std::map<double, float3>& map, char** cursor)
{
	//Keys save structure: float-float3 // float-float3 // ... (key time-key data)
	std::map<double, float3>::const_iterator it = map.begin();
	for (it = map.begin(); it != map.end(); it++)
	{
		memcpy(*cursor, &it->first, sizeof(double));
		*cursor += sizeof(double);

		memcpy(*cursor, &it->second, sizeof(float) * 3);
		*cursor += sizeof(float) * 3;
	}
}

void AnimationImporter::SaveKeys(const std::map<double, Quat>& map, char** cursor)
{
	//Keys save structure: float-float3 // float-float3 // ... (key time-key data)
	std::map<double, Quat>::const_iterator it = map.begin();
	for (it = map.begin(); it != map.end(); it++)
	{
		memcpy(*cursor, &it->first, sizeof(double));
		*cursor += sizeof(double);

		memcpy(*cursor, &it->second, sizeof(float) * 4);
		*cursor += sizeof(float) * 4;
	}
}

//Animation Load -------------------------------------------
void AnimationImporter::LoadAnimation(const char* path, ResourceFileAnimation* animation)
{
	char* buffer = nullptr;

	if (App->file_system->Load(path, &buffer) != 0)
	{
		char* cursor = buffer;

		//Duration
		memcpy(&animation->full_duration, cursor, sizeof(float));
		cursor += sizeof(float);

		//Ticks per sec
		memcpy(&animation->ticks_per_second, cursor, sizeof(float));
		cursor += sizeof(float);

		//Channels number
		memcpy(&animation->num_channels, cursor, sizeof(uint));
		cursor += sizeof(uint);

		animation->channels = new Channel[animation->num_channels];
		for (uint i = 0; i < animation->num_channels; i++)
		{
			LoadChannelData(animation->channels[i], &cursor);
		}
	}
	if (buffer)
		delete[] buffer;
	buffer = nullptr;
}

void AnimationImporter::LoadChannelData(Channel& channel, char** cursor)
{
	uint bytes = 0;

	//Name (size and string)
	uint nameSize = 0;
	memcpy(&nameSize, *cursor, sizeof(uint));
	*cursor += sizeof(uint);

	if (nameSize > 0)
	{
		char* string = new char[nameSize + 1];
		bytes = sizeof(char) * nameSize;
		memcpy(string, *cursor, bytes);
		*cursor += bytes;
		string[nameSize] = '\0';
		channel.name = string;
		delete[] string;
	}

	//Ranges
	uint ranges[3];
	memcpy(&ranges, *cursor, sizeof(uint) * 3);
	*cursor += sizeof(uint) * 3;

	LoadKeys(channel.positionKeys, cursor, ranges[0]);
	LoadKeys(channel.rotationKeys, cursor, ranges[1]);
	LoadKeys(channel.scaleKeys, cursor, ranges[2]);
}

void AnimationImporter::LoadKeys(std::map<double, float3>& map, char** cursor, uint size)
{
	for (uint i = 0; i < size; i++)
	{
		double time;
		memcpy(&time, *cursor, sizeof(double));
		*cursor += sizeof(double);
		float data[3];
		memcpy(&data, *cursor, sizeof(float) * 3);
		*cursor += sizeof(float) * 3;

		map[time] = float3(data);
	}
}

void AnimationImporter::LoadKeys(std::map<double, Quat>& map, char** cursor, uint size)
{
	for (uint i = 0; i < size; i++)
	{
		double time;
		memcpy(&time, *cursor, sizeof(double));
		*cursor += sizeof(double);
		float data[4];
		memcpy(&data, *cursor, sizeof(float) * 4);
		*cursor += sizeof(float) * 4;

		map[time] = Quat(data);
	}
}
//-----------------------------------------------------------

uint AnimationImporter::CalcChannelSize(const Channel& channel)
{
	//Name (size and string) // Ranges (pos, rot, scale) // Pos floats // Pos // Rot floats // Rots // Scale floats // Scales
	uint ret = sizeof(uint) + channel.name.size() + sizeof(uint) * 3;
	//Positions
	ret += sizeof(double) * channel.positionKeys.size() + sizeof(float) * channel.positionKeys.size() * 3;
	//Rotations
	ret += sizeof(double) * channel.rotationKeys.size() + sizeof(float) * channel.rotationKeys.size() * 4;
	//Scales
	ret += sizeof(double) * channel.scaleKeys.size() + sizeof(float) * channel.scaleKeys.size() * 3;

	return ret;
}

void AnimationImporter::CollectGameObjectNames(GameObject* game_object, std::map<std::string, GameObject*>& map)
{
	map[game_object->name.c_str()] = game_object;

	for (std::vector<GameObject*>::const_iterator it = game_object->GetChilds()->begin(); it != game_object->GetChilds()->end(); it++)
		CollectGameObjectNames(*it, map);
}

void AnimationImporter::ImportSceneBones(const std::vector<const aiMesh*>& boned_meshes, const std::vector<const GameObject*>& boned_game_objects, GameObject* root, const char* base_path, std::vector<unsigned int>& uuids)
{
	std::map<std::string, GameObject*> map;
	CollectGameObjectNames(root, map);

	for (uint i = 0; i < boned_meshes.size(); i++)
	{
		for (uint b = 0; b < boned_meshes[i]->mNumBones; b++)
		{
			while (uuids.size() < boned_meshes.size() + boned_meshes[i]->mNumBones -1)
				uuids.push_back(0);

			std::string mesh_path = "";
			ComponentMesh* mesh = (ComponentMesh*)boned_game_objects[i]->GetComponent(C_MESH);
			if (mesh != nullptr)
			{
				mesh_path = App->file_system->GetNameFromPath(mesh->GetMesh()->file_path);
			}

			std::string bone_file = "";
			ImportBone(boned_meshes[i]->mBones[b], base_path, mesh_path.c_str(), bone_file, uuids[i + b]);
			std::map<std::string, GameObject*>::iterator bone_it = map.find(boned_meshes[i]->mBones[b]->mName.C_Str());
			if (bone_it != map.end())
			{
				ComponentBone* bone = (ComponentBone*)bone_it->second->AddComponent(C_BONE);
				bone->SetResource((ResourceFileBone*)App->resource_manager->LoadResource(bone_file, RES_BONE));
			}
		}
	}
}

bool AnimationImporter::ImportBone(const aiBone* bone, const char* base_path, const char* mesh_path, std::string& output_name, unsigned int& uuid)
{
	ResourceFileBone rBone(base_path, 0);
	rBone.numWeights = bone->mNumWeights;
	rBone.weights = new float[rBone.numWeights];
	rBone.weightsIndex = new uint[rBone.numWeights];
	rBone.mesh_path = mesh_path;

	rBone.offset = float4x4(bone->mOffsetMatrix.a1, bone->mOffsetMatrix.a2, bone->mOffsetMatrix.a3, bone->mOffsetMatrix.a4,
		bone->mOffsetMatrix.b1, bone->mOffsetMatrix.b2, bone->mOffsetMatrix.b3, bone->mOffsetMatrix.b4,
		bone->mOffsetMatrix.c1, bone->mOffsetMatrix.c2, bone->mOffsetMatrix.c3, bone->mOffsetMatrix.c4,
		bone->mOffsetMatrix.d1, bone->mOffsetMatrix.d2, bone->mOffsetMatrix.d3, bone->mOffsetMatrix.d4);

	for (uint i = 0; i < rBone.numWeights; i++)
		memcpy(rBone.weights + i, &((bone->mWeights + i)->mWeight), sizeof(float));

	for (uint i = 0; i < rBone.numWeights; i++)
		memcpy(rBone.weightsIndex + i, &((bone->mWeights + i)->mVertexId), sizeof(uint));

	bool ret = SaveBone(rBone, base_path, output_name, uuid);

	delete[] rBone.weights;
	delete[] rBone.weightsIndex;

	return ret;
}

bool AnimationImporter::SaveBone(const ResourceFileBone& bone, const char* folder_path, std::string& output_name, unsigned int& uuid)
{
	uint size = sizeof(uint) + bone.mesh_path.size() + sizeof(uint) + bone.numWeights * sizeof(float) + bone.numWeights * sizeof(uint)
		+ sizeof(float) * 16;

	char* data = new char[size];
	char* cursor = data;

	//Mesh resource path
	uint fileSize = bone.mesh_path.size();
	memcpy(cursor, &fileSize, sizeof(uint));
	cursor += sizeof(uint);

	memcpy(cursor, bone.mesh_path.c_str(), bone.mesh_path.size());
	cursor += bone.mesh_path.size();

	//Num weights
	memcpy(cursor, &bone.numWeights, sizeof(uint));
	cursor += sizeof(uint);

	//Weights
	memcpy(cursor, bone.weights, sizeof(float) * bone.numWeights);
	cursor += sizeof(float) * bone.numWeights;

	//Weights index
	memcpy(cursor, bone.weightsIndex, sizeof(uint) * bone.numWeights);
	cursor += sizeof(uint) * bone.numWeights;

	//Offset matrix
	memcpy(cursor, &bone.offset, sizeof(float) * 16);
	cursor += sizeof(float) * 16;

	if (uuid == 0)
		uuid = App->rnd->RandomInt();
	bool ret = App->file_system->Save(std::to_string(uuid).data(), data, size, folder_path, "bone", output_name);

	delete[] data;
	data = nullptr;

	return ret;
}

void AnimationImporter::LoadBone(const char* path, ResourceFileBone* bone)
{
	char* buffer;
	uint size = App->file_system->Load(path, &buffer);

	if (size > 0)
	{
		char* cursor = buffer;

		uint stringSize = 0;
		memcpy(&stringSize, cursor, sizeof(uint));
		cursor += sizeof(uint);

		if (stringSize > 0)
		{
			char* string = new char[stringSize + 1];
			memcpy(string, cursor, sizeof(char) * stringSize);
			cursor += sizeof(char) * stringSize;
			string[stringSize] = '\0';
			bone->mesh_path = string;
			delete [] string;
		}

		memcpy(&bone->numWeights, cursor, sizeof(uint));
		cursor += sizeof(uint);

		bone->weights = new float[bone->numWeights];
		memcpy(bone->weights, cursor, sizeof(float) * bone->numWeights);
		cursor += sizeof(float) * bone->numWeights;

		bone->weightsIndex = new uint[bone->numWeights];
		memcpy(bone->weightsIndex, cursor, sizeof(uint) * bone->numWeights);
		cursor += sizeof(uint) * bone->numWeights;

		float* offset = new float[16];
		memcpy(offset, cursor, sizeof(float) * 16);
		cursor += sizeof(float) * 16;

		bone->offset = float4x4(offset[0], offset[1], offset[2], offset[3],
			offset[4], offset[5], offset[6], offset[7],
			offset[8], offset[9], offset[10], offset[11],
			offset[12], offset[13], offset[14], offset[15]);

		delete [] offset;
		delete [] buffer;
	}
}

