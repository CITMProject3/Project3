#include "Application.h"

#include "ModuleFileSystem.h"
#include "ModuleGOManager.h"
#include "ModuleEditor.h"

#include "Assets.h"

#include "GameObject.h"
#include "Component.h"
#include "ComponentTransform.h"

#include "ResourceFilePrefab.h"

#include "Random.h"

ResourceFilePrefab::ResourceFilePrefab(ResourceFileType type, const std::string& file_path, unsigned int uuid) : ResourceFile(type, file_path, uuid)
{}

ResourceFilePrefab::~ResourceFilePrefab()
{}

GameObject* ResourceFilePrefab::LoadPrefabAsCopy()
{
	GameObject* ret = nullptr;
	char* buffer = nullptr;
	uint size = App->file_system->Load(file_path.data(), &buffer);
	if (size == 0)
	{
		LOG("[ERROR] While loading prefab resource %s", file_path.data());
		App->editor->DisplayWarning(WarningType::W_ERROR, "While loading prefab resource %s", file_path.data());
		if (buffer)
			delete[] buffer;
		return nullptr;
	}

	Data scene(buffer);
	Data root_objects;
	root_objects = scene.GetArray("GameObjects", 0);
	map<unsigned int, unsigned int> uuids;
	if (root_objects.IsNull() == false)
	{
		GameObject* root = nullptr;
		for (size_t i = 0; i < scene.GetArraySize("GameObjects"); i++)
		{
			root = App->go_manager->LoadPrefabGameObject(scene.GetArray("GameObjects", i), uuids);
			if (i == 0)
			{
				instances.push_back(root); //Save the root GO of the prefab
				if (ret == nullptr)
				{
					ret = root;
				}
				Load();
				root->rc_prefab = this;
			}
		}

		instances.back()->prefab_path = file_path.data();
	}
	else
	{
		LOG("[ERROR] The %s is not a valid mesh/prefab file", file_path.data());
		App->editor->DisplayWarning(WarningType::W_ERROR, "The %s is not a valid mesh / prefab file", file_path.data());
	}

	delete[] buffer;
	return ret;
}

GameObject* ResourceFilePrefab::LoadPrefabFromScene(const Data & go_data, GameObject* parent)
{
	const char* name = go_data.GetString("name");
	unsigned int uuid = go_data.GetUInt("UUID");
	unsigned int local_uuid = go_data.GetUInt("local_UUID");
	unsigned int uuid_parent = go_data.GetUInt("parent");

	
	bool active = go_data.GetBool("active");
	bool is_prefab = true;
	unsigned int prefab_root_uuid = go_data.GetUInt("prefab_root_uuid"); 

	GameObject* game_object = new GameObject(name, uuid, parent, active, false, true, 0, prefab_root_uuid, file_path);

	game_object->rc_prefab = this;

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
		LOG("[ERROR] While loading prefab resource %s", file_path.data());
		App->editor->DisplayWarning(WarningType::W_ERROR, "While loading prefab resource %s", file_path.data());
		
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
 
	game_object->layer = layer;
	game_object->local_uuid = local_uuid;

	Data component;
	unsigned int comp_size = root_prefab.GetArraySize("components");
	for (size_t i = 0; i < comp_size; i++)
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
			float4x4 transform_matrix = component.GetMatrix("matrix");
			c_transform->SetScale(transform_matrix.GetScale());
			c_transform->Update(); //To update the matrix manually
		}
	}

	game_object->ForceStatic(is_static);

	//go childs specifiying the uuid
	//read the prefab_file and create new GO with the given uuid
	int childs_uuids_size = go_data.GetArraySize("children_uuids");
	Data child_uuid_info;
	map<unsigned int, unsigned int> childs_uuid;
	for (int i = 0; i < childs_uuids_size; i++)
	{
		child_uuid_info = go_data.GetArray("children_uuids", i);
		int child_uuid = child_uuid_info.GetUInt("uuid");
		int child_local_uuid = child_uuid_info.GetUInt("local_uuid");

		childs_uuid.insert(pair<unsigned int, unsigned int>(child_local_uuid, child_uuid));
	}

	map<unsigned int, unsigned int> uuids;
	uuids.insert(pair<unsigned int, unsigned int>(local_uuid, uuid));
	list<GameObject*> parents;
	parents.push_back(game_object);
	for (size_t i = 1; i < prefab_file.GetArraySize("GameObjects"); i++)
	{
		//Read the uuid of the prefab
		//find the local uuid of the child

		//find it -> great, pass the uuid and the local
		//not find-> new go, new uuid
		unsigned int prefab_go_uuid = prefab_file.GetArray("GameObjects", i).GetUInt("UUID");

		unsigned int child_uuid = 0;
		map<unsigned int, unsigned int>::iterator child_it = childs_uuid.find(prefab_go_uuid);
		if (child_it != childs_uuid.end())
		{
			child_uuid = (*child_it).second;
		}
		else
		{
			child_uuid = App->rnd->RandomInt();
		}
		
		CreateChildsByUUID(prefab_file.GetArray("GameObjects", i), uuids, child_uuid, parents);
	}

	instances.push_back(game_object);
	Load();
	
	return game_object;
}

