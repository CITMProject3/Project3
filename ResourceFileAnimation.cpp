#include "ResourceFileAnimation.h"
#include "AnimationImporter.h"
#include "Brofiler/include/Brofiler.h"

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
	std::map<double, float3>::iterator ret = positionKeys.upper_bound(currentKey);
	return ret;
}

bool Channel::HasRotKey() const
{
	return ((rotationKeys.size() == 1 && rotationKeys.begin()->first == -1) == false);
}

std::map<double, Quat>::iterator Channel::GetPrevRotKey(double currentKey)
{
	BROFILER_CATEGORY("Channel::GetPrevRotKey", Profiler::Color::LightGoldenRodYellow)
	//Looping through keys
	std::map<double, Quat>::iterator ret = rotationKeys.lower_bound(currentKey);
	if (ret != rotationKeys.begin())
		ret--;
	return ret;
}

std::map<double, Quat>::iterator Channel::GetNextRotKey(double currentKey)
{
	BROFILER_CATEGORY("Channel::GetNextRotKey", Profiler::Color::DarkMagenta)

	std::map<double, Quat>::iterator ret = rotationKeys.upper_bound(currentKey);
	return ret;
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
	std::map<double, float3>::iterator ret = scaleKeys.upper_bound(currentKey);
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