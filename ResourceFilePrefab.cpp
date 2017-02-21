#include "Application.h"
#include "ResourceFilePrefab.h"
#include "GameObject.h"


ResourceFilePrefab::ResourceFilePrefab(ResourceFileType type, const std::string& file_path, unsigned int uuid) : ResourceFile(type, file_path, uuid)
{}

ResourceFilePrefab::~ResourceFilePrefab()
{}

void ResourceFilePrefab::LoadPrefabAsCopy()
{
	char* buffer = nullptr;
	uint size = App->file_system->Load(file_path.data(), &buffer);
	if (size == 0)
	{
		LOG("Error while loading: %s", file_path.data());
		if (buffer)
			delete[] buffer;
		return;
	}

	Data scene(buffer);
	Data root_objects;
	root_objects = scene.GetArray("GameObjects", 0);
	map<unsigned int, unsigned int> uuids;
	if (root_objects.IsNull() == false)
	{
		GameObject* root = nullptr;
		for (int i = 0; i < scene.GetArraySize("GameObjects"); i++)
		{
			root = App->go_manager->LoadPrefabGameObject(scene.GetArray("GameObjects", i), uuids);

			if (i == 0)
				instances.push_back(root); //Save the root GO of the prefab
		}

		instances.back()->prefab_path = file_path.data();
	}
	else
	{
		LOG("The %s is not a valid mesh/prefab file", file_path.data());
	}

	delete[] buffer;
}

void ResourceFilePrefab::Save()
{
}

void ResourceFilePrefab::LoadInMemory()
{}

void ResourceFilePrefab::UnloadInMemory()
{
	instances.clear();
}

