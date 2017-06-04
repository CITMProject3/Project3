#include "Application.h"

#include "ModuleEditor.h"

#include "GameObject.h"
#include "Component.h"
#include "ComponentTransform.h"
#include "ComponentMesh.h"
#include "ComponentMaterial.h"
#include "ComponentCamera.h"
#include "ComponentLight.h"
#include "ComponentAudioSource.h"
#include "ComponentAudioListener.h"
#include "ComponentCollider.h"
#include "ComponentCar.h"
#include "ComponentRectTransform.h"
#include "ComponentUiImage.h"
#include "ComponentUiText.h"
#include "ComponentCanvas.h"
#include "ComponentUiButton.h"
#include "ComponentGrid.h"
#include "ComponentSprite.h"
#include "ComponentParticleSystem.h"

#include "MeshImporter.h"
#include "RaycastHit.h"
#include "ComponentScript.h"
#include "ComponentLight.h"
#include "ComponentAnimation.h"
#include "ComponentBone.h"
#include "ModuleGOManager.h"
#include "ResourceFilePrefab.h"

#include "Random.h"

GameObject::GameObject()
{
	name.resize(30);
	name = "Empty GameObject";
	uuid = App->rnd->RandomInt();
	AddComponent(C_TRANSFORM);
}

GameObject::GameObject(GameObject* parent) : parent(parent)
{
	name.resize(30);
	name = "Empty GameObject";
	uuid = App->rnd->RandomInt();
	AddComponent(C_TRANSFORM);
	if (parent)
	{
		if (parent->IsPrefab())
		{
			is_prefab = true;
			prefab_path = parent->prefab_path;
			prefab_root_uuid = parent->prefab_root_uuid;
		}
	}
}

GameObject::GameObject(const char* name, unsigned int uuid, GameObject* parent, bool active, bool is_static, bool is_prefab, int layer, unsigned int prefab_root_uuid, const string& prefab_path) 
	: name(name), uuid(uuid), parent(parent), active(active), is_static(is_static), is_prefab(is_prefab), layer(layer), prefab_root_uuid(prefab_root_uuid), prefab_path(prefab_path)
{
	AddComponent(C_TRANSFORM);
}

GameObject::~GameObject()
{
	global_matrix = nullptr;
	bounding_box = nullptr;
	mesh_to_draw = nullptr;
	for (std::vector<Component*>::iterator component = components.begin(); component != components.end(); ++component)
	{
		delete (*component);
		(*component) = nullptr;
	}

	components.clear();
	components_to_remove.clear();

	if (rc_prefab)
	{
		rc_prefab->UnloadInstance(this);
		rc_prefab->Unload();
	}
}

void GameObject::PreUpdate()
{
	//Reset elements to draw
	mesh_to_draw = nullptr;

	//Remove all components that need to be removed. Secure way.
	for (std::vector<Component*>::iterator component = components_to_remove.begin(); component != components_to_remove.end(); ++component)
	{
		for (std::vector<Component*>::iterator comp_remove = components.begin(); comp_remove != components.end(); ++comp_remove) //Remove the component from the components list
		{
			if ((*comp_remove) == (*component))
			{
				components.erase(comp_remove);
				break;
			}
		}
		delete (*component);
	}

	components_to_remove.clear();
}

void GameObject::Update()
{
	for (std::vector<Component*>::iterator comp = components.begin(); comp != components.end(); comp++)
	{
		(*comp)->PreUpdate();
	}
	for (std::vector<Component*>::iterator comp = components.begin(); comp != components.end(); comp++)
	{
		(*comp)->Update();
	}
	for (std::vector<Component*>::iterator comp = components.begin(); comp != components.end(); comp++)
	{
		(*comp)->PostUpdate();
	}
}

bool GameObject::AddChild(GameObject* child)
{
	bool ret = false;
	if (child)
	{
		childs.push_back(child);
		ret = true;
	}

	return ret;
}

bool GameObject::RemoveChild(GameObject* child)
{
	bool ret = false;

	if (child)
	{
		std::vector<GameObject*>::iterator item = childs.begin();

		while (item != childs.end())
		{
			if (*item == child)
			{
				childs.erase(item);
				ret = true;
				break;
			}
			++item;
		}
	}

	return ret;
}

void GameObject::RemoveAllChilds()
{
	for (size_t i = 0; i < childs.size(); i++)
	{
		App->go_manager->FastRemoveGameObject(childs[i]);
	}

	childs.clear();
}

