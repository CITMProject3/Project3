#include "ResourceFile.h"

ResourceFile::ResourceFile(ResourceFileType type, const std::string& file_path, unsigned int uuid) : type(type), file_path(file_path), uuid(uuid)
{
}

ResourceFile::~ResourceFile()
{
}

const char * ResourceFile::GetFile() const
{
	return file_path.data();
}

unsigned int ResourceFile::GetUUID() const
{
	return uuid;
}

ResourceFileType ResourceFile::GetType() const
{
	return type;
}

unsigned int ResourceFile::GetBytes() const
{
	return bytes;
}

void ResourceFile::Load()
{
	if (used == 0)
		LoadInMemory();

	used++;
}

void ResourceFile::Unload()
{
	used--;
	if (used == 0)
		UnloadInMemory();
}

void ResourceFile::UnLoadAll()
{
	UnloadInMemory();
}

void ResourceFile::Reload()
{
	LoadInMemory();
}

void ResourceFile::LoadInMemory()
{}

void ResourceFile::UnloadInMemory()
{}
