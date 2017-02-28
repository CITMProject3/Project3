#ifndef __RESOURCEFILEPREFAB_H__
#define __RESOURCEFILEPREFAB_H__

#include "ResourceFile.h"

class GameObject;

class ResourceFilePrefab : public ResourceFile
{
public:
	ResourceFilePrefab(ResourceFileType type, const std::string& file_path, unsigned int uuid);
	~ResourceFilePrefab();

	void LoadPrefab(GameObject* root); //Actual load
	void Save(); //Applies new changes

private:
	void LoadInMemory();
	void UnloadInMemory();
};

#endif // !__RESOURCEFILEPREFAB_H__
