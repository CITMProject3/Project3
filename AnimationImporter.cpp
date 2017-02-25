#include "AnimationImporter.h"
#include "Globals.h"

#include "ResourceFileAnimation.h"
#include "application.h"

#include "ModuleFileSystem.h"

//External libraries
#include "Assimp\include\scene.h"
#include "Assimp\include\anim.h"
#pragma comment (lib, "Assimp/libx86/assimp.lib")

bool AnimationImporter::ImportSceneAnimations(const aiScene* scene, GameObject* root, const char* base_path, std::string& output_name)
{
	uint ret = 0;
	if (scene->HasAnimations() == true)
	{
		//In case there is more than one animation, all animations will be imported
		//but only the first one will be assigned to the root
		for (int i = scene->mNumAnimations - 1; i >= 0; i--)
		{
			bool imported = ImportAnimation(scene->mAnimations[i], base_path, output_name);
			if (imported == false)
				LOG("Warning: could not import animation nº %i / %i", i, scene->mNumAnimations);
		}
	}
	return ret;
}

bool AnimationImporter::ImportAnimation(const aiAnimation* anim, const char* base_path, std::string& output_name)
{
	ResourceFileAnimation animation("", 0);

	animation.full_duration = anim->mDuration;
	animation.ticks_per_second = anim->mTicksPerSecond;
	animation.num_channels = anim->mNumChannels;
	animation.channels = new Channel[animation.num_channels];

	for (uint i = 0; i < anim->mNumChannels; i++)
		ImportChannel(anim->mChannels[i], animation.channels[i]);

	bool ret = Save(animation, base_path, output_name);

	delete[] animation.channels;
	animation.channels = nullptr;
	//animation->ID = ID;
	//animation->name = anim->mName.C_Str();
	//std::string full_path("/Library/Animations/");
	//full_path.append(std::to_string(ID));
	//animation->resource_file = full_path;
	//animation->original_file = source_file;
	//animation->LoadOnMemory();
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
		channel.positionKeys[node->mPositionKeys[i].mTime] = float3(node->mPositionKeys[i].mValue.x, node->mPositionKeys[i].mValue.y, node->mPositionKeys[i].mValue.z);

	//Loading rotation keys
	for (uint i = 0; i < node->mNumRotationKeys; i++)
		channel.rotationKeys[node->mRotationKeys[i].mTime] = Quat(node->mRotationKeys[i].mValue.x, node->mRotationKeys[i].mValue.y, node->mRotationKeys[i].mValue.z, node->mRotationKeys[i].mValue.w);

	//Loading scale keys
	for (uint i = 0; i < node->mNumScalingKeys; i++)
		channel.scaleKeys[node->mScalingKeys[i].mTime] = float3(node->mScalingKeys[i].mValue.x, node->mScalingKeys[i].mValue.y, node->mScalingKeys[i].mValue.z);
}

bool AnimationImporter::Save(const ResourceFileAnimation& anim, const char* folder_path, std::string& output_name)
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
	ret = App->file_system->SaveUnique(std::to_string((unsigned int)App->rnd->RandomInt()).data(), data, size, folder_path, "msh", output_name);

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

void AnimationImporter::CollectGameObjectNames(GameObject* gameObject, std::map<std::string, GameObject*>& map)
{

}