GameObject* GameObject::GetParent()const
{
	return parent;
}

//TODO: keep world transformation matrix, change local
void GameObject::SetParent(GameObject * parent)
{
	if (this->parent != parent && parent != this)
	{
		if (this->parent != nullptr)
		{
			this->parent->RemoveChild(this);
		}

		float4x4 global = transform->GetGlobalMatrix();
		this->parent = parent;
		if (parent)
		{
			float4x4 new_local = parent->transform->GetGlobalMatrix().Inverted() * global;
			float3 translate, scale;
			Quat rotation;
			new_local.Decompose(translate, rotation, scale);
			transform->SetPosition(translate);
			transform->SetRotation(rotation);
			transform->SetScale(scale);

			parent->AddChild(this);

			if (rc_prefab == nullptr)
			{
				if (parent->IsPrefab())
				{
					prefab_path = parent->prefab_path;
					SetAsPrefab(parent->prefab_root_uuid);
				}
				else
				{
					if (is_prefab)
					{
						is_prefab = false;
						prefab_root_uuid = 0;
						prefab_path = "";
						local_uuid = 0;

						for (vector<GameObject*>::iterator child = childs.begin(); child != childs.end(); ++child)
						{
							(*child)->is_prefab = false;
							(*child)->prefab_root_uuid = 0;
							(*child)->prefab_path = "";
							(*child)->local_uuid = 0;
						}
					}
				}
			}	
		}
	}
}

const std::vector<GameObject*>* GameObject::GetChilds()
{
	return &childs;
}

size_t GameObject::ChildCount()
{
	return childs.size();
}

void GameObject::CollectAllChilds(std::vector<GameObject*>& vector)
{
	vector.push_back(this);
	for (uint i = 0; i < childs.size(); i++)
		childs[i]->CollectAllChilds(vector);
}

void GameObject::OnPlay()
{
	for (std::vector<Component*>::iterator comp = components.begin(); comp != components.end(); comp++)
	{
		(*comp)->OnPlay();
	}
}

void GameObject::OnStop()
{
	for (std::vector<Component*>::iterator comp = components.begin(); comp != components.end(); comp++)
	{
		(*comp)->OnStop();
	}
}

void GameObject::OnPause()
{
	for (std::vector<Component*>::iterator comp = components.begin(); comp != components.end(); comp++)
	{
		(*comp)->OnPause();
	}
}

bool *GameObject::GetActiveBoolean()
{
	return &active;
}

bool GameObject::IsActive() const
{	
	// Obtaning all gameobjects from "this" to root, up in the hierarchy
	const GameObject *curr_go = this;
	std::vector<const GameObject*> hierarchy_gos;	

	while (curr_go != nullptr && !App->go_manager->IsRoot(curr_go))
	{
		hierarchy_gos.push_back(curr_go);
		curr_go = curr_go->parent;
	}

	// Checking if all Gameobjects are active
	bool is_active = true;
	for (std::vector<const GameObject*>::const_iterator it = hierarchy_gos.begin(); it != hierarchy_gos.end(); ++it)
	{
		if (!(*it)->active)
		{
			is_active = false;	 // There is one that it's not active, so...
			break;				 // this gameobject is not active at the end.
		}			
	}

	return is_active;
}

void GameObject::SetActive(bool value)
{
	if (value != active) active = value;
}

void GameObject::SetAllActive(bool value)
{
	if (value != active) active = value;
	for (uint i = 0; i < childs.size(); i++)
	{
		childs[i]->SetAllActive(value);
	}
}

bool GameObject::IsStatic() const
{
	return is_static;
}

void GameObject::SetStatic(bool value, bool changeChilds)
{	//Conditions: if is static parent MUST be static too. If is dynamic childs must be dynamic too.
	if (is_static != value && App->go_manager->IsRoot(this) == false)
	{
		is_static = value;
		if (is_static) //Set parents static too. Except root.
		{
			bool ret = App->go_manager->InsertGameObjectInOctree(this);
			if (!ret)
				LOG("INSERTION FAILED");
			if (parent)
				parent->SetStatic(true);
		}
		else //Set childs dynamic too
		{
			bool ret = App->go_manager->RemoveGameObjectOfOctree(this);
			if (!ret)
				LOG("REMOVING FAILED");
			for (std::vector<GameObject*>::iterator child = childs.begin(); child != childs.end(); ++child)
				(*child)->SetStatic(false);
		}
		if (changeChilds && childs.empty() == false)
		{
			for (std::vector<GameObject*>::iterator it = childs.begin(); it != childs.end(); it++)
			{
				(*it)->SetStatic(value, changeChilds);
			}
		}
	}
}

