#include "Application.h"
#include "Skybox.h"
#include "ComponentCamera.h"
#include "CubeMap.h"
#include "ComponentMesh.h"
#include "Globals.h"
#include "MeshImporter.h"
#include "Glew\include\glew.h"
#include <gl/GL.h>
#include <gl/GLU.h>
#include "ShaderComplier.h"
#include "GameObject.h"
#include "Brofiler/include/Brofiler.h"

Skybox::Skybox()
{
}

Skybox::~Skybox()
{
	if (cubemap)
	{
		//cubemap->Unload();
		delete cubemap;
	}

	if (sphere_mesh)
	{
		//MeshImporter::DeleteBuffers(sphere_mesh);
		delete sphere_mesh;
	}
}		

void Skybox::Init(const string & posx_filename, const string & negx_filename, const string & posy_filename, const string & negy_filename, const string & posz_filename, const string & negz_filename)
{
	cubemap = new CubeMap(posx_filename, negx_filename, posy_filename, negy_filename, posz_filename, negz_filename);
	bool cubemap_loaded = cubemap->Load();
	if(cubemap_loaded == false)
		LOG("Error while loading the cubemap for the skybox");
	
	sphere_mesh = MeshImporter::Load(SKYBOX_MESH_PATH);
	if (sphere_mesh == nullptr)
		LOG("Error while loading the sphere mesh for the skybox");

	int vertex_id = ShaderCompiler::CompileVertex(SKYBOX_VERTEX_PROGRAM);
	int fragment_id = ShaderCompiler::CompileFragment(SKYBOX_FRAGMENT_PROGRAM);
	shader_id = ShaderCompiler::CompileShader(vertex_id, fragment_id);
}

void Skybox::Render(ComponentCamera* camera)
{
	BROFILER_CATEGORY("Skybox::Render", Profiler::Color::WhiteSmoke);

	glUseProgram(shader_id);

	GLint old_cull_face_mode;
	glGetIntegerv(GL_CULL_FACE_MODE, &old_cull_face_mode);
	GLint old_depth_func_mode;
	glGetIntegerv(GL_DEPTH_FUNC, &old_depth_func_mode);

	glCullFace(GL_FRONT);
	glDepthFunc(GL_LEQUAL);

	if (camera)
	{
		//TODO: been changed
		math::float3 translate = camera->GetWorldMatrix().TranslatePart();
		math::float4x4 model_matrix = math::float4x4::identity;
		model_matrix.SetTranslatePart(translate);
		//model_matrix.Scale(200, 200, 200);
		math::float4x4 wvp_matrix = model_matrix.Transposed() * camera->GetViewMatrix() * camera->GetProjectionMatrix();

		GLint wvp_location = glGetUniformLocation(shader_id, "_WVP");
		glUniformMatrix4fv(wvp_location, 1, GL_FALSE, *wvp_matrix.v);
		cubemap->Bind(GL_TEXTURE0);
		
		//Buffer vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, sphere_mesh->id_vertices);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

		//Index buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphere_mesh->id_indices);
		glDrawElements(GL_TRIANGLES, sphere_mesh->num_indices, GL_UNSIGNED_INT, (void*)0);

	}

	glCullFace(old_cull_face_mode);
	glDepthFunc(old_depth_func_mode);
	glUseProgram(0);
	glDisableVertexAttribArray(0);
	
}
