#include "Application.h"
#include "ModuleGOManager.h"
#include "Component.h"
#include "GameObject.h"
#include "Imgui\imgui.h"
#include "ComponentCamera.h"
#include "ComponentMesh.h"
#include "RaycastHit.h"
#include <algorithm>
#include "ComponentLight.h"
#include "LayerSystem.h"

ModuleGOManager::ModuleGOManager(const char* name, bool start_enabled) : Module(name, start_enabled)
{}

ModuleGOManager::~ModuleGOManager()
{
	if (root)
		delete root;

	selected_GO = nullptr;
	dynamic_gameobjects.clear();
	delete layer_system;
}

bool ModuleGOManager::Init(Data & config)
{
	//Load last open scene
	const char* path = config.GetString("current_scene_path");
	bool scene_success = false;
	if (path)
	{
		if (strcmp(path, "") != 0)
		{
			current_scene_path = path; //The scene is loaded at start because OpenGL needs to be init at ModuleRender
			scene_success = true;
		}
	}

	if (!scene_success)
	{
		LoadEmptyScene();
	}

	layer_system = new LayerSystem();
	layer_system->Load(config);
	
	return true;
}

bool ModuleGOManager::Start()
{
	octree.Create(OCTREE_SIZE);
	//Load last scene 
	if (root == nullptr)
	{
		if (App->resource_manager->LoadScene(current_scene_path.data()) == false)
		{
			LoadEmptyScene();
		}
	}

	return true;
}

update_status ModuleGOManager::PreUpdate()
{
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
	//Update GameObjects
	if(root)
		UpdateGameObjects(time->DeltaTime(), root);

	//Display windows
	HierarchyWindow();
	InspectorWindow();

	PickObjects();

	//Selected Object shows it's boudning box
	if (selected_GO)
	{
		if (selected_GO->bounding_box)
		{
			g_Debug->AddAABB(*selected_GO->bounding_box, g_Debug->green);
		}
	}

	if(draw_octree)
		octree.Draw();

	return UPDATE_CONTINUE;
}

void ModuleGOManager::SaveBeforeClosing(Data& data) const
{
	data.AppendString("current_scene_path", current_scene_path.data());
	data.AppendArray("layers");
	layer_system->Save(data);
}