void GameObject::ForceStatic(bool value)
{
	//Only inserts the gameobject in the octree or dynamic vector. Doesn't remove if was inserted previously. Doesn't update parent or children
	is_static = value;
	if (is_static) 
	{
		bool ret = App->go_manager->InsertGameObjectInOctree(this);
		if (!ret)
			LOG("INSERTION FAILED");
	}
	else 
	{
		bool ret = App->go_manager->RemoveGameObjectOfOctree(this);
		if (!ret)
			LOG("REMOVING FAILED");
	}
}

void GameObject::SetAsPrefab(unsigned int root_uuid)
{
	is_prefab = true;
	local_uuid = uuid;
	prefab_root_uuid = root_uuid;
	for (vector<GameObject*>::iterator child = childs.begin(); child != childs.end(); ++child)
		(*child)->SetAsPrefab(root_uuid);
}

bool GameObject::IsPrefab() const
{
	return is_prefab;
}

void GameObject::SetLayerChilds(int _layer)
{
	layer = _layer;
	for (vector<GameObject*>::iterator child = childs.begin(); child != childs.end(); ++child)
		(*child)->SetLayerChilds(_layer);
}

Component* GameObject::AddComponent(ComponentType type)
{
	Component* item = nullptr;
	switch (type)
	{
	case C_TRANSFORM:
		if (!transform) //Only one transform component for gameobject
		{
			item = new ComponentTransform(type, this, &global_matrix);
			transform = (ComponentTransform*)item;
		}		
		break;
	case C_MESH:
		if(transform)
			item = new ComponentMesh(type, this);
		break;
	case C_MATERIAL:
		if (transform && GetComponent(C_MESH))
			item = new ComponentMaterial(type, this);
		break;
	case C_CAMERA:
		if (transform)
			item = new ComponentCamera(type, this);
		break;
	case C_COLLIDER:
		if (transform)
			item = new ComponentCollider(this);
		break;
	case C_LIGHT:	
		if (transform)
			item = new ComponentLight(type, this);
		break;
	case C_ANIMATION:
		if (transform && GetComponent(C_ANIMATION) == nullptr)
			item = new ComponentAnimation(this);
		break;
	case C_BONE:
		if (transform)
			item = new ComponentBone(this);
		break;
	case C_CAR:
		if (transform)
			item = new ComponentCar(this);
		break;
	case C_AUDIO_SOURCE:
		if (transform)
			item = new ComponentAudioSource(type, this);
		break;
	case C_AUDIO_LISTENER:
		if (transform)
			item = new ComponentAudioListener(type, this);
		break;
	case C_SCRIPT:
		item = new ComponentScript(type, this);
		break;
	case C_RECT_TRANSFORM:
		if (GetComponent(type) == nullptr) // Only one rect transform component for gameobject
			item = new ComponentRectTransform(type, this);
		break;
	case C_UI_IMAGE:
		if (!GetComponent(C_RECT_TRANSFORM)) AddComponent(C_RECT_TRANSFORM);			
		item = new ComponentUiImage(type, this);
		break;
	case C_UI_TEXT:
		if (!GetComponent(C_RECT_TRANSFORM)) AddComponent(C_RECT_TRANSFORM);
		item = new ComponentUiText(type,this);
		break;
	case C_UI_BUTTON:
		if (!GetComponent(C_RECT_TRANSFORM)) AddComponent(C_RECT_TRANSFORM);
		item = new ComponentUiButton(type, this);
		break;
	case C_GRID:
		if (!GetComponent(C_RECT_TRANSFORM)) AddComponent(C_RECT_TRANSFORM);
		item = new ComponentGrid(type, this);
		break;
	case C_CANVAS:
		if (App->go_manager->current_scene_canvas == nullptr)
		{
			if (!GetComponent(C_RECT_TRANSFORM)) AddComponent(C_RECT_TRANSFORM);
			item = new ComponentCanvas(type, this);
			App->go_manager->current_scene_canvas = (ComponentCanvas*)item;
		}
		break;
	case C_SPRITE:
		if (transform)
			item = new ComponentSprite(type, this);
		break;
	case C_PARTICLE_SYSTEM:
		item = new ComponentParticleSystem(type, this);
		break;
	default:
		LOG("[WARNING] Unknown type specified for GameObject %s", name);
		App->editor->DisplayWarning(WarningType::W_WARNING, "Unknown type specified for GameObject %s", name);
		break;
	}

	if (item != nullptr)
	{
		components.push_back(item);
	}
	else
	{
		LOG("[ERROR] When adding component to %s", this->name.c_str());
		App->editor->DisplayWarning(WarningType::W_ERROR, "When adding component to %s", this->name.c_str());
	}		

	return item;
}

