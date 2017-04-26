#ifndef __RESOURCEFILEPREFAB_H__
#define __RESOURCEFILEPREFAB_H__

#include "ResourceFile.h"
#include "Data.h"
#include <list>
#include <map>

class GameObject;

class ResourceFilePrefab : public ResourceFile
{
public:
	ResourceFilePrefab(ResourceFileType type, const std::string& file_path, unsigned int uuid);
	~ResourceFilePrefab();

	GameObject* LoadPrefabAsCopy(); //Loads a new prefab instance loaded from the Assets(library actually) file
	GameObject* LoadPrefabFromScene(const Data& file, GameObject* parent); //Loads a prefab from a scene file

	void UnloadInstance(GameObject* instance);
	void ApplyChanges(GameObject* gameobject);
	void RevertChanges(GameObject* gameobject);

	void InsertOriginalInstance(GameObject* original_go);

private:
	void LoadInMemory();
	void UnloadInMemory();

	void CreateChildsByUUID(const Data & go_data, std::map<unsigned int, unsigned int>& uuids, unsigned int uuid, list<GameObject*>& parents)const;

	void SaveNewChanges(GameObject* gameobject)const;
	void SaveChangesGameObject(Data& file, GameObject* gameobject, unsigned int prefab_root_uuid)const;

	void ResetInstance(GameObject* gameobject, vector<GameObject*>& new_gameobjects);

private:
	list<GameObject*> instances;
};

#endif // !__RESOURCEFILEPREFAB_H__
