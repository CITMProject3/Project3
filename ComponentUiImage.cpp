#include "Application.h"
#include "Editor.h"
#include "Assets.h"
#include "ComponentUiImage.h"
#include "ModuleResourceManager.h"
#include "ResourceFileTexture.h"
#include "GameObject.h"
#include "ComponentMaterial.h"
#include "imgui\imgui.h"

ComponentUiImage::ComponentUiImage(ComponentType type, GameObject * game_object) : Component(type,game_object)
{
	UImaterial = new ComponentMaterial(C_MATERIAL,nullptr);
}

void ComponentUiImage::Update(float dt)
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
		UImaterial->DefaultMaterialInspector();
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