#include "ResourceScriptsLibrary.h"

#include "Application.h"

ResourceScriptsLibrary::ResourceScriptsLibrary(ResourceFileType type, const std::string& file_path, unsigned int uuid) : ResourceFile(type, file_path, uuid)
{ }

ResourceScriptsLibrary::~ResourceScriptsLibrary()
{ }


void ResourceScriptsLibrary::LoadInMemory()
{
	lib = LoadLibrary(file_path.c_str());
}

void ResourceScriptsLibrary::UnloadInMemory()
{
	FreeLibrary(lib);
}