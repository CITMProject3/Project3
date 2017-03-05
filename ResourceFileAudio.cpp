#include "ResourceFileAudio.h"

#include "Application.h"
#include "ModuleAudio.h"

ResourceFileAudio::ResourceFileAudio(ResourceFileType type, const std::string& file_path, unsigned int uuid) : ResourceFile(type, file_path, uuid)
{ 
	//init_sb_lib_path = 
}

ResourceFileAudio::~ResourceFileAudio()
{ }


void ResourceFileAudio::LoadInMemory()
{
	App->audio->LoadSoundBank(file_path.c_str());
}

void ResourceFileAudio::UnloadInMemory()
{
	App->audio->UnloadSoundBank(file_path.c_str());
}
