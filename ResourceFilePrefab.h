#ifndef __RESOURCEFILEPREFAB_H__
#define __RESOURCEFILEPREFAB_H__

#include "ResourceFile.h"
#include <list>
class GameObject;

class ResourceFilePrefab : public ResourceFile
{
public:
	ResourceFilePrefab(ResourceFileType type, const std::string& file_path, unsigned int uuid);
	~ResourceFilePrefab();

	void LoadPrefabAsCopy(); //Loads a new prefab instance loaded from the Assets(library actually) file
	GameObject* LoadPrefabFromScene(const Data& file, GameObject* parent)const; //Loads a prefab from a scene file
	void Save(); //Applies new changes

	void UnloadInstance(GameObject* instance);
	void ApplyChanges(GameObject* gameobject);

private:
	void LoadInMemory();
	void UnloadInMemory();

	void CreateChildsByUUID(const Data & go_data, map<unsigned int, unsigned int>& uuids, unsigned int uuid, list<GameObject*>& parents)const;

	void SaveNewChanges(GameObject* gameobject)const;
	void SaveChangesGameObject(Data& file, GameObject* gameobject, unsigned int prefab_root_uuid)const;

	void ResetInstance(GameObject* gameobject, vector<GameObject*>& new_gameobjects)const;

private:
	list<GameObject*> instances;
};

#endif // !__RESOURCEFILEPREFAB_H__
