#include "ComponentUiImage.h"
#include "Application.h"
#include "Assets.h"
#include "ModuleResourceManager.h"
#include "ComponentRectTransform.h"
#include "ResourceFileTexture.h"
#include "GameObject.h"
#include "ComponentMaterial.h"
#include "imgui\imgui.h"

ComponentUiImage::ComponentUiImage(ComponentType type, GameObject * game_object) : Component(type,game_object)
{
	UImaterial = new ComponentMaterial(C_MATERIAL,nullptr);
}

ComponentUiImage::~ComponentUiImage()
{
	delete UImaterial;
}

void ComponentUiImage::Update()
{
}

void ComponentUiImage::CleanUp()
{
}

void ComponentUiImage::OnInspector(bool debug)
{
	std::string str = (std::string("UI Image") + std::string("##") + std::to_string(uuid));
	if (ImGui::CollapsingHeader(str.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (ImGui::IsItemClicked(1))
		{
			ImGui::OpenPopup("delete##uiImage");
		}

		if (ImGui::BeginPopup("delete##uiImage"))
		{
			if (ImGui::MenuItem("Delete"))
			{
				Remove();
			}
			ImGui::EndPopup();
		}


		UImaterial->DefaultMaterialInspector();
		if (ImGui::Button("Set Size"))
		{
			string tex_path = (*UImaterial->list_textures_paths.begin());
			ResourceFileTexture* rc_tmp = (ResourceFileTexture*)App->resource_manager->LoadResource(tex_path, ResourceFileType::RES_TEXTURE);
			ComponentRectTransform* c = (ComponentRectTransform*)game_object->GetComponent(C_RECT_TRANSFORM);
			c->SetSize(float2(rc_tmp->GetWidth(), rc_tmp->GetHeight()));
			c->OnTransformModified();
		}
	}
}



void ComponentUiImage::Save(Data & file) const
{
	Data data;

	data.AppendInt("type", type);
	data.AppendUInt("UUID", uuid);
	data.AppendBool("active", active);
	data.AppendArray("Material");
	UImaterial->Save(data);
	file.AppendArrayValue(data);
}

void ComponentUiImage::Load(Data & conf)
{
	uuid = conf.GetUInt("UUID");
	active = conf.GetBool("active");
	Data mat_file;
	mat_file = conf.GetArray("Material", 0);
	UImaterial->Load(mat_file);
}