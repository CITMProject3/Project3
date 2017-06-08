#include "Application.h"
#include "ModuleGOManager.h"
#include "ModuleRenderer3D.h"
#include "ModuleResourceManager.h"
#include "ModuleCamera3D.h"
#include "ModuleInput.h"
#include "ModuleEditor.h"
#include "ModuleFileSystem.h"
#include "ModulePhysics3D.h"

#include "GameObject.h"
#include "ComponentTransform.h"
#include "ComponentMesh.h"
#include "ComponentLight.h"
#include "ComponentAnimation.h"
#include "ComponentCar.h"

#include "Assets.h"

#include "RaycastHit.h"
#include "LayerSystem.h"
#include "Random.h"

#include "ResourceFilePrefab.h"

#include "Brofiler\include\Brofiler.h"

ModuleGOManager::ModuleGOManager(const char* name, bool start_enabled) : Module(name, start_enabled)
{}

ModuleGOManager::~ModuleGOManager()
{
	if (root)
		delete root;

	dynamic_gameobjects.clear();
	delete layer_system;
}

bool ModuleGOManager::Init(Data & config)
{
	layer_system = new LayerSystem();
	layer_system->Load(config);

	// Whether exists one scene on Configuration.json, load it on Start()!
	if (App->IsGameRunning())
		current_library_scene_path = config.GetString("current_library_scene_path");
	else
		current_library_scene_path = "";
	
	team1_front = PLAYER_1;
	team1_back = PLAYER_2;
	team2_front = PLAYER_3;
	team2_back = PLAYER_4;

	team1_car = 0;
	team2_car = 1;

	team1_p1_c = 0;
	team1_p2_c = 3;

	team2_p1_c = 0;
	team2_p2_c = 2;
	return true;
}

bool ModuleGOManager::Start()
{
	DeleteTemporalScene();

	octree.Create(OCTREE_SIZE);

	if (!current_library_scene_path.empty() && App->IsGameRunning())
		App->resource_manager->LoadScene(current_library_scene_path.data());
	else
		LoadEmptyScene();

	return true;
}

update_status ModuleGOManager::PreUpdate()
{
	BROFILER_CATEGORY("ModuleGOManager::PreUpdate", Profiler::Color::Aquamarine)
	//Remove all GameObjects that needs to be erased
	for (vector<GameObject*>::iterator go = go_to_remove.begin(); go != go_to_remove.end(); ++go)
	{
		if ((*go)->IsStatic())
		{
			if ((*go)->bounding_box)
				octree.Remove((*go), (*go)->bounding_box->CenterPoint());
		}
		else
		{
			dynamic_gameobjects.remove(*go);
		}
		delete (*go);
	}

	go_to_remove.clear();

	if (root)
		PreUpdateGameObjects(root);

	return UPDATE_CONTINUE;
}

update_status ModuleGOManager::Update()
{
	BROFILER_CATEGORY("ModuleGOManager::Update", Profiler::Color::SkyBlue)
	//Update GameObjects
	if(root)
		UpdateGameObjects(root);

	if(draw_octree)
		octree.Draw();

	return UPDATE_CONTINUE;
}

void ModuleGOManager::SaveBeforeClosing(Data& data) const
{
	data.AppendString("current_assets_scene_path", current_assets_scene_path.data());
	data.AppendString("current_library_scene_path", current_library_scene_path.data());
	data.AppendArray("layers");
	layer_system->Save(data);
}

GameObject* ModuleGOManager::CreateGameObject(GameObject* parent)
{
	GameObject* obj_parent = (parent) ? parent : root;

	GameObject* object = new GameObject(obj_parent);

	if (obj_parent->AddChild(object) == false)
		LOG("A child insertion to GameObject %s could not be done", obj_parent->name.data());

	if (parent)
	{
		if (parent->IsPrefab())
		{
			object->SetAsPrefab(parent->prefab_root_uuid);
			object->prefab_path = parent->prefab_path;
		}
	}

	dynamic_gameobjects.push_back(object);

	return object;
}

