#include "Application.h"
#include "ModuleEditor.h"

#include "Assets.h"
#include "MaterialCreatorWindow.h"
#include "ShaderComplier.h"

#include <vector>

MaterialCreatorWindow::MaterialCreatorWindow()
{
	save_path.resize(512);
	u_sampler2d.resize(512);
}

MaterialCreatorWindow::~MaterialCreatorWindow()
{}

void MaterialCreatorWindow::Draw()
{
	if (!active)
		return;

	ImGui::Begin("Material Creator", &active, flags);

	ImGui::Text("Save path: "); ImGui::SameLine();
	ImGui::InputText("###save_path", save_path._Myptr(), save_path.capacity());
	
	ImGui::Text("Vertex program: "); ImGui::SameLine();
	string vertex_name = vertex + "###vertex_title";
	if(ImGui::BeginMenu(vertex_name.data()))
	{
		vector<string> vertex_list;
		App->editor->assets->GetAllFilesByType(FileType::VERTEX, vertex_list);

		for (vector<string>::iterator ver = vertex_list.begin(); ver != vertex_list.end(); ver++)
		{
			if (ImGui::MenuItem((*ver).data()))
			{
				vertex = (*ver);
				material.vertex_path = App->resource_manager->FindFile(vertex);
			}
		}

		ImGui::EndMenu();
	}

	ImGui::Text("Fragment program: "); ImGui::SameLine();
	string frag_name = fragment + "###frag_title";
	if (ImGui::BeginMenu(frag_name.data()))
	{
		vector<string> fragment_list;
		App->editor->assets->GetAllFilesByType(FileType::FRAGMENT, fragment_list);

		for (vector<string>::iterator frag = fragment_list.begin(); frag != fragment_list.end(); frag++)
		{
			if (ImGui::MenuItem((*frag).data()))
			{
				fragment = (*frag);
				material.fragment_path = App->resource_manager->FindFile(fragment);
			}
		}

		ImGui::EndMenu();
	}

	SetUniforms();

	if (ImGui::Button("Create Shader"))
	{
		if (ShaderCompiler::TryCompileShader(vertex.data(), fragment.data()))
		{
			compilation_result = "Shader creation success!";
			App->resource_manager->SaveMaterial(material, save_path.data(), material.uuid);
		}
		else
			compilation_result = "Compilation failed";
	}

	ImGui::Text("Result: %s", compilation_result.data());

	ImGui::End();
}

void MaterialCreatorWindow::LoadToEdit(const char * path)
{
	material.Load(path);
	vertex = material.vertex_path.data();
	fragment = material.fragment_path.data();
	save_path = "Write again the save path";
}

void MaterialCreatorWindow::SetUniforms()
{
	ImGui::Separator();
	ImGui::Text("Variable name: "); ImGui::SameLine();
	ImGui::InputText("###variable_name", uniform_name._Myptr(), uniform_name.capacity());
	ImGui::Text("Type: "); ImGui::SameLine();
	if (ImGui::BeginMenu(uniform_type_char.data()))
	{
		if (ImGui::MenuItem("Bool"))
		{
			uniform_type_char = "Bool";
			uniform_type = UniformType::U_BOOL;
		}

		if (ImGui::MenuItem("Int"))
		{
			uniform_type_char = "Int";
			uniform_type = UniformType::U_INT;
		}

		if (ImGui::MenuItem("Float"))
		{
			uniform_type_char = "Float";
			uniform_type = UniformType::U_FLOAT;
		}

		if (ImGui::MenuItem("Vec2"))
		{
			uniform_type_char = "Vec2";
			uniform_type = UniformType::U_VEC2;
		}

		if (ImGui::MenuItem("Vec3"))
		{
			uniform_type_char = "Vec3";
			uniform_type = UniformType::U_VEC3;
		}

		if (ImGui::MenuItem("Vec4"))
		{
			uniform_type_char = "Vec4";
			uniform_type = UniformType::U_VEC4;
		}

		if (ImGui::MenuItem("Mat4x4"))
		{
			uniform_type_char = "Mat4x4";
			uniform_type = UniformType::U_MAT4X4;
		}

		if (ImGui::MenuItem("Sampler2D"))
		{
			uniform_type_char = "Sampler2D";
			uniform_type = UniformType::U_SAMPLER2D;
		}
		ImGui::EndMenu();
	}

	SetUniformValue();

	if (ImGui::Button("Add uniform"))
	{
		material.AddUniform(uniform_name, uniform_type, content);
	}

	PrintUniforms();
	
}

