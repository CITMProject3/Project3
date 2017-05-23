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
}

void MasterRender::RenderDefaultShader(const GameObject* obj, const ComponentCamera* cam, const ComponentMaterial* material, const LightInfo* light) const
{
	glUseProgram(df_shader.id);

	glUniformMatrix4fv(df_shader.model, 1, GL_FALSE, *(obj->GetGlobalMatrix().Transposed()).v);
	glUniformMatrix4fv(df_shader.projection, 1, GL_FALSE, *cam->GetProjectionMatrix().v);
	glUniformMatrix4fv(df_shader.view, 1, GL_FALSE, *cam->GetViewMatrix().v);

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
	
	glUniform1f(df_shader.Ia, light->ambient_intensity);
	glUniform3f(df_shader.Ka, light->ambient_color.x, light->ambient_color.y, light->ambient_color.z);
	glUniform1f(df_shader.Id, light->directional_intensity);
	glUniform3f(df_shader.Kd, light->directional_color.x, light->directional_color.y, light->directional_color.z);
	glUniform3f(df_shader.L, light->directional_direction.x, light->directional_direction.y, light->directional_direction.z);
	glUniform1f(df_shader.Is, material->specular);

	glUniform4fv(df_shader.material_color, 1, float4(material->color).ptr());	
}

void MasterRender::InitDefaultShader()
{
	unsigned int def_shader_v = ShaderCompiler::CompileVertex("Resources/Shaders/defaultV.ver");
	unsigned int def_shader_f = ShaderCompiler::CompileFragment("Resources/Shaders/defaultF.fra");
	df_shader.id = ShaderCompiler::CompileShader(def_shader_v, def_shader_f);

	df_shader.model = glGetUniformLocation(df_shader.id, "model");
	df_shader.projection = glGetUniformLocation(df_shader.id, "projection");
	df_shader.view = glGetUniformLocation(df_shader.id, "view");

	df_shader.has_texture = glGetUniformLocation(df_shader.id, "_HasTexture");
	df_shader.texture = glGetUniformLocation(df_shader.id, "_Texture");

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
	
}
