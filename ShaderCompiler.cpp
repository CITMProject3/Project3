#include "Application.h"
#include "ShaderComplier.h"
#include "ModuleFileSystem.h"
#include "Glew\include\glew.h"
#include "SDL\include\SDL_opengl.h"
#include <gl/GL.h>
#include <gl/GLU.h>

#pragma comment (lib, "glu32.lib")    /* link OpenGL Utility lib     */
#pragma comment (lib, "opengl32.lib") /* link Microsoft OpenGL lib   */
#pragma comment (lib, "Glew/libx86/glew32.lib") 

bool ShaderCompiler::TryCompileVertex(const char* path)
{
	char* buffer = nullptr;
	int size = App->file_system->Load(path, &buffer);

	if (size <= 0)
	{
		if(buffer)
			delete[] buffer;
		LOG("Vertex shader: %s can't be loaded with filesystem.", path);
		return false;
	}

	string source = string(buffer, size);
	GLuint vertex_shader;
	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	const GLchar* gl_source = (const GLchar *)source.c_str();
	glShaderSource(vertex_shader, 1, &gl_source, 0);
	glCompileShader(vertex_shader);

	GLint success;
	GLchar info[512];
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
	if (success == 0)
	{
		glGetShaderInfoLog(vertex_shader, 512, NULL, info);
		LOG("Vertex shader: %s compilation error (%s)", path, info);
	}

	glDeleteShader(vertex_shader);

	delete[] buffer;

	return (success != 0) ? true : false;
}

bool ShaderCompiler::TryCompileFragment(const char * path)
{
	char* buffer = nullptr;
	int size = App->file_system->Load(path, &buffer);

	if (size <= 0)
	{
		if(buffer)
			delete[] buffer;
		LOG("Vertex shader: %s can't be loaded with filesystem.", path);
		return false;
	}

	string source = string(buffer, size);
	GLuint fragment_shader;
	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	const GLchar* gl_source = (const GLchar *)source.c_str();
	glShaderSource(fragment_shader, 1, &gl_source, 0);
	glCompileShader(fragment_shader);

	GLint success;
	GLchar info[512];
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
	if (success == 0)
	{
		glGetShaderInfoLog(fragment_shader, 512, NULL, info);
		LOG("Fragment shader: %s compilation error (%s)", path, info);
	}

	glDeleteShader(fragment_shader);

	delete[] buffer;

	return (success != 0) ? true : false;
}

bool ShaderCompiler::TryCompileShader(const char * vertex_path, const char * fragment_path)
{
	bool ret = false;

	int ret_vertex, ret_fragment;
	ret_vertex = CompileVertex(vertex_path);
	ret_fragment = CompileFragment(fragment_path);

	if (ret_vertex != -1 && ret_fragment != -1)
	{
		GLint success;
		GLuint vertex_shader = ret_vertex;
		GLuint fragment_shader = ret_fragment;

		GLuint shader_program = glCreateProgram();
		glAttachShader(shader_program, vertex_shader);
		glAttachShader(shader_program, fragment_shader);
		glLinkProgram(shader_program);

		glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
		if (success == 0)
		{
			GLchar info[512];
			glGetProgramInfoLog(shader_program, 512, NULL, info);
			LOG("Shader link error: %s", info);
		}
		else
			ret = true;

		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);
		glDeleteShader(shader_program);
	}

	return ret;
}

int ShaderCompiler::CompileVertex(const char * path)
{
	int ret = -1;
	char* buffer = nullptr;
	int size = App->file_system->Load(path, &buffer);

	if (size <= 0)
	{
		if(buffer)
			delete[] buffer;
		LOG("Vertex shader: %s can't be loaded with filesystem.", path);
		return ret;
	}

	string source = string(buffer, size);
	GLuint vertex_shader;
	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	const GLchar* gl_source = (const GLchar *)source.c_str();
	glShaderSource(vertex_shader, 1, &gl_source, 0);
	glCompileShader(vertex_shader);

	GLint success;
	GLchar info[512];
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
	if (success == 0)
	{
		glGetShaderInfoLog(vertex_shader, 512, NULL, info);
		LOG("Vertex shader: %s compilation error (%s)", path, info);
	}
	else
		ret = vertex_shader;

	delete[] buffer;

	return ret;
}

