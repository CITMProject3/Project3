#include "Application.h"
#include "ShaderComplier.h"
#include "ResourceFileMaterial.h"
#include "ShaderComplier.h"
#include "Material.h"

ResourceFileMaterial::ResourceFileMaterial(ResourceFileType type, const std::string& file_path, unsigned int uuid) : ResourceFile(type, file_path, uuid)
{}

ResourceFileMaterial::~ResourceFileMaterial()
{}

uint ResourceFileMaterial::GetShaderId()
{
	return shader_id;
}


void ResourceFileMaterial::LoadInMemory()
{
	material.Load(file_path.data());
	vertex_id = ShaderCompiler::CompileVertex(material.vertex_path.data());
	fragment_id = ShaderCompiler::CompileFragment(material.fragment_path.data());
	shader_id = ShaderCompiler::CompileShader(vertex_id, fragment_id);
}

void ResourceFileMaterial::UnloadInMemory()
{
	ShaderCompiler::DeleteShader(vertex_id);
	ShaderCompiler::DeleteShader(fragment_id);
	ShaderCompiler::DeleteShader(shader_id);
}