GameObject* ModuleGOManager::CreateGameObject(GameObject* parent)
{
	GameObject* obj_parent = (parent) ? parent : root;

	GameObject* object = new GameObject(obj_parent);

	if (obj_parent->AddChild(object) == false)
		LOG("A child insertion to GameObject %s could not be done", obj_parent->name.data());

	dynamic_gameobjects.push_back(object);

	return object;
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

void ModuleGOManager::GetAllCameras(std::vector<ComponentCamera*>& list, GameObject* from) const
{
	GameObject* go = (from) ? from : root;
	
	ComponentCamera* cam = (ComponentCamera*)go->GetComponent(C_CAMERA);
	if (cam)
		list.push_back(cam);
	
	const vector<GameObject*>* childs = go->GetChilds();
	for (vector<GameObject*>::const_iterator child = childs->begin(); child != childs->end(); ++child)
		GetAllCameras(list, (*child));
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

	//Empty scene
	root = new GameObject();
	root->name = "Root";
	current_scene_path = "";
	App->camera->ChangeCurrentCamera(App->camera->GetEditorCamera());
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

void ModuleGOManager::PickObjects()
{
	if (App->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_UP && App->input->GetKey(SDL_SCANCODE_LALT) == KEY_REPEAT)
	{
		Ray ray = App->camera->GetCurrentCamera()->CastCameraRay(float2(App->input->GetMouseX(), App->input->GetMouseY()));
		selected_GO = Raycast(ray);
	}
}

void ModuleGOManager::SaveSceneBeforeRunning()
{
	Data root_node;
	root_node.AppendArray("GameObjects");

	root->Save(root_node);

	char* buf;
	size_t size = root_node.Serialize(&buf);

	App->file_system->Save("Library/current_scene.json", buf, size); //TODO: Find the right place to save the scene.

	delete[] buf;
}

void ModuleGOManager::LoadSceneBeforeRunning()
{
	App->resource_manager->LoadScene("Library/current_scene.json");
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
	RemoveGameObject(root);
	selected_GO = nullptr;
	root = nullptr;
	dynamic_gameobjects.clear();
	octree.Create(OCTREE_SIZE);
}

GameObject * ModuleGOManager::LoadGameObject(const Data & go_data) 
{
	const char* name = go_data.GetString("name");
	unsigned int uuid = go_data.GetUInt("UUID");
	unsigned int uuid_parent = go_data.GetUInt("parent");
	bool active = go_data.GetBool("active");
	bool is_static = go_data.GetBool("static");
	bool is_prefab = go_data.GetBool("is_prefab");
	int layer = go_data.GetInt("layer");
	//Find parent GameObject reference
	GameObject* parent = nullptr;
	if (uuid_parent != 0 && root)
	{
		parent = FindGameObjectByUUID(root, uuid_parent);
	}

	//Basic GameObject properties
	GameObject* go = new GameObject(name, uuid, parent, active, is_static, is_prefab, layer);
	go->local_uuid = go_data.GetUInt("local_UUID");
	if(parent)
		parent->AddChild(go);
	
	//Components
	Data component;
	unsigned int comp_size = go_data.GetArraySize("components");
	for (int i = 0; i < comp_size; i++)
	{
		component = go_data.GetArray("components", i);

		int type = component.GetInt("type");
		Component* go_component;
		if(type != (int)ComponentType::C_TRANSFORM)
			go_component = go->AddComponent(static_cast<ComponentType>(type));
		else
			go_component = (Component*)go->GetComponent(C_TRANSFORM);
		go_component->Load(component);
	}

	if (is_static)
		octree.Insert(go, go->bounding_box->CenterPoint()); //Needs to go after the components because of the bounding box reference
	else
		dynamic_gameobjects.push_back(go);

	return go;
}

void ModuleGOManager::SetCurrentScenePath(const char * scene_path)
{
	current_scene_path = scene_path;
}

void ModuleGOManager::LoadPrefabGameObject(const Data & go_data, map<unsigned int, unsigned int>& uuids)
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
	int layer = go_data.GetInt("layer");

	//Find parent GameObject reference
	GameObject* parent = nullptr;
	if (uuid_parent != 0)
		parent = FindGameObjectByUUID(root, uuid_parent);
	else
		parent = root;

	//Basic GameObject properties
	GameObject* go = new GameObject(name, uuid, parent, active, is_static, is_prefab, layer);

	if(is_prefab)
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
		octree.Insert(go, go->bounding_box->CenterPoint()); //Needs to go after the components because of the bounding box reference
	else
		dynamic_gameobjects.push_back(go);
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

//Sort the AABBs for the distance from the current camera
int  CompareAABB(const void * a, const void * b)
{
	float3 cam_pos = App->camera->GetPosition();
	float a_dst = (cam_pos - ((GameObject*)a)->bounding_box->CenterPoint()).Length();
	float b_dst = (cam_pos - ((GameObject*)b)->bounding_box->CenterPoint()).Length();
	if (a_dst < b_dst) return -1;
	if (a_dst = b_dst) return 0;
	if (a_dst > b_dst) return 1;
}

GameObject * ModuleGOManager::Raycast(const Ray & ray) const
{
	GameObject* ret = nullptr;

	vector<GameObject*> collisions;
	octree.Intersect(collisions, ray);
	for (list<GameObject*>::const_iterator dyn_go = dynamic_gameobjects.begin(); dyn_go != dynamic_gameobjects.end(); dyn_go++)
		if((*dyn_go)->bounding_box)
			if (ray.Intersects(*(*dyn_go)->bounding_box))
				collisions.push_back((*dyn_go));

	std::sort(collisions.begin(), collisions.end(), CompareAABB);

	vector<GameObject*>::iterator it = collisions.begin(); //Test with vertices
	RaycastHit hit;
	while (it != collisions.end())
	{
		if ((*it)->RayCast(ray, hit))
		{
			ret = (*it);
			break;
		}
		++it;
	}

	return ret;
}

void ModuleGOManager::HierarchyWindow()
{
	ImGui::Begin("Hierarchy");

	DisplayGameObjectsChilds(root->GetChilds());

	if (ImGui::IsMouseHoveringWindow())
		if (ImGui::IsMouseClicked(1))
			ImGui::OpenPopup("HierarchyOptions");

	if (ImGui::BeginPopup("HierarchyOptions"))
	{
		if (ImGui::Selectable("Create Empty GameObject"))
		{
			selected_GO = CreateGameObject(NULL);
		}

		if (ImGui::Selectable("Create Empty Child"))
		{
			selected_GO = CreateGameObject(selected_GO);
		}

		if (ImGui::Selectable("Remove selected GameObject"))
		{
			if (selected_GO != nullptr)
			{
				RemoveGameObject(selected_GO);
				selected_GO = nullptr;
			}
		}
		if (ImGui::Selectable("Create Prefab"))
		{
			if (selected_GO != nullptr)
			{
				App->resource_manager->SavePrefab(selected_GO);
			}
		}

		ImGui::EndPopup();
	}

	ImGui::End();
}

void ModuleGOManager::DisplayGameObjectsChilds(const std::vector<GameObject*>* childs)
{
	for (vector<GameObject*>::const_iterator object = (*childs).begin(); object != (*childs).end(); ++object)
	{
		uint flags = 0;
		if ((*object) == selected_GO)
			flags = ImGuiTreeNodeFlags_Selected;
		
		if ((*object)->ChildCount() > 0)
		{
			if (ImGui::TreeNodeEx((*object)->name.data(), flags))
			{
				if (ImGui::IsItemClicked(0))
				{
					selected_GO = (*object);
				}

				DisplayGameObjectsChilds((*object)->GetChilds());
				ImGui::TreePop();
			}
		}
		else
		{
			if (ImGui::TreeNodeEx((*object)->name.data(), flags | ImGuiTreeNodeFlags_Leaf))
			{
				if (ImGui::IsItemClicked(0))
				{
					selected_GO = (*object);
				}
				ImGui::TreePop();
			}
		}
	}
}

void ModuleGOManager::InspectorWindow()
{
	ImGui::Begin("Inspector");

	ImGui::Text("Debug: "); ImGui::SameLine(); ImGui::Checkbox("##debug_inspector", &debug_inspector);
	ImGui::Separator();

	if (selected_GO)
	{

		//Active
		bool is_active = selected_GO->IsActive();
		if (ImGui::Checkbox("", &is_active))
		{
			selected_GO->SetActive(is_active);
		}

		//Name
		ImGui::SameLine();
		ImGui::InputText("###goname", selected_GO->name._Myptr(), selected_GO->name.capacity());

		//Static
		ImGui::SameLine();
		ImGui::Text("Static:");
		ImGui::SameLine();
		bool is_static = selected_GO->IsStatic();
		if (ImGui::Checkbox("###static_option", &is_static))
		{
			selected_GO->SetStatic(is_static);
		}

		if (selected_GO->IsPrefab())
		{
			ImGui::TextColored(ImVec4(0, 0.5f, 1, 1), "Prefab: ");
		}

		ImGui::Separator();
		layer_system->DisplayLayerSelector(selected_GO->layer);

		if (debug_inspector)
		{
			ImGui::Text("UUID: %u", (int)selected_GO->GetUUID());
			ImGui::Text("Local UUID: %u", (int)selected_GO->local_uuid);
			ImGui::Text("Layer id: %i", selected_GO->layer);
		}

		//Components
		const std::vector<Component*>* components = selected_GO->GetComponents();
		for (std::vector<Component*>::const_iterator component = (*components).begin(); component != (*components).end(); ++component)
		{
			(*component)->OnInspector();
		}

		//Options
		if (ImGui::IsMouseHoveringWindow())
			if (ImGui::IsMouseClicked(1))
				ImGui::OpenPopup("InspectorOptions");

		if (ImGui::BeginPopup("InspectorOptions"))
		{
			if (ImGui::Selectable("Add Transform"))
			{
				selected_GO->AddComponent(C_TRANSFORM);
			}

			if (ImGui::Selectable("Add Mesh"))
			{
				selected_GO->AddComponent(C_MESH);
			}

			if (ImGui::Selectable("Add Material"))
			{
				selected_GO->AddComponent(C_MATERIAL);
			}

			if (ImGui::Selectable("Add Camera"))
			{
				selected_GO->AddComponent(C_CAMERA);
			}

			if (ImGui::Selectable("Add Light"))
			{
				selected_GO->AddComponent(C_LIGHT);
			}

			ImGui::EndPopup();
		}
	}

	ImGui::End();
}


void ModuleGOManager::UpdateGameObjects(float dt, GameObject* object)
{
	PROFILE("ModuleGOManager::UpdateGameObjects");

	if(root != object && object->IsActive() == true)
		object->Update();

	std::vector<GameObject*>::const_iterator child = object->GetChilds()->begin();
	for (child; child != object->GetChilds()->end(); ++child)
	{
		UpdateGameObjects(dt, (*child));
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