int ShaderCompiler::CompileFragment(const char * path)
{
	int ret = -1;
	char* buffer = nullptr;
	int size = App->file_system->Load(path, &buffer);

	if (size <= 0)
	{
		if(buffer)
			delete[] buffer;
		LOG("Vertex shader: %s can't be loaded with filesystem.", path);
		return ret;
	}

	string source = string(buffer, size);
	GLuint fragment_shader;
	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	const GLchar* gl_source = (const GLchar *)source.c_str();
	glShaderSource(fragment_shader, 1, &gl_source, 0);
	glCompileShader(fragment_shader);

	GLint success;
	GLchar info[512];
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
	if (success == 0)
	{
		glGetShaderInfoLog(fragment_shader, 512, NULL, info);
		LOG("Fragment shader: %s compilation error (%s)", path, info);
	}
	else
		ret = fragment_shader;
	delete[] buffer;

	return ret;
}

int ShaderCompiler::CompileShader(unsigned int vertex_id, unsigned int fragment_id)
{
	GLint success;
	GLuint vertex_shader = vertex_id;
	GLuint fragment_shader = fragment_id;

	GLuint shader_program = glCreateProgram();
	glAttachShader(shader_program, vertex_shader);
	glAttachShader(shader_program, fragment_shader);
	glLinkProgram(shader_program);

	glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
	if (success == 0)
	{
		GLchar info[512];
		glGetProgramInfoLog(shader_program, 512, NULL, info);
		LOG("Shader link error: %s", info);
	}
	
	return shader_program;
}

