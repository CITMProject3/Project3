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
#include "ComponentAudio.h"
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
	if (game_object->IsActive())
	{
		if (App->input->GetJoystickButton(player_num, JOY_BUTTON::START) == KEY_DOWN || App->input->GetKey(SDL_SCANCODE_G) == KEY_STATE::KEY_DOWN)
		{
			// Only for Vertical Slice 3 --> Launching Double Drum sound
			ComponentAudio *a = (ComponentAudio*)game_object->GetComponent(ComponentType::C_AUDIO);
			if (a) App->audio->PostEvent(a->GetEvent(), a->GetWiseID());

			if (UImaterial->texture_ids.size() >= 2)
			{
				uint tmp = UImaterial->texture_ids.at("0");
				UImaterial->texture_ids.at("0") = UImaterial->texture_ids.at("1");
				UImaterial->texture_ids.at("1") = tmp;
				ready = !ready;
				if (App->go_manager->current_scene_canvas != nullptr)
				{
					App->go_manager->current_scene_canvas->SetPlayerReady(player_num, ready);
				}
			}
		}
	}
	else
		ready = false;
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
		int tmp_num = player_num + 1;
		if(ImGui::InputInt("Player Number", &tmp_num, 1, 1))
		{
			if (tmp_num >= 2)
				player_num = 1;
			else
				player_num = 0;
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

void ComponentUiButton::Save(Data & file) const
{
	Data data;

	data.AppendInt("type", type);
	data.AppendUInt("UUID", uuid);
	data.AppendBool("active", active);
	data.AppendUInt("player_num", player_num);
	data.AppendArray("Material");
	UImaterial->Save(data);
	file.AppendArrayValue(data);
}

void ComponentUiButton::Load(Data & conf)
{
	uuid = conf.GetUInt("UUID");
	active = conf.GetBool("active");
	player_num = conf.GetUInt("player_num");
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
