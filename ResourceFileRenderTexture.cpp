#include "Application.h"

#include "ModuleFileSystem.h"
#include "ModuleEditor.h"

#include "ResourceFileRenderTexture.h"
#include "OpenGLFunc.h"

ResourceFileRenderTexture::ResourceFileRenderTexture(ResourceFileType type, const std::string& file_path, unsigned int uuid) : ResourceFile(type, file_path, uuid)
{}

ResourceFileRenderTexture::~ResourceFileRenderTexture()
{}

void ResourceFileRenderTexture::Bind()
{
	OpenGLFunc::Bind(frame_buffer);
}

void ResourceFileRenderTexture::Unbind()
{
	OpenGLFunc::Unbind();
}

int ResourceFileRenderTexture::GetTexture() const
{
	return texture_buffer;
}

void ResourceFileRenderTexture::LoadInMemory()
{
	char* buffer;
	int size = App->file_system->Load(file_path.data(), &buffer);

	if (size > 0)
	{
		Data data(buffer);
		width = data.GetInt("width");
		height = data.GetInt("height");
		use_depth_as_texture = data.GetBool("use_depth_as_texture");

		frame_buffer = OpenGLFunc::CreateFBO(width, height, texture_buffer, depth_buffer);
	}
	else
	{
		LOG("[ERROR] Could not load resource %s", file_path.data());
		App->editor->DisplayWarning(WarningType::W_ERROR, "Could not load resource %s", file_path.data());
	}

	if(buffer)
		delete[] buffer;
}

void ResourceFileRenderTexture::UnloadInMemory()
{
	/*OpenGLFunc::DeleteFrameBuffer(frame_buffer);
	OpenGLFunc::DeleteTexture(texture_buffer);
	if (use_depth_as_texture)
		OpenGLFunc::DeleteTexture(depth_buffer);
	else
		OpenGLFunc::DeleteRenderBuffer(depth_buffer);*/
}
