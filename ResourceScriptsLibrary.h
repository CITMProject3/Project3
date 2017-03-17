#ifndef __RESOURCESCRIPTSLIBRARY_H__
#define __RESOURCESCRIPTSLIBRARY_H__

#include "ResourceFile.h"
#include "Globals.h"

class ResourceScriptsLibrary : public ResourceFile
{
public:
	ResourceScriptsLibrary(ResourceFileType type, const std::string& file_path, unsigned int uuid);
	~ResourceScriptsLibrary();


	HINSTANCE lib;
private:

	void LoadInMemory();
	void UnloadInMemory();

};

#endif // !__RESOURCESCRIPTSLIBRARY_H__