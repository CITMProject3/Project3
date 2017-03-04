#ifndef __RESOURCEFILEMATERIAL_H__
#define __RESOURCEFILEMATERIAL_H__

#include "ResourceFile.h"
#include "ResourceFileMaterial.h"
#include "Material.h"

class ResourceFileMaterial : public ResourceFile
{
public:
	ResourceFileMaterial(ResourceFileType type, const std::string& file_path, unsigned int uuid);
	~ResourceFileMaterial();

	uint GetShaderId();

private:

	void LoadInMemory();
	void UnloadInMemory();

public:
	Material material;
private:
	uint shader_id = 0;
	uint vertex_id = 0;
	uint fragment_id = 0;

};

#endif // !__RESOURCEFILEMATERIAL_H__