int ShaderCompiler::LoadDefaultShader()
{
	GLuint vertex_shader, fragment_shader;
	GLuint shader = glCreateProgram();

	const GLchar* vertex_code =
		"#version 330 core \n"
		"layout(location = 0) in vec3 position;\n"
		"layout(location = 1) in vec2 texCoord;\n"
		"layout(location = 2) in vec3 normal;\n"
		"layout(location = 3) in vec3 tangent;\n"
		"out vec2 TexCoord;\n"
		"out vec3 normal0;\n"
		"out vec3 tangent0;\n"
		"out vec3 world_pos0;\n"
		"uniform mat4 model;\n"
		"uniform mat4 view;\n"
		"uniform mat4 projection;\n"
		"void main()\n"
		"{\n"
		"	gl_Position = projection * view * model * vec4(position, 1.0f);\n"
		"	TexCoord = texCoord;\n"
		"	normal0 = (model * vec4(normal, 0.0f)).xyz;\n"
		"   tangent0 = (model * vec4(tangent, 0.0f)).xyz;\n"
		"   world_pos0 = (model * vec4(position, 1.0f)).xyz;\n"
		"}\n";

	const GLchar* fragment_code =
		"#version 330 core\n"
		"in vec2 TexCoord;\n"
		"in vec3 normal0;\n"
		"in vec3 tangent0;\n"
		"in vec3 world_pos0;\n"
		"out vec4 color;\n"
		"uniform vec3 _EyeWorldPos;\n"
		"uniform bool _HasTexture;\n"
		"uniform sampler2D _Texture;\n"
		"uniform sampler2D _NormalMap;\n"
		"uniform bool _HasNormalMap;\n"
		"uniform float _AmbientIntensity;\n"
		"uniform vec3 _AmbientColor;\n"
		"uniform bool _HasDirectional;\n"
		"uniform float _DirectionalIntensity;\n"
		"uniform vec3 _DirectionalColor;\n"
		"uniform vec3 _DirectionalDirection;\n"
		"uniform vec4 material_color;\n"
		"uniform float _specular;\n"
		"uniform float _alpha_val;\n"

		"vec3 CalculateBumpedNormal()\n"
		"{\n"
		"	vec3 normal = normalize(normal0);\n"
		"   vec3 tangent = normalize(tangent0);\n"
		"   tangent = normalize(tangent - dot(tangent, normal) * normal);\n"
		"   vec3 bitangent = cross(tangent, normal);\n"
		"   vec3 bumpmap_normal = texture(_NormalMap, TexCoord).xyz;\n"
		"   bumpmap_normal = 2.0f * bumpmap_normal - vec3(1.0f, 1.0f, 1.0f);\n"
		"   vec3 new_normal;\n"
		"   mat3 tan_bit_nor = mat3(tangent, bitangent, normal);\n"
		"   new_normal = tan_bit_nor * bumpmap_normal;\n"
		"   new_normal = normalize(new_normal);\n"
		"   return new_normal;\n"
		"}\n"

		"float stepmix(float edge0, float edge1, float E, float x)\n"
		"{\n"
		"	float T = clamp(0.5f * (x - edge0 + E) / E, 0.0f, 1.0f);\n"
		"	return mix(edge0, edge1, T);\n"
		"}\n"

		"float step(float edge, float x)\n"
		"{\n"
		"	return x < edge ? 0.0f : 1.0f;\n"
		"}\n"

		"void main()\n"
		"{\n"
		"   vec4 tex_color = (_HasTexture) ? texture(_Texture, TexCoord) : vec4(1,1,1,1);\n"
		"	if(tex_color.a < _alpha_val) discard;\n"
		"   vec3 new_normal = (_HasNormalMap) ? CalculateBumpedNormal() : normal0;\n"
		"	vec4 ambient = vec4(_AmbientIntensity) * vec4(_AmbientColor, 1.0f);\n"
		"	vec4 directional_color = vec4(_DirectionalColor * _DirectionalIntensity, 1.0f);\n"
		"	float ddf = dot(normalize(new_normal), -_DirectionalDirection);\n"
		"   vec4 diffuse = vec4(0,0,0,0);\n"
		"   vec4 specular_color = vec4(0,0,0,0);\n"
		"   if(_HasDirectional && ddf > 0)\n"
		"   {\n"
		"		const float A = 0.1f;\n"
		"		const float B = 0.3f;\n"
		"		const float C = 0.6f;\n"
		"		const float D = 1.0f;\n"
		"		float E = fwidth(ddf);\n"

		"			 if(ddf > A - E && ddf < A + E) ddf = stepmix(A, B, E, ddf);\n"
		"       else if(ddf > B - E && ddf < B + E) ddf = stepmix(B, C, E, ddf);\n"
		"       else if(ddf > C - E && ddf < C + E) ddf = stepmix(C, D, E, ddf);\n"
		"		else if(ddf < A) ddf = 0.0f;\n"
		"		else if(ddf < B) ddf = B;\n"
		"		else if(ddf < C) ddf = C;\n"
		"		else ddf = D;\n"

		"		diffuse = vec4(_DirectionalColor * _DirectionalIntensity * ddf, 1.0f);\n"

		"       vec3 vertex_to_eye = normalize(_EyeWorldPos - world_pos0);\n"
		"       vec3 light_reflect = normalize(reflect(_DirectionalDirection, normalize(normal0)));\n"
		"       float sf = dot(vertex_to_eye, light_reflect);\n"
		"       if(sf > 0)\n"
		"       {\n"

		"			E = fwidth(sf);\n"
		"			if(sf > 0.5f - E && sf < 0.5f + E)\n"
		"				sf = stepmix(0.5f, 0.8f, E, sf);\n"
		"			else\n"
		"				sf = step(0.5f, sf);\n"
		"			specular_color = vec4(_DirectionalColor * _specular * sf, 1.0f);\n"
		"       }\n"
		"   }\n"
		"	color = material_color * tex_color * (ambient + diffuse + specular_color);\n"
		"}\n";

	GLint success;
	GLchar info[512];

	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_code, 0);
	glShaderSource(fragment_shader, 1, &fragment_code, 0);
	glCompileShader(vertex_shader);
	
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
	if (success == 0)
	{
		glGetShaderInfoLog(vertex_shader, 512, NULL, info);
		LOG("Default shader vertex compilation error (%s)", info);
	}
	glCompileShader(fragment_shader);
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
	if (success == 0)
	{
		glGetShaderInfoLog(fragment_shader, 512, NULL, info);
		LOG("Default shader fragment compilation error (%s)", info);
	}

	glAttachShader(shader, vertex_shader);
	glAttachShader(shader, fragment_shader);
	glLinkProgram(shader);

	return shader;
}

