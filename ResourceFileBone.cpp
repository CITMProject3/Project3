#include "ResourceFileBone.h"
#include "AnimationImporter.h"

ResourceFileBone::ResourceFileBone(const std::string& file_path, unsigned int uuid) : ResourceFile(RES_BONE, file_path, uuid)
{

}

ResourceFileBone::~ResourceFileBone()
{

}

void ResourceFileBone::LoadInMemory()
{
	AnimationImporter::LoadBone(file_path.c_str(), this);
}

void ResourceFileBone::UnloadInMemory()
{

}