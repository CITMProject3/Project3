#include "Application.h"
#include "ComponentMaterial.h"
#include "GameObject.h"
#include "imgui\imgui.h"
#include "Data.h"
#include "ResourceFileTexture.h"
#include "Assets.h"
#include "ResourceFileMaterial.h"
#include "ResourceFileRenderTexture.h"
#include "Glew\include\glew.h"

ComponentMaterial::ComponentMaterial(ComponentType type, GameObject* game_object) : Component(type, game_object)
{}

ComponentMaterial::~ComponentMaterial()
{
	CleanUp();
}

void ComponentMaterial::OnInspector(bool debug)
{
	string str = (string("Material") + string("##") + std::to_string(uuid));
	if (ImGui::CollapsingHeader(str.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (ImGui::IsItemClicked(1))
		{
			ImGui::OpenPopup("delete##material");
		}

		if (ImGui::BeginPopup("delete##material"))
		{
			if (ImGui::MenuItem("Delete"))
			{
				Remove();
			}
			ImGui::EndPopup();
		}

		if (material_path.size() == 0)
		{
			ImGui::Text("Material: ");
			ImGui::SameLine();
			std::string str = (material_name + "##" + std::to_string(uuid));
			if (ImGui::BeginMenu(str.c_str()))
			{
				vector<string> materials;
				App->editor->assets->GetAllFilesByType(FileType::MATERIAL, materials);

				for (vector<string>::iterator it = materials.begin(); it != materials.end(); ++it)
				{
					if (ImGui::MenuItem((*it).data()))
					{
						CleanUp();

						change_material_enabled = false;
						material_name = (*it).data();
						if (rc_material)
						{
							rc_material->Unload();
						}
						material_path = App->resource_manager->FindFile(material_name);
						rc_material = (ResourceFileMaterial*)App->resource_manager->LoadResource(material_path, ResourceFileType::RES_MATERIAL);
						for (vector<Uniform*>::iterator uni = rc_material->material.uniforms.begin(); uni != rc_material->material.uniforms.end(); ++uni)
						{
							if ((*uni)->type == UniformType::U_SAMPLER2D)
							{
								string texture_path;
								int name_size = *reinterpret_cast<int*>((*uni)->value);
								texture_path.resize(name_size);
								memcpy(texture_path._Myptr(), (*uni)->value + sizeof(int), name_size);

								ResourceFileType type = App->resource_manager->GetResourceType(texture_path.data());

								if (type == ResourceFileType::RES_TEXTURE)
								{
									ResourceFileTexture* rc_tmp = (ResourceFileTexture*)App->resource_manager->LoadResource(texture_path, ResourceFileType::RES_TEXTURE);
									tex_resources.push_back(rc_tmp);
									texture_ids.insert(pair<string, uint>((*uni)->name.data(), rc_tmp->GetTexture()));
								}
								else
								{
									ResourceFileRenderTexture* rc_rndtx = (ResourceFileRenderTexture*)App->resource_manager->LoadResource(texture_path, ResourceFileType::RES_RENDER_TEX);
									tex_resources.push_back(rc_rndtx);
									texture_ids.insert(pair<string, uint>((*uni)->name.data(), rc_rndtx->GetTexture()));
								}
							}
						}
					}
				}
				ImGui::EndMenu();
			}

			//if (texture_ids.size() == 0)
				AddTexture();
				int i = 0;
			for (map<string, uint>::iterator it = texture_ids.begin(); it != texture_ids.end(); it++)
			{
				ImGui::Text("%s", (*it).first.data());
				ImGui::Image((ImTextureID)(*it).second, ImVec2(50, 50));
				ChangeTextureNoMaterial((*it).first,i);
				i++;
			}

			ImGui::ColorEdit4("Color: ###materialColorDefault", color);
			ChooseAlphaType();

		}
		else
		{
			ImGui::Text("Material: %s", material_name.data());
			PrintMaterialProperties();
			if (rc_material->material.has_color)
			{
				ImGui::ColorEdit4("Color: ###materialColor", color);
				ChooseAlphaType();
			}
				
		}
		
		
	}
}

void ComponentMaterial::Update(float dt)
{

	
}

void ComponentMaterial::Save(Data & file)const
{
	Data data;
	data.AppendInt("type", type);
	data.AppendUInt("UUID", uuid);
	data.AppendBool("active", active);
	data.AppendString("path", material_path.data());
	data.AppendBool("properties_set", true);
	data.AppendUInt("alpha", alpha);
	data.AppendUInt("blend_type", blend_type);
	data.AppendFloat("alpha_test", alpha_test);
	data.AppendFloat3("color", color);
	data.AppendBool("texture_changed", texture_changed);

	if (material_path.size() == 0 || texture_changed)
	{
		data.AppendArray("textures");
		for (vector<string>::const_iterator it = list_textures_paths.begin(); it != list_textures_paths.end(); it++)
		{
			Data texture;
			texture.AppendString("path", (*it).data());
			data.AppendArrayValue(texture);
		}
	}
	

	file.AppendArrayValue(data);
}

void ComponentMaterial::Load(Data & conf)
{
	uuid = conf.GetUInt("UUID");
	active = conf.GetBool("active");
	material_path = conf.GetString("path");
	bool properties_set = conf.GetBool("properties_set");
	if (properties_set)
	{
		alpha = conf.GetUInt("alpha");
		blend_type = conf.GetUInt("blend_type");
		alpha_test = conf.GetFloat("alpha_test");
		float3 color_tmp = conf.GetFloat3("color");
		color[0] = color_tmp[0]; color[1] = color_tmp[1]; color[2] = color_tmp[2]; color[3] = color_tmp[3];
		texture_changed = conf.GetBool("texture_changed");
	}

	if (material_path.size() != 0)
	{
		rc_material = (ResourceFileMaterial*)App->resource_manager->LoadResource(material_path, ResourceFileType::RES_MATERIAL);

		if (texture_changed == false)
		{
			for (vector<Uniform*>::iterator uni = rc_material->material.uniforms.begin(); uni != rc_material->material.uniforms.end(); ++uni)
			{
				if ((*uni)->type == UniformType::U_SAMPLER2D)
				{
					string texture_path;
					int name_size = *reinterpret_cast<int*>((*uni)->value);
					texture_path.resize(name_size);
					memcpy(texture_path._Myptr(), (*uni)->value + sizeof(int), name_size);


					ResourceFileType type = App->resource_manager->GetResourceType(texture_path.data());

					if (type == ResourceFileType::RES_TEXTURE)
					{
						ResourceFileTexture* rc_tmp = (ResourceFileTexture*)App->resource_manager->LoadResource(texture_path, ResourceFileType::RES_TEXTURE);
						tex_resources.push_back(rc_tmp);
						texture_ids.insert(pair<string, uint>((*uni)->name.data(), rc_tmp->GetTexture()));
					}
					else
					{	
						ResourceFileRenderTexture* rc_rndtx = (ResourceFileRenderTexture*)App->resource_manager->LoadResource(texture_path, ResourceFileType::RES_RENDER_TEX);
						tex_resources.push_back(rc_rndtx);
						texture_ids.insert(pair<string, uint>((*uni)->name.data(), rc_rndtx->GetTexture()));
					}

				
				}
			}
		}
		else
		{
			
			int i = 0;
			for (vector<Uniform*>::iterator uni = rc_material->material.uniforms.begin(); uni != rc_material->material.uniforms.end(); ++uni)
			{
				if ((*uni)->type == UniformType::U_SAMPLER2D)
				{
					Data texture;
					texture = conf.GetArray("textures", i);
					i++;
					string tex_path = texture.GetString("path");

					ResourceFileTexture* rc_tmp = (ResourceFileTexture*)App->resource_manager->LoadResource(tex_path, ResourceFileType::RES_TEXTURE);

					if (rc_tmp)
					{
						int sampler_size = tex_path.size();
						char* content = new char[sizeof(int) + sampler_size * sizeof(char)];
						memcpy(content, &sampler_size, sizeof(int));
						char* pointer = content + sizeof(int);
						memcpy(pointer, tex_path.c_str(), sizeof(char)* sampler_size);
						int size;
						memcpy(&size, content, sizeof(int));
						(*uni)->value = new char[sizeof(int) + sizeof(char) * size];
						memcpy((*uni)->value, content, sizeof(int) + sizeof(char) * size);

						tex_resources.push_back(rc_tmp);
						texture_ids.insert(pair<string, uint>((*uni)->name.data(), rc_tmp->GetTexture()));
						list_textures_paths.push_back(tex_path);
					}
					else
					{
						ResourceFileTexture* rc_rndtx = (ResourceFileTexture*)App->resource_manager->LoadResource(tex_path, ResourceFileType::RES_RENDER_TEX);
						if (rc_rndtx)
						{
							int sampler_size = tex_path.size();
							char* content = new char[sizeof(int) + sampler_size * sizeof(char)];
							memcpy(content, &sampler_size, sizeof(int));
							char* pointer = content + sizeof(int);
							memcpy(pointer, tex_path.c_str(), sizeof(char)* sampler_size);
							int size;
							memcpy(&size, content, sizeof(int));
							(*uni)->value = new char[sizeof(int) + sizeof(char) * size];
							memcpy((*uni)->value, content, sizeof(int) + sizeof(char) * size);
							tex_resources.push_back(rc_rndtx);
							texture_ids.insert(pair<string, uint>((*uni)->name.data(), rc_rndtx->GetTexture()));
							list_textures_paths.push_back(tex_path);
						}
						else
							LOG("Material: error while loading texture %s", tex_path.data());
					}
				}
				

			}
			//RefreshTextures();
		}

	}
	else //Default material
	{
		Data texture;

		unsigned int tex_size = conf.GetArraySize("textures");
		for (int i = 0; i < tex_size; i++)
		{
			texture = conf.GetArray("textures", i);

			string tex_path = texture.GetString("path");
			ResourceFileTexture* rc_tmp = (ResourceFileTexture*)App->resource_manager->LoadResource(tex_path, ResourceFileType::RES_TEXTURE);

			if (rc_tmp)
			{
				tex_resources.push_back(rc_tmp);
				texture_ids.insert(pair<string, uint>(std::to_string(i), rc_tmp->GetTexture()));
				list_textures_paths.push_back(tex_path);
			}
			else
			{
					LOG("Material: error while loading texture %s", tex_path.data());
			}
			
		}

	}
	
}

bool ComponentMaterial::DefaultMaterialInspector()
{
	bool ret = false;
	ret = AddTexture();
	int i = 0;
	bool ret_tmp = false;
	for (map<string, uint>::iterator it = texture_ids.begin(); it != texture_ids.end(); it++)
	{
		
		ImGui::Text("%s", (*it).first.data());
		ImGui::Image((ImTextureID)(*it).second, ImVec2(50, 50));
		ret_tmp = ChangeTextureNoMaterial((*it).first,i);
		if (ret_tmp)
			ret = true;
		i++;
	}

	ImGui::ColorEdit4("Color: ###materialColorDefault", color);
	ChooseAlphaType();
	return ret;
}

void ComponentMaterial::PrintMaterialProperties()
{
	for (vector<Uniform*>::iterator it = rc_material->material.uniforms.begin(); it != rc_material->material.uniforms.end(); ++it)
	{
		ImGui::Text("Variable: ");
		ImGui::SameLine();
		ImGui::Text("%s", (*it)->name.data());

		switch ((*it)->type)
		{
		case U_BOOL:
			ImGui::Checkbox("###bool_u", reinterpret_cast<bool*>((*it)->value));
			break;
		case U_INT:
			ImGui::InputInt("###int_u", reinterpret_cast<int*>((*it)->value));
			break;
		case U_FLOAT:
			ImGui::InputFloat("###float_m", reinterpret_cast<float*>((*it)->value));
			break;
		case U_VEC2:
			ImGui::InputFloat2("###float2_u", reinterpret_cast<float*>((*it)->value));
			break;
		case U_VEC3:
			ImGui::InputFloat3("###float3_u", reinterpret_cast<float*>((*it)->value));
			break;
		case U_VEC4:
			ImGui::InputFloat4("###float4_u", reinterpret_cast<float*>((*it)->value));
			break;
		case U_MAT4X4:
			ImGui::Text("Matrices can be initalized only by code");
			break;
		case U_SAMPLER2D:
			string tex_name;
			tex_name.resize(*reinterpret_cast<int*>((*it)->value));
			memcpy(tex_name._Myptr(), (*it)->value + sizeof(int), *reinterpret_cast<int*>((*it)->value));

			//Commented because its to hard to serialize each time you change it in runtime, better use create Material window for custom shaders.
			ChangeTexture(tex_name, (*it));

			/*ResourceFileTexture* rc = texture_list.at(tex_name.data());
			if (rc)
			{
				ImGui::Text("Texture: %s", tex_name.data());
				ImGui::Image((ImTextureID)rc->GetTexture(), ImVec2(50, 50));
			}*/
			break;
		}
		ImGui::Separator();
	}
}

void ComponentMaterial::ChooseAlphaType()
{
	if (ImGui::RadioButton("OPAQUE", alpha == 0))
	{
		alpha = 0;
	}
	ImGui::SameLine();

	if (ImGui::RadioButton("ALPHA", alpha == 1))
	{
		alpha = 1;
	}
	ImGui::SameLine();

	if (ImGui::RadioButton("BLEND", alpha == 2))
	{
		alpha = 2;
	}

	if (alpha > 0)
	{
		if (ImGui::DragFloat("##MaterialAlphaTest", &alpha_test, 0.01f, 0.0f, 1.0f));
	}

	if (alpha == 2)
	{
		if (ImGui::CollapsingHeader("Alpha Blend Types"))
		{
			ImGui::RadioButton("Zero##BlendTypes", &blend_type, GL_ZERO);
			ImGui::RadioButton("One##BlendTypes", &blend_type, GL_ONE);
			ImGui::RadioButton("Src_Color##BlendTypes", &blend_type, GL_SRC_COLOR);
			ImGui::RadioButton("One_Minus_Src_Color##BlendTypes", &blend_type, GL_ONE_MINUS_SRC_COLOR);
			ImGui::RadioButton("Dst_Color##BlendTypes", &blend_type, GL_DST_COLOR);
			ImGui::RadioButton("One_Minus_Dst_Color##BlendTypes", &blend_type, GL_ONE_MINUS_DST_COLOR);
			ImGui::RadioButton("Src_Alpha##BlendTypes", &blend_type, GL_SRC_ALPHA);
			ImGui::RadioButton("One_Minus_Src_Alpha##BlendTypes", &blend_type, GL_ONE_MINUS_SRC_ALPHA);
			ImGui::RadioButton("Dst_Alpha##BlendTypes", &blend_type, GL_DST_ALPHA);
			ImGui::RadioButton("One_Minus_Dst_Alpha##BlendTypes", &blend_type, GL_ONE_MINUS_DST_ALPHA);
			ImGui::RadioButton("Constant_Color##BlendTypes", &blend_type, GL_CONSTANT_COLOR);
			ImGui::RadioButton("One_Minus_Constant_Color##BlendTypes", &blend_type, GL_ONE_MINUS_CONSTANT_COLOR);
			ImGui::RadioButton("Constant_Alpha##BlendTypes", &blend_type, GL_CONSTANT_ALPHA);
			ImGui::RadioButton("One_Minus_Constant_Alpha##BlendTypes", &blend_type, GL_ONE_MINUS_CONSTANT_ALPHA);
		}
	}

}

bool ComponentMaterial::ChangeTextureNoMaterial(string tex_name, int num)
{
	bool ret = false;
	ImGui::Text("Change Texture: ");
	ImGui::SameLine();
	std::string str = ("Textures: ##ChangeTexture" + tex_name);
	if (ImGui::BeginMenu(str.c_str()))
	{
		vector<string> textures;
		App->editor->assets->GetAllFilesByType(FileType::IMAGE, textures);

		for (vector<string>::iterator it = textures.begin(); it != textures.end(); ++it)
		{
			if (ImGui::MenuItem((*it).data()))
			{
				
				string u_sampler2d = App->resource_manager->FindFile(*it);
				ResourceFileTexture* rc_tmp = (ResourceFileTexture*)App->resource_manager->LoadResource(u_sampler2d, ResourceFileType::RES_TEXTURE);
				if (rc_tmp)
				{
					tex_resources.pop_back();
					texture_ids.at(std::to_string(num)) = rc_tmp->GetTexture();
					list_textures_paths.pop_back();
					tex_resources.push_back(rc_tmp);
					list_textures_paths.push_back(u_sampler2d);
					ret = true;
				}
				else
				{
					LOG("Material: error while loading texture %s", (*it).data());
				}
			}
		}
		ImGui::EndMenu();
	}
	return ret;
}

void ComponentMaterial::ChangeTexture(string tex_name, Uniform* &value)
{
		ImGui::Text("Texture: ");
		ImGui::SameLine();
		string name = "Textures:";
		if (value->name != "")
			name = value->name;
		std::string str = (name + "##ChangeTexture" + tex_name);
		if (ImGui::BeginMenu(str.c_str()))
		{
			vector<string> textures;
			App->editor->assets->GetAllFilesByType(FileType::IMAGE, textures);
			App->editor->assets->GetAllFilesByType(FileType::RENDER_TEXTURE, textures);

			for (vector<string>::iterator it = textures.begin(); it != textures.end(); ++it)
			{
				if (ImGui::MenuItem((*it).data()))
				{
					string u_sampler2d = App->resource_manager->FindFile(*it);

					if (value != nullptr)
					{
						int sampler_size = u_sampler2d.size();
						char* content = new char[sizeof(int) + sampler_size * sizeof(char)];
						memcpy(content, &sampler_size, sizeof(int));
						char* pointer = content + sizeof(int);
						memcpy(pointer, u_sampler2d.c_str(), sizeof(char)* sampler_size);
						int size;
						memcpy(&size, content, sizeof(int));
						value->value = new char[sizeof(int) + sizeof(char) * size];
						memcpy(value->value, content, sizeof(int) + sizeof(char) * size);
						RefreshTextures();

						texture_changed = true;
					}
					
				}
			}
			ImGui::EndMenu();
		}
}

void ComponentMaterial::RefreshTextures()
{
	texture_ids.clear();
	list_textures_paths.clear();
	for (vector<Uniform*>::iterator uni = rc_material->material.uniforms.begin(); uni != rc_material->material.uniforms.end(); ++uni)
	{
		if ((*uni)->type == UniformType::U_SAMPLER2D)
		{
			string texture_path;
			int name_size = *reinterpret_cast<int*>((*uni)->value);
			texture_path.resize(name_size);
			memcpy(texture_path._Myptr(), (*uni)->value + sizeof(int), name_size);


			ResourceFileType type = App->resource_manager->GetResourceType(texture_path.data());

			if (type == ResourceFileType::RES_TEXTURE)
			{
				ResourceFileTexture* rc_tmp = (ResourceFileTexture*)App->resource_manager->LoadResource(texture_path, ResourceFileType::RES_TEXTURE);
				tex_resources.push_back(rc_tmp);
				texture_ids.insert(pair<string, uint>((*uni)->name.data(), rc_tmp->GetTexture()));
				list_textures_paths.push_back(texture_path);
			}
			else
			{
				ResourceFileRenderTexture* rc_rndtx = (ResourceFileRenderTexture*)App->resource_manager->LoadResource(texture_path, ResourceFileType::RES_RENDER_TEX);
				tex_resources.push_back(rc_rndtx);
				texture_ids.insert(pair<string, uint>((*uni)->name.data(), rc_rndtx->GetTexture()));
				list_textures_paths.push_back(texture_path);
			}
		}
	}
}

bool ComponentMaterial::AddTexture()
{
	bool ret = false;
	ImGui::Text("Add Texture: ");
	ImGui::SameLine();
	std::string str = ("##AddTexture");
	if (ImGui::BeginMenu(str.c_str()))
	{
		vector<string> textures;
		
		App->editor->assets->GetAllFilesByType(FileType::IMAGE, textures);

		for (vector<string>::iterator it = textures.begin(); it != textures.end(); ++it)
		{
			if (ImGui::MenuItem((*it).data()))
			{
				string path = App->resource_manager->FindFile((*it).data());
				ResourceFileTexture* rc_tmp = (ResourceFileTexture*)App->resource_manager->LoadResource(path, ResourceFileType::RES_TEXTURE);
				if (rc_tmp)
				{
					tex_resources.push_back(rc_tmp);
					texture_ids.insert(pair<string, uint>(to_string(texture_ids.size()), rc_tmp->GetTexture()));
					list_textures_paths.push_back(path);
					ret = true;
				}
				else
				{
					LOG("Material: error while loading texture %s", (*it).data());
				}
			}
		}
		ImGui::EndMenu();
	}
	return ret;
}

void ComponentMaterial::CleanUp()
{
	for (vector<ResourceFile*>::iterator it = tex_resources.begin(); it != tex_resources.end(); it++)
	{
		(*it)->Unload();
	}

	texture_ids.clear();
	list_textures_paths.clear();

}
