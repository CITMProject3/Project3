#include "Application.h"
#include "ResourceFilePrefab.h"
#include "GameObject.h"
#include "Component.h"
#include "ComponentTransform.h"

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

GameObject* ResourceFilePrefab::LoadPrefabFromScene(const Data & go_data, GameObject* parent) const
{
	const char* name = go_data.GetString("name");
	unsigned int uuid = go_data.GetUInt("UUID");
	unsigned int local_uuid = go_data.GetUInt("local_UUID");
	unsigned int uuid_parent = go_data.GetUInt("parent");

	//parent = App->go_manager->FindGameObjectByUUID(root, uuid_parent); //I wil deal with this later
	bool active = go_data.GetBool("active");
	bool is_prefab = true;
	unsigned int prefab_root_uuid = go_data.GetUInt("prefab_root_uuid"); 

	GameObject* game_object = new GameObject(name, uuid, parent, active, false, true, 0, prefab_root_uuid, file_path);

	if (parent)
		parent->AddChild(game_object);
	ComponentTransform* c_transform = (ComponentTransform*)game_object->GetComponent(ComponentType::C_TRANSFORM);

	float3 position = go_data.GetArray("components", 0).GetFloat3("position");
	float3 rotation = go_data.GetArray("components", 0).GetFloat3("rotation");

	c_transform->SetPosition(position);
	c_transform->SetRotation(rotation);
	
	char* buffer = nullptr;
	uint size = App->file_system->Load(file_path.data(), &buffer);
	if (size == 0)
	{
		LOG("Error while loading: %s", file_path.data());
		if (buffer)
			delete[] buffer;
		return nullptr;
	}
	Data prefab_file(buffer);
	Data root_prefab;

	root_prefab = prefab_file.GetArray("GameObjects", 0);

	//Read the prefab file and create all other variables
	bool is_static = root_prefab.GetBool("static");
	int layer = root_prefab.GetInt("layer");

	game_object->SetStatic(is_static); //Automatic insertion NOPE I don't know hot to handle this...OK I KNOW BUT...
	game_object->layer = layer;

	Data component;
	unsigned int comp_size = root_prefab.GetArraySize("components");
	for (int i = 0; i < comp_size; i++)
	{
		component = root_prefab.GetArray("components", i);

		int type = component.GetInt("type");
		Component* go_component;
		if (type != (int)ComponentType::C_TRANSFORM)
		{
			go_component = game_object->AddComponent(static_cast<ComponentType>(type));
			go_component->Load(component);
		}
		else
		{
			c_transform->SetScale(component.GetFloat3("scale"));
			c_transform->Update(); //To update the matrix manually
		}
	}

	//go childs specifiying the uuid
	//read the prefab_file and create new GO with the given uuid

	map<unsigned int, unsigned int> uuids;
	uuids.insert(pair<unsigned int, unsigned int>(local_uuid, uuid));
	list<GameObject*> parents;
	parents.push_back(game_object);
	for (int i = 1; i < prefab_file.GetArraySize("GameObjects"); i++)
	{
		CreateChildsByUUID(prefab_file.GetArray("GameObjects", i), uuids, uuid, parents);
	}
	
	return game_object;
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

void ResourceFilePrefab::CreateChildsByUUID(const Data & go_data, map<unsigned int, unsigned int>& uuids, unsigned int uuid, list<GameObject*>parents) const
{
	const char* name = go_data.GetString("name");
	uuids.insert(pair<unsigned int, unsigned int>(go_data.GetUInt("UUID"), uuid));
	map<unsigned int, unsigned int>::iterator parent_old_uuid = uuids.find(go_data.GetUInt("parent"));
	unsigned int uuid_parent = 0;
	if (parent_old_uuid != uuids.end())
	{
		uuid_parent = parent_old_uuid->second;
	}

	bool active = go_data.GetBool("active");
	bool is_static = go_data.GetBool("static");
	bool is_prefab = go_data.GetBool("is_prefab");
	string prefab_path = go_data.GetString("prefab_path");
	unsigned int prefab_root_uuid = 0;
	if (is_prefab)
		prefab_root_uuid = uuids.find(go_data.GetUInt("prefab_root_uuid"))->second;
	int layer = go_data.GetInt("layer");

	//Find parent GameObject reference
	GameObject* parent = nullptr;
	for (list<GameObject*>::const_iterator it = parents.begin(); it != parents.end(); ++it)
	{
		if ((*it)->GetUUID() == uuid_parent)
		{
			parent = *it;
			break;
		}
	}

	//Basic GameObject properties
	GameObject* go = new GameObject(name, uuid, parent, active, is_static, is_prefab, layer, prefab_root_uuid, prefab_path);

	if (is_prefab)
		go->local_uuid = go_data.GetUInt("UUID");

	if (parent)
		parent->AddChild(go);

	//Components
	Data component;
	unsigned int comp_size = go_data.GetArraySize("components");
	for (int i = 0; i < comp_size; i++)
	{
		component = go_data.GetArray("components", i);

		int type = component.GetInt("type");
		Component* go_component;
		if (type != (int)ComponentType::C_TRANSFORM)
			go_component = go->AddComponent(static_cast<ComponentType>(type));
		else
			go_component = (Component*)go->GetComponent(C_TRANSFORM);
		go_component->Load(component);
	}

	if (is_static)
		App->go_manager->octree.Insert(go, go->bounding_box->CenterPoint()); //Needs to go after the components because of the bounding box reference
	else
		App->go_manager->dynamic_gameobjects.push_back(go);

	parents.push_back(go);
}

