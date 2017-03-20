#include "ResourceScriptsLibrary.h"

#include "Application.h"

ResourceScriptsLibrary::ResourceScriptsLibrary(ResourceFileType type, const std::string& file_path, unsigned int uuid) : ResourceFile(type, file_path, uuid)
{ }

ResourceScriptsLibrary::~ResourceScriptsLibrary()
{ }


void ResourceScriptsLibrary::LoadInMemory()
{
	string path = file_path.data();
	path.erase(0, 1);// erase the first '/'
	lib = LoadLibrary(path.c_str());
}

void ResourceScriptsLibrary::UnloadInMemory()
{
	FreeLibrary(lib);
}