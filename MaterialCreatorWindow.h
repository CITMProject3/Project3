#ifndef __MATERIALCREATORWINDOW_H__
#define __MATERIALCREATORWINDOW_H__

#include "Window.h"
#include <string>
#include "Material.h"

using namespace std;

class MaterialCreatorWindow : public Window
{
public:
	MaterialCreatorWindow();
	~MaterialCreatorWindow();

	void Draw();
	void LoadToEdit(const char* path);

private:

	void SetUniforms();
	void SetUniformValue();
	void PrintUniforms();

private:
	Material material;
	string save_path;
	string vertex = "none";
	string fragment = "none";
	string compilation_result = "";
	string uniform_name;
	string uniform_type_char = "no type";
	UniformType uniform_type;
	char* content = nullptr;

	//Types
	bool u_bool;
	int u_int;
	float u_float;
	float2 u_vec2;
	float3 u_vec3;
	float4 u_vec4;
	float4x4 u_mat;
	string u_sampler2d;
};

#endif
