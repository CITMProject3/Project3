#include "MasterRender.h"

#include "Globals.h"
#include <map>

#include "Glew\include\glew.h"
#include <gl/GL.h>
#include <gl/GLU.h>

#include "ShaderComplier.h"

#include "GameObject.h"
#include "ComponentCamera.h"
#include "ComponentMaterial.h"
#include "ComponentMesh.h"

#include "Application.h"
#include "ModuleRenderer3D.h"
#include "ShadowMap.h"

#include "Light.h"


MasterRender::MasterRender()
{
}

MasterRender::~MasterRender()
{
}

void MasterRender::Init()
{
	InitDefaultShader();
	InitDefaultNormalShader();
	InitAnimShader();
	InitAnimNormalShader();
	InitTerrainShader();
	InitShadowShader();
	InitParticleShader();
}

void MasterRender::RenderDefaultShader(const GameObject* obj, const ComponentCamera* cam, const ComponentMaterial* material, const LightInfo* light) const
{
	glUseProgram(df_shader.id);

	glUniformMatrix4fv(df_shader.model, 1, GL_FALSE, *(obj->GetGlobalMatrix().Transposed()).v);
	glUniformMatrix4fv(df_shader.projection, 1, GL_FALSE, *cam->GetProjectionMatrix().v);
	glUniformMatrix4fv(df_shader.view, 1, GL_FALSE, *cam->GetViewMatrix().v);

	glUniformMatrix4fv(df_shader.shadow_view, 1, GL_FALSE, *App->renderer3D->shadow_map->GetShadowView().v);
	glUniformMatrix4fv(df_shader.shadow_projection, 1, GL_FALSE, *App->renderer3D->shadow_map->GetShadowProjection().v);

	std::map<std::string, uint>::const_iterator tex = material->texture_ids.find("0");
	if (tex->second != 0)
	{
		glUniform1i(df_shader.has_texture, 1);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, (*tex).second);
		glUniform1i(df_shader.texture, 0);
	}
	else
	{
		glUniform1i(df_shader.has_texture, 0);
		glUniform1i(df_shader.texture, 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, App->renderer3D->shadow_map->GetShadowMapId());
	glUniform1i(df_shader.shadowmap, 1);
	
	glUniform1f(df_shader.Ia, light->ambient_intensity);
	glUniform3f(df_shader.Ka, light->ambient_color.x, light->ambient_color.y, light->ambient_color.z);
	glUniform1f(df_shader.Id, light->directional_intensity);
	glUniform3f(df_shader.Kd, light->directional_color.x, light->directional_color.y, light->directional_color.z);
	glUniform3f(df_shader.L, light->directional_direction.x, light->directional_direction.y, light->directional_direction.z);
	glUniform1f(df_shader.Is, material->specular);

	glUniform4fv(df_shader.material_color, 1, float4(material->color).ptr());	
}

void MasterRender::RenderNormalShader(const GameObject * obj, const ComponentCamera * cam, const ComponentMaterial * material, const LightInfo * light) const
{
	glUseProgram(normal_shader.id);

	glUniformMatrix4fv(normal_shader.model, 1, GL_FALSE, *(obj->GetGlobalMatrix().Transposed()).v);
	glUniformMatrix4fv(normal_shader.projection, 1, GL_FALSE, *cam->GetProjectionMatrix().v);
	glUniformMatrix4fv(normal_shader.view, 1, GL_FALSE, *cam->GetViewMatrix().v);

	glUniformMatrix4fv(normal_shader.shadow_view, 1, GL_FALSE, *App->renderer3D->shadow_map->GetShadowView().v);
	glUniformMatrix4fv(normal_shader.shadow_projection, 1, GL_FALSE, *App->renderer3D->shadow_map->GetShadowProjection().v);

	std::map<std::string, uint>::const_iterator tex = material->texture_ids.find("0");
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, (*tex).second);
	glUniform1i(normal_shader.texture, 0);

	tex = material->texture_ids.find("1");
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, (*tex).second);
	glUniform1i(normal_shader.normal, 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, App->renderer3D->shadow_map->GetShadowMapId());
	glUniform1i(normal_shader.shadowmap, 2);
	
	glUniform1f(normal_shader.Ia, light->ambient_intensity);
	glUniform3f(normal_shader.Ka, light->ambient_color.x, light->ambient_color.y, light->ambient_color.z);
	glUniform1f(normal_shader.Id, light->directional_intensity);
	glUniform3f(normal_shader.Kd, light->directional_color.x, light->directional_color.y, light->directional_color.z);
	glUniform3f(normal_shader.L, light->directional_direction.x, light->directional_direction.y, light->directional_direction.z);
	glUniform1f(normal_shader.Is, material->specular);

	glUniform4fv(normal_shader.material_color, 1, float4(material->color).ptr());
}

