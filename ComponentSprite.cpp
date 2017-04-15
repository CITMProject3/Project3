#include "ComponentSprite.h"

#include "imgui\imgui.h"

#include "ResourceFileTexture.h"

#include "Application.h"
#include "Assets.h"
#include "GameObject.h"

#include "ModuleEditor.h"
#include "ModuleRenderer3D.h"
#include "DebugDraw.h"

ComponentSprite::ComponentSprite(ComponentType type, GameObject* game_object) : Component(type, game_object)
{
	aabb.SetNegativeInfinity();
	bounding_box.SetNegativeInfinity();
}

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

			ImGui::Text("Width: %i Height: %i", width, height);
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

void ComponentSprite::OnTransformModified()
{
	math::OBB ob = aabb.Transform(game_object->GetGlobalMatrix());
	bounding_box = ob.MinimalEnclosingAABB();
	game_object->bounding_box = &bounding_box;
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
				width = texture->GetWidth();
				height = texture->GetHeight();
				size.x = width / 100.0f;
				size.y = height / 100.0f;

				float max_size = (width > height) ? width : height;
				max_size = (max_size / 100.0f) * 0.5f;
				aabb = math::AABB(math::vec(-max_size), math::vec(max_size));
				OnTransformModified();
			}
			else
			{
				LOG("[ERROR] Loading failure on sprite %s", (*it).data());
				App->editor->DisplayWarning(WarningType::W_ERROR, "Loading failure on sprite %s", (*it).data());
			}
		}
	}
}

