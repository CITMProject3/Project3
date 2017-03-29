#ifndef __RESOURCEFILE_H__
#define __RESOURCEFILE_H__

#include <string>

enum ResourceFileType
{
	RES_MESH,
	RES_TEXTURE,
	RES_MATERIAL,
	RES_RENDER_TEX,
	RES_ANIMATION,
	RES_BONE,
	RES_SOUNDBANK,
	RES_PREFAB,
	RES_SCRIPTS_LIBRARY,
};

class ResourceFile
{
public:
	ResourceFile(ResourceFileType type, const std::string& file_path, unsigned int uuid);
	~ResourceFile();

	const char* GetFile()const;
	unsigned int GetUUID()const;
	ResourceFileType GetType()const;
	unsigned int GetBytes()const;
	void Load();
	void Unload();
	void UnLoadAll();
	void Reload();

private:
	virtual void LoadInMemory();
	virtual void UnloadInMemory();

protected:
	unsigned int used = 0;
	ResourceFileType type;
	std::string file_path;
	unsigned int uuid = 0;
	unsigned int bytes = 0;
};

#endif // !__RESOURCEFILE_H__