void ResourceFilePrefab::UnloadInstance(GameObject * instance)
{
	instances.remove(instance);
}

void ResourceFilePrefab::ApplyChanges(GameObject* gameobject)
{
	//Serialize the GO as new prefab
	SaveNewChanges(gameobject);

	//LoadAgain all prefabs in instances
	vector<GameObject*> new_gameobjects;
	for (list<GameObject*>::iterator it = instances.begin(); it != instances.end(); ++it)
		ResetInstance(*it, new_gameobjects);

	instances.clear();

	for (vector<GameObject*>::iterator it = new_gameobjects.begin(); it != new_gameobjects.end(); ++it)
	{
		instances.push_back((*it));
		Load();
	}

	//TODO:
	//Selected go bug
}

void ResourceFilePrefab::RevertChanges(GameObject * gameobject)
{
	vector<GameObject*> new_gameobjects;
	ResetInstance(gameobject, new_gameobjects);

	instances.remove(gameobject);
	instances.push_back(new_gameobjects.front());
}

void ResourceFilePrefab::InsertOriginalInstance(GameObject * original_go)
{
	instances.push_back(original_go);
}

void ResourceFilePrefab::LoadInMemory()
{}

void ResourceFilePrefab::UnloadInMemory()
{
	instances.clear();
}

void ResourceFilePrefab::CreateChildsByUUID(const Data & go_data, map<unsigned int, unsigned int>& uuids, unsigned int uuid, list<GameObject*>&parents) const
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
	for (unsigned int i = 0; i < comp_size; i++)
	{
		component = go_data.GetArray("components", i);

		int type = component.GetInt("type");
		Component* go_component;
		if (type != (int)ComponentType::C_TRANSFORM)
			go_component = go->AddComponent(static_cast<ComponentType>(type));
		else
			go_component = (Component*)go->transform;
		go_component->Load(component);
	}

	if (is_static)
		App->go_manager->octree.Insert(go, go->bounding_box->CenterPoint()); //Needs to go after the components because of the bounding box reference
	else
		App->go_manager->dynamic_gameobjects.push_back(go);

	parents.push_back(go);
}

void ResourceFilePrefab::SaveNewChanges(GameObject * gameobject) const
{
	Data root_node;
	root_node.AppendArray("GameObjects");
	GameObject* parent = gameobject->GetParent();
	gameobject->SetParent(nullptr);
	SaveChangesGameObject(root_node, gameobject, gameobject->local_uuid);
	char* buf;
	size_t size = root_node.Serialize(&buf);

	App->file_system->Save(file_path.data(), buf, size);
	string assets_file = App->editor->assets->FindAssetFileFromLibrary(file_path);
	if (assets_file.size() > 0)
		App->file_system->Save(assets_file.data(), buf, size);
	else
		LOG("Error while applying changes to prefab. Couldn't find assets path");

	delete[] buf;
	
	unsigned int pref_uuid = App->resource_manager->GetUUIDFromLib(file_path);
	App->resource_manager->GenerateMetaFile(assets_file.data(), FileType::PREFAB, pref_uuid, file_path);

	gameobject->SetParent(parent);
}

void ResourceFilePrefab::SaveChangesGameObject(Data & file, GameObject* gameobject, unsigned int prefab_root_uuid) const
{
	Data data;

	//GameObject data
	data.AppendString("name", gameobject->name.data());
	data.AppendUInt("UUID", gameobject->local_uuid);
	data.AppendUInt("local_UUID", gameobject->local_uuid);
	if (gameobject->GetParent() == nullptr)
		data.AppendUInt("parent", 0);
	else
		data.AppendUInt("parent", gameobject->GetParent()->local_uuid);
	data.AppendBool("active", *gameobject->GetActiveBoolean());

	data.AppendBool("is_prefab", gameobject->IsPrefab());
	data.AppendUInt("prefab_root_uuid", prefab_root_uuid);
	data.AppendString("prefab_path", gameobject->prefab_path.data());

	data.AppendInt("layer", gameobject->layer);
	data.AppendBool("static", gameobject->IsStatic());
	data.AppendArray("components");

	//Components data
	const vector<Component*>* components = gameobject->GetComponents();
	for (vector<Component*>::const_iterator component = (*components).begin(); component != (*components).end(); component++)
	{
		(*component)->Save(data);
	}

	file.AppendArrayValue(data);

	const vector<GameObject*>* childs = gameobject->GetChilds();
	for (vector<GameObject*>::const_iterator child = (*childs).begin(); child != (*childs).end(); ++child)
		SaveChangesGameObject(file, *child, prefab_root_uuid);
}