int ShaderCompiler::LoadDefaultAnimShader()
{
	GLuint vertex_shader, fragment_shader;
	GLuint shader = glCreateProgram();

	const GLchar* vertex_code =
		"#version 330 core\n"
		"layout(location = 0) in vec3 position;\n"
		"layout(location = 1) in vec2 texCoord;\n"
		"layout(location = 2) in vec3 normal;\n"
		"layout(location = 3) in vec3 tangent;\n"
		"layout(location = 4) in ivec4 bone_ids;\n"
		"layout(location = 5) in vec4 weights;\n"
		"out vec2 TexCoord;\n"
		"out vec3 normal0;\n"
		"out vec3 tangent0;\n"
		"out vec3 world_pos0;\n"
		"const int MAX_BONES = 100;\n"
		"uniform mat4 model;\n"
		"uniform mat4 view;\n"
		"uniform mat4 projection;\n"
		"uniform mat4 bones[MAX_BONES];\n"
		"void main()\n"
		"{\n"
		"	mat4 bone_transform = bones[bone_ids[0]] * weights[0];\n"
		"	bone_transform += bones[bone_ids[1]] * weights[1];\n"
		"	bone_transform += bones[bone_ids[2]] * weights[2];\n"
		"	bone_transform += bones[bone_ids[3]] * weights[3];\n"
		"	vec4 pos_t = bone_transform * vec4(position, 1.0f);\n"
		"	gl_Position = projection * view * model * pos_t;\n"
		"	TexCoord = texCoord;\n"
		"	normal0 = (model * bone_transform * vec4(normal, 0.0f)).xyz;\n"
		"	tangent0 = (model *  bone_transform * vec4(tangent, 0.0f)).xyz;\n"
		"	world_pos0 = (model *  bone_transform * vec4(position, 1.0f)).xyz;\n"
		"}\n";

	const GLchar* fragment_code =
		"#version 330 core\n"
		"in vec2 TexCoord;\n"
		"in vec3 normal0;\n"
		"in vec3 tangent0;\n"
		"in vec3 world_pos0;\n"
		"out vec4 color;\n"
		"uniform vec3 _EyeWorldPos;\n"
		"uniform bool _HasTexture;\n"
		"uniform sampler2D _Texture;\n"
		"uniform sampler2D _NormalMap;\n"
		"uniform bool _HasNormalMap;\n"
		"uniform float _AmbientIntensity;\n"
		"uniform vec3 _AmbientColor;\n"
		"uniform bool _HasDirectional;\n"
		"uniform float _DirectionalIntensity;\n"
		"uniform vec3 _DirectionalColor;\n"
		"uniform vec3 _DirectionalDirection;\n"
		"uniform vec4 material_color;\n"
		"uniform float _specular;\n"
		"uniform float _alpha_val;\n"

		"vec3 CalculateBumpedNormal()\n"
		"{\n"
		"	vec3 normal = normalize(normal0);\n"
		"   vec3 tangent = normalize(tangent0);\n"
		"   tangent = normalize(tangent - dot(tangent, normal) * normal);\n"
		"   vec3 bitangent = cross(tangent, normal);\n"
		"   vec3 bumpmap_normal = texture(_NormalMap, TexCoord).xyz;\n"
		"   bumpmap_normal = 2.0f * bumpmap_normal - vec3(1.0f, 1.0f, 1.0f);\n"
		"   vec3 new_normal;\n"
		"   mat3 tan_bit_nor = mat3(tangent, bitangent, normal);\n"
		"   new_normal = tan_bit_nor * bumpmap_normal;\n"
		"   new_normal = normalize(new_normal);\n"
		"   return new_normal;\n"
		"}\n"

		"float stepmix(float edge0, float edge1, float E, float x)\n"
		"{\n"
		"	float T = clamp(0.5f * (x - edge0 + E) / E, 0.0f, 1.0f);\n"
		"	return mix(edge0, edge1, T);\n"
		"}\n"

		"float step(float edge, float x)\n"
		"{\n"
		"	return x < edge ? 0.0f : 1.0f;\n"
		"}\n"

		"void main()\n"
		"{\n"
		"   vec4 tex_color = (_HasTexture) ? texture(_Texture, TexCoord) : vec4(1,1,1,1);\n"
		"	if(tex_color.a < _alpha_val) discard;\n"
		"   vec3 new_normal = (_HasNormalMap) ? CalculateBumpedNormal() : normal0;\n"
		"	vec4 ambient = vec4(_AmbientIntensity) * vec4(_AmbientColor, 1.0f);\n"
		"	vec4 directional_color = vec4(_DirectionalColor * _DirectionalIntensity, 1.0f);\n"
		"	float ddf = dot(normalize(new_normal), -_DirectionalDirection);\n"
		"   vec4 diffuse = vec4(0,0,0,0);\n"
		"   vec4 specular_color = vec4(0,0,0,0);\n"
		"   if(_HasDirectional && ddf > 0)\n"
		"   {\n"
		"		const float A = 0.1f;\n"
		"		const float B = 0.3f;\n"
		"		const float C = 0.6f;\n"
		"		const float D = 1.0f;\n"
		"		float E = fwidth(ddf);\n"

		"			 if(ddf > A - E && ddf < A + E) ddf = stepmix(A, B, E, ddf);\n"
		"       else if(ddf > B - E && ddf < B + E) ddf = stepmix(B, C, E, ddf);\n"
		"       else if(ddf > C - E && ddf < C + E) ddf = stepmix(C, D, E, ddf);\n"
		"		else if(ddf < A) ddf = 0.0f;\n"
		"		else if(ddf < B) ddf = B;\n"
		"		else if(ddf < C) ddf = C;\n"
		"		else ddf = D;\n"

		"		diffuse = vec4(_DirectionalColor * _DirectionalIntensity * ddf, 1.0f);\n"

		"       vec3 vertex_to_eye = normalize(_EyeWorldPos - world_pos0);\n"
		"       vec3 light_reflect = normalize(reflect(_DirectionalDirection, normalize(normal0)));\n"
		"       float sf = dot(vertex_to_eye, light_reflect);\n"
		"       if(sf > 0)\n"
		"       {\n"

		"			E = fwidth(sf);\n"
		"			if(sf > 0.5f - E && sf < 0.5f + E)\n"
		"				sf = stepmix(0.5f, 0.8f, E, sf);\n"
		"			else\n"
		"				sf = step(0.5f, sf);\n"
		"			specular_color = vec4(_DirectionalColor * _specular * sf, 1.0f);\n"
		"       }\n"
		"   }\n"
		"	color = material_color * tex_color * (ambient + diffuse + specular_color);\n"
		"   color.w = material_color.w;\n"
		"}\n";
	
	GLint success;
	GLchar info[512];

	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_code, 0);
	glShaderSource(fragment_shader, 1, &fragment_code, 0);
	glCompileShader(vertex_shader);

	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
	if (success == 0)
	{
		glGetShaderInfoLog(vertex_shader, 512, NULL, info);
		LOG("Default Anim shader vertex compilation error (%s)", info);
	}
	glCompileShader(fragment_shader);
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
	if (success == 0)
	{
		glGetShaderInfoLog(fragment_shader, 512, NULL, info);
		LOG("Default Anim shader fragment compilation error (%s)", info);
	}

	glAttachShader(shader, vertex_shader);
	glAttachShader(shader, fragment_shader);
	glLinkProgram(shader);

	return shader;
}

