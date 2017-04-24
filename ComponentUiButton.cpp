#include "ComponentUiButton.h"
#include "Application.h"
#include "Assets.h"
#include "ModuleResourceManager.h"
#include "ModuleGOManager.h"
#include "ModuleInput.h"
#include "ComponentRectTransform.h"
#include "ResourceFileTexture.h"
#include "GameObject.h"
#include "ComponentMaterial.h"
#include "ComponentCanvas.h"
#include "imgui\imgui.h"

// Only for Vertical Slice 3
#include "SDL\include\SDL_scancode.h"
#include "ComponentAudioSource.h"
#include "ModuleAudio.h"

ComponentUiButton::ComponentUiButton(ComponentType type, GameObject * game_object) : Component(type, game_object)
{
	UImaterial = new ComponentMaterial(C_MATERIAL, nullptr);
}

ComponentUiButton::~ComponentUiButton()
{
	delete UImaterial;
}

void ComponentUiButton::Update()
{
}

void ComponentUiButton::CleanUp()
{
}

void ComponentUiButton::OnInspector(bool debug)
{
	std::string str = (std::string("UI Button") + std::string("##") + std::to_string(uuid));
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

void ComponentUiButton::OnFocus()
{
}

void ComponentUiButton::OnPress()
{
	if (pressed == false)
	{
		UImaterial->SetIdToRender(UImaterial->GetIdToRender() + 1);
		pressed = true;
	}
	else
	{
		UImaterial->SetIdToRender(UImaterial->GetIdToRender() -1);
		pressed = false;
	}
}

void ComponentUiButton::OnPressId(int i)
{
	if (pressed == false)
	{
		UImaterial->SetIdToRender(i + 1);
		pressed = true;
	}
	else
	{
		UImaterial->SetIdToRender(0);
		pressed = false;
	}
}

void ComponentUiButton::ChangeState()
{
	pressed = !pressed;
}

void ComponentUiButton::Save(Data & file) const
{
	Data data;

	data.AppendInt("type", type);
	data.AppendUInt("UUID", uuid);
	data.AppendBool("active", active);
	data.AppendArray("Material");
	UImaterial->Save(data);
	file.AppendArrayValue(data);
}

void ComponentUiButton::Load(Data & conf)
{
	uuid = conf.GetUInt("UUID");
	active = conf.GetBool("active");
	Data mat_file;
	mat_file = conf.GetArray("Material", 0);
	UImaterial->Load(mat_file);
}

void ComponentUiButton::Reset()
{
	if (UImaterial->texture_ids.size() >= 2)
	{
		uint tmp = UImaterial->texture_ids.at("0");
		UImaterial->texture_ids.at("0") = UImaterial->texture_ids.at("1");
		UImaterial->texture_ids.at("1") = tmp;
	}
}

void ComponentUiButton::ResetId(int i)
{
	if (UImaterial->texture_ids.size() >= i+1)
	{
		string str = to_string(i+1);
		uint tmp = UImaterial->texture_ids.at("0");
		UImaterial->texture_ids.at("0") = UImaterial->texture_ids.at(str);
		UImaterial->texture_ids.at(str) = tmp;
	}
}

bool ComponentUiButton::GetState() const
{
	return pressed;
}
