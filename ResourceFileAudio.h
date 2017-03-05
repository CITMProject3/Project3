#ifndef __RESOURCEFILEAUDIO_H__
#define __RESOURCEFILEAUDIO_H__

#include "ResourceFile.h"

class ResourceFileAudio : public ResourceFile
{
public:
	ResourceFileAudio(ResourceFileType type, const std::string& file_path, unsigned int uuid);
	~ResourceFileAudio();

private:

	void LoadInMemory();
	void UnloadInMemory();

};

#endif // !__RESOURCEFILEAUDIO_H__

