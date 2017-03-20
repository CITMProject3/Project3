#ifndef __RESOURCEFILEANIMATION_H__
#define __RESOURCEFILEANIMATION_H__

#include "ResourceFile.h"
#include "MathGeoLib\include\MathGeoLib.h"
#include <map>
#include "Globals.h"

struct Channel
{
	std::string name;

	std::map<double, float3> positionKeys;
	std::map<double, Quat> rotationKeys;
	std::map<double, float3> scaleKeys;

	bool HasPosKey() const;
	std::map<double, float3>::iterator GetPrevPosKey(double currentKey);
	std::map<double, float3>::iterator GetNextPosKey(double currentKey);

	bool HasRotKey() const;
	std::map<double, Quat>::iterator GetPrevRotKey(double currentKey);
	std::map<double, Quat>::iterator GetNextRotKey(double currentKey);

	bool HasScaleKey() const;
	std::map<double, float3>::iterator GetPrevScaleKey(double currentKey);
	std::map<double, float3>::iterator GetNextScaleKey(double currentKey);
};

class ResourceFileAnimation : public ResourceFile
{
public:
	ResourceFileAnimation(const std::string& file_path, unsigned int uuid);
	~ResourceFileAnimation();

private:
	void LoadInMemory();
	void UnloadInMemory();

public:
	float full_duration = 0;
	uint ticks_per_second = 0;

	uint num_channels = 0;
	Channel* channels = nullptr;
};

#endif // !__RESOURCEFILANIMATION_H__