GameObject* ModuleGOManager::CreateLight(GameObject* parent, LightType type)
{
	GameObject* obj = CreateGameObject(parent);
	ComponentLight* light = (ComponentLight*)obj->AddComponent(C_LIGHT);
	light->SetType(type);
	switch (type)
	{
		case(DIRECTIONAL_LIGHT): obj->name = "Directional Light"; break;
	}
	return obj;
}

GameObject* ModuleGOManager::CreatePrimitive(PrimitiveType type)
{
	GameObject *primitive = CreateGameObject(root);														// Creating empty GO with root as parent
	ComponentMesh *mesh_comp = ((ComponentMesh*)primitive->AddComponent(ComponentType::C_MESH));	    // Adding Component Mesh
	primitive->AddComponent(ComponentType::C_MATERIAL);													// Adding Default Material
	
	string prim_path = "Resources/Primitives/";

	std::map<PrimitiveType, unsigned> prim_codes;
	prim_codes[P_CUBE] = 2147000001;
	prim_codes[P_CYLINDER] = 2147000002;
	prim_codes[P_PLANE] = 2147000003;
	prim_codes[P_SPHERE] = 2147000004;

	switch (type)
	{
		case(PrimitiveType::P_CUBE):
		{
			primitive->name.assign("Cube");			
			prim_path += std::to_string(prim_codes[P_CUBE]); break;
		}
		case(PrimitiveType::P_CYLINDER):
		{
			primitive->name.assign("Cylinder");
			prim_path += std::to_string(prim_codes[P_CYLINDER]); break;
		}
		case(PrimitiveType::P_PLANE):
		{
			primitive->name.assign("Plane");
			prim_path += std::to_string(prim_codes[P_PLANE]); break;
		}
		case(PrimitiveType::P_SPHERE):
		{
			primitive->name.assign("Sphere");
			prim_path += std::to_string(prim_codes[P_SPHERE]); break;
		}		
	}

	prim_path += ".msh";

	// Loading mesh for each primitive
	Data load_info;
	load_info.AppendUInt("UUID", prim_codes[type] );
	load_info.AppendBool("active", true);
	load_info.AppendString("path", prim_path.c_str());

	mesh_comp->Load(load_info);
	return primitive;
}

bool ModuleGOManager::RemoveGameObject(GameObject* object)
{
	bool ret = false;

	if (object)
	{
		if(object->GetParent() != nullptr)
			object->GetParent()->RemoveChild(object);
		object->RemoveAllChilds();
		
		go_to_remove.push_back(object);

		ret = true;
	}

	return ret;
}

bool ModuleGOManager::FastRemoveGameObject(GameObject * object)
{
	bool ret = false;

	if (object)
	{
		object->RemoveAllChilds();
		go_to_remove.push_back(object);
		ret = true;
	}

	return ret;
}

void ModuleGOManager::DuplicateGameObject(GameObject* object)
{
	if (object != nullptr)
	{
		if (object->IsPrefab() == true)
		{
			if (object->rc_prefab)
			{
				GameObject* new_go = object->rc_prefab->LoadPrefabAsCopy();
				ComponentTransform* transform = (ComponentTransform*)new_go->GetComponent(C_TRANSFORM);
				transform->Set(((ComponentTransform*)object->GetComponent(C_TRANSFORM))->GetLocalTransformMatrix());
				App->editor->SelectSingle(new_go);
			}
			LOG("Error: GameObject prefab has no resource prefab!");
		}
		else
		{
			ResourceFilePrefab* prefab = App->resource_manager->SavePrefab(object);
			App->editor->RefreshAssets();
			if (prefab)
			{
				GameObject* new_go = prefab->LoadPrefabAsCopy();
				object->UnlinkPrefab();
				new_go->UnlinkPrefab();
				//Funny stuff. Adding assets here because files are managed on ImGui class x)
				std::string asset_file = App->editor->assets->FindAssetFileFromLibrary(prefab->GetFile());
				AssetFile* file = App->editor->assets->FindAssetFile(asset_file);
				App->editor->assets->DeleteAssetFile(file);

				ComponentTransform* transform = (ComponentTransform*)new_go->GetComponent(C_TRANSFORM);
				transform->Set(((ComponentTransform*)object->GetComponent(C_TRANSFORM))->GetLocalTransformMatrix());
				App->editor->SelectSingle(new_go);
			}
			else
			{
				LOG("Error while duplicating GameObject: prefab not saved");
			}
		}
	}
	else
	{
		LOG("Error: attempting to duplicate null GameObject");
	}
}

