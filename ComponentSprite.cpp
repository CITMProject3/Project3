#include "ComponentSprite.h"

#include "imgui\imgui.h"

#include "ResourceFileTexture.h"

#include "Application.h"
#include "Assets.h"
#include "ModuleEditor.h"
#include "ModuleRenderer3D.h"

ComponentSprite::ComponentSprite(ComponentType type, GameObject* game_object) : Component(type, game_object)
{}

ComponentSprite::~ComponentSprite()
{}

void ComponentSprite::Update()
{
	if(texture && active)
		App->renderer3D->AddToDrawSprite(this);
}

void ComponentSprite::OnInspector(bool debug)
{
	if (ImGui::CollapsingHeader("Sprite", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (ImGui::IsItemClicked(1))
			ImGui::OpenPopup("delete##sprite");

		if (ImGui::BeginPopup("delete##sprite"))
		{
			if (ImGui::MenuItem("Delete"))
				Remove();
			ImGui::EndPopup();
		}

		//Show texture
		if (texture)
		{
			ImGui::Image((ImTextureID)texture->GetTexture(), ImVec2(50, 50));
		}

		//Change texture
		ImGui::Text("Change sprite: ");
		if (ImGui::BeginMenu("###sprite_change_tex"))
		{
			ChangeTexture();
			ImGui::EndMenu();
		}

	}
}

unsigned int ComponentSprite::GetTextureId() const
{
	if (texture)
		return texture->GetTexture();

	return 0;
}

void ComponentSprite::ChangeTexture()
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
				if (texture)
					texture->Unload();
				texture = rc_tmp;
			}
			else
			{
				LOG("[ERROR] Loading failure on sprite %s", (*it).data());
				App->editor->DisplayWarning(WarningType::W_ERROR, "Loading failure on sprite %s", (*it).data());
			}
		}
	}
}