void MasterRender::RenderAnimShader(const GameObject * obj, const ComponentCamera * cam, const ComponentMaterial * material, const LightInfo * light) const
{
	glUseProgram(anim_shader.id);

	glUniformMatrix4fv(anim_shader.model, 1, GL_FALSE, *(obj->GetGlobalMatrix().Transposed()).v);
	glUniformMatrix4fv(anim_shader.projection, 1, GL_FALSE, *cam->GetProjectionMatrix().v);
	glUniformMatrix4fv(anim_shader.view, 1, GL_FALSE, *cam->GetViewMatrix().v);

	glUniformMatrix4fv(anim_shader.shadow_view, 1, GL_FALSE, *App->renderer3D->shadow_map->GetShadowView().v);
	glUniformMatrix4fv(anim_shader.shadow_projection, 1, GL_FALSE, *App->renderer3D->shadow_map->GetShadowProjection().v);

	std::map<std::string, uint>::const_iterator tex = material->texture_ids.find("0");
	if (tex->second != 0)
	{
		glUniform1i(anim_shader.has_texture, 1);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, (*tex).second);
		glUniform1i(anim_shader.texture, 0);
	}
	else
	{
		glUniform1i(anim_shader.has_texture, 0);
		glUniform1i(anim_shader.texture, 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, App->renderer3D->shadow_map->GetShadowMapId());
	glUniform1i(anim_shader.shadowmap, 1);

	glUniform1f(anim_shader.Ia, light->ambient_intensity);
	glUniform3f(anim_shader.Ka, light->ambient_color.x, light->ambient_color.y, light->ambient_color.z);
	glUniform1f(anim_shader.Id, light->directional_intensity);
	glUniform3f(anim_shader.Kd, light->directional_color.x, light->directional_color.y, light->directional_color.z);
	glUniform3f(anim_shader.L, light->directional_direction.x, light->directional_direction.y, light->directional_direction.z);
	glUniform1f(anim_shader.Is, material->specular);

	glUniform4fv(anim_shader.material_color, 1, float4(material->color).ptr());

	ComponentMesh* c_mesh = (ComponentMesh*)obj->GetComponent(C_MESH);
	glUniformMatrix4fv(anim_shader.bones, c_mesh->bones_trans.size(), GL_FALSE, reinterpret_cast<GLfloat*>(c_mesh->bones_trans.data()));
}