void ResourceFilePrefab::ResetInstance(GameObject * origin, vector<GameObject*>& new_gameobjects) 
{
	const char* name = origin->name.data();
	unsigned int uuid = origin->GetUUID();
	unsigned int local_uuid = origin->local_uuid;
	unsigned int uuid_parent = origin->GetParent()->GetUUID();

	GameObject* parent = App->go_manager->FindGameObjectByUUID(App->go_manager->root, uuid_parent); 
	bool active = origin->IsActive();
	bool is_prefab = true;
	unsigned int prefab_root_uuid = origin->prefab_root_uuid;

	if (App->go_manager->current_scene_canvas == (ComponentCanvas*)origin->GetComponent(C_CANVAS))
		App->go_manager->current_scene_canvas = nullptr;

	GameObject* game_object = new GameObject(name, uuid, parent, active, false, true, 0, prefab_root_uuid, file_path);

	game_object->rc_prefab = this;

	if (parent)
		parent->AddChild(game_object);

	ComponentTransform* c_transform = (ComponentTransform*)game_object->GetComponent(ComponentType::C_TRANSFORM);
	ComponentTransform* origin_c_transform = (ComponentTransform*)origin->GetComponent(ComponentType::C_TRANSFORM);
	float3 position = origin_c_transform->GetPosition();
	float3 rotation = origin_c_transform->GetRotationEuler();

	c_transform->SetPosition(position);
	c_transform->SetRotation(rotation);

	char* buffer = nullptr;
	uint size = App->file_system->Load(file_path.data(), &buffer);
	if (size == 0)
	{
		LOG("[ERROR] While loading prefab resource %s", file_path.data());
		App->editor->DisplayWarning(WarningType::W_ERROR, "While loading prefab resource %s", file_path.data());
		if (buffer)
			delete[] buffer;
		return;
	}
	Data prefab_file(buffer);
	Data root_prefab;

	root_prefab = prefab_file.GetArray("GameObjects", 0);

	//Read the prefab file and create all other variables
	bool is_static = root_prefab.GetBool("static");
	int layer = root_prefab.GetInt("layer");

	game_object->layer = layer;
	game_object->local_uuid = local_uuid;

	Data component;
	unsigned int comp_size = root_prefab.GetArraySize("components");
	for (unsigned int i = 0; i < comp_size; i++)
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
			float4x4 transform_matrix = component.GetMatrix("matrix");
			c_transform->SetScale(transform_matrix.GetScale());
			c_transform->Update(); //To update the matrix manually
		}
	}

	game_object->ForceStatic(is_static);

	//go childs specifiying the uuid
	//read the prefab_file and create new GO with the given uuid
	
	vector<unsigned int> c_childs_uuid;
	vector<unsigned int> c_childs_local_uuid;
	origin->CollectChildrenUUID(c_childs_uuid, c_childs_local_uuid);
	Data child_uuid_info;
	map<unsigned int, unsigned int> childs_uuid;
	for (size_t i = 0; i < c_childs_uuid.size(); i++)
	{
		
		int child_uuid = c_childs_uuid[i];
		int child_local_uuid = c_childs_local_uuid[i];

		childs_uuid.insert(pair<unsigned int, unsigned int>(child_local_uuid, child_uuid));
	}

	map<unsigned int, unsigned int> uuids;
	uuids.insert(pair<unsigned int, unsigned int>(local_uuid, uuid));
	list<GameObject*> parents;
	parents.push_back(game_object);
	for (size_t i = 1; i < prefab_file.GetArraySize("GameObjects"); i++)
	{
		//Read the uuid of the prefab
		//find the local uuid of the child

		//find it -> great, pass the uuid and the local
		//not find-> new go, new uuid
		unsigned int prefab_go_uuid = prefab_file.GetArray("GameObjects", i).GetUInt("UUID");

		unsigned int child_uuid = 0;
		map<unsigned int, unsigned int>::iterator child_it = childs_uuid.find(prefab_go_uuid);
		if (child_it != childs_uuid.end())
		{
			child_uuid = (*child_it).second;
		}
		else
		{
			child_uuid = App->rnd->RandomInt();
		}

		CreateChildsByUUID(prefab_file.GetArray("GameObjects", i), uuids, child_uuid, parents);
	}

	new_gameobjects.push_back(game_object);
	App->go_manager->RemoveGameObject(origin);
}

