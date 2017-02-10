#ifndef __RESOURCEFILEMESH_H__
#define __RESOURCEFILEMESH_H__

#include "ResourceFile.h"
#include <string>

struct Mesh;

class ResourceFileMesh : public ResourceFile
{
public:
	ResourceFileMesh(ResourceFileType type, const std::string& file_path, unsigned int uuid);
	~ResourceFileMesh();

	Mesh* GetMesh()const;

private:
	void LoadInMemory();
	void UnloadInMemory();

private:
	Mesh* mesh = nullptr;
};

#endif // !__RESOURCEFILEMESH_H__