void MasterRender::RenderAnimNormalShader(const GameObject * obj, const ComponentCamera * cam, const ComponentMaterial * material, const LightInfo * light) const
{
	glUseProgram(anim_normal_shader.id);

	glUniformMatrix4fv(anim_normal_shader.model, 1, GL_FALSE, *(obj->GetGlobalMatrix().Transposed()).v);
	glUniformMatrix4fv(anim_normal_shader.projection, 1, GL_FALSE, *cam->GetProjectionMatrix().v);
	glUniformMatrix4fv(anim_normal_shader.view, 1, GL_FALSE, *cam->GetViewMatrix().v);

	glUniformMatrix4fv(anim_normal_shader.shadow_view, 1, GL_FALSE, *App->renderer3D->shadow_map->GetShadowView().v);
	glUniformMatrix4fv(anim_normal_shader.shadow_projection, 1, GL_FALSE, *App->renderer3D->shadow_map->GetShadowProjection().v);

	std::map<std::string, uint>::const_iterator tex = material->texture_ids.find("0");
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, (*tex).second);
	glUniform1i(anim_normal_shader.texture, 0);

	tex = material->texture_ids.find("1");
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, (*tex).second);
	glUniform1i(anim_normal_shader.normal, 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, App->renderer3D->shadow_map->GetShadowMapId());
	glUniform1i(anim_normal_shader.shadowmap, 2);

	glUniform1f(anim_normal_shader.Ia, light->ambient_intensity);
	glUniform3f(anim_normal_shader.Ka, light->ambient_color.x, light->ambient_color.y, light->ambient_color.z);
	glUniform1f(anim_normal_shader.Id, light->directional_intensity);
	glUniform3f(anim_normal_shader.Kd, light->directional_color.x, light->directional_color.y, light->directional_color.z);
	glUniform3f(anim_normal_shader.L, light->directional_direction.x, light->directional_direction.y, light->directional_direction.z);
	glUniform1f(anim_normal_shader.Is, material->specular);

	glUniform4fv(anim_normal_shader.material_color, 1, float4(material->color).ptr());

	ComponentMesh* c_mesh = (ComponentMesh*)obj->GetComponent(C_MESH);
	glUniformMatrix4fv(anim_normal_shader.bones, c_mesh->bones_trans.size(), GL_FALSE, reinterpret_cast<GLfloat*>(c_mesh->bones_trans.data()));
}

void MasterRender::InitDefaultShader()
{
	unsigned int def_shader_v = ShaderCompiler::CompileVertex("Resources/Shaders/defaultV.ver");
	unsigned int def_shader_f = ShaderCompiler::CompileFragment("Resources/Shaders/defaultF.fra");
	df_shader.id = ShaderCompiler::CompileShader(def_shader_v, def_shader_f);

	df_shader.model = glGetUniformLocation(df_shader.id, "model");
	df_shader.projection = glGetUniformLocation(df_shader.id, "projection");
	df_shader.view = glGetUniformLocation(df_shader.id, "view");

	df_shader.shadow_view = glGetUniformLocation(df_shader.id, "shadowView");
	df_shader.shadow_projection = glGetUniformLocation(df_shader.id, "shadowProjection");

	df_shader.has_texture = glGetUniformLocation(df_shader.id, "_HasTexture");
	df_shader.texture = glGetUniformLocation(df_shader.id, "_Texture");
	df_shader.shadowmap = glGetUniformLocation(df_shader.id, "shadowMap");

	df_shader.Ia = glGetUniformLocation(df_shader.id, "Ia");
	df_shader.Ka = glGetUniformLocation(df_shader.id, "Ka");
	df_shader.Id = glGetUniformLocation(df_shader.id, "Id");
	df_shader.Kd = glGetUniformLocation(df_shader.id, "Kd");
	df_shader.L = glGetUniformLocation(df_shader.id, "L");
	df_shader.Is = glGetUniformLocation(df_shader.id, "Is");

	df_shader.material_color = glGetUniformLocation(df_shader.id, "material_color");
}

void MasterRender::InitDefaultNormalShader()
{
	unsigned int def_shader_v = ShaderCompiler::CompileVertex("Resources/Shaders/defaultNormalV.ver");
	unsigned int def_shader_f = ShaderCompiler::CompileFragment("Resources/Shaders/defaultNormalF.fra");
	normal_shader.id = ShaderCompiler::CompileShader(def_shader_v, def_shader_f);

	normal_shader.model = glGetUniformLocation(normal_shader.id, "model");
	normal_shader.projection = glGetUniformLocation(normal_shader.id, "projection");
	normal_shader.view = glGetUniformLocation(normal_shader.id, "view");

	normal_shader.shadow_view = glGetUniformLocation(normal_shader.id, "shadowView");
	normal_shader.shadow_projection = glGetUniformLocation(normal_shader.id, "shadowProjection");
	normal_shader.shadowmap = glGetUniformLocation(normal_shader.id, "shadowMap");

	normal_shader.texture = glGetUniformLocation(normal_shader.id, "_Texture");
	normal_shader.normal = glGetUniformLocation(normal_shader.id, "_NormalMap");

	normal_shader.Ia = glGetUniformLocation(normal_shader.id, "Ia");
	normal_shader.Ka = glGetUniformLocation(normal_shader.id, "Ka");
	normal_shader.Id = glGetUniformLocation(normal_shader.id, "Id");
	normal_shader.Kd = glGetUniformLocation(normal_shader.id, "Kd");
	normal_shader.L = glGetUniformLocation(normal_shader.id, "L");
	normal_shader.Is = glGetUniformLocation(normal_shader.id, "Is");

	normal_shader.material_color = glGetUniformLocation(normal_shader.id, "material_color");
}