const std::vector<Component*>* GameObject::GetComponents()
{
	return &components;
}

Component* GameObject::GetComponent(ComponentType type)const
{
	if (components.empty() == false)
	{
		if (type == C_TRANSFORM)
			return transform;

		std::vector<Component*>::const_iterator comp = components.begin();
		while (comp != components.end())
		{
			if ((*comp)->GetType() == type)
			{
				return (*comp);
			}
			++comp;
		}
	}
	return nullptr;
}

Component* GameObject::GetComponentInChilds(ComponentType type) const
{
	Component* ret = nullptr;
	if ((ret = GetComponent(type)) != nullptr)
		return ret;
	vector<GameObject*>::const_iterator it = childs.begin();
	for (it; it != childs.end(); ++it)
	{
		if ((ret = (*it)->GetComponentInChilds(type)) != nullptr)
		{
			return ret;
		}
	}
	return ret;
}

void GameObject::GetComponentsInChilds(ComponentType type, std::vector<Component*>& vector) const
{
	Component* comp = GetComponent(type);
	if (comp != nullptr)
		vector.push_back(comp);

	std::vector<GameObject*>::const_iterator it = childs.begin();
	for (it; it != childs.end(); ++it)
	{
		(*it)->GetComponentsInChilds(type, vector);
	}
}

void GameObject::RemoveComponent(Component * component)
{
	//Search if the component exists inside the GameObject

	std::vector<Component*>::iterator it = components.begin();

	for (it; it != components.end(); ++it)
	{
		if ((*it) == component)
		{
			components_to_remove.push_back(component);
			break;
		}
	}
}

float4x4 GameObject::GetGlobalMatrix() const
{
	return (global_matrix) ? *global_matrix : float4x4::identity;
}

unsigned int GameObject::GetUUID() const
{
	return uuid;
}

void GameObject::TransformModified()
{
	if (global_matrix == nullptr)
		return;
	std::vector<Component*>::iterator component = components.begin();

	for (component; component != components.end(); component++)
	{
		(*component)->OnTransformModified();
	}
}

void GameObject::Save(Data & file, bool ignore_prefab) const
{
	Data data;

	//GameObject data
	data.AppendString("name", name.data());
	data.AppendUInt("UUID", uuid);
	data.AppendUInt("local_UUID", local_uuid);
	if(parent == nullptr)
		data.AppendUInt("parent", 0);
	else
		data.AppendUInt("parent", parent->GetUUID());
	data.AppendBool("active", active);
	
	data.AppendBool("is_prefab", is_prefab);
	data.AppendUInt("prefab_root_uuid", prefab_root_uuid);
	data.AppendString("prefab_path", prefab_path.data());

	if (is_prefab == false || ignore_prefab == true)
	{	//Normal GameObject
		data.AppendInt("layer", layer);
		data.AppendBool("static", is_static);
		data.AppendArray("components");

		//Components data
		vector<Component*>::const_iterator component = components.begin();
		for (component; component != components.end(); component++)
		{
			(*component)->Save(data);
		}

		file.AppendArrayValue(data);

		for (vector<GameObject*>::const_iterator child = childs.begin(); child != childs.end(); ++child)
			(*child)->Save(file, ignore_prefab);
	}
	else
	{	//Prefab GameObject
		//Save Component Transform (translation & rotation)
		ComponentTransform* c_transform = (ComponentTransform*)GetComponent(ComponentType::C_TRANSFORM);

		data.AppendArray("components");
		c_transform->SaveAsPrefab(data);

		//Save all children uuids
		data.AppendArray("children_uuids");
		for (vector<GameObject*>::const_iterator child = childs.begin(); child != childs.end(); ++child)
			(*child)->SaveAsChildPrefab(data);
		file.AppendArrayValue(data);
	}	
}