void ModuleGOManager::GetAllComponents(std::vector<Component*> &list, ComponentType type, GameObject *from) const 
{
	GameObject* go = (from) ? from : root;

	Component* component = go->GetComponent(type);
	if (component) list.push_back(component);

	const vector<GameObject*>* childs = go->GetChilds();
	for (vector<GameObject*>::const_iterator child = childs->begin(); child != childs->end(); ++child)
		GetAllComponents(list, type,(*child));
}

ComponentLight * ModuleGOManager::GetDirectionalLight(GameObject* from) const
{
	ComponentLight* light = nullptr;
	GameObject* go = (from) ? from : root;

	light = (ComponentLight*)go->GetComponent(C_LIGHT);
	if (light)
	{
		if(light->GetLightType() == LightType::DIRECTIONAL_LIGHT)
			return light;
	}		

	const vector<GameObject*>* childs = go->GetChilds();
	for (vector<GameObject*>::const_iterator child = childs->begin(); child != childs->end(); ++child)
	{
		light = GetDirectionalLight((*child));
		if (light)
			return light;
	}

	return nullptr;
}

void ModuleGOManager::LoadEmptyScene()
{
	ClearScene();
	App->physics->DeleteHeightmap();
	//Empty scene
	root = new GameObject();
	root->name = "Root";

	current_assets_scene_path = "";
	current_library_scene_path = "";

	App->renderer3D->SetCamera(App->camera->GetEditorCamera());
}

bool ModuleGOManager::IsRoot(const GameObject * go) const
{
	bool ret = false;

	if (go)
	{
		if (go == root)
			ret = true;
	}

	return ret;
}

void ModuleGOManager::SaveSceneBeforeRunning()
{
	Data root_node;
	root_node.AppendArray("GameObjects");

	root->Save(root_node);

	char* buf;
	size_t size = root_node.Serialize(&buf);

	App->file_system->Save(TEMPORAL_SCENE, buf, size); //TODO: Find the right place to save the scene.

	delete[] buf;

	std::string textureMapPath  = TEMPORAL_SCENE;
	textureMapPath = textureMapPath.substr(0, textureMapPath.length() - 4);
	textureMapPath += "txmp";
	App->physics->SaveTextureMap(textureMapPath.data());

}

void ModuleGOManager::LoadSceneBeforeRunning()
{
	App->resource_manager->LoadScene(TEMPORAL_SCENE);
	DeleteTemporalScene();
}

void ModuleGOManager::DeleteTemporalScene()
{
	App->file_system->Delete(TEMPORAL_SCENE);

	string terrain = TEMPORAL_SCENE;
	uint len = terrain.find(".ezx");
	if (len == string::npos)
	{
		len = terrain.find(".json");
	}
	len++;
	terrain = terrain.substr(0, len);
	terrain += "txmp";
	App->file_system->Delete(terrain.data());
}

bool ModuleGOManager::InsertGameObjectInOctree(GameObject * go)
{
	bool ret = false;
	if (go->IsStatic())
	{
		if (go->bounding_box) //Only GameObjects with mesh can go inside for now.
		{
			dynamic_gameobjects.remove(go);
			ret = octree.Insert(go, go->bounding_box->CenterPoint());
		}
	}
	return ret;
}

bool ModuleGOManager::RemoveGameObjectOfOctree(GameObject * go)
{
	bool ret = false;
	if (go->bounding_box)
	{
		ret = octree.Remove(go, go->bounding_box->CenterPoint());
		dynamic_gameobjects.push_back(go);
	}
	return ret;
}

