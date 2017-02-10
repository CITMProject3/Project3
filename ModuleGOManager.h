#ifndef __MODULE_GO_MANAGER_H__
#define __MODULE_GO_MANAGER_H__

#include "Module.h"
#include <vector>
#include <list>
#include <string>
#include <map>
#include "Octree.h"

class GameObject;
class ComponentCamera;
class ComponentLight;
class LayerSystem;

#define OCTREE_SIZE 800

class ModuleGOManager : public Module
{
public:
	ModuleGOManager(const char* name, bool start_enabled = true);
	~ModuleGOManager();

	bool Init(Data& config);
	bool Start(); 
	update_status PreUpdate();
	update_status Update();
	void SaveBeforeClosing(Data& data)const;

	GameObject* CreateGameObject(GameObject* parent);
	bool RemoveGameObject(GameObject* object);
	bool FastRemoveGameObject(GameObject* object); //Doesn't remove the GameObject from the parent list.

	void GetAllCameras(std::vector<ComponentCamera*>& list, GameObject* from = nullptr) const;
	ComponentLight* GetDirectionalLight(GameObject* from = nullptr)const;

	void LoadEmptyScene();
	void LoadPrefabGameObject(const Data& go_data, map<unsigned int, unsigned int>& uuids);

	bool IsRoot(const GameObject* go)const;

	void PickObjects();

	void SaveSceneBeforeRunning();//Saves the scene before running the game
	void LoadSceneBeforeRunning();
	void ClearScene(); //Removes the current scene
	GameObject* LoadGameObject(const Data& go_data);
	void SetCurrentScenePath(const char* scene_path);

	//Handles the insertion / remove of the octree and dynamic gameobjects list. TODO: Rename the methods. Look confusing.
	bool InsertGameObjectInOctree(GameObject* go);
	bool RemoveGameObjectOfOctree(GameObject* go);

private:

	void HierarchyWindow();
	void DisplayGameObjectsChilds(const std::vector<GameObject*>* childs);

	void InspectorWindow();

	void UpdateGameObjects(float dt, GameObject* obj);
	void PreUpdateGameObjects(GameObject* obj);

	GameObject* FindGameObjectByUUID(GameObject* start, unsigned int uuid)const; //Should be a public method?

	GameObject* Raycast(const Ray& ray)const;

private:
	GameObject* selected_GO = nullptr;
	vector<GameObject*> go_to_remove;
	string current_scene_path = "";

	bool debug_inspector = false;
public: 
	LayerSystem* layer_system = nullptr;

	//GameObjects TODO:Add functionallity to make it private
	Octree<GameObject*> octree; //Static
	list<GameObject*> dynamic_gameobjects;
	bool draw_octree = false;
	GameObject* root = nullptr;

};

#endif // !__MODULE_GO_MANAGER_H__

