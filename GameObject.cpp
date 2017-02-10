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
}

GameObject::GameObject(const char* name, unsigned int uuid, GameObject* parent, bool active, bool is_static, bool is_prefab, int layer) : name(name), uuid(uuid), parent(parent), active(active), is_static(is_static), is_prefab(is_prefab), layer(layer)
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

void GameObject::SetAsPrefab()
{
	is_prefab = true;
	local_uuid = uuid;
	for (vector<GameObject*>::iterator child = childs.begin(); child != childs.end(); ++child)
		(*child)->SetAsPrefab();
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

void GameObject::Save(Data & file) const
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
	data.AppendBool("static", is_static);
	data.AppendBool("is_prefab", is_prefab);
	data.AppendInt("layer", layer);
	data.AppendArray("components");

	//Components data
	vector<Component*>::const_iterator component = components.begin();
	for (component; component != components.end(); component++)
	{
		(*component)->Save(data);
	}

	file.AppendArrayValue(data);

	for (vector<GameObject*>::const_iterator child = childs.begin(); child != childs.end(); ++child)
		(*child)->Save(file);
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