void ModuleGOManager::ClearScene()
{
	if (root != nullptr)
	{
		App->renderer3D->CleanCameras();

		if (App->IsGameRunning())
		{
			App->OnStop();
		}

		RemoveGameObject(root);
		current_scene_canvas = nullptr;
		//TODO: modules should have remove GameObject events and load scene events
		App->editor->selected.clear();

		root = nullptr;
		dynamic_gameobjects.clear();
		octree.Create(OCTREE_SIZE);
	}
}

GameObject * ModuleGOManager::LoadGameObject(const Data & go_data) 
{
	const char* name = go_data.GetString("name");
	unsigned int uuid = go_data.GetUInt("UUID");

	unsigned int uuid_parent = go_data.GetUInt("parent");
	//Find parent GameObject reference
	GameObject* parent = nullptr;
	if (uuid_parent != 0 && root)
	{
		parent = FindGameObjectByUUID(root, uuid_parent);
	}

	bool active = go_data.GetBool("active");	
	bool is_prefab = go_data.GetBool("is_prefab");
	unsigned int prefab_root_uuid = go_data.GetUInt("prefab_root_uuid");
	string prefab_path = go_data.GetString("prefab_path");

	GameObject* go = nullptr;

	if (is_prefab == false)
	{	//Normal GameObject
		bool is_static = go_data.GetBool("static");
		int layer = go_data.GetInt("layer");

		//Basic GameObject properties
		go = new GameObject(name, uuid, parent, active, is_static, is_prefab, layer, prefab_root_uuid, prefab_path);
		go->local_uuid = go_data.GetUInt("local_UUID");
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
	}
	else
	{	//Prefab
		ResourceFilePrefab* rc_prefab = (ResourceFilePrefab*)App->resource_manager->LoadResource(prefab_path, ResourceFileType::RES_PREFAB);
		if (rc_prefab)
		{
			go = rc_prefab->LoadPrefabFromScene(go_data, parent);
			if (go != nullptr)
				go->rc_prefab = rc_prefab;
			else
			{
				LOG("[ERROR] When loading prefab '%s'", prefab_path.c_str());
				App->editor->DisplayWarning(WarningType::W_ERROR, "When loading prefab '%s'", prefab_path.c_str());
			}
		}
	}

	if (go != nullptr && go->bounding_box != nullptr)
	{
		//Space partioning
		if (go->IsStatic())
			octree.Insert(go, go->bounding_box->CenterPoint()); //Needs to go after the components because of the bounding box reference
		else
			dynamic_gameobjects.push_back(go);
	}

	return go;
}

void ModuleGOManager::SetCurrentAssetsScenePath(const char* scene_path)
{
	current_assets_scene_path = scene_path;
}

const char* ModuleGOManager::GetCurrentAssetsScenePath() const
{
	return current_assets_scene_path.c_str();
}

void ModuleGOManager::SetCurrentLibraryScenePath(const char* scene_path)
{
	current_library_scene_path = scene_path;
}

const char* ModuleGOManager::GetCurrentLibraryScenePath() const
{
	return current_library_scene_path.c_str();
}

GameObject* ModuleGOManager::LoadPrefabGameObject(const Data & go_data, map<unsigned int, unsigned int>& uuids)
{
	const char* name = go_data.GetString("name");
	unsigned int uuid = App->rnd->RandomInt();
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
	
	unsigned int prefab_root_uuid = 0;
	string prefab_path;

	if (is_prefab)
	{
		prefab_root_uuid = uuids.find(go_data.GetUInt("prefab_root_uuid"))->second;
		string prefab_path = go_data.GetString("prefab_path");
	}
	else
		prefab_path = "";

	int layer = go_data.GetInt("layer");

	//Find parent GameObject reference
	GameObject* parent = nullptr;
	if (uuid_parent != 0)
		parent = FindGameObjectByUUID(root, uuid_parent);
	else
		parent = root;

	//Basic GameObject properties
	GameObject* go = new GameObject(name, uuid, parent, active, is_static, is_prefab, layer, prefab_root_uuid, prefab_path);

	if(is_prefab)
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
		if(go_component != nullptr)
			go_component->Load(component);
	}

	if (is_static)
		octree.Insert(go, go->bounding_box->CenterPoint()); //Needs to go after the components because of the bounding box reference
	else
		dynamic_gameobjects.push_back(go);

	return go;
}