void MaterialCreatorWindow::SetUniformValue()
{
	switch (uniform_type)
	{
	case U_BOOL:
		if (ImGui::Checkbox("###bool_u", &u_bool))
		{
			if (content != nullptr)
				delete[] content;

			content = new char[sizeof(bool)];
			memcpy(content, &u_bool, sizeof(bool));
		}
		break;
	case U_INT:
		if (ImGui::InputInt("###int_u", &u_int))
		{
			if (content != nullptr)
				delete[] content;
			content = new char[sizeof(int)];
			memcpy(content, &u_int, sizeof(int));
		}
		break;
	case U_FLOAT:
		if(ImGui::InputFloat("###float_u", &u_float))
		{
			if (content != nullptr)
				delete[] content;
			content = new char[sizeof(float)];
			memcpy(content, &u_float, sizeof(float));
		}
		break;
	case U_VEC2:
		if (ImGui::InputFloat2("###float2_u", u_vec2.ptr()))
		{
			if (content != nullptr)
				delete[] content;
			content = new char[sizeof(float) * 2];
			memcpy(content, &u_vec2, sizeof(float) * 2);
		}
		break;
	case U_VEC3:
		if (ImGui::InputFloat3("###float3_u", u_vec3.ptr()))
		{
			if (content != nullptr)
				delete[] content;
			content = new char[sizeof(float) * 3];
			memcpy(content, &u_vec3, sizeof(float) * 3);
		}
		break;
	case U_VEC4:
		if (ImGui::InputFloat4("###float4_u", u_vec4.ptr()))
		{
			if (content != nullptr)
				delete[] content;
			content = new char[sizeof(float) * 4];
			memcpy(content, &u_vec4, sizeof(float) * 4);
		}
		break;
	case U_MAT4X4:
		ImGui::Text("Matrices can be initalized only by code");
		break;
	case U_SAMPLER2D:
		if (ImGui::BeginMenu("Select a texture:"))
		{
			vector<string> textures_list;
			App->editor->assets->GetAllFilesByType(FileType::IMAGE, textures_list);
			App->editor->assets->GetAllFilesByType(FileType::RENDER_TEXTURE, textures_list);

			for (size_t i = 0; i < textures_list.size(); ++i)
			{
				if (ImGui::MenuItem(textures_list[i].data()))
				{
					if (content != nullptr)
						delete[] content;

					u_sampler2d = App->resource_manager->FindFile(textures_list[i]);

					int sampler_size = u_sampler2d.size();
					content = new char[sizeof(int) + sampler_size * sizeof(char)];

					memcpy(content, &sampler_size, sizeof(int));
					char* pointer = content + sizeof(int);
					memcpy(pointer, u_sampler2d.c_str(), sizeof(char)* sampler_size);
				}
			}

			ImGui::EndMenu();
		}
		break;
	}
}

void MaterialCreatorWindow::PrintUniforms()
{
	vector<Uniform*> uniforms_to_remove;
	int i = 0;
	for (vector<Uniform*>::iterator it = material.uniforms.begin(); it != material.uniforms.end(); ++it)
	{
		ImGui::TextColored(ImVec4(0, 0, 1, 1), "Variable: ");
		ImGui::SameLine();
		ImGui::Text("%s", (*it)->name.data());

		switch ((*it)->type)
		{
		case U_BOOL:
		{
			ImGui::Text("Type: bool");
			bool uni_bool = *(reinterpret_cast<bool*>((*it)->value));
			if (uni_bool)
				ImGui::Text("Value: True");
			else
				ImGui::Text("Value: False");

			break;
		}
		case U_INT:
		{
			ImGui::Text("Type: int");
			int uni_int = *(reinterpret_cast<int*>((*it)->value));
			ImGui::Text("Value: %i", uni_int);
			break;
		}
		case U_FLOAT:
		{
			ImGui::Text("Type: float");
			float uni_float = *(reinterpret_cast<float*>((*it)->value));
			ImGui::Text("Value: %f", uni_float);
			break;
		}
		case U_VEC2:
		{
			ImGui::Text("Type: vec2");
			float2 uni_vec2 = float2(reinterpret_cast<float*>((*it)->value));
			ImGui::Text("Value X: %f", uni_vec2.x);
			ImGui::Text("Value Y: %f", uni_vec2.y);
			break;
		}
		case U_VEC3:
		{
			ImGui::Text("Type: vec3");
			float3 uni_vec3 = float3(reinterpret_cast<float*>((*it)->value));
			ImGui::Text("Value X: %f", uni_vec3.x);
			ImGui::Text("Value Y: %f", uni_vec3.y);
			ImGui::Text("Value Z: %f", uni_vec3.z);
			break;
		}
		case U_VEC4:
		{
			ImGui::Text("Type: vec4");
			float4 uni_vec4 = float4(reinterpret_cast<float*>((*it)->value));
			ImGui::Text("Value X: %f", uni_vec4.x);
			ImGui::Text("Value Y: %f", uni_vec4.y);
			ImGui::Text("Value Z: %f", uni_vec4.z);
			ImGui::Text("Value W: %f", uni_vec4.w);
			break;
		}
		case U_MAT4X4:
		{
			ImGui::Text("Type: mat4x4");
			ImGui::Text("Value: No matrix value visualization supported");
			break;
		}
		case U_SAMPLER2D:
			ImGui::Text("Type: sampler2D");
			int sampler_size = *(reinterpret_cast<int*>((*it)->value));
			string sampler_name;
			sampler_name.resize(sampler_size);
			memcpy(sampler_name._Myptr(), (*it)->value + sizeof(int), sampler_size);
			ImGui::Text("Value: %s", sampler_name.data());
			break;
		}
		ImGui::PushID(i);
		if (ImGui::Button("Remove"))
		{
			uniforms_to_remove.push_back(*it);
		}
		ImGui::PopID();
		ImGui::Separator();
		++i;
	}

	for (vector<Uniform*>::iterator it = uniforms_to_remove.begin(); it != uniforms_to_remove.end(); it++)
	{
		delete *it;
		material.uniforms.erase(std::find(material.uniforms.begin(), material.uniforms.end(), *it));
	}
}

