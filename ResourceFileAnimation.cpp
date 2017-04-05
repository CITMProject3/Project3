#include "ResourceFileAnimation.h"
#include "AnimationImporter.h"

bool Channel::HasPosKey() const
{
	return ((positionKeys.size() == 1 && positionKeys.begin()->first == -1) == false);
}

std::map<double, float3>::iterator Channel::GetPrevPosKey(double currentKey)
{
	std::map<double, float3>::iterator ret = positionKeys.begin();
	std::map<double, float3>::iterator it = ret;
	it++;

	while (it != positionKeys.end() && it->first <= currentKey)
	{
		ret = it;
		it++;
	}
	return ret;
}

std::map<double, float3>::iterator Channel::GetNextPosKey(double currentKey)
{
	std::map<double, float3>::iterator ret = positionKeys.begin();
	while (ret != positionKeys.end() && ret->first < currentKey)
	{
		ret++;
	}
	return ret;
}

bool Channel::HasRotKey() const
{
	return ((rotationKeys.size() == 1 && rotationKeys.begin()->first == -1) == false);
}

std::map<double, Quat>::iterator Channel::GetPrevRotKey(double currentKey)
{
	//Looping through keys
	std::map<double, Quat>::iterator ret = rotationKeys.begin();
	std::map<double, Quat>::iterator it = ret;
	it++;

	while (it != rotationKeys.end() && it->first <= currentKey)
	{
		ret = it;
		it++;
	}
	return ret;
}

std::map<double, Quat>::iterator Channel::GetNextRotKey(double currentKey)
{
	std::map<double, Quat>::iterator ret = rotationKeys.begin();
	while (ret != rotationKeys.end() && ret->first < currentKey)
	{
		ret++;
	}
	return ret;
}

bool Channel::HasScaleKey() const
{
	return ((scaleKeys.size() == 1 && scaleKeys.begin()->first == -1) == false);
}

std::map<double, float3>::iterator Channel::GetPrevScaleKey(double currentKey)
{
	//Looping through keys
	std::map<double, float3>::iterator ret = scaleKeys.begin();
	std::map<double, float3>::iterator it = ret;
	it++;

	while (it != scaleKeys.end() && it->first <= currentKey)
	{
		ret = it;
		it++;
	}
	return ret;
}

std::map<double, float3>::iterator Channel::GetNextScaleKey(double currentKey)
{
	std::map<double, float3>::iterator ret = scaleKeys.begin();
	while (ret != scaleKeys.end() && ret->first < currentKey)
	{
		ret++;
	}
	return ret;
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