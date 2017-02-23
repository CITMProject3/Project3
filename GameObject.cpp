#include "Application.h"
#include "GameObject.h"
#include "Component.h"
#include "ComponentTransform.h"
#include "ComponentMesh.h"
#include "ComponentMaterial.h"
#include "ComponentCamera.h"
#include "MeshImporter.h"
#include "RaycastHit.h"
#include "ComponentLight.h"
#include "ResourceFilePrefab.h"

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
	std::vector<Component*>::iterator comp = components.begin();

	for (comp; comp != components.end(); comp++)
	{
		(*comp)->Update();
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
	for (int i = 0; i < childs.size(); i++)
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
	this->parent = parent;
}

const std::vector<GameObject*>* GameObject::GetChilds()
{
	return &childs;
}

size_t GameObject::ChildCount()
{
	return childs.size();
}

bool GameObject::IsActive() const
{
	return active;
}

void GameObject::SetActive(bool value)
{
	if (value == true && parent->IsActive() == false)
		return;

	if ((value == true && active == false) || (value == false && active == true))
	{
		active = value;

		for (std::vector<GameObject*>::iterator child = childs.begin(); child != childs.end(); ++child)
			(*child)->SetActive(value);
	}
}

bool GameObject::IsStatic() const
{
	return is_static;
}

void GameObject::SetStatic(bool value)
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

Component* GameObject::AddComponent(ComponentType type)
{
	Component* item = nullptr;

	switch (type)
	{
	case C_TRANSFORM:
		if(GetComponent(type) == nullptr) //Only one transform compoenent for gameobject
			item = new ComponentTransform(type, this, &global_matrix);
		break;
	case C_MESH:
		if(GetComponent(C_TRANSFORM))
			item = new ComponentMesh(type, this);
		break;
	case C_MATERIAL:
		if (GetComponent(C_TRANSFORM) && GetComponent(C_MESH))
			item = new ComponentMaterial(type, this);
		break;
	case C_CAMERA:
		if (GetComponent(C_TRANSFORM))
			item = new ComponentCamera(type, this);
		break;
	case C_LIGHT:	
		if (GetComponent(C_TRANSFORM))
			item = new ComponentLight(type, this);
		break;
	default:
		break;
	}

	if (item != nullptr)
	{
		components.push_back(item);
	}
	else
	{
		LOG("Error while adding component to %s", this->name);
	}
		

	return item;
}

const std::vector<Component*>* GameObject::GetComponents()
{
	return &components;
}

void* GameObject::GetComponent(ComponentType type)const
{
	std::vector<Component*>::const_iterator comp = components.begin();

	while (comp != components.end())
	{
		if ((*comp)->GetType() == type)
		{
			return (*comp);
		}
		++comp;
	}

	return NULL;
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

bool GameObject::RayCast(const Ray & ray, RaycastHit & hit)
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
			Ray raycast = ray;
			raycast.Transform(global_matrix->Inverted());

			uint u1, u2, u3;
			float3 v1, v2, v3;
			float distance;
			vec hit_point;
			Triangle triangle;
			for (int i = 0; i < mesh->num_indices / 3; i++)
			{
				u1 = mesh->indices[i * 3];
				u2 = mesh->indices[i * 3 + 1];
				u3 = mesh->indices[i * 3 + 2];
				v1 = float3(&mesh->vertices[u1]);
				v2 = float3(&mesh->vertices[u2]);
				v3 = float3(&mesh->vertices[u3]);
				triangle = Triangle(v1, v2, v3);
				if (raycast.Intersects(triangle, &distance, &hit_point))
				{
					ret = true;
					if (hit_info.distance == 0) //First entry
					{
						hit_info.distance = distance;
						hit_info.point = hit_point;
						hit_info.normal = triangle.PlaneCCW().normal;
						
					}
					else
					{
						if (hit_info.distance > distance)
						{
							hit_info.distance = distance;
							hit_info.point = hit_point;
							hit_info.normal = triangle.PlaneCCW().normal;
						}
					}
				}

				if (ret == true)
				{
					//Transfrom the hit parameters to global coordinates
					hit.point = global_matrix->MulPos(hit_info.point);
					hit.distance = ray.pos.Distance(hit.point);
					hit.object = this;
					hit.normal = global_matrix->MulDir(hit_info.normal); //TODO: normal needs revision. May not work as expected.
					hit.normal.Normalize();
				}
			}
		}
	}

	return ret;
}

void GameObject::ApplyPrefabChanges()
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
