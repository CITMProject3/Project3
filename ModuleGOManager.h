#ifndef __MODULE_GO_MANAGER_H__
#define __MODULE_GO_MANAGER_H__

#include "Module.h"
#include "Octree.h"
#include "Primitive.h"

#include <vector>
#include <map>

class GameObject;
class Component;
class ComponentCamera;
class ComponentLight;
class ComponentCanvas;

class LayerSystem;
class RaycastHit;

enum LightType;
enum PLAYER;
enum ComponentType;

#define OCTREE_SIZE 800
#define TEMPORAL_SCENE "Library/temporal_scene.json"

enum PrimitiveType
{
	P_CUBE,
	P_SPHERE,
	P_PLANE,
	P_CYLINDER
};

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

	// Factory methods
	GameObject* CreateGameObject(GameObject* parent);
	GameObject* CreateLight(GameObject* parent, LightType type);
	GameObject* CreatePrimitive(PrimitiveType type);

	bool RemoveGameObject(GameObject* object);
	bool FastRemoveGameObject(GameObject* object); // Doesn't remove the GameObject from the parent list.
	void DuplicateGameObject(GameObject* object);

	void GetAllComponents(std::vector<Component*> &list, ComponentType type, GameObject *from = nullptr) const;
	ComponentLight* GetDirectionalLight(GameObject* from = nullptr)const;

	void LoadEmptyScene();
	GameObject* LoadPrefabGameObject(const Data& go_data, map<unsigned int, unsigned int>& uuids); //Used to load prefabs and mesh files

	bool IsRoot(const GameObject* go)const;

	void SaveSceneBeforeRunning();//Saves the scene before running the game
	void LoadSceneBeforeRunning();
	void DeleteTemporalScene();
	void ClearScene(); //Removes the current scene
	GameObject* LoadGameObject(const Data& go_data);

	void SetCurrentAssetsScenePath(const char* scene_path);
	const char* GetCurrentAssetsScenePath() const;
	void SetCurrentLibraryScenePath(const char* scene_path);
	const char* GetCurrentLibraryScenePath() const;

	//Handles the insertion / remove of the octree and dynamic gameobjects list. TODO: Rename the methods. Look confusing.
	bool InsertGameObjectInOctree(GameObject* go);
	bool RemoveGameObjectOfOctree(GameObject* go);

	GameObject* FindGameObjectByUUID(GameObject* start, unsigned int uuid)const;
	void LinkGameObjectPointer(GameObject **pointer_to_pointer_go, unsigned int uuid_to_assign);

	RaycastHit Raycast(const Ray& ray, std::vector<int> layersToCheck = std::vector<int>(), bool keepDrawing = false);


	void LinkAnimation(GameObject* root)const; //Searches all go and links the meshes with the animation bones if is not done yet.

	AABB GetWorldAABB(std::vector<int> layersToCheck = std::vector<int>());
private:
	std::vector<float3> GetWorldAABB(std::vector<int> layersToCheck, GameObject* go);

private:

	void UpdateGameObjects(GameObject* obj);
	void PreUpdateGameObjects(GameObject* obj);

	void OnPlay();
	void OnPlayGameObjects(GameObject* obj);

	void OnPause();
	void OnPauseGameObjects(GameObject* obj);

	void OnStop();
	void OnStopGameObjects(GameObject* obj);

private:

	vector<GameObject*> go_to_remove;

	string current_assets_scene_path = "";
	string current_library_scene_path = "";

	bool debug_inspector = false;
public: 
	LayerSystem* layer_system = nullptr;

	//GameObjects TODO:Add functionallity to make it private
	Octree<GameObject*> octree; //Static
	list<GameObject*> dynamic_gameobjects;
	bool draw_octree = false;
	GameObject* root = nullptr;
	ComponentCanvas* current_scene_canvas = nullptr;
	float3 lastRayData[3];

	//Sorry to put this here... we will need
	//a place to store global variables through scenes
	//To change for the next sprint
	uint team1_car = 1;
	uint team2_car = 1;

	uint team1_p1_c = 1;
	uint team1_p2_c = 3;

	uint team2_p1_c = 1;
	uint team2_p2_c = 3;
	uint current_scene = 1; // 0 - ROGER; 1 - ASIER

	PLAYER team1_front;
	PLAYER team1_back;
	PLAYER team2_front;
	PLAYER team2_back;
};

#endif // !__MODULE_GO_MANAGER_H__