int ShaderCompiler::LoadDefaultTerrainShader()
{
	GLuint vertex_shader, fragment_shader;
	GLuint shader = glCreateProgram();

	const GLchar* vertex_code =
		"#version 330 core \n"
		"layout(location = 0) in vec3 position;\n"
		"layout(location = 1) in vec2 texCoord;\n"
		"layout(location = 2) in vec3 normal;\n"
		"layout(location = 3) in vec3 tangent;\n"
		"layout(location = 4) in vec2 TerrainCoord;\n"
		"out vec2 TexCoord;\n"
		"out vec3 normal0;\n"
		"out vec2 terrainCoord;\n"
		"out vec3 tangent0;\n"
		"out vec3 world_pos0;\n"
		"uniform mat4 model;\n"
		"uniform mat4 view;\n"
		"uniform mat4 projection;\n"
		"void main()\n"
		"{\n"
		"	gl_Position = projection * view * model * vec4(position, 1.0f);\n"
		"	TexCoord = texCoord;\n"
		"   terrainCoord = TerrainCoord;\n"
		"	normal0 = (model * vec4(normal, 0.0f)).xyz;\n"
		"   tangent0 = (model * vec4(tangent, 0.0f)).xyz;\n"
		"   world_pos0 = (model * vec4(position, 1.0f)).xyz;\n"
		"}\n";

	const GLchar* fragment_code =
		"#version 330 core\n" //1
		"in vec2 TexCoord;\n"
		"in vec3 normal0;\n"
		"in vec2 terrainCoord;\n"
		"in vec3 tangent0;\n"
		"in vec3 world_pos0;\n"
		"out vec4 color;\n"
		"uniform vec3 _EyeWorldPos;\n"

		"uniform sampler2D _TextureDistributor;\n"
		"uniform int _nTextures;\n" //10
		"uniform sampler2D _Texture_0;\n"
		"uniform sampler2D _Texture_1;\n"
		"uniform sampler2D _Texture_2;\n"
		"uniform sampler2D _Texture_3;\n"
		"uniform sampler2D _Texture_4;\n"
		"uniform sampler2D _Texture_5;\n"
		"uniform sampler2D _Texture_6;\n"
		"uniform sampler2D _Texture_7;\n"
		"uniform sampler2D _Texture_8;\n"
		"uniform sampler2D _Texture_9;\n" //20

		"uniform sampler2D _NormalMap;\n"
		"uniform bool _HasNormalMap;\n"
		"uniform float _AmbientIntensity;\n"
		"uniform vec3 _AmbientColor;\n"
		"uniform bool _HasDirectional;\n"
		"uniform float _DirectionalIntensity;\n"
		"uniform vec3 _DirectionalColor;\n"
		"uniform vec3 _DirectionalDirection;\n"
		"uniform vec4 material_color;\n"
		"uniform float _specular;\n" //30

		"vec3 CalculateBumpedNormal()\n"
		"{\n"
		"	vec3 normal = normalize(normal0);\n"
		"   vec3 tangent = normalize(tangent0);\n"
		"   tangent = normalize(tangent - dot(tangent, normal) * normal);\n"
		"   vec3 bitangent = cross(tangent, normal);\n"
		"   vec3 bumpmap_normal = texture(_NormalMap, TexCoord).xyz;\n"
		"   bumpmap_normal = 2.0f * bumpmap_normal - vec3(1.0f, 1.0f, 1.0f);\n"
		"   vec3 new_normal;\n"
		"   mat3 tan_bit_nor = mat3(tangent, bitangent, normal);\n" //40
		"   new_normal = tan_bit_nor * bumpmap_normal;\n"
		"   new_normal = normalize(new_normal);\n"
		"   return new_normal;\n"
		"}\n"

		"float stepmix(float edge0, float edge1, float E, float x)\n"
		"{\n"
		"	float T = clamp(0.5f * (x - edge0 + E) / E, 0.0f, 1.0f);\n"
		"	return mix(edge0, edge1, T);\n"
		"}\n"

		"float step(float edge, float x)\n" //50
		"{\n"
		"	return x < edge ? 0.0f : 1.0f;\n"
		"}\n"

		"int getTexN(float a)\n"
		"{\n"
		"	return int(a / 0.1);\n"
		"}\n"

		"void main()\n"
		"{\n"
		"   vec3 new_normal = (_HasNormalMap) ? CalculateBumpedNormal() : normal0;\n" //60
		"	vec4 ambient = vec4(_AmbientIntensity) * vec4(_AmbientColor, 1.0f);\n"
		"	vec4 directional_color = vec4(_DirectionalColor * _DirectionalIntensity, 1.0f);\n"
		"	float ddf = dot(normalize(new_normal), -_DirectionalDirection);\n"
		"   vec4 diffuse = vec4(0,0,0,0);\n"
		"   vec4 specular_color = vec4(0,0,0,0);\n"
		"   if(_HasDirectional && ddf > 0)\n"
		"   {\n"
		"		const float A = 0.1f;\n"
		"		const float B = 0.3f;\n"
		"		const float C = 0.6f;\n"//70
		"		const float D = 1.0f;\n"
		"		float E = fwidth(ddf);\n"

		"			 if(ddf > A - E && ddf < A + E) ddf = stepmix(A, B, E, ddf);\n"
		"       else if(ddf > B - E && ddf < B + E) ddf = stepmix(B, C, E, ddf);\n"
		"       else if(ddf > C - E && ddf < C + E) ddf = stepmix(C, D, E, ddf);\n"
		"		else if(ddf < A) ddf = 0.0f;\n"
		"		else if(ddf < B) ddf = B;\n"
		"		else if(ddf < C) ddf = C;\n"
		"		else ddf = D;\n"

		"		diffuse = vec4(_DirectionalColor * _DirectionalIntensity * ddf, 1.0f);\n" //80

		"       vec3 vertex_to_eye = normalize(_EyeWorldPos - world_pos0);\n"
		"       vec3 light_reflect = normalize(reflect(_DirectionalDirection, normalize(normal0)));\n"
		"       float sf = dot(vertex_to_eye, light_reflect);\n"
		"       if(sf > 0)\n"
		"       {\n"

		"			E = fwidth(sf);\n"
		"			if(sf > 0.5f - E && sf < 0.5f + E)\n"
		"				sf = stepmix(0.5f, 0.8f, E, sf);\n"
		"			else\n"
		"				sf = step(0.5f, sf);\n" //90
		"			specular_color = vec4(_DirectionalColor * _specular * sf, 1.0f);\n"
		"       }\n"
		"   }\n"

		"vec4 tex_color = vec4(0,0,0,0);\n"
		"if(_nTextures > 0)\n"
		"{\n"
		"	vec4 dist = texture(_TextureDistributor, terrainCoord);\n"
		"	if(dist[0] > 0) {\n"
		"		tex_color += texture(_Texture_0, TexCoord) * dist[0];\n"
		"		}\n"
		"	if(dist[1] > 0) {\n"
		"		tex_color += texture(_Texture_1, TexCoord) * dist[1];\n"
		"		}\n"
		"	if(dist[2] > 0) {\n"
		"		tex_color += texture(_Texture_2, TexCoord) * dist[2];\n"
		"		}\n"
		"	if(dist[3] > 0) {\n"
		"		tex_color += texture(_Texture_3, TexCoord) * dist[3];\n"
		"		}\n"

		"}\n"
		"else\n"
		"{\n"
		"	tex_color = texture(_TextureDistributor, terrainCoord); \n"
		"}\n"

		"	color = material_color * tex_color * (ambient + diffuse + specular_color);\n"
		"   color.w = material_color.w;\n"
		"}\n";


	GLint success;
	GLchar info[512];

	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_code, 0);
	glShaderSource(fragment_shader, 1, &fragment_code, 0);
	glCompileShader(vertex_shader);

	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
	if (success == 0)
	{
		glGetShaderInfoLog(vertex_shader, 512, NULL, info);
		LOG("Default shader vertex compilation error (%s)", info);
	}
	glCompileShader(fragment_shader);
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
	if (success == 0)
	{
		glGetShaderInfoLog(fragment_shader, 512, NULL, info);
		LOG("Default shader fragment compilation error (%s)", info);
	}

	glAttachShader(shader, vertex_shader);
	glAttachShader(shader, fragment_shader);
	glLinkProgram(shader);

	return shader;
}