GameObject * ModuleGOManager::FindGameObjectByUUID(GameObject* start, unsigned int uuid) const
{
	GameObject* ret = nullptr;
	if (start)
	{
		if (start->GetUUID() == uuid)
		{
			ret = start;
		}
		else
		{
			const std::vector<GameObject*>* childs = start->GetChilds();
			for (std::vector<GameObject*>::const_iterator child = childs->begin(); child != childs->end(); ++child)
			{
				ret = FindGameObjectByUUID((*child), uuid);
				if (ret != nullptr)
					break;
			}
		}
	}
	
	return ret;
}

void ModuleGOManager::LinkGameObjectPointer(GameObject **pointer_to_pointer_go, unsigned int uuid_to_assign)
{
	*pointer_to_pointer_go = FindGameObjectByUUID(root, uuid_to_assign);
}

//Sort the AABBs for the distance from the current camera
int  CompareAABB(const void * a, const void * b)
{
	float3 cam_pos = App->camera->GetPosition();
	float a_dst = (cam_pos - ((GameObject*)a)->bounding_box->CenterPoint()).Length();
	float b_dst = (cam_pos - ((GameObject*)b)->bounding_box->CenterPoint()).Length();
	if (a_dst < b_dst) return -1;
	if (a_dst = b_dst) return 0;
	if (a_dst > b_dst) return 1;
	return 99999;
}

RaycastHit ModuleGOManager::Raycast(const Ray & ray, std::vector<int> layersToCheck, bool keepDrawing)
{
	vector<GameObject*> collisions;
	octree.Intersect(collisions, ray);
	for (list<GameObject*>::const_iterator dyn_go = dynamic_gameobjects.begin(); dyn_go != dynamic_gameobjects.end(); dyn_go++)
	{
		if ((*dyn_go)->bounding_box)
		{
			if (ray.Intersects(*(*dyn_go)->bounding_box))
			{
				collisions.push_back((*dyn_go));
			}
		}
	}
	std::sort(collisions.begin(), collisions.end(), CompareAABB);

	vector<GameObject*>::iterator it = collisions.begin(); //Test with vertices
	RaycastHit hit;
	for (;it != collisions.end(); it++)
	{
		//If layers to check is empty, object must be checked (check = true)
		bool check = layersToCheck.empty();		
		for (std::vector<int>::iterator l = layersToCheck.begin(); l != layersToCheck.end() && check == false; l++)
		{
			if ((*it)->layer == *l) { check = true; }
		}
		if (check)
		{
			if ((*it)->RayCast(ray, hit))
			{
				break;
			}
		}
	}

	if (keepDrawing && hit.object != nullptr)
	{
		lastRayData[0] = ray.pos;
		lastRayData[1] = hit.point;
		lastRayData[2] = hit.normal;
	}

	return hit;
}

AABB ModuleGOManager::GetWorldAABB(std::vector<int> layersToCheck)
{
	AABB ret;
	std::vector<float3> points = GetWorldAABB(layersToCheck, root);
	if (points.empty() == false)
	{
		ret.minPoint = points[0];
		ret.maxPoint = points[1];
	}
	else
	{
		ret.maxPoint = float3::zero;
		ret.minPoint = float3::zero;
	}
	return ret;
}

