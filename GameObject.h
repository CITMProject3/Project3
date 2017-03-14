#ifndef __GAMEOBJECT_H__
#define __GAMEOBJECT_H__

#include <vector>
#include <string>
#include "MathGeoLib\include\MathGeoLib.h"

class Component;
enum ComponentType;
struct Mesh;
class RaycastHit;
class Data;
class ResourceFilePrefab;

class GameObject
{
public:
	GameObject();
	GameObject(GameObject* parent);
	GameObject(const char* name, unsigned int uuid, GameObject* parent, bool active, bool is_static, bool is_prefab, int layer, unsigned int prefab_root_uuid,const std::string& prefab_path);
	~GameObject();

	void PreUpdate();
	void Update(float dt);

	bool AddChild(GameObject* child);
	bool RemoveChild(GameObject* child); //Breaks the link with the parent but does not delete the child.
	void RemoveAllChilds();
	GameObject* GetParent()const;
	void SetParent(GameObject* parent);
	const std::vector<GameObject*>* GetChilds();
	size_t ChildCount();
	void CollectAllChilds(std::vector<GameObject*>& vector);

	void OnPlay();
	void OnStop();
	void OnPause();

	bool IsActive()const;
	void SetActive(bool value);
	bool IsStatic()const;
	void SetStatic(bool value);
	void ForceStatic(bool value);
	void SetAsPrefab(unsigned int root_uuid);
	bool IsPrefab()const;

	Component* AddComponent(ComponentType type);
	const std::vector<Component*>* GetComponents();
	void* GetComponent(ComponentType type)const;
	void RemoveComponent(Component* component);

	float4x4 GetGlobalMatrix()const;
	unsigned int GetUUID()const;

	void TransformModified();

	void Save(Data& file, bool ignore_prefab = false) const;
	void SaveAsChildPrefab(Data& file)const; //Only saves the UUID

	bool RayCast(Ray ray, RaycastHit& hit); //Raycast testing ONLY against geometry. 

	void ApplyPrefabChanges();
	void CollectChildrenUUID(std::vector<unsigned int>& uuid, std::vector<unsigned int>& local_uuid)const;
	void RevertPrefabChanges();

public:
	std::string name;
	//Filled the moment to draw. Do not use it elsewhere.
	Mesh* mesh_to_draw = nullptr; //Pointer to the mesh to draw in one frame

	AABB* bounding_box = nullptr; //Only mesh component can Set this.
	//UUID of the equal GameObject inside the prefab. It's 0 if the GameObject is not a prefab
	unsigned int local_uuid = 0;
	int layer = 0;

	std::string prefab_path = "";
	ResourceFilePrefab* rc_prefab = nullptr;
	unsigned int prefab_root_uuid = 0;
private:
	GameObject* parent = NULL;
	std::vector<GameObject*> childs;

	bool active = true;
	bool is_static = false;
	std::vector<Component*> components;
	std::vector<Component*> components_to_remove;

	float4x4* global_matrix = nullptr;
	bool is_prefab = false;
	unsigned int uuid = 0;
};

#endif // !__GAMEOBJECT_H__
