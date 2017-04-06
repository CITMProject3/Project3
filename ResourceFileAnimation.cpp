#include "ResourceFileAnimation.h"
#include "AnimationImporter.h"

bool Channel::HasPosKey() const
{
	return ((positionKeys.size() == 1 && positionKeys.begin()->first == -1) == false);
}

std::map<double, float3>::iterator Channel::GetPrevPosKey(double currentKey)
{

	std::map<double, float3>::iterator ret = positionKeys.lower_bound(currentKey);
	if (ret != positionKeys.begin())
		ret--;

	return ret;
}

std::map<double, float3>::iterator Channel::GetNextPosKey(double currentKey)
{
	return positionKeys.upper_bound(currentKey);
}

bool Channel::HasRotKey() const
{
	return ((rotationKeys.size() == 1 && rotationKeys.begin()->first == -1) == false);
}

std::map<double, Quat>::iterator Channel::GetPrevRotKey(double currentKey)
{
	std::map<double, Quat>::iterator ret = rotationKeys.lower_bound(currentKey);
	if (ret != rotationKeys.begin())
		ret--;
	return ret;
}

std::map<double, Quat>::iterator Channel::GetNextRotKey(double currentKey)
{
	return rotationKeys.upper_bound(currentKey);
}

bool Channel::HasScaleKey() const
{
	return ((scaleKeys.size() == 1 && scaleKeys.begin()->first == -1) == false);
}

std::map<double, float3>::iterator Channel::GetPrevScaleKey(double currentKey)
{
	//Looping through keys
	std::map<double, float3>::iterator ret = scaleKeys.lower_bound(currentKey);
	if (ret != scaleKeys.begin())
		ret--;
	return ret;
}

std::map<double, float3>::iterator Channel::GetNextScaleKey(double currentKey)
{
	return scaleKeys.upper_bound(currentKey);
}

ResourceFileAnimation::ResourceFileAnimation(const std::string& file_path, unsigned int uuid) : ResourceFile(RES_ANIMATION, file_path, uuid)
{

}

ResourceFileAnimation::~ResourceFileAnimation()
{

}

void ResourceFileAnimation::LoadInMemory()
{
	AnimationImporter::LoadAnimation(file_path.c_str(), this);
}

void ResourceFileAnimation::UnloadInMemory()
{

}