std::vector<float3> ModuleGOManager::GetWorldAABB(std::vector<int> layersToCheck, GameObject * go)
{
	//return, [0] is min Point and [1] is MaxPoint
	std::vector<float3> ret;

	//Checking if we must check this object according to the layers. If no layers are passed, all of them are checked
	bool inLayer = false;
	if (layersToCheck.empty() == false)
	{
		for (std::vector<int>::iterator l = layersToCheck.begin(); l != layersToCheck.end(); l++)
		{
			if (go->layer == *l)
			{
				inLayer = true;
				break;
			}
		}
	}
	else
	{
		inLayer = true;
	}

	if (inLayer)
	{
		//If we need to consider the GO, we add Max and Min points.
		ComponentMesh* msh = (ComponentMesh*) go->GetComponent(C_MESH);
		
		if (msh)
		{
			ret.push_back(msh->GetBoundingBox().minPoint);
			ret.push_back(msh->GetBoundingBox().maxPoint);
		}
		else
		{
			ret.push_back(go->transform->GetPosition());
			ret.push_back(go->transform->GetPosition());
		}
	}

	//We check all childs and keep the min and max points
	std::vector<GameObject*>::const_iterator it = go->GetChilds()->begin();
	for(; it != go->GetChilds()->end(); it++)
	{
		std::vector<float3> p = GetWorldAABB(layersToCheck, *it);
		if (p.empty() == false)
		{
			if (ret.empty() == true)
			{
				ret = p;
			}
			else
			{
				ret[0].x = min(ret[0].x, p[0].x); ret[0].y = min(ret[0].y, p[0].y); ret[0].z = min(ret[0].z, p[0].z);
				ret[1].x = max(ret[1].x, p[1].x); ret[1].y = max(ret[1].y, p[1].y);	ret[1].z = max(ret[1].z, p[1].z);
			}
		}
	}
	return ret;
}

void ModuleGOManager::LinkAnimation(GameObject* root) const
{
	if (root == nullptr)
		return;

	ComponentAnimation* c_anim = (ComponentAnimation*)root->GetComponent(C_ANIMATION);

	if (c_anim)
	{
		if (c_anim->linked == false)
		{
			c_anim->LinkAnimation();
		}
	}

	const vector<GameObject*>* childs = root->GetChilds();
	for (vector<GameObject*>::const_iterator child = (*childs).begin(); child != (*childs).end(); ++child)
		LinkAnimation(*child);
}

void ModuleGOManager::UpdateGameObjects(GameObject* object)
{
	if(root != object && object->IsActive() == true)
		object->Update();

	std::vector<GameObject*>::const_iterator child = object->GetChilds()->begin();
	for (child; child != object->GetChilds()->end(); ++child)
	{
		UpdateGameObjects((*child));
	}
}

void ModuleGOManager::PreUpdateGameObjects(GameObject * obj)
{
	if (root != obj && obj->IsActive() == true)
		obj->PreUpdate();

	std::vector<GameObject*>::const_iterator child = obj->GetChilds()->begin();
	for (child; child != obj->GetChilds()->end(); ++child)
	{
		PreUpdateGameObjects((*child));
	}
}

void ModuleGOManager::OnPlay()
{
	std::vector<GameObject*>::const_iterator child = root->GetChilds()->begin();
	for (; child != root->GetChilds()->end(); ++child)
	{
		OnPlayGameObjects((*child));
	}
}

void ModuleGOManager::OnPlayGameObjects(GameObject * obj)
{
	obj->OnPlay();
	std::vector<GameObject*>::const_iterator child = obj->GetChilds()->begin();
	for (; child != obj->GetChilds()->end(); ++child)
	{
		OnPlayGameObjects((*child));
	}
}

void ModuleGOManager::OnPause()
{
	std::vector<GameObject*>::const_iterator child = root->GetChilds()->begin();
	for (; child != root->GetChilds()->end(); ++child)
	{
		OnPauseGameObjects((*child));
	}
}

void ModuleGOManager::OnPauseGameObjects(GameObject * obj)
{
	obj->OnPause();
	std::vector<GameObject*>::const_iterator child = obj->GetChilds()->begin();
	for (; child != obj->GetChilds()->end(); ++child)
	{
		OnPauseGameObjects((*child));
	}
}

void ModuleGOManager::OnStop()
{
	if (root != nullptr)
	{
		std::vector<GameObject*>::const_iterator child = root->GetChilds()->begin();
		for (; child != root->GetChilds()->end(); ++child)
		{
			OnStopGameObjects((*child));
		}
	}
}

void ModuleGOManager::OnStopGameObjects(GameObject * obj)
{
	obj->OnStop();
	std::vector<GameObject*>::const_iterator child = obj->GetChilds()->begin();
	for (; child != obj->GetChilds()->end(); ++child)
	{
		OnStopGameObjects((*child));
	}
}