void MasterRender::InitAnimShader()
{
	unsigned int def_shader_v = ShaderCompiler::CompileVertex("Resources/Shaders/animV.ver");
	unsigned int def_shader_f = ShaderCompiler::CompileFragment("Resources/Shaders/animF.fra");
	anim_shader.id = ShaderCompiler::CompileShader(def_shader_v, def_shader_f);

	anim_shader.model = glGetUniformLocation(anim_shader.id, "model");
	anim_shader.projection = glGetUniformLocation(anim_shader.id, "projection");
	anim_shader.view = glGetUniformLocation(anim_shader.id, "view");

	anim_shader.shadow_view = glGetUniformLocation(anim_shader.id, "shadowView");
	anim_shader.shadow_projection = glGetUniformLocation(anim_shader.id, "shadowProjection");

	anim_shader.shadowmap = glGetUniformLocation(anim_shader.id, "shadowMap");

	anim_shader.has_texture = glGetUniformLocation(anim_shader.id, "_HasTexture");
	anim_shader.texture = glGetUniformLocation(anim_shader.id, "_Texture");

	anim_shader.Ia = glGetUniformLocation(anim_shader.id, "Ia");
	anim_shader.Ka = glGetUniformLocation(anim_shader.id, "Ka");
	anim_shader.Id = glGetUniformLocation(anim_shader.id, "Id");
	anim_shader.Kd = glGetUniformLocation(anim_shader.id, "Kd");
	anim_shader.L = glGetUniformLocation(anim_shader.id, "L");
	anim_shader.Is = glGetUniformLocation(anim_shader.id, "Is");

	anim_shader.material_color = glGetUniformLocation(anim_shader.id, "material_color");
	anim_shader.bones = glGetUniformLocation(anim_shader.id, "bones");
}

void MasterRender::InitAnimNormalShader()
{
	unsigned int def_shader_v = ShaderCompiler::CompileVertex("Resources/Shaders/animNormalV.ver");
	unsigned int def_shader_f = ShaderCompiler::CompileFragment("Resources/Shaders/animNormalF.fra");
	anim_normal_shader.id = ShaderCompiler::CompileShader(def_shader_v, def_shader_f);

	anim_normal_shader.model = glGetUniformLocation(anim_normal_shader.id, "model");
	anim_normal_shader.projection = glGetUniformLocation(anim_normal_shader.id, "projection");
	anim_normal_shader.view = glGetUniformLocation(anim_normal_shader.id, "view");

	anim_normal_shader.shadow_view = glGetUniformLocation(anim_normal_shader.id, "shadowView");
	anim_normal_shader.shadow_projection = glGetUniformLocation(anim_normal_shader.id, "shadowProjection");

	anim_normal_shader.shadowmap = glGetUniformLocation(anim_normal_shader.id, "shadowMap");

	anim_normal_shader.texture = glGetUniformLocation(anim_normal_shader.id, "_Texture");
	anim_normal_shader.normal = glGetUniformLocation(anim_normal_shader.id, "_NormalMap");

	anim_normal_shader.Ia = glGetUniformLocation(anim_normal_shader.id, "Ia");
	anim_normal_shader.Ka = glGetUniformLocation(anim_normal_shader.id, "Ka");
	anim_normal_shader.Id = glGetUniformLocation(anim_normal_shader.id, "Id");
	anim_normal_shader.Kd = glGetUniformLocation(anim_normal_shader.id, "Kd");
	anim_normal_shader.L = glGetUniformLocation(anim_normal_shader.id, "L");
	anim_normal_shader.Is = glGetUniformLocation(anim_normal_shader.id, "Is");

	anim_normal_shader.material_color = glGetUniformLocation(anim_normal_shader.id, "material_color");
	anim_normal_shader.bones = glGetUniformLocation(anim_normal_shader.id, "bones");
}