void GameObject::SaveAsChildPrefab(Data & file) const
{
	Data data;

	data.AppendUInt("uuid", uuid);
	data.AppendUInt("local_uuid", local_uuid);

	for (vector<GameObject*>::const_iterator child = childs.begin(); child != childs.end(); ++child)
		(*child)->SaveAsChildPrefab(file);

	file.AppendArrayValue(data);
}

bool GameObject::RayCast(Ray raycast, RaycastHit & hit_OUT)
{
	RaycastHit hit_info;
	bool ret = false;
	ComponentMesh* c_mesh = (ComponentMesh*)GetComponent(C_MESH);
	if (c_mesh)
	{
		const Mesh* mesh = c_mesh->GetMesh();
		if (mesh)
		{
			//Transform ray into local coordinates
			raycast.Transform(global_matrix->Inverted());

			uint u1, u2, u3;
			float distance;
			vec hit_point;
			Triangle triangle;
			for (unsigned int i = 0; i < mesh->num_indices; i+=3)
			{
				u1 = mesh->indices[i];
				u2 = mesh->indices[i+1];
				u3 = mesh->indices[i+2];
				triangle = Triangle(float3(&mesh->vertices[u1 * 3]), float3(&mesh->vertices[u2 * 3]), float3(&mesh->vertices[u3 * 3]));
				if (raycast.Intersects(triangle, &distance, &hit_point))
				{
					ret = true;
					if (hit_info.distance > distance || hit_info.distance == 0)
					{
						hit_info.distance = distance;
						hit_info.point = hit_point;
						hit_info.normal = triangle.NormalCCW();
					}
				}
				if (ret == true)
				{
					//Transfrom the hit parameters to global coordinates
					hit_OUT.point = global_matrix->MulPos(hit_info.point);
					hit_OUT.distance = hit_info.distance;
					//hit_OUT.distance = ray.pos.Distance(hit.point);
					hit_OUT.object = this;
					hit_OUT.normal = global_matrix->MulDir(hit_info.normal); //TODO: normal needs revision. May not work as expected.
					hit_OUT.normal.Normalize();
				}
			}
		}
	}

	return ret;
}

void GameObject::ApplyPrefabChanges()
{
	if (is_prefab)
	{
		if (rc_prefab)
		{
			rc_prefab->ApplyChanges(this);
		}
		else
		{
			GameObject* prefab_go = App->go_manager->FindGameObjectByUUID(App->go_manager->root, prefab_root_uuid);
			if (prefab_go)
				prefab_go->rc_prefab->ApplyChanges(prefab_go);
		}
	}
}

void GameObject::CollectChildrenUUID(vector<unsigned int>& uuid, vector<unsigned int>& local_uuid) const
{
	vector<GameObject*>::const_iterator it = childs.begin();
	for (it; it != childs.end(); ++it)
	{
		uuid.push_back((*it)->GetUUID());
		local_uuid.push_back((*it)->local_uuid);

		(*it)->CollectChildrenUUID(uuid, local_uuid);
	}
}

void GameObject::RevertPrefabChanges()
{
	if (is_prefab)
	{
		if (rc_prefab)
		{
			rc_prefab->RevertChanges(this);
		}
		else
		{
			GameObject* prefab_go = App->go_manager->FindGameObjectByUUID(App->go_manager->root, prefab_root_uuid);
			if (prefab_go)
				prefab_go->rc_prefab->RevertChanges(prefab_go);
		}
	}
}

void GameObject::UnlinkPrefab()
{
	if (is_prefab)
	{
		if (rc_prefab == nullptr)
		{
			GameObject* prefab_go = App->go_manager->FindGameObjectByUUID(App->go_manager->root, prefab_root_uuid);
			if (prefab_go && prefab_go->IsPrefab())
			{
				prefab_go->UnlinkPrefab();
				return;
			}
		}
		else
		{
			rc_prefab->Unload();
			rc_prefab->UnloadInstance(this);
			rc_prefab = nullptr;
		}

		is_prefab = false;
		prefab_path = "";
		prefab_root_uuid = 0;
		local_uuid = 0;
		for (std::vector<GameObject*>::iterator it = childs.begin(); it != childs.end(); it++)
		{
			(*it)->UnlinkPrefab();
		}
	}
}
