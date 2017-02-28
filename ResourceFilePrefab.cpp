#include "ResourceFilePrefab.h"
#include "GameObject.h"

ResourceFilePrefab::ResourceFilePrefab(ResourceFileType type, const std::string& file_path, unsigned int uuid) : ResourceFile(type, file_path, uuid)
{}

ResourceFilePrefab::~ResourceFilePrefab()
{}

void ResourceFilePrefab::LoadPrefab(GameObject* root)
{

}

void ResourceFilePrefab::Save()
{
}

void ResourceFilePrefab::LoadInMemory()
{}

void ResourceFilePrefab::UnloadInMemory()
{}

