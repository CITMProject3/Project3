#include "ResourceFileAudio.h"

#include "Application.h"
#include "ModuleAudio.h"

ResourceFileAudio::ResourceFileAudio(ResourceFileType type, const std::string& file_path, unsigned int uuid) : ResourceFile(type, file_path, uuid)
{ }

ResourceFileAudio::~ResourceFileAudio()
{ }


void ResourceFileAudio::LoadInMemory()
{
	App->audio->LoadingSoundBank(file_path.c_str());
}

void ResourceFileAudio::UnloadInMemory()
{
	App->audio->UnloadingSoundBank(file_path.c_str());
}