int ShaderCompiler::LoadDefaultBilboardShader()
{
	GLuint vertex_shader, fragment_shader;
	GLuint shader = glCreateProgram();

	const GLchar* vertex_code =
		"#version 330 core \n"
		"layout(location = 0) in vec3 position;\n"
		"layout(location = 1) in vec2 texCoord;\n"
		"out vec2 TexCoord;\n"
		"uniform mat4 view;\n"
		"uniform mat4 projection;\n"
		"uniform vec3 center;\n"
		"uniform vec2 size;\n"
		"void main()\n"
		"{\n"
		"   vec3 vertex_position = center + vec3(view[0][0], view[1][0], view[2][0]) * position.x * size.x + vec3(view[0][1], view[1][1], view[2][1]) * position.y * size.y;\n"
		"	gl_Position = projection * view * vec4(vertex_position, 1.0);\n"
		"	TexCoord = texCoord;\n"
		"}\n";

	const GLchar* fragment_code =
		"#version 330 core\n"
		"in vec2 TexCoord;\n"
		"out vec4 color;\n"

		"uniform sampler2D tex;\n"

		"void main()\n"
		"{\n"
		"	color = texture(tex, TexCoord);\n"
		"}\n";

	GLint success;
	GLchar info[512];

	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_code, 0);
	glShaderSource(fragment_shader, 1, &fragment_code, 0);
	glCompileShader(vertex_shader);

	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
	if (success == 0)
	{
		glGetShaderInfoLog(vertex_shader, 512, NULL, info);
		LOG("Default shader billboard vertex compilation error (%s)", info);
	}
	glCompileShader(fragment_shader);
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
	if (success == 0)
	{
		glGetShaderInfoLog(fragment_shader, 512, NULL, info);
		LOG("Default shader billboard fragment compilation error (%s)", info);
	}

	glAttachShader(shader, vertex_shader);
	glAttachShader(shader, fragment_shader);
	glLinkProgram(shader);

	return shader;
}

void ShaderCompiler::DeleteShader(unsigned int shader_id)
{
	glDeleteShader(shader_id);
}