void MasterRender::InitTerrainShader()
{
	unsigned int def_shader_v = ShaderCompiler::CompileVertex("Resources/Shaders/terrainV.ver");
	unsigned int def_shader_f = ShaderCompiler::CompileFragment("Resources/Shaders/terrainF.fra");
	terrain_shader.id = ShaderCompiler::CompileShader(def_shader_v, def_shader_f);

	terrain_shader.model = glGetUniformLocation(terrain_shader.id, "model");
	terrain_shader.projection = glGetUniformLocation(terrain_shader.id, "projection");
	terrain_shader.view = glGetUniformLocation(terrain_shader.id, "view");

	terrain_shader.shadow_view = glGetUniformLocation(terrain_shader.id, "shadowView");
	terrain_shader.shadow_projection = glGetUniformLocation(terrain_shader.id, "shadowProjection");

	terrain_shader.shadowmap = glGetUniformLocation(terrain_shader.id, "shadowMap");

	terrain_shader.n_textures = glGetUniformLocation(terrain_shader.id, "_nTextures");
	terrain_shader.texture_distributor = glGetUniformLocation(terrain_shader.id, "_TextureDistributor");
	terrain_shader.tex0 = glGetUniformLocation(terrain_shader.id, "_Texture_0");
	terrain_shader.tex1 = glGetUniformLocation(terrain_shader.id, "_Texture_1");
	terrain_shader.tex2 = glGetUniformLocation(terrain_shader.id, "_Texture_2");
	terrain_shader.tex3 = glGetUniformLocation(terrain_shader.id, "_Texture_3");

	terrain_shader.Ia = glGetUniformLocation(terrain_shader.id, "Ia");
	terrain_shader.Ka = glGetUniformLocation(terrain_shader.id, "Ka");
	terrain_shader.Id = glGetUniformLocation(terrain_shader.id, "Id");
	terrain_shader.Kd = glGetUniformLocation(terrain_shader.id, "Kd");
	terrain_shader.L = glGetUniformLocation(terrain_shader.id, "L");
}

void MasterRender::InitShadowShader()
{
	unsigned int def_shader_v = ShaderCompiler::CompileVertex("Resources/Shaders/shadowV.ver");
	unsigned int def_shader_f = ShaderCompiler::CompileFragment("Resources/Shaders/shadowF.fra");
	shadow_shader.id = ShaderCompiler::CompileShader(def_shader_v, def_shader_f);

	shadow_shader.projection = glGetUniformLocation(shadow_shader.id, "projection");
	shadow_shader.view = glGetUniformLocation(shadow_shader.id, "view");
	shadow_shader.model = glGetUniformLocation(shadow_shader.id, "model");

	shadow_shader.has_anim = glGetUniformLocation(shadow_shader.id, "has_anim");
	shadow_shader.bones = glGetUniformLocation(shadow_shader.id, "bones");
}

void MasterRender::InitParticleShader()
{
	unsigned int vertex = ShaderCompiler::CompileVertex("Resources/Particles/particleV.ver");
	unsigned int fragment = ShaderCompiler::CompileFragment("Resources/Particles/particleF.fra");

	particle_shader.id = ShaderCompiler::CompileShader(vertex, fragment);

	particle_shader.projection = glGetUniformLocation(particle_shader.id, "projection");
	particle_shader.view = glGetUniformLocation(particle_shader.id, "view");

	particle_shader.size = glGetUniformLocation(particle_shader.id, "size");
	particle_shader.tex = glGetUniformLocation(particle_shader.id, "tex");
	particle_shader.s_color = glGetUniformLocation(particle_shader.id, "s_color");
	particle_shader.use_color_time = glGetUniformLocation(particle_shader.id, "use_color_time");

	particle_shader.texture_anim = glGetUniformLocation(particle_shader.id, "texture_anim");
	particle_shader.life_time = glGetUniformLocation(particle_shader.id, "life_time");
	particle_shader.tex_anim_data = glGetUniformLocation(particle_shader.id, "tex_anim_data");
}
