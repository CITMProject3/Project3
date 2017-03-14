#ifndef __RESOURCEFILEBONE_H__
#define __RESOURCEFILEBONE_H__

#include "ResourceFile.h"
#include "MathGeoLib\include\MathGeoLib.h"
#include <map>
#include "Globals.h"

class ResourceFileBone : public ResourceFile
{
public:
	ResourceFileBone(const std::string& file_path, unsigned int uuid);
	~ResourceFileBone();

private:
	void LoadInMemory();
	void UnloadInMemory();

public:
	uint numWeights = 0;
	uint* weightsIndex = nullptr;
	float* weights = nullptr;
	float4x4 offset = float4x4::identity;

	std::string mesh_path;
};

#endif // !__RESOURCEFILEBONE_H__
