#include "Application.h"
#include "ComponentMaterial.h"
#include "GameObject.h"
#include "imgui\imgui.h"
#include "Data.h"
#include "ResourceFileTexture.h"
#include "Assets.h"
#include "ResourceFileMaterial.h"
#include "ResourceFileRenderTexture.h"

ComponentMaterial::ComponentMaterial(ComponentType type, GameObject* game_object) : Component(type, game_object)
{}

ComponentMaterial::~ComponentMaterial()
{
	CleanUp();
}

void ComponentMaterial::OnInspector()
{
	if (ImGui::CollapsingHeader("Material"))
	{
		if (material_path.size() == 0)
		{
			ImGui::Text("Material: Default material");

			for (map<string, uint>::iterator it = texture_ids.begin(); it != texture_ids.end(); it++)
			{
				ImGui::Text("%s", (*it).first.data());
				ImGui::Image((ImTextureID)(*it).second, ImVec2(50, 50));
			}
		}
		else
		{
			ImGui::Text("Material: %s", material_name.data());
			PrintMaterialProperties();
		}
		

		if (ImGui::Button("Change material"))
		{
			change_material_enabled = true;
		}

		if (change_material_enabled)
		{
			if (ImGui::BeginMenu("Select a material"))
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
		}
	}
}

void ComponentMaterial::Update()
{

	
}

void ComponentMaterial::Save(Data & file)const
{
	Data data;
	data.AppendInt("type", type);
	data.AppendUInt("UUID", uuid);
	data.AppendBool("active", active);
	data.AppendString("path", material_path.data());

	if (material_path.size() == 0)
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

	if (material_path.size() != 0)
	{
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
				texture_ids.insert(pair<string, uint>("", rc_tmp->GetTexture()));
				list_textures_paths.push_back(tex_path);
			}
			else
			{
				LOG("Material: error while loading texture %s", tex_path.data());
			}
			
		}

	}
	
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

void ComponentMaterial::CleanUp()
{
	for (vector<ResourceFile*>::iterator it = tex_resources.begin(); it != tex_resources.end(); it++)
	{
		(*it)->Unload();
	}

	texture_ids.clear();
	list_textures_paths.clear();

}
