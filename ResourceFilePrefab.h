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
	void Save(); //Applies new changes

private:
	void LoadInMemory();
	void UnloadInMemory();

private:
	list<GameObject*> instances;
};

#endif // !__RESOURCEFILEPREFAB_H__
