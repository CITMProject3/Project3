#include "Application.h"
#include "ModuleRenderer3D.h"
#include "ResourceFileTexture.h"
#include "TextureImporter.h"
#include "ModuleResourceManager.h"

ResourceFileTexture::ResourceFileTexture(ResourceFileType type, const std::string& file_path, unsigned int uuid) : ResourceFile(type, file_path, uuid)
{
}

ResourceFileTexture::~ResourceFileTexture()
{
}

unsigned int ResourceFileTexture::GetTexture()
{
	return texture_id;
}

void ResourceFileTexture::SetProperties(unsigned int texture_id, unsigned int width, unsigned int height, unsigned int depth, unsigned int mips, unsigned int bytes, unsigned char* data)
{
	this->texture_id = texture_id;
	this->width = width;
	this->height = height;
	this->depth = depth;
	this->mips = mips;
	this->bytes = bytes;
	this->data = data;
}

unsigned int ResourceFileTexture::GetWidth() const
{
	return width;
}

unsigned int ResourceFileTexture::GetHeight() const
{
	return height;
}

unsigned char * ResourceFileTexture::GetData() const
{
	return data;
}

void ResourceFileTexture::LoadInMemory()
{
	TextureImporter::Load(this);
}

void ResourceFileTexture::UnloadInMemory()
{
	//TextureImporter::Unload(texture_id);
	App->renderer3D->RemoveTextureBuffer(texture_id);
	//App->resource_manager->RemoveResourceFromList(this);
}
