#ifndef __RESOURCEFILE_RENDERTEXTURE_H__
#define __RESOURCEFILE_RENDERTEXTURE_H__

#include "ResourceFile.h"

class ResourceFileRenderTexture : public ResourceFile
{
public:
	ResourceFileRenderTexture(ResourceFileType type, const std::string& file_path, unsigned int uuid);
	~ResourceFileRenderTexture();

	void Bind();
	void Unbind();

	int GetTexture()const;

private:
	void LoadInMemory();
	void UnloadInMemory();

private:

	int width = 0;
	int height = 0;
	bool use_depth_as_texture = false;

	unsigned int frame_buffer = 0;
	unsigned int texture_buffer = 0;
	unsigned int depth_buffer = 0;


};



#endif // !__RESOURCEFILE_RENDERTEXTURE_